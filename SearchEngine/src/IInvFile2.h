#ifndef _IINVFILE_
#define _IINVFIEL_
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <vector>
#include <map>

using namespace std;

typedef struct _post2: post {
	vector<int> * locList;
} post2;

typedef struct _RetRec2: RetRec {
	int minFreq;
	string minStem;
	map<string, vector<int> *> * stemMap;
} RetRec2;

#define MAXTOKENS       256
#define MAXLINE         1024
#define MINLEN          3
#define STMINLEN        2

struct tnode {
 char *word;
 int count;
 struct tnode *left, *right;
};

class IInvFile2: public IInvFile {
public:
	IInvFile2();
	virtual ~IInvFile2();
	post * Add(char * s, int docid, int freq, int loc);
	void Save(char * f) override;
	void Load(char * f) override;
	void PrintTop(RetRec2 * r, int N);
	void PrintTopTRECFormat(RetRec2 * r, int top, int queryNumber,
			char * identifier);
	void PrintTopTRECFormatInTxt(RetRec2 * r, int top, int queryNumber, char * identifier, FILE * fp);
	void SearchTRECFormat(char * q, int queryNumber, char * identifier, FILE * fp) override;
	void Search(char * q) override;
	void CombineResult(RetRec2 * r, string stem, post * kk, float idf);
	void Normalize(RetRec2 * r, float qsize);
	void RecalSim(RetRec2 * r, int totalStem);

	RetRec2 * result;
	vector<int> * docList;
	float allSeqFoundRate;

	char *delim = ".,:;`\"+-_(){}[]<>*&^%$#@!?~/|\\= \t\r\n1234567890";
	struct tnode *root = {0};
	struct tnode *querry = {0};
	struct tnode *buildstoptree(char *, struct tnode *);
	struct tnode *addtree(struct tnode *, char *);
	struct tnode *findstopword(struct tnode *, char *);
	struct tnode *talloc(void);
	void freetree(struct tnode *);
	char **split(char *, char *);

protected:
	int BinSearch(int top, int bottom, int key, vector<int> position,
			int range);
	bool InRange(int key, int mid, vector<int> position, int range);
};

#endif // _IINVFILE_
