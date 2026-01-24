#include "PCH.hpp"
#include "image_helpers.h"

namespace smp
{
	std::unique_ptr<Gdiplus::Bitmap> LoadWithWIC(IStream* stream)
	{
		auto factory = wil::CoCreateInstance<IWICImagingFactory>(CLSID_WICImagingFactory);

		if (!factory)
			return nullptr;

		wil::com_ptr<IWICBitmapDecoder> decoder;
		wil::com_ptr<IWICBitmapFrameDecode> frame;
		wil::com_ptr<IWICBitmapSource> source;

		if FAILED(factory->CreateDecoderFromStream(stream, nullptr, WICDecodeMetadataCacheOnDemand, &decoder))
			return nullptr;

		if FAILED(decoder->GetFrame(0, &frame))
			return nullptr;

		if FAILED(WICConvertBitmapSource(GUID_WICPixelFormat32bppPBGRA, frame.get(), &source))
			return nullptr;

		uint32_t w{}, h{};

		if FAILED(source->GetSize(&w, &h))
			return nullptr;

		auto bitmap = std::make_unique<Gdiplus::Bitmap>(w, h, PixelFormat32bppPARGB);
		const auto rect = Gdiplus::Rect(0, 0, static_cast<int32_t>(w), static_cast<int32_t>(h));
		Gdiplus::BitmapData bmpdata{};

		if (Gdiplus::Ok != bitmap->LockBits(&rect, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite, PixelFormat32bppPARGB, &bmpdata))
			return nullptr;

		const auto hr = source->CopyPixels(nullptr, bmpdata.Stride, bmpdata.Stride * bmpdata.Height, static_cast<uint8_t*>(bmpdata.Scan0));
		bitmap->UnlockBits(&bmpdata);

		if FAILED(hr)
			return nullptr;

		return bitmap;
	}
}
