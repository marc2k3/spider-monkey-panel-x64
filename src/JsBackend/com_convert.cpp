#include "PCH.hpp"
#include "com_convert.h"
#include "js_to_native.h"

#include <ComUtils/com_interface_h.h>
#include <ComUtils/com_tools.h>
#include <ComUtils/dispatch_ptr.h>
#include <interfaces/active_x_object.h>
#include <Panel/user_message.h>

using namespace smp;

namespace
{
	using namespace mozjs;

	class WrappedJs : public JSDispatch<IWrappedJs>, public mozjs::IHeapUser
	{
	protected:
		WrappedJs(JSContext* ctx, JS::HandleFunction jsFunction) : m_ctx(ctx)
		{
			JS::RootedObject jsGlobal(ctx, JS::CurrentGlobalOrNull(ctx));
			m_native_global = JsGlobalObject::ExtractNative(ctx, jsGlobal);
			auto& heapMgr = m_native_global->GetHeapManager();
			heapMgr.RegisterUser(this);

			m_func_id = heapMgr.Store(jsFunction);
			m_global_id = heapMgr.Store(jsGlobal);
			m_js_available = true;
		}

		/// @details Might be called off main thread
		~WrappedJs() override = default;

		/// @details Might be called off main thread
		void FinalRelease() override
		{
			// most of the JS object might be invalid at GC time,
			// so we need to be extra careful
			std::scoped_lock sl(m_lock);

			if (!m_js_available)
			{
				return;
			}

			auto& heapMgr = m_native_global->GetHeapManager();

			heapMgr.Remove(m_global_id);
			heapMgr.Remove(m_func_id);
			heapMgr.UnregisterUser(this);
		}

		void PrepareForGlobalGc() override
		{
			std::scoped_lock sl(m_lock);
			// Global is being destroyed, can't access anything
			m_js_available = false;
		}

		/// @details Executed on the main thread
		STDMETHODIMP ExecuteValue(VARIANT arg1, VARIANT arg2, VARIANT arg3, VARIANT arg4, VARIANT arg5, VARIANT arg6, VARIANT arg7, VARIANT* pResult) override
		{
			if (!pResult)
			{
				return E_POINTER;
			}

			if (!m_js_available)
			{ // shutting down, no need to log errors here
				pResult->vt = VT_ERROR;
				pResult->scode = E_FAIL;
				return E_FAIL;
			}

			// Might be executed outside of main JS workflow, so we need to set realm

			auto& heapMgr = m_native_global->GetHeapManager();

			JS::RootedObject jsGlobal(m_ctx, heapMgr.Get(m_global_id).toObjectOrNull());
			JSAutoRealm ac(m_ctx, jsGlobal);

			JS::RootedValue vFunc(m_ctx, heapMgr.Get(m_func_id));
			JS::RootedFunction rFunc(m_ctx, JS_ValueToFunction(m_ctx, vFunc));
			std::array<VARIANT*, 7> args = { &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7 };
			JS::RootedValueArray<args.size()> wrappedArgs(m_ctx);

			try
			{
				for (size_t i = 0; i < args.size(); ++i)
				{
					convert::VariantToJs(m_ctx, *args[i], wrappedArgs[i]);
				}

				JS::RootedValue retVal(m_ctx);
				if (!JS::Call(m_ctx, jsGlobal, rFunc, wrappedArgs, &retVal))
				{
					throw JsException();
				}

				convert::JsToVariant(m_ctx, retVal, *pResult);
			}
			catch (...)
			{
				auto wnd = m_native_global->GetPanelHwnd();
				
				if (wnd)
				{
					const auto errorMsg = mozjs::ExceptionToText(m_ctx);
					SendMessageW(wnd, std::to_underlying(InternalSyncMessage::script_fail), 0, reinterpret_cast<LPARAM>(&errorMsg));
				}
				else
				{
					mozjs::SuppressException(m_ctx);
				}

				pResult->vt = VT_ERROR;
				pResult->scode = E_FAIL;
				return E_FAIL;
			}

			return S_OK;
		}

	private:
		JSContext* m_ctx{};
		uint32_t m_func_id{};
		uint32_t m_global_id{};
		mozjs::JsGlobalObject* m_native_global{};
		std::mutex m_lock;
		bool m_js_available{};
	};

