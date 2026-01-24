#include "PCH.hpp"
#include "gdi_bitmap.h"
#include "gdi_raw_bitmap.h"

#include <2K3/KMeans.hpp>
#include <2K3/StackBlur.hpp>
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
	std::unique_ptr<Gdiplus::Bitmap> CreateDownsizedImage(Gdiplus::Bitmap& srcImg)
	{
		if (srcImg.GetWidth() <= 220u && srcImg.GetHeight() <= 220u)
		{
			std::unique_ptr<Gdiplus::Bitmap> pBitmap(srcImg.Clone(
				0,
				0,
				static_cast<int32_t>(srcImg.GetWidth()),
				static_cast<int32_t>(srcImg.GetHeight()),
				PixelFormat32bppPARGB
			));

			smp::CheckGdiPlusObject(pBitmap);
			return pBitmap;
		}

		const auto dwidth = static_cast<double>(srcImg.GetWidth());
		const auto dheight = static_cast<double>(srcImg.GetHeight());
		const auto ratio = std::min(220.0 / dwidth, 220.0 / dheight);
		const auto imgWidth = static_cast<int32_t>(dwidth * ratio);
		const auto imgHeight = static_cast<int32_t>(dheight * ratio);

		auto pBitmap = std::make_unique<Gdiplus::Bitmap>(imgWidth, imgHeight, PixelFormat32bppPARGB);
		smp::CheckGdiPlusObject(pBitmap);

		auto gr = Gdiplus::Graphics(pBitmap.get());
		auto status = gr.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBilinear);
		smp::CheckGdi(status, "SetInterpolationMode");

		status = gr.DrawImage(&srcImg, 0, 0, imgWidth, imgHeight);
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

	JsGdiBitmap::JsGdiBitmap(JSContext* ctx, std::unique_ptr<Gdiplus::Bitmap> bitmap) : m_ctx(ctx), m_bitmap(std::move(bitmap)) {}

	std::unique_ptr<JsGdiBitmap> JsGdiBitmap::CreateNative(JSContext* cx, std::unique_ptr<Gdiplus::Bitmap> gdiBitmap)
	{
		QwrException::ExpectTrue(!!gdiBitmap, "Internal error: Gdiplus::Bitmap object is null");

		return std::unique_ptr<JsGdiBitmap>(new JsGdiBitmap(cx, std::move(gdiBitmap)));
	}

	std::unique_ptr<Gdiplus::Bitmap> JsGdiBitmap::ApplyAttributes(const Gdiplus::ImageAttributes& ia)
	{
		const auto width = static_cast<int32_t>(m_bitmap->GetWidth());
		const auto height = static_cast<int32_t>(m_bitmap->GetHeight());
		auto bitmap = std::make_unique<Gdiplus::Bitmap>(width, height, PixelFormat32bppPARGB);

		auto gr = Gdiplus::Graphics(bitmap.get());
		auto status = gr.DrawImage(m_bitmap.get(), Gdiplus::Rect(0, 0, width, height), 0, 0, width, height, Gdiplus::UnitPixel, &ia);
		smp::CheckGdi(status, "DrawImage");

		return bitmap;
	}

	uint32_t JsGdiBitmap::GetInternalSize()
	{
		return m_bitmap->GetWidth() * m_bitmap->GetHeight() * Gdiplus::GetPixelFormatSize(m_bitmap->GetPixelFormat()) / 8;
	}

	Gdiplus::Bitmap* JsGdiBitmap::GdiBitmap() const
	{
		return m_bitmap.get();
	}

	JSObject* JsGdiBitmap::Constructor(JSContext* ctx, JsGdiBitmap* other)
	{
		QwrException::ExpectTrue(other, "Invalid argument type");

		auto pGdi = other->GdiBitmap();

		std::unique_ptr<Gdiplus::Bitmap> img(pGdi->Clone(0, 0, pGdi->GetWidth(), pGdi->GetHeight(), PixelFormat32bppPARGB));
		smp::CheckGdiPlusObject(img, pGdi);

		return JsGdiBitmap::CreateJs(ctx, std::move(img));
	}

	uint32_t JsGdiBitmap::get_Height()
	{
		return m_bitmap->GetHeight();
	}

	uint32_t JsGdiBitmap::get_Width()
	{
		return m_bitmap->GetWidth();
	}

	JSObject* JsGdiBitmap::ApplyAlpha(uint8_t alpha)
	{
		Gdiplus::ImageAttributes ia;
		Gdiplus::ColorMatrix cm{};
		cm.m[0][0] = cm.m[1][1] = cm.m[2][2] = cm.m[4][4] = 1.f;
		cm.m[3][3] = static_cast<float>(alpha) / 255.f;
		ia.SetColorMatrix(&cm);
		
		auto bitmap = ApplyAttributes(ia);
		return JsGdiBitmap::CreateJs(m_ctx, std::move(bitmap));
	}

	void JsGdiBitmap::ApplyMask(JsGdiBitmap* mask)
	{
		QwrException::ExpectTrue(mask, "mask argument is null");

		auto bitmap_mask = mask->GdiBitmap();

		QwrException::ExpectTrue(
			bitmap_mask->GetHeight() == m_bitmap->GetHeight() && bitmap_mask->GetWidth() == m_bitmap->GetWidth(),
			"Mismatched dimensions"
		);

		const auto rect = Gdiplus::Rect(0, 0, static_cast<int32_t>(m_bitmap->GetWidth()), static_cast<int32_t>(m_bitmap->GetHeight()));
		Gdiplus::BitmapData bmpdata_dst, bmpdata_mask;

		if (Gdiplus::Ok == bitmap_mask->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpdata_mask))
		{
			if (Gdiplus::Ok == m_bitmap->LockBits(&rect, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &bmpdata_dst))
			{
				uint32_t* mask_data = static_cast<uint32_t*>(bmpdata_mask.Scan0);
				uint32_t* dst_data = static_cast<uint32_t*>(bmpdata_dst.Scan0);
				const uint32_t* mask_end = mask_data + (rect.Width * rect.Height);

				while (mask_data < mask_end)
				{
					uint32_t alpha = (((~*mask_data & UINT8_MAX) * (*dst_data >> 24)) << 16) & 0xff000000;
					*dst_data = alpha | (*dst_data & 0xffffff);
					++mask_data;
					++dst_data;
				}

				m_bitmap->UnlockBits(&bmpdata_dst);
			}
			bitmap_mask->UnlockBits(&bmpdata_mask);
		}
	}

	JSObject* JsGdiBitmap::Clone(float x, float y, float w, float h)
	{
		std::unique_ptr<Gdiplus::Bitmap> img(m_bitmap->Clone(x, y, w, h, PixelFormat32bppPARGB));
		smp::CheckGdiPlusObject(img, m_bitmap.get());

		return JsGdiBitmap::CreateJs(m_ctx, std::move(img));
	}

	JSObject* JsGdiBitmap::CreateRawBitmap()
	{
		return JsGdiRawBitmap::CreateJs(m_ctx, m_bitmap.get());
	}

	JS::Value JsGdiBitmap::GetColourScheme(uint32_t count)
	{
		auto resized = CreateDownsizedImage(*m_bitmap);
		const auto rect = Gdiplus::Rect(0, 0, static_cast<int32_t>(resized->GetWidth()), static_cast<int32_t>(resized->GetHeight()));
		Gdiplus::BitmapData bmpdata;

		const auto status = resized->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpdata);
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

		std::ignore = resized->UnlockBits(&bmpdata);

		std::vector<std::pair<uint32_t, uint32_t>> sort_vec(color_counters.cbegin(), color_counters.cend());
		std::ranges::sort(sort_vec, [](const auto& a, const auto& b)
			{
				return a.second > b.second;
			});

		sort_vec.resize(std::min<size_t>(count, color_counters.size()));

		JS::RootedValue jsValue(m_ctx);
		convert::to_js::ToArrayValue(
			m_ctx,
			sort_vec,
			[](const auto& vec, auto index)
				{
					return vec[index].first;
				},
			&jsValue);

		return jsValue;
	}

	std::string JsGdiBitmap::GetColourSchemeJSON(uint32_t count)
	{
		auto resized = CreateDownsizedImage(*m_bitmap);
		const auto rect = Gdiplus::Rect(0, 0, static_cast<int32_t>(resized->GetWidth()), static_cast<int32_t>(resized->GetHeight()));
		Gdiplus::BitmapData bmpdata;

		const auto status = resized->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpdata);
		smp::CheckGdi(status, "LockBits");

		static constexpr std::array shifts = { RED_SHIFT, GREEN_SHIFT, BLUE_SHIFT };
		const auto colours_length = bmpdata.Width * bmpdata.Height;
		const auto* colours = static_cast<const uint32_t*>(bmpdata.Scan0);
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

		std::ignore = resized->UnlockBits(&bmpdata);

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
		JS::RootedObject jsObject(m_ctx, JsGdiGraphics::CreateJs(m_ctx));
		auto* pNativeObject = JsGdiGraphics::ExtractNative(m_ctx, jsObject);
		QwrException::ExpectTrue(pNativeObject, "Internal error: failed to get JsGdiGraphics object");

		auto gr = std::make_unique<Gdiplus::Graphics>(m_bitmap.get());
		pNativeObject->SetGraphicsObject(gr.release());
		return jsObject;
	}

	JSObject* JsGdiBitmap::InvertColours()
	{
		Gdiplus::ImageAttributes ia;
		Gdiplus::ColorMatrix cm{};
		cm.m[0][0] = cm.m[1][1] = cm.m[2][2] = -1.f;
		cm.m[3][3] = cm.m[4][0] = cm.m[4][1] = cm.m[4][2] = cm.m[4][4] = 1.f;
		ia.SetColorMatrix(&cm);

		auto bitmap = ApplyAttributes(ia);
		return JsGdiBitmap::CreateJs(m_ctx, std::move(bitmap));
	}

	void JsGdiBitmap::ReleaseGraphics(JsGdiGraphics* graphics)
	{
		if (graphics)
		{
			auto pGdiGraphics = graphics->GetGraphicsObject();
			graphics->SetGraphicsObject(nullptr);
			delete pGdiGraphics;
		}
	}

	JSObject* JsGdiBitmap::Resize(uint32_t w, uint32_t h, uint32_t interpolationMode)
	{
		std::unique_ptr<Gdiplus::Bitmap> bitmap(new Gdiplus::Bitmap(w, h, PixelFormat32bppPARGB));
		smp::CheckGdiPlusObject(bitmap);

		auto gr = Gdiplus::Graphics(bitmap.get());
		auto status = gr.SetInterpolationMode(static_cast<Gdiplus::InterpolationMode>(interpolationMode));
		smp::CheckGdi(status, "SetInterpolationMode");

		status = gr.DrawImage(m_bitmap.get(), 0, 0, w, h);
		smp::CheckGdi(status, "DrawImage");

		return JsGdiBitmap::CreateJs(m_ctx, std::move(bitmap));
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
		const auto status = m_bitmap->RotateFlip(static_cast<Gdiplus::RotateFlipType>(mode));
		smp::CheckGdi(status, "RotateFlip");
	}

	bool JsGdiBitmap::SaveAs(const std::wstring& path, const std::wstring& format)
	{
		std::map<std::wstring, CLSID> encoder_map;
		uint32_t num{}, size{};

		if (Gdiplus::Ok == Gdiplus::GetImageEncodersSize(&num, &size) && size > 0u)
		{
			std::vector<Gdiplus::ImageCodecInfo> codecs(size);

			if (Gdiplus::Ok == Gdiplus::GetImageEncoders(num, size, codecs.data()))
			{
				for (const auto& codec : std::views::take(codecs, num))
				{
					encoder_map.emplace(codec.MimeType, codec.Clsid);
				}
			}
		}

		const auto it = encoder_map.find(format);

		if (it == encoder_map.end())
			return false;

		return Gdiplus::Ok == m_bitmap->Save(path.data(), &it->second);
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
		const auto size = D2D1_SIZE_U(m_bitmap->GetWidth(), m_bitmap->GetHeight());
		const auto rect = Gdiplus::Rect(0, 0, static_cast<int32_t>(size.width), static_cast<int32_t>(size.height));

		if (Gdiplus::Ok == m_bitmap->LockBits(&rect, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite, PixelFormat32bppPARGB, &bmpdata))
		{
			auto job = ::StackBlur(radius, size);
			job.Run(static_cast<uint8_t*>(bmpdata.Scan0));

			m_bitmap->UnlockBits(&bmpdata);
		}
	}
}
