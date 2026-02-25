#pragma once
#include <resources/resource.h>
#include <utils/error_popup.h>
#include <utils/unicode.h>
#include <utils/js_exception.h>
#include <utils/type_traits.h>
#include <utils/winapi_error_helpers.h>
#include <utils/qwr_exception.h>
#include <utils/error_helpers.h>

#include <FB2K/API.hpp>
#include <Helpers/Helpers.hpp>
#include <Helpers/String.hpp>

#include <Helpers/FileDialog.hpp>
#include <Helpers/FileHelper.hpp>
#include <Helpers/TextFile.hpp>

#include "Component.hpp"
#include "GUIDS.hpp"
#include "Paths.hpp"

namespace smp
{
	class js_panel_window;
}

#include "MozJS.hpp"

#include <JsBackend/Events/event_dispatcher.h>
#include <JsBackend/Events/event_js_callback.h>
