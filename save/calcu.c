/*
 * calcu.c
 *
 *  Created on: Nov 23, 2018
 *      Author: yoram
 *
 *      Calcudoku solver.
 *      inout file format:


<cboard>
 1  1  2  2  3  3  4  4  5
 1  6  2  8  3 10 10  .  5
12  6  7  8  9  9 10 11  5
12  6  7  7  . 15 15 11  .
13 14 14 16 16 15 17 11 18
13  . 19 16 21 22 17 23 18
20 20 19 21 21 22 24 23 23
20 20  . 25 25 25 24 23 23
26 26 26 26 25 27 27 28 28
</cboard>

<cage>
1,	15,	3, :
2, 	19,	3, +
3,	16,	3, -
4,	3,	2, *
5,	19,	3, +
6,	15,	3, :
7,	16,	3, +
8,	4,	2
9,	10,	2
10,	18,	3
11,	12,	3
12,	7,	2
13,	16,	2
14,	10,	2
15,	17,	3
16,	20,	3
17,	7,	2
18,	8,	2
19,	6,	2
20,	23,	4
21,	9,	3
22,	7,	2
23,	26,	5
24,	12,	2
25,	25,	4
26,	21,	4
27,	12,	2
28,	10,	2
</cage>
* 
 <cboard> describes the calcudoku board
 <cboard> describes the calcudoku cages (numbers indicate cage ID)
 <cage>   descibed the cages (ID, Val, number of digits/cells, and Operand (+-:*))

*
*       usage: calcu -f inFile -s Size [-z] -l[1/2]
*
*       -s Size - set the size of the Calcudoku board. Max size is 15 (15x15 board)
*       -z numbers include 0 (ZERO)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "calcu.h"

char *CageB = "<cage>";
char *CageE = "</cage>";
char *cBoardB = "<cboard>";
char *cBoardE = "</cboard>";
char *iBoardB = "<iboard>";
char *iBoardE = "</iboard";


static _cage 	**cageInd = NULL; // allow direct access to cage information
long long int   iter = 0, stack = 0, deepest = 0; // for statistic printouts
int	        loging = 0;
int			bSize; 	// board size = bSize x bSize
int			zero = 0;

/********************************************************************************************
*	Utility functions
*********************************************************************************************/

// return number of set bits in word
char countSetBits(unsigned short int num)
{
	char count = 0;

	while (num)
	{
		if (num & 1)
			count++;
		num >>= 1;
	}
	return count;
}

// get next available value from given bitmask
// return -1 if no bit was set
char getVal(unsigned short int bits)
{
	char i, s;

	for (i = 0; i < bSize+1-zero; i++)
	{
		if ((s = isBitSet(i, bits)))
		{
			return i;
		}
	}
	
	return -1;
}

int readInput(FILE *fp, char *iBoard, char *cBoard)
{
	char line [100];
	char *tok;
	int s;

	while ((fgets(line, sizeof(line) - 1, fp)))
	{
		if (tok = strstr(line, CageB))
		{
			if (!(s = readCage(fp)))
				break;
		}
		else if (tok = strstr(line, cBoardB))
		{
			if (!(s = readBoard(fp, cBoard, cBoardE)))
				break;
		}
		else if ((tok = strstr(line, iBoardB)))
		{
			if (!(s = readBoard(fp, iBoard, iBoardE)))
				break;
		}
	}

	return s;
}

// Read a raw of numbers and update the relevant raw pon board
// It also validate that each cage ID is valid
int readLineB(char *board, char *line)
{
	char *token;
	int i = 0;

	token = strtok(line, " ");

	while ((token != NULL) && (i < bSize))
	{
		board[i] = atoi(token);
		i++;
		token = strtok(NULL, " ");
	}

	if (i == bSize)
		return 1;

	return 0;
}

