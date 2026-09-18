#!/bin/bash
# Zephyr RTOS lab setup - pulls everything from the local lab server over LAN.
# Bundles its own Python 3.12, so your system Python version does not matter.
# Works on Debian/Ubuntu (apt) and Fedora/RHEL (dnf).
set -e

# ---- EDIT THIS: your lab server address ----
SERVER="http://192.168.1.100:8080"
SERVER_HOST="${SERVER#http://}"; SERVER_HOST="${SERVER_HOST%%:*}"
WORKDIR="$HOME/zephyr-work"
# identifies this laptop in the server download log
ID="?host=$(hostname)&user=$USER"

echo "=== 1/7: Installing system build dependencies (needs your sudo password) ==="
if command -v apt >/dev/null 2>&1; then
  sudo apt update
  sudo apt install -y --no-install-recommends \
    git cmake ninja-build gperf ccache dfu-util device-tree-compiler wget \
    xz-utils file make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1 \
    openocd
elif command -v dnf >/dev/null 2>&1; then
  sudo dnf install -y \
    git cmake ninja-build gperf ccache dfu-util dtc wget xz file make \
    gcc gcc-c++ SDL2-devel file-libs openocd
else
  echo "ERROR: need apt or dnf; neither found. Install build tools manually."
  exit 1
fi

mkdir -p "$WORKDIR" && cd "$WORKDIR"

echo "=== 2/7: Downloading bundled Python 3.12 from lab server ==="
wget -q --show-progress -O python3.12-standalone_linux-x86_64.tar.xz "$SERVER/python3.12-standalone_linux-x86_64.tar.xz$ID"
tar xf python3.12-standalone_linux-x86_64.tar.xz
rm python3.12-standalone_linux-x86_64.tar.xz
PYTHON="$WORKDIR/python3.12-standalone/bin/python3"
echo "Using Python: $($PYTHON --version)"

echo "=== 3/7: Downloading Zephyr SDK from lab server ==="
wget -q --show-progress -O zephyr-sdk-1.0.1_linux-x86_64.tar.xz "$SERVER/zephyr-sdk-1.0.1_linux-x86_64.tar.xz$ID"
tar xf zephyr-sdk-1.0.1_linux-x86_64.tar.xz
rm zephyr-sdk-1.0.1_linux-x86_64.tar.xz

echo "=== 4/7: Running SDK setup (registers udev rules for USB board flashing) ==="
cd "$WORKDIR/zephyr-sdk-1.0.1"
yes | ./setup.sh -t all -h -c

echo "=== 5/7: Downloading Zephyr workspace (source + all board/driver modules) ==="
cd "$WORKDIR"
wget -q --show-progress -O zephyrproject.tar.xz "$SERVER/zephyrproject.tar.xz$ID"
tar xf zephyrproject.tar.xz
rm zephyrproject.tar.xz

echo "=== 6/7: Setting up Python environment from local wheelhouse (no internet needed) ==="
"$PYTHON" -m venv "$WORKDIR/venv"
source "$WORKDIR/venv/bin/activate"
pip install --trusted-host "$SERVER_HOST" --no-index --find-links "$SERVER/wheelhouse" west
cd "$WORKDIR/zephyrproject"
pip install --trusted-host "$SERVER_HOST" --no-index --find-links "$SERVER/wheelhouse" -r zephyr/scripts/requirements.txt

echo "=== 7/7: Adding you to groups for USB board access ==="
ADDGROUPS=""
for g in dialout plugdev; do
  if getent group "$g" >/dev/null 2>&1; then ADDGROUPS="$ADDGROUPS,$g"; fi
done
ADDGROUPS="${ADDGROUPS#,}"
if [ -n "$ADDGROUPS" ]; then
  sudo usermod -aG "$ADDGROUPS" "$USER"
  echo "Added to: $ADDGROUPS"
else
  echo "Note: no dialout/plugdev group found; you may need to set USB permissions manually."
fi

# openocd and the SDK both drop udev rules; reload so a debug probe works
# without rebooting. The board still needs a replug to pick up new rules.
if command -v udevadm >/dev/null 2>&1; then
    sudo udevadm control --reload-rules && sudo udevadm trigger
    echo "udev rules reloaded (unplug and replug your board before flashing)"
fi

cat << 'EOF'

==================================================
Setup complete!

IMPORTANT: log out and back in for USB group permissions to take effect.

To build your first sample after logging back in:

  export ZEPHYR_SDK_INSTALL_DIR=~/zephyr-work/zephyr-sdk-1.0.1
  source ~/zephyr-work/venv/bin/activate
  source ~/zephyr-work/zephyrproject/zephyr/zephyr-env.sh
  cd ~/zephyr-work/zephyrproject
  west build -p always -b qemu_x86 zephyr/samples/hello_world
  west build -t run          # runs it in QEMU simulation

To flash a real board, replace qemu_x86 with your board name, then:
  west flash
==================================================
EOF
