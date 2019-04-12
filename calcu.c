/*
 * calcu.c
 *
 *  Created on: Nov 23, 2018
 *      Author: yoram
 *
 *      Calcudoku solver.
 *      inout file format:

<size>
12
<cboard>
1  1  2  2  3  3  4  4  5  5  6  7
8  9  9  11 12 13 14 14 5  15 16 7
8  8  9  17 17 13 18 19 19 15 16 21
22 22 23 23 24 24 25 25 19 19 26 21
22 27 27 28 29 24 31 19 19 32 32 33
34 27 27 28 29 35 31 31 36 36 37 33
38 38 39 39 41 35 42 42 43 44 37 45
46 38 47 47 41 48 49 42 43 44 51 45
46 52 47 47 53 53 49 54 55 55 51 56
57 52 58 58 59 53 61 54 54 62 62 56
57 63 64 65 59 59 61 66 67 62 68 68
69 63 63 65 65 71 72 72 67 73 68 74
</cboard>
<cage>
1,	19,	2,	+
2,	2,	2,	:
3,	2,	2,	:
4,	77,	2,	*
5,	192,3,	*
6,	12,	1,	-
7,	17,	2,	+
8,	3,	3,	-
9,	350,3,	*
11,	11,	1,	-
12,	3,	1,	-
13,	4,	2,	-
14,	18,	2,	*
15,	24,	2,	*
16,	4,	2,	-
17,	22,	2,	*
18,	12,	1,	-
19,	38,	6,	+
21,	15,	2,	+
22,	112,3,	*
23,	16,	2,	+
24,	26,	3,	+
25,	15,	2,	*
26,	11,	1,	-
27,	31,	4,	+
28,	12,	2,	+
29,	5,	2,	+
31,	720,3,	*
32,	5,	2,	*
33,	1,	2,	-
34,	9,	1,	-
35,	14,	2,	*
36,	21,	2,	*
37,	11,	2,	+
38,	13,	3,	+
39,	21,	2,	+
41,	13,	2,	+
42,	13,	3,	+
43,	14,	2,	+
44,	110,2,	*
45,	9,	2,	*
46,	132,2,	*
47,	25,	4,	+
48,	12,	1,	-
49,	6,	2,	-
51,	1,	2,	-
52,	12,	2,	+
53,	0,	3,	-
54,	32,	3,	*
55,	14,	2,	+
56,	16,	2,	+
57,	7,	2,	+
58,	11,	2,	+
59,	432,3,	*
61,	2,	2,	:
62,	648,3,	*
63,	18,	3,	+
64,	3,	1,	-
65,	21,	3,	+
66,	11,	1,	-
67,	84,	2,	*
68,	19,	3,	+
69,	5,	1,	-
71,	3,	1,	-
72,	7,	2,	+
73,	2,	1,	-
74,	11,	1,	-
</cage>

the result:
10  9  6  3  2  1 11  7  8  4 12  5 
 1  7 10 11  3  5  2  9  6  8  4 12 
 6 10  5  2 11  9 12  4  1  3  8  7 
 7  2  4 12 10  6  3  5  9  1 11  8 
 8  6  2  7  4 10  9 12 11  5  1  3 
 9 12 11  5  1  2  8 10  3  7  6  4 
 2  3 12  9  8  7  4  6 10 11  5  1 
11  8  1  6  5 12  7  3  4 10  2  9 
12 11  8 10  7  4  1  2  5  9  3  6 
 3  1  7  4  6 11  5  8  2 12  9 10 
 4  5  3  1  9  8 10 11 12  6  7  2 
 5  4  9  8 12  3  6  1  7  2 10 11 
Iterations: 2342097 (recd:36)


* 
 <cboard> describes the calcudoku board
 <cboard> describes the calcudoku cages (numbers indicate cage ID)
 <cage>   descibed the cages (ID, Val, number of digits/cells, and Operand (+-:*%^|))

*
*       usage: calcu -f inFile [-z] -l[1/2/3]
*
*       -s Size - set the size of the Calcudoku board. Max size is 15 (15x15 board)
*       -z numbers include 0 (ZERO)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "calcu.h"
#include "../timer/timer.h"

char *CageB = "<cage>";
char *CageE = "</cage>";
char *cBoardB = "<cboard>";
char *cBoardE = "</cboard>";
char *BSize = "<size>";


static _cage 	**cageInd = NULL; // allow direct access to cage information
long long int   iter = 0, stack = 0, deepest = 0, duplicates = 0; // for statistic printouts
int	        loging = 0;
int			bSize; 	// board size = bSize x bSize
int			zero = 0;
int 		cellsC; // number of cells in board

/********************************************************************************************
*	Utility functions
*********************************************************************************************/

