For new methods/callbacks, see the [API](./docs/api.md) page.

## 1.7.26.1.30
- Add the following new methods:
```
utils.ConvertToAscii
utils.ListFonts
utils.Now
utils.ReadUTF8
```

## 1.7.26.1.26
- Fix regression in yesterday's release which broke caching in samples that fetch online info.
- Fix missing auto-complete entries for recently added `utils` methods.

## 1.7.26.1.25
- Add the following new methods:
```
utils.CopyFile
utils.CopyFolder
utils.CreateFolder
utils.GetLastModified
utils.RemovePath
utils.RenamePath
utils.ReplaceIllegalChars
```
- Transparency is no longer optional in `Columns UI`. It's automatically enabled for toolbars and unavailable everywhere else.

## 1.7.26.1.20
- Add `on_console_refresh` callback.
- Add `console.GetLines` and `console.ClearBacklog` methods.
- Users of `Last.fm Bio` and `Last.fm Similar Artists` will need to set their API key again. Now it's stored inside your `profile\js_data\lastfm.ini` and
remains permanently instead of being lost when panels are reset. Also, updating a key in one panel will refresh all panels that use it.

## 1.7.26.1.16
- `FbTitleFormat` `EvalWithMetadb` and `FbMetadbHandle` `GetFileInfo` now accept an optional `want_full_info`
argument. This enables full retrieval of tags that have been blocked with [LargeFieldsConfig-v2](https://www.foobar2000.org/LargeFieldsConfig-v2)
in the latest `foobar2000` `2.26` previews.

## 1.7.26.1.14
- Include `CaTRoX Playlist` as a built in sample. Note that:
  * It is not DPI aware.
  * The colours are fixed.
  * It can be used in `Default UI` or `Columns UI`.
  * I am not taking feature requests!

## 1.7.26.1.12
- Fix regression in `GdiBitmap` `GetColourSchemeJSON` causing unexpected script errors.
- Increase tooltip max length to 1024 characters.

## 1.7.26.1.11
- Large internal code refactor removing dead code.
- Limit tooltip length to 256 characters.
- Pseudo transparency options are now suppressed in `Default UI`. It's only useful for `Columns UI` toolbars.
- Fix bug so invalid `FbPlaybackQueueItem` indexes are now `-1`.
- Fix bug in the `Properties` sample where `Unix` line breaks in tag values were ignored leading to display errors.

## 1.7.26.1.2
- Hopefully fix long standing crash on exit bug from original component.

## 1.7.25.12.30
- Fix stupid(!) crash in yesterday's release.

## 1.7.25.12.29
- Various bug fixes.

## 1.7.25.12.27
- Remove `Spider Monkey` built in `Playback Statistics`. Do not update if you make use of this.
- Rework advanced preferences removing an option that caused a crash if enabled. This bug was present in the last official `1.6.1` release.
- The default max heap size has been doubled to 2GB. You may need to right click and reset the advanced preference node to see this.

## 1.7.25.12.22
- Fix various properties dialog bugs.

## 1.7.25.12.21
- Rework font handling so built in methods never return `null`. If using an unsupported raster
font in your UI preferences, the component will fallback to `Segoe UI` automatically.

## 1.7.25.12.20
- The main configure dialog now supports dark mode.
- For the above change to work, the properties tab has been removed and the properties are now a separate dialog. This dialog does not support dark mode.
- The appearance tab has been removed and you can no longer set an edge style.
- Pseudo transparency can be toggled directly from the default context menu.
- Maybe fix crash for WINE users when deleting properties.

## 1.7.25.12.18
- Update `Spider Monkey` engine to `102.12`.

## 1.7.25.12.15
- Fix crashing when using certain library iteration methods in WINE.

## 1.7.25.12.11
- The Scintilla library used for code highlighting has been updated to the latest version. This means:
  * any previous custom colour scheme will be lost and previously generated .cfg files are no longer compatible.
  * custom colour schemes are still supported but the default is to update itself automatically whenever dark mode is toggled.
- The main editor, find/replace and goto dialogs have been updated to support dark mode.

## 1.7.25.12.10
- Show Spider Monkey version in about dialog (91.10).
- Fix editor window `Apply` button so it can be pressed repeatedly even when text changes
have not been made.
- Build should now be compatible with Windows 7 but I'm unable to test that myself.

## 1.7.25.12.8
- First 64bit build.

## 1.6.2.25.12.15
- Fix crashing when using certain library iteration methods in WINE.
- Last 32bit release. It remains [available to download](https://github.com/marc2k3/fb2k-graveyard) but is unsupported.

## 1.6.2.25.11.15
- Tooltips are now dark mode aware.
- Update `utils.InputBox` to support dark mode.
- Update samples to use `window.ShowConfigureV2()`.

## 1.6.2.25.11.09
- Fix various main menu and context menu execution bugs.
- Fix `fb.DoDragDrop` regression where album art was not automatically displayed.
- The `Thumbs` sample now reports last.fm server errors in the console.
- Various other bug fixes.

## 1.6.2.25.10.29
- Fix long standing `plman.SetPlaylistLockedActions` bug related to `ExecuteDefaultAction`.

https://github.com/TheQwertiest/foo_spider_monkey_panel/issues/198

This may be considered a breaking change considering it was unusable in the previous state and
workarounds must have been added to use it. Those workarounds will now be broken by the
newly correct behaviour!

## 1.6.2.25.10.16
- Update panel error text replacing `Aw, crashed :(` with `Spider Monkey Panel JavaScript error`.
- Add `fb.RequestBacklog(time)` for `AudioChunk` users. See this thread:
https://hydrogenaudio.org/index.php/topic,125037.0.html

## 1.6.2.25.09.26
- Fix bug when using `FbMetadbHandle` methods to write component playback
statistics values.

## 1.6.2.25.09.13
- Add `FbMetadbHandleList` `OptimiseFileLayout`. It takes a `boolean` `minimise`
argument. This provides the same functionality as the native context menu items
under `Utilities` except there are no prompts.
- Suppress all library callbacks on startup until it is fully initialised. All
library items will be passed to `on_library_items_added` which existing scripts
already expect.

## 1.6.2.25.09.10
- Fix bug where the `on_key_up` callback was not triggered for certain keys.
- Fix missing auto-complete entry for `on_download_file_done`.

## 1.6.2.25.09.06
- Various internal improvements.
- Minor sample fixes.

## 1.6.2.25.07.31
Update `utils.HTTPRequestAsync` to use a thread pool which limits concurrent
requests.

## 1.6.2.25.07.22
- The `type` values supplied to the following methods are now validated and will
throw errors if invalid.

```js
window.GetColourCUI
window.GetColourDUI
window.GetFontCUI
window.GetFontDUI
```
- Add [build](https://github.com/marc2k3/spider-monkey-panel-x64/tree/main/docs/BUILDING.md) instructions.

## 1.6.2.25.06.21
- Add utils.HTTPRequestAsync. See [web requests](https://github.com/marc2k3/spider-monkey-panel-x64/tree/main/docs/web-requests.md).

## 1.6.2.25.06.17
- Various bug fixes.

## 1.6.2.25.06.09
- Fix crash with original `plman.AddLocations` array parser that was recently
reused in the new `fb.AddLocationsAsync` method. Now a script error is thrown. Note
that array elements are not validated and each one is treated as a string.
- `plman.AddLocations` now throws a script error if a playlist lock is in
place that prevents adding of new items.

## 1.6.2.25.05.27
- Fix includes and package imports for paths containing non-ascii chars.
- Lots of internal refactoring around image, album art and text file
handling. Please report any issues if you find them.

## 1.6.2.25.05.23
- Fix regression affecting JSON property exports.

- Add `fb.CustomVolume` property. It can be used for displaying the volume from `UPnP` devices. It
will return a value of `-1` when using a normal device and that also indicates that `fb.Volume` is
writable. When a custom volume control is active, you can not use `fb.Volume` and must use
`fb.VolumeUp()` / `fb.VolumeDown()` / `fb.VolumeMute()`.

- Add `window.DPI`. This value never changes while `foobar2000` is running. If you change `DPI`
settings, you must restart the application.

- Remove all `Smooth` samples and `JSPlaylist-mod`. They are in an appalling state
and I refuse to distribute or be responsible for them.

## 1.6.2.25.05.20
- Update to `Spider Monkey` `91.10.0esr`.
- Fix `utils.Glob` so it works with `;` in folder names.
- Add `utils.DownloadFileAsync(url, path)` and `on_download_file_done` callback. 

The parent folder for `path` must already exist.

Example:

```js
// no return value, the path you supply is the first argument in the
// on_download_file_done callback
utils.DownloadFileAsync(
    "https://lastfm.freetls.fastly.net/i/u/770x0/0be145cbf80930684d41ad524fe53768.jpg",
    "z:\\blah.jpg"
);

// success is a boolean value
// error_text is always empty if success was true
function on_download_file_done(path, success, error_text) {
	console.log("on_download_file_done", path, success, error_text);
}
```

- Update included `Thumbs` sample to use the new method above.
- Smooth wallpaper fixes.
- Various other sample fixes.

## 1.6.2.25.05.19
- Add `window.IsDark` boolean property. The `on_colours_changed` callback has always
responded to dark mode being toggled.
- Ensure `fb.IsMainMenuCommandChecked` is always reliable.

## 1.6.2.25.05.14
- Add `fb.AddLocationsAsync(locations)` and `on_locations_added` callback.

`locations` must be an array of strings and it can contain file paths, playlists or urls.

Example:

```js
function on_mouse_lbtn_dblclk() {
	var files = ["z:\\1.mp3", "z:\\2.flac"];
	var task_id = fb.AddLocationsAsync(files);
	console.log("got task_id", task_id);
}

function on_locations_added(task_id, handle_list) {
	console.log("callback task_id", task_id);
	console.log(handle_list.Count);
}
```

## 1.6.2.25.05.13
- Add `FbMetadbHandleList` `SaveAs`. Saves using native `.fpl`
format so you should use that as the file extension. The
parent folder must already exist. No return value and if
it fails, it happens silently.

- Add `fb.ShowPictureViewer(image_path)`. This uses the image
viewer built in to `foobar2000`. No return value.

## 1.6.2.25.05.10
- `foobar2000` `2.25` preview has changed the behaviour of `FbMetadbHandle` `RawPath` if you
have a portable install and music files on the same drive. Any code that checks `startsWith("file://")`
will fail because the `RawPath` now starts with `file-relative://`. This release restores the old
behaviour.

## 1.6.2.25.05.05
- Add `fb.GetAudioChunk` / `FbAudioChunk` interface. See `vu meter` sample.
- Add `utils.GetClipboardText`. Returns an empty string if the clipboard contents
are not text.
- Add `utils.SetClipboardText`. No return value.
- Add `plman.GetGUID` / `plman.FindByGUID`.

A `GUID` is a unique identifier that persists with the playlist for its entire
lifetime between restarts and being renamed.

```js
var guid_str = plman.GetGUID(playlistIndex); // throws an error if playlistIndex is out of bounds
var playlistIndex = plman.FindByGUID(guid_str); // returns playlistIndex or -1 if not found
```

- Fix `utils.ColourPicker` bugs

- Fix `utils.IsFile` / `utils.IsDirectory` bugs.

- Update various samples fixed by `regor`.

- The `on_library_items_changed` callback now has a secondary `fromhook` argument so you can ignore updates that are not tag edits but database updates from components like `foo_playcount`.

```js
function on_library_items_changed(handles, fromhook) {
    if (fromhook)
        return;
   // react to actual file tag changes here
}
```

- Various improvements to handle list iteration methods.

- Various other bug fixes.
