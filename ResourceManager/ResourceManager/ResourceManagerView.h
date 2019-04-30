// ResourceManagerView.h : interface of the CResourceManagerView class 
// 
////////////////////////////////////////////////////////////////////////// 
#if !defined(AFX_SCROLLDEMOVIEW_H__DCCF4E0D_9735_11D2_8E53_006008A82731__INCLUDED_) 
#define AFX_SCROLLDEMOVIEW_H__DCCF4E0D_9735_11D2_8E53_006008A82731__INCLUDED_ 
#if _MSC_VER > 1000 
#pragma once 
#endif // _MSC_VER > 1000 

class CResourceManagerDoc;
class CLeftView;

class CResourceManagerView : public CScrollView
{
protected:
	// create from serialization only
	CResourceManagerView();
	DECLARE_DYNCREATE(CResourceManagerView)
	// Attributes 
public:     
	CResourceManagerDoc* GetDocument();
			
	// Operations 
public:
	// Overrides     
	// ClassWizard generated virtual function overrides     
	//{{AFX_VIRTUAL(CResourceManagerView)     
public:
	virtual void OnDraw(CDC* pDC);
	// overridden to draw this view     
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate();
	virtual void OnUpdate(CView * pSender, LPARAM lHint, CObject * pHint);
	// called first time after construct     
	//}}AFX_VIRTUAL 
	// Implementation public:     
	virtual ~CResourceManagerView();
#ifdef _DEBUG     
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif protected: 
	// Generated message map functions 
protected:
	BOOL m_bSmooth;
	void GetCellRect(int row, int col, LPRECT pRect);
	void DrawAddress(CDC* pDC, int row, int col);
	void DrawAddress(CDC * pDC, int row, int col, CString label, bool active);
	void DrawPointer(CDC* pDC, int row, int col, bool bHighlight);
	void DrawTree(CDC * pDC, string current, unordered_map<string, GraphNode>& nodes);
	bool DrawLines(CDC * pDC, string current, unordered_map<string, GraphNode>& nodes);
	void DrawNodes(CDC * pDC, unordered_map<string, GraphNode>& nodes);
	pair<double, double> FindRatios(string current, string parent, unordered_map<string, GraphNode> nodes);
	CFont m_font;
	int m_nCurrentCol;
	int m_nCurrentRow;
	int m_nRibbonWidth;
	int m_nCellHeight;
	int m_nCellWidth;
	//{{AFX_MSG(CResourceManagerView)     
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG     

	DECLARE_MESSAGE_MAP()
public:
	// How deep the grid is 
	int width;
	// How tall the grid is
	int height;
	// Sets a pointer to the left view (called from main window)
	void SetLeftView(CLeftView* left);
protected:
	CLeftView* m_leftView;
public:
	// Called by left view when a new item is selected to synchronize views
	void SynchronizeSelection(string name);
	void UpdateSelection(string name);
protected:
	string m_selection;
	int m_scale;
public:
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	void InsertNewNodePair();
};
#ifndef _DEBUG  
// debug version in ScrollDemoView.cpp 
inline CScrollDemoDoc* CResourceManagerView::GetDocument()
{
	return (CScrollDemoDoc*)m_pDocument;
}
#endif 

/////////////////////////////////////////////////////////////////////////// 
//{{AFX_INSERT_LOCATION}} 
// Microsoft Visual C++ will insert additional declarations immediately 
// before the previous line. 
#endif // !defined(AFX_SCROLLDEMOVIEW_H__DCCF4E0D_9735_11D2_8E53_006008A82731__INCLUDED_) 