// return number of set bits in word
int countSetBits(unsigned short int num) {
	int count = 0;

	while (num) {
		if (num & 1)
			count++;
		num >>= 1;
	}
	return count;
}

// get next available value from given bitmask
// return -1 if no bit was set
int getVal(unsigned short int bits) {
	int i, s;

	for (i = 0; i < bSize+1-zero; i++) {
		if ((s = isBitSet(i, bits))) {
			return i;
		}
	}
	
	return -1;
}

int readInput(FILE *fp, int *cBoard) {
	char line [100];
	char *tok;
	int s;

	while ((fgets(line, sizeof(line) - 1, fp))) {
		if (tok = strstr(line, BSize)) {
			if(fgets(line, sizeof(line) -1, fp)) {
				bSize = atoi(line);
			}
		}
		else if (tok = strstr(line, CageB)) {
			if (!(s = readCage(fp))) {
				break;
			}
		}
		else if (tok = strstr(line, cBoardB)) {
			if (!(s = readBoard(fp, cBoard, cBoardE))) {
				break;
			}
		}
	}

	return s;
}

// Read a raw of numbers and update the relevant raw pon board
// It also validate that each cage ID is valid
int readLineB(int *board, char *line) {
	char *token;
	int i = 0;

	token = strtok(line, " ");

	while ((token != NULL) && (i < bSize)) {
		board[i] = atoi(token);
		i++;
		token = strtok(NULL, " ");
	}

	if (i == bSize) {
		return 1;
	}

	return 0;
}

int readBoard(FILE *fp, int *board, char *token) {
	int i;
	char *tok;
	char line[100];

	i = 0;
	while ((fgets(line, sizeof(line) - 1, fp)) && (i < bSize)) {
		if ((tok = strstr(line, token))) {
			break;
		}
		else {
			if (readLineB(&board[i * NUMBERS], line)) {
				i++;
			}
			else {
				break;
			}
		}
	}

	if (i < bSize) {
		printf("failed to read board %s\n", token);
		return 0;
	}

	return 1;
}

char getOp(char *str) {
	char *p;

	p = str;

	while (*p) {
		switch (*p) {
			case '+':
			case '-':
			case '*':
			case ':':
			case '^':
			case '%':
			case '|':
				return *p;
			default:
				p++;
		}
	}
}


// This function gets array of digits (diritsA) of size (nDigits), insters them into
// _comb internal array and update the appearence count per digit
// It returns a bitmask of all recieved digits 
unsigned short inDigits(_digit *digits, const char *digitsA, const int nDigits) {
	int i; 
	unsigned short mask = 0;

	for (i = 0; i < nDigits; i++ ) {
		digits[i].oDigit = digitsA[i];
		digits[i].rDigit = digitsA[i]; 
		mask = setBit(digitsA[i], mask);
	}

	return mask;
}


unsigned short clearDigit(const int val, _digit *digits, const int nDigits) {
	int i, set = 0;
	unsigned short mask = 0;

	for (i = 0; i < nDigits; i++) {
		// we check if this digit was already cleared in this loop
		// in case we have same digit multiple times, this loop will clear only one appearance
		if ((digits[i].rDigit == val) && !set) {
			// -1 indicates clreared digit - we support ZERO digit as well!!
			digits[i].rDigit = -1;
			set = 1;
		}

		// constract updated bit mask for remaining digits
		if (digits[i].rDigit > -1) {
			mask = setBit(digits[i].rDigit, mask);
		}
	}

	return mask;
}


