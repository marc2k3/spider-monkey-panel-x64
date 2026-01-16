#include "PCH.hpp"
#include "global_heap_manager.h"

namespace mozjs
{
	GlobalHeapManager::GlobalHeapManager(JSContext* ctx) : m_ctx(ctx) {}

	std::unique_ptr<GlobalHeapManager> GlobalHeapManager::Create(JSContext* ctx)
	{
		return std::unique_ptr<GlobalHeapManager>(new GlobalHeapManager(ctx));
	}

	void GlobalHeapManager::RegisterUser(IHeapUser* heapUser)
	{
		std::scoped_lock sl(m_users_lock);
		m_users.emplace(heapUser, heapUser);
	}

	void GlobalHeapManager::UnregisterUser(IHeapUser* heapUser)
	{
		std::scoped_lock sl(m_users_lock);
		m_users.erase(heapUser);
	}

	uint32_t GlobalHeapManager::Store(JS::HandleValue valueToStore)
	{
		std::scoped_lock sl(m_elements_lock);

		if (!m_unused_elements.empty())
		{
			m_unused_elements.clear();
		}

		while (m_elements.contains(m_current_id))
		{
			++m_current_id;
		}

		m_elements.emplace(m_current_id, std::make_unique<HeapElement>(valueToStore));
		return m_current_id++;
	}

	uint32_t GlobalHeapManager::Store(JS::HandleObject valueToStore)
	{
		JS::RootedValue jsValue(m_ctx, JS::ObjectValue(*valueToStore));
		return Store(jsValue);
	}

	uint32_t GlobalHeapManager::Store(JS::HandleFunction valueToStore)
	{
		JS::RootedObject jsObject(m_ctx, JS_GetFunctionObject(valueToStore));
		return Store(jsObject);
	}

	JS::Heap<JS::Value>& GlobalHeapManager::Get(uint32_t id)
	{
		std::scoped_lock sl(m_elements_lock);

		if (!m_unused_elements.empty())
		{
			m_unused_elements.clear();
		}

		return *m_elements[id];
	}

	void GlobalHeapManager::Remove(uint32_t id)
	{
		std::scoped_lock sl(m_elements_lock);

		m_unused_elements.emplace_back(std::move(m_elements[id]));
		m_elements.erase(id);
	}

	void GlobalHeapManager::Trace(JSTracer* trc)
	{
		std::scoped_lock sl(m_elements_lock);

		for (auto& [id, heapElement] : m_elements)
		{
			JS::TraceEdge(trc, heapElement.get(), "CustomHeap_Global");
		}

		for (auto& heapElement : m_unused_elements)
		{
			JS::TraceEdge(trc, heapElement.get(), "CustomHeap_Global");
		}
	}

	void GlobalHeapManager::PrepareForGc()
	{
		std::scoped_lock sl(m_users_lock);

		for (auto& [id, heapUser] : m_users)
		{
			heapUser->PrepareForGlobalGc();
		}

		m_unused_elements.clear();
		m_elements.clear();
	}
}
