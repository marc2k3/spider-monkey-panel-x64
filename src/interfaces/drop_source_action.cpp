#include <stdafx.h>
#include "drop_source_action.h"

namespace
{
	using namespace mozjs;

	DEFINE_JS_CLASS_OPS(JsDropSourceAction::FinalizeJsObject)

	DEFINE_JS_CLASS("DropSourceAction")

	MJS_DEFINE_JS_FN_FROM_NATIVE(get_Effect, JsDropSourceAction::get_Effect)
	MJS_DEFINE_JS_FN_FROM_NATIVE(put_Base, JsDropSourceAction::put_Base)
	MJS_DEFINE_JS_FN_FROM_NATIVE(put_Effect, JsDropSourceAction::put_Effect)
	MJS_DEFINE_JS_FN_FROM_NATIVE(put_Playlist, JsDropSourceAction::put_Playlist)
	MJS_DEFINE_JS_FN_FROM_NATIVE(put_Text, JsDropSourceAction::put_Text)
	MJS_DEFINE_JS_FN_FROM_NATIVE(put_ToSelect, JsDropSourceAction::put_ToSelect)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_IsInternal, JsDropSourceAction::get_IsInternal)

	constexpr auto jsProperties = std::to_array<JSPropertySpec>(
		{
			JS_PSGS("Base", DummyGetter, put_Base, kDefaultPropsFlags),
			JS_PSGS("Effect", get_Effect, put_Effect, kDefaultPropsFlags),
			JS_PSGS("Playlist", DummyGetter, put_Playlist, kDefaultPropsFlags),
			JS_PSGS("Text", DummyGetter, put_Text, kDefaultPropsFlags),
			JS_PSGS("ToSelect", DummyGetter, put_ToSelect, kDefaultPropsFlags),
			JS_PSG("IsInternal", get_IsInternal, kDefaultPropsFlags),
			JS_PS_END,
		});

	constexpr auto jsFunctions = std::to_array<JSFunctionSpec>({
		JS_FS_END,
	});
}

namespace mozjs
{
	const JSClass JsDropSourceAction::JsClass = jsClass;
	const JSFunctionSpec* JsDropSourceAction::JsFunctions = jsFunctions.data();
	const JSPropertySpec* JsDropSourceAction::JsProperties = jsProperties.data();
	const JsPrototypeId JsDropSourceAction::PrototypeId = JsPrototypeId::DropSourceAction;

	JsDropSourceAction::JsDropSourceAction(JSContext* cx) : pJsCtx_(cx) {}

	std::unique_ptr<mozjs::JsDropSourceAction> JsDropSourceAction::CreateNative(JSContext* cx)
	{
		return std::unique_ptr<JsDropSourceAction>(new JsDropSourceAction(cx));
	}

	uint32_t JsDropSourceAction::GetInternalSize()
	{
		return 0;
	}

	smp::DragActionParams& JsDropSourceAction::AccessDropActionParams()
	{
		return actionParams_;
	}

	uint32_t JsDropSourceAction::get_Effect() const
	{
		return actionParams_.effect;
	}

	void JsDropSourceAction::put_Base(uint32_t base)
	{
		actionParams_.base = base;
	}

	void JsDropSourceAction::put_Effect(uint32_t effect)
	{
		actionParams_.effect = effect;
	}

	void JsDropSourceAction::put_Playlist(int32_t id)
	{
		actionParams_.playlistIdx = id;
	}

	void JsDropSourceAction::put_Text(const std::wstring& text)
	{
		actionParams_.text = text;
	}

	void JsDropSourceAction::put_ToSelect(bool toSelect)
	{
		actionParams_.toSelect = toSelect;
	}

	bool JsDropSourceAction::get_IsInternal() const
	{
		return actionParams_.isInternal;
	}
}
