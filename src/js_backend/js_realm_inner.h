#pragma once

namespace mozjs
{
	class JsRealmInner final
	{
	public:
		JsRealmInner() = default;
		~JsRealmInner() = default;
		JsRealmInner(const JsRealmInner&) = delete;
		JsRealmInner& operator=(const JsRealmInner&) = delete;

	public:
		void MarkForDeletion();
		[[nodiscard]] bool IsMarkedForDeletion() const;

		void OnGcStart();
		void OnGcDone();
		[[nodiscard]] bool IsMarkedForGc() const;

		[[nodiscard]] uint64_t GetCurrentHeapBytes() const;
		[[nodiscard]] uint64_t GetLastHeapBytes() const;

		[[nodiscard]] uint32_t GetCurrentAllocCount() const;
		[[nodiscard]] uint32_t GetLastAllocCount() const;

		void OnHeapAllocate(uint32_t size);
		void OnHeapDeallocate(uint32_t size);

	private:
		bool m_is_marked_for_deletion{};
		bool m_is_marked_for_gc{};
		mutable std::mutex m_lock;
		uint64_t m_curHeapSize{};
		uint64_t m_lastHeapSize{};
		uint32_t m_curAllocCount{};
		uint32_t m_lastAllocCount{};
	};
}
