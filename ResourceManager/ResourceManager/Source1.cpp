// ResourceManagerView.h : interface of the CResourceManagerView class 
// 
////////////////////////////////////////////////////////////////////////// 
#if !defined(AFX_SCROLLDEMOVIEW_H__DCCF4E0D_9735_11D2_8E53_006008A82731__INCLUDED_) 
#define AFX_SCROLLDEMOVIEW_H__DCCF4E0D_9735_11D2_8E53_006008A82731__INCLUDED_ 
#if _MSC_VER > 1000 
#pragma once 
#endif // _MSC_VER > 1000 

class CResourceManagerView : public CScrollView 
{ 
protected: 
	// create from serialization only     
	CResourceManagerView();     
	DECLARE_DYNCREATE(CResourceManagerView) 
	// Attributes 
public:     CScrollDemoDoc* GetDocument(); 
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
	void GetCellRect (int row, int col, LPRECT pRect);     
	void DrawAddress (CDC* pDC, int row, int col);     
	void DrawPointer (CDC* pDC, int row, int col, BOOL bHighlight);     
	CFont m_font;     
	int m_nCurrentCol;     
	int m_nCurrentRow;    
	int m_nRibbonWidth;     
	int m_nCellHeight;     
	int m_nCellWidth;     
	//{{AFX_MSG(CResourceManagerView)     
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);     
	//}}AFX_MSG     
	
	DECLARE_MESSAGE_MAP() }; 
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

ScrollDemoView.cpp

// ResourceManagerView.cpp : implementation of the CResourceManagerView class 
// 
#include "stdafx.h" 
#include "ScrollDemo.h" 
#include "ScrollDemoDoc.h" 
#include "ScrollDemoView.h" 
#ifdef _DEBUG 
#define new DEBUG_NEW 
#undef THIS_FILE 
static char THIS_FILE[] = __FILE__; 
#endif 

/////////////////////////////////////////////////////////////////////////// 
// CResourceManagerView 
IMPLEMENT_DYNCREATE(CResourceManagerView, CScrollView) 
BEGIN_MESSAGE_MAP(CResourceManagerView, CScrollView)     
	//{{AFX_MSG_MAP(CResourceManagerView)     
	ON_WM_LBUTTONDOWN()     
	//}}AFX_MSG_MAP 
END_MESSAGE_MAP() 

/////////////////////////////////////////////////////////////////////////// 
// CResourceManagerView construction/destruction 
CResourceManagerView::CResourceManagerView() 
{     
	m_font.CreatePointFont (80, _T ("MS Sans Serif")); 
} 

CResourceManagerView::~CResourceManagerView() 
{ 
} 

BOOL CResourceManagerView::PreCreateWindow(CREATESTRUCT& cs) 
{     
	return CScrollView::PreCreateWindow(cs); 
}

/////////////////////////////////////////////////////////////////////////// 
// CResourceManagerView drawing 
void CResourceManagerView::OnDraw(CDC* pDC) 
{     
	CScrollDemoDoc* pDoc = GetDocument();     
	ASSERT_VALID(pDoc);     
	//     
	// Draw the grid lines. 
	//     
	CSize size = GetTotalSize ();     
	CPen pen (PS_SOLID, 0, RGB (192, 192, 192));    
	CPen* pOldPen = pDC->SelectObject (&pen);     
	for (int i=0; i<99; i++) {         
		int y = (i * m_nCellHeight) + m_nCellHeight;         
		pDC->MoveTo (0, y);         
		pDC->LineTo (size.cx, y);     
	}     
	for (int j=0; j<26; j++) {         
		int x = (j * m_nCellWidth) + m_nRibbonWidth;         
		pDC->MoveTo (x, 0);         
		pDC->LineTo (x, size.cy);     
	}     
	pDC->SelectObject (pOldPen);          
	//     
	// Draw the bodies of the rows and column headers.     
	//     CBrush brush;     
	brush.CreateStockObject (LTGRAY_BRUSH);     
	CRect rcTop (0, 0, size.cx, m_nCellHeight);    
	pDC->FillRect (rcTop, &brush);     
	CRect rcLeft (0, 0, m_nRibbonWidth, size.cy);     
	pDC->FillRect (rcLeft, &brush);     
	pDC->MoveTo (0, m_nCellHeight);     
	pDC->LineTo (size.cx, m_nCellHeight);     
	pDC->MoveTo (m_nRibbonWidth, 0);     
	pDC->LineTo (m_nRibbonWidth, size.cy);     
	pDC->SetBkMode (TRANSPARENT);     
	//     
	// Add numbers and button outlines to the row headers.     
	//     
	for (i=0; i<99; i++) {         
		int y = (i * m_nCellHeight) + m_nCellHeight;         
		pDC->MoveTo (0, y);         
		pDC->LineTo (m_nRibbonWidth, y);         
		CString string;         
		string.Format (_T ("%d"), i + 1);         
		CRect rect (0, y, m_nRibbonWidth, y + m_nCellHeight);         
		pDC->DrawText (string, &rect, DT_SINGLELINE � DT_CENTER � DT_VCENTER);         
		rect.top++;         
		pDC->Draw3dRect (rect, RGB (255, 255, 255), RGB (128, 128, 128));     
	}     
	//     
	// Add letters and button outlines to the column headers.     
	//     
	for (j=0; j<26; j++) {         
		int x = (j * m_nCellWidth) + m_nRibbonWidth;         
		pDC->MoveTo (x, 0);         
		pDC->LineTo (x, m_nCellHeight);         
		CString string;         
		string.Format (_T ("%c"), j + `A');         
		CRect rect (x, 0, x + m_nCellWidth, m_nCellHeight);         
		pDC->DrawText (string, &rect, DT_SINGLELINE � DT_CENTER � DT_VCENTER);         
		rect.left++;         
		pDC->Draw3dRect (rect, RGB (255, 255, 255), RGB (128, 128, 128));     
	}     
	//     
	// Draw address labels into the individual cells.     
	//     
	CRect rect;     
	pDC->GetClipBox (&rect);     
	int nStartRow = max (0, (rect.top - m_nCellHeight) / m_nCellHeight);     
	int nEndRow = min (98, (rect.bottom - 1) / m_nCellHeight);     
	int nStartCol = max (0, (rect.left - m_nRibbonWidth) / m_nCellWidth);     
	int nEndCol = min (25, ((rect.right + m_nCellWidth - 1) - m_nRibbonWidth) / m_nCellWidth);     
	for (i=nStartRow; i<=nEndRow; i++)         
		for (j=nStartCol; j<=nEndCol; j++)             
			DrawAddress (pDC, i, j);     
	//     
	// Draw the cell pointer.     
	//     
	DrawPointer (pDC, m_nCurrentRow, m_nCurrentCol, TRUE); 
} 

void CResourceManagerView::OnInitialUpdate() 
{     
	CScrollView::OnInitialUpdate();     
	m_nCurrentRow = 0;     
	m_nCurrentCol = 0;     
	m_bSmooth = FALSE;     
	CClientDC dc (this);     
	m_nCellWidth = dc.GetDeviceCaps (LOGPIXELSX);     
	m_nCellHeight = dc.GetDeviceCaps (LOGPIXELSY) / 4;     
	m_nRibbonWidth = m_nCellWidth / 2;     
	int nWidth = (26 * m_nCellWidth) + m_nRibbonWidth;     
	int nHeight = m_nCellHeight * 100;     
	SetScrollSizes (MM_TEXT, CSize (nWidth, nHeight)); 
} 

/////////////////////////////////////////////////////////////////////////// 
// CResourceManagerView diagnostics 
#ifdef _DEBUG 
void CResourceManagerView::AssertValid() const 
{     
	CScrollView::AssertValid(); 
} 

void CResourceManagerView::Dump(CDumpContext& dc) const 
{     
	CScrollView::Dump(dc); 
} 

CScrollDemoDoc* CResourceManagerView::GetDocument() // non-debug version is inline 
{     
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CScrollDemoDoc)));     
	return (CScrollDemoDoc*)m_pDocument; } 
#endif //_DEBUG 

/////////////////////////////////////////////////////////////////////////// 
// CResourceManagerView message handlers 
void CResourceManagerView::OnLButtonDown(UINT nFlags, CPoint point)  
{     CScrollView::OnLButtonDown(nFlags, point);     
	//     
	// Convert the click point to logical coordinates.     
	//     
	CPoint pos = point;     
	CClientDC dc (this);     
	OnPrepareDC (&dc);     
	dc.DPtoLP (&pos);     
	//     
	// If a cell was clicked, move the cell pointer.     
	//     
	CSize size = GetTotalSize ();     
	if (pos.x > m_nRibbonWidth && pos.x < size.cx && pos.y > m_nCellHeight && pos.y < size.cy) 
	{         
		int row = (pos.y - m_nCellHeight) / m_nCellHeight;         
		int col = (pos.x - m_nRibbonWidth) / m_nCellWidth;         
		ASSERT (row >= 0 && row <= 98 && col >= 0 && col <= 25);         
		DrawPointer (&dc, m_nCurrentRow, m_nCurrentCol, FALSE);         
		m_nCurrentRow = row;         
		m_nCurrentCol = col;         
		DrawPointer (&dc, m_nCurrentRow, m_nCurrentCol, TRUE);     
	} 
} 

void CResourceManagerView::DrawPointer(CDC *pDC, int row, int col,      BOOL bHighlight) 
{     
	CRect rect;     
	GetCellRect (row, col, &rect);     
	CBrush brush (bHighlight ? RGB (0, 255, 255) : ::GetSysColor (COLOR_WINDOW));     
	pDC->FillRect (rect, &brush);     
	DrawAddress (pDC, row, col); 
} 

void CResourceManagerView::DrawAddress(CDC *pDC, int row, int col) 
{     
	CRect rect;     
	GetCellRect (row, col, &rect);     
	CString string;     
	string.Format (_T ("%c%d"), col + _T (`A'), row + 1);     
	pDC->SetBkMode (TRANSPARENT);     
	CFont* pOldFont = pDC->SelectObject (&m_font);     
	pDC->DrawText (string, rect, DT_SINGLELINE � DT_CENTER � DT_VCENTER);     
	pDC->SelectObject (pOldFont); 
} 

void CResourceManagerView::GetCellRect(int row, int col, LPRECT pRect) 
{     
	pRect->left = m_nRibbonWidth + (col * m_nCellWidth) + 1;     
	pRect->top = m_nCellHeight + (row * m_nCellHeight) + 1;     
	pRect->right = pRect->left + m_nCellWidth - 1;     
	pRect->bottom = pRect->top + m_nCellHeight - 1; 
} 