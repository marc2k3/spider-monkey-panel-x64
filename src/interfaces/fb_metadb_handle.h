#pragma once

namespace mozjs
{
	class JsFbMetadbHandle : public JsObjectBase<JsFbMetadbHandle>
	{
	public:
		~JsFbMetadbHandle() override = default;

		DEFINE_JS_INTERFACE_VARS

		static std::unique_ptr<JsFbMetadbHandle> CreateNative(JSContext* cx, const metadb_handle_ptr& handle);
		uint32_t GetInternalSize();

	public:
		metadb_handle_ptr& GetHandle();

	public: // methods
		void ClearStats();
		bool Compare(JsFbMetadbHandle* handle);
		JSObject* GetFileInfo();
		void RefreshStats();
		void SetFirstPlayed(const pfc::string8& first_played);
		void SetLastPlayed(const pfc::string8& last_played);
		void SetLoved(uint32_t loved);
		void SetPlaycount(uint32_t playcount);
		void SetRating(uint32_t rating);

	public: // props
		int64_t get_FileSize();
		double get_Length();
		std::string get_Path();
		std::string get_RawPath();
		uint32_t get_SubSong();

	private:
		JsFbMetadbHandle(JSContext* cx, const metadb_handle_ptr& handle);

	private:
		JSContext* pJsCtx_ = nullptr;
		metadb_handle_ptr metadbHandle_;
	};
}
