#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include "suffix_tree.h"
#define ALPH_SIZE 4 //size of alphabet being used
/*
Author: WIlliam Hinthorn
File: suffix_tree.cpp


References: Algorithm developed by Ukkonen. This code developed while referencing
http://www.geeksforgeeks.org/generalized-suffix-tree-1/
Though those algs were incomplete, filled with memory leaks, and had  a number of other problems.

*/
typedef struct SuffixNode {
	struct SuffixNode ** children;
	struct SuffixNode * link;
	int start;
	int * end; // pointer so it can be managed globally
	int indx;
} sNode;

// Three-tuple of active node, edge, and length
/*typeDef struct activePoint {
	sNode * aN = NULL;
	int aE = -1;
	int aL = 0;
} aPoint;*/


typedef struct SuffixTree {
	int alpha_size;
	char text[202]; // line size
	sNode * root;
	sNode * lastCreated;

	// Active Point
	sNode * aN; // Node which is parent of active point
	int aE; //Indicates the POSITION in text of the active point (used for corresponding letter)
	int aL; //Indicates length from current nodewhich the active point targets

	int impSuffixes; // outstanding implicit suffixes (due to shortcuts via repeat)
	int lEnd;
	int * rootEnd;
	int * splitEnd;
	int size; 
} sTree;

/* Functions*/
sNode * newNode(int begin, int * end, sTree * tree);
sTree * newTree(std::string text, int textLength, int alpha_size);
int edgeLength(sNode * n);
int walkDown(sNode * curr, sTree * tree);
int bioTable(char c);
char bioTable(int i);
void extendTree(int pos, sTree * tree);
void print(int i, int j, sTree * tree);
void _printTree(sNode * n, int labelHeight, sTree * tree);
void printTree(sTree * tree);
void setIndex(sNode * n, int labelHeight, sTree * tree);
void freeSuffixTree(sNode * n, sTree * tree);
void freeTree(sTree * tree);
sTree * buildTree(std::string text, int textLength, int alpha_size);
sTree * buildMultiTree(std::string s1, std::string s2, int alpha_size);
int longestCommonSubstring(sTree * tree);

sTree * newTree(std::string text, int textLength, int alpha_size){
	sTree * tree = new sTree();
	int * rE = new int;
	*rE = -1;
	tree->alpha_size = alpha_size;
	tree->lastCreated = NULL;
	tree->aN = NULL;
	tree->aE = -1;
	tree->aL = 0;
	tree->impSuffixes = 0;
	tree->lEnd = -1;
	tree->rootEnd = rE; 
	tree->splitEnd = NULL;
	tree->size = textLength + 1;
	//Root has start & end indices of -1
	tree->root = NULL; // root of root is NULL
	tree->root = newNode(-1, tree->rootEnd, tree);
	tree->aN = tree->root;
	if(textLength > 202){fprintf(stderr, "Text Length (%d) cannot be larger than 200.\n", textLength); return NULL;}

	for(int i = 0; i < textLength; i++){
		//std::cout << "Text at " << i << " is (" << text.at(i) << ")" << std::endl;
		tree->text[i] = text.at(i);
	}
	tree->text[textLength] = '$';
	return tree;
}


sNode * newNode(int begin, int * end, sTree * tree){
	sNode * node = new sNode();
	//std::vector<sNode *> children;
	sNode ** children = (new  sNode * [tree->alpha_size]);  	//TODO -> make it so this is poitners
	node->children = children;
	for(int i = 0; i < tree->alpha_size; i++){
		node->children[i] = NULL;
	}
	// Initialize
	node->link = tree->root;
	node->start = begin;
	node->end = end;
	node->indx = -1; // default to -1
	return node;
}

int edgeLength(sNode * n){
	return *(n->end) - (n->start) + 1; // +1 is needed since the indices are INCLUSIVE
}

//Walk down a partiular connection
int walkDown(sNode * curr, sTree * tree){
	int l = edgeLength(curr);
	if (tree->aL >= l){
		tree->aE += l;
		tree->aL -= l;
		tree->aN = curr;
		return 1;
	}
	return 0;
}

//Overloaded operators to convert from "ACGT" to 0123 for shortened alphabet
int bioTable(char c){
	switch(c){
		case 'A':
			return 0;
		case 'C':
			return 1;
		case 'G': 
			return 2;
		case 'T':
			return 3;
		case '#': 
			return 4;
		case '$':
			return 5;
		default:
			std::cout << "Input (" << c <<") is not in alphabet.\n";
			return -1; // will hopefully give a segfault
	}
}
char bioTable(int i){
	switch(i){
		case 0:
			return 'A';
		case 1:
			return 'C';
		case 2:
			return 'G';
		case 3:
			return 'T';
		case 4:
			return '#';
		case 5:
			return '$';
		default:
			std::cout << "Input (" << i <<") is not in alphabet.\n";
			return '\0'; // lol
	}
}

