#pragma once

namespace mozjs
{
	class JsFbMetadbHandle : public JsObjectBase<JsFbMetadbHandle>
	{
	public:
		~JsFbMetadbHandle() override = default;

		DEFINE_JS_INTERFACE_VARS

		static std::unique_ptr<JsFbMetadbHandle> CreateNative(JSContext* ctx, const metadb_handle_ptr& handle);
		uint32_t GetInternalSize();

	public:
		metadb_handle_ptr& GetHandle();

	public: // methods
		bool Compare(JsFbMetadbHandle* handle);
		JSObject* GetFileInfo(bool want_full_info = false);
		JSObject* GetFileInfoWithOpt(size_t optArgCount, bool want_full_info);

	public: // props
		uint64_t get_FileSize();
		double get_Length();
		std::string get_Path();
		std::string get_RawPath();
		uint32_t get_SubSong();

	private:
		JsFbMetadbHandle(JSContext* ctx, const metadb_handle_ptr& handle);

	private:
		JSContext* m_ctx{};
		metadb_handle_ptr m_handle;
	};
}
