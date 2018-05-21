/* GNOME Software AppImage plugin
 * Licensing to be determined (MIT like AppImageKit or GPLv2 like GNOME Software)
 */

#include <gnome-software.h>
#include <appimage.h> // From https://github.com/AppImage/AppImageKit
#include <sys/stat.h>

/*
Once this plugin is complete, it should:
* Show AppImages in the known locations as "installed".
  At least those which have already been integrated into the system by appimaged or other tools
  and have desktop files integrated into the system like
  /home/me/.local/share/applications/appimagekit_96c121fd6971e6073de75aa0cdad64dd-AppImageUpdate.desktop
* When an AppImage is opened with GS (e.g., using a right-click "open with...", 
  then show the application's details using the information from inside the AppImage (only)
  and offer to "install", which in our case means 
  move to $HOME/Applications and integrate desktop files etc. into the system
  if it hasn't already been integrated into the system (in which case we show the information
  regarding this AppImage version of the application (only) and offer to run or "uninstall", which in our case
  means unintegrate with the system and delete the AppImage file
* Show available AppImages from feed.xml as installable
* Be able to check for and trigger updates using libappimageupdate.
  Ideally only if a user triggers a version check explicitly. Don't "check all the AppImages on the whole system"
  because checking versions results in http requests for each application, and we may run into quota quickly
  and because due to the nature of AppImage (allowing for multiple versions of the same app in parallel)
  users may want to keep old versions; hence we should not make them belive they _have to_ update them.
  Only offer updates if the user asks for them for this app at a certain point in time.
  "Updating" for AppImage means, "Download the latest version _in addition to_ the existing version"
  and keep around both (for rollback reasons)
  
Install libappimage-dev (e.g., from the Nitrux repo or possibly by compiling from git), then run:
sudo cp /usr/lib/x86_64-linux-gnu/pkgconfig/flatpak.pc /usr/lib/x86_64-linux-gnu/pkgconfig/appimage.pc
sudo sed -i-e 's|flatpak|appimage|g' /usr/lib/x86_64-linux-gnu/pkgconfig/appimage.pc # TODO: Ship libappimage with a .pc file

# TODO: Ship libappimage with a .a file so that we can link it statically
# TODO: Link libappimage statically so that this plugin doesn't have a runtime dependency on it;
# we don't assume distributions have it installed

# Compile and run on Ubuntu 18.04 with:

sudo apt update
sudo apt -y install gnome-software-dev git
sudo apt-get -y build-dep gnome-software
# NOTE: Probably we can't compile against git if I want the plugin work on Ubuntu 18.04.
# At least that's how I read DI_KNOW_THE_GNOME_SOFTWARE_API_IS_SUBJECT_TO_CHANGE.
# This makes it hard to change stuff in this plugin and still provide it for LTS versions of distros out there
# without making an AppImage of GNOME Software and this plugin to run on the oldest LTS distros out there.
# Systematically, I want all my stuff to run on the oldest LTS releases (so that "all users" out there can use it). 
# Which is why normally I only develop on the oldest still-supported LTS releases
wget http://archive.ubuntu.com/ubuntu/pool/main/g/gnome-software/gnome-software_3.28.1.orig.tar.xz
tar xf gnome-software_*.orig.tar.xz
# git clone https://github.com/GNOME/gnome-software
cd gnome-software-*

meson --prefix $PWD/install build/
ninja -C build/ all install

cd contrib/

gcc -shared -o libgs_plugin_example.so gs-plugin-example.c -fPIC \
`pkg-config --libs --cflags gnome-software appimage` \
-DI_KNOW_THE_GNOME_SOFTWARE_API_IS_SUBJECT_TO_CHANGE &&
sudo cp libgs_plugin_appimage.so `pkg-config gnome-software --variable=plugindir`

cp libgs_plugin_appimage.so ../install/lib/x86_64-linux-gnu/gs-plugins-11/
XDG_DATA_DIRS=install/share:$XDG_DATA_DIRS ../install/bin/gnome-software --verbose  2>&1

*/


void
gs_plugin_initialize (GsPlugin *plugin)
{
    g_debug("AppImage gs_plugin_initialize");
    gs_plugin_add_rule (plugin, GS_PLUGIN_RULE_RUN_BEFORE, "appstream");


    /* prioritize over packages */
    gs_plugin_add_rule (plugin, GS_PLUGIN_RULE_BETTER_THAN, "packagekit");
}

