#pragma once

namespace mozjs
{
	class IHeapUser
	{
	public:
		IHeapUser() = default;
		virtual ~IHeapUser() = default;
		virtual void PrepareForGlobalGc() = 0;
	};

	/// @details Contains a tracer, which is removed only in destructor
	class GlobalHeapManager
	{
	public:
		/// @remark No need to cleanup JS here, since it must be performed manually beforehand anyway
		~GlobalHeapManager() = default;
		GlobalHeapManager(const GlobalHeapManager&) = delete;
		GlobalHeapManager& operator=(const GlobalHeapManager&) = delete;

		[[nodiscard]] static std::unique_ptr<GlobalHeapManager> Create(JSContext* ctx);

	public:
		void RegisterUser(IHeapUser* heapUser);
		void UnregisterUser(IHeapUser* heapUser);

		[[nodiscard]] uint32_t Store(JS::HandleValue valueToStore);
		[[nodiscard]] uint32_t Store(JS::HandleObject valueToStore);
		[[nodiscard]] uint32_t Store(JS::HandleFunction valueToStore);
		[[nodiscard]] JS::Heap<JS::Value>& Get(uint32_t id);
		void Remove(uint32_t id);

		void Trace(JSTracer* trc);
		void PrepareForGc();

	private:
		GlobalHeapManager(JSContext* ctx);

	private:
		using HeapElement = JS::Heap<JS::Value>;

		JSContext* m_ctx{};
		uint32_t m_current_id = 0;
		std::mutex m_elements_lock;
		std::mutex m_users_lock;
		std::unordered_map<uint32_t, std::unique_ptr<HeapElement>> m_elements;
		std::list<std::unique_ptr<HeapElement>> m_unused_elements;
		std::unordered_map<IHeapUser*, IHeapUser*> m_users;
	};
}
