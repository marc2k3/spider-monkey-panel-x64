#include <stdafx.h>
#include "js_gc.h"
#include "js_error_helper.h"

#include <fb2k/advanced_config.h>

namespace
{
	constexpr uint32_t kDefaultHeapMaxMb = 1024u * 1024u * 1024u * 2u;
	constexpr uint32_t kDefaultHeapThresholdMb = 50u * 1024u * 1024u;
	constexpr uint32_t kHighFreqTimeLimitMs = 1000u;
	constexpr uint32_t kHighFreqBudgetMultiplier = 2u;
	constexpr uint32_t kHighFreqHeapGrowthMultiplier = 2u;

	enum class CustomGcReason
	{
		kIncrementalNormal = JS::GCReason::FIRST_FIREFOX_REASON,
		kIncrementalFull,
		kNonIncremental
	};

	constexpr JS::GCReason GetGcReason(CustomGcReason customReason)
	{
		return static_cast<JS::GCReason>(customReason);
	}
}

namespace mozjs
{
	uint32_t JsGc::GetMaxHeap()
	{
		UpdateGcConfig();

		return static_cast<uint32_t>(config::advanced::gc_max_heap.get());
	}

	uint64_t JsGc::GetTotalHeapUsageForGlobal(JSContext*, JS::HandleObject jsGlobal)
	{
		auto pJsRealm = static_cast<JsRealmInner*>(JS::GetRealmPrivate(js::GetNonCCWObjectRealm(jsGlobal)));
		return pJsRealm->GetCurrentHeapBytes();
	}

	uint64_t JsGc::GetTotalHeapUsage() const
	{
		return lastTotalHeapSize_;
	}

	void JsGc::Initialize(JSContext* pJsCtx)
	{
		pJsCtx_ = pJsCtx;

		UpdateGcConfig();

		maxHeapSize_ = static_cast<uint32_t>(config::advanced::gc_max_heap.get());
		heapGrowthRateTrigger_ = static_cast<uint32_t>(config::advanced::gc_max_heap_growth.get());
		gcSliceTimeBudget_ = static_cast<uint32_t>(config::advanced::gc_budget.get());
		gcCheckDelay_ = static_cast<uint32_t>(config::advanced::gc_delay.get());
		allocCountTrigger_ = static_cast<uint32_t>(config::advanced::gc_max_alloc_increase.get());

		JS_SetGCParameter(pJsCtx_, JSGC_INCREMENTAL_GC_ENABLED, 1);
		// The following two parameters are not used, since we are doing everything manually.
		// Left here mostly for future-proofing.
		JS_SetGCParameter(pJsCtx_, JSGC_SLICE_TIME_BUDGET_MS, gcSliceTimeBudget_);
		JS_SetGCParameter(pJsCtx_, JSGC_HIGH_FREQUENCY_TIME_LIMIT, kHighFreqTimeLimitMs);
	}

	void JsGc::Finalize()
	{
		PerformNormalGc();

		const auto curTime = timeGetTime();

		isHighFrequency_ = false;
		lastGcCheckTime_ = curTime;
		lastGcTime_ = curTime;
		lastTotalHeapSize_ = 0;
		lastTotalAllocCount_ = 0;
		lastGlobalHeapSize_ = 0;
	}

	bool JsGc::MaybeGc()
	{
		if (!IsTimeToGc())
		{
			return true;
		}

		GcLevel gcLevel = GetRequiredGcLevel();
		if (GcLevel::None == gcLevel)
		{
			return true;
		}

		PerformGc(gcLevel);
		UpdateGcStats();

		return lastTotalHeapSize_ < maxHeapSize_;
	}

	bool JsGc::TriggerGc()
	{
		isManuallyTriggered_ = true;
		return MaybeGc();
	}