// restore remaining digits to their oroginal state. return bitmask
unsigned short resetDIgits(_digit *digits, int nDigits) {
	int i;
	unsigned short mask = 0;

	for (i = 0; i < nDigits; i++) {
		digits[i].rDigit = digits[i].oDigit;
		mask = setBit(digits[i].oDigit, mask);
	}

	return mask;
}

void simpleSort(char *digitsS, const char *digits, const int nDigits) {
	int i, j, k;
	char t;

	// copy source digits array to target
	for (i = 0; i < nDigits; i++) {
		digitsS[i] = digits[i];
	}

	// and sort it
	for (i = 0; i < nDigits - 1; i++) {
		k = i;
		for (j = i + 1; j < nDigits; j++) {
			if (digitsS[k] > digitsS[j])
				k = j;
		}

		if (k > i) {
			t = digitsS[k];
			digitsS[k] = digitsS[i];
			digitsS[i] = t;
		}
	}
}

// compare tow digit strings
// digits1 is simple char string
// digits2 is array of struct _digit
// return 0 if equal. -1 if digits1 < digits2, and 1 if digits1 > digits2
int compDigits(const char *digits1, const _digit *digits2, const int nDigits) {
	int i = 0, isEqual = 0;

	while ((i < nDigits) && !isEqual) {
		isEqual = digits1[i] - digits2[i].oDigit;
		i++;
	}

	return isEqual;
}

/********************************************************************************************
*	Calcudoku related functions
*********************************************************************************************/
int nextNumbers(char *digits, const int num) {
	int i, j = 0;
	int found = 0;

	// get current counter
	i = num - 1;
	while (!found && (i > -1)) {
		digits[i]++;
		if (digits[i] > (bSize - zero)) {
			i--;
		}
		else {
			for (i++; i < num; i++) {
				digits[i] = 1 - zero;
			}

			found = 1;
		}
	}

	return found;
}



int calcSum(const char *digits, const int num) {
	int i,s;

	s = digits[0];

	for (i = 1; i < num; i++) {
		s += digits[i];
	}

	return s;
}

int calcSub(const char *digits, const int num) {
	int i, s;

	s = digits[0];

	for (i = 1; i < num; i++) {
		s -= digits[i];
	}

	return s;
}

int calcMod(const char *digits, const int num) {
	int i, s;

	s = digits[0];

	for (i = 1; i < num; i++) {
		s %= digits[i];
	}

	return s;
}

int calcMul(const char *digits, const int num) {
	int i, m = 1;

	for (i = 0; i < num; i++) {
		m *= digits[i];
	}

	return m;
}

int calcOr(const char *digits, const int num) {
	int i, m;

	m = digits[0];

	for (i = 1; i < num; i++) {
		m |= digits[i];
	}

	return m;
}

int calcPower(const char *digits, const int num) {
	int i, s;

	s = digits[0];

	for (i = 1; i < num; i++) {
		s = pow(digits[i], s);
	}

	return s;
}

int calcDiv(const char *digits, const int num) {
	int i, d;

	d = digits[0];

	for (i = 1; i < num; i++) {
		if (digits[i] == 0) {
			return 0;
		}
		else {
			// we must make sure that the div result will be an integer number
			if (!(d % digits[i])) {
				d /= digits[i];
			}
			else {
				return INVALIDNUMBER;
			}
		}
	}

	return d;
}


// This function will go through all calcudoku cells which are part of a given cage
// and update their bitmask according to cage bitmask.
void updateCage(_brdR *b, int cId) {
	int i, r, c, ind;
	_cage *cPtr;
	_addR *cellPtr;
	_comb *combPtr;
	unsigned short *bitMask;
	int *score;
	unsigned short cBits = 0;

	cPtr = cageInd[cId];
	bitMask = &(b->bitMask[0][0]);
	score = &(b->score[0][0]);

	if (cPtr != NULL) {
		// first constract the cage remaining digits
		combPtr = cPtr->ptr;
		while (combPtr) {
			cBits |= combPtr->hdr.remainingDigits;
			combPtr = combPtr->hdr.next;
		}

		// now go through all sudo board cells belonging to this cage and update their bitmask and scores
		cellPtr = cPtr->cellPtr;
		if (cellPtr != NULL) {
			for (i = 0; i < cPtr->nCells; i++) {
				r = cellPtr[i].raw;
				c = cellPtr[i].col;

				ind = r * NUMBERS + c;

				if (score[ind] > 0) {
					bitMask[ind] &= cBits;
					score[ind] = countSetBits(bitMask[ind]);
				}
			}
		}
	}
}

