#include "PCH.hpp"
#include "fb_file_info.h"

namespace
{
	using namespace mozjs;

	DEFINE_JS_CLASS_OPS(JsFbFileInfo::FinalizeJsObject)

	DEFINE_JS_CLASS("FbFileInfo")

	MJS_DEFINE_JS_FN_FROM_NATIVE(InfoFind, JsFbFileInfo::InfoFind);
	MJS_DEFINE_JS_FN_FROM_NATIVE(InfoName, JsFbFileInfo::InfoName);
	MJS_DEFINE_JS_FN_FROM_NATIVE(InfoValue, JsFbFileInfo::InfoValue);
	MJS_DEFINE_JS_FN_FROM_NATIVE(MetaFind, JsFbFileInfo::MetaFind);
	MJS_DEFINE_JS_FN_FROM_NATIVE(MetaName, JsFbFileInfo::MetaName);
	MJS_DEFINE_JS_FN_FROM_NATIVE(MetaValue, JsFbFileInfo::MetaValue);
	MJS_DEFINE_JS_FN_FROM_NATIVE(MetaValueCount, JsFbFileInfo::MetaValueCount);

	constexpr auto jsFunctions = std::to_array<JSFunctionSpec>(
		{
			JS_FN("InfoFind", InfoFind, 1, kDefaultPropsFlags),
			JS_FN("InfoName", InfoName, 1, kDefaultPropsFlags),
			JS_FN("InfoValue", InfoValue, 1, kDefaultPropsFlags),
			JS_FN("MetaFind", MetaFind, 1, kDefaultPropsFlags),
			JS_FN("MetaName", MetaName, 1, kDefaultPropsFlags),
			JS_FN("MetaValue", MetaValue, 2, kDefaultPropsFlags),
			JS_FN("MetaValueCount", MetaValueCount, 1, kDefaultPropsFlags),
			JS_FS_END,
		});

	MJS_DEFINE_JS_FN_FROM_NATIVE(get_InfoCount, JsFbFileInfo::get_InfoCount);
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_MetaCount, JsFbFileInfo::get_MetaCount);

	constexpr auto jsProperties = std::to_array<JSPropertySpec>(
		{
			JS_PSG("InfoCount", get_InfoCount, kDefaultPropsFlags),
			JS_PSG("MetaCount", get_MetaCount, kDefaultPropsFlags),
			JS_PS_END,
		});
}

namespace mozjs
{
	const JSClass JsFbFileInfo::JsClass = jsClass;
	const JSFunctionSpec* JsFbFileInfo::JsFunctions = jsFunctions.data();
	const JSPropertySpec* JsFbFileInfo::JsProperties = jsProperties.data();
	const JsPrototypeId JsFbFileInfo::PrototypeId = JsPrototypeId::FbFileInfo;

	JsFbFileInfo::JsFbFileInfo(JSContext* ctx, metadb_info_container::ptr info)
		: m_ctx(ctx)
		, m_info(info) {}

	std::unique_ptr<mozjs::JsFbFileInfo> JsFbFileInfo::CreateNative(JSContext* ctx, metadb_info_container::ptr info)
	{
		return std::unique_ptr<JsFbFileInfo>(new JsFbFileInfo(ctx, info));
	}

	uint32_t JsFbFileInfo::GetInternalSize()
	{
		return sizeof(file_info_impl);
	}

	int32_t JsFbFileInfo::InfoFind(const std::string& name)
	{
		const auto idx = m_info->info().info_find_ex(name.c_str(), name.length());
		return to_int(idx);
	}

	std::string JsFbFileInfo::InfoName(uint32_t index)
	{
		QwrException::ExpectTrue(index < m_info->info().info_get_count(), "Index is out of bounds");

		return m_info->info().info_enum_name(index);
	}

	std::string JsFbFileInfo::InfoValue(uint32_t index)
	{
		QwrException::ExpectTrue(index < m_info->info().info_get_count(), "Index is out of bounds");

		return m_info->info().info_enum_value(index);
	}

	int32_t JsFbFileInfo::MetaFind(const std::string& name)
	{
		const auto idx = m_info->info().meta_find_ex(name.c_str(), name.length());
		return to_int(idx);
	}

	std::string JsFbFileInfo::MetaName(uint32_t index)
	{
		QwrException::ExpectTrue(index < m_info->info().meta_get_count(), "Index is out of bounds");

		return m_info->info().meta_enum_name(index);
	}

	std::string JsFbFileInfo::MetaValue(uint32_t infoIndex, uint32_t valueIndex)
	{
		QwrException::ExpectTrue(infoIndex < m_info->info().meta_get_count(), "Index is out of bounds");
		QwrException::ExpectTrue(valueIndex < m_info->info().meta_enum_value_count(infoIndex), "Index is out of bounds");

		return m_info->info().meta_enum_value(infoIndex, valueIndex);
	}

	uint32_t JsFbFileInfo::MetaValueCount(uint32_t index)
	{
		QwrException::ExpectTrue(index < m_info->info().meta_get_count(), "Index is out of bounds");

		return to_uint(m_info->info().meta_enum_value_count(index));
	}

	uint32_t JsFbFileInfo::get_InfoCount()
	{
		return to_uint(m_info->info().info_get_count());
	}

	uint32_t JsFbFileInfo::get_MetaCount()
	{
		return to_uint(m_info->info().meta_get_count());
	}
}
