#pragma once
#define _ATL_MODULES
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#define WINVER _WIN32_WINNT_WIN7
#define NOMINMAX

// stl
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <queue>
#include <ranges>
#include <set>
#include <span>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

#include <WinSock2.h>
#include <Windows.h>
#include <windowsx.h>
#include <GdiPlus.h>
#include <Shlwapi.h>
#include <wincodec.h>
#include <windef.h>

// COM objects
#include <ActivScp.h>
#include <activdbg.h>
#include <comutil.h>
#include <MsHTML.h>
#include <ExDispid.h>
#include <MsHtmHst.h>
#include <oleauto.h>
#include <ShlDisp.h>
#include <ShlObj.h>
#include <exdisp.h>
#include <shobjidl_core.h>
// Generates wrappers for COM listed above
#include <ComDef.h>

#include <atlstr.h>
#include <atlapp.h>
#include <atlcom.h>
#include <atlcrack.h>
#include <atlctrls.h>
#include <atlddx.h>
#include <atlframe.h>
#include <atltypes.h>
#include <atlwin.h>

// fmt
#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <fmt/xchar.h>

// json
#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include <nlohmann/json.hpp>
using JSON = nlohmann::json;

// wil
#include <wil/com.h>
#include <wil/filesystem.h>
#include <wil/resource.h>
#include <wil/win32_helpers.h>

// foobar2000 SDK
#pragma warning(push, 0)
#include <SDK/foobar2000.h>
#include <SDK/coreDarkMode.h>
#include <SDK/file_info_filter_impl.h>
#include <helpers/atl-misc.h>
#include <helpers/file_list_helper.h>
#include <helpers/WindowPositionUtils.h>
#include <libPPUI/CDialogResizeHelper.h>
#include <libPPUI/CListControlOwnerData.h>
#include <libPPUI/gdiplus_helpers.h>
#include <libPPUI/pp-COM-macros.h>
#include <libPPUI/win32_utility.h>
#include <libPPUI/windowLifetime.h>
#include <pfc/string-conv-lite.h>
#pragma warning(pop)

// Columns UI SDK
#pragma warning(push, 0)
#include <columns_ui-sdk/ui_extension.h>
#pragma warning(pop)

// Component
#include "SpiderMonkey.hpp"
