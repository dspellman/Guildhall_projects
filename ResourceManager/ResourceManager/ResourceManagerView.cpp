// ResourceManagerView.cpp : implementation of the CResourceManagerView class 
// 
#include "stdafx.h" 
#include "ResourceManager.h" 
#include "ResourceManagerDoc.h" 
#include "LeftView.h"
#include "ResourceManagerView.h" 
#include "InsertDialog.h"

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
	ON_WM_KEYDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_CHAR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////// 
// CResourceManagerView construction/destruction 
CResourceManagerView::CResourceManagerView() : width(0), height(0)
{
	m_font.CreatePointFont(80, _T("MS Sans Serif"));
	m_scale = 0;
}

CResourceManagerView::~CResourceManagerView()
{
}

BOOL CResourceManagerView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CScrollView::PreCreateWindow(cs);
}

//////////////////////////////////////////////////////
// Initialize the area to draw in
void CResourceManagerView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
	m_scale = 2; // default to double canvas space, but could be larger for a dense graph
	m_nCurrentRow = 0;
	m_nCurrentCol = 0;
	m_bSmooth = FALSE;
	CClientDC dc(this);
	m_nCellWidth = dc.GetDeviceCaps(LOGPIXELSX);
	m_nCellHeight = dc.GetDeviceCaps(LOGPIXELSY) / 3;
	m_nRibbonWidth = m_nCellWidth / 2;

	CResourceManagerDoc* pDoc = GetDocument();
	ResourceList& reslist = pDoc->GetResourceList();

	if (!reslist.Credible())
	{
		SetScrollSizes(MM_TEXT, CSize(m_nRibbonWidth, 0));
		return; // Cannot do anything with bad data
	}
	pair<int, int> levels = reslist.SizeGrid();
	width = m_scale * levels.first; // the canvas is double sized to allow room between nodes for drawing
	height = m_scale * levels.second;
	SetScrollSizes(MM_TEXT, CSize(width * m_nCellWidth + m_nRibbonWidth, height * m_nCellHeight));
	m_selection = m_leftView->GetSelection(); // set the default pointer position (tree view gets created first)
}

///////////////////////////////////////////////////////////////////////////////////
void CResourceManagerView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)

{
	CResourceManagerDoc* pDoc = GetDocument();
	ResourceList& reslist = pDoc->GetResourceList();
	CClientDC dc(this);

	if (!reslist.Credible())
		return; // Cannot do anything with bad data
	unordered_map<string, GraphNode> nodes = reslist.GetNodes();

	Invalidate();
	pair<int, int> levels = reslist.SizeGrid();
	width = m_scale * levels.first; // the canvas is double sized to allow room between nodes for drawing
	height = m_scale * levels.second;
	SetScrollSizes(MM_TEXT, CSize(width * m_nCellWidth + m_nRibbonWidth, height * m_nCellHeight));

	m_selection = m_leftView->GetSelection(); // set the default pointer position (tree view gets updated first)
}

/////////////////////////////////////////////////////////////////////////// 
// Update the viewport drawing routine
void CResourceManagerView::OnDraw(CDC* pDC)
{
	CResourceManagerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CSize size = GetTotalSize(); // the scroll window area we have to work with

	ResourceList& reslist = pDoc->GetResourceList();
	if (!reslist.Credible())
		return; // Cannot do anything with bad data
	unordered_map<string, GraphNode> nodes = reslist.GetNodes();

	for (auto root : reslist.RootOrder())
		DrawTree(pDC, root, nodes);
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

CResourceManagerDoc* CResourceManagerView::GetDocument() // non-debug version is inline 
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CResourceManagerDoc)));
	return (CResourceManagerDoc*)m_pDocument;
}
#endif //_DEBUG 