	void JsGc::UpdateGcConfig()
	{
		MEMORYSTATUSEX statex{};
		statex.dwLength = sizeof(statex);
		const auto bRet = GlobalMemoryStatusEx(&statex);
		smp::CheckWinApi(bRet == TRUE, "GlobalMemoryStatusEx");


		if (config::advanced::gc_max_heap.get() == ZERO)
		{
			// detect settings automatically
			config::advanced::gc_max_heap = std::min<uint64_t>(statex.ullTotalPhys / 4, kDefaultHeapMaxMb);
		}
		else if (config::advanced::gc_max_heap.get() > statex.ullTotalPhys)
		{
			config::advanced::gc_max_heap = statex.ullTotalPhys;
		}

		if (config::advanced::gc_max_heap_growth.get() == ZERO)
		{
			// detect settings automatically
			config::advanced::gc_max_heap_growth = std::min<uint64_t>(config::advanced::gc_max_heap.get() / 8, kDefaultHeapThresholdMb);
		}
		else if (config::advanced::gc_max_heap_growth.get() > config::advanced::gc_max_heap.get() / 2)
		{
			config::advanced::gc_max_heap_growth = config::advanced::gc_max_heap.get() / 2;
		}
	}

	bool JsGc::IsTimeToGc()
	{
		const auto curTime = timeGetTime();
		if ((curTime - lastGcCheckTime_) < gcCheckDelay_)
		{
			return false;
		}

		lastGcCheckTime_ = curTime;
		return true;
	}

	JsGc::GcLevel JsGc::GetRequiredGcLevel()
	{
		if (GcLevel gcLevel = GetGcLevelFromHeapSize(); gcLevel > GcLevel::None)
		{
			// heap trigger always has the highest priority
			return gcLevel;
		}
		else if (JS::IsIncrementalGCInProgress(pJsCtx_) || isManuallyTriggered_ || GetGcLevelFromAllocCount() > GcLevel::None)
		{
			// currently alloc trigger can be at most `GcLevel::Incremental`
			isManuallyTriggered_ = false; // reset trigger
			return GcLevel::Incremental;
		}
		else
		{
			return GcLevel::None;
		}
	}

	JsGc::GcLevel JsGc::GetGcLevelFromHeapSize()
	{
		auto curTotalHeapSize = GetCurrentTotalHeapSize();

		if (lastTotalHeapSize_ == ZERO || lastTotalHeapSize_ > curTotalHeapSize)
		{
			lastTotalHeapSize_ = curTotalHeapSize;
		}

		const auto maxHeapGrowthRate = isHighFrequency_
			? kHighFreqHeapGrowthMultiplier * heapGrowthRateTrigger_
			: heapGrowthRateTrigger_;

		if (curTotalHeapSize <= lastTotalHeapSize_ + maxHeapGrowthRate)
		{
			return GcLevel::None;
		}
		else if (curTotalHeapSize <= maxHeapSize_ * 0.75)
		{
			return GcLevel::Incremental;
		}
		else if (curTotalHeapSize <= maxHeapSize_ * 0.9)
		{
			return GcLevel::Normal;
		}
		else
		{
			return GcLevel::Full;
		}
	}

	JsGc::GcLevel JsGc::GetGcLevelFromAllocCount()
	{
		uint64_t curTotalAllocCount = GetCurrentTotalAllocCount();
		if (lastTotalAllocCount_ == ZERO || lastTotalAllocCount_ > curTotalAllocCount)
		{
			lastTotalAllocCount_ = curTotalAllocCount;
		}

		if (curTotalAllocCount <= lastTotalAllocCount_ + allocCountTrigger_)
		{
			return GcLevel::None;
		}
		else
		{
			return GcLevel::Incremental;
		}
		// Note: check all method invocations when adding new GcLevel,
		// since currently it's assumed that method returns `GcLevel::Incremental` at most
	}

	void JsGc::UpdateGcStats()
	{
		if (JS::IsIncrementalGCInProgress(pJsCtx_))
		{
			// update only after current gc cycle is finished
			return;
		}

		lastGlobalHeapSize_ = JS_GetGCParameter(pJsCtx_, JSGC_BYTES);
		lastTotalHeapSize_ = GetCurrentTotalHeapSize();
		lastTotalAllocCount_ = GetCurrentTotalAllocCount();

		const auto curTime = timeGetTime();

		isHighFrequency_ = lastGcTime_
			? curTime < (lastGcTime_ + kHighFreqTimeLimitMs)
			: false;

		lastGcTime_ = curTime;
	}

