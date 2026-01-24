#pragma once

namespace mozjs
{
	class AutoJsReport
	{
	public:
		[[nodiscard]] explicit AutoJsReport(JSContext* ctx);
		~AutoJsReport() noexcept;

		void Disable();

	private:
		JSContext* m_ctx{};
		bool m_is_disabled{};
	};

	// DO NOT TOUCH
	class JsAutoRealmWithErrorReport
	{
	public:
		[[nodiscard]] JsAutoRealmWithErrorReport(JSContext* ctx, JS::HandleObject global);

		JsAutoRealmWithErrorReport(const JsAutoRealmWithErrorReport&) = delete;
		JsAutoRealmWithErrorReport& operator=(const JsAutoRealmWithErrorReport&) = delete;

		void DisableReport();

	private:
		JSAutoRealm m_ac;
		AutoJsReport m_are;
	};
}
