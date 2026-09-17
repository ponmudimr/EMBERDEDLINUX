# Offline Zephyr RTOS Distribution for a Teaching Lab

Serving a complete Zephyr RTOS development environment to ~20 student laptops over
a local network, so that no student machine needs a reliable internet connection.

---

## The Problem

A Zephyr RTOS install is large: the SDK (cross-compilers for every supported
architecture), the source workspace (Zephyr plus ~60 hardware-vendor module
repositories), and ~146 Python packages. A normal `west init && west update`
pulls roughly **20 GB** from GitHub and PyPI.

Twenty students each doing that on unreliable internet is not workable: it is slow,
it repeatedly fails partway, and every student ends up on a slightly different
version — which means their build errors are all different too.

**Approach:** build it once on a lab server, then let laptops pull it over LAN.

```
                 internet (once)
                       |
            +----------v-----------+
            |     Lab Server       |
            |  builds + packages   |
            |  HTTP file server    |
            +----------+-----------+
                       | LAN  (~6 GB per laptop)
        +--------------+--------------+
        |              |              |
   [ laptop ]     [ laptop ]     [ laptop ]  x20
```

---

## What Gets Served

| Artifact | Size | Purpose |
| :--- | :--- | :--- |
| `zephyr-sdk-<ver>_linux-x86_64.tar.xz` | 2.1 GB | Cross-compiler toolchains |
| `zephyrproject.tar.xz` | 3.6 GB | Zephyr source + all vendor modules, pinned to a release tag |
| `python3.12-standalone_linux-x86_64.tar.xz` | 25 MB | Self-contained Python interpreter |
| `wheelhouse/` | 203 MB | 146 prebuilt Python wheels |
| `student-setup.sh` | 3 KB | One-command installer |

Total per student: **~6 GB over LAN**, a few minutes instead of hours.

---

## Server Build

System build dependencies (Debian/Ubuntu):

```bash
sudo apt install -y --no-install-recommends \
  git cmake ninja-build gperf ccache dfu-util device-tree-compiler wget \
  xz-utils file make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1
```

Fetch the SDK, build the workspace, and pin it to a **stable release tag** rather
than tracking the development branch:

```bash
cd /srv/zephyr
wget <sdk-release-url>
tar xf zephyr-sdk-<ver>_linux-x86_64.tar.xz
cd zephyr-sdk-<ver> && ./setup.sh -t all -h -c && cd ..

python3 -m venv venv && source venv/bin/activate
pip install west
west init zephyrproject
cd zephyrproject/zephyr && git checkout v<release>   # pin to a release, not main
cd .. && west update
pip install -r zephyr/scripts/requirements.txt
```

Build the wheelhouse with `pip wheel` (not `pip download`) so every package is a
prebuilt `.whl` and nothing has to compile on a student machine:

```bash
pip wheel -r zephyr/scripts/requirements.txt -w wheelhouse west
```

Package for distribution. Use multi-threaded compression — single-threaded `xz`
on ~20 GB is painfully slow:

```bash
XZ_OPT="-T0 -6" tar cJf dist/zephyr-sdk-<ver>_linux-x86_64.tar.xz zephyr-sdk-<ver>
XZ_OPT="-T0 -6" tar cJf dist/zephyrproject.tar.xz zephyrproject
```

Serve it as a systemd unit so it survives reboots:

