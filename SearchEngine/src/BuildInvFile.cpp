///////////////////////////////////////////////////////////////////////////////////////
//
// Author: Robert Luk
// Year: 2010
// Robert Luk (c) 2010 
//
// Convert data into inverted file format using the Integrated Inverted Index class:
// This software is made available only to students studying COMP433 (Information
// Retreieval). It should not be used or distributed without consent by the author.
//
////////////////////////////////////////////////////////////////////////////////////////
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "IInvFile.h"

// Integrated Inverted Index (see lecture notes on Implementation)
IInvFile InvFile;

int main() {
	char tmp[10000];
	char str[1000];
	int docid;
	int loc;
	int cnt=0;
//	FILE * fp = fopen("./post1.txt","rb");
//	if (fp == NULL) {
//		printf("Cannot open file \r\n");
//		return 1;
//	}
//
//	// Initialize the Hash Table
//	InvFile.MakeHashTable(13023973);
//
//	while(fgets(tmp,10000,fp) != NULL) {
//		// Get the stem, the document identifier and the location
//		sscanf(tmp,"%s %d %d", &(str[0]), &docid, &loc);
//
//		// Add posting into the Integrated Inverted index
//		// See lecture notes on Implementation
//		InvFile.Add(str, docid, 1);
//
//		// Keep us informed about the progress
//		cnt++;
//		if ((cnt % 100000) == 0) printf("Added [%d]\r\n",cnt);
//	}
//
//	printf("Saving inverted file ...\r\n");
//	InvFile.Save("InvFile.txt");
//	fclose(fp);
//
//	printf("Loading Inverted File\r\n");
//	InvFile.Load("InvFile.txt");
//	printf("Creating Document Records (size = %d)\r\n", InvFile.MaxDocid + 1);
//	InvFile.MakeDocRec();	// allocate document records
//	printf("Compute Document Lengths...\r\n");
//	InvFile.DocLen(InvFile.Files);
//	printf("Save Document Lengths\r\n");
//
//	InvFile.SaveDocRec("InvFile.doc");

	// Initialize the Hash Table
	InvFile.MakeHashTable(13023973);

	printf("Loading Inverted File\r\n");
	InvFile.Load("InvFile.txt");

	printf("Load Document Lengths\r\n");
	InvFile.LoadDocRec("InvFile.doc");
	InvFile.ReadTRECID("file.txt");
	// Start interactive retrieval
	InvFile.RetrievalTRECFormat();

	return 0;
}
