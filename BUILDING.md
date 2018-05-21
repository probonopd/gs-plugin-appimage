# Building

## Ubuntu 18.04

### AppImage plugin

```
sudo apt update
sudo apt -y install gnome-software-dev git

git clone https://github.com/probonopd/gs-plugin-appimage
cd gs-plugin-appimage/

# Build and install libappimage and libappimageupdate according
# to the instructions on
# https://github.com/AppImage/AppImageUpdate/blob/rewrite/BUILDING.md#building-the-libraries

gcc -shared -o libgs_plugin_appimage.so gs-plugin-appimage.c -fPIC \
`pkg-config --libs --cflags gnome-software` -I/usr/include/appimage \
-lappimage -DI_KNOW_THE_GNOME_SOFTWARE_API_IS_SUBJECT_TO_CHANGE

# Install system-wide
sudo cp libgs_plugin_appimage.so `pkg-config gnome-software --variable=plugindir`
#  Run system GNOME Software
gnome-software --verbose
```

### GNOME Software

We can also build a private build of GNOME Software, although this may not be necessary in many cases.

```
sudo apt-get -y build-dep gnome-software
wget http://archive.ubuntu.com/ubuntu/pool/main/g/gnome-software/gnome-software_3.28.1.orig.tar.xz
tar xf gnome-software_*.orig.tar.xz
# or
git clone https://github.com/GNOME/gnome-software
cd gnome-software-*
meson --prefix $PWD/install build/
ninja -C build/ all install

# In this case we can install the plugin into our private build of GNOME Software
cp libgs_plugin_appimage.so ../install/lib/x86_64-linux-gnu/gs-plugins-11/
#  Run private GNOME Software
XDG_DATA_DIRS=install/share:$XDG_DATA_DIRS ../install/bin/gnome-software --verbose  2>&1
```