//Actual Stuff ***************
//WHAT I DID: Find all "tree->text[" replace with bioTree(tree->text[]
void extendTree(int pos, sTree * tree){
	tree->lEnd = pos;
	tree->lastCreated = NULL; // No newly created node for new phase
	tree->impSuffixes++;;
	//for(tree->impSuffixes++; tree->impSuffixes > 0; tree->impSuffixes -= 1){
	while(tree->impSuffixes > 0){
		// If active point is a node (i.e. not pointing to a character)
		// Set active Edge to the current character (pos)
		if(tree->aL == 0){
			tree->aE = pos;
		}
		
		// If edge not created
		if (tree->aN->children[bioTable(tree->text[tree->aE])] == NULL){
			tree->aN->children[bioTable(tree->text[tree->aE])] = newNode(pos, &(tree->lEnd), tree);

			//update suffix link
			if (tree->lastCreated != NULL){
				tree->lastCreated->link = tree->aN;
				tree->lastCreated = NULL;
			}

		}
		else{
			sNode * next = tree->aN->children[bioTable(tree->text[tree->aE])]; // next node is the one pointed to by active edge
			if(walkDown(next, tree)){
				continue;				//WILL: WHY?
			}

			if (tree->text[next->start + tree->aL] == tree->text[pos]){
				if(tree->lastCreated != NULL && tree->aN != tree->root){
					tree->lastCreated->link = tree->aN;
					tree->lastCreated = NULL;
				}

				tree->aL++;
				break; // Can break since the rest will all be repeat implicits

			}

			//Otherwise, must split in the middle of an edge. update split end to 
			tree->splitEnd = new int();
			*(tree->splitEnd) = next->start + tree->aL - 1; // End of new split

			sNode * split = newNode(next->start, tree->splitEnd, tree);
			tree->aN->children[bioTable(tree->text[tree->aE])] = split;

			//add new leaf node from split
			split->children[bioTable(tree->text[pos])] = newNode(pos, &(tree->lEnd), tree);
			next->start += tree->aL;
			split->children[bioTable(tree->text[next->start])] = next;

			//if needed, update suffx link before continuing
			if(tree->lastCreated != NULL){
				tree->lastCreated->link = split;
			}
			tree->lastCreated = split;
		}

		//remove following comments to mirror template //DELETE
		tree->impSuffixes--;
		if(tree->aN == tree->root && tree->aL > 0){
			tree->aL--;
			tree->aE = pos - tree->impSuffixes +1;
		}
		else if (tree->aN != tree->root){ // next phase will be one tier closer to root
			tree->aN = tree->aN->link;
		}

	}

}

//Print out decompressed edge label
void print(int i, int j, sTree * tree){
	int k;
	for (k = i; k <= j; k++){
		std::cout << tree->text[k];
	}
	//cout << endl;
}

void _printTree(sNode * n, sTree * tree){
	if (n == NULL) return;
	if (n->start != -1){
		print(n->start, *(n->end), tree);
	}

	int leaf = 1;
	int i;
	for(i = 0; i < tree->alpha_size; i++){
		if(n->children[i] != NULL){
			if(leaf == 1 && n->start != -1)
				std::cout << " [" << n->indx << "]" << std::endl;

			//n has at least 1 child, i.e. not a leaf
			leaf = 0;
			_printTree(n->children[i], tree);
		}
	}
	//if none found, node is leaf
	if(leaf == 1){
		std::cout << " [" << n->indx << "]" << std::endl;
	}

}

void printTree(sTree * tree){
	_printTree(tree->root, tree);
}

//Set indices of tree & print as DFS
void setIndex(sNode * n, int labelHeight, sTree * tree){
	if (n == NULL) return; // base case
	
	int leaf = 1;
	int i;
	for(i = 0; i < tree->alpha_size; i++){
		if (n->children[i] != NULL){
			//n has at least 1 child, i.e. not a leaf
			leaf = 0;
			setIndex(n->children[i], labelHeight + edgeLength(n->children[i]), tree);
		}


	}
	//if none found, node is leaf
	if(leaf == 1){
		for(i = n->start; i <= *(n->end); i++){
			if(tree->text[i] == '#'){
				n->end = new int();
				*(n->end) = i;
				// std::cout << "Creating (" << *(n->end) << ") at (" << n->end << "). Indx = " << n->indx <<".\n";
			}
		}
		n->indx = tree->size - labelHeight;
	}
}



//WILL TODO: delete the actual tree struct..
void freeSuffixTree(sNode * n, sTree * tree, int size1){
	if (n == NULL)
		return;
	int i;
	for (i = 0; i < tree->alpha_size; i++){
		if(n->children[i] != NULL){
			freeSuffixTree(n->children[i], tree, size1);
		}
	}
	//std::cout << "Checking (" << *(n->end) << ") at (" << n->end << "). Indx = " << n->indx <<".\n";
	if((n->end) != &(tree->lEnd) && (n->end) != tree->rootEnd /*(n->end) == size1*/ /*n->indx > -1 && n->indx < size1*/){ //Either a leaf or a split end
		//delete(n->children);
		//std::cout << "Deleting (" << *(n->end) << ") at (" << n->end << "). Indx = " << n->indx <<".\n";
		delete(n->end);
	}
	delete[] (n->children);
	delete(n);
}