int readBoard(FILE *fp, char *board, char *token)
{
	int i;
	char *tok;
	char line[100];

	i = 0;
	while ((fgets(line, sizeof(line) - 1, fp)) && (i < bSize))
	{
		if ((tok = strstr(line, token)))
		{
			break;
		}
		else
		{
			if (readLineB(&board[i * NUMBERS], line))
				i++;
			else
			{
				break;
			}
		}
	}

	if (i < bSize)
	{
		printf("failed to read board %s\n", token);
		return 0;
	}

	return 1;
}

char getOp(char *str)
{
	char *p;

	p = str;

	while (*p)
	{
		switch (*p)
		{
		case '+':
		case '-':
		case '*':
		case ':':
			return *p;
		default:
			p++;
		}
	}
}

/********************************************************************************************
*	Calcudoku related functions
*********************************************************************************************/
int nextNumbers(int *digits, const int num)
{
	int i, j = 0, k;
	int found = 0;

	// get current counter
	i = num - 1;
	while (!found && (i > -1))
	{
		digits[i]++;
		if (digits[i] > (bSize - zero - j))
		{
			i--;
			j++;
		}
		else
		{
			for (i++; i < num; i++)
				digits[i] = digits[i - 1] + 1;

			found = 1;
		}
	}

	return found;
}

int calcSum(const int *digits, const int num)
{
	int i, s = 0;

	for (i = 0; i < num; i++)
		s += digits[i];

	return s;
}

int calcSub(const int *digits, const int num)
{
	int i, s;

	s = digits[num-1];

	for (i = 0; i < num-1; i++)
		s -= digits[i];

	return s;
}

int calcMul(const int *digits, const int num)
{
	int i, m = 1;

	for (i = 0; i < num; i++)
		m *= digits[i];

	return m;
}

int calcDiv(const int *digits, const int num)
{
	int i, d;

	d = digits[num-1];

	for (i = 0; i < num-1; i++)
	{
		if (digits[i] == 0)
			return 0;
		else
		{
			// we must make sure that the div result will be an integer number
			if (!(d % digits[i]))
				d /= digits[i];
			else
				return INVALIDNUMBER;
		}

	}

	return d;
}


// This function will go through all calcudoku cells which are part of a given cage
// and update their bitmask according to cage bitmask.
void updateCage(_brdR *b, int cId)
{
	int i, r, c;
	_cage *cPtr;
	_addR *cellPtr;
	_comb *combPtr;
	unsigned short *bitMask;
	char *score;
	unsigned short cBits = 0;

	cPtr = cageInd[cId];
	bitMask = &(b->bitMask[0][0]);
	score = &(b->score[0][0]);

	if (cPtr != NULL)
	{
		// first constract the cage remaining digits
		combPtr = cPtr->ptr;
		while (combPtr)
		{
			cBits |= combPtr->remainingDigits;
			combPtr = combPtr->next;
		}

		// now go through all sudo board cells belonging to this cage and uodate their bitmask and scores
		cellPtr = cPtr->cellPtr;
		if (cellPtr != NULL)
		{
			for (i = 0; i < cPtr->nCells; i++)
			{
				r = cellPtr[i].raw;
				c = cellPtr[i].col;

				if (score[r * NUMBERS + c] > 0)
				{
					bitMask[r * NUMBERS + c] &= cBits;
					score[r*NUMBERS+c] = countSetBits(bitMask[r*NUMBERS+c]);
				}
			}
		}
	}
}

// release all allocated RAM
void releaseMem()
{
	_comb *combPtr, *rlPtr;
	_cage *cagePtr;
	int i, j;

	for (i = 0; i < INDEXS; i++)
	{
		cagePtr = cageInd[i];

		if (cagePtr)
		{
			combPtr = cagePtr->ptr;
			while (combPtr)
			{
				rlPtr = combPtr;
				combPtr = combPtr->next;
				free(rlPtr);
			}
			free(cagePtr);
		}
	}
	free(cageInd);
}

