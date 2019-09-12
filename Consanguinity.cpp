//============================================================================
// Name        : Consanguinity.cpp
// Author      : Willow Black
// Version     : 1.0
// Description : Consanguinity calculator
//============================================================================

#include <iostream>
#include <new>
#include <vector>
#include <set>
#include <cmath>

using namespace std;

struct Creature{
    string name;
    Creature* Mother;
    Creature* Father;
    vector<Creature*> Children;
    float Consanguinity;

    Creature(){
    	Consanguinity = 0;
        Mother = nullptr;
        Father = nullptr;
        Children.reserve(16);
    }
};

void findAncestors(Creature* src,set<Creature*>& Ancestors){
	if(src->Mother != nullptr && src->Father != nullptr){
		Ancestors.insert(src->Mother);
		Ancestors.insert(src->Father);
		findAncestors(src->Mother, Ancestors);
		findAncestors(src->Father, Ancestors);
	}
}

set<Creature*> findcommonAncestors(Creature* src, Creature* dst){
	set<Creature*> srcAncestors;
	findAncestors(src,srcAncestors);
	set<Creature*> dstAncestors;
	findAncestors(dst,dstAncestors);
	set<Creature*> commonAncestors;

	for(set<Creature*>::iterator it = srcAncestors.begin(); it != srcAncestors.end(); ++it){
		if(dstAncestors.find(*it) != dstAncestors.end())
			commonAncestors.insert(*it);
	}

	return commonAncestors;
}

void directRelation(Creature* src, Creature* dst, vector<int>& distances, int distance){
	//Search down the tree until dst is found.
	if (src == dst)
		distances.push_back(distance);
	else
		for(vector<Creature*>::iterator it = src->Children.begin(); it != src->Children.end(); ++it)
			directRelation(*it,dst,distances,distance+1);
}

void collateralRelation(bool up, Creature* src, Creature* dst, set<Creature*> common, vector<int>& distances, int distance, set<Creature*> visited){
	visited.insert(src);
	//This function is funky. Basically, it goes up the tree until it finds a common ancestor
	//Then it allows searching down the tree from that node, too. Both cases, we do not revisit previously visited nodes.
	if(src == dst)
		distances.push_back(distance);
	else if (up){
		if(common.find(src) != common.end())
			for(vector<Creature*>::iterator it = src->Children.begin(); it != src->Children.end(); ++it){
				if(visited.find(*it) == visited.end())
					collateralRelation(false,*it,dst,common,distances,distance+1,visited);
			}
		else {
			if(src->Mother != nullptr)
				collateralRelation(up, src->Mother,dst,common,distances,distance+1,visited);
			if(src->Father != nullptr)
				collateralRelation(up, src->Father,dst,common,distances,distance+1,visited);
		}
	}else{
		for(vector<Creature*>::iterator it = src->Children.begin(); it != src->Children.end(); ++it)
			if(true)
				collateralRelation(false,*it,dst,common,distances,distance+1,visited);
	}

}

vector<int> determineRelatedness(Creature* src, Creature* dst){
	vector<int> distances;
	set<Creature*> visited;
	set<Creature*> srcAncestors;
	set<Creature*> dstAncestors;
	findAncestors(src,srcAncestors);

	//Edge case handling if we want funky ways to handle twins.
	if(src == dst)
		distances.push_back(0);

	//A set is a red-black tree, find() returns the end of the set if the element searched for isn't in the set
	//This is to make sure that src and dst are in the correct order for doing direct relation search
	if(srcAncestors.find(dst) != srcAncestors.end()){
		Creature* temp = src;
		src = dst;
		dst = temp;
	}

	findAncestors(src,srcAncestors);
	findAncestors(dst,dstAncestors);
	set<Creature*> commonAncestors = findcommonAncestors(src,dst);
	visited.insert(src);

	//If src is an ancestor of dst, perform a direct relation search
	if(dstAncestors.find(src) != dstAncestors.end())
		directRelation(src,dst,distances,0);
	//If there are common ancestors, perform a collateral relation search
	if(commonAncestors.size() > 0)
		collateralRelation(true,src,dst,commonAncestors,distances,0, visited);
	//collateral and direct relations are calculated separately, as per advice
	//from the website I read describing how to perform consanguinity calculations

	/*
	for(vector<int>::iterator it = distances.begin(); it != distances.end(); ++it)
	    	cout << *it << " ";
	cout << endl;
	*/
	return distances;
}



Creature* Mate(Creature* Mom, Creature* Dad){
    Creature* Child = new Creature();
    //Child->name = "(" + Mom->name + "&" + Dad->name + ")";
    Child->Mother = Mom;
    Child->Father = Dad;
    Mom->Children.push_back(Child);
    Dad->Children.push_back(Child);
    vector<int> degrees = determineRelatedness(Mom,Dad);
    //Calculating the coefficient of relationship, r, and summing the average of the parents with it.
    //Coefficient of relationship, without the parental averaging, approaches 1 with an arbitrary
    //amount of inbreeding. Perhaps just going to leave out the parental averaging, but it makes
    //for an interesting way of more immediately reflecting how inbred a genetic line is.
    float r = (Mom->Consanguinity+Dad->Consanguinity)/2.0;
    for(vector<int>::iterator it = degrees.begin(); it != degrees.end(); ++it){
    	r += pow(0.5,*it);
    }
    Child->Consanguinity=r;
    return Child;
}

int main() {
	Creature* A = new Creature();
	Creature* B = new Creature();
	A->name = 'A';
	B->name = 'B';

	vector<Creature*> creatures;
	creatures.push_back(A);
	creatures.push_back(B);

	for(int i = 0; i < 24; i ++){
		creatures.push_back(Mate(creatures.at(creatures.size()-1),creatures.at(creatures.size()-2)));
		creatures.back()->name = 'C' + i;
	}


	cout << "Incestuousness: " << endl;
	for(vector<Creature*>::iterator it = creatures.begin(); it != creatures.end(); ++it){
		Creature* temp = *it;
		if(temp->Mother != nullptr){
			cout << temp->Mother->name << "x" << temp->Father->name << " | ";
		}
		cout << temp->name + ": " << temp->Consanguinity << endl;
	}

    return 0;
}
