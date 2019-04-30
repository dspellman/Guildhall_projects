// InsertDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ResourceManager.h"
#include "InsertDialog.h"
#include "afxdialogex.h"


// CInsertDialog dialog

IMPLEMENT_DYNAMIC(CInsertDialog, CDialog)

CInsertDialog::CInsertDialog(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_INSERTDIALOG, pParent)
{
	m_parent = _T("");
	m_child = _T("");
}

CInsertDialog::~CInsertDialog()
{
}

void CInsertDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDITPARENT, m_editParent);
	DDX_Control(pDX, IDC_EDITCHILD, m_editChild);

	int len = m_editParent.LineLength(m_editParent.LineIndex(0));
	m_editParent.GetLine(0, m_parent.GetBuffer(len), len);
	m_parent.ReleaseBuffer(len);
	m_parent.Trim();
	m_parent.Replace(_T(" "), _T("_")); // no spaces in names allowed based on the resource file format

	len = m_editChild.LineLength(m_editChild.LineIndex(0));
	m_editChild.GetLine(0, m_child.GetBuffer(len), len);
	m_child.ReleaseBuffer(len);
	m_child.Trim();
	m_child.Replace(_T(" "), _T("_"));
}


BEGIN_MESSAGE_MAP(CInsertDialog, CDialog)
END_MESSAGE_MAP()


// CInsertDialog message handlers


BOOL CInsertDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	//m_editParent.SetCueBanner(_T("Parent name"));
	//m_editChild.SetCueBanner(_T("Child name"));
	GotoDlgCtrl(GetDlgItem(IDC_EDITPARENT));
	return FALSE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}