void freeTree(sTree * tree){
	int i;
	int size1 = 0;
	for(i = 0; i < tree->size; i++){
		if(tree->text[i] == '#'){break;}
		size1++;
	}
	freeSuffixTree(tree->root, tree, size1);
	delete(tree->rootEnd);
	//delete(tree->splitEnd);
	delete(tree);
}

sTree * buildTree(std::string text, int textLength, int alpha_size){
	int i;
	int labelHeight = 0;
	sTree * tree = newTree(text, textLength, alpha_size);
	/*std::cout << "Tree Text is : ";
	for(i = 0; i <= tree->size; i++)
		std::cout << tree->text[i] << ".";*/
	//std::cout << std::endl;
	for(i = 0; i < tree->size; i++){
		extendTree(i, tree);
	}
	setIndex(tree->root, labelHeight, tree);
	return tree;
}
sTree * buildMultiTree(std::string s1, std::string s2, int alpha_size){
	std::string combo = s1 + "#" + s2;
	return buildTree(combo, combo.length(), alpha_size);
}
int traverse(sNode *n, int labelHeight, int* maxHeight, int * startIndex, sTree * tree, int size1){
	if(n == NULL) return -5; // base
	int i = 0;
	int ret = -1;
	if(n->indx < 0){ // Not a leaf
		for(i = 0; i < tree->alpha_size; i++){
			if(n->children[i] != NULL){
				ret = traverse(n->children[i], labelHeight + edgeLength(n->children[i]), maxHeight, startIndex, tree, size1);
				if(n->indx == -1)
					n->indx = ret;
				else if((n->indx == -2 && ret == -3) || (n->indx == -3 && ret == -2) || n->indx == -4){
					n->indx = -4; //-4 indicates that it node is internal and is shared by both strings

					if(*(n->end) == size1-1 && *maxHeight < labelHeight){
						// std::cout << "Height = " << labelHeight << std::endl;			
						// std::cout <<"LabelHeight=" << labelHeight << "\tstartIndex=" <<*(n->end) - labelHeight + 1 << ".\tEnd = " << *(n->end)<<".\tWordLength = " << size1 << ".\t";
						// int k;
						// for(k = 0; k < labelHeight; k++)
						// 	std::cout << tree->text[k + *(n->end) - labelHeight + 1];
						// std::cout << std:: endl;
						//std::cout << "Child: " << n->children[tree->text[*(n->end) - labelHeight + 1]]->indx << ".\n"; 
						for(int j = 0; j < tree->alpha_size; j++){
							if(n->children[j] != NULL) {
								//std::cout << "Child (" << (char) j << "): at " <<n->children[j]->indx << ".\t";
								//print(n->children[j]->start -1, *(n->children[j]->end), tree);
								//std::cout << "\n";
								if(n->children[j]->indx == size1 + 1){
									*maxHeight = labelHeight;
									*startIndex = *(n->end) - labelHeight + 1;
								}
							}
						}

						
					}
				}


				
			}
		}

	}
	
	else if(n->indx > -1 && n->indx < size1) return -2; // Returns if a leaf that in string 1 onl y
	else if(n->indx >= size1) return -3;	//Returns if a leaf that is in string 2 only
	return n->indx; // Returns only if not a leaf & is suffix shared by two strings
	

}

int longestCommonSubstring(sTree * tree){
	int maxHeight = 0;
	int startIndex = 0;
	int size1 = 0;
	int i;
	for(i = 0; i < tree->size; i++){
		if(tree->text[i] == '#'){break;}
		size1++;
	}
	//std::cout << "SIze = " << size1 << std::endl;
	traverse(tree->root, 0, &maxHeight, &startIndex, tree, size1);
	int k;
	// for(k = 0; k < maxHeight; k++)
	// 	std::cout << tree->text[k + startIndex];
	// std::cout << std:: endl;
	return maxHeight;
}

// int main(int argc, char * argv[]){
// 	/*char t = 'A';
// 	int ** num = new int *[256];
// 	for(int i = 0; i < 256; i++){
// 		num[i] = new int();
// 		*(num[i]) = i;
// 	}
// 	std::cout << "Testing" << std::endl;
// 	std::cout << "Val of 'A' = " << num[t] << " = " << *num[t] << std::endl;
// 	delete *num;*/
// 	std::string input = "CAAAAAAAAAAAAAACGTGT";
// 	std::string input2 = "AGTGTAAAAAAAAAAAAAAAAAA";
// 	int length;
// 	sTree * tree;
// 	int DEFAULTALPHA = 6;
// 	// std::cout << "Input a string to build a suffix tree." << std::endl;	
// 	// std::cin >> input;
// 	// std::cin >> input2;
// 	//length = input.length();
// 	//tree = buildTree(input, length, DEFAULTALPHA);
// 	tree = buildMultiTree(input, input2, DEFAULTALPHA);
// 	std::cout << "Finding Longest Common Substring" << std::endl;
// 	std::cout << longestCommonSubstring(tree) << std::endl;
// 	//printTree(tree);
// 	freeTree(tree);
// 	return 0;


// }