Basic commands:

left click selects a node in either view.  clicking the check box in the left view will
toggle the active state of the node.  Inactive nodes make their parent nodes "unusable"
and the edges of the graph are marked in red.  This option is an interpretation of the
requirements for the program.  This seemed like a reasonable decision because it could 
be used to actually deactivate nodes in a testing or configuration situation in a real 
game.

Ctrl-D deletes the selected node.  The child dependencies are propagated to the parents 
of the node.  Unwanted dependencies can be deleted before or after deleting a given node.
Orphaned nodes (nodes without a dependency) are lost from the graph.  This is based on the
format of the resource.txt file, which only has edges.  Therefore, the assumption is that
if an entity has no dependencies, it is not part of the dependency hierarchy.  Of course,
a node can be added again by creating it with a dependency pairing using Ctrl-I below.

Ctrl-I inserts node pairings (and creates one or both nodes if they are new).  The names
of nodes are case sensitive.  Due to the format of the resource.txt file, spaces are not
allowed and are automatically replaced with underscore characters.  Nodes are inserted in
pairs because the representation of the graph is based on edges in the resource.txt file.

Right clicking a node in the graph that is not the selected node will offer to delete a
dependency edge between the nodes if it exists, or else offer to create a dependency edge
between the nodes if one does not exist.  The selected node will depend on the right clicked
node, so select the desired parent and right click the child to set the correct direction.

Right clicking empty space will perform the same function as Ctrl-I to insert a new node
pairing.

Generally, the way to move nodes or trees around in the graph is to create a new dependency
between nodes first, then delete nodes or edges that are unwanted.  This avoids creating 
orphan nodes that might disappear if their parent dependencies are deleted.  It also may
identify cycles in the graph before committing to deleting data.  On the other hand, a
cycle in the graph may warrant deleting edges before creating other dependencies (but
having tested the change, this may make it easier to determine what to change).

Ctrl-O opens a file using the windows standard file chooser.  The program requirements
specified opening the default file resource.txt, so the program attempts to do this if it
is found in the local directory.

Ctrl-S will save the file and selecting save as... from the menu allows saving with a new
name as usual in Windows.

Since this is a Windows program, the Q to quit command is not implemented (thus avoiding a
slight danger of inadvertently quitting).  Us the standard menu command or the upper right X
to end the program.

Enhancements that could be made to this program include autocomplete for entering new node
names in the insert command, data validation for new nodes, flagging that the file is changed 
and asking whether to save changes on exit, recursively laying out the graph to minimize edge 
crossings, creating a metric for prioritizing and positioning nodes in the list and graph, and
expanding the save file format to store nodes as well as edges so that the data can represent
all potential entities that can have dependencies (this would allow better data validation,
node insertion and game data integrity).  

Obviously, the user interface could be improved a great deal.  I was tempted to write this using 
QT qanava for a slick interface, but I decided it was better to keep things generic.