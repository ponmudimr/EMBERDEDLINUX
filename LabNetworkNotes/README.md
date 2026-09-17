# Accessing a Central Lab Server from a Laptop

Notes from running a teaching lab where desktop machines authenticate against a
central server, and why a laptop should **not** be configured the same way.

---

## The Lab Setup

Lab desktops use the classic centralised arrangement:

- **NIS** distributes user accounts, so any student can log in at any machine.
- **NFS** exports `/home` from the server, so their files follow them.

On machines that are permanently wired to the lab network, this works well.

---

## Why the Same Config Breaks a Laptop

Applying the lab client configuration to a laptop caused it to fail at boot once
it was off the network — badly enough to require reinstalling the OS.

Two causes, both structural rather than a misconfiguration:

**1. The home directory lives on the network.** The client config mounts the
server's `/home` over the local `/home`. Off-network, the user's home directory
simply does not exist at boot, so the graphical session cannot start. The login
screen accepts the password, then immediately bounces back.

**2. Account lookups block.** With `nis` in `/etc/nsswitch.conf`, every username
lookup queries a server that is not reachable and waits for the timeout. Login
becomes extremely slow or fails outright.

A mount entry without `nofail` compounds this — boot can stop and drop into
emergency mode rather than continuing.

> A laptop leaves the network. Any design that assumes the server is always
> reachable is the wrong design for one.

---

## A Safer Approach: sshfs on Demand

Instead of wiring the laptop into NIS/NFS, mount the lab files over SSH only when
they are actually wanted.

```bash
sudo dnf install -y fuse-sshfs        # or: sudo apt install -y sshfs
mkdir -p ~/lab
sshfs <user>@<server>:/home/<user> ~/lab
```

Unmount with `fusermount3 -u ~/lab`.

### Why this is better on a laptop

| | NIS + NFS client | sshfs |
| :--- | :--- | :--- |
| Boots off-network | Fails — no home directory | Unaffected |
| Runs at boot | Yes, blocking | No, on demand |
| Modifies PAM / nsswitch | Yes — risk of lockout | No |
| Encrypted | No | Yes, over SSH |
| UID mismatch | Requires NIS to resolve | Handled automatically |

### The UID detail

This one is easy to miss. NFS decides file access by **numeric UID**. If the
laptop account is UID 1000 and the server account is UID 1014, a plain NFS mount
gives permission denied on the user's own files — the names match, the numbers
do not.

`sshfs` avoids the problem entirely: the remote side acts as the remote user, and
files are presented to the local user regardless of the underlying UID. Files
written from the laptop still land on the server owned by the correct account.

---

## Convenience Wrappers

Two small scripts in `~/.local/bin`. The mount script checks reachability first,
so running it off-network reports a clear message instead of hanging:

```bash
#!/bin/bash
# lab-mount
SERVER=192.168.1.100
USER_ON_SERVER=labuser
MOUNT=~/lab

mountpoint -q "$MOUNT" && { echo "Already mounted at $MOUNT"; exit 0; }

if ! ping -c 1 -W 2 "$SERVER" > /dev/null 2>&1; then
    echo "Lab server ($SERVER) is not reachable - are you on the lab network?"
    exit 1
fi

mkdir -p "$MOUNT"
sshfs "$USER_ON_SERVER@$SERVER:/home/$USER_ON_SERVER" "$MOUNT" \
    -o reconnect,ServerAliveInterval=15,ServerAliveCountMax=3 \
    && echo "Lab files mounted at $MOUNT"
```

```bash
#!/bin/bash
# lab-unmount
MOUNT=~/lab
mountpoint -q "$MOUNT" && fusermount3 -u "$MOUNT" && echo "Unmounted $MOUNT" \
    || echo "Not mounted."
```

Adding an SSH key to the lab account removes the password prompt:

```bash
ssh-copy-id <user>@<server>
```

`reconnect` matters on a laptop — the mount survives a suspend/resume or a brief
Wi-Fi drop instead of going stale.

---

## Takeaway

Centralised authentication with network home directories suits machines that
never leave the network. For anything portable, mount on demand instead. The
laptop then boots and logs in normally everywhere, and the lab files are one
command away when they are needed.
