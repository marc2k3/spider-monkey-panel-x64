#include <PCH.hpp>
#include "gdi_bitmap.h"
#include "gdi_raw_bitmap.h"

#include <2K3/KMeans.hpp>
#include <2K3/StackBlur.hpp>
#include <utils/gdi_error_helpers.h>
#include <utils/image_helpers.h>

namespace
{
	using namespace mozjs;

	DEFINE_JS_CLASS_OPS(JsGdiBitmap::FinalizeJsObject)

	DEFINE_JS_CLASS("GdiBitmap")

	MJS_DEFINE_JS_FN_FROM_NATIVE(ApplyAlpha, JsGdiBitmap::ApplyAlpha)
	MJS_DEFINE_JS_FN_FROM_NATIVE(ApplyMask, JsGdiBitmap::ApplyMask)
	MJS_DEFINE_JS_FN_FROM_NATIVE(Clone, JsGdiBitmap::Clone)
	MJS_DEFINE_JS_FN_FROM_NATIVE(CreateRawBitmap, JsGdiBitmap::CreateRawBitmap)
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetColourScheme, JsGdiBitmap::GetColourScheme)
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetColourSchemeJSON, JsGdiBitmap::GetColourSchemeJSON)
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetGraphics, JsGdiBitmap::GetGraphics)
	MJS_DEFINE_JS_FN_FROM_NATIVE(InvertColours, JsGdiBitmap::InvertColours)
	MJS_DEFINE_JS_FN_FROM_NATIVE(ReleaseGraphics, JsGdiBitmap::ReleaseGraphics)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(Resize, JsGdiBitmap::Resize, JsGdiBitmap::ResizeWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE(RotateFlip, JsGdiBitmap::RotateFlip)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(SaveAs, JsGdiBitmap::SaveAs, JsGdiBitmap::SaveAsWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE(StackBlur, JsGdiBitmap::StackBlur)

	constexpr auto jsFunctions = std::to_array<JSFunctionSpec>(
		{
			JS_FN("ApplyAlpha", ApplyAlpha, 1, kDefaultPropsFlags),
			JS_FN("ApplyMask", ApplyMask, 1, kDefaultPropsFlags),
			JS_FN("Clone", Clone, 4, kDefaultPropsFlags),
			JS_FN("CreateRawBitmap", CreateRawBitmap, 0, kDefaultPropsFlags),
			JS_FN("GetColourScheme", GetColourScheme, 1, kDefaultPropsFlags),
			JS_FN("GetColourSchemeJSON", GetColourSchemeJSON, 1, kDefaultPropsFlags),
			JS_FN("GetGraphics", GetGraphics, 0, kDefaultPropsFlags),
			JS_FN("InvertColours", InvertColours, 0, kDefaultPropsFlags),
			JS_FN("ReleaseGraphics", ReleaseGraphics, 1, kDefaultPropsFlags),
			JS_FN("Resize", Resize, 2, kDefaultPropsFlags),
			JS_FN("RotateFlip", RotateFlip, 1, kDefaultPropsFlags),
			JS_FN("SaveAs", SaveAs, 1, kDefaultPropsFlags),
			JS_FN("StackBlur", StackBlur, 1, kDefaultPropsFlags),
			JS_FS_END,
		});

	MJS_DEFINE_JS_FN_FROM_NATIVE(get_Height, JsGdiBitmap::get_Height)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_Width, JsGdiBitmap::get_Width)

	constexpr auto jsProperties = std::to_array<JSPropertySpec>(
		{
			JS_PSG("Height", get_Height, kDefaultPropsFlags),
			JS_PSG("Width", get_Width, kDefaultPropsFlags),
			JS_PS_END,
		});

	MJS_DEFINE_JS_FN_FROM_NATIVE(GdiBitmap_Constructor, JsGdiBitmap::Constructor)
}

