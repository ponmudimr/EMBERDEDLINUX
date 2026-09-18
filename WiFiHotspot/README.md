# WiFi Hotspot on Fedora (concurrent AP + Station)

Share a WiFi connection over a hotspot **from the same WiFi card**, while staying
connected to that network — the Linux equivalent of Windows' *Mobile Hotspot*.

Built and tested on **Fedora 44**, GNOME Shell 50 (Wayland), Intel CNVi WiFi
(`iwlwifi`).

---

## The problem

GNOME's built-in *"Turn On Wi-Fi Hotspot"* does **not** do this on an Intel card.
It calls NetworkManager's shared mode, which takes over the WiFi interface and
**drops the connection you were trying to share**.

The reason is a driver constraint. The card reports:

```text
valid interface combinations:
  * #{ managed } <= 1, #{ AP, P2P-client, P2P-GO } <= 1, #{ P2P-device } <= 1,
    total <= 3, #channels <= 1
```

So the hardware **can** run one station + one AP at once — but:

1. **Only one `managed` (station) interface is allowed.**
   NetworkManager hands every WiFi device it owns to `wpa_supplicant`, which
   brings each one up as a *station* first. A second interface therefore fails:

   ```text
   wpa_supplicant: Could not set interface ap0 flags (UP): Device or resource busy
   NetworkManager:  device (ap0): Couldn't initialize supplicant interface
   ```

   Having failed on `ap0`, NM falls back to seizing the primary interface.

2. **Both interfaces must share one channel** (`#channels <= 1`).

## The fix

Create a virtual **AP-type** interface (`ap0`) on the same PHY, keep
NetworkManager away from it, and drive it with **`hostapd`** directly. The
client link on `wlp0s20f3` is never touched.

```text
        ┌──────────── one physical radio (phy0) ────────────┐
        │                                                    │
   wlp0s20f3  ── managed ──> upstream WiFi (NetworkManager)   │
   ap0        ── AP      ──> hostapd + dnsmasq + NAT          │
        │         both locked to the SAME channel             │
        └────────────────────────────────────────────────────┘
```

`hostapd` serves the AP, `dnsmasq` hands out DHCP, and `firewalld` masquerades
onto the upstream interface.

---

## The channel trap

The AP is pinned to whatever channel the client link is on. That makes the
regulatory domain decisive — under country `IN`, **every** 5 GHz channel is
unusable for an AP:

| Band | Channels | Status |
| :--- | :--- | :--- |
| 2.4 GHz | 1–13 | usable |
| 5 GHz UNII-1 | 36–48 | `IR-CONCURRENT` — AP only alongside a station on that same channel |
| 5 GHz UNII-2 | 52–144 | `DFS` — radar detection required |
| 5 GHz UNII-3 | 149–165 | `no IR` — beaconing forbidden |

Check yours with:

```bash
iw reg get
iw phy phy0 info | grep -A2 "Frequencies"
```

**So the client link must be on 2.4 GHz.** Force it with:

```bash
nmcli con modify "<your-ssid>" 802-11-wireless.band bg
nmcli con up "<your-ssid>"
```

`hotspot on` refuses to start with a clear message if the client is parked on a
`no-IR` channel.

---

## Install

```bash
sudo ./install.sh
```

Installs `hostapd`, drops `hotspot` into `/usr/local/bin`, adds the systemd unit,
polkit rule and NetworkManager exclusion, generates `/etc/hotspot.conf` with a
random passphrase, and installs the GNOME toggle.

Then **log out and back in** — Wayland cannot restart GNOME Shell in place, so a
new extension only loads at login.

## Configure

`/etc/hotspot.conf` (mode `600`, never committed):

```bash
SSID=fedora-hotspot
PSK=your-passphrase      # 8-63 characters
NET=192.168.12           # /24; the host takes .1
# PARENT=wlp0s20f3       # auto-detected when unset
```

## Use

```bash
sudo hotspot on       # reads the live channel, brings everything up
sudo hotspot off      # tears down; client connection untouched
sudo hotspot status   # connected clients + current link
```

Or click the **Hotspot** tile in GNOME Quick Settings, next to Wi-Fi and
Bluetooth. It shows the SSID when on, `Off` when off, and adds a panel icon
while broadcasting. It polls every 5 s, so it stays accurate even if you use the
CLI instead.

---

## Files

| File | Installed to | Purpose |
| :--- | :--- | :--- |
| `hotspot` | `/usr/local/bin/hotspot` | Main script — creates `ap0`, writes configs, starts `hostapd`/`dnsmasq`, sets up NAT |
| `install.sh` | — | One-shot installer |
| `hotspot.service` | `/etc/systemd/system/` | `Type=oneshot` wrapper so the toggle can start/stop it |
| `49-hotspot.rules` | `/etc/polkit-1/rules.d/` | Lets `wheel` manage *only* `hotspot.service` without a password |
| `99-ap0-unmanaged.conf` | `/etc/NetworkManager/conf.d/` | Keeps NM and `wpa_supplicant` off `ap0` |
| `gnome-extension/` | `~/.local/share/gnome-shell/extensions/hotspot-toggle@local/` | Quick Settings toggle (GNOME 45+) |

---

## Verifying

Both interfaces live at once:

```console
$ iw dev | grep -E "Interface|type|ssid|channel"
	Interface ap0
		ssid fedora-hotspot
		type AP
		channel 1 (2412 MHz), width: 20 MHz
	Interface wlp0s20f3
		type managed
		channel 1 (2412 MHz), width: 20 MHz
```

The toggle path, as an unprivileged user — no password prompt:

```bash
systemctl --no-ask-password stop  hotspot.service
systemctl --no-ask-password start hotspot.service
```

---

## Notes & limitations

* **Roaming breaks the hotspot.** If the client link roams to another channel,
  the AP dies — both must share one channel. On a campus network with APs on
  channels 1/6/11, pin the BSSID:

  ```bash
  nmcli con modify "<ssid>" 802-11-wireless.bssid AA:BB:CC:DD:EE:FF
  ```

* **2.4 GHz only** in regulatory domains where 5 GHz is `no-IR`/DFS — expect
  ~50 Mbps rather than 5 GHz speeds.
* **Nothing persists across reboot** by design; the interface, firewall rules
  and `ip_forward` are all runtime-only. Run `sudo hotspot on` again, or
  `sudo systemctl enable hotspot` to start it at boot (it fails cleanly if WiFi
  isn't up yet).
* **The AP MAC** is derived from the parent interface with the
  locally-administered bit set and the last octet bumped.
* **Clients are on a trusted firewalld zone** and NAT out through your upstream
  connection — their traffic appears as yours on that network.

## Troubleshooting

| Symptom | Cause / fix |
| :--- | :--- |
| `Device or resource busy` on `ap0` | NM/`wpa_supplicant` grabbed it — check `99-ap0-unmanaged.conf` is installed and NM was reloaded |
| `hostapd` exits immediately | Read `/run/hotspot/hostapd.log`; usually a channel the regdomain forbids |
| Clients associate but get no DHCP | `dnsmasq` not running, or another instance owns port 67 — `pgrep -a dnsmasq` |
| Clients get an IP but no internet | `sysctl net.ipv4.ip_forward` should be `1`; `firewall-cmd --zone=<upstream-zone> --query-masquerade` should be `yes` |
| Toggle missing after install | Log out and back in; check `gnome-extensions list` |
