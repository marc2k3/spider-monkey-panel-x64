#pragma once
#define JS_CODEGEN_X64 1
#define JS_STANDALONE 1
#define XP_WIN 1

__pragma(warning(push))
__pragma(warning(disable : 4100 4244 4251))
#include <jsapi.h>
#include <jsfriendapi.h>

#include <js/experimental/JSStencil.h>
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

#include <JsBackend/global_object.h>

#include <JsBackend/js_container.h>
#include <JsBackend/js_engine.h>
#include <JsBackend/js_to_native_invoker.h>

#include <JsBackend/js_heap_helper.h>
#include <JsBackend/js_property_helper.h>