// call once to initialize the cage index array
_cage **cageIndInit()
{

	_cage **cPtr;

	// we allocate 82 to cover 1-81 indexes
	cPtr = malloc(sizeof(cageInd) * INDEXS);
	if (cPtr != NULL)
		memset(cPtr, 0, sizeof(cageInd) * INDEXS);

	return cPtr;
}

// Add new sun combination
_comb *addComb(short int cID, unsigned short digits)
{
	_comb *nPtr, *ptr;

	nPtr = malloc(sizeof(_comb));

	if (nPtr != NULL)
	{
		nPtr->allDigits = digits;
		nPtr->remainingDigits = digits;
		nPtr->next = NULL;

		if (cageInd[cID]->ptr)
		{
			ptr = cageInd[cID]->ptr;
			while (ptr->next != NULL)
				ptr = ptr->next;

			ptr->next = nPtr;
		}
		else
			cageInd[cID]->ptr = nPtr;
	}

	return nPtr;
}

// convert array of digits to a Mask
unsigned short genBMask(const int *digits, const int num)
{
	int i;
	unsigned short mask = 0;

	for (i = 0; i < num; i++)
		mask = setBit(digits[i], mask);

	return mask;
}

// find all digit combinations that generat val based on the given op
int brk(const int cID, const int val, const int num, const char op)
{
	int i, j = 0, keepSearching = 1;
	int digits[NUMBERS];
	unsigned short dMask;
	int calcV;
	int (*calcF)(const int *digits, const int num);


	// initialize calcF based on input op
	switch (op)
	{
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
		default:
			return -1;
	}

	// inint original numbers
	for (i = 0; i < num; i++)
		digits[i] = i + 1 - zero;

	// start scanning all possible combinations
	// for each num numbers check if sun is what we are looking for
	// if yes, add this combination to resultArr

	do
	{
		if (calcF(digits, num) == val)
		{
			dMask = genBMask(digits, num);
			if (addComb(cID, dMask) == NULL)
			{
				j = -1;
				break;
			}
			j++;
		}
	} while (nextNumbers(digits, num));
	return j;
}

// Being called every time new cage is found
_cage *cageCreate(const short int id, const int val, const short int nCells, const char op)
{
	_cage *cPtr;
	_addR *cellPtr;
	int i;

	if ((id >= INDEXS) || (id < 1))
	{
		printf("Invalid cage ID [%d]\n", id);
		return NULL;
	}

	// if cage has already been created - ignore this call
	if (cageInd[id] != NULL)
		return cageInd[id];

	// allocate ram for cell back pointers
	if ((cellPtr = calloc(nCells, sizeof(_addR))) == NULL)
	{
		printf("failed to allocate memory inside cageCreate\n");
		return NULL;
	}
	else
	{
		// inint pointers with -1 we will need that later
		for (i = 0; i < nCells; i++)
		{
			cellPtr[i].col = -1;
			cellPtr[i].raw = -1;
		}
	}

	cPtr = calloc(1, sizeof(_cage));
	if (cPtr != NULL)
	{
		cPtr->value = val;
		cPtr->nCells = nCells;
		cPtr->cellPtr = cellPtr;
		cageInd[id] = cPtr;

		if (brk(id, val, nCells, op) < 0)
			return NULL;
	}

	return cPtr;
}



int readCage(FILE *fp)
{
	char line[100];
	char *tok, op;
	int cageNum, cageVal, nCells;

	while ((fgets(line, sizeof(line) - 1, fp)))
	{
		tok = strstr(line, CageE);
		if (!tok)
		{
			tok = strtok(line, ",");
			if (tok)
			{
				cageNum = atoi(tok);
				tok = strtok(NULL, ",");
				cageVal = atoi(tok);
				tok = strtok(NULL, ",");
				nCells = atoi(tok);
				tok = strtok(NULL,"\n");
				op = getOp(tok);
				if (!cageCreate(cageNum, cageVal, nCells, op))
				{
					printf("failed to add new cage ID [%d]\n", cageNum);
					return 0;
				}
			}
		}
		else
			break;
	}
	return 1;
}

