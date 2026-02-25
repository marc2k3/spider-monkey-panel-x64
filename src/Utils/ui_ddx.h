#pragma once

namespace smp
{
	class IUiDdx
	{
	public:
		IUiDdx() = default;
		virtual ~IUiDdx() = default;

		virtual bool IsMatchingId(int controlId) const = 0;
		virtual void SetHwnd(HWND hWnd) = 0;
		virtual void ReadFromUi() = 0;
		virtual void WriteToUi() = 0;
	};

	class UiDdx_CheckBox final : public IUiDdx
	{
	public:
		UiDdx_CheckBox(bool& value, int controlId) : value_(value), controlId_(controlId) {}
		~UiDdx_CheckBox() override = default;

		bool IsMatchingId(int controlId) const override
		{
			return (controlId == controlId_);
		}

		void SetHwnd(HWND hWnd) override
		{
			hWnd_ = hWnd;
		}

		void ReadFromUi() override
		{
			if (hWnd_)
			{
				value_ = uButton_GetCheck(hWnd_, controlId_);
			}
		}

		void WriteToUi() override
		{
			if (hWnd_)
			{
				uButton_SetCheck(hWnd_, controlId_, value_);
			}
		}

	private:
		HWND hWnd_{};
		const int controlId_;
		bool& value_;
	};

	class UiDdx_TextEdit final : public IUiDdx
	{
	public:
		UiDdx_TextEdit(std::string& value, int controlId) : value_(value), controlId_(controlId) {}
		~UiDdx_TextEdit() override = default;

		bool IsMatchingId(int controlId) const override
		{
			return controlId == controlId_;
		}

		void SetHwnd(HWND hWnd) override
		{
			hWnd_ = hWnd;
		}

		void ReadFromUi() override
		{
			if (hWnd_)
			{
				value_ = uGetDlgItemText(hWnd_, controlId_).get_ptr();
			}
		}

		void WriteToUi() override
		{
			if (hWnd_)
			{
				uSetDlgItemText(hWnd_, controlId_, value_.c_str());
			}
		}

	private:
		HWND hWnd_{};
		const int controlId_;
		std::string& value_;
	};

	class UiDdx_RadioRange final : public IUiDdx
	{
	public:
		UiDdx_RadioRange(int& value, std::span<const int> controlIdList) : value_(value), controlIdList_(controlIdList.begin(), controlIdList.end()) {}
		~UiDdx_RadioRange() override = default;

		bool IsMatchingId(int controlId) const override
		{
			return controlIdList_.cend() != std::ranges::find(controlIdList_, controlId);
		}

		void SetHwnd(HWND hWnd) override
		{
			hWnd_ = hWnd;
		}

		void ReadFromUi() override
		{
			if (hWnd_)
			{
				for (const auto& id : controlIdList_)
				{
					if (uButton_GetCheck(hWnd_, id))
					{
						value_ = id;
						break;
					}
				}
			}
		}

		void WriteToUi() override
		{
			if (hWnd_)
			{
				for (const auto& id : controlIdList_)
				{
					uButton_SetCheck(hWnd_, id, value_ == id);
				}
			}
		}

	private:
		HWND hWnd_{};
		const std::vector<int> controlIdList_;
		int& value_;
	};

	template <typename ListT>
	class UiDdx_ListBase final : public IUiDdx
	{
	public:
		UiDdx_ListBase(int& value, int controlId) : value_(value), controlId_(controlId) {}
		~UiDdx_ListBase() override = default;

		bool IsMatchingId(int controlId) const override
		{
			return controlId == controlId_;
		}

		void SetHwnd(HWND hWnd) override
		{
			hWnd_ = hWnd;
		}

		void ReadFromUi() override
		{
			if (hWnd_)
			{
				value_ = ListT(GetDlgItem(hWnd_, controlId_)).GetCurSel();
			}
		}
		void WriteToUi() override
		{
			if (hWnd_)
			{
				ListT(GetDlgItem(hWnd_, controlId_)).SetCurSel(value_);
			}

		}

	private:
		HWND hWnd_{};
		const int controlId_;
		int& value_;
	};

	using UiDdx_ComboBox = UiDdx_ListBase<CComboBox>;
	using UiDdx_ListBox = UiDdx_ListBase<CListBox>;

	template <typename DdxT, typename... Args>
	std::unique_ptr<IUiDdx> CreateUiDdx(auto& value, Args&&... args)
	{
		return std::make_unique<DdxT>(value, std::forward<Args>(args)...);
	}
}
