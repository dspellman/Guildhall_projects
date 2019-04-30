// Huffman.cpp : Defines the entry point for the console application.
//
// Microsoft specific header
#include "stdafx.h"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

typedef unsigned char byte;

///////////////////////////////////////////////////////////
// The application is implemented by two main classes    //
// CompressedFileWriter does the compression of a file   //
// CompressedFileReader does the uncompression of a file //
// Two helper classes are defined: Code and Node         //
// The file is self-contained to make compiling simple   //
///////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
// A Huffman code object for bit-wise setting and retriving codes
// Defined as a class is to allow changing the underlying data structure
class Code
{
private:
	int length;
	vector<char> code;

public:
	Code() : length(0)
	{
	}

	// Add a bit to the code and return the object
	void Append(char bit)
	{
		code.push_back(bit); // add the new bit to the end of the code
		length++; // count the bit we added
	}

	// Get the length for looping to read the code
	int Length()
	{
		return length;
	}

	// Get the vector for writing the bit pattern to the file
	const vector<char>& Vector()
	{
		return code;
	}

	// Return the one bit at a time for writing the code
	int Bit(int offset)
	{
		if (offset >= 0 && offset < length)
			return code[offset];
		else
			return -1; // out of range
	}
};

//////////////////////////////////////////////////////////////////
// Code Tree node definition, a structure rather than a true class
// because an object with getters and setters is not really an object
class Node
{
public:
	byte ch;
	int freq;
	Node *left, *right;

	Node(char c, int f, Node* lft = nullptr, Node* rght = nullptr)
	{
		ch = c;
		freq = f;
		left = lft;
		right = rght;
	}

	// recursively delete the tree
	~Node()
	{
		delete right;
		delete left;
	}

	////////////////////////////////////////////////////////////////////
	// debug code to show contents of tree in a rather obfuscated format
	static void PrintTree(Node* root, int level, char bit)
	{
		if (root == nullptr)
			return; // end of tree so nothing to print

		PrintTree(root->right, level + 1, '1');  // reverse inorder traversal (90 degree rotation)

		for (int i = 0; i < level; i++)
			printf("     ");
		if (root->right == nullptr)
			printf(isprint(root->ch) ? "%c %c  : %i\n" : "%c %02X : %i\n", bit == '*' ? '=' : bit == '0' ? '\\' : '/', root->ch & 0xFF, root->freq); // leaf node data
		else
			printf("%c %c : %i\n", bit == '*' ? '=' : bit == '0' ? '\\' : '/', bit, root->freq); // internal node data

		PrintTree(root->left, level + 1, '0');
	}
};

///////////////////////////////////////////////////////////////////
// Helper comparison function for the priority queue
// Priority queue default order is highest, but we want the lowest
// frequency first, so define a comparison operation to do that
struct comp
{
	bool operator()(Node* l, Node* r)
	{
		// lower frequency has higher priority
		return l->freq > r->freq;
	}
};

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
// Write compressed file (count, table, data)
class CompressedFileWriter
{
private:
	unordered_set<char> options;
	string inputFile;
	string outputFile;
	ifstream input;
	ofstream output;

	Node * codeTree; // dynamically allocated, so destroy
	int length;

public:
	CompressedFileWriter(string infile, string outfile, unordered_set<char> opts) : inputFile(infile), outputFile(outfile), options(opts)
	{
	}

	~CompressedFileWriter()
	{
		// not strictly necessary, since close is called when stream is destroyed anyway
		if (input.is_open())
			input.close();
		if (output.is_open())
			output.close();

		delete codeTree;
	}

