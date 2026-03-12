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
