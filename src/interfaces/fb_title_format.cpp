#include "PCH.hpp"
#include "fb_title_format.h"
#include "fb_metadb_handle.h"
#include "fb_metadb_handle_list.h"

namespace
{
	using namespace mozjs;

	DEFINE_JS_CLASS_OPS(JsFbTitleFormat::FinalizeJsObject)

	DEFINE_JS_CLASS_NO_PROPERTIES("FbTitleFormat")

	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(Eval, JsFbTitleFormat::Eval, JsFbTitleFormat::EvalWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(EvalWithMetadb, JsFbTitleFormat::EvalWithMetadb, JsFbTitleFormat::EvalWithMetadbWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE(EvalWithMetadbs, JsFbTitleFormat::EvalWithMetadbs)

	constexpr auto jsFunctions = std::to_array<JSFunctionSpec>(
		{
			JS_FN("Eval", Eval, 0, kDefaultPropsFlags),
			JS_FN("EvalWithMetadb", EvalWithMetadb, 2, kDefaultPropsFlags),
			JS_FN("EvalWithMetadbs", EvalWithMetadbs, 1, kDefaultPropsFlags),
			JS_FS_END,
		});

	MJS_DEFINE_JS_FN_FROM_NATIVE(FbTitleFormat_Constructor, JsFbTitleFormat::Constructor)
}

namespace mozjs
{
	const JSClass JsFbTitleFormat::JsClass = jsClass;
	const JSFunctionSpec* JsFbTitleFormat::JsFunctions = jsFunctions.data();
	const JSPropertySpec* JsFbTitleFormat::JsProperties = jsProperties.data();
	const JsPrototypeId JsFbTitleFormat::PrototypeId = JsPrototypeId::FbTitleFormat;
	const JSNative JsFbTitleFormat::JsConstructor = ::FbTitleFormat_Constructor;

	JsFbTitleFormat::JsFbTitleFormat(JSContext* ctx, const std::string& expr) : m_ctx(ctx)
	{
		titleformat_compiler::get()->compile_safe(m_obj, expr.c_str());
	}

	std::unique_ptr<JsFbTitleFormat> JsFbTitleFormat::CreateNative(JSContext* ctx, const std::string& expr)
	{
		return std::unique_ptr<JsFbTitleFormat>(new JsFbTitleFormat(ctx, expr));
	}

	uint32_t JsFbTitleFormat::GetInternalSize()
	{
		return sizeof(titleformat_object);
	}

	titleformat_object::ptr JsFbTitleFormat::GetTitleFormat()
	{
		return m_obj;
	}

	JSObject* JsFbTitleFormat::Constructor(JSContext* cx, const std::string& expr)
	{
		return JsFbTitleFormat::CreateJs(cx, expr);
	}

	pfc::string8 JsFbTitleFormat::Eval(bool force)
	{
		pfc::string8 text;

		if (fb2k::api::pc->playback_format_title(nullptr, text, m_obj, nullptr, playback_control::display_level_all))
		{
			return text;
		}
		else if (force)
		{
			metadb_handle_ptr handle;

			if (!metadb::g_get_random_handle(handle))
			{
				metadb::get()->handle_create(handle, make_playable_location("file://C:\\________.ogg", 0));
			}

			handle->format_title(nullptr, text, m_obj, nullptr);
		}

		return text;
	}

	pfc::string8 JsFbTitleFormat::EvalWithOpt(size_t optArgCount, bool force)
	{
		switch (optArgCount)
		{
		case 0:
			return Eval(force);
		case 1:
			return Eval();
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	pfc::string8 JsFbTitleFormat::EvalWithMetadb(JsFbMetadbHandle* handle , bool want_full_info)
	{
		QwrException::ExpectTrue(handle, "handle argument is null");

		const auto& nativeHandle = handle->GetHandle();
		metadb_info_container::ptr info;
		pfc::string8 text;

		if (want_full_info && fb2k::api::is_2_26 && !filesystem::g_is_remote_or_unrecognized(nativeHandle->get_path()))
		{
			try
			{
				info = nativeHandle->get_full_info_ref(fb2k::noAbort);
			}
			catch (...) {}
		}

		if (info.is_valid())
		{
			nativeHandle->format_title_from_external_info(info->info(), nullptr, text, m_obj, nullptr);
		}
		else
		{
			nativeHandle->format_title(nullptr, text, m_obj, nullptr);
		}

		return text;
	}

	pfc::string8 JsFbTitleFormat::EvalWithMetadbWithOpt(size_t optArgCount, JsFbMetadbHandle* handle, bool want_full_info)
	{
		switch (optArgCount)
		{
		case 0:
			return EvalWithMetadb(handle, want_full_info);
		case 1:
			return EvalWithMetadb(handle);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	JS::Value JsFbTitleFormat::EvalWithMetadbs(JsFbMetadbHandleList* handles)
	{
		QwrException::ExpectTrue(handles, "handles argument is null");

		const auto& native = handles->GetHandleList();
		const auto count = native.get_count();
		auto values = pfc_array<pfc::string8>(count);
		auto api = metadb_v2::get();

		api->queryMultiParallel_(native, [&](size_t index, const metadb_v2::rec_t& rec)
			{
				api->formatTitle_v2(native[index], rec, nullptr, values[index], m_obj, nullptr);
			});

		JS::RootedValue jsValue(m_ctx);
		convert::to_js::ToArrayValue(m_ctx, values, &jsValue);
		return jsValue;
	}
}
