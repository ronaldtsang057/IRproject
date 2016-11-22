/*
 * IInvFile2.cpp
 *
 *  Created on: 2016/11/16
 *      Author: Jonathan
 */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "IInvFile.h"
#include "IInvFile2.h"

// Add a posting
post * IInvFile2::Add(char * s, int docid, int freq, int loc) {
	hnode * w = Find(s);	// Does the stem exist in the dictionary?
	post * k = NULL;

	if (w == NULL)
		w = MakeHnode(s); // If not exist, create a new hash node
	else
		k = FindPost(w, docid);	// if exists, is the first posting the wanted one?

	if (k == NULL) {			// no posting has the same docid
		k = w->posting; 		// push the data into the posting field
		w->posting = new post;		// create a new posting record
		w->posting->docid = docid;	// save the document id
		w->posting->freq = freq;	// save the term frequency
		w->df += 1;			// keep track of the document frequency
		w->posting->next = k;		// push the data into the posting field

		//my
		((post2 *) w->posting)->locList = new vector<int>;
		((post2 *) w->posting)->locList->push_back(loc);
	} else {
		k->freq += freq;// The posting exists, so add the freq to the freq field

		//my
		((post2 *) k)->locList->push_back(loc);
	}

	return k;
}

// Save the data into the file
void IInvFile2::Save(char * f) {
	FILE * fp = fopen(f, "wb");		// open the file for writing
	hnode * w;
	post * k;
	int size;
	int count;
	vector<int> locList;
	if ((hsize > 0) && (htable != NULL)) {	// Is there a hash table?
		for (int i = 0; i < hsize; i++) {	// For each hash table entry do
			w = htable[i];		// Go through each hash node
			while (w != NULL) {
				fprintf(fp, "%s %d:", w->stem, w->df);
				k = w->posting;	// Go through each post node
				while (k != NULL) {
					fprintf(fp, "%d %d", k->docid, k->freq);

					//my start
					//use . for term location
					fprintf(fp, ".");
					locList = *((post2 *) k)->locList;
					size = locList.size();

					for (int i = 0; i < size; i++) {
						count = locList[i];
						fprintf(fp, "%d ", count);
					}
					fprintf(fp, ",");
					//my end

					k = k->next;	// next post node
				}
				fprintf(fp, "\r\n");
				w = w->next;		// next hash node
			}
		}
	}
	fclose(fp);				// close the file
}

void IInvFile2::Load(char * f) {
	FILE * fp = fopen(f, "rb");
	hnode * w;
	post * k;
	char c;
	bool next;
	int state = 0;
	int cnt;
	char line[1000];
	char stem[1000];
	int df;
	int i;
	int docid;
	int freq;

	//my
	int loc;

	if (fp == NULL) {
		printf("Aborted: file not found for <%s>\r\n", f);
		return;
	}
	if ((hsize > 0) && (htable != NULL)) {
		i = 0;
		cnt = 0;
		do {
			next = true;
			if (fread(&c, 1, 1, fp) > 0) { // read a character
				switch (state) {
				case 0:
					if (c != ':')
						line[i++] = c;
					else {
						line[i] = '\0';
						sscanf(line, "%s %d", stem, &df);
						w = Find(stem);
						if (w == NULL) {
							w = MakeHnode(stem);
							w->df = df;
						}
						i = 0;
						state = 1;
						//printf("Read [%s,%d]\r\n",stem, df);
					}
					break;
				case 1:
					if (c == '\r')
						i = 0;
					else if (c == '\n') {
						cnt = 0;
						i = 0;
						state = 0;
					}
					//my
					else if (c == '.') {
						line[i] = '\0';
						cnt++;
						sscanf(line, "%d %d", &docid, &freq);
						k = w->posting; // push the data into the posting field
						w->posting = new post;
						w->posting->docid = docid;
						w->posting->freq = freq;
						w->posting->next = k;

						//my
						((post2 *) w->posting)->locList = new vector<int>;

						if (MaxDocid < docid)
							MaxDocid = docid;
						//printf("[%d] %d %d,\r\n", cnt, docid, freq);
						i = 0;

						//
						state = 2;
					} else
						line[i++] = c;
					break;

				case 2:
					if (c == ',') {
						i = 0;
						state = 1;
					} else if (c == ' ') {
						line[i] = '\0';
						sscanf(line, "%d", &loc);
						((post2 *) w->posting)->locList->push_back(loc);
						i = 0;
					} else
						line[i++] = c;
					break;
				}
			} else
				next = false;
		} while (next == true);
	} else
		printf("Aborted: no hash table\r\n");
	fclose(fp);
}

