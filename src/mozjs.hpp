#pragma once

__pragma(warning(push))
__pragma(warning(disable : 4244 4251))
#include <jsapi.h>
#include <jsfriendapi.h>

#include <js/Array.h>
#include <js/CompilationAndEvaluation.h>
#include <js/Conversions.h>
#include <js/Date.h>
#include <js/GCHashTable.h>
#include <js/Initialization.h>
#include <js/Promise.h>
#include <js/Proxy.h>
#include <js/SourceText.h>
#include <js/Wrapper.h>
__pragma(warning(pop))

// fwd
namespace mozjs
{
	class JsAsyncTask;
	class JsContainer;
	class JsGlobalObject;
}

#include <js_backend/global_object.h>
#include <js_backend/js_object_helper.h>

#include <js_backend/js_container.h>
#include <js_backend/js_engine.h>
#include <js_backend/js_to_native_invoker.h>

#include <js_backend/js_error_helper.h>
#include <js_backend/js_heap_helper.h>
#include <js_backend/js_property_helper.h>