namespace
{
	std::unique_ptr<Gdiplus::Bitmap> CreateDownsizedImage(Gdiplus::Bitmap& srcImg, uint32_t maxPixelCount)
	{
		const auto [imgWidth, imgHeight] = [&srcImg, maxPixelCount]
			{
				if (srcImg.GetWidth() * srcImg.GetHeight() > maxPixelCount)
				{
					const double ratio = static_cast<double>(srcImg.GetWidth()) / srcImg.GetHeight();
					const auto imgHeight = static_cast<uint32_t>(std::round(std::sqrt(maxPixelCount / ratio)));
					const auto imgWidth = static_cast<uint32_t>(std::round(imgHeight * ratio));

					return std::make_tuple(imgWidth, imgHeight);
				}
				else
				{
					return std::make_tuple(srcImg.GetWidth(), srcImg.GetHeight());
				}
			}();

		auto pBitmap = std::make_unique<Gdiplus::Bitmap>(imgWidth, imgHeight, PixelFormat32bppPARGB);
		smp::CheckGdiPlusObject(pBitmap);

		auto gr = Gdiplus::Graphics(pBitmap.get());

		auto status = gr.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBilinear);
		smp::CheckGdi(status, "SetInterpolationMode");

		status = gr.DrawImage(&srcImg, 0, 0, imgWidth, imgHeight); // scale image down
		smp::CheckGdi(status, "DrawImage");

		return pBitmap;
	}
}

namespace mozjs
{
	const JSClass JsGdiBitmap::JsClass = jsClass;
	const JSFunctionSpec* JsGdiBitmap::JsFunctions = jsFunctions.data();
	const JSPropertySpec* JsGdiBitmap::JsProperties = jsProperties.data();
	const JsPrototypeId JsGdiBitmap::PrototypeId = JsPrototypeId::GdiBitmap;
	const JSNative JsGdiBitmap::JsConstructor = ::GdiBitmap_Constructor;

	JsGdiBitmap::JsGdiBitmap(JSContext* cx, std::unique_ptr<Gdiplus::Bitmap> gdiBitmap) : pJsCtx_(cx), pGdi_(std::move(gdiBitmap)) {}

	std::unique_ptr<JsGdiBitmap> JsGdiBitmap::CreateNative(JSContext* cx, std::unique_ptr<Gdiplus::Bitmap> gdiBitmap)
	{
		QwrException::ExpectTrue(!!gdiBitmap, "Internal error: Gdiplus::Bitmap object is null");

		return std::unique_ptr<JsGdiBitmap>(new JsGdiBitmap(cx, std::move(gdiBitmap)));
	}

	uint32_t JsGdiBitmap::GetInternalSize()
	{
		return pGdi_->GetWidth() * pGdi_->GetHeight() * Gdiplus::GetPixelFormatSize(pGdi_->GetPixelFormat()) / 8;
	}

	Gdiplus::Bitmap* JsGdiBitmap::GdiBitmap() const
	{
		return pGdi_.get();
	}

	JSObject* JsGdiBitmap::Constructor(JSContext* cx, JsGdiBitmap* other)
	{
		QwrException::ExpectTrue(other, "Invalid argument type");

		auto pGdi = other->GdiBitmap();

		std::unique_ptr<Gdiplus::Bitmap> img(pGdi->Clone(0, 0, pGdi->GetWidth(), pGdi->GetHeight(), PixelFormat32bppPARGB));
		smp::CheckGdiPlusObject(img, pGdi);

		return JsGdiBitmap::CreateJs(cx, std::move(img));
	}

	uint32_t JsGdiBitmap::get_Height()
	{
		return pGdi_->GetHeight();
	}

	uint32_t JsGdiBitmap::get_Width()
	{
		return pGdi_->GetWidth();
	}

	JSObject* JsGdiBitmap::ApplyAlpha(uint8_t alpha)
	{
		const UINT width = pGdi_->GetWidth();
		const UINT height = pGdi_->GetHeight();

		std::unique_ptr<Gdiplus::Bitmap> out(new Gdiplus::Bitmap(width, height, PixelFormat32bppPARGB));
		smp::CheckGdiPlusObject(out);

		Gdiplus::ColorMatrix cm{};
		cm.m[0][0] = cm.m[1][1] = cm.m[2][2] = cm.m[4][4] = 1.0;
		cm.m[3][3] = static_cast<float>(alpha) / 255;

		Gdiplus::ImageAttributes ia;
		auto status = ia.SetColorMatrix(&cm);
		smp::CheckGdi(status, "SetColorMatrix");

		Gdiplus::Graphics g(out.get());
		status = g.DrawImage(
			pGdi_.get(),
			Gdiplus::Rect{ 0, 0, static_cast<int>(width), static_cast<int>(height) },
			0,
			0,
			width,
			height,
			Gdiplus::UnitPixel,
			&ia
		);

		smp::CheckGdi(status, "DrawImage");
		return JsGdiBitmap::CreateJs(pJsCtx_, std::move(out));
	}

