#pragma once

namespace mozjs
{
	class JsFbFileInfo : public JsObjectBase<JsFbFileInfo>
	{
	public:
		~JsFbFileInfo() override = default;

		DEFINE_JS_INTERFACE_VARS

		static std::unique_ptr<JsFbFileInfo> CreateNative(JSContext* cx, metadb_info_container::ptr containerInfo);
		uint32_t GetInternalSize();

	public:
		int32_t InfoFind(const std::string& name);
		std::string InfoName(uint32_t index);
		std::string InfoValue(uint32_t index);
		int32_t MetaFind(const std::string& name);
		std::string MetaName(uint32_t index);
		std::string MetaValue(uint32_t index, uint32_t valueIndex);
		uint32_t MetaValueCount(uint32_t index);

	public:
		uint32_t get_InfoCount();
		uint32_t get_MetaCount();

	private:
		JsFbFileInfo(JSContext* cx, metadb_info_container::ptr containerInfo);

	private:
		[[maybe_unused]] JSContext* pJsCtx_ = nullptr;
		metadb_info_container::ptr containerInfo_;
		const file_info& fileInfo_;
	};
}
