**Properties**

|||||
|---|---|---|---|
|ChannelConfig|`number`|
|ChannelCount|`number`|
|Data|`array`|
|SampleCount|`number`|
|SampleRate|`number`|

You must check the return value from `fb.GetAudioChunk` is valid before using.

```js
// offset is optional, defaults to zero
var chunk = fb.GetAudioChunk(requested_length, offset);

if (chunk) {
	// chunk now has the following properties
	// chunk.ChannelConfig
	// chunk.ChannelCount
	// chunk.SampleRate
	// chunk.SampleCount
	// chunk.Data

	var data = chunk.Data;

	var channel_count = chunk.ChannelCount;
	for (var i = 0; i < data.length; i += channel_count) {
		// assuming stereo
		var l = data[i];
		var r = data[i + 1];
	}
}
```

`FbAudioChunk` is a simplified wrapper around this:

https://github.com/marc2k3/foobar2000-sdk/blob/main/foobar2000/SDK/audio_chunk.h