```ini
[Unit]
Description=Zephyr Lab Distribution HTTP Server
After=network.target

[Service]
Type=simple
User=labuser
WorkingDirectory=/srv/zephyr/dist
ExecStart=/usr/bin/python3 -m http.server 8080 --bind 0.0.0.0
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

---

## Student Install

One command per laptop:

```bash
wget -O student-setup.sh http://<server>:8080/student-setup.sh
chmod +x student-setup.sh && ./student-setup.sh
```

The script installs build dependencies, pulls everything from the LAN server, sets
up a virtual environment from the local wheelhouse, registers the USB flashing
rules, and adds the user to the serial-device groups.

Verify:

```bash
export ZEPHYR_SDK_INSTALL_DIR=~/zephyr-work/zephyr-sdk-<ver>
source ~/zephyr-work/venv/bin/activate
source ~/zephyr-work/zephyrproject/zephyr/zephyr-env.sh
cd ~/zephyr-work/zephyrproject
west build -p always -b qemu_x86 zephyr/samples/hello_world
west build -t run      # prints: Hello World! qemu_x86
```

---

## Four Problems Worth Knowing About

These cost real debugging time and are not obvious from the documentation.

### 1. The SDK version is coupled to the Zephyr version

`zephyr/SDK_VERSION` in the checkout dictates which SDK is required. Pairing a
current Zephyr tree with an older SDK fails at CMake configure time with a
`find_package` version error that does not name the real cause:

```
Could not find a configuration file for package "Zephyr-sdk" that is
compatible with requested version "1.0".
```

Read `SDK_VERSION` **first**, then download the matching SDK. Note also that the
SDK 1.0 release renamed its assets (the Linux archive gained a `_gnu` suffix), so
URL patterns from older guides 404.

### 2. Pin to a release tag, not `main`

`west init` with no arguments tracks the development branch. For teaching, pin to
a release tag — `git checkout v<release>` inside the manifest repo, then re-run
`west update` to resync every module to that release's pinned revisions. A moving
target means students hit different breakage on different days.

### 3. Compiled Python wheels are locked to one Python version

This was the subtle one. Prebuilt wheels with C extensions are tagged for a
specific interpreter ABI (`cp312` = CPython 3.12). A laptop running a newer
distro with Python 3.14 silently rejects every one of them — and `pip` reports it
as a **misleading error about an unrelated package**:

```
ERROR: Could not find a version that satisfies the requirement PyYAML>=5.1
ERROR: No matching distribution found for PyYAML>=5.1
```

PyYAML was present in the wheelhouse the whole time. It was simply built for a
different Python. Pure-Python packages installed fine, which made it look like a
network or index problem rather than an ABI mismatch.

**Fix:** stop depending on whatever Python the laptop happens to ship. Bundle a
standalone interpreter (from `astral-sh/python-build-standalone`), ship it
alongside the SDK, and build the virtual environment from *that*:

```bash
wget http://<server>:8080/python3.12-standalone_linux-x86_64.tar.xz
tar xf python3.12-standalone_linux-x86_64.tar.xz
./python3.12-standalone/bin/python3 -m venv ~/zephyr-work/venv
```

The wheelhouse now always matches, regardless of the host distribution.

### 4. Do not assume a uniform fleet

Personal laptops vary in distribution *and* Python version. Two consequences:

- The installer detects `apt` vs `dnf` rather than assuming Debian.
- Group membership is checked before use — `plugdev` exists on Debian/Ubuntu but
  not on Fedora, and blindly calling `usermod -aG plugdev` aborts the script.

```bash
for g in dialout plugdev; do
    getent group "$g" >/dev/null 2>&1 && ADDGROUPS="$ADDGROUPS,$g"
done
[ -n "$ADDGROUPS" ] && sudo usermod -aG "${ADDGROUPS#,}" "$USER"
```

Everything else — the standalone Python, the SDK, and `manylinux` wheels — is
distribution-agnostic, so only the package-install step needs branching.

---

## Two Smaller Notes

**`pip` refuses plain-HTTP package sources.** A LAN wheelhouse served over HTTP is
silently ignored unless the host is explicitly trusted:

```bash
pip install --trusted-host <server> --no-index --find-links http://<server>:8080/wheelhouse west
```

**Tracking who has installed.** Python's `http.server` logs the full request line,
so appending an identifier to the download URL makes each laptop visible in the
server log. When `wget` is given a URL with a query string it must also be told
the output filename, or the query ends up in the saved filename and breaks `tar`:

```bash
ID="?host=$(hostname)&user=$USER"
wget -O zephyrproject.tar.xz "http://<server>:8080/zephyrproject.tar.xz$ID"
```

---

## Result

| | Before | After |
| :--- | :--- | :--- |
| Source | Internet, per laptop | LAN, from one server |
| Data pulled | ~20 GB each | ~6 GB each |
| Time | Hours, often failing | A few minutes |
| Versions | Drift between students | Identical for everyone |
| Internet needed | Throughout | Only for base OS packages |

Validated end-to-end on both Ubuntu and Fedora: sample application built and run
under QEMU from a completely LAN-sourced toolchain.

The one remaining internet dependency is the initial `apt`/`dnf` install of build
tools. A caching proxy such as `apt-cacher-ng` on the same server would close
that gap.
