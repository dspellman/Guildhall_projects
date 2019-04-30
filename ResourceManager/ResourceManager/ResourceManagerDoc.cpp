
// ResourceManagerDoc.cpp : implementation of the CResourceManagerDoc class
//

#include "stdafx.h"

// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "ResourceManager.h"
#endif

#include "ResourceManagerDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CResourceManagerDoc

IMPLEMENT_DYNCREATE(CResourceManagerDoc, CDocument)

BEGIN_MESSAGE_MAP(CResourceManagerDoc, CDocument)
END_MESSAGE_MAP()


// CResourceManagerDoc construction/destruction

CResourceManagerDoc::CResourceManagerDoc()
{
	// TODO: add one-time construction code here
}

CResourceManagerDoc::~CResourceManagerDoc()
{
}


BOOL CResourceManagerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

BOOL CResourceManagerDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	if (!rlist.CreateParentChildLists()) // build and verify the sparse tree
	{
		// false means we encountered a cycle, so we cannot process the file
		return FALSE;
	}

	return TRUE;
}

// This example is the handler for an Edit Clear All command.
void CResourceManagerDoc::OnEditClearAll()
{
	DeleteContents();
	UpdateAllViews(NULL);
}

void CResourceManagerDoc::DeleteContents()
{
	// Re-initialize document data here.
}

// CResourceManagerDoc serialization

void CResourceManagerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
		rlist.Serialize(ar);
	}
	else
	{
		// TODO: add loading code here
		rlist.Serialize(ar);
	}
}

ResourceList& CResourceManagerDoc::GetResourceList()
{
	return rlist;
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CResourceManagerDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CResourceManagerDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CResourceManagerDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CResourceManagerDoc diagnostics

#ifdef _DEBUG
void CResourceManagerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CResourceManagerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CResourceManagerDoc commands

/////////////////////////////////////////////////////
// Called by CLeftView when a check box changes state
// We assume this will never get out of synchronization
void CResourceManagerDoc::ToggleActive(string name, CView* pSender)
{
	unordered_map<string, GraphNode>& nodes = rlist.GetNodes();
	for (auto node : nodes)
	{
		if (node.first == name)
		{
			nodes[node.first].ToggleActive();
			UpdateAllViews(pSender, 0, 0);
			break; // found the node so we are done
		}
	}
}

//////////////////////////////////////////////////////
// Delete a node selected in one of the views
void CResourceManagerDoc::DeleteNode(string name)
{
	rlist.DeleteNode(name);
	rlist.CreateParentChildLists(); // rebuild the tree (should not have error)
	UpdateAllViews(NULL);
}

void CResourceManagerDoc::DeleteEdge(string parent, string child)
{
	rlist.DeleteEdge(parent, child);
	rlist.CreateParentChildLists(); // rebuild the tree (should not have error)
	UpdateAllViews(NULL);
}

void CResourceManagerDoc::InsertEdge(string parent, string child)
{
	// make sure that adding the edge does not form a cycle
	rlist.InsertEdge(parent, child); // tests for cycles, but do view update either way
	rlist.CreateParentChildLists(); // rebuild the tree (should not have error at this point)
	UpdateAllViews(NULL);
}