/* Claim AppImages that should be handled by this plugin.
hughsie: gnome-software doesn't understand what a desktop file is but appstrem-glib does, so we need
to let appstream-glib discover them and then adopt them in the plugin
*/
void
gs_plugin_adopt_app (GsPlugin *plugin, GsApp *app)
{
    if (gs_app_get_bundle_kind (app) == AS_BUNDLE_KIND_APPIMAGE)
        gs_app_set_management_plugin (app, gs_plugin_get_name (plugin));
}

/* Launch AppImages
 * QUESTION: Do I even need to do something special? Why doesn't it just launch the desktop file?
 * hughsie: don't implement gs_plugin_launch and see if it works (it should)
 */
gboolean
gs_plugin_launch (GsPlugin *plugin,
                  GsApp *app,
                  GCancellable *cancellable,
                  GError **error)
{

    if (gs_app_get_bundle_kind (app) == AS_BUNDLE_KIND_APPIMAGE) {
        g_debug("AppImage shall be launched");

        g_autofree gchar *md5 = appimage_get_md5(g_file_get_path (gs_app_get_local_file (app)));
        g_autofree gchar *partial_path = g_strdup_printf("applications/appimagekit_%s-%s", md5, "*");
        g_autofree const gchar *data_home = g_get_user_data_dir();
        g_autofree gchar *appimage_integrated_desktop_file_path = g_build_filename(data_home, partial_path, NULL);


        if (g_file_test (appimage_integrated_desktop_file_path, G_FILE_TEST_EXISTS))
        {
            g_debug("AppImage is already integrated at %s", appimage_integrated_desktop_file_path);
        } else {
            g_debug("AppImage is not integrated yet");
        }
        return TRUE;
    } else {
        g_debug("Not an AppImage");
        return FALSE; // FIXME: Proper error handling using GError
    }

}



/*
Handle AppImages "opened" with GNOME Software
This works; it does show the product detail page when launched like this:
XDG_DATA_DIRS=install/share:$XDG_DATA_DIRS ../install/bin/gnome-software --verbose --local-filename=/isodevice/Applications/XChat-2.8.8-x86_64.AppImage 2>&1 | grep AppImage

*/