// Combine partial retrieval results
void IInvFile2::CombineResult(RetRec2 * r, string stem, post * kk, float idf) {
	post * k = kk;				// Get the pointer to the posting list
	int docid;

	//my
	int curFreq;
	vector<int> * locList;

	while (k != NULL) {
		docid = k->docid;
		if (docid > MaxDocid)
			printf("CombineResult error: Docid = %d > MaxDocID = %d\r\n",
					k->docid, MaxDocid);

		if (r[docid].sim == 0)
			docList->push_back(docid);

		r[docid].docid = docid;		// make sure we store the document id
		r[docid].sim += idf * (float) k->freq;// add the partial dot product score

		//my start
		curFreq = ((post2 *) k)->locList->size();
		if (r[docid].minFreq == 0 || r[docid].minFreq > curFreq) {
			r[docid].minFreq = curFreq;
			r[docid].minStem = stem;
		}

		locList = ((post2 *) k)->locList;

		if (r[docid].stemMap == NULL)
			r[docid].stemMap = new map<string, vector<int> *>;

		r[docid].stemMap->insert(pair<string, vector<int> *>(stem, locList));
		//my end

		k = k->next;				// next posting
	}

	//free(locList);

}

// Comparison function used by qsort(.): see below
int compare2(const void * aa, const void * bb) {
	RetRec2 * a = (RetRec2 *) aa;
	RetRec2 * b = (RetRec2 *) bb;

	if (a->sim > b->sim)
		return -1;
	else if (a->sim < b->sim)
		return 1;
	else
		return 0;
}

// Print top N results
void IInvFile2::PrintTop(RetRec2 * r, int top) {
	int i = MaxDocid + 1;
	//qsort(r, MaxDocid + 1, sizeof(RetRec), compare); // qsort is a C function: sort results
	qsort(r, MaxDocid + 1, sizeof(RetRec2), compare2); // qsort is a C function: sort results
	i = 0;
	printf("Search Results:\r\n");
	while (i < top) {
		if ((r[i].docid == 0) && (r[i].sim == 0.0))
			return; // no more results; so exit
		//printf("[%d]\t%d\t%s\t%f\r\n", i + 1, r[i].docid,
		//	Files[r[i].docid].TRECID, r[i].sim);
		printf("[%d]\t%d\t%e\r\n", i + 1, r[i].docid, r[i].sim);
		i++;
	}
}

// Print top N results
void IInvFile2::PrintTopTRECFormat(RetRec2 * r, int top, int queryNumber,
		char * identifier) {
	int i = MaxDocid + 1;
	//qsort(r, MaxDocid + 1, sizeof(RetRec), compare); // qsort is a C function: sort results
	qsort(r, MaxDocid + 1, sizeof(RetRec2), compare2); // qsort is a C function: sort results
	i = 0;
	while (i < top) {
		if ((r[i].docid == 0) && (r[i].sim == 0.0))
			return; // no more results; so exit
		printf("\t%d\t%s\t%d\t%f\t%s\r\n", queryNumber,
				Files[r[i].docid].TRECID, i + 1, r[i].sim, identifier);
		i++;
	}
}

void IInvFile2::PrintTopTRECFormatInTxt(RetRec2 * r, int top, int queryNumber,
		char * identifier, FILE * fp) {

	int i = MaxDocid + 1;
	qsort(r, MaxDocid + 1, sizeof(RetRec2), compare2); // qsort is a C function: sort results
	i = 0;
	int temp = 0;
	while (i < top) {
		if ((r[i].docid == 0) && (r[i].sim == 0.0))
			return; // no more results; so exit
		fprintf(fp, "%d\t%d\t%s\t%d\t%f\t%s\n", queryNumber, temp,
				Files[r[i].docid].TRECID, i, (r[i].sim * 100), identifier);
		i++;
	}
}

// Perform retrieval
void IInvFile2::SearchTRECFormat(char * q, int queryNumber, char * identifier,
		FILE * fp) {
	char * s = q;
	char * w;
	bool next = true;
	hnode * h;
	float qsize = 0.0; // query size
	// Initialize the result set
	if (result != NULL)
		free(result);
	//my start
	if (docList != NULL)
		free(docList);

	//result = (RetRec *) calloc(MaxDocid+1, sizeof(RetRec));
	result = (RetRec2 *) calloc(MaxDocid + 1, sizeof(RetRec2));
	docList = new vector<int>;

	bool recordFound = false;
	char *t;
	string stem;
	int totalStem = 0;
	int curSeq = 0;
	//my end

	do {
		w = s;					// Do searching
		s = GotoNextWord(s);			// Delimit the term
		if (s == NULL)
			next = false;		// If no more terms, exit
		else {
			if (*s != '\0')
				*(s - 1) = '\0';	// If not the last term, delimit the term
			Stemming.Stem(w);		// Stem the term w
			h = Find(w);			// Find it in the integrated inverted index
			if (h != NULL) {			// Add the scores to the result set
				totalStem++;
				//my
				stem = "";
				t = w;
				while (*t != '\0') {
					stem = stem + *t;
					t++;
				}

				//CombineResult(result, h->posting, GetIDF(h->df));
				CombineResult(result, stem, h->posting, GetIDF(h->df));
				recordFound = true;

				qsize += 1.0;

			} else if (strlen(w) > 0)
				printf("Query term does not exist <%s>\r\n", w);
		}
	} while (next == true);				// More query terms to handle?

	Normalize(result, qsize);
	//my
	if (recordFound)
		RecalSim(result, totalStem);

	PrintTopTRECFormatInTxt(result, 1000, queryNumber, identifier, fp);	// Print top 1000 retrieved results
}

