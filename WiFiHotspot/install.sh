#!/bin/bash
# Install the concurrent AP+STA WiFi hotspot and its GNOME Quick Settings toggle.
#   sudo ./install.sh
set -e

[[ $EUID -eq 0 ]] || { echo "run with sudo" >&2; exit 1; }

SRC=$(cd "$(dirname "$0")" && pwd)
REAL_USER=${SUDO_USER:-$(logname 2>/dev/null || true)}

echo "==> installing hostapd"
dnf install -y hostapd

echo "==> /usr/local/bin/hotspot"
install -m 0755 "$SRC/hotspot" /usr/local/bin/hotspot

echo "==> keeping NetworkManager off ap0"
install -m 0644 "$SRC/99-ap0-unmanaged.conf" \
    /etc/NetworkManager/conf.d/99-ap0-unmanaged.conf
nmcli general reload conf 2>/dev/null || systemctl reload NetworkManager

echo "==> systemd unit"
install -m 0644 "$SRC/hotspot.service" /etc/systemd/system/hotspot.service
systemctl daemon-reload

echo "==> polkit rule (passwordless toggle for wheel)"
install -m 0644 "$SRC/49-hotspot.rules" /etc/polkit-1/rules.d/49-hotspot.rules

if [[ ! -f /etc/hotspot.conf ]]; then
    echo "==> generating /etc/hotspot.conf with a random passphrase"
    cat > /etc/hotspot.conf <<EOF
# Hotspot settings. Keep this file out of version control.
SSID=fedora-hotspot
PSK=$(tr -dc 'a-z0-9' </dev/urandom | head -c 12)
NET=192.168.12
# PARENT=wlp0s20f3   # auto-detected when unset
EOF
    chmod 600 /etc/hotspot.conf
else
    echo "==> /etc/hotspot.conf already exists, leaving it alone"
fi

if [[ -n $REAL_USER ]]; then
    EXTDIR=$(getent passwd "$REAL_USER" | cut -d: -f6)/.local/share/gnome-shell/extensions/hotspot-toggle@local
    echo "==> GNOME Quick Settings toggle -> $EXTDIR"
    install -d -o "$REAL_USER" -g "$REAL_USER" "$EXTDIR"
    install -m 0644 -o "$REAL_USER" -g "$REAL_USER" \
        "$SRC/gnome-extension/metadata.json" "$SRC/gnome-extension/extension.js" "$EXTDIR/"
    sudo -u "$REAL_USER" env DISPLAY="${DISPLAY:-:0}" bash -c '
        cur=$(gsettings get org.gnome.shell enabled-extensions)
        case "$cur" in
            *hotspot-toggle@local*) ;;
            "@as []"|"[]") gsettings set org.gnome.shell enabled-extensions "[\x27hotspot-toggle@local\x27]" ;;
            *) gsettings set org.gnome.shell enabled-extensions "${cur%]}, \x27hotspot-toggle@local\x27]" ;;
        esac
        gsettings set org.gnome.shell disable-user-extensions false' 2>/dev/null || \
        echo "    (could not enable via gsettings -- enable it manually)"
fi

echo
echo "Done. Review the passphrase in /etc/hotspot.conf, then:"
echo "  sudo hotspot on"
echo "Log out and back in for the Quick Settings toggle to appear (Wayland)."
