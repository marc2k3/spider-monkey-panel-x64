#pragma once
#include "com_interface_h.h"

extern wil::com_ptr<ITypeLib> typelib_smp;

template <typename T>
class JSDispatchBase : public T
{
protected:
	JSDispatchBase()
	{
		if (!s_type_info)
		{
			THROW_IF_FAILED(typelib_smp->GetTypeInfoOfGuid(__uuidof(T), &s_type_info));
		}
	}

	virtual ~JSDispatchBase() = default;

public:
	STDMETHODIMP GetIDsOfNames(REFIID, OLECHAR** names, uint32_t, LCID, DISPID* dispids) noexcept final
	{
		RETURN_HR_IF_NULL(E_POINTER, dispids);

		const auto hash = LHashValOfName(LANG_NEUTRAL, names[0]);
		const auto it = s_dispid_map.find(hash);

		if (it != s_dispid_map.end())
		{
			dispids[0] = it->second;
		}
		else
		{
			RETURN_IF_FAILED(s_type_info->GetIDsOfNames(&names[0], 1, &dispids[0]));

			s_dispid_map.emplace(hash, dispids[0]);
		}

		return S_OK;
	}

	STDMETHODIMP GetTypeInfo(uint32_t i, LCID, ITypeInfo** out) noexcept final
	{
		RETURN_HR_IF_NULL(E_POINTER, out);
		RETURN_HR_IF(DISP_E_BADINDEX, i != 0u);

		s_type_info->AddRef();
		*out = s_type_info.get();
		return S_OK;
	}

	STDMETHODIMP GetTypeInfoCount(uint32_t* n) noexcept final
	{
		RETURN_HR_IF_NULL(E_POINTER, n);

		*n = 1;
		return S_OK;
	}

	STDMETHODIMP Invoke(DISPID dispid, REFIID, LCID, WORD flags, DISPPARAMS* params, VARIANT* result, EXCEPINFO* excep, uint32_t* err) noexcept final
	{
		return s_type_info->Invoke(this, dispid, flags, params, result, excep, err);
	}

	static inline std::unordered_map<ULONG, DISPID> s_dispid_map;
	static inline wil::com_ptr<ITypeInfo> s_type_info;
};

template <class T>
class JSDispatch : public JSDispatchBase<T>
{
protected:
	JSDispatch<T>() = default;
	~JSDispatch<T>() = default;

	virtual void FinalRelease() {}

private:
	COM_QI_BEGIN()
		COM_QI_ENTRY(IDispatch)
		COM_QI_ENTRY(T)
	COM_QI_END()
};

template <typename T>
class ComObject : public T
{
protected:
	virtual ~ComObject() = default;

public:
	ComObject(auto&&... args) : T(std::forward<decltype(args)>(args)...) {}

	ULONG STDMETHODCALLTYPE AddRef() noexcept final
	{
		return ++m_counter;
	}

	ULONG STDMETHODCALLTYPE Release() noexcept final
	{
		const auto n = --m_counter;

		if (n == 0ul)
		{
			this->FinalRelease();
			delete this;
		}

		return n;
	}

private:
	std::atomic<ULONG> m_counter = 1ul;
};