void printComb(_digit *digits, int nDigits) {
	int i;

	for (i = 0; i < nDigits; i++) {
		printf("%d ", digits[i].oDigit);
	}
	printf("\n");
}

void dumpCages() {
	_comb *combPtr, *rlPtr;
	_cage *cagePtr;
	int i, j;

	for (i = 0; i < INDEXS; i++) {
		cagePtr = cageInd[i];

		if (cagePtr) {
			printf("cageID: %d, Value: %d, Cells: %d \n", i, cagePtr->value, cagePtr->nCells);
			combPtr = cagePtr->ptr;
			j = 1;
			while (combPtr) {
				rlPtr = combPtr;
				printf("[CID: %d], [%d]: ", i, j++);
				printComb(combPtr->digits, cagePtr->nCells);
				combPtr = combPtr->hdr.next;
			}
		}
	}
}

// release all allocated RAM
void releaseMem() {
	_comb *combPtr, *rlPtr;
	_cage *cagePtr;
	int i;

	for (i = 0; i < INDEXS; i++) {
		cagePtr = cageInd[i];

		if (cagePtr) {
			combPtr = cagePtr->ptr;
			while (combPtr) {
				rlPtr = combPtr;
				combPtr = combPtr->hdr.next;
				free(rlPtr);
			}
			free(cagePtr);
		}
	}
	free(cageInd);
}

// call once to initialize the cage index array
_cage **cageIndInit() {

	_cage **cPtr;

	cPtr = malloc(sizeof(cageInd) * INDEXS);
	if (cPtr != NULL) {
		memset(cPtr, 0, sizeof(cageInd) * INDEXS);
	}

	return cPtr;
}

// Add new sun combination
_comb *addComb(const short int cID, const char *digitsA) {
	_comb *nPtr, *cPtr, *tPtr;
	int nDigits;
	unsigned short digits;
	char dSorted[NUMBERS];
	int comp = 1;

	nDigits = cageInd[cID]->nCells;
	
	// first sort input digits
	simpleSort(dSorted, digitsA, nDigits);

	// now search in current combo list if this set of digits are alredy in
	// comb list is sorted!!
	// if YES - just return that comb location.
	// if NO - insert the new comb in its righ location.

	cPtr = cageInd[cID]->ptr;
	tPtr = NULL;

	if (cPtr != NULL) {
		do {
			// compare new digits array with current combo array
			comp = compDigits(dSorted, cPtr->digits, nDigits);

			if (comp > 0) {
				tPtr = cPtr;
				cPtr = cPtr->hdr.next;
			}
			else {
				break;
			}
		} while (cPtr != NULL);
	}

	// At this point we have the following scenarios
	// ptr == NULL i.e. this is the first combo for this cage
	// comp == 0 i.e. we found a match. ptr points to this match - just return ptr
	// comp < 0 i.e. we need to add new combo before current combo
	// comp > 0 i.e. we need to add new combo after current combo
	if (comp) {
		// Note that we allocate a variable length structure
		nPtr = malloc(sizeof(_combHdr) + sizeof(_digit) * nDigits);

		if (nPtr != NULL) {
			// insert digits to variable part of _comb record
			digits = inDigits(nPtr->digits, dSorted, nDigits);
			nPtr->hdr.allDigits = digits;
			nPtr->hdr.remainingDigits = digits;

			if (comp > 0) {
				// either this is the first combo (tPtr = NULL, for comp>0 cPtr will be NULL
				// in all cases), or it needs to be added to the end of the list
				nPtr->hdr.next = NULL;
				if (tPtr == NULL) {
					cageInd[cID]->ptr = nPtr;
				}
				else {
					tPtr->hdr.next = nPtr;
				}
			}
			else {
				// it needs to be added before cPtre.
				// if tPtr == NULL then this new combo needs to be the first in list
				nPtr->hdr.next = cPtr;
				if (tPtr == NULL) {
					cageInd[cID]->ptr = nPtr;
				}
				else {
					tPtr->hdr.next = nPtr;
				}
			}
		}
	}
	else {
		nPtr = cPtr;
		duplicates++;
	}

	return nPtr;
}

