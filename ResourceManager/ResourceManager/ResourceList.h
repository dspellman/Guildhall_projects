#pragma once

// ResourceList command target

class ResourceList : public CObject
{
private:
	bool credible = false; // data is not valid yet
	multimap<string, string> resource; // the raw data for the edge definition of the graph
	unordered_map<string, GraphNode> nodes; // the list of nodes in the graph
	vector<string> rootOrder; // an ordered list of codependent root nodes

public:
	ResourceList();
	virtual ~ResourceList();
	void Serialize(CArchive& archive);

	bool Credible() { return credible; } // tell whether the data is valid
	vector<string> RootOrder() { return rootOrder; }

	multimap<string, string>& GetResources() { return resource; };  // convenience accessors
	unordered_map<string, GraphNode>& GetNodes() { return nodes; };

	bool CreateParentChildLists();
	bool DepthFirstTraversal(string curr, set<string>& wSet, set<string>& gSet, set<string>& bSet);
	bool TestCycle(set<string> &gSet);
	void SetLevel(string node);
	void Tangle(string current, set<string>& roots);
	void Untangle();
	pair<int, int> SizeGrid();
	void GridLayout();
	void DepthFirstLayout(vector<int>& heights, int minimum, string current);
	void DeleteNode(string name);
	void DeleteEdge(string parent, string child);
	bool InsertEdge(string parent, string child);
};
