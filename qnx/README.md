# Compile the port for QNX

**NOTE**: QNX ports are only supported from a Linux host operating system

- Setup your QNX SDP environment

```bash
# Install scons
sudo apt-get install scons

# Clone library
git clone -b qnx-sdp71-main https://github.com/qnx/computelibrary.git && cd computelibrary

# Build
make -C qnx/build install
```
