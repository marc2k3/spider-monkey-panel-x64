#pragma once
#include "global_heap_manager.h"

namespace mozjs
{
	class HeapHelper : public IHeapUser
	{
	public:
		HeapHelper(JSContext* ctx) : m_ctx(ctx)
		{
			JS::RootedObject jsGlobal(ctx, JS::CurrentGlobalOrNull(ctx));
			m_native_global = JsGlobalObject::ExtractNative(ctx, jsGlobal);
			m_native_global->GetHeapManager().RegisterUser(this);
			m_js_available = true;
		}

		~HeapHelper() override
		{
			Finalize();
		};

		template <typename T>
		[[nodiscard]] uint32_t Store(const T& jsObject)
		{
			return m_heap_ids.emplace_back(m_native_global->GetHeapManager().Store(jsObject));
		}

		[[nodiscard]] JS::Heap<JS::Value>& Get(uint32_t objectId)
		{
			return m_native_global->GetHeapManager().Get(objectId);
		}

		bool IsJsAvailable() const
		{
			return m_js_available;
		}

		void Finalize()
		{
			// might be called from worker thread
			std::scoped_lock sl(m_lock);

			if (!m_js_available)
			{
				return;
			}

			for (auto heapId : m_heap_ids)
			{
				m_native_global->GetHeapManager().Remove(heapId);
			}

			m_native_global->GetHeapManager().UnregisterUser(this);
			m_js_available = false;
		}

		void PrepareForGlobalGc() final
		{
			std::scoped_lock sl(m_lock);
			m_js_available = false;
		}

	private:
		JSContext* m_ctx{};
		std::vector<uint32_t> m_heap_ids;
		JsGlobalObject* m_native_global{};
		mutable std::mutex m_lock;
		bool m_js_available{};
	};
}
