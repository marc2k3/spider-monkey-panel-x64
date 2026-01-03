#include <PCH.hpp>
#include "fb_metadb_handle.h"
#include "fb_file_info.h"

namespace
{
	using namespace mozjs;

	DEFINE_JS_CLASS_OPS(JsFbMetadbHandle::FinalizeJsObject)

	DEFINE_JS_CLASS("FbMetadbHandle")

	MJS_DEFINE_JS_FN_FROM_NATIVE(Compare, JsFbMetadbHandle::Compare)
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetFileInfo, JsFbMetadbHandle::GetFileInfo)

	constexpr auto jsFunctions = std::to_array<JSFunctionSpec>(
		{
			JS_FN("Compare", Compare, 1, kDefaultPropsFlags),
			JS_FN("GetFileInfo", GetFileInfo, 0, kDefaultPropsFlags),
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

	JsFbMetadbHandle::JsFbMetadbHandle(JSContext* cx, const metadb_handle_ptr& handle) : pJsCtx_(cx), metadbHandle_(handle) {}

	std::unique_ptr<mozjs::JsFbMetadbHandle> JsFbMetadbHandle::CreateNative(JSContext* cx, const metadb_handle_ptr& handle)
	{
		QwrException::ExpectTrue(handle.is_valid(), "Internal error: metadb_handle_ptr is null");

		return std::unique_ptr<JsFbMetadbHandle>(new JsFbMetadbHandle(cx, handle));
	}

	uint32_t JsFbMetadbHandle::GetInternalSize()
	{
		return sizeof(metadb_handle);
	}

	metadb_handle_ptr& JsFbMetadbHandle::GetHandle()
	{
		return metadbHandle_;
	}

	bool JsFbMetadbHandle::Compare(JsFbMetadbHandle* handle)
	{
		QwrException::ExpectTrue(handle, "handle argument is null");

		return (handle->GetHandle() == metadbHandle_);
	}

	JSObject* JsFbMetadbHandle::GetFileInfo()
	{
		auto containerInfo = metadbHandle_->query_v2_().info;

		if (containerInfo.is_empty())
		{
			containerInfo = metadbHandle_->get_info_ref();
		}

		return JsFbFileInfo::CreateJs(pJsCtx_, containerInfo);
	}

	uint64_t JsFbMetadbHandle::get_FileSize()
	{
		return metadbHandle_->get_filesize();
	}

	double JsFbMetadbHandle::get_Length()
	{
		return metadbHandle_->get_length();
	}

	std::string JsFbMetadbHandle::get_Path()
	{
		return filesystem::g_get_native_path(metadbHandle_->get_path()).get_ptr();
	}

	std::string JsFbMetadbHandle::get_RawPath()
	{
		const std::string rp = metadbHandle_->get_path();

		if (rp.starts_with("file-relative://"))
		{
			const auto native = filesystem::g_get_native_path(rp.c_str());
			return fmt::format("file://{}", native.get_ptr());
		}

		return rp;
	}

	uint32_t JsFbMetadbHandle::get_SubSong()
	{
		return metadbHandle_->get_subsong_index();
	}
}
