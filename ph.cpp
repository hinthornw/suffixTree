#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
#include "suffix_tree.h"
#include <algorithm>    // std::sort
#include <unordered_map>
#include <deque>
#define ALPH_SIZE 4 
#define MAX_ITERATIONS 2500
#define FILENAME "lambda_scramble.fa"
/* 
	FILENAME : ph.cpp
	AUTHOR: William Hinthorn
	Functions:
		int longestOverlap(string a, string b)
			returns an integer value of the longest overlap between strings a and b.
			Overlap must start from string b's beginning and touch a's end;
			Could you use REGEX instead? Yeah, but I wanted to try Ukkonen's alg.
		findWrongDNA(sSequencer * unProcessed)
			returns a vector of anomoly sequences which don't fit well with the existing dataset
			Uses a variant of the "stable marriage problem" to sort sequences into a deque 
			then check for cycles
		assemble(sSequencer * processed)
			returns the string of the assembled sequence

Note* longestOverlap uses linear time to create, but the overall process is quadratic in time since
distances are checkes pairwise. There are many optimizations that can be done if given more time.
Note #2: There are memory leaks in the current form.. In future iterations, this will be fixed.
Note #3: Yes I could have written all of this in python in a fraction of the space, time, and 
effort, but while I am in school, I want to stay close to the hardware. I wouldn't do this
for prototyping or production code since it is not flexible, is time-consuming, and it is 
generally awful style.
Improvements to be made:
*add heaps for each node
*avoid cycling through so many times
*Make more robust/ scalable

*/




typedef std::unordered_map<std::string, std::string> stringmap;
typedef std::unordered_map<std::string, std::string>::const_iterator mapIndx;
typedef std::vector<std::string> stringVec;


typedef struct entry{
	int found;
	struct entry * previous;
	struct entry * next;
	int prevDist;
	//heap of possibl nexts // Could use a heap of top matches to avoid reiterating over the array
	std::string text;			// HOWEVER, there are few repeats in this case, so it's not really a bottleneck for this dataset
} sEntry;
typedef std::deque<sEntry * > deque;
typedef struct sSequencer{
	int clean; // 0 if haven't removed questionable sequences, 1 otherwise
	stringVec raw; // Raw (uncleaned) input
	stringVec anomalies; //May be too large since greedy algorithm
	std::string final;
	sEntry * begin; // Once initialized (i.e. clean), will point to first sequence
	sEntry * end;
} sSequencer;


// Uses Generalized Suffix Tree with modified substring search to do this
int longestOverlap(std::string read1, std::string read2){
	int substr;
	int ALPHASIZE = 6; // 4 letters plus 2 dividers
	sTree * tree = buildMultiTree(read1, read2, ALPHASIZE);
	substr = longestCommonSubstring(tree); //substring modified to only do overlap (read1->read2)
	freeTree(tree);
	return substr;
}

