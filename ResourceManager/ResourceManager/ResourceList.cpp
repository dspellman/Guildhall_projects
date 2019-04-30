// ResourceList.cpp : implementation file
//

#include "stdafx.h"
#include "ResourceManager.h"
#include "ResourceList.h"


// ResourceForest

ResourceList::ResourceList()
{
}

ResourceList::~ResourceList()
{
	resource.clear();
}

// ResourceForest member functions

void ResourceList::Serialize(CArchive& archive)
{
	string item;
	string dependency;
	char ch;
	int count;

	if (archive.IsStoring()) 
	// writing file
	{
		// we have to format the file as text since serialization is binary
		char space[] = { ' ' };
		char crlf[] = { '\r', '\n' };
		for (auto pair : resource)
		{
			archive.Write(pair.first.c_str(), pair.first.size());
			archive.Write(space, 1);
			archive.Write(pair.second.c_str(), pair.second.size());
			archive.Write(crlf, 2);
		}
	}
	else 
	// reading file
	{
		while (true) // read the file until the bitter end
		{
			item.clear(); // empty the scratch strings for each pair reading
			do
			{
				count = archive.Read(&ch, 1);
				if (count != 1)
					return; // we have reached the end of the list
				if (!isspace(ch))
					item.push_back(ch);
			}
			while (!isspace(ch));

			dependency.clear();
			do
			{
				count = archive.Read(&ch, 1);
				if (count != 1)
				{
					if (dependency.size() > 0)
						resource.insert(pair<string, string>(item, dependency)); // if we have anything, assume its good and insert it
					return; // this is a possibly incomplete or corrupt file (no EOL terminator)
							// we degrade gracefully by using what we read successfully
				}
				if (!isspace(ch))
					dependency.push_back(ch);
				else
					resource.insert(pair<string, string>(item, dependency)); // read a pair successfully so insert it

				if (ch == 13)
				{
					count = archive.Read(&ch, 1); // handle the \r\n combination
					if (count != 1)
						return; // we have reached the end of the list (should not happen)
				}
			} while (!isspace(ch));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Create a list of all the nodes with lists of all the parent and child node links
// Effectively this is a sparse matrix representation of the directed graph
// 
bool ResourceList::CreateParentChildLists()
{
	nodes.clear(); // make sure the list is empty when we start
	// Build the list of nodes and populate the parent and child link lists
	for (auto pair : resource)
	{
		if (nodes.count(pair.first) == 0)
			nodes[pair.first] = GraphNode(pair.first);
		if (nodes[pair.first].AddChild(pair.second))
			; // handle duplicate link error (effectively ignores duplicates)

		if (nodes.count(pair.second) == 0)
			nodes[pair.second] = GraphNode(pair.second);
		if (nodes[pair.second].AddParent(pair.first))
			; // handle duplicate link error (effectively ignores duplicates)
	}

	// Test for cycles in the graph
	set<string> cycle; // the path being checked
	if (TestCycle(cycle))
	{
		// This output is hard to make sense of, need to use a list instead of a set
		/* 
		string warning("A cycle was found comprised of the following items:\n\n");
		int length = 0;
		for (auto item : cycle)
		{
			warning += item;
			warning += ", ";
			length += item.size() + 2;
			if (length > 80)
			{
				length = 0;
				warning += "\n"; // try to avoid a mesage too wide for the screen
			}
		}
		warning.pop_back(); // remove final comma and put a period
		warning.pop_back();
		warning += ".";
		CString str(warning.c_str());
		AfxMessageBox(str, MB_OK | MB_ICONSTOP);
		*/
		AfxMessageBox(_T("A cycle was found"), MB_OK | MB_ICONSTOP);
		credible = false; // flag that the data is invalid
		return false; // cycle error
	}

	// if no cycles exist then determine the maximum depth for the nodes in their trees
	for (auto pair : nodes)
		SetLevel(pair.first);
	Untangle();	// order the root nodes for a less cluttered and prettier display format
	GridLayout(); // determine a grid layout for the directed graph with some useful properties
	credible = true; // flag that the data is apparently good
	return true; // created graph without errors
}

//////////////////////////////////////////////////////
// Depth first search for a cycle in the graph
bool ResourceList::DepthFirstTraversal(string current, set<string>&open, set<string>&path, set<string>&closed) {
	//moving curr to white set to grey set.
	open.erase(open.find(current));
	path.insert(current);

	for (auto child : nodes[current].children)
	{    //for all neighbour nodes
		if (closed.find(child) != closed.end())
			continue;    //if the nodes are in the black set, skip them
		if (path.find(child) != path.end())
		{

			return true;    //it is a cycle because the node is on the path
		}
		if (DepthFirstTraversal(child, open, path, closed))
			return true;    //cycle found at lower level
	}

	//moving v to grey set to black set.
	path.erase(path.find(current));
	closed.insert(current);
	return false;
}

/////////////////////////////////////////////////////////////
// returns true if cycle, false if not
bool ResourceList::TestCycle(set<string> &path) {
	set<string> open, closed;    // open and closed list of nodes processed
	for (auto pair : nodes)
		open.insert(pair.first);    //initially add all nodes into the open set

	while (open.size() > 0) {
		for (auto pair : nodes)
		{
			if (open.find(pair.first) != open.end())
				if (DepthFirstTraversal(pair.first, open, path, closed))
					return true; // found a cycle
		}
	}

	return false; // no cycle found
}

////////////////////////////////////////////////////////////////
void ResourceList::SetLevel(string current)
{
	for (auto child : nodes[current].children)
	{
		if (nodes[child].column < nodes[current].column + 1)
			nodes[child].column = nodes[current].column + 1; // set minimum depth level to one more than any parent node
		SetLevel(child); // recursively update all down stream node levels
	}
	
	vector<int> depths;
}

////////////////////////////////////////////////////////////////////
// Creates a list of root nodes grouped by codendencies in child trees
// Sorts in a first come, first served ordering but could be more sophisticated
// The idea is to minimize criss-crossing edges in the graph display since 
// game resource graphs are sparse and should have few connections anyway
void ResourceList::Untangle()
{
	rootOrder.clear(); // reset the previous ordering of root nodes
	set<string> coroots; // the list of root nodes tied by common dependencies
	set<string> open; // the list of root nodes not processed yet

	for (auto element : nodes)
		Tangle(element.first, nodes[element.first].Roots()); // find the set of root nodes depending on this node

	for (auto element : nodes)
		if (element.second.parents.empty())
			open.insert(element.first); // if the node has no parent, it is a root node on our list to process

	for (auto root : nodes)
	{
		if (open.find(root.first) == open.end())
			continue; // this is not a node on our list to process (not root or already done)
		coroots.insert(root.first); // the root node is always in the list
		for (auto element : nodes)
			if (element.second.roots.find(root.first) != element.second.roots.end())
				coroots.insert(element.second.roots.begin(), element.second.roots.end()); // collect all coroots found

	// process the set of coroots together since they share dependent nodes
		for (auto root : coroots)
			rootOrder.push_back(root); // can be sorted by some criteria such as priority instead of default set sorting
		for (auto done : coroots)
			open.erase(done); // remove the processed nodes from our open list
		coroots.clear(); // clear the set for the next iteration
	}
}

//////////////////////////////////////////////////////////////////////////
// Finds all the root nodes that ultimate depend on a given node
void ResourceList::Tangle(string current, set<string>& roots)
{
	if (nodes[current].parents.empty())
		roots.insert(current);
	else
		for (auto parent : nodes[current].parents)
			Tangle(parent, roots);
}

/////////////////////////////////////////////////////////////////////
// Determine the size of the grid to show the graph one node per cell
pair<int, int> ResourceList::SizeGrid()
{
	int height = 0;
	int width = 0;
	for (auto pair : nodes)
	{
		if (width < pair.second.Column())
			width = pair.second.Column(); // find the maximum in the array
		if (height < pair.second.Row())
			height = pair.second.Row(); // find the maximum in the array
	}
	return make_pair(width + 1, height + 1);
}

///////////////////////////////////////////////////////////////////////
// Create a grid layout of the directed graph with some useful topology 
// for searching the display space for user interactions
void ResourceList::GridLayout()
{
	vector<int> heights;
	int max = 0; // maximum height of the grid
	for (auto pair : nodes)
		nodes[pair.first].row = -1; // flag each node as needing updated height
	for (auto root : rootOrder)
	{
		DepthFirstLayout(heights, max, root); // layout a tree using the current height start
		for (auto index : heights)
			if (max < index)
				max = index; 
		// vertically justify the heights for each new tree
		for (int index = 0; index < heights.size(); index++)
			heights[index] = max;
	}
}

//////////////////////////////////////////////////////////
// Recursive helper function for laying out the graph grid
void ResourceList::DepthFirstLayout(vector<int>& heights, int minimum, string current)
{
	while (heights.size() < nodes[current].column + 1)
		heights.push_back(minimum); // extend array as needed with starting minimum default
	if (nodes[current].row < 0) // update node location in grid if not set already
	{
		nodes[current].row = heights[nodes[current].column]; // assign the current position
		heights[nodes[current].column]++; // set the next grid position at the level
	}
	for (auto child : nodes[current].children)
		DepthFirstLayout(heights, minimum, child);
}

/////////////////////////////////////////////////////////////////////////////////
// Deletes a node from the tree and handles breaking and making links
// Based on the resource file format, a simplifying assumption is made that 
// removing a node will result in parent nodes becoming dependent on
// the deleted node's children to conserve the graph.  Otherwise, we
// have a messy problem of what to do about the links and orphaned nodes.
// However, an orphan node with no remaining dependencies will be lost.
// The assumption is that since the resource file only defines edges, unconnected
// nodes do not belong in the graph.  It can be added again by editing, of course.
void ResourceList::DeleteNode(string name)
{
	// make connections between parents and children to preserve graph integrity

	for (auto parent : nodes[name].parents)
		for (auto child : nodes[name].children)
			if (nodes[parent].children.find(child) == nodes[parent].children.end())
				resource.insert(pair<string, string>(parent, child)); // add any child links not already in the list

	resource.erase(name); // remove edges where this is the parent node

	// remove edges where this is the child node
	for (auto iter = resource.begin(); iter != resource.end(); )
		(*iter).second == name ? resource.erase(iter++) : ++iter;
}

////////////////////////////////////////////////////////////////
void ResourceList::DeleteEdge(string parent, string child)
{
	pair <multimap<string, string>::iterator, multimap<string, string>::iterator> range;
	range = resource.equal_range(parent);
	for (auto iter = range.first; iter != range.second; )
		(*iter).second == child ? resource.erase(iter++) : ++iter;
}

////////////////////////////////////////////////////////////////
bool ResourceList::InsertEdge(string parent, string child)
{
	resource.insert(pair<string, string>(parent, child));										  
	// Test for cycles in the graph before finalizing adding the edge
	if (!CreateParentChildLists())
	{
		DeleteEdge(parent, child); // caller will update the view as needed
		return false;
	}
	return true;
}
