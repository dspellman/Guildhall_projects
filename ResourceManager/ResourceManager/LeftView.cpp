
// LeftView.cpp : implementation of the CLeftView class
//

#include "stdafx.h"
#include "ResourceManager.h"

#include "ResourceManagerDoc.h"
#include "ResourceManagerView.h"
#include "LeftView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLeftView

IMPLEMENT_DYNCREATE(CLeftView, CTreeView)

BEGIN_MESSAGE_MAP(CLeftView, CTreeView)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, &CLeftView::OnTvnSelchanged)
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, &CLeftView::OnTvnKeydown)
	ON_NOTIFY_REFLECT(NM_CLICK, &CLeftView::OnNMClick)
	ON_WM_CHAR()
END_MESSAGE_MAP()


// CLeftView construction/destruction

CLeftView::CLeftView()
{
	// TODO: add construction code here
	m_initializing = false;
}

CLeftView::~CLeftView()
{
}

BOOL CLeftView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs

	return CTreeView::PreCreateWindow(cs);
}

void CLeftView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();

	// TODO: You may populate your TreeView with items by directly accessing
	//  its tree control through a call to GetTreeCtrl().
}

void CLeftView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	// TODO: Add your specialized code here and/or call the base class
	DrawTree();
}

/////////////////////////////////////////////////////////////
// Display an alphabetized list of the nodes and their status
void CLeftView::DrawTree()
{
	CTreeCtrl& tree = GetTreeCtrl();
	CResourceManagerDoc* pDoc = GetDocument();
	ResourceList& reslist = pDoc->GetResourceList();
	unordered_map<string, GraphNode> nodes = reslist.GetNodes();
	set<string> names; 

	m_initializing = true;
	tree.DeleteAllItems(); // if we are updating the tree, do it all
	if (!reslist.Credible())
		return;

	SetWindowTheme(tree, L"Explorer", NULL);
	tree.ModifyStyle(NULL, TVS_CHECKBOXES | TVS_SHOWSELALWAYS);

	for (auto node : nodes)
		names.insert(node.first); // create an alphabetized list of elements for the tree

	for (auto name : names)
	{
		HTREEITEM hItem;
		hItem = tree.InsertItem(CString(name.c_str()), TVI_ROOT);
		SetCheckState(m_hWnd, hItem, nodes[name].Active());
	}

	m_initializing = false;
	hSelection = tree.GetRootItem();
	tree.SelectItem(hSelection);
}

// CLeftView diagnostics

#ifdef _DEBUG
void CLeftView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CLeftView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

CResourceManagerDoc* CLeftView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CResourceManagerDoc)));
	return (CResourceManagerDoc*)m_pDocument;
}
#endif //_DEBUG

void CLeftView::SetCheckState(HWND hwndTreeView, HTREEITEM hItem, bool fCheck)
{
	TVITEM tvItem;

	tvItem.mask = TVIF_HANDLE | TVIF_STATE;
	tvItem.hItem = hItem;
	tvItem.stateMask = TVIS_STATEIMAGEMASK;

	// Image 1 in the tree-view check box image list is the unchecked box. 
	// Image 2 is the checked box.
	tvItem.state = INDEXTOSTATEIMAGEMASK((fCheck ? 2 : 1));
	TreeView_SetItem(hwndTreeView, &tvItem);
}

bool CLeftView::GetCheckState(HWND hwndTreeView, HTREEITEM hItem)
{
	TVITEM tvItem;

	// Prepare to receive the desired information.
	tvItem.mask = TVIF_HANDLE | TVIF_STATE;
	tvItem.hItem = hItem;
	tvItem.stateMask = TVIS_STATEIMAGEMASK;

	// Request the information.
	TreeView_GetItem(hwndTreeView, &tvItem);

	// Return zero if it's not checked, or nonzero otherwise.
	return ((bool)(tvItem.state >> 12) - 1);
}

// CLeftView message handlers

///////////////////////////////////////////////////////////////////
// Handles selecting a new item in the tree view
void CLeftView::OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
	if (m_initializing)
		return; // don't update the selection while drawing the tree
	hSelection = pNMTreeView->itemNew.hItem; // save the selection for reference
	// Now get the text of the selected item
	CTreeCtrl& tree = GetTreeCtrl();
	CString text = tree.GetItemText(hSelection);
	// synchronize selection in both views
	string selection(CW2A(text.GetString()));
	m_rightView->SynchronizeSelection(selection);
}