	bool ComArrayToJsArray(JSContext* ctx, const VARIANT& src, JS::MutableHandleValue& dest)
	{
		// We only support one dimensional arrays for now
		QwrException::ExpectTrue(SafeArrayGetDim(src.parray) == 1, "Multi-dimensional array are not supported failed");

		// Get the upper bound;
		long ubound; // NOLINT (google-runtime-int)
		HRESULT hr = SafeArrayGetUBound(src.parray, 1, &ubound);
		smp::CheckHR(hr, "SafeArrayGetUBound");

		// Get the lower bound
		long lbound; // NOLINT (google-runtime-int)
		hr = SafeArrayGetLBound(src.parray, 1, &lbound);
		smp::CheckHR(hr, "SafeArrayGetLBound");

		// Create the JS Array
		JS::RootedObject jsArray(ctx, JS::NewArrayObject(ctx, ubound - lbound + 1));
		JsException::ExpectTrue(jsArray);

		// Divine the type of our array
		VARTYPE vartype;
		if ((src.vt & VT_ARRAY) != 0)
		{
			vartype = src.vt & ~VT_ARRAY;
		}
		else // This was maybe a VT_SAFEARRAY
		{
			hr = SafeArrayGetVartype(src.parray, &vartype);
			smp::CheckHR(hr, "SafeArrayGetVartype");
		}

		JS::RootedValue jsVal(ctx);
		for (long i = lbound; i <= ubound; ++i) // NOLINT (google-runtime-int)
		{
			_variant_t var;
			if (vartype == VT_VARIANT)
			{
				hr = SafeArrayGetElement(src.parray, &i, &var);
			}
			else
			{
				var.vt = vartype;
				hr = SafeArrayGetElement(src.parray, &i, &var.byref);
			}
			smp::CheckHR(hr, "SafeArrayGetElement");

			convert::VariantToJs(ctx, var, &jsVal);

			if (!JS_SetElement(ctx, jsArray, i, jsVal))
			{
				throw JsException();
			}
		}

		dest.setObjectOrNull(jsArray);
		return true;
	}

	template <typename T>
	void PutCastedElementInSafeArrayData(void* arr, size_t idx, T value)
	{
		auto castedArr = static_cast<T*>(arr);
		castedArr[idx] = value;
	}

	void PutVariantInSafeArrayData(void* arr, size_t idx, VARIANTARG& var)
	{
		switch (var.vt)
		{
		case VT_I1:
			PutCastedElementInSafeArrayData<int8_t>(arr, idx, var.cVal);
			break;
		case VT_I2:
			PutCastedElementInSafeArrayData<int16_t>(arr, idx, var.iVal);
			break;
		case VT_INT:
		case VT_I4:
			PutCastedElementInSafeArrayData<int32_t>(arr, idx, var.lVal);
			break;
		case VT_R4:
			PutCastedElementInSafeArrayData<float>(arr, idx, var.fltVal);
			break;
		case VT_R8:
			PutCastedElementInSafeArrayData<double>(arr, idx, var.dblVal);
			break;
		case VT_BOOL:
			PutCastedElementInSafeArrayData<int16_t>(arr, idx, var.boolVal);
			break;
		case VT_UI1:
			PutCastedElementInSafeArrayData<uint8_t>(arr, idx, var.bVal);
			break;
		case VT_UI2:
			PutCastedElementInSafeArrayData<uint16_t>(arr, idx, var.uiVal);
			break;
		case VT_UINT:
		case VT_UI4:
			PutCastedElementInSafeArrayData<uint32_t>(arr, idx, var.ulVal);
			break;
		default:
			throw QwrException("ActiveX: unsupported array type: {:#x}", var.vt);
		}
	}
}

