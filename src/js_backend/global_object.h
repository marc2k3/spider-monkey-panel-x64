#pragma once
#include "global_heap_manager.h"
#include "object_base.h"

namespace mozjs
{
	class JsContainer;
	class Window;

	class JsGlobalObject
	{
	public:
		static constexpr bool HasProto = false;
		static constexpr bool HasProxy = false;
		static constexpr bool HasPostCreate = false;

		static const JSClass& JsClass;

	public:
		// @remark No need to cleanup JS here, since it must be performed manually beforehand anyway
		~JsGlobalObject() = default;

		static JSObject* CreateNative(JSContext* ctx, JsContainer& parentContainer);
		static JsGlobalObject* ExtractNative(JSContext* ctx, JS::HandleObject jsObject);

	public:
		void Fail(const std::string& errorText);

		[[nodiscard]] GlobalHeapManager& GetHeapManager() const;

		static void PrepareForGc(JSContext* ctx, JS::HandleObject self);

	public: // methods
		/// @remark HWND might be null, if called before fb2k initialization is completed
		[[nodiscard]] HWND GetPanelHwnd() const;

		void ClearInterval(uint32_t intervalId);
		void ClearTimeout(uint32_t timeoutId);

		void IncludeScript(const std::wstring& path, JS::HandleValue options = JS::UndefinedHandleValue);
		void IncludeScriptWithOpt(size_t optArgCount, const std::wstring& path, JS::HandleValue options);
		uint32_t SetInterval(JS::HandleValue func, uint32_t delay, JS::HandleValueArray funcArgs = JS::HandleValueArray{ JS::UndefinedHandleValue });
		uint32_t SetIntervalWithOpt(size_t optArgCount, JS::HandleValue func, uint32_t delay, JS::HandleValueArray funcArgs);
		uint32_t SetTimeout(JS::HandleValue func, uint32_t delay, JS::HandleValueArray funcArgs);
		uint32_t SetTimeoutWithOpt(size_t optArgCount, JS::HandleValue func, uint32_t delay, JS::HandleValueArray funcArgs);

	private:
		JsGlobalObject(JSContext* ctx, JsContainer& parentContainer, Window* pWindow);

		struct IncludeOptions
		{
			bool alwaysEvaluate = false;
		};

		IncludeOptions ParseIncludeOptions(JS::HandleValue options);

		template <typename T>
		static T* GetNativeObjectProperty(JSContext* ctx, JS::HandleObject self, const std::string& propName)
		{
			JS::RootedValue jsPropertyValue(ctx);
			if (JS_GetProperty(ctx, self, propName.data(), &jsPropertyValue) && jsPropertyValue.isObject())
			{
				JS::RootedObject jsProperty(ctx, &jsPropertyValue.toObject());
				return JsObjectBase<T>::ExtractNative(ctx, jsProperty);
			}

			return nullptr;
		}

		template <typename T>
		static void CleanupObjectProperty(JSContext* ctx, JS::HandleObject self, const std::string& propName)
		{
			auto pNative = GetNativeObjectProperty<T>(ctx, self, propName);
			if (pNative)
			{
				pNative->PrepareForGc();
			}
		}

		static void Trace(JSTracer* trc, JSObject* obj);

	private:
		JSContext* m_ctx{};
		JsContainer& m_parent_container;
		Window* m_window{};

		std::unordered_set<std::wstring> m_included_files;
		std::unique_ptr<GlobalHeapManager> m_heap_manager;
	};

	static HWND GetPanelHwndForCurrentGlobal(JSContext* ctx)
	{
		JS::RootedObject jsGlobal(ctx, JS::CurrentGlobalOrNull(ctx));
		const auto pNativeGlobal = JsGlobalObject::ExtractNative(ctx, jsGlobal);
		return pNativeGlobal->GetPanelHwnd();
	}
}
