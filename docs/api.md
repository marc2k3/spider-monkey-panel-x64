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
|handle_list|[IMetadbHandleList](../interfaces/IMetadbHandleList.md)|
|fromhook|`boolean`|`true` if changes comes from `Playback Statistics` or other components that utilise the same DB functionality.|

This callback has always existed but the `fromhook` argument is new. May be useful if you want to ignore playback statistics or similar updates in a library viewer.

## `on_locations_added(task_id, handle_list)`
|Arguments|||
|---|---|---|
|task_id|`number`|The return value from the original `fb.AddLocationsAsync` call.|
|handle_list|`FbMetadbHandleList`|

Called when thread created by `fb.AddLocationsAsync` is done.

# Methods
// TODO

# Properties
|window.IsDark|boolean|Should be used with `on_colours_changed`|