namespace mozjs::convert
{
	/// VariantToJs assumes that the caller will call VariantClear on `var`, so call AddRef on new objects
	void VariantToJs(JSContext* ctx, VARIANTARG& var, JS::MutableHandleValue rval)
	{
		const bool ref = !!(var.vt & VT_BYREF);
		const int type = (ref ? var.vt &= ~VT_BYREF : var.vt);

#define FETCH(x) (ref ? *(var.p##x) : var.x)

		switch (type)
		{
		case VT_ERROR:
			rval.setUndefined();
			break;
		case VT_NULL:
			rval.setNull();
			break;
		case VT_EMPTY:
			rval.setUndefined();
			break;
		case VT_I1:
			rval.setInt32(static_cast<int32_t>(FETCH(cVal)));
			break;
		case VT_I2:
			rval.setInt32(static_cast<int32_t>(FETCH(iVal)));
			break;
		case VT_INT:
		case VT_I4:
			rval.setInt32(FETCH(lVal));
			break;
		case VT_R4:
			rval.setNumber(FETCH(fltVal));
			break;
		case VT_R8:
			rval.setNumber(FETCH(dblVal));
			break;
		case VT_BOOL:
			rval.setBoolean(FETCH(boolVal));
			break;
		case VT_UI1:
			rval.setNumber(static_cast<uint32_t>(FETCH(bVal)));
			break;
		case VT_UI2:
			rval.setNumber(static_cast<uint32_t>(FETCH(uiVal)));
			break;
		case VT_UINT:
		case VT_UI4:
			rval.setNumber(static_cast<uint32_t>(FETCH(ulVal)));
			break;
		case VT_BSTR:
		{
			JS::RootedString jsString(ctx, JS_NewUCStringCopyN(ctx, reinterpret_cast<const char16_t*>(FETCH(bstrVal)), SysStringLen(FETCH(bstrVal))));
			JsException::ExpectTrue(jsString);

			rval.setString(jsString);
			break;
		};
		case VT_DATE:
		{
			DATE d = FETCH(date);
			SYSTEMTIME time;
			VariantTimeToSystemTime(d, &time);

			JS::RootedObject jsObject(ctx, JS::NewDateObject(ctx, time.wYear, time.wMonth - 1, time.wDay, time.wHour, time.wMinute, time.wSecond));
			JsException::ExpectTrue(jsObject);

			rval.setObjectOrNull(jsObject);
			break;
		}
		case VT_UNKNOWN:
		{
			if (!FETCH(punkVal))
			{
				rval.setNull();
				break;
			}

			std::unique_ptr<JsActiveXObject> x(new JsActiveXObject(ctx, FETCH(punkVal), true));
			JS::RootedObject jsObject(ctx, JsActiveXObject::CreateJsFromNative(ctx, std::move(x)));
			rval.setObjectOrNull(jsObject);
			break;
		}
		case VT_DISPATCH:
		{
			if (!FETCH(pdispVal))
			{
				rval.setNull();
				break;
			}

			std::unique_ptr<JsActiveXObject> x(new JsActiveXObject(ctx, FETCH(pdispVal), true));
			JS::RootedObject jsObject(ctx, JsActiveXObject::CreateJsFromNative(ctx, std::move(x)));
			rval.setObjectOrNull(jsObject);
			break;
		}
		case VT_VARIANT: //traverse the indirection list?
			if (ref)
			{
				VARIANTARG* v = var.pvarVal;
				if (v)
				{
					return VariantToJs(ctx, *v, rval);
				}
			}
			break;
		default:
			if ((type & VT_ARRAY) && !(type & VT_UI1))
			{ // convert all arrays that are not binary data: SMP has no use for it, but it's needed in COM interface
				ComArrayToJsArray(ctx, var, rval);
				break;
			}
			else
			{
				QwrException::ExpectTrue(type <= VT_CLSID || type == (VT_ARRAY | VT_UI1), "ActiveX: unsupported object type: {:#x}", type);

				JS::RootedObject jsObject(ctx, JsActiveXObject::CreateJsFromNative(ctx, std::make_unique<JsActiveXObject>(ctx, var)));
				rval.setObjectOrNull(jsObject);
			}
		}

#undef FETCH
	}