// find all digit combinations that generat val based on the given op
int brk(const int cID, const int val, const int num, const char op) {
	int i, j = 0, keepSearching = 1;
	char digits[NUMBERS];
	unsigned short dMask;
	int calcV;
	int (*calcF)(const char *digits, const int num);


	// initialize calcF based on input op
	switch (op) {
		case '+':
			calcF = calcSum;
			break;
		case '-':
			calcF = calcSub;
			break;
		case '*':
			calcF = calcMul;
			break;
		case ':':
			calcF = calcDiv;
			break;
		case '^':
			calcF = calcPower;
			break;
		case '%':
			calcF = calcMod;
			break;
		case '|':
			calcF = calcOr;
			break;
		default:
			return -1;
	}

	// inint original numbers
	for (i = 0; i < num; i++) {
		digits[i] = 1 - zero;
	}

	// start scanning all possible combinations
	// for each num numbers check if sun is what we are looking for
	// if yes, add this combination to resultArr

	do {
		if (calcF(digits, num) == val) {
			if (addComb(cID, digits) == NULL) {
				j = -1;
				break;
			}
			j++;
		}
	} while (nextNumbers(digits, num));

	return j;
}

// Being called every time new cage is found
_cage *cageCreate(const short int id, const int val, const short int nCells, const char op) {
	_cage *cPtr;
	_addR *cellPtr;
	int i;

	if ((id >= INDEXS) || (id < 1)) {
		printf("Invalid cage ID [%d]\n", id);
		return NULL;
	}

	// if cage has already been created - ignore this call
	if (cageInd[id] != NULL) {
		return cageInd[id];
	}

	// allocate ram for cell back pointers
	if ((cellPtr = calloc(nCells, sizeof(_addR))) == NULL) {
		printf("failed to allocate memory inside cageCreate\n");
		return NULL;
	}
	else {
		// inint pointers with -1 we will need that later
		for (i = 0; i < nCells; i++) {
			cellPtr[i].col = -1;
			cellPtr[i].raw = -1;
		}
	}

	cPtr = calloc(1, sizeof(_cage));
	if (cPtr != NULL) {
		cPtr->value = val;
		cPtr->nCells = nCells;
		cPtr->cellPtr = cellPtr;
		cageInd[id] = cPtr;

		if (brk(id, val, nCells, op) < 0) {
			return NULL;
		}
	}

	return cPtr;
}



int readCage(FILE *fp) {
	char line[100];
	char *tok, op;
	int cageNum, cageVal, nCells;

	while ((fgets(line, sizeof(line) - 1, fp))) {
		tok = strstr(line, CageE);
		if (!tok) {
			tok = strtok(line, ",");
			if (tok) {
				cageNum = atoi(tok);
				tok = strtok(NULL, ",");
				cageVal = atoi(tok);
				tok = strtok(NULL, ",");
				nCells = atoi(tok);
				tok = strtok(NULL,"\n");
				op = getOp(tok);
				if (!cageCreate(cageNum, cageVal, nCells, op)) {
					printf("failed to add new cage ID [%d]\n", cageNum);
					return 0;
				}
			}
		}
		else {
			break;
		}
	}

	return 1;
}

// check for cages with 2 or more cells that all cells are neighbors and no one is isolated

