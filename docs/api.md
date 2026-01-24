The [original documentation](https://theqwertiest.github.io/foo_spider_monkey_panel/assets/generated_files/docs/html/index.html) remains
valid and should still be used as a reference. This page documents new features added since the last official release by `TheQwertiest`.

# Callbacks

## `on_console_refresh()`
Called whenever new messages appear in the [foobar2000](https://www.foobar2000.org) `Console`.

## `on_download_file_done(path, success, error_text)`
|Arguments|||
|---|---|---|
|path|`string`|The path that was originally supplied to `utils.DownloadFileAsync`.|
|success|`boolean`|If `true` it means the web request was succesful and the file was saved correctly.|
|error_text|`string`|Empty if success is `true`. If success is `false`, it should describe what went wrong.|

Called when thread created by `utils.DownloadFileAsync` is done.

## `on_http_request_done(task_id, success, response_text, status, response_headers)`
|Arguments|||
|---|---|---|
|task_id|`number`|The return value from the original `utils.HTTPRequestAsync` call.|
|success|`boolean`|If `true`, it doesn't necessarily mean `HTTP` status `200` but it indicates the request was completed succesfully so the `response text` is from the server.|
|response_text|`string`|
|status|`number`|Will be `0` if the server was unreachable otherwise it should be the `HTTP` status code.|
|response_headers|`string`|This is a stringified `JSON` object so you need to use `JSON.parse` to access it. It's only valid if `success` was `true`.|

Called when thread created by `utils.HTTPRequestAsync` is done.

See [Web Requests](web-requests.md) for examples.

## `on_library_items_changed(handle_list, fromhook)`
|Arguments|||
|---|---|---|
|handle_list|`FbMetadbHandleList`|
|fromhook|`boolean`|`true` if changes comes from `Playback Statistics` or other components that utilise the same DB functionality.|

This callback has always existed but the `fromhook` argument is new. May be useful if you want to ignore playback statistics or similar updates in a library viewer.

## `on_locations_added(task_id, handle_list)`
|Arguments|||
|---|---|---|
|task_id|`number`|The return value from the original `fb.AddLocationsAsync` call.|
|handle_list|`FbMetadbHandleList`|

Called when thread created by `fb.AddLocationsAsync` is done.

# Methods

## `console.ClearBacklog()`
No return value.

## `console.GetLines(with_timestamp)`
|Arguments|||
|---|---|---|
|with_timestamp|`boolean`|

Returns an array of strings.

Use in conjunction with `on_console_refresh`.

## `fb.AddLocationsAsync(paths)`
|Arguments|||
|---|---|---|
|paths|`array`|An array of strings which could be files, urls, playlists.|

Returns a unique `task_id`.

Similar to `plman.AddLocations` except rather than specifiying a target playlist, you get a handle list generated
from the supplied paths/urls which are sent to a new [on_locations_added](#on_locations_addedtask_id-handle_list) callback.

```js
var files = ["z:\\1.mp3", "z:\\2.flac"];

function on_mouse_lbtn_dblclk() {
	var task_id = fb.AddLocationsAsync(files);
	console.log("got task_id", task_id);
}

function on_locations_added(task_id, handle_list) {
	console.log("callback task_id", task_id);
	console.log(handle_list.Count);
}
```

## `fb.GetAudioChunk(requested_length[, offset])`
|Arguments|||
|---|---|---|
|requested_length|`number`|seconds|
|offset|`number`|Default `0`|

Returns a [FbAudioChunk](FbAudioChunk.md) instance or `null` on failure.

Also see the included `VU Meter` sample.

## `fb.ShowPictureViewer(path)`
|Arguments|||
|---|---|---|
|path|`string`|

No return value.

## `plman.FindByGUID(str)`
|Arguments|||
|---|---|---|
|str|`string`|

Returns the index or `-1` on failure. Only strings returned by `plman.GetGUID` would be valid.

## `plman.GetGUID(playlistIndex)`
|Arguments|||
|---|---|---|
|playlistIndex|`number`|Must be valid.|

Returns a `string` which is a unique persistent property of the playlist.

## `utils.DownloadFileAsync(url, path)`
|Arguments|||
|---|---|---|
|url|`string`|The remote file you want to download.|
|path|`string`|Full path including extension. The parent folder must already exist.|

No return value.

See also: [on_download_file_done](#on_download_file_donepath-success-error_text).

> [!IMPORTANT]  
> You must ensure none of the folders in the `path` end with a period character.

## `utils.GetClipboardText()`
Returns a `string`. It will be empty if the clipboard contents are not text.

## `utils.SetClipboardText(text)`
|Arguments|||
|---|---|---|
|text|`string`|

No return value.

# Interfaces

## `FbMetadbHandle` `GetFileInfo([want_full_info])`
|Arguments|||
|---|---|---|
|want_full_info|`boolean`|Default `false`. Will be ignored if not using `foobar2000` `2.26` as it's not required. Even then it's only required if tag values are blocked by [LargeFieldsConfig-v2.txt](https://www.foobar2000.org/LargeFieldsConfig-v2).

While this method has always existed, the optional `want_full_info` argument is new.

Return a `FbFileInfo` instance.

## `FbMetadbHandleList` `OptimiseFileLayout([minimise])`
|Arguments|||
|---|---|---|
|minimise|`boolean`|Default `false`.|

No return value.

With `minimise` set to `false`, provides the functionality
of `Utilities>Optimize file layout` or if `minimise`
is `true` then `Utilities>Optimize file layout + minimize file size`.
Unlike the context menu versions, there is no prompt.

## `FbMetadbHandleList` `SaveAs(path)`
|Arguments|||
|---|---|---|
|path|`string`|

No return value.

Saves using native [foobar2000](https://www.foobar2000.org) `.fpl`
format so you should use that as the file extension. The
parent folder must already exist.

> [!IMPORTANT]  
> You must ensure none of the folders in the `path` end with a period character.

An example:
```js
var handle_list = plman.GetPlaylistItems(plman.ActivePlaylist);
handle_list.SaveAs("z:\\blah.fpl");
```

## `FbTitleFormat` `EvalWithMetadb(handle[, want_full_info])`
|Arguments|||
|---|---|---|
|handle|`FbMetadbHandle`|
|want_full_info|`boolean`|Default `false`. Will be ignored if not using `foobar2000` `2.26` as it's not required. Even then it's only required if tag values are blocked by [LargeFieldsConfig-v2.txt](https://www.foobar2000.org/LargeFieldsConfig-v2).

While this method has always existed, the optional `want_full_info` argument is new.

Returns a `string`.

# Properties
|||||
|---|---|---|---|
|window.IsDark|`boolean`|`read`|Should be used with `on_colours_changed`|