	/////////////////////////////////////////////////////////////////////////////
	// The main function that performs the steps of Huffman compression on a file
	int CompressFile()
	{
		// Create frequency table from file
		unordered_map<byte, int> freq = CountFrequencies(inputFile, options, &length);

		// Create Huffman code tree
		codeTree = CreateTree(freq, options);

		// Create Code table
		unordered_map<char, Code> codeTable = CreateTable(codeTree, options);

		input.open(inputFile, ios::binary);
		output.open(outputFile, ios::binary);
		if (!input.is_open() || !output.is_open())
			return -1; // unexpected file error

		// write table and header information
		// count the number of nodes in the tree
		int size = CountNodes(codeTree);
		//divide 32 bit int values into 4 bytes in a non-endian dependent way
		output.put(static_cast<byte>((size >> 24) & 0xFF));
		output.put(static_cast<byte>((size >> 16) & 0xFF));
		output.put(static_cast<byte>((size >> 8) & 0xFF));
		output.put(static_cast<byte>(size & 0xFF));

		WritePostorder(codeTree); // write the tree data post order to be able to retrieve using a stack

		// Write the number of bytes to be compressed to count when uncompressing
		//divide 32 bit int values into 4 bytes in a non-endian dependent way
		output.put(static_cast<byte>((length >> 24) & 0xFF));
		output.put(static_cast<byte>((length >> 16) & 0xFF));
		output.put(static_cast<byte>((length >> 8) & 0xFF));
		output.put(static_cast<byte>(length & 0xFF));

		// write the compressed bit stream
		int count = 0; // count of bytes processed to compare to expected length
		int bit = 0; // how many bits in buffer
		char buffer = 0; // scratch buffer for building codes

		char ch = input.get(); // prime the read operation

		while(input.good())
		{
			for (int i = 0; i < codeTable[ch].Length(); i++)
			{
				buffer = buffer << 1 | codeTable[ch].Bit(i) - '0';
				if (++bit == 8)
				{
					bit = 0; // reset counter
					output.put(buffer); // write the completed byte
				}
			}
			ch = input.get(); // read file a byte at a time
			count++; // count the bytes processed
		}

		// we need to flush the last byte after left justifying it if needed
		if (bit > 0)
		{
			buffer <<= 8 - bit; // shift up the final bits to their proper position
			output.put(buffer); // output the final partial byte worth of bits
		}

		if (count != length)
			return -3; //printf("ERROR: %i characters processed when %i characters expected\n", count, length);

		input.close();
		output.close();
		return 0; // no apparent errors writing compressed file
	}

private:
	////////////////////////////////////////////////////////////////
	// Read the file contents and count the frequencies of each byte
	unordered_map<byte, int> CountFrequencies(string fileName, unordered_set<char> options, int * size)
	{
		unordered_map<byte, int> frequencyMap;
		ifstream input(fileName, ios::binary);
		if (!input.is_open())
			return frequencyMap; // graceful failure

		byte ch = input.get(); // prime the read operation by getting the first byte if it exists

							   // loop processing bytes from the file until the end
		while (input.good())
		{
			frequencyMap[ch]++; // count the occurance of this byte
			ch = input.get(); // read file a byte at a time
			(*size)++; // count the bytes processed
		}

		if (options.find('f') != options.end() && frequencyMap.size() > 0)
		{
			printf("\nFrequency table:\nchar  frequency\n");
			for (auto pair : frequencyMap)
			{
				printf(isprint(pair.first) ? " %c  : %i\n" : "%02X  : %i\n", pair.first, pair.second);
			}
			printf("\n");
		}

		input.close();
		return frequencyMap;
	}

