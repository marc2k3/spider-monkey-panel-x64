#include "PCH.hpp"
#include "js_art_helpers.h"

#include "js_async_task.h"

#include <FB2K/AlbumArtStatic.hpp>
#include <interfaces/gdi_bitmap.h>
#include <utils/thread_pool.h>

using namespace smp;

namespace
{
	using namespace mozjs;

	struct JsAlbumArtTask : public JsAsyncTaskImpl<JS::HandleValue>
	{
		JsAlbumArtTask(JSContext* ctx, JS::HandleValue jsPromise);
		~JsAlbumArtTask() override = default;

		bool InvokeJsImpl(JSContext* ctx, JS::HandleObject jsGlobal, JS::HandleValue jsPromiseValue) override;

		std::unique_ptr<Gdiplus::Bitmap> m_image;
		std::string m_path;
	};

	class AlbumArtV2FetchTask
	{
	public:
		AlbumArtV2FetchTask(
			JSContext* ctx,
			JS::HandleObject jsPromise,
			HWND wnd,
			const metadb_handle_ptr& handle,
			uint32_t art_id,
			bool want_stub,
			bool only_embed
		);

		/// @details Executed off main thread
		~AlbumArtV2FetchTask() = default;

		AlbumArtV2FetchTask(const AlbumArtV2FetchTask&) = delete;
		AlbumArtV2FetchTask& operator=(const AlbumArtV2FetchTask&) = delete;

		/// @details Executed off main thread
		void operator()();

	private:
		HWND m_wnd;
		metadb_handle_ptr m_handle;
		uint32_t m_art_id{};
		bool m_want_stub{}, m_only_embed{};

		std::shared_ptr<JsAlbumArtTask> m_task;
	};

	AlbumArtV2FetchTask::AlbumArtV2FetchTask(
		JSContext* ctx,
		JS::HandleObject jsPromise,
		HWND wnd,
		const metadb_handle_ptr& handle,
		uint32_t art_id,
		bool want_stub,
		bool only_embed)
		: m_wnd(wnd)
		, m_handle(handle)
		, m_art_id(art_id)
		, m_want_stub(want_stub)
		, m_only_embed(only_embed)
	{
		JS::RootedValue jsPromiseValue(ctx, JS::ObjectValue(*jsPromise));
		m_task = std::make_unique<JsAlbumArtTask>(ctx, jsPromiseValue);
	}

	void AlbumArtV2FetchTask::operator()()
	{
		if (!m_task->IsCanceled())
		{ // the task still might be executed and posted, since we don't block here
			return;
		}

		auto data = AlbumArtStatic::get(m_handle, m_art_id, m_want_stub, m_only_embed, m_task->m_path);
		auto bitmap = AlbumArtStatic::to_bitmap(data);
		m_task->m_image = std::move(bitmap);
		EventDispatcher::Get().PutEvent(m_wnd, std::make_unique<Event_JsTask>(EventId::kInternalGetAlbumArtPromiseDone, m_task));
	}

	JsAlbumArtTask::JsAlbumArtTask(JSContext* ctx, JS::HandleValue jsPromise) : JsAsyncTaskImpl(ctx, jsPromise) {}

	bool JsAlbumArtTask::InvokeJsImpl(JSContext* ctx, JS::HandleObject, JS::HandleValue jsPromiseValue)
	{
		JS::RootedObject jsPromise(ctx, &jsPromiseValue.toObject());

		try
		{
			JS::RootedValue jsBitmapValue(ctx);
			if (m_image)
			{
				JS::RootedObject jsBitmap(ctx, JsGdiBitmap::CreateJs(ctx, std::move(m_image)));
				jsBitmapValue = jsBitmap ? JS::ObjectValue(*jsBitmap) : JS::UndefinedValue();
			}

			JS::RootedObject jsResult(ctx, JS_NewPlainObject(ctx));
			if (!jsResult)
			{
				throw JsException();
			}

			AddProperty(ctx, jsResult, "image", JS::HandleValue{ jsBitmapValue });
			AddProperty(ctx, jsResult, "path", m_path);

			JS::RootedValue jsResultValue(ctx, JS::ObjectValue(*jsResult));
			(void)JS::ResolvePromise(ctx, jsPromise, jsResultValue);
		}
		catch (...)
		{
			mozjs::ExceptionToJsError(ctx);

			JS::RootedValue jsError(ctx);
			(void)JS_GetPendingException(ctx, &jsError);

			JS::RejectPromise(ctx, jsPromise, jsError);
		}

		return true;
	}
}

namespace mozjs
{
	JSObject* GetAlbumArtPromise(JSContext* ctx, HWND hWnd, const metadb_handle_ptr& handle, uint32_t art_id, bool want_stub, bool only_embed)
	{
		JS::RootedObject jsObject(ctx, JS::NewPromiseObject(ctx, nullptr));
		JsException::ExpectTrue(jsObject);

		QwrThreadPool::GetInstance().AddTask([task = std::make_shared<AlbumArtV2FetchTask>(ctx, jsObject, hWnd, handle, art_id, want_stub, only_embed)]
			{
				std::invoke(*task);
			});

		return jsObject;
	}
}