//Marriage Problem
stringVec findWrongDNA(sSequencer * unProcessed){
	if(unProcessed->clean) return unProcessed->anomalies;
	deque toEmpty;
	deque remainFull;
	deque suspect;
	int valid = 50; // Should be 50, but hey...
	int overlap;
	//Initialize deque and vector
	for(stringVec::size_type j  = 0; j != unProcessed->raw.size(); j++){
		sEntry * temp = new sEntry();
		temp->found = 0;
		temp->previous = NULL;
		temp->next = NULL;
		temp->prevDist = -1;
		temp->text = unProcessed->raw.at(j);
		toEmpty.push_back(temp);
		remainFull.push_back(temp);
	}	
	int BACKUP = 0;//Prevent infinite loops
	 while (!toEmpty.empty() > 0 && BACKUP++ < MAX_ITERATIONS)  {
	 	sEntry * v = toEmpty.front();
	 	toEmpty.pop_front();
	 	if(v->found == -1 && v->previous == NULL){ // Give it another chance
	 		v->found = -2;// No one wants it and it doesn't want anyone...	 		 
	 	}
	 	else if (v->found == -2 && v->previous == NULL){ // been through twice with no luck
	 		suspect.push_back(v);
	 		continue;
	 	}
	 	else if (v->found == -1){
	 		unProcessed->end = v;
	 		continue;
	 	}
	 	else
	 		v->found = -1; // been touched
	 	int maxDistance = -1;
	 	sEntry * maxIndx = NULL;
	 	
	 	for(stringVec::size_type j  = 0; j != remainFull.size(); j++){
	 		sEntry * w = remainFull.at(j);
			if(w  == v) continue; // don't check self..
	 		overlap = longestOverlap(v->text, w->text);
	  		if (overlap >= valid){
		 		v->found = 1;
		 		// //This match can be 
		 		// 	1) NULL and
					// 	* better than current maxDistance
					// 		-> update indx & maxDistance
					// 	* worse than current maxDistance
					// 		-> do nothing
					// 2) Not NULL and
					// 	- This match is weaker than existing
					// 		* do nothing
					// 	- This match is stronger than existing and
					// 		* better than current maxDistance
					// 			-> update maxDistance & indx
					// 		*worse than current maxDistance
					// 			-> nothing
	 	  		if (w->previous == NULL){
	 	  			if(overlap > maxDistance){
	 	  				maxIndx = w;
	 	  				maxDistance = overlap;
	 	  			} 
	 	  			//else do nothing
		 		}
		 		else // j.previous is OCCUPIED
		 		{
		 			if(overlap > maxDistance && overlap > w->prevDist){
		 				maxIndx = w;
		 				maxDistance = overlap;
		 			}

		 		}
		 	} //IF
		 	/*else // Not technically valid by instructions, but hey
		 	{
		 		//So not "found"
		 		if (w->previous == NULL){
	 	  			if(overlap > maxDistance){
	 	  				maxIndx = w;
	 	  				maxDistance = overlap;
	 	  			} 
	 	  			//else do nothing
		 		}
		 		else // j.previous is OCCUPIED
		 		{
		 			if(overlap > maxDistance && overlap > w->prevDist){
		 				maxIndx = w;
		 				maxDistance = overlap;
		 			}

		 		}
		 	}*/
		 } // FOR
		 if(maxDistance != -1){
		 	if (maxIndx->previous != NULL){
		 		maxIndx->previous->found = 0;
		 		maxIndx->previous->next = NULL;
		 		toEmpty.push_back(maxIndx->previous);
		 		v->next = maxIndx;
		 		maxIndx->previous = v;
		 		maxIndx->prevDist = maxDistance;
		 	}
		 	else{
		 		v->next = maxIndx;
		 		maxIndx->previous = v;
		 		maxIndx->prevDist = maxDistance;
		 	}
		 }
		 else{
		 	toEmpty.push_back(v);
		 	//suspect.pushBack(v);
		 	// add to suspects?
		}

	}

	for(stringVec::size_type j  = 0; j != suspect.size(); j++){
		unProcessed->anomalies.push_back(suspect.at(j)->text);
		delete suspect.at(j);
	}
	sEntry * fast, *slow;
	int randomNumber = 15;// OK not so random but not that important for this.
	if(unProcessed->end){
		fast = unProcessed->end;
		slow = unProcessed->end;
	}
	else{
		fast = remainFull.at(randomNumber);
		slow = remainFull.at(randomNumber);
	}
	int l = 0;
	while(fast->previous != NULL){
		l++;
		fast = fast->previous;
		if(fast->previous == NULL){
			unProcessed->begin = fast;
			break;
		}
		fast = fast->previous;
		slow = slow->previous;
		if(fast == slow){
			fast = fast->previous;
			slow->previous = NULL;
			slow->prevDist = -1;
			fast->next = NULL;
			unProcessed->begin = slow;
			break;
		}
		if(fast->previous == NULL){
			unProcessed->begin = fast;
			break;
		}

	}
	unProcessed->clean = 1;
	return unProcessed->anomalies;
}
 	

