#pragma once
#include "afxwin.h"


// CInsertDialog dialog

class CInsertDialog : public CDialog
{
	DECLARE_DYNAMIC(CInsertDialog)

public:
	CInsertDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInsertDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INSERTDIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CEdit m_editParent;
	CEdit m_editChild;

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CString m_parent;
	CString m_child;
};
