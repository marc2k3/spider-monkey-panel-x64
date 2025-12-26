#pragma once

namespace mozjs
{
	class JsGc final
	{
	public:
		JsGc() = default;
		~JsGc() = default;
		JsGc(const JsGc&) = delete;
		JsGc& operator=(const JsGc&) = delete;

	public:
		/// @throw QwrException
		[[nodiscard]] static uint32_t GetMaxHeap();
		[[nodiscard]] static uint64_t GetTotalHeapUsageForGlobal(JSContext* cx, JS::HandleObject jsGlobal);
		/// @details Returns last heap size instead of the current size,
		/// but this should be good enough for users
		[[nodiscard]] uint64_t GetTotalHeapUsage() const;

		/// @throw QwrException
		void Initialize(JSContext* pJsCtx);
		void Finalize();

		bool MaybeGc();
		// @brief Force gc trigger (e.g. on panel unload)
		bool TriggerGc();

	private:
		enum class GcLevel : uint8_t
		{
			None,
			Incremental,
			Normal,
			Full
		};

		static void UpdateGcConfig();

		// GC stats handling
		[[nodiscard]] bool IsTimeToGc();
		[[nodiscard]] GcLevel GetRequiredGcLevel();
		[[nodiscard]] GcLevel GetGcLevelFromHeapSize();
		[[nodiscard]] GcLevel GetGcLevelFromAllocCount();
		[[nodiscard]] uint64_t GetCurrentTotalHeapSize();
		[[nodiscard]] uint64_t GetCurrentTotalAllocCount();
		void UpdateGcStats();

		// GC implementation
		void PerformGc(GcLevel gcLevel);
		void PerformIncrementalGc();
		void PerformNormalGc();
		void PerformFullGc();
		void PrepareRealmsForGc(GcLevel gcLevel);
		void NotifyRealmsOnGcEnd();

	private:
		static constexpr auto ZERO = uint64_t{};

		JSContext* pJsCtx_{};

		bool isManuallyTriggered_{};
		bool isHighFrequency_{};
		uint32_t lastGcCheckTime_{};
		uint32_t lastGcTime_{};
		uint64_t lastTotalHeapSize_{};
		uint64_t lastTotalAllocCount_{};
		uint64_t lastGlobalHeapSize_{};

		// These values are overwritten by config.
		// Remain here mostly as a reference.
		uint32_t maxHeapSize_ = 1024u * 1024u * 1024u * 2u;
		uint32_t heapGrowthRateTrigger_ = 50u * 1024u * 1024u;
		uint32_t gcSliceTimeBudget_ = 10u;
		uint32_t gcCheckDelay_ = 50u;
		uint32_t allocCountTrigger_ = 50u;
	};
}