gboolean
gs_plugin_file_to_app (GsPlugin *plugin,
                       GsAppList *list,
                       GFile *file,
                       GCancellable *cancellable,
                       GError **error)
{
    g_debug("AppImage gs_plugin_url_to_app");
    g_debug("file: %s", g_file_get_path(file));


    int appimage_type = appimage_get_type(g_file_get_path(file), TRUE);
    g_debug("AppImage type: %i", appimage_type);

    /* Error if we cannot determine the type of the AppImage */
    if(appimage_type <0) {
        return FALSE;
    }

    /* Extract the desktop file from the AppImage */
    char** files = appimage_list_files(g_file_get_path(file));
    g_autofree gchar *desktop_file = NULL;
    gchar *extracted_desktop_file = "/tmp/gs-plugin-appimage.desktop";
    int i = 0;
    for (; files[i] != NULL ; i++) {
        // g_debug("AppImage file: %s", files[i]);
        if (g_str_has_suffix (files[i],".desktop")) {
            desktop_file = files[i];
            g_debug("AppImage desktop file: %s", desktop_file);
            break;
        }
    }
    /* Exit if we cannot find the desktop file */
    if(desktop_file == NULL) {
        g_debug("AppImage desktop file not found");
        appimage_string_list_free(files);
        return FALSE;
    }
    /* Extract desktop file to temporary location */
    appimage_extract_file_following_symlinks(g_file_get_path(file), desktop_file,
            extracted_desktop_file);

    /* Load contents of desktop file */
    gboolean success = FALSE;
    GKeyFile* key_file_structure =  g_key_file_new();

    /* QUESTION: Do we need to load desktop files like this?
     * hughsie: appstream-glib can do that work
     */
    g_debug("Loading AppImage desktop file from %s", extracted_desktop_file);
    success = g_key_file_load_from_file(key_file_structure, extracted_desktop_file, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL);
    if (!success) {
        g_debug("AppImage desktop file could not be loaded");
        appimage_string_list_free(files);
        return FALSE;
    } else {
        g_debug("Loaded AppImage desktop file");  // QUESTION: Can we directly gs_app_new() from this? hughsie: use appstream-glib
    }

    /* TODO: AppStream
    Save an AppStream-format XML file in either /usr/share/app-info/xmls/, /var/cache/app-info/xmls/ or ~/.local/share/app-info/xmls/. GNOME Software will immediately notice any new files, or changes to existing files as it has set up the various inotify watches.
    
    QUESTION: Do we need to copy XML there for AppImages that are not integrated into the system? 
    Can we load in data from an xml file which we delete immediately afterwards?
    Because we don't want to litter the system with XML files.
    hughsie: Don't need to copy; just use the helpers in gs-appstream.c; 
    e.g. the Flatpak plugin loads the AppStream xml from a nonstandard path

    QUESTION: If we need to ensure that our AppStream information is not mixed with AppStream 
    information from other locations or from version to version of an application, is it OK to place it in
    /home/me/.local/share/app-info/xmls/appimagekit_98080cfc981c9098c6a5e41794add640-deepin-screenshot.appdata.xml
    hughsie: the filename is almost unimportant, it's the <id> that has to match

    QUESTION: If we have appimagekit_98080cfc981c9098c6a5e41794add640-deepin-screenshot.desktop in one of 
    the well-known locations for desktop files, will it automatically be associated with the AppStream metadata from above?
    hughsie: the filename is almost unimportant, it's the <id> that has to match
    
    QUESTION: Is the AppStream ID the same as the gs ID?
    hughsie: In most cases, yes. The appstream file can also have a <launchable> tab pointing to the desktop file.
    If there's no launchable then we use a heuristic to try and create one
    
    QUESTION: Is there a screenshot where i could see the effect of using different IDs for gs_app_new() vs. gs_app_set_branch()?
    hughsie: ...
    
    I see:
    08:20:32:0181 Gs  searching appstream for user / * / * / desktop / appimagekit_98080cfc981c9098c6a5e41794add640-deepin-screenshot.desktop / *

    QUESTION: Should we have appimaged (also) integrate AppStream files to ~/.local/share/app-info/xmls/?
    hughsie: I think using reverse DNS style IDs everywhere is a very good idea; I also think it's too early to optimise anything
    */

    g_autofree gchar *md5 = appimage_get_md5(g_file_get_path (file));

    /* The following does not work; the page never loads (just shows spinner)
        g_autofree gchar *appstream_file;
        // QUESTION: desktop_file.replace("desktop", "") is missing in Glib apparently - why?
        // hughsie: There is as_string_replace if you have a GString
        g_autofree gchar *appstream_filename = g_strconcat (g_path_get_basename(desktop_file), "appdata.xml", NULL);
        g_autofree gchar *extracted_appstream_file = g_strconcat(g_get_user_data_dir(), "/app-info/xmls/", "appimagekit_", md5, "-", appstream_filename, NULL);

        g_debug("AppImage AppStream filename: %s", appstream_filename);
        g_debug("AppImage AppStream file to be extracted to: %s", extracted_appstream_file);
    */

    g_autofree gchar *fn = NULL;
    g_autoptr(GsApp) app = NULL;
    g_autoptr(AsIcon) icon = NULL;
  
    app = gs_app_new ("NULL"); // NOTE: We set the ID down below, including the md5 from appimage_get_md5
    gs_app_set_scope (app, AS_APP_SCOPE_USER);
    gs_app_set_management_plugin (app, "appimage");
    gs_app_set_kind (app, AS_APP_KIND_DESKTOP);
    gs_app_set_bundle_kind (app, AS_BUNDLE_KIND_APPIMAGE);
    gs_app_set_local_file (app, file);
    gs_app_add_quirk (app, AS_APP_QUIRK_PROVENANCE); // QUESTION: How to mark "3rd party"?
    // gs_app_set_state (app, AS_APP_STATE_AVAILABLE_LOCAL);
    // QUESTION: Is there an easier way to make an app from an exsiting desktop file, without reading each value manually using g_key_file_get_value?
    gs_app_set_name (app, GS_APP_QUALITY_NORMAL, g_key_file_get_value(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, NULL));
    gs_app_set_summary (app, GS_APP_QUALITY_NORMAL, g_key_file_get_value(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_COMMENT, NULL));
    gs_app_set_description (app, GS_APP_QUALITY_NORMAL, g_key_file_get_value(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_COMMENT, NULL));

    /* these are all optional, but make details page look better */
    gs_app_set_version (app, g_key_file_get_value(key_file_structure, G_KEY_FILE_DESKTOP_GROUP, "X-AppImage-Version", NULL));

    /* Get the size of the AppImage on disk */
    struct stat st;
    if (lstat(g_file_get_path(file), &st) == -1) {
        g_debug("Could not determine AppImage file size");
        return FALSE;
    }
    gs_app_set_size_installed (app, st.st_size);
    gs_app_set_size_download (app, st.st_size);

    /* Check if this AppImage is already integrated in the system in which case we treat it as "installed */
    
    g_autofree gchar *partial_path = g_strdup_printf("applications/appimagekit_%s-%s", md5, g_path_get_basename(desktop_file));
    g_autofree gchar *appimage_integrated_desktop_file_path = g_build_filename(g_get_user_data_dir(), partial_path, NULL);
    g_autofree const gchar *appimage_id = g_path_get_basename(partial_path);
    gs_app_set_id (app, appimage_id); // This makes it use the desktop file and icon already integrated into the system

    if (g_file_test (appimage_integrated_desktop_file_path, G_FILE_TEST_EXISTS))
    {
        g_debug("AppImage is already integrated at %s", appimage_integrated_desktop_file_path);
        gs_app_set_state (app, AS_APP_STATE_INSTALLED);
    } else {
        g_debug("AppImage is not integrated yet");
        gs_app_set_state (app, AS_APP_STATE_AVAILABLE_LOCAL);
    }

    g_debug("AppImage gs_app_is_installed: %i", gs_app_is_installed (app));
    gs_app_set_origin (app, "AppImage");
    
    // QUESTION: "Install" doesn't really cut it. For AppImages, we would want 
    // "Move to /Applications", "Move to $HOME/Applications", etc. Is there a way to cusomize this?
  
    /* return new app */
    gs_app_list_add (list, app);

    g_key_file_free(key_file_structure);
    // appimage_string_list_free(files); // FIXME: This results in a segfault!

    return TRUE;
}



