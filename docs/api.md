The [original documentation](https://theqwertiest.github.io/foo_spider_monkey_panel/assets/generated_files/docs/html/index.html) remains
valid and can still be used as a reference. This page documents new features added since the last official release by `TheQwertiest`.

# Callbacks

## `on_download_image_done(url, image)`
|Arguments|||
|---|---|---|
|url|`string`|The url that was originally supplied to `utils.DownloadImageAsync`|
|image|`GdiBitmap` or `null` on failure.|

Called when thread created by `utils.DownloadImageAsync` is done.

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

## `fb.AddLocationsAsync(paths)`
|Arguments|||
|---|---|---|
|paths|`array`|An array of strings which could be files, urls, playlists.|

Returns a unique `task_id`.

Similar to `plman.AddLocations` except rather than specifiying a target playlist, you get a handle list generated
from the supplied paths/urls which are sent to a new [on_locations_added](#on_locations_addedtask_id-handle_list) callback.

!!! example
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

Returns an [FbAudioChunk](FbAudioChunk.md) instance or `null` on failure.

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

## `utils.DownloadFileAsync(url, path[, verify_image])`
|Arguments|||
|---|---|---|
|url|`string`|The remote file you want to download.|
|path|`string`|Full path including extension. The parent folder must already exist.|

No return value.

See also: [on_download_image_done](#on_download_image_doneurl-image).

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

# Properties
|||||
|---|---|---|---|
|window.IsDark|`boolean`|`read`|Should be used with `on_colours_changed`|