int validateCageId(int *board, int r, int c) {
	int cId;
	int rm, rp, cm, cp;
	int s = 0;
	_cage *cPtr;

	cId = board[r * NUMBERS + c];

	// no need to check for cage ID 0
	if (cId == 0) {
		return 1;
	}


	cPtr = cageInd[cId];
	if (cPtr == NULL) {
		printf("Cage [CID: %d] was not initialized\n", cId);
		return 0;
	}

	// for single cell cage - return valid
	if (cPtr->nCells < 2) {
		return 1;
	}

	rm = (r > 0 ? r - 1 : r);
	rp = (r < bSize-1 ? r + 1 : r);
	cm = (c > 0 ? c - 1 : c);
	cp = (c < bSize-1 ? c + 1 : c);

	if ((rm != r) && (board[rm * NUMBERS + c] == cId)) s = 1;
	if ((rp != r) && (board[rp * NUMBERS + c] == cId)) s = 1;
	if ((cm != c) && (board[r * NUMBERS + cm] == cId)) s = 1;
	if ((cp != c) && (board[r * NUMBERS + cp] == cId)) s = 1;

	return s;
}

// cage backpointers are two bit masks holding the numbers of raws as columns of cells
// belonging to a particular cage.
void updateBackPointers(int cId, int r, int c) {
	_cage *cPtr;
	int i;

	// check that input cId is in frange, otherwise ignore
	if ((cId < 1) || (cId >= INDEXS)) {
		return;
	}

	cPtr = cageInd[cId];

	if (cPtr != NULL) {
		i = 0;

		// advance to first empty address slot
		while (i < cPtr->nCells) {
			if (cPtr->cellPtr[i].raw >= 0) {
				i++;
			}
			else {
				break;
			}
		}

		if (i < cPtr->nCells) {
			cPtr->cellPtr[i].raw = r;
			cPtr->cellPtr[i].col = c;
		}
		else {
			printf("Failed to update back pointer. All slots are full. [cId: %d] [raw: %d] [col: %d]\n", cId, r, c);
		}
	}
	else {
		printf("Error updating back pointers [cId: %d] [raw: %d] [col: %d]\n", cId, r, c);
	}
}

int initCalcudoku(int *cBoard) {
	char line[100];
	int i, j, ind, failure = 0;
	char *tok;

	// Validate input board.
	// validateCageId will also update each cage with its backpointers to board cells
	for (i = 0; i < bSize && !failure; i++) {
		for (j = 0; j < bSize && !failure; j++) {
			if (validateCageId(cBoard, i, j)) {
				ind = i * NUMBERS + j;

				//update cage backpointers
				updateBackPointers(cBoard[ind], i, j);
			}
			else {
				failure = 1;
				printf("failed to update cage %d, in cell [%d, %d]\n", cBoard[ind], i, j);
			}
		}
	}

	return (!failure);
}

// Set Combo Bits.
// Update all combo bits according to given cell value.
void setCageComboBits (int cId, int val) {
	_cage *cPtr;
	_comb *combPtr;

	cPtr = cageInd[cId];

	if (cPtr) {
		combPtr = cPtr->ptr;

		while (combPtr) {
			if (isBitSet(val, combPtr->hdr.remainingDigits)) {
				combPtr->hdr.remainingDigits = clearDigit (val, combPtr->digits, cPtr->nCells);
			}
			else {
				combPtr->hdr.remainingDigits = 0;
			}
			combPtr = combPtr->hdr.next;			
		}
	}
}

// set digit combination change flag and reset all combination digits
void resetCombChange() {
	int i;
	_cage *cPtr;
	_comb *combPtr;

	for (i = 1; i < INDEXS; i++) {
		if ((cPtr = cageInd[i]) != NULL) {
			combPtr = cPtr->ptr;

			while (combPtr) {
				combPtr->hdr.remainingDigits = resetDIgits(combPtr->digits, cPtr->nCells);			
				combPtr = combPtr->hdr.next;
			}
		}
	}
}


// reset all cages after returning from failed solvBoard()
void restoreCageStates(const _brdR *board) {
	int i, j;
	int *cBoard;

	cBoard = board->cBoard;

	if (!cBoard) {
		return;
	}

	// reset all combinations digits in all cages
	resetCombChange();

	for (i = 0; i < bSize; i++) {
		for (j = 0; j < bSize; j++) {
			if (board->score[i][j] < 0)
				setCageComboBits(cBoard[i * NUMBERS + j], board->board[i][j]);
		}
	}
}

