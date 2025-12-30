#pragma once

namespace mozjs
{
	class AutoJsReport
	{
	public:
		[[nodiscard]] explicit AutoJsReport(JSContext* cx);
		~AutoJsReport() noexcept;

		void Disable();

	private:
		JSContext* cx{};
		bool isDisabled_{};
	};

	// DO NOT TOUCH
	class JsAutoRealmWithErrorReport
	{
	public:
		[[nodiscard]] JsAutoRealmWithErrorReport(JSContext* cx, JS::HandleObject global);

		JsAutoRealmWithErrorReport(const JsAutoRealmWithErrorReport&) = delete;
		JsAutoRealmWithErrorReport& operator=(const JsAutoRealmWithErrorReport&) = delete;

		void DisableReport();

	private:
		JSAutoRealm ac_;
		AutoJsReport are_;
	};
}
