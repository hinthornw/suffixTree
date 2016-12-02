#ifndef SUFF_TREE_INCLUDE
#define SUFF_TREE_INCLUDE


//typedef struct SuffixNode sNode;
typedef struct SuffixTree sTree;
int longestCommonSubstring(sTree * tree);
void printTree(sTree * tree);
void freeTree(sTree * tree);
sTree * buildTree(std::string text, int textLength, int alpha_size);
sTree * buildMultiTree(std::string s1, std::string s2, int alpha_size);

#endif