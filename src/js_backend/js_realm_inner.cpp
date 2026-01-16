#include "PCH.hpp"
#include "js_realm_inner.h"

namespace mozjs
{
	void JsRealmInner::OnGcStart()
	{
		m_is_marked_for_gc = true;
	}

	void JsRealmInner::OnGcDone()
	{
		std::scoped_lock sl(m_lock);

		m_lastHeapSize = m_curHeapSize;
		m_lastAllocCount = m_curAllocCount;
		m_is_marked_for_gc = false;
	}

	bool JsRealmInner::IsMarkedForGc() const
	{
		return m_is_marked_for_gc;
	}

	uint64_t JsRealmInner::GetCurrentHeapBytes() const
	{
		std::scoped_lock sl(m_lock);
		return m_curHeapSize;
	}

	uint64_t JsRealmInner::GetLastHeapBytes() const
	{
		std::scoped_lock sl(m_lock);
		return std::min(m_lastHeapSize, m_curHeapSize);
	}

	uint32_t JsRealmInner::GetCurrentAllocCount() const
	{
		std::scoped_lock sl(m_lock);
		return m_curAllocCount;
	}

	uint32_t JsRealmInner::GetLastAllocCount() const
	{
		std::scoped_lock sl(m_lock);
		return std::min(m_lastAllocCount, m_curAllocCount);
	}

	void JsRealmInner::OnHeapAllocate(uint32_t size)
	{
		std::scoped_lock sl(m_lock);
		m_curHeapSize += size;
		++m_curAllocCount;
	}

	void JsRealmInner::OnHeapDeallocate(uint32_t size)
	{
		std::scoped_lock sl(m_lock);

		if (size > m_curHeapSize)
		{
			m_curHeapSize = 0;
		}
		else
		{
			m_curHeapSize -= size;
		}

		if (m_curAllocCount)
		{
			--m_curAllocCount;
		}
	}

	void JsRealmInner::MarkForDeletion()
	{
		m_is_marked_for_deletion = true;
	}

	bool JsRealmInner::IsMarkedForDeletion() const
	{
		return m_is_marked_for_deletion;
	}
}