	void JsGdiBitmap::ApplyMask(JsGdiBitmap* mask)
	{
		QwrException::ExpectTrue(mask, "mask argument is null");

		Gdiplus::Bitmap* pBitmapMask = mask->GdiBitmap();

		QwrException::ExpectTrue(
			pBitmapMask->GetHeight() == pGdi_->GetHeight() && pBitmapMask->GetWidth() == pGdi_->GetWidth(),
			"Mismatched dimensions"
		);

		const Gdiplus::Rect rect{ 0, 0, static_cast<int>(pGdi_->GetWidth()), static_cast<int>(pGdi_->GetHeight()) };

		Gdiplus::BitmapData maskBmpData = { 0 };
		auto status = pBitmapMask->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &maskBmpData);
		smp::CheckGdi(status, "mask::LockBits");

		auto autoMaskBits = wil::scope_exit([pBitmapMask, &maskBmpData]
			{
				pBitmapMask->UnlockBits(&maskBmpData);
			});

		Gdiplus::BitmapData dstBmpData = { 0 };
		status = pGdi_->LockBits(&rect, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &dstBmpData);
		smp::CheckGdi(status, "dst::LockBits");

		auto autoDstBits = wil::scope_exit([&pGdi = pGdi_, &dstBmpData]
			{
				pGdi->UnlockBits(&dstBmpData);
			});

		const auto maskRange = std::ranges::subrange(
			reinterpret_cast<uint32_t*>(maskBmpData.Scan0),
			reinterpret_cast<uint32_t*>(maskBmpData.Scan0) + rect.Width * rect.Height
		);

