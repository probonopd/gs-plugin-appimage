# gs-plugin-appimage

Plugin for [GNOME Software](https://wiki.gnome.org/Apps/Software) to manage applications in the [AppImage](https://appimage.org/) format ([specification](https://github.com/AppImage/AppImageSpec)). 

Note that the use of GNOME Software is purely optional, as AppImages do not need to be installed in order to be used.
Nevertheless, this may add comfort to some users who are already using GNOME Software to manager other software.

## Objectives

Once this plugin is complete, it should:

* Show AppImages in the known locations as "installed".
  At least those which have already been integrated into the system by appimaged or other tools
  and have desktop files integrated into the system like
  `/home/me/.local/share/applications/appimagekit_96c121fd6971e6073de75aa0cdad64dd-AppImageUpdate.desktop`
* When an AppImage is opened with GS (e.g., using a right-click "open with...", 
  then show the application's details using the information from inside the AppImage (only)
  and offer to "install", which in our case means 
  move to `$HOME/Applications` and integrate desktop files etc. into the system
  if it hasn't already been integrated into the system (in which case we show the information
  regarding this AppImage version of the application (only) and offer to run or "uninstall", which in our case
  means unintegrate with the system and delete the AppImage file
* Show available AppImages from https://appimage.github.io/pages/appstream.xml as installable
* Be able to check for and trigger updates using libappimageupdate.
  Ideally only if a user triggers a version check explicitly. Don't "check all the AppImages on the whole system"
  because checking versions results in http requests for each application, and we may run into quota quickly
  and because due to the nature of AppImage (allowing for multiple versions of the same app in parallel)
  users may want to keep old versions; hence we should not make them belive they _have to_ update them.
  Only offer updates if the user asks for them for this app at a certain point in time.
  "Updating" for AppImage means, "Download the latest version _in addition to_ the existing version"
  and keep around both (for rollback reasons)
  
## Installation

This plugin is under development. See [BUILDING.md](BUILDING.md) for instructions on how to build it.

## Acknowledgments

This plugin would not be possible without the help of
* [hughsie](https://github.com/hughsie) for his work on GNOME Software and for his detailed explanations on IRC
* [TheAssassin](https://github.com/TheAssassin) for his work on `libappimage` and `libappimageupdate`
* [azubieta](https://github.com/azubieta) for his work on `libappimage`
* [Conan-Kudo](https://github.com/Conan-Kudo) for his work on distribution packaging