////////////////////////////////////////////////////////////////////////////
// Key command handler for delete and anything else I have time to implement
void CLeftView::OnTvnKeydown(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
	if (true)
		return; // change processing for this
	WORD key = pTVKeyDown->wVKey;
	if (key == 68) // D key
	{
		*pResult = 1; // we handled the keypress, so no further processing
		CTreeCtrl& tree = GetTreeCtrl();
		CString text = tree.GetItemText(hSelection);
		if (text == "")
			return; // no selection or tree is empty
		CString question("Delete selected node: ");
		question += text;
		question += " ?";
		int response = AfxMessageBox(question, MB_YESNO);
		if (response == IDYES)
		{
			// synchronize active states in both views
			CResourceManagerDoc* pDoc = GetDocument();
			string selection(CW2A(text.GetString()));
			pDoc->DeleteNode(selection);

			hSelection = tree.GetRootItem();
			text = tree.GetItemText(hSelection);
			selection = CW2A(text.GetString());
			m_rightView->SynchronizeSelection(selection); // synchornize selections
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Handles checking and unchecking the check box for nodes in the tree
void CLeftView::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	// TODO: Add your control notification handler code here
	UINT uFlags = 0;
	CPoint pt(0, 0);
	GetCursorPos(&pt);
	CTreeCtrl& tree = GetTreeCtrl();
	tree.ScreenToClient(&pt);
	HTREEITEM hItem = tree.HitTest(pt, &uFlags);
	if (NULL != hItem && (TVHT_ONITEMSTATEICON  & uFlags))
	{
		//hSelection = hItem;
		CTreeCtrl& tree = GetTreeCtrl();
		CString text = tree.GetItemText(hItem);
		string selection(CW2A(text.GetString()));

		// synchronize active states in both views
		CResourceManagerDoc* pDoc = GetDocument();
		pDoc->ToggleActive(selection, this);
	}
}

// Set a pointer to the right side view (called from the main window)
void CLeftView::SetRightView(CResourceManagerView* right)
{
	m_rightView = right;
}

/////////////////////////////////////////////////////////////////////
// Set the same selection in the other view to keep them synchronized
void CLeftView::SynchronizeSelection(string selection)
{
	CTreeCtrl& tree = GetTreeCtrl();
	CString text = tree.GetItemText(hSelection);
	CString target(selection.c_str());
	if (target.Compare(text) == 0)
		return; // no change, so don't do the work
	HTREEITEM hRoot = tree.GetRootItem();
	while (hRoot != NULL)
	{
		text = tree.GetItemText(hRoot);
		if (text.Compare(target) == 0)
		{
			hSelection = hRoot; // found the matching node
			tree.SelectItem(hSelection);
			break; // done with loop at this point
		}
		hRoot = tree.GetNextSiblingItem(hRoot);
	}
}

//////////////////////////////////////////////////////////////////////
// provides the current selection to synchronize views (pull version)
string CLeftView::GetSelection()
{
	CTreeCtrl& tree = GetTreeCtrl();
	CString text = tree.GetItemText(hSelection);
	string selection(CW2A(text.GetString()));
	return selection;
}


void CLeftView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// ctrl-D is 4
	// ctrl-I is 9
	// TODO: Add your message handler code here and/or call default
	if (nChar == 4)
	{
		CTreeCtrl& tree = GetTreeCtrl();
		CString text = tree.GetItemText(hSelection);
		if (text == "")
			return; // no selection or tree is empty
		CString question("Delete selected node: ");
		question += text;
		question += " ?";
		int response = AfxMessageBox(question, MB_YESNO);
		if (response == IDYES)
		{
			// synchronize active states in both views
			CResourceManagerDoc* pDoc = GetDocument();
			string selection(CW2A(text.GetString()));
			pDoc->DeleteNode(selection);

			hSelection = tree.GetRootItem();
			text = tree.GetItemText(hSelection);
			selection = CW2A(text.GetString());
			m_rightView->SynchronizeSelection(selection); // synchornize selections
		}
	}
	else
		if (nChar == 9)
			m_rightView->InsertNewNodePair(); // Offer to add a new node pair
		else
			CTreeView::OnChar(nChar, nRepCnt, nFlags);
}