std::string assemble(sSequencer * processed){
	if(!processed->clean)findWrongDNA(processed); // If called assemble without checking for bacteria, clean
	std::string squish;
	int subSize;
	sEntry * it = processed->begin;
	std::string line1, line2;
	line1 = it->text;
	squish = line1;
	while(it->next != NULL){
		line2 = it->next->text;
		subSize = longestOverlap(line1,line2);
		squish += line2.substr( subSize, line2.size());
		line1 = line2;
		it = it->next;
	}

	return squish;
}


	

	

void fastaParser(std::string filename, sSequencer * seq){
	std::ifstream file;
	stringVec * inputs = &(seq->raw);
	//stringVec inputs;
	file.open(FILENAME); //file.open(filename);
	std::string temp;
	while(std::getline(file, temp)){
		if(temp.at(0) != '>'){ // Ignore the sequence labels
			inputs->push_back(temp);
		}
	}
	file.close();
	// return inputs;
	
}

main(int argc, char * argv[]){
	//if(argc !=2) {fprintf(stderr, "Function Call Error.\n");}
	int N; // number of sequences extracted
	int i; //iterator
	clock_t t;

	std::vector<std::string>::iterator it1, it2;
	stringVec errors;
	std::string filename = "lambda_scramble.fa";
	std::cout << "Parsing " << filename << ".\n";

	sSequencer * seq = new sSequencer();

	t = clock();
	fastaParser(filename, seq);
	t = clock() - t;
	std::cout << "TIME: " << ((float) t / CLOCKS_PER_SEC) << "\n";


	t = clock();
	errors = findWrongDNA(seq);
	std::cout << "FindWrongDNA took: " <<((float)(clock() - t))/CLOCKS_PER_SEC << " seconds\n";
	std::cout << "Found " << errors.size() << " Possible Bacterium Sequences:\n";
	for(std::vector<std::string>::size_type i  = 0;i != errors.size(); i++){
		std::cout << errors.at(i) << "\n";
	}

	t = clock();
	std::string assembled = assemble(seq);
	std::cout << "Correct Sequence:\n" << assembled <<"\n";
	std::cout << "Sequence Length = " << assembled.length() << ".\n";
	std::cout << "Assemble took: " <<((float)(clock() - t))/CLOCKS_PER_SEC << " seconds\n";
	//sSequencer * seq = new sSequencer();
	sEntry * it = seq->begin->next;
	while(it->next != NULL){
		delete it->previous;
		it = it->next;
	}
	delete seq;
	return 0;

}


/*stringmap  map = processed->onto;
	std::string squish; //will be the returned item
	stringVec subStrings;
	stringVec looseEnds = processed->end;
	for(unsigned int i = 0; i != looseEnds.size(); i++)
	std::string line2 = processed->end;
	std::string line1;
	std::cout << "End: " << line2 << "\n";
	subStrings.push_back(line2);
	int subSize;
	std::cout << "Entering loop\n";
	int N = map.size();
	for(unsigned int j  = 0; j != N; j++){
		mapIndx got = map.find(line2); // find index of string
		if(got==map.end()){std::cout << "String not found?\n"; break;}
		else{
			//std::cout << "Found\n";
			//std::cout << got->second << "\n" << got->first << "\n";
			line1 =  got->second; //extract string
			//map.erase(got);
			//line2 = map.erase(line1); 
			//std::cout << "Overlapping Again\n";
			subSize = longestOverlap(line1, line2);
			//std::cout << "Overlap (" << subSize << ")\tBegin(" << line1.front() << ")\tEnd("<< line1.size() - subSize + 1 << ")\tSize (" << line1.size() << "\n";
			//std::cout << line1.substr(line1.front(), subSize);
			subStrings.push_back(line1.substr(line1.front(), line1.size() - subSize + 1));//+1 to include last member
			line2 = line1;
		}
	}
	std::cout << "Exited Loop\n";
	for(unsigned int j  = subStrings.size(); j-- > 0;){
		squish += subStrings.at(j);
	}

	processed->final = squish;
	return squish;*/
//}

