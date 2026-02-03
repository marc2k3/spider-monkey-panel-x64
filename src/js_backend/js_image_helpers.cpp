#include "PCH.hpp"
#include "js_image_helpers.h"

#include "js_async_task.h"

#include <events/event_dispatcher.h>
#include <interfaces/gdi_bitmap.h>
#include <utils/thread_pool.h>

using namespace mozjs;

namespace
{
	class JsImageTask : public JsAsyncTaskImpl<JS::HandleValue>
	{
	public:
		JsImageTask(JSContext* ctx, JS::HandleValue promise);
		~JsImageTask() override = default;

		bool InvokeJsImpl(JSContext* ctx, JS::HandleObject jsGlobal, JS::HandleValue promise_value) override;

		std::unique_ptr<Gdiplus::Bitmap> m_image;
	};

	class ImageFetchTask
	{
	public:
		ImageFetchTask(JSContext* ctx, JS::HandleObject promise, HWND notify_wnd, const std::wstring& path);

		/// @details Executed off main thread
		~ImageFetchTask() = default;

		ImageFetchTask(const ImageFetchTask&) = delete;
		ImageFetchTask& operator=(const ImageFetchTask&) = delete;

		/// @details Executed off main thread
		void operator()();

	private:
		HWND m_notify_wnd;
		std::wstring m_path;
		std::shared_ptr<JsImageTask> m_task;
	};

	ImageFetchTask::ImageFetchTask(
		JSContext* ctx,
		JS::HandleObject promise,
		HWND notify_wnd,
		const std::wstring& path)
		: m_notify_wnd(notify_wnd)
		, m_path(path)
	{
		JS::RootedValue jsPromiseValue(ctx, JS::ObjectValue(*promise));
		m_task = std::make_unique<JsImageTask>(ctx, jsPromiseValue);
	}

	void ImageFetchTask::operator()()
	{
		if (!m_task->IsCanceled())
		{ // the task still might be executed and posted, since we don't block here
			return;
		}

		auto bitmap = FileHelper(m_path).load_image();
		m_task->m_image = std::move(bitmap);
		smp::EventDispatcher::Get().PutEvent(m_notify_wnd, std::make_unique<smp::Event_JsTask>(smp::EventId::kInternalLoadImagePromiseDone, m_task));
	}

	JsImageTask::JsImageTask(JSContext* ctx, JS::HandleValue promise) : JsAsyncTaskImpl(ctx, promise) {}

	bool JsImageTask::InvokeJsImpl(JSContext* ctx, JS::HandleObject, JS::HandleValue jsPromiseValue)
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

			JS::ResolvePromise(ctx, jsPromise, jsBitmapValue);
		}
		catch (...)
		{
			mozjs::ExceptionToJsError(ctx);
			JS::RootedValue jsError(ctx);
			JS_GetPendingException(ctx, &jsError);
			JS::RejectPromise(ctx, jsPromise, jsError);
		}

		return true;
	}
}

namespace mozjs
{
	JSObject* GetImagePromise(JSContext* ctx, HWND hWnd, const std::wstring& imagePath)
	{
		JS::RootedObject jsObject(ctx, JS::NewPromiseObject(ctx, nullptr));
		JsException::ExpectTrue(jsObject);

		QwrThreadPool::GetInstance().AddTask([task = std::make_shared<ImageFetchTask>(ctx, jsObject, hWnd, imagePath)]
			{
				std::invoke(*task);
			});

		return jsObject;
	}
}
