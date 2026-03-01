#pragma once

class PropertyList : public CListControlComplete
{
public:
	BEGIN_MSG_MAP_EX(PropertyList)
		CHAIN_MSG_MAP(CListControlComplete);
		MSG_WM_CREATE(OnCreate)
	END_MSG_MAP()

	bool GetCellCheckState(size_t row, size_t column) const final;
	bool GetSubItemText(size_t row, size_t column, pfc::string_base& out) const final;
	bool TableEdit_IsColumnEditable(size_t column) const final;
	cellType_t GetCellType(size_t row, size_t column) const final;
	size_t GetItemCount() const final;
	void OnSubItemClicked(size_t row, size_t column, CPoint pt) final;
	void RequestRemoveSelection() final;
	void SetCellCheckState(size_t row, size_t column, bool value) final;
	void TableEdit_SetField(size_t row, size_t column, const char* value) final;

	void ExecuteDefaultAction(size_t) final {}
	void RequestReorder(size_t const*, size_t) final {}

	config::PanelProperties GetProperties();
	void OnContextMenu(CPoint pt);
	void SetProperties(const config::PanelProperties& properties);

private:
	struct ListItem
	{
		std::string key, value;
		bool bool_value{};
		bool is_bool{};
		bool is_string{};
	};

	int32_t OnCreate(LPCREATESTRUCT);

	CButton m_btn_clear, m_btn_export;
	std::vector<ListItem> m_items;
};