//GREEDY ALGORITHM: Removes the best match for some in order which may lead to disjointed sequences
//Possible updates: Take top k best overlaps then do perfect matching to make sure all valid sequences have
//have a match (ran out of time)
// stringVec findWrongDNA(sSequencer * unProcessed){ // assume that it will not overlap either way well with anyone. Can stop if it works
// 	if(unProcessed->clean) return unProcessed->anomalies;
// 	int valid = 50;
// 	int found = 0;
// 	int forward, backward;
// 	stringVec::size_type MAXINDX;
// 	int maxDistance;

// 	stringVec * suspect = &(unProcessed->anomalies); // Will store suspected sequences
// 	stringVec * dirty = &(unProcessed->raw); //Point at raw items
// 	stringVec * unclaimed = new stringVec();
// 	for(stringVec::size_type i  = 0; /*i < 20*/ i != dirty->size(); i++){ // make a deep copy of all strings
// 		unclaimed->push_back(dirty->at(i));
// 	}
// 	stringmap * map = &(unProcessed->onto);
// 	//stringVec unclaimed;
// 	std::cout << "Finding Wrong DNA\n"; 

// 	//clock_t t;
// 	for(stringVec::size_type i  = 0; /*i < 20*/ i != dirty->size(); i++){
// 		found = 0;
// 		//std::cout << "Round " << i << "\n";
// 		//t = clock();
// 		MAXINDX = -1;
// 		maxDistance = -1;
// 		for(stringVec::size_type j  = 0; j != unclaimed->size(); j++){
// 			if (dirty->at(i) == unclaimed->at(j)) {continue;}
// 			forward = longestOverlap(dirty->at(i), unclaimed->at(j));
// 			//std::cout << "Forward Overlap = "<<forward<<"\nBackward Overlap = " <<backward<<"\n";
// 			if (forward >= valid /*|| backward  >= valid*/){
// 				found = 1; 
// 				if (forward > maxDistance){
// 					maxDistance = forward;
// 					MAXINDX = j;
// 				}
// 				//std::cout << "Item " << i << " with " << j << " are " << forward << "\n";
// 				//std::cout << "Took: " <<((float)(clock() - t))/CLOCKS_PER_SEC << " seconds\n";
// 				//break; // NEED TO FIND BEST!
// 			}
// 		}
// 		if(found == 1){
// 			//Remove the max val from the list
// 			(*map)[unclaimed->at(MAXINDX)] = dirty->at(i);
// 			//std::cout << "Item " << i << " with " << MAXINDX << " are " << maxDistance << "\n";
// 			unclaimed->erase(unclaimed->begin() + MAXINDX);
// 		}
// 		else 
// 		{ // could be bacteria, last segment, OR a victim of greed
// 			std::cout << "No forward...\n";
// 			for(stringVec::size_type j  = 0; j != unclaimed->size(); j++){
// 				backward = longestOverlap(unclaimed->at(j), dirty->at(i));
// 				if(backward  >= valid){
// 					found = 1; // found, so probably last element
// 					std::cout << "Found Backward?\n";
// 					if (backward > maxDistance){
// 						maxDistance = backward;
// 						MAXINDX = j;
// 					}				 
// 				}
// 			}
// 			if(found == 0){
// 				std::string bac = dirty->at(i);
// 				dirty->erase(dirty->begin() + i); // remove from the list
// 				suspect->push_back(bac);
// 				//std::cout << "Took: " <<((float)(clock() - t))/CLOCKS_PER_SEC << " seconds\n";
// 				// continue;
// 			}
// 			else{
// 				unProcessed->end.push_back(dirty->at(i));
// 				std::cout << "Found possible end:\n" << unProcessed->end <<"\n";
// 				//(*map)[dirty->at(i)] = unclaimed->at(MAXINDX);
// 			}
// 		}
// 	}
// 	std::cout << "Printing out Unclaimed Seq:\n";
// 	for(stringVec::size_type i  = 0; /*i < 20*/ i != unclaimed->size(); i++){
// 		std::cout << unclaimed->at(i) << "\n";
// 	}
// 	delete unclaimed;
// 	unProcessed->clean = 1;
// 	return *(suspect);
// }