	uint64_t JsGc::GetCurrentTotalHeapSize()
	{
		uint64_t curTotalHeapSize = JS_GetGCParameter(pJsCtx_, JSGC_BYTES);

		JS::IterateRealms(pJsCtx_, &curTotalHeapSize, [](JSContext*, void* data, JS::Realm* pJsRealm, const JS::AutoRequireNoGC& /*nogc*/)
			{
				auto pCurTotalHeapSize = static_cast<uint64_t*>(data);
				auto pNativeRealm = static_cast<JsRealmInner*>(JS::GetRealmPrivate(pJsRealm));

				if (pNativeRealm)
				{
					*pCurTotalHeapSize += pNativeRealm->GetCurrentHeapBytes();
				}
			});

		return curTotalHeapSize;
	}

	uint64_t JsGc::GetCurrentTotalAllocCount()
	{
		uint64_t curTotalAllocCount{};

		JS::IterateRealms(pJsCtx_, &curTotalAllocCount, [](JSContext*, void* data, JS::Realm* pJsRealm, const JS::AutoRequireNoGC& /*nogc*/)
			{
				auto pCurTotalAllocCount = static_cast<uint64_t*>(data);
				auto pNativeRealm = static_cast<JsRealmInner*>(JS::GetRealmPrivate(pJsRealm));
				if (pNativeRealm)
				{
					*pCurTotalAllocCount += pNativeRealm->GetCurrentAllocCount();
				}
			});

		return curTotalAllocCount;
	}

	void JsGc::PerformGc(GcLevel gcLevel)
	{
		if (!JS::IsIncrementalGCInProgress(pJsCtx_))
		{
			PrepareRealmsForGc(gcLevel);
		}

		switch (gcLevel)
		{
		case mozjs::JsGc::GcLevel::Incremental:
			PerformIncrementalGc();
			break;
		case mozjs::JsGc::GcLevel::Normal:
			PerformNormalGc();
			break;
		case mozjs::JsGc::GcLevel::Full:
			PerformFullGc();
			break;
		default:
			break;
		}

		if (!JS::IsIncrementalGCInProgress(pJsCtx_))
		{
			NotifyRealmsOnGcEnd();
		}
	}

	void JsGc::PrepareRealmsForGc(GcLevel gcLevel)
	{
		const auto markAllRealms = [this]
			{
				JS::IterateRealms(pJsCtx_, nullptr, [](JSContext*, void*, JS::Realm* pJsRealm, const JS::AutoRequireNoGC& nogc)
					{
						auto pNativeRealm = static_cast<JsRealmInner*>(JS::GetRealmPrivate(pJsRealm));
						if (pNativeRealm)
						{
							pNativeRealm->OnGcStart();
						}
					});
			};

		switch (gcLevel)
		{
		case mozjs::JsGc::GcLevel::Incremental:
		{
			struct TriggerData
			{
				uint32_t heapGrowthRateTrigger;
				uint32_t allocCountTrigger;
			};
			TriggerData triggers{
				(isHighFrequency_ ? kHighFreqHeapGrowthMultiplier * heapGrowthRateTrigger_ : heapGrowthRateTrigger_) / 2,
				allocCountTrigger_ / 2
			};

			if (uint64_t curGlobalHeapSize = JS_GetGCParameter(pJsCtx_, JSGC_BYTES); curGlobalHeapSize > (lastGlobalHeapSize_ + triggers.heapGrowthRateTrigger))
			{
				// mark all, since we don't have any per-realm information about allocated native JS objects
				markAllRealms();
			}
			else
			{
				JS::IterateRealms(pJsCtx_, &triggers, [](JSContext*, void* data, JS::Realm* pJsRealm, const JS::AutoRequireNoGC& nogc)
					{
						auto pNativeRealm = static_cast<JsRealmInner*>(JS::GetRealmPrivate(pJsRealm));
						if (!pNativeRealm)
						{
							return;
						}

						const TriggerData& pTriggerData = *reinterpret_cast<const TriggerData*>(data);
						const bool hasHeapOvergrowth = pNativeRealm->GetCurrentHeapBytes() > (pNativeRealm->GetLastHeapBytes() + pTriggerData.heapGrowthRateTrigger);
						const bool hasOveralloc = pNativeRealm->GetCurrentAllocCount() > (pNativeRealm->GetLastAllocCount() + pTriggerData.allocCountTrigger);

						if (hasHeapOvergrowth || hasOveralloc || pNativeRealm->IsMarkedForDeletion())
						{
							pNativeRealm->OnGcStart();
						}
					});
			}

			break;
		}
		case mozjs::JsGc::GcLevel::Normal:
		case mozjs::JsGc::GcLevel::Full:
		{
			markAllRealms();
			break;
		}
		default:
			break;
		}
	}

