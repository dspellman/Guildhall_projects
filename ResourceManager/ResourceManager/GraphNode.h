#pragma once

// We could have named this the node class, but as a sparse graph representation
// it made sense to define nodes in terms of their edge connections instead
class GraphNode
{
	friend class ResourceList;
private:
	set<string> roots; // the root values that depend on this node
	set<string> parents; // the immediate parents of this node
	set<string> children; // the children this node depends on
	int column; // the minimum depth in the forest of trees
	int row; // the current position of the element in the displayed graph grid
	bool active; // flag whether node is inactive (directly or due to child dependencies)

public:
	string value; // the key value for this node

	GraphNode();
	GraphNode(string val);
	~GraphNode();
	int AddParent(string link);
	int AddChild(string link);
	int Column() { return column; };
	int Row() { return row; };
	int Active() { return active; };
	set<string>& Roots() { return roots; };
	set<string>& Parents() { return parents; };
	set<string>& Children() { return children; };
	void ToggleActive();
};