/* 

This works; it DOES show up in the installed list!

QUESTION: How can I achieve a similar thing for all the desktop files of already integrated AppImages that show up like this:

08:25:05:0456 As  adding existing file: /home/me/.local/share/applications/appimagekit_96c121fd6971e6073de75aa0cdad64dd-AppImageUpdate.desktop
08:25:05:0456 As  adding existing file: /home/me/.local/share/applications/appimagekit_d6d51dc8061f0166e11ecd040af70e8e-AppImageUpdate.desktop

08:26:09:0696 GsPluginPackageKit ignoring /usr/share/applications/appimagekit_96c121fd6971e6073de75aa0cdad64dd-AppImageUpdate.desktop as does not exist
08:26:09:0696 GsPluginPackageKit ignoring /usr/share/applications/appimagekit_d6d51dc8061f0166e11ecd040af70e8e-AppImageUpdate.desktop as does not exist

08:26:13:0491 Gs  app invalid as state unknown user / * / * / desktop / appimagekit_96c121fd6971e6073de75aa0cdad64dd-AppImageUpdate.desktop / *
08:26:13:0491 Gs  app invalid as state unknown user / * / * / desktop / appimagekit_d6d51dc8061f0166e11ecd040af70e8e-AppImageUpdate.desktop / *

QUESTION: What do I have to do in order to make these show up as installed? 
QUESTION: Why does it say "ignoring... as does not exist"? 
QUESTION: What does "app invalid as state unknown user" mean? Probably I have to set them to AS_APP_STATE_INSTALLED, but how?

*/

gboolean
gs_plugin_add_installed (GsPlugin *plugin,
                         GsAppList *list,
                         GCancellable *cancellable,
                         GError **error)
{
    g_debug("AppImage gs_plugin_add_installed");

    g_autofree gchar *fn = NULL;
    g_autoptr(GsApp) app = NULL;
    g_autoptr(AsIcon) icon = NULL;

    // One entry will show up in the list of installed files for each DIFFERENTLY NAMED desktop file
    // "System appstream" will be searched for metadata with matching desktop file names
    app = gs_app_new ("aaatestapp.desktop"); // "Launch" button only available if ID is passed in
    gs_app_set_scope (app, AS_APP_SCOPE_USER); // TODO: Distinguish system-wide AppImages. Those which are read-only for the current user?
    gs_app_set_management_plugin (app, "appimage");
    gs_app_set_kind (app, AS_APP_KIND_DESKTOP);
    gs_app_set_bundle_kind (app, AS_BUNDLE_KIND_APPIMAGE);
    gs_app_set_state (app, AS_APP_STATE_INSTALLED);
    gs_app_set_name (app, GS_APP_QUALITY_NORMAL, "AAATestApp 1.2.3a"); // QUESTION: How to handle multiple versions properly?
    gs_app_set_summary (app, GS_APP_QUALITY_NORMAL, "A teaching application");
    gs_app_set_description (app, GS_APP_QUALITY_NORMAL,
                            "AAATestApp is the name of an application.\n\n"
                            "It can be used to demo some of our features");

    /* create a stock icon which will be loaded by the 'icons' plugin
     * NOTE: Without doing this, it will NOT show up in the list of installed files.
     * QUESTION: Is this intentional?
     */
    icon = as_icon_new ();
    as_icon_set_kind (icon, AS_ICON_KIND_STOCK);
    as_icon_set_name (icon, "application-x-executable");
    gs_app_add_icon (app, icon);

    /* return new app */
    gs_app_list_add (list, app);

    return TRUE;
}