	void JsToVariant(JSContext* ctx, JS::HandleValue rval, VARIANTARG& arg)
	{
		VariantInit(&arg);

		if (rval.isObject())
		{
			JS::RootedObject j0(ctx, &rval.toObject());
			auto pNative = JsActiveXObject::ExtractNative(ctx, j0);

			if (pNative)
			{
				JsActiveXObject* x = pNative;
				if (x->pStorage_->variant.vt != VT_EMPTY)
				{
					HRESULT hr = VariantCopyInd(&arg, &x->pStorage_->variant);
					smp::CheckHR(hr, "VariantCopyInd");
				}
				else if (x->pStorage_->pDispatch)
				{
					arg.vt = VT_DISPATCH;
					arg.pdispVal = x->pStorage_->pDispatch;
					x->pStorage_->pDispatch->AddRef();
				}
				else if (x->pStorage_->pUnknown)
				{
					arg.vt = VT_UNKNOWN;
					arg.punkVal = x->pStorage_->pUnknown;
					x->pStorage_->pUnknown->AddRef();
				}
				else
				{
					arg.vt = VT_BYREF | VT_UNKNOWN;
					arg.ppunkVal = &x->pStorage_->pUnknown;
				}
			}
			else if (JS_ObjectIsFunction(j0))
			{
				JS::RootedFunction func(ctx, JS_ValueToFunction(ctx, rval));

				arg.vt = VT_DISPATCH;
				arg.pdispVal = new ComObject<WrappedJs>(ctx, func);
			}
			else
			{
				bool is;
				if (!JS::IsArrayObject(ctx, rval, &is))
				{
					throw JsException();
				}

				if (is)
				{ // other types of arrays are created manually (e.g. VT_ARRAY|VT_I1)
					JsArrayToVariantArray(ctx, j0, VT_VARIANT, arg);
				}
				else
				{
					throw QwrException("ActiveX: unsupported JS object type");
				}
			}
		}
		else if (rval.isBoolean())
		{
			arg.vt = VT_BOOL;
			arg.boolVal = rval.toBoolean() ? -1 : 0;
		}
		else if (rval.isInt32())
		{
			arg.vt = VT_I4;
			arg.lVal = rval.toInt32();
		}
		else if (rval.isDouble())
		{
			arg.vt = VT_R8;
			arg.dblVal = rval.toDouble();
		}
		else if (rval.isNull())
		{
			arg.vt = VT_NULL;
			arg.scode = 0;
		}
		else if (rval.isUndefined())
		{
			arg.vt = VT_EMPTY;
			arg.scode = 0;
		}
		else if (rval.isString())
		{
			const auto str = convert::to_native::ToValue<std::wstring>(ctx, rval);
			_bstr_t bStr = str.c_str();

			arg.vt = VT_BSTR;
			arg.bstrVal = bStr.Detach();
		}
		else
		{
			throw QwrException("ActiveX: unsupported JS value type");
		}
	}

	void JsArrayToVariantArray(JSContext* ctx, JS::HandleObject obj, VARTYPE elementVariantType, VARIANT& var)
	{
		uint32_t len;
		if (!JS::GetArrayLength(ctx, obj, &len))
		{
			throw JsException();
		}

		// Create the safe array of variants and populate it
		SAFEARRAY* safeArray = SafeArrayCreateVector(elementVariantType, 0, len);
		QwrException::ExpectTrue(safeArray, "SafeArrayCreateVector failed");

		auto autoSa = wil::scope_exit([safeArray]
			{
				SafeArrayDestroy(safeArray);
			});

		if (len)
		{
			if (elementVariantType == VT_VARIANT)
			{
				VARIANT* varArray = nullptr;
				HRESULT hr = SafeArrayAccessData(safeArray, reinterpret_cast<void**>(&varArray));
				smp::CheckHR(hr, "SafeArrayAccessData");

				auto autoSaData = wil::scope_exit([safeArray]
					{
						SafeArrayUnaccessData(safeArray);
					});

				for (uint32_t i = 0; i < len; ++i)
				{
					JS::RootedValue val(ctx);
					if (!JS_GetElement(ctx, obj, i, &val))
					{
						throw JsException();
					}

					JsToVariant(ctx, val, varArray[i]);
				}
			}
			else
			{
				void* dataArray = nullptr;
				HRESULT hr = SafeArrayAccessData(safeArray, reinterpret_cast<void**>(&dataArray));
				smp::CheckHR(hr, "SafeArrayAccessData");

				auto autoSaData = wil::scope_exit([safeArray]
					{
						SafeArrayUnaccessData(safeArray);
					});

				for (uint32_t i = 0; i < len; ++i)
				{
					JS::RootedValue val(ctx);
					if (!JS_GetElement(ctx, obj, i, &val))
					{
						throw JsException();
					}

					_variant_t tmp;
					JsToVariant(ctx, val, tmp);
					tmp.ChangeType(elementVariantType);

					PutVariantInSafeArrayData(dataArray, i, tmp);
				}
			}
		}

		var.vt = VT_ARRAY | elementVariantType;
		var.parray = safeArray;

		autoSa.release();
	}
}