// check for cages with 2 or more cells that all cells are neighbors and no one is isolated

int validateCageId(char *board, int r, int c)
{
	int cId;
	int rm, rp, cm, cp;
	int s = 0;
	_cage *cPtr;

	cId = board[r * NUMBERS + c];

	// no need to check for cage ID 0
	if (cId == 0)
		return 1;

	// for single cell cage - return valid
	cPtr = cageInd[cId];
	if (cPtr == NULL)
	{
		printf("Cage [CID: %d] was not initialized\n", cId);
		return 0;
	}

	if (cPtr->nCells < 2)
		return 1;

	rm = (r > 0 ? r - 1 : r);
	rp = (r < bSize-1 ? r + 1 : r);
	cm = (c > 0 ? c - 1 : c);
	cp = (c < bSize-1 ? c + 1 : c);

	if ((rm != r) && (board[rm * NUMBERS + c] == cId))
		s = 1;
	if ((rp != r) && (board[rp * NUMBERS + c] == cId))
		s = 1;
	if ((cm != c) && (board[r * NUMBERS + cm] == cId))
		s = 1;
	if ((cp != c) && (board[r * NUMBERS + cp] == cId))
		s = 1;

	return s;
}

// cage backpointers are two bit masks holding the numbers of raws as columns of cells
// belonging to a particular cage.
void updateBackPointers(int cId, int r, int c)
{
	_cage *cPtr;
	int i;

	// check that input cId is in frange, otherwise ignore
	if ((cId < 1) || (cId >= INDEXS))
		return;

	cPtr = cageInd[cId];

	if (cPtr != NULL)
	{
		i = 0;

		// advance to first empty address slot
		while (i < cPtr->nCells)
		{
			if (cPtr->cellPtr[i].raw >= 0)
				i++;
			else
				break;
		}

		if (i < cPtr->nCells)
		{
			cPtr->cellPtr[i].raw = r;
			cPtr->cellPtr[i].col = c;
		}
		else
			printf("Failed to update back pointer. All slots are full. [cId: %d] [raw: %d] [col: %d]\n", cId, r, c);
	}
	else
		printf("Error updating back pointers [cId: %d] [raw: %d] [col: %d]\n", cId, r, c);
}

int initCalcudoku(_brdR *pBlk, char *iBoard, char *cBoard)
{
	char line[100];
	int i, j, ind, failure = 0;
	char *tok;

	// Validate input board.
	// validateCageId will also update each cage with its backpointers to board cells
	for (i = 0; i < bSize && !failure; i++)
	{
		for (j = 0; j < bSize && !failure; j++)
			if (validateCageId(cBoard, i, j))
			{
				ind = i * NUMBERS + j;

				//update cage backpointers
				updateBackPointers(cBoard[ind], i, j);

				// fill result board with initial values
				if (iBoard[ind] > 0)
				{
					if (!setNewValue(pBlk, i, j, iBoard[ind]))
					{
						printf("Invalid board, can't add value: %d in (r:%d) (c:%d)\n", iBoard[ind], i + 1, j + 1);
						failure = 1;
					}
				}
			}
			else
			{
				failure = 1;
				printf("failed to update cage %d, in cell [%d, %d]\n", cBoard[i*NUMBERS+j], i, j);
			}

	}


	for (i = 0; i < 9; i++)
	{
		for (j = 0; j < 9; j++)
		{

		}
	}

	return (!failure);
}

// Set Combo Bits.
// Update all combo bits according to given cell value.
void setCageComboBits (int cId, int val)
{
	_cage *cPtr;
	_comb *combPtr;

	cPtr = cageInd[cId];

	if (cPtr)
	{
		combPtr = cPtr->ptr;

		while (combPtr)
		{
			if (isBitSet(val, combPtr->remainingDigits))
			{
				combPtr->remainingDigits = clearBit (val, combPtr->remainingDigits);
			}
			else
			{
				combPtr->remainingDigits = 0;
			}
			combPtr = combPtr->next;			
		}
	}
}

