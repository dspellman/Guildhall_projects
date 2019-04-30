
// LeftView.h : interface of the CLeftView class
//


#pragma once

class CResourceManagerDoc;
class CResourceManagerView;

class CLeftView : public CTreeView
{
protected: // create from serialization only
	CLeftView();
	DECLARE_DYNCREATE(CLeftView)

// Attributes
public:
	CResourceManagerDoc* GetDocument();

	void SetCheckState(HWND hwndTreeView, HTREEITEM hItem, bool fCheck);
	bool GetCheckState(HWND hwndTreeView, HTREEITEM hItem);

// Operations
public:

// Overrides
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CLeftView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult);
private:
	HTREEITEM hSelection;
public:
	afx_msg void OnTvnKeydown(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
protected:
	CResourceManagerView* m_rightView;
public:
	// Set a pointer to the right side view (called from the main window)
	void SetRightView(CResourceManagerView* right);
	void SynchronizeSelection(string selection);
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	void DrawTree();
	string GetSelection();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
private:
	bool m_initializing;
};

#ifndef _DEBUG  // debug version in LeftView.cpp
inline CResourceManagerDoc* CLeftView::GetDocument()
   { return reinterpret_cast<CResourceManagerDoc*>(m_pDocument); }
#endif

