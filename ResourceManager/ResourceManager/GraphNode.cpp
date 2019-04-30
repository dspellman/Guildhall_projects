#include "stdafx.h"
#include "GraphNode.h"


GraphNode::GraphNode()
{
	column = 0;
}

GraphNode::GraphNode(string val) : value(val)
{
	column = 0;
	if (val.size() == 0)
		val = "ERROR_ZERO"; // should never happen but if it did...
};


GraphNode::~GraphNode()
{
}

// The add functions test whether it is a duplicate link
// but duplicates will not harm the functionality since they are effectively ignored

int GraphNode::AddParent(string link)
{
	if (parents.count(link))
		return 1; // the link already exists, so duplicate error
	parents.insert(link);
	return 0; // new link so okay
}

int GraphNode::AddChild(string link)
{
	if (children.count(link))
		return 1; // the link already exists, so duplicate error
	children.insert(link);
	return 0; // new link so okay
}

///////////////////////////////////////
// Toggles the active state of the node
void GraphNode::ToggleActive()
{
	active = !active; // assumes that the state is always in synch
}