	///////////////////////////////////////////////////////////////////////
	// Build the Huffman code tree in place in a priority queue
	// priority queue should have a comparator defined for lowest priority
	Node * CreateTree(unordered_map<byte, int> frequencyMap, unordered_set<char> options)
	{
		// Create a priority queue to store nodes of the Huffman tree;
		priority_queue<Node*, vector<Node*>, comp> nodeQueue;

		// Create a leaf node for each character and add it to the priority queue.
		for (auto pair : frequencyMap) {
			nodeQueue.push(new Node(pair.first, pair.second, nullptr, nullptr));
		}

		// repeat until there is only one node in the queue
		while (nodeQueue.size() != 1)
		{
			// Remove the two lowest frequency nodes from the queue
			Node *left = nodeQueue.top();
			nodeQueue.pop();
			Node *right = nodeQueue.top();
			nodeQueue.pop();

			// Create a new internal node with these two nodes as children
			// Make the node's frequency equal to the sum of the two nodes' frequencies. 
			// Push the new node back on to the priority queue.
			int sum = left->freq + right->freq;
			nodeQueue.push(new Node(0, sum, left, right));
		}

		if (options.find('t') != options.end())
		{
			printf("Code tree:\n");
			Node::PrintTree(nodeQueue.top(), 0, '*');
			printf("\n");
		}

		// root node of Huffman Tree is top of priority queue
		return nodeQueue.top();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	// Helper function for CreateTable to traverse the Huffman Tree and store Huffman Codes in a map.
	void Encode(Node* root, Code code, char bit, unordered_map<char, Code> &huffmanCode)
	{
		if (root == nullptr)
			return; // terminating condition for recursion

		code.Append(bit); // add the bit to process at this level

						  // check for a leaf node and add the value with its code if so
		if (root->left == nullptr && root->right == nullptr) {
			huffmanCode[root->ch] = code; // store the current code if a leaf node
			return; // no need to keep going since the children are null
		}

		// process child nodes with appropriate bit to be added for path
		// pass code by value, not reference, to keep original
		Encode(root->left, code, '0', huffmanCode);
		Encode(root->right, code, '1', huffmanCode);
	}

	//////////////////////////////////////////////////////////
	// Walk tree and create a table of value/bit code pairings
	unordered_map<char, Code> CreateTable(Node * root, unordered_set<char> options)
	{
		// traverse the Huffman Tree and store Huffman Codes
		// in a map. Also prints them
		unordered_map<char, Code> codeTable;
		Code code;

		// walk the tree and insert leaf node codes into the code table
		if (root == nullptr)
			return codeTable; // no tree to process (empty file or other error)


		if (root->left == nullptr && root->right == nullptr)
		{
			code.Append('1'); // we have exactly one kind of byte in the file
			codeTable[root->ch] = code;
		}
		else
		{
			// process child nodes with appropriate bit to be added for path
			// pass code by value, not reference, to keep original uncorrupted
			Encode(root->left, code, '0', codeTable);
			Encode(root->right, code, '1', codeTable);
		}

		if (options.find('t') != options.end())
		{
			printf("Huffman code table:\nchar len  code bits\n");
			for (auto pair : codeTable)
			{
				printf(isprint(pair.first & 0xFF) ? " %c  " : "%02X  ", pair.first & 0xFF);
				printf("%3i : ", pair.second.Length());
				for (int i = 0; i < pair.second.Length(); i++)
					printf("%c", pair.second.Bit(i));
				printf("\n");
			}
			printf("\n");
		}

		return codeTable;
	}

	////////////////////////////////////////////////////////////////////////////
	// Helper function to write the code tree in "bottom-up" postorder traversal
	void WritePostorder(struct Node* node)
	{
		if (node == NULL)
			return;

		// first recur on left subtree 
		WritePostorder(node->left);

		// then recur on right subtree 
		WritePostorder(node->right);

		// now write the node 
		char leaf = node->left == nullptr ? 1 : 0; // flag whether this is a leaf node (1) or internal node (0)
		output.put(leaf);
		output.put(node->ch);																	
		//divide 32 bit int values into 4 bytes in a non-endian dependent way
		output.put(static_cast<byte>((node->freq >> 24) & 0xFF));
		output.put(static_cast<byte>((node->freq >> 16) & 0xFF));
		output.put(static_cast<byte>((node->freq >> 8) & 0xFF));
		output.put(static_cast<byte>(node->freq & 0xFF));
	}

	int CountNodes(Node* root)
	{
		if (root == nullptr)
			return 0;
		else
			return CountNodes(root->left) + CountNodes(root->right) + 1;
	}
};

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
// Read and uncompress a file
class CompressedFileReader
{
	unordered_set<char> options; // options for printing data
	string inputFile;
	string outputFile;
	ifstream input;
	ofstream output;
	Node * codeTree = nullptr; // dynamically allocated, so destroy
	int buffer = 0; // storage for current byte from bitstream
	int bit = 0; // count of bits used in buffer

public:
	CompressedFileReader(string infile, string outfile, unordered_set<char> opts) : inputFile(infile), outputFile(outfile), options(opts)
	{
	}