	void JsGc::PerformIncrementalGc()
	{
		const js::SliceBudget sliceBudget{
			js::TimeBudget{
				static_cast<int64_t>(isHighFrequency_ ? kHighFreqBudgetMultiplier * gcSliceTimeBudget_ : gcSliceTimeBudget_) }
		};

		if (!JS::IsIncrementalGCInProgress(pJsCtx_))
		{
			std::vector<JS::Realm*> realms;

			JS::IterateRealms(pJsCtx_, &realms, [](JSContext*, void* data, JS::Realm* pJsRealm, const JS::AutoRequireNoGC& nogc)
				{
					auto pRealms = static_cast<std::vector<JS::Realm*>*>(data);
					auto pNativeRealm = static_cast<JsRealmInner*>(JS::GetRealmPrivate(pJsRealm));

					if (pNativeRealm && pNativeRealm->IsMarkedForGc())
					{
						pRealms->push_back(pJsRealm);
					}
				});

			if (realms.empty())
			{
				JS::PrepareForFullGC(pJsCtx_);
			}
			else
			{
				for (auto pRealm : realms)
				{
					JS::PrepareZoneForGC(pJsCtx_, js::GetRealmZone(pRealm));
				}
			}

			JS::StartIncrementalGC(pJsCtx_, JS::GCOptions::Normal, GetGcReason(CustomGcReason::kIncrementalNormal), sliceBudget);
		}
		else
		{
			JS::PrepareForIncrementalGC(pJsCtx_);
			JS::IncrementalGCSlice(pJsCtx_, GetGcReason(CustomGcReason::kIncrementalNormal), sliceBudget);
		}
	}

	void JsGc::PerformNormalGc()
	{
		if (JS::IsIncrementalGCInProgress(pJsCtx_))
		{
			JS::PrepareForIncrementalGC(pJsCtx_);
			JS::FinishIncrementalGC(pJsCtx_, GetGcReason(CustomGcReason::kIncrementalNormal));
		}

		JS_GC(pJsCtx_);
	}

	void JsGc::PerformFullGc()
	{
		if (JS::IsIncrementalGCInProgress(pJsCtx_))
		{
			JS::PrepareForIncrementalGC(pJsCtx_);
			JS::FinishIncrementalGC(pJsCtx_, JS::GCReason::RESERVED7);
		}

		JS_SetGCParameter(pJsCtx_, JSGC_INCREMENTAL_GC_ENABLED, 0);
		JS::PrepareForFullGC(pJsCtx_);
		JS::NonIncrementalGC(pJsCtx_, JS::GCOptions::Shrink, GetGcReason(CustomGcReason::kNonIncremental));
		JS_SetGCParameter(pJsCtx_, JSGC_INCREMENTAL_GC_ENABLED, 1);
	}

	void JsGc::NotifyRealmsOnGcEnd()
	{
		JS::IterateRealms(pJsCtx_, nullptr, [](JSContext*, void*, JS::Realm* pJsRealm, const JS::AutoRequireNoGC& nogc)
			{
				auto pNativeRealm = static_cast<JsRealmInner*>(JS::GetRealmPrivate(pJsRealm));

				if (pNativeRealm && pNativeRealm->IsMarkedForGc())
				{
					pNativeRealm->OnGcDone();
				}
			});
	}
}
