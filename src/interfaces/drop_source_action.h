#pragma once
#include <Panel/drag_action_params.h>

namespace mozjs
{
	class JsDropSourceAction : public JsObjectBase<JsDropSourceAction>
	{
	public:
		~JsDropSourceAction() override = default;

		DEFINE_JS_INTERFACE_VARS

		static std::unique_ptr<JsDropSourceAction> CreateNative(JSContext* cx);
		uint32_t GetInternalSize();

	public:
		DragActionParams& AccessDropActionParams();

	public:
		uint32_t get_Effect() const;
		void put_Base(uint32_t base);
		void put_Effect(uint32_t effect);
		void put_Playlist(int32_t id);
		void put_Text(const std::wstring& text);
		void put_ToSelect(bool toSelect);
		bool get_IsInternal() const;

	private:
		JsDropSourceAction(JSContext* cx);

	private:
		[[maybe_unused]] JSContext* pJsCtx_ = nullptr;

		DragActionParams actionParams_;
	};
}