	~CompressedFileReader()
	{
		// not strictly necessary, since close is called when stream is destroyed anyway
		if (input.is_open())
			input.close();
		if (output.is_open())
			output.close();

		delete codeTree;
	}

	int UncompressFile()
	{
		stack<Node *> treeStack; // stack to rebuild the code tree

		input.open(inputFile, ios::binary);
		output.open(outputFile, ios::binary);
		if (!input.is_open() || !output.is_open())
			return -1; // unexpected file error

		/////////////////////////////////////////////////
		// first read the code tree and create it again
		int size = 0; 
		int shift = 1; // shift operand to multiply bytes to get correct position
		for (int b = 3; b >= 0; --b)
		{
			byte c = input.get();
			size += c * (shift << (8 * b));
		}

		for (int i = 0; i < size; i++)
		{
			char leaf = input.get(); // read the node flag (leaf == 1, internal == 0)
			char ch = input.get(); // read the character for this node (internal is meaningless)

			//read 4 bytes and combine them into one 32 bit int value in a non-endian dependent way
			int freq = 0;
			int shift = 1; // shift operand to multiply bytes to get correct position
			for (int b = 3; b >= 0; --b)
			{
				byte c = input.get();
				freq += c * (shift << (8 * b));
			}
			Node* node = new Node(ch, freq);

			if (leaf == 0) // an internal node, so pop its leaves off the stack and save them
			{
				// since its LIFO, we pushed left then right so now we pop right then left
				node->right = treeStack.top();
				treeStack.pop();
				node->left = treeStack.top();
				treeStack.pop();
			}
			treeStack.push(node); // push the new node on the stack whether its a leaf or internal node
		}

		// we should have one node on the stack representing the root of our code tree
		if (options.find('t') != options.end())
		{
			printf("Code tree:\n");
			Node::PrintTree(treeStack.top(), 0, '*');
			printf("\n");
		}

		if (treeStack.size() != 1)
			return -2; // we failed to read the tree correctly (file was corrupted or not a compressed file)

		Node * nodeTree = treeStack.top(); // save the pointer

		//////////////////////////////////////////////////////////////////
		// Now we can read the bits and write the uncompressed characters
		// read 4 bytes and combine them into one 32 bit int value in a non-endian dependent way
		int length = 0;
		shift = 1; // shift operand to multiply bytes to get correct position
		for (int b = 3; b >= 0; --b)
		{
			byte c = input.get();
			length += c * (shift << (8 * b));
		}

		// uncompress <length> byte codes
		int count = 0; // count processed codes to compare to expected length
		for (int i = 0; i < length; i++)
		{
			int bit = GetBit(); // read the next bit in the compressed stream
			if (nodeTree->left == nullptr && nodeTree->right == nullptr)
				output.put(nodeTree->ch); // there is only one type of byte, so write it
			else
				Decode(bit == 0 ? nodeTree->left : nodeTree->right); // follow tree path to decode
			count++; // count each code processed
		}

		if (count != length)
			return -3; //printf("ERROR: %i characters processed when %i characters expected\n", count, length);

		input.close();
		output.close();
		return 0; // no apparent errors uncompressing file
	}

	//////////////////////////////////////////
	// Get the next bit from the input stream
	int GetBit()
	{
		if (bit == 0)
		{
			buffer = input.get(); // read the next byte if needed
		}
		int value = buffer >> 7 - bit & 1; // read the next bit
		bit = ++bit % 8; // count bits read up to 8 per byte
		return value;
	}