/////////////////////////////////////////////////////////////////////////// 
// CResourceManagerView message handlers 
void CResourceManagerView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CResourceManagerDoc* pDoc = GetDocument();
	ResourceList& reslist = pDoc->GetResourceList();
	if (!reslist.Credible())
	{
		CScrollView::OnLButtonDown(nFlags, point);
		return; // Cannot do anything with bad data
	}

	unordered_map<string, GraphNode> nodes = reslist.GetNodes();

	pair<int, int> levels = reslist.SizeGrid();
	width = m_scale * levels.first; // the canvas is double sized to allow room between nodes for drawing
	height = m_scale * levels.second;
	SetScrollSizes(MM_TEXT, CSize(width * m_nCellWidth + m_nRibbonWidth, height * m_nCellHeight));
	CClientDC dc(this);
	OnPrepareDC(&dc);

	// Convert the click point to logical coordinates.     
	CPoint pos = point;
	dc.DPtoLP(&pos);
	// If a cell with data in it was clicked, move the cell pointer.     
	CSize size = GetTotalSize();
	if (pos.x > m_nRibbonWidth && pos.x < size.cx && pos.y > m_nCellHeight && pos.y < size.cy)
	{
		int row = (pos.y - m_nCellHeight) / m_nCellHeight;
		int col = (pos.x - m_nRibbonWidth) / m_nCellWidth;
		for (auto node : nodes)
			if (m_scale * node.second.Row() == row && m_scale * node.second.Column() == col)
			{
				if (node.first != m_selection) // only update if its a new selection
				{
					UpdateSelection(node.first); // updates m_selection
					m_leftView->SynchronizeSelection(m_selection); // synchronize the left view pane, too
				}
				break; // we found the right cell, so stop searching
			}
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CResourceManagerView::DrawTree(CDC *pDC, string current, unordered_map<string, GraphNode>& nodes)
{

	DrawLines(pDC, current, nodes); // draw lines first to be behind the nodes
	DrawNodes(pDC, nodes); // draw nodes second to avoid obscuring them in a dense graph
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CResourceManagerView::DrawLines(CDC *pDC, string current, unordered_map<string, GraphNode>& nodes)
{
	CPen* pOldPen;
	CPen blue(PS_SOLID, 0, RGB(0, 0, 255));
	CPen red(PS_SOLID, 0, RGB(255, 0, 0));
	CRect rect;
	CRect rectParent;

	GetCellRect(nodes[current].Row(), nodes[current].Column(), &rect);
	rect.left--; // don't draw into border pixels

	bool active = nodes[current].Active(); // get whatever the node is right now
	for (auto child : nodes[current].Children())
		active = DrawLines(pDC, child, nodes) && active; // draw from bottom up to get active status

	for (auto parent : nodes[current].Parents())
	{
		pair <double, double> ratios = FindRatios(current, parent, nodes);
		// draw the lines to the parents
		if (active)
			pOldPen = pDC->SelectObject(&blue);
		else
			pOldPen = pDC->SelectObject(&red);

		GetCellRect(nodes[parent].Row(), nodes[parent].Column(), &rectParent);

		int triScale = 3;
		pDC->MoveTo(rectParent.right, (int)(rectParent.top - (rectParent.top - rectParent.bottom) * ratios.second + 0.5));
		pDC->LineTo(rect.left - triScale, (int)(rect.top - (rect.top - rect.bottom) * ratios.first + 0.5));
		pDC->MoveTo(rect.left, (int)(rect.top - (rect.top - rect.bottom) * ratios.first + 0.5));
		pDC->LineTo(rect.left - triScale, (int)(rect.top - (rect.top - rect.bottom) * ratios.first + 0.5) + triScale);
		pDC->LineTo(rect.left - triScale, (int)(rect.top - (rect.top - rect.bottom) * ratios.first + 0.5) - triScale);
		pDC->LineTo(rect.left, (int)(rect.top - (rect.top - rect.bottom) * ratios.first + 0.5));

		pDC->SelectObject(pOldPen); // clean up
	}

	return active; // whatever conclusion we drew at this level gets propagated back up the tree
}

////////////////////////////////////////////////////////////////////////////////////////////
void CResourceManagerView::DrawNodes(CDC *pDC, unordered_map<string, GraphNode>& nodes)
{
	CPen* pOldPen;
	CPen blue(PS_SOLID, 0, RGB(0, 0, 255));
	CPen red(PS_SOLID, 0, RGB(255, 0, 0));
	CRect rect;

	for (auto current : nodes)
	{
		GetCellRect(current.second.Row(), current.second.Column(), &rect);
		CBrush brush;
		brush.CreateStockObject(LTGRAY_BRUSH);
		pDC->FillRect(rect, &brush);
		pDC->Draw3dRect(rect, RGB(255, 255, 255), RGB(128, 128, 128));
		if (current.second.Row() == m_nCurrentRow && current.second.Column() == m_nCurrentCol)
		{
			m_selection = current.first;
			DrawPointer(pDC, m_nCurrentRow, m_nCurrentCol, TRUE);
		}
		DrawAddress(pDC, current.second.Row(), current.second.Column(), CString(current.first.c_str()), current.second.Active());
	}
}

///////////////////////////////////////////////////////////////////////////
// Find the print position ratio for the two ends of the line between nodes
pair <double, double> CResourceManagerView::FindRatios(string current, string parent, unordered_map<string, GraphNode> nodes)
{
	double atParent;
	double atCurrent;
	int ordinal = 0;

	for (auto index : nodes[current].Parents())
		if (nodes[index].Row() < nodes[parent].Row())
			ordinal++; // count the number of lines above the one we are drawing
	atCurrent = (double)(ordinal + 1) / (nodes[current].Parents().size() + 1);

	ordinal = 0;
	for (auto index : nodes[parent].Children())
		if (nodes[index].Row() < nodes[current].Row())
			ordinal++; // count the number of lines above the one we are drawing
	atParent = (double)(ordinal + 1) / (nodes[parent].Children().size() + 1);

	return make_pair(atCurrent, atParent);
}

void CResourceManagerView::DrawPointer(CDC *pDC, int row, int col, bool bHighlight)
{
	CRect rect;
	GetCellRect(row, col, &rect);
	CBrush brush(bHighlight ? RGB(0, 255, 255) : LTGRAY_BRUSH);
	pDC->FillRect(rect, &brush);
	pDC->Draw3dRect(rect, RGB(255, 255, 255), RGB(128, 128, 128));
}

void CResourceManagerView::DrawAddress(CDC *pDC, int row, int col)
{
	CRect rect;
	GetCellRect(row, col, &rect);
	CString string;
	string.Format(_T("%c%d"), col + _T('A'), row + 1);
	pDC->SetBkMode(TRANSPARENT);
	CFont* pOldFont = pDC->SelectObject(&m_font);
	pDC->DrawText(string, rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	pDC->SelectObject(pOldFont);
}

void CResourceManagerView::DrawAddress(CDC *pDC, int row, int col, CString label, bool active)
{
	CRect rect;
	GetCellRect(row, col, &rect);
	pDC->SetBkMode(TRANSPARENT);
	CFont* pOldFont = pDC->SelectObject(&m_font);
	if (!active)
		pDC->SetTextColor(RGB(255, 0, 0));
	else
		pDC->SetTextColor(COLOR_BTNTEXT);
	pDC->DrawText(label, rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	pDC->SelectObject(pOldFont);
}

void CResourceManagerView::GetCellRect(int row, int col, LPRECT pRect)
{
	row *= m_scale;
	col *= m_scale;
	pRect->left = m_nRibbonWidth + (col * m_nCellWidth) + 1;
	pRect->top = m_nCellHeight + (row * m_nCellHeight) + 1;
	pRect->right = pRect->left + m_nCellWidth - 1;
	pRect->bottom = pRect->top + m_nCellHeight - 1;
}

// Sets a pointer to the left view (called from main window)
void CResourceManagerView::SetLeftView(CLeftView* left)
{
	m_leftView = left;
}

// Called by left view when a new item is selected to synchronize views
void CResourceManagerView::SynchronizeSelection(string name)
{
	if (m_selection == name)
		return; // already selected, so don't do the work
	UpdateSelection(name);
}

void CResourceManagerView::UpdateSelection(string name)
{
	CResourceManagerDoc* pDoc = GetDocument();
	ResourceList& reslist = pDoc->GetResourceList();
	if (!reslist.Credible())
		return; // Cannot do anything with bad data
	unordered_map<string, GraphNode> nodes = reslist.GetNodes();

	//Invalidate();
	pair<int, int> levels = reslist.SizeGrid();
	width = m_scale * levels.first; // the canvas is double sized to allow room between nodes for drawing
	height = m_scale * levels.second;
	SetScrollSizes(MM_TEXT, CSize(width * m_nCellWidth + m_nRibbonWidth, height * m_nCellHeight));
	CClientDC dc(this);
	OnPrepareDC(&dc);

	DrawPointer(&dc, m_nCurrentRow, m_nCurrentCol, FALSE);
	for (auto pair : nodes)
		if (pair.second.Row() == m_nCurrentRow && pair.second.Column() == m_nCurrentCol)
		{
			CRect rect;
			GetCellRect(pair.second.Row(), pair.second.Column(), &rect);
			CBrush brush;
			brush.CreateStockObject(LTGRAY_BRUSH);
			dc.FillRect(rect, &brush);
			dc.Draw3dRect(rect, RGB(255, 255, 255), RGB(128, 128, 128));
			DrawAddress(&dc, pair.second.Row(), pair.second.Column(), CString(pair.first.c_str()), pair.second.Active());
			break; // found node, so finished
		}
	for (auto pair : nodes)
		if (pair.first == name)
		{
			m_nCurrentRow = pair.second.Row();
			m_nCurrentCol = pair.second.Column();
			DrawPointer(&dc, m_nCurrentRow, m_nCurrentCol, TRUE);
			CRect rect;
			GetCellRect(m_nCurrentRow, m_nCurrentCol, &rect);
			dc.Draw3dRect(rect, RGB(255, 255, 255), RGB(128, 128, 128));
			DrawAddress(&dc, pair.second.Row(), pair.second.Column(), CString(pair.first.c_str()), pair.second.Active());
			break; // found element so done
		}
	m_selection = name;
}

////////////////////////////////////////////////////////////////////////////
// Key command handler for delete and anything else I have time to implement
void CResourceManagerView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	// ctrl-D is 4
	// ctrl-I is 9
	if (nChar == 4) // ctrl-D key
	{
		CString text(m_selection.c_str());
		CString question("Delete selected node: ");
		question += text;
		question += " ?";
		int response = AfxMessageBox(question, MB_YESNO);
		if (response == IDYES)
		{
			// synchronize active states in both views
			CResourceManagerDoc* pDoc = GetDocument();
			pDoc->DeleteNode(m_selection);
			ResourceList& reslist = pDoc->GetResourceList();
			m_selection.clear(); // in case this is the last node in the graph
			for (auto current : reslist.GetNodes())
				if (current.second.Row() == 0 && current.second.Column() == 0)
				{
					m_selection = current.first;
					m_nCurrentRow = 0;
					m_nCurrentCol = 0;
					UpdateSelection(m_selection);
					m_leftView->SynchronizeSelection(m_selection);
					break; // found what we wanted, so done
				}
		}
	}
	else
		if (nChar == 9) // ctrl-I key
			InsertNewNodePair(); // offer to insert a new node pair
		else
			CScrollView::OnChar(nChar, nRepCnt, nFlags);
}

//////////////////////////////////////////////////////////////////////
void CResourceManagerView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CScrollView::OnRButtonDown(nFlags, point);
	CResourceManagerDoc* pDoc = GetDocument();
	ResourceList& reslist = pDoc->GetResourceList();
	if (!reslist.Credible())
		return; // Cannot do anything with bad data
	unordered_map<string, GraphNode> nodes = reslist.GetNodes();
	CClientDC dc(this);

	// Convert the click point to logical coordinates.     
	CPoint pos = point;
	OnPrepareDC(&dc);
	dc.DPtoLP(&pos);
	// If a cell with data in it was clicked, offer to add or delete an edge as appropriate.     
	CSize size = GetTotalSize();
	if (pos.x > m_nRibbonWidth && pos.x < size.cx && pos.y > m_nCellHeight && pos.y < size.cy)
	{
		int row = (pos.y - m_nCellHeight) / m_nCellHeight;
		int col = (pos.x - m_nRibbonWidth) / m_nCellWidth;
		for (auto node : nodes)
			if (m_scale * node.second.Row() == row && m_scale * node.second.Column() == col)
			{
				if (node.first == m_selection) 
				{
					// does not do anything for now
					// if we handle case of selecting the selection--
					//could be deactivate or delete node or cascading version...
					//AfxMessageBox(_T("Selection"));
				}
				else
				{
					// Offer a choice to add or remove an edge connection
					for (auto child : nodes[m_selection].Children())
						if (child == node.first)
						{
							CString text(m_selection.c_str());
							CString question("Delete edge: ");
							question += text;
							question += " => ";
							text = node.first.c_str();
							question += text;
							question += " ?";
							int response = AfxMessageBox(question, MB_YESNO);
							if (response == IDYES)
							{
								// synchronize active states in both views
								CResourceManagerDoc* pDoc = GetDocument();
								pDoc->DeleteEdge(m_selection, node.first);
							}
							return; // handled click for selected cell
						}
					for (auto parent : nodes[m_selection].Parents())
						if (parent == node.first)
						{
							CString text(node.first.c_str());
							CString question("Delete edge: ");
							question += text;
							question += " => ";
							text = m_selection.c_str();
							question += text;
							question += " ?";
							int response = AfxMessageBox(question, MB_YESNO);
							if (response == IDYES)
							{
								// synchronize active states in both views
								CResourceManagerDoc* pDoc = GetDocument();
								pDoc->DeleteEdge(node.first, m_selection);
							}
							return; // handled click for selected cell
						}
					// if nodes are not connected, then offer to connect them
					// simplifying assumption is that the current selection is
					// the parent and we are adding a dependency to the hierarchy
					CString text(m_selection.c_str());
					CString question("Insert edge: ");
					question += text;
					question += " => ";
					text = node.first.c_str();
					question += text;
					question += " ?";
					int response = AfxMessageBox(question, MB_YESNO);
					if (response == IDYES)
					{
						// synchronize active states in both views
						CResourceManagerDoc* pDoc = GetDocument();
						pDoc->InsertEdge(m_selection, node.first);
					}
					return; // handled click for selected cell
				}
				return; // we found the selected cell, so this click is handled
			}
	}
	// right click outside of graph nodes, so offer to add a new node
	InsertNewNodePair();
}

///////////////////////////////////////////////////////////////////////////
// Basic dialog to enter a new node pair.  Allows entering brand new nodes
// to expand the graph and to bootstrap an empty graph.
void CResourceManagerView::InsertNewNodePair()
{
	CInsertDialog insertDlg(this);
	insertDlg.DoModal();
	CResourceManagerDoc* pDoc = GetDocument();
	string parent(CW2A(insertDlg.m_parent.GetString()));
	string child(CW2A(insertDlg.m_child.GetString()));
	if (!parent.empty() && !child.empty())
		pDoc->InsertEdge(parent, child);
}
