#pragma once
#include "definemargin_grid.h"
#include <atldbcli.h>
#include "TableGroup_Symbol.h"
#include "afxwin.h"

// define_margin dialog

class define_margin : public CDialogEx
{
	DECLARE_DYNAMIC(define_margin)

public:
	define_margin(CWnd* pParent = NULL);   // standard constructor
	virtual ~define_margin();

	//data base connection
	CDataSource connection;
    CSession session;
    HRESULT hr;
	CCommand<CNoAccessor, CNoRowset> cmd;
// Dialog Data
	enum { IDD = IDD_DEFINE_MARGIN };
	definemargin_grid d_grid;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void getSymbolData();
	
	CEdit m_textsearch;
	afx_msg void OnEnChangeEdit1();
	
	afx_msg void OnBnClickedButtonsave();
	afx_msg void OnClose();
};