void printBoard (int *b) {
	int i,j;

	for (i = 0; i < bSize; i++) {
		for (j = 0; j < bSize; j++) {
			printf("%2d ", b[i*NUMBERS+j]);
		}

		printf("\n");
	}
}

void printStat(_brdR *b, int r, int c, int v) {
	switch (loging) {
		case 2:
			printBoard(&(b->board[0][0]));
		case 1:
			printf("Iterations: %I64d, (stk:%I64d), (recd:%I64d), (r:%d), (c:%d), (v:%d) Non empty cells: %d\n", iter, stack, deepest, r, c, v, b->cells);
		default:
			break;
	}
}



// Once we set a cell with nre value, all cells on same raw, col must be updated
// so that this value is not a valid value for them
void updateLines (_brdR *b, const int r, const int c, const int v) {
	int i;

	for (i = 0; i < bSize; i++) {
		b->bitMask[r][i] = clearBit(v, b->bitMask[r][i]);
		b->bitMask[i][c] = clearBit(v, b->bitMask[i][c]);

		if (b->score[r][i] > 0) {
			b->score[r][i] = countSetBits(b->bitMask[r][i]);
		}
		if (b->score[i][c] > 0) {
			b->score[i][c] = countSetBits(b->bitMask[i][c]);
		}
	}
}

//set new value in a given cell
int setNewValue (_brdR *b, int r, int c, int v) {
	int cId;

	if (!b->cBoard) {
		return 0;
	}
	else {
		cId = b->cBoard[r * NUMBERS + c];
	}

	iter++;

	// check that selected location is free, otherwise return error
	if ((b->board[r][c] == 0) && (isBitSet(v, b->bitMask[r][c]))) {
		b->board[r][c] = v;
		b->score[r][c] = -1;

		updateLines(b, r, c, v);

		// first update cage information based on given set value
		setCageComboBits(cId, v);

		// Then update back all relevant calcudoku cells
		updateCage(b, cId);

		b->cells++;

		if (loging) {
			printStat(b, r, c, v);
		}

		return 1;
	}
	else {
		return 0;
	}
}

// find the first non set cell location
// maxOpt indicates max value options allowed for the selected cell
int findFirstNonSet (_brdR *b, int *r, int *c, int maxOpt) {
	int i, j, score;

	// if board is complete no next cell for update
	if (b->cells == cellsC) {
		return 0;
	}

	for (i = 0; i < bSize; i++) {
		for (j = 0; j < bSize; j++) {
			score = b->score[i][j];
			if(score > -1) {
				if ((score > 0) && (score <= maxOpt)) {
					*r = i;
					*c = j;
					return 1;
				}
				else if (score == 0) {
					// dead-end
					return 0;
				}
			}
		}
	}

	return 0;
}


// this is the recursive function that is used to solve the board
// the function will update the given board only if it managed to sove it.
// otherwise it is using a local copy to try and fing solution using recursion calls
int solveBoard (_brdR *board, const int raw, const int col, const int value) {
	int r , c;
	_brdR *b;
	int changed;
	int v;
	unsigned short bitmask;

	stack++;
	deepest = (deepest > stack? deepest: stack);

	// create local copy of board and sent it to solvBoard()
	if ((b = malloc(sizeof(_brdR))) == NULL) {
		return 0;
	}
	memcpy(b, board, sizeof(_brdR));

	//  if we call solveBoard with Value > 0 we need first to change the input board to have this value
	// as final value for this recursion
	if (value > -1) {
		if (!setNewValue(b, raw, col, value)) {
			printf("fatal Error!\n");
			return 0;
		}
	}

	
	// try to solve board assuming we now might have new cells with single possible value option
	changed = 1;
	while (changed && (b->cells < cellsC)) {
		changed = findFirstNonSet(b, &r, &c, 1);
		if	(changed) {
			changed = setNewValue(b, r, c, getVal(b->bitMask[r][c]));
		}
	}

	if (b->cells == cellsC) {
		// found solution - exit recursion
		memcpy(board, b, sizeof(_brdR));
		free(b);
		stack--;
		return 1;
	}


	// if we get here, it means we could not find any other cells with single value option.
	// now scan the board for all cells with more value options
	if (findFirstNonSet(b, &r, &c, bSize)) {
		bitmask = b->bitMask[r][c];
		while ((v = getVal(bitmask)) > -1) {
			// recursive call !!
			if (!solveBoard(b, r, c, v)) {
				bitmask = clearBit(v, bitmask);
				restoreCageStates(b);
			}
			else {
				// recursive call solved the board. we need to updated the input board and return
				memcpy(board, b, sizeof(_brdR));
				free(b);
				stack--;
				return 1;
			}
		}
	}

	// if we get here it means we did not find solution for this recursive branch
	free(b);
	stack--;

	return 0;
}