// Perform retrieval
void IInvFile2::Search(char * q) {
	char * s = q;
	char * w;
	bool next = true;
	hnode * h;
	float qsize = 0.0;				// query size
	// Initialize the result set
	if (result != NULL)
		free(result);
	if (docList != NULL)
		free(docList);

	result = (RetRec2 *) calloc(MaxDocid + 1, sizeof(RetRec2));
	docList = new vector<int>;

	//my
	bool recordFound = false;
	char *t;
	string stem;
	int totalStem = 0;
	int curSeq = 0;
	do {
		w = s;					// Do searching
		s = GotoNextWord(s);			// Delimit the term
		if (s == NULL)
			next = false;		// If no more terms, exit
		else {
			totalStem++;
			if (*s != '\0')
				*(s - 1) = '\0';	// If not the last term, delimit the term
			Stemming.Stem(w);		// Stem the term w
			h = Find(w);			// Find it in the integrated inverted index
			if (h != NULL)			// Add the scores to the result set
			{
				stem = "";
				t = w;
				while (*t != '\0') {
					stem = stem + *t;
					t++;
				}
				//CombineResult(result, h->posting, GetIDF(h->df));
				CombineResult(result, stem, h->posting, GetIDF(h->df));
				recordFound = true;

				qsize += 1.0;
			} else if (strlen(w) > 0)
				printf("Query term does not exist <%s>\r\n", w);
		}
	} while (next == true);				// More query terms to handle?

	Normalize(result, qsize);

	//my
	if (recordFound)
		RecalSim(result, totalStem);

	PrintTop(result, 1000);				// Print top 10 retrieved results
}

void IInvFile2::Normalize(RetRec2 * r, float qsize) {
	float qlen = sqrt(qsize);
	int docid;

	for (int i = 0; i <= MaxDocid; i++) {
		docid = r[i].docid;
		if (qlen > 0.0 && Files[docid].len > 0.0) {
			r[i].sim = r[i].sim / Files[docid].len / qlen;
		} else {
			r[i].sim = 0;
		}
	}
}

void IInvFile2::RecalSim(RetRec2 * r, int totalStem) {

	string minStem;
	string previousStem;
	//RetRec2 * r2 = (RetRec2 *) r;
	int curCount;
	int findLoc;
	int docID;
	int seqFound;
	int mid;
	float rate;
	bool isLongQuery = totalStem > 10;
	vector<int> minPostList;
	vector<int> postList;
	vector<int> docList = *this->docList;
	map<string, vector<int>*> stemMap;
	map<string, vector<int>*>::iterator mapLT;
	for (int i = 0; i < docList.size(); i++) {
		docID = docList[i];
		if (r[docID].minFreq == 0)
			continue;
		minStem = r[docID].minStem;

		//no poiner
		//curCount = r[i].stemMap[curStem].size();
		stemMap = *r[docID].stemMap;
		minPostList = *stemMap[minStem];
		for (int i = 0; i < minPostList.size(); i++) {
			findLoc = minPostList[i];
			seqFound = 0;
			for (mapLT = stemMap.begin(); mapLT != stemMap.end(); mapLT++) {
				if ((*mapLT).first == minStem)
					continue;

				//findLoc = findLoc;
				postList = *(*mapLT).second;
				mid = this->BinSearch(postList.size() - 1, 0, minPostList[i],
						postList, 3);
				if (mid == -1) {
					break;
				}
				findLoc = postList[mid];
				//sequence found
				seqFound++;
			}

			if (seqFound != 0) {
				rate = (seqFound / (float) totalStem * this->allSeqFoundRate);
				r[docID].sim *= 1 + rate;
			}
		}

		//curCount = postList.size();
	}
}

int IInvFile2::BinSearch(int top, int bottom, int key, vector<int> position,
		int range) {
	if (top < bottom)
		return -1; // not found
	int mid = (top + bottom) / 2;
	if (InRange(key, mid, position, range) == true)
		return mid;
	if (key > position[mid])
		return BinSearch(top, mid + 1, key, position, range);
	else
		return BinSearch(mid - 1, bottom, key, position, range);
}

bool IInvFile2::InRange(int key, int mid, vector<int> position, int range) {
//changed +-
	if ((key - range <= position[mid]) && key + range >= position[mid])
		return true;
	else
		return false;
}

IInvFile2::IInvFile2() :
		IInvFile() {
	docList = NULL;
}
IInvFile2::~IInvFile2() {

}
