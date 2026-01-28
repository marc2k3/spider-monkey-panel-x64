#include "PCH.hpp"
#include "fb_metadb_handle.h"
#include "fb_file_info.h"

namespace
{
	using namespace mozjs;

	DEFINE_JS_CLASS_OPS(JsFbMetadbHandle::FinalizeJsObject)

	DEFINE_JS_CLASS("FbMetadbHandle")

	MJS_DEFINE_JS_FN_FROM_NATIVE(Compare, JsFbMetadbHandle::Compare)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(GetFileInfo, JsFbMetadbHandle::GetFileInfo, JsFbMetadbHandle::GetFileInfoWithOpt, 1)

	constexpr auto jsFunctions = std::to_array<JSFunctionSpec>(
		{
			JS_FN("Compare", Compare, 1, kDefaultPropsFlags),
			JS_FN("GetFileInfo", GetFileInfo, 1, kDefaultPropsFlags),
			JS_FS_END,
		});

	MJS_DEFINE_JS_FN_FROM_NATIVE(get_FileSize, JsFbMetadbHandle::get_FileSize)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_Length, JsFbMetadbHandle::get_Length)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_Path, JsFbMetadbHandle::get_Path)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_RawPath, JsFbMetadbHandle::get_RawPath)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_SubSong, JsFbMetadbHandle::get_SubSong)

	constexpr auto jsProperties = std::to_array<JSPropertySpec>(
		{
			JS_PSG("FileSize", get_FileSize, kDefaultPropsFlags),
			JS_PSG("Length", get_Length, kDefaultPropsFlags),
			JS_PSG("Path", get_Path, kDefaultPropsFlags),
			JS_PSG("RawPath", get_RawPath, kDefaultPropsFlags),
			JS_PSG("SubSong", get_SubSong, kDefaultPropsFlags),
			JS_PS_END,
		});
}

namespace mozjs
{
	using namespace smp;

	const JSClass JsFbMetadbHandle::JsClass = jsClass;
	const JSFunctionSpec* JsFbMetadbHandle::JsFunctions = jsFunctions.data();
	const JSPropertySpec* JsFbMetadbHandle::JsProperties = jsProperties.data();
	const JsPrototypeId JsFbMetadbHandle::PrototypeId = JsPrototypeId::FbMetadbHandle;

	JsFbMetadbHandle::JsFbMetadbHandle(JSContext* ctx, const metadb_handle_ptr& handle) : m_ctx(ctx), m_handle(handle) {}

	std::unique_ptr<mozjs::JsFbMetadbHandle> JsFbMetadbHandle::CreateNative(JSContext* ctx, const metadb_handle_ptr& handle)
	{
		QwrException::ExpectTrue(handle.is_valid(), "Internal error: metadb_handle_ptr is null");

		return std::unique_ptr<JsFbMetadbHandle>(new JsFbMetadbHandle(ctx, handle));
	}

	uint32_t JsFbMetadbHandle::GetInternalSize()
	{
		return sizeof(metadb_handle);
	}

	metadb_handle_ptr& JsFbMetadbHandle::GetHandle()
	{
		return m_handle;
	}

	bool JsFbMetadbHandle::Compare(JsFbMetadbHandle* handle)
	{
		QwrException::ExpectTrue(handle, "handle argument is null");

		return (handle->GetHandle() == m_handle);
	}

	JSObject* JsFbMetadbHandle::GetFileInfo(bool want_full_info)
	{
		metadb_info_container::ptr info = m_handle->get_info_ref();

		if (info->isInfoPartial() && want_full_info)
		{
			try
			{
				info = m_handle->get_full_info_ref(fb2k::noAbort);
			}
			catch (...) {}
		}

		return JsFbFileInfo::CreateJs(m_ctx, info);
	}

	JSObject* JsFbMetadbHandle::GetFileInfoWithOpt(size_t optArgCount, bool want_full_info)
	{
		switch (optArgCount)
		{
		case 0:
			return GetFileInfo(want_full_info);
		case 1:
			return GetFileInfo();
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	uint64_t JsFbMetadbHandle::get_FileSize()
	{
		return m_handle->get_filesize();
	}

	double JsFbMetadbHandle::get_Length()
	{
		return m_handle->get_length();
	}

	std::string JsFbMetadbHandle::get_Path()
	{
		return filesystem::g_get_native_path(m_handle->get_path()).get_ptr();
	}

	std::string JsFbMetadbHandle::get_RawPath()
	{
		const std::string rp = m_handle->get_path();

		if (rp.starts_with("file-relative://"))
		{
			const auto native = filesystem::g_get_native_path(rp.c_str());
			return fmt::format("file://{}", native.get_ptr());
		}

		return rp;
	}

	uint32_t JsFbMetadbHandle::get_SubSong()
	{
		return m_handle->get_subsong_index();
	}
}