void init_brdR(_brdR *brd) {
	int i, j;
	unsigned short mask = 0;

	// constract mask. By default we set bits 1 to bSize. If zero is set then we set bits 0 to bSize-1
	for (i = 0; i < bSize; i++) {
		mask = setBit(i+1-zero, mask);
	}

	brd->cells = 0;

	for (i = 0; i < bSize; i++) {
		for (j = 0; j < bSize; j++) {
			brd->board[i][j] = 0;
			brd->bitMask[i][j] = mask;
			brd->score[i][j] = bSize;
		}
	}
}

void timerFunc (const char *prompt) {
	printf("%s > [IT: %I64d] [ST: %I64d] [D: %I64d] [DUPS: %I64d]\n",prompt, iter, stack, deepest, duplicates);
}

int main (int argc, char **argv) {
	char  opt, *fname = NULL, str[31];
	FILE *fp = NULL;
	int stat = 1, i;
	int cBoard[NUMBERS][NUMBERS];
	_brdR *pBlk;
	void *timerS;
	int timerT = 0;

	// get commad line input
	if (argc > 1) {
		while ((opt = getopt(argc, argv, "f:l:zt:")) > 0) { 
			switch (opt) {
				case 'f':
					fname = optarg;
					fp = fopen(fname, "r");
					break;
				case 'l':
					loging = atoi(optarg);
					break;
				case 'z':
					zero = 1;
					break;
				case 't':
					timerT = atoi(optarg);
					break;
				default:
					printf("wrong input\n");
					return stat;
			}
		}
	}

	if (!fp) {
		printf("error opening file %s, error: %s\n", fname, strerror(errno));
		return stat;
	}

	// set timer if it was requested
	if (timerT) {
		int s;

		if ((s = initTimerService())) {
			s = setTimer(timerT, 1, timerFunc, &timerS);
		}
		if (!s) {
			printf("failed to set timer. Will be ignored...\n");
		}
	}

	// allocate board record
	if ((pBlk = calloc(1, sizeof(_brdR))) == NULL) {
		return stat;
	}

	// inint cage index array
	if ((cageInd = cageIndInit()) == NULL) {
		printf("failed to inintialize cage index array\n");
		return stat;
	}


	if (!readInput(fp, &cBoard[0][0])) {
		printf("failed to read input file\n");
		return stat;
	}

	// now that we got bSize we can set cellsC variable
	cellsC = bSize*bSize;

	// init pBlk before starting processing the input
	// we can call init_brdR only after readin input where board size is read.
	init_brdR(pBlk);
	pBlk->cBoard = &cBoard[0][0];

	if (loging == 3) {
		dumpCages();
	}

	if (!initCalcudoku(&cBoard[0][0])) {
		printf("failed to initialize Clacudoku board\n");
		return stat;
	}

	// after initilizing resut board based on initput board
	// we need to update bitmasks on all cages before we go to solveboard
	for (i = 1; i < INDEXS; i++) {
		updateCage(pBlk, i);
	}

	iter=0;
	if ((stat = !solveBoard(pBlk, 0, 0, -1))) {
		printf("failed to find solution\n");
	}

	printBoard(&(pBlk->board[0][0]));
	printf("Iterations: %I64d (recd:%I64d)\n", iter, deepest);
	free(pBlk);

	if (fp != stdin) {
		fclose(fp);
	}

	releaseMem();

	stopTimerService();

	return stat;
}