		for (auto pMaskIt = maskRange.begin(), pDst = reinterpret_cast<uint32_t*>(dstBmpData.Scan0); pMaskIt != maskRange.end(); ++pMaskIt, ++pDst)
		{
			/// Method 1:
			// alpha = (~*p_mask & 0xff) * (*p_dst >> 24) + 0x80;
			// *p_dst = ((((alpha >> 8) + alpha) & 0xff00) << 16) | (*p_dst & 0xffffff);

			/// Method 2
			uint32_t alpha = (((~*pMaskIt & 0xff) * (*pDst >> 24)) << 16) & 0xff000000;
			*pDst = alpha | (*pDst & 0xffffff);
		}
	}

	JSObject* JsGdiBitmap::Clone(float x, float y, float w, float h)
	{
		std::unique_ptr<Gdiplus::Bitmap> img(pGdi_->Clone(x, y, w, h, PixelFormat32bppPARGB));
		smp::CheckGdiPlusObject(img, pGdi_.get());

		return JsGdiBitmap::CreateJs(pJsCtx_, std::move(img));
	}

	JSObject* JsGdiBitmap::CreateRawBitmap()
	{
		return JsGdiRawBitmap::CreateJs(pJsCtx_, pGdi_.get());
	}

	JS::Value JsGdiBitmap::GetColourScheme(uint32_t count)
	{
		constexpr uint32_t kMaxPixelCount = 220 * 220;
		auto pBitmap = CreateDownsizedImage(*pGdi_, kMaxPixelCount);
		const Gdiplus::Rect rect{ 0, 0, static_cast<int>(pBitmap->GetWidth()), static_cast<int>(pBitmap->GetHeight()) };
		Gdiplus::BitmapData bmpdata{};

		const auto status = pBitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpdata);
		smp::CheckGdi(status, "LockBits");

		std::map<uint32_t, uint32_t> color_counters;
		const auto colourRange = std::ranges::subrange(
			reinterpret_cast<const uint32_t*>(bmpdata.Scan0),
			reinterpret_cast<const uint32_t*>(bmpdata.Scan0) + bmpdata.Width * bmpdata.Height
		);

		for (auto colour : colourRange)
		{
			// format: 0xaarrggbb
			uint32_t r = (colour >> 16) & 0xff;
			uint32_t g = (colour >> 8) & 0xff;
			uint32_t b = colour & 0xff;

			// Round colors
			r = (r > 0xef) ? 0xff : (r + 0x10) & 0xe0;
			g = (g > 0xef) ? 0xff : (g + 0x10) & 0xe0;
			b = (b > 0xef) ? 0xff : (b + 0x10) & 0xe0;

			++color_counters[Gdiplus::Color::MakeARGB(
				0xff,
				static_cast<BYTE>(r),
				static_cast<BYTE>(g),
				static_cast<BYTE>(b)
			)];
		}

		pBitmap->UnlockBits(&bmpdata);

		std::vector<std::pair<uint32_t, uint32_t>> sort_vec(color_counters.cbegin(), color_counters.cend());
		std::ranges::sort(sort_vec, [](const auto& a, const auto& b)
			{
				return a.second > b.second;
			});

		sort_vec.resize(std::min<size_t>(count, color_counters.size()));

		JS::RootedValue jsValue(pJsCtx_);
		convert::to_js::ToArrayValue(
			pJsCtx_,
			sort_vec,
			[](const auto& vec, auto index) {
				return vec[index].first;
			},
			&jsValue);

		return jsValue;
	}

	std::string JsGdiBitmap::GetColourSchemeJSON(uint32_t count)
	{
		const int width = std::min<int>(pGdi_->GetWidth(), 220);
		const int height = std::min<int>(pGdi_->GetHeight(), 220);
		const Gdiplus::Rect rect(0, 0, width, height);
		Gdiplus::BitmapData bmpdata;

		auto resized = CreateDownsizedImage(*pGdi_, 220 * 220);
		auto status = resized->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpdata);
		smp::CheckGdi(status, "LockBits");

		const uint32_t colours_length = bmpdata.Width * bmpdata.Height;
		const uint32_t* colours = static_cast<const uint32_t*>(bmpdata.Scan0);
		static constexpr std::array shifts = { RED_SHIFT, GREEN_SHIFT, BLUE_SHIFT };
		std::map<ColourValues, uint32_t> colour_counters;

		for (const auto i : indices(colours_length))
		{
			ColourValues values{};

			std::ranges::transform(shifts, values.begin(), [colour = colours[i]](const auto& shift)
				{
					const uint8_t value = (colour >> shift) & UINT8_MAX;
					return static_cast<double>(value > 251 ? UINT8_MAX : (value + 4) & 0xf8);
				});

			++colour_counters[values];
		}

		status = resized->UnlockBits(&bmpdata);
		smp::CheckGdi(status, "UnlockBits");

		KPoints points;
		for (auto&& [index, value] : std::views::enumerate(colour_counters))
		{
			points.emplace_back(index, value.first, value.second);
		}

		auto kmeans = KMeans(points, count);
		auto clusters = kmeans.run();
		auto j = JSON::array();

		for (auto&& cluster : clusters)
		{
			j.push_back({
				{ "col", cluster.get_colour() },
				{ "freq", cluster.get_frequency(colours_length) },
			});
		}

		return j.dump(2);
	}

	JSObject* JsGdiBitmap::GetGraphics()
	{
		std::unique_ptr<Gdiplus::Graphics> g(new Gdiplus::Graphics(pGdi_.get()));
		smp::CheckGdiPlusObject(g);

		JS::RootedObject jsObject(pJsCtx_, JsGdiGraphics::CreateJs(pJsCtx_));

		auto* pNativeObject = JsGdiGraphics::ExtractNative(pJsCtx_, jsObject);
		QwrException::ExpectTrue(pNativeObject, "Internal error: failed to get JsGdiGraphics object");

		pNativeObject->SetGraphicsObject(g.release());

		return jsObject;
	}

	JSObject* JsGdiBitmap::InvertColours()
	{
		const UINT width = pGdi_->GetWidth();
		const UINT height = pGdi_->GetHeight();

		std::unique_ptr<Gdiplus::Bitmap> out(new Gdiplus::Bitmap(width, height, PixelFormat32bppPARGB));
		smp::CheckGdiPlusObject(out);

		Gdiplus::ColorMatrix cm{};
		cm.m[0][0] = cm.m[1][1] = cm.m[2][2] = -1.f;
		cm.m[3][3] = cm.m[4][0] = cm.m[4][1] = cm.m[4][2] = cm.m[4][4] = 1.f;

		Gdiplus::ImageAttributes ia;
		auto status = ia.SetColorMatrix(&cm);
		smp::CheckGdi(status, "SetColorMatrix");

		auto g = Gdiplus::Graphics(out.get());

		status = g.DrawImage(
			pGdi_.get(),
			Gdiplus::Rect{ 0, 0, static_cast<int>(width), static_cast<int>(height) },
			0,
			0,
			width,
			height,
			Gdiplus::UnitPixel,
			&ia
		);

		smp::CheckGdi(status, "DrawImage");

		return JsGdiBitmap::CreateJs(pJsCtx_, std::move(out));
	}

	void JsGdiBitmap::ReleaseGraphics(JsGdiGraphics* graphics)
	{
		if (!graphics)
		{ // Not an error
			return;
		}

		auto pGdiGraphics = graphics->GetGraphicsObject();
		graphics->SetGraphicsObject(nullptr);
		delete pGdiGraphics;
	}

	JSObject* JsGdiBitmap::Resize(uint32_t w, uint32_t h, uint32_t interpolationMode)
	{
		std::unique_ptr<Gdiplus::Bitmap> bitmap(new Gdiplus::Bitmap(w, h, PixelFormat32bppPARGB));
		smp::CheckGdiPlusObject(bitmap);

		Gdiplus::Graphics g(bitmap.get());
		auto status = g.SetInterpolationMode(static_cast<Gdiplus::InterpolationMode>(interpolationMode));
		smp::CheckGdi(status, "SetInterpolationMode");

		status = g.DrawImage(pGdi_.get(), 0, 0, w, h);
		smp::CheckGdi(status, "DrawImage");

		return JsGdiBitmap::CreateJs(pJsCtx_, std::move(bitmap));
	}

	JSObject* JsGdiBitmap::ResizeWithOpt(size_t optArgCount, uint32_t w, uint32_t h, uint32_t interpolationMode)
	{
		switch (optArgCount)
		{
		case 0:
			return Resize(w, h, interpolationMode);
		case 1:
			return Resize(w, h);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	void JsGdiBitmap::RotateFlip(uint32_t mode)
	{
		const auto status = pGdi_->RotateFlip(static_cast<Gdiplus::RotateFlipType>(mode));
		smp::CheckGdi(status, "RotateFlip");
	}

	bool JsGdiBitmap::SaveAs(const std::wstring& path, const std::wstring& format)
	{
		uint32_t num{}, size{};
		if (Gdiplus::Ok != Gdiplus::GetImageEncodersSize(&num, &size))
			return false;

		std::vector<uint8_t> imageCodeInfoBuf(size);
		auto* pImageCodecInfo = reinterpret_cast<Gdiplus::ImageCodecInfo*>(imageCodeInfoBuf.data());

		if (Gdiplus::Ok != Gdiplus::GetImageEncoders(num, size, pImageCodecInfo))
			return false;

		std::span<Gdiplus::ImageCodecInfo> codecSpan{ pImageCodecInfo, num };

		const auto it = std::ranges::find_if(codecSpan, [&format](const auto& codec)
			{
				return (format == codec.MimeType);
			});

		if (it == codecSpan.end())
			return false;

		return Gdiplus::Ok == pGdi_->Save(path.c_str(), &it->Clsid);
	}

	bool JsGdiBitmap::SaveAsWithOpt(size_t optArgCount, const std::wstring& path, const std::wstring& format /* ='image/png' */)
	{
		switch (optArgCount)
		{
		case 0:
			return SaveAs(path, format);
		case 1:
			return SaveAs(path);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	void JsGdiBitmap::StackBlur(uint8_t radius)
	{
		Gdiplus::BitmapData bmpdata;
		const auto size = D2D1_SIZE_U(pGdi_->GetWidth(), pGdi_->GetHeight());

		const auto rect = Gdiplus::Rect(
			0,
			0,
			static_cast<int>(size.width),
			static_cast<int>(size.height)
		);

		if (Gdiplus::Ok == pGdi_->LockBits(&rect, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite, PixelFormat32bppPARGB, &bmpdata))
		{
			auto job = ::StackBlur(radius, size);
			job.Run(static_cast<uint8_t*>(bmpdata.Scan0));

			pGdi_->UnlockBits(&bmpdata);
		}
	}
}
