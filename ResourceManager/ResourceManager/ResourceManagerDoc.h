
// ResourceManagerDoc.h : interface of the CResourceManagerDoc class
//


#pragma once


class CResourceManagerDoc : public CDocument
{
protected: // create from serialization only
	CResourceManagerDoc();
	DECLARE_DYNCREATE(CResourceManagerDoc)

// Attributes
public:
	ResourceList rlist;
	int count = 0;

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void OnEditClearAll();
	virtual void DeleteContents();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CResourceManagerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	ResourceList& GetResourceList(); //accessor to model for view

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	// Called by CLeftView when a check box changes state
	void ToggleActive(string node, CView* pSender);
	void DeleteNode(string name);
	void DeleteEdge(string parent, string child);
	void InsertEdge(string parent, string child);
};