// set digit combination change flag and reset all combination digits
void resetCombChange()
{
	int i;
	_cage *cPtr;
	_comb *combPtr;

	for (i = 1; i < INDEXS; i++)
	{
		if ((cPtr = cageInd[i]) != NULL)
		{
			combPtr = cPtr->ptr;

			while (combPtr)
			{
				combPtr->remainingDigits = combPtr->allDigits;			
				combPtr = combPtr->next;
			}
		}
	}
}


// reset all cages after returning from failed solvBoard()
void restoreCageStates(const _brdR *board)
{
	int i, j;
	char *cBoard;

	cBoard = board->cBoard;

	if (!cBoard)
		return;

	// reset all combinations digits in all cages
	resetCombChange();

	for (i = 0; i < bSize; i++)
	{
		for (j = 0; j < bSize; j++)
		{
			if (board->score[i][j] < 0)
				setCageComboBits(cBoard[i * NUMBERS + j], board->board[i][j]);
		}
	}
}

void printBoard (char *b)
{
	int i,j;

	for (i = 0; i < bSize; i++)
	{
		for (j = 0; j < bSize; j++)
			printf("%d ", b[i*NUMBERS+j]);

		printf("\n");
	}
}

void printStat(_brdR *b, int r, int c, int v)
{
	switch (loging)
	{
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
void updateLines (_brdR *b, const int r, const int c, const char v)
{
	int i;

	for (i = 0; i < bSize; i++)
	{
		b->bitMask[r][i] = clearBit(v, b->bitMask[r][i]);
		b->bitMask[i][c] = clearBit(v, b->bitMask[i][c]);

		if (b->score[r][i] > 0)
			b->score[r][i] = countSetBits(b->bitMask[r][i]);
		if (b->score[i][c] > 0)
			b->score[i][c] = countSetBits(b->bitMask[i][c]);
	}
}


//set new value in a given cell
int setNewValue (_brdR *b, int r, int c, char v)
{
	int cId;

	if (!b->cBoard)
		return 0;
	else
		cId = b->cBoard[r * NUMBERS + c];

	iter++;
	// check that selected location is free, otherwise return error
	if ((b->board[r][c] == 0) && (isBitSet(v, b->bitMask[r][c])))
	{
		b->board[r][c] = v;
		b->score[r][c] = -1;

		updateLines(b, r, c, v);

		// first update cage information based on given set value
		setCageComboBits(cId, v);

		// Then update back all relevant calcudoku cells
		updateCage(b, cId);

		b->cells++;

		if (loging)
			printStat(b, r, c, v);
		return 1;
	}
	else
		return 0;
}

// find the first non set cell location
// maxOpt indicates max value options allowed for the selected cell
int findFirstNonSet (_brdR *b, int *r, int *c, int maxOpt)
{
	int i, j, score;

	// if board is complete no next cell for update
	if (b->cells == bSize*bSize)
		return 0;

	for (i = 0; i < bSize; i++)
		for (j = 0; j < bSize; j++)
		{
			score = b->score[i][j];
			if(score > -1)
			{
				if ((score > 0) && (score <= maxOpt))
				{
					*r = i;
					*c = j;
					return 1;
				}
				else if (score == 0)
					// dead-end
					return 0;
			}
		}

	return 0;
}


// this is the recursive function that is used to solve the board
// the function will update the given board only if it managed to sove it.
// otherwise it is using a local copy to try and fing solution using recursion calls
int solveBoard (_brdR *board, const int raw, const int col, const char value)
{
	int r , c, cellsC = bSize * bSize;
	_brdR *b;
	int changed;
	char v;
	unsigned short bitmask;

	stack++;
	deepest = (deepest > stack? deepest: stack);

	// create local copy of board and sent it to solvBoard()
	if ((b = malloc(sizeof(_brdR))) == NULL)
		return 0;
	memcpy(b, board, sizeof(_brdR));

	//  if we call solveBoard with Value > 0 we need first to change the input board to have this value
	// as final value for this recursion
	if (value > -1)
	{
		if (!setNewValue(b, raw, col, value))
		{
			printf("fatal Error!\n");
			return 0;
		}
	}

	
	// try to solve board assuming we now might have new cells with single possible value option
	changed = 1;
	while (changed && (b->cells < cellsC))
	{
		changed = findFirstNonSet(b, &r, &c, 1);
		if	(changed)
			changed = setNewValue(b, r, c, getVal(b->bitMask[r][c]));
	}

	if (b->cells == cellsC)
	{
		// found solution - exit recursion
		memcpy(board, b, sizeof(_brdR));
		free(b);
		stack--;
		return 1;
	}


	// if we get here, it means we could not find any other cells with single value option.
	// now scan the board for all cells with more value options
	if (findFirstNonSet(b, &r, &c, bSize))
	{
		bitmask = b->bitMask[r][c];
		while ((v = getVal(bitmask)) > -1)
		{
			// recursive call !!
			if (!solveBoard(b, r, c, v))
			{
				bitmask = clearBit(v, bitmask);
				restoreCageStates(b);
			}
			else
			{
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

void init_brdR(_brdR *brd)
{
	int i, j;
	unsigned short mask = 0;

	// constract mask. By default we set bits 1 to bSize. If zero is set then we set bits 0 to bSize-1
	for (i = 0; i < bSize; i++)
	{
		mask = setBit(i+1-zero, mask);
	}

	brd->cells = 0;

	for (i = 0; i < bSize; i++)
	{
		for (j = 0; j < bSize; j++)
		{
			brd->board[i][j] = 0;
			brd->bitMask[i][j] = mask;
			brd->score[i][j] = bSize;
		}
	}

}

int main (int argc, char **argv)
{
	char  opt, *fname = NULL, str[31];
	FILE *fp = NULL;
	int stat = 1, i;
	char cBoard[NUMBERS][NUMBERS];
	char iBoard[NUMBERS][NUMBERS];
	_brdR *pBlk;

	// get commad line input
	if (argc > 1)
	{
		while ((opt = getopt(argc, argv, "f:l:s:z")) > 0)
			switch (opt)
			{
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
			case 's':
				bSize = atoi(optarg);
				break;
			default:
				printf("wrong input\n");
				return stat;
			}
		}

	if (!fp)
	{
		printf("error opening file %s, error: %s\n Change to stdin\n", fname, strerror(errno));
		fp = stdin;
	}

	// allocate board record
	if ((pBlk = calloc(1, sizeof(_brdR))) == NULL)
		return stat;

	// inint cage index array
	if ((cageInd = cageIndInit()) == NULL)
	{
		printf("failed to inintialize cage index array\n");
		return stat;
	}

	// init pBlk
	init_brdR (pBlk);
	pBlk->cBoard = &cBoard[0][0];

	if (!readInput(fp, &iBoard[0][0], &cBoard[0][0]))
	{
		printf("failed to read input file\n");
		return stat;
	}

	if (!initCalcudoku(pBlk, &iBoard[0][0], &cBoard[0][0]))
	{
		printf("failed to initialize Clacudoku board\n");
		return stat;
	}

	// after initilizing resut board based on initput board
	// we need to update bitmasks on all cages before we go to solveboard
	for (i = 1; i < INDEXS; i++)
		updateCage(pBlk, i);

	iter=0;
	if ((stat = !solveBoard(pBlk, 0, 0, -1)))
		printf("failed to find solution\n");

	printBoard(&(pBlk->board[0][0]));
	printf("Iterations: %I64d (recd:%I64d)\n", iter, deepest);
	free(pBlk);

	if (fp != stdin)
		fclose(fp);

	releaseMem();

	return stat;
}