	/////////////////////////////////////////////////////////
	// Walk the code tree to find the decoded character byte
	void Decode(Node* root)
	{
		if (root == nullptr)
			return; // we got to the end of the tree without finding a valid character!

		// Did we reach a leaf node?
		if (root->left == nullptr && root->right == nullptr)
		{
			output.put(root->ch); // write the byte for this code
		}
		else
			Decode(GetBit() == 0 ? root->left : root->right); // continue walking tree in search of byte
	}
};

/////////////////////////////////////////////////////////
// Console implementation of the Huffman coding algorithm
// suitable for running as a tool or batch command
int main(int argc, char* argv[])
{
//	int argc = 10;
//	char* argv[] = { "Huffman", "E:\\static18.txt", "-c", "E:\\compressed.bin", "-u", "E:\\test.txt", "-s", "-t", "-b", "-f" };

	int length = 0; // length of file to be compressed
	string infileName;
	string compressFileName;
	string uncompressFileName;
	unordered_set<char> options;

	if (argc < 2)
	{
		cout << "Usage: " << argv[0] << " <inputfile> [options]" << endl << "where options can be:" << endl
			<< "-c <filename> : compress file" << endl << "-u <filename> : uncompress file" << endl 
			<<  "-f : print frequency table" << endl << "-t : print code tree" << endl 
			<< "-b : print code table" << endl << "-s : print compression stats" << endl;
		//getchar();
		return 1;
	}

	if (argc > 1)
		infileName = argv[1]; // assume the file is in the right place and correct until proven otherwise when trying to open it

	for (int i = 2; i < argc; ++i) 
	{
		string arg = argv[i];
		if (arg[0] == '-')
			switch (arg[1])
			{
			case 'c':
				options.insert('c'); // compress the file to the following destination
				compressFileName = argv[++i]; // we could do some serious checking, but not today
				printf("c option file %s\n", argv[i]);
				break;
			case 'u':
				options.insert('u'); // uncompress the file to follwing destination
				uncompressFileName = argv[++i];
				printf("u option file %s\n", argv[i]);
				break;
			case 'f':
				options.insert('f'); // print the frequency table
				break;
			case 't':
				options.insert('t'); // print the code tree
				break;
			case 'b':
				options.insert('b'); // print the code table
				break;
			case 's':
				options.insert('s'); // print the compression stats
				break;
			default:
				cout << endl << "Unknow option " << i << ": \"" << arg << "\"" << endl;
				break;
			}
		else
			cout << endl << "Bad argument " << i << ": \"" << arg << "\"" << endl;
	}

	if (options.find('c') != options.end())
	{
		CompressedFileWriter cfw(infileName, compressFileName, options);
		int error = cfw.CompressFile();
		if (error < 0)
			return error;

		////////////////////////////////////////////////
		// print simple encoding ratio for two files
		// which can be greater than 100% for small or uniform files
		if (options.find('s') != options.end())
		{
			struct stat filestatus;
			stat(infileName.c_str(), &filestatus);
			int inSize = filestatus.st_size;
			stat(compressFileName.c_str(), &filestatus);
			int outSize = filestatus.st_size;
			printf("\n%s was encoded to %4.2f%% of its original size\n\n", infileName.c_str(), (double)outSize / inSize * 100.0);
		}
	}

	if (options.find('u') != options.end())
	{
		if (options.find('c') != options.end())
		{
			infileName = compressFileName; // allows chaining for testing
		}
		CompressedFileReader cfr(infileName, uncompressFileName, options);
		int error = cfr.UncompressFile();
		if (error < 0)
			return error;

		////////////////////////////////////////////////
		// print simple encoding ratio for two files
		// which can be greater than 100% for small or uniform files
		if (options.find('s') != options.end())
		{
			struct stat filestatus;
			stat(infileName.c_str(), &filestatus);
			int inSize = filestatus.st_size;
			stat(uncompressFileName.c_str(), &filestatus);
			int outSize = filestatus.st_size;
			printf("\n%s was encoded to %4.2f%% of the original size\n\n", infileName.c_str(), (double)inSize / outSize * 100.0);
		}
	}

	//getchar();
	return 0;
}