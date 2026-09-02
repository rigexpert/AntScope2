The AntScope2 software is designed to support various models of RigExpert analyzers under various operating systems.

Windows: Qt6, Qt Creator 2 and higher

Linux: Qt6, qmake. Build with:

```
qmake6 CONFIG+=release AntScope.pro
make
```

Depends on `qt6-base`, `qt6-serialport`, `qt6-connectivity`, and `libusb` (used via the vendored HIDAPI backend for HID-connected analyzers). Packaged for Arch Linux/AUR as
[`antscope2`](https://aur.archlinux.org/packages/antscope2) (pinned to a known-good commit, patched)
and [`antscope2-git`](https://aur.archlinux.org/packages/antscope2-git) (tracks a patched branch's tip - see either package's `PKGBUILD` for the udev rule, desktop file, and data-path packaging details this repo's own build doesn't set up on its own).

Mac OS: to do
