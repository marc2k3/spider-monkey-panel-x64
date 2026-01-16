#pragma once

namespace mozjs
{
	class JsAsyncTask : public IHeapUser
	{
	public:
		JsAsyncTask() = default;
		~JsAsyncTask() override = default;

		virtual bool InvokeJs() = 0;
		virtual bool IsCanceled() const = 0;
	};

	template <typename... Args>
	class JsAsyncTaskImpl : public JsAsyncTask
	{
		static_assert((std::is_same_v<Args, JS::HandleValue> && ...));

	public:
		/// throws JsException
		JsAsyncTaskImpl(JSContext* ctx, Args... args) : m_ctx(ctx)
		{
			JS::RootedObject jsGlobal(ctx, JS::CurrentGlobalOrNull(ctx));
			m_native_global = JsGlobalObject::ExtractNative(ctx, jsGlobal);
			m_heap_ids = { m_native_global->GetHeapManager().Store(args)... };
			m_native_global->GetHeapManager().RegisterUser(this);
			m_js_available = true;
		}

		~JsAsyncTaskImpl() override
		{
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
		};

		/// @details Assumes that JS environment is ready (global, realm and etc).
		bool InvokeJs() final
		{
			std::scoped_lock sl(m_lock);

			if (!m_js_available)
			{
				return true;
			}

			JS::RootedObject jsGlobal(m_ctx, JS::CurrentGlobalOrNull(m_ctx));
			return InvokeJsInternal(jsGlobal, std::make_index_sequence<sizeof...(Args)>{});
		}

		void PrepareForGlobalGc() final
		{
			std::scoped_lock sl(m_lock);
			m_js_available = false;
		}

		bool IsCanceled() const final
		{
			std::scoped_lock sl(m_lock);
			return m_js_available;
		}

	private:
		virtual bool InvokeJsImpl(JSContext* ctx, JS::HandleObject jsGlobal, Args... args) = 0;

		template <size_t... Indices>
		bool InvokeJsInternal(JS::HandleObject jsGlobal, std::index_sequence<Indices...>)
		{
			return InvokeJsImpl(m_ctx, jsGlobal, JS::RootedValue{ m_ctx, m_native_global->GetHeapManager().Get(std::get<Indices>(m_heap_ids)) }...);
		}

	private:
		JSContext* m_ctx{};
		std::array<uint32_t, sizeof...(Args)> m_heap_ids{};
		JsGlobalObject* m_native_global{};
		mutable std::mutex m_lock;
		bool m_js_available{};
	};
}
