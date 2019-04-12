
#ifndef CALCU_H_
#define CALCU_H_

#include <stdio.h>

#define isBitSet(b, c) ((1 << (b) & (c)))
#define setBit(b, c) ((1 << (b)) | (c))
#define clearBit(b, c) ((~(1 << (b))) & (c))

#define INDEXS 226 
#define NUMBERS 15
#define INVALIDNUMBER -1

typedef struct __add
{
    char raw;
    char col;
} _addR;

typedef struct __combHdr
{
    unsigned short allDigits;       // list of digits constructing the sun
    unsigned short remainingDigits; // remaining unassigned digits
    struct __comb *next;            // next combination record
} _combHdr;

typedef struct __digit          // -1 indicates no digit (i.e. digit was cleared)
{
    char oDigit;                // belongs to Original combination - will be used for cage state reset
    char rDigit;                // belongs to remining combination during solving process
} _digit;

typedef struct __comb
{
    _combHdr hdr;                   // fix part of the record
    _digit  digits[1];          // variable part - this array will change from cage to cage
} _comb;

typedef struct __cage
{
    int value;          // value of cage
    int nCells;         // number of cells in this cage. It is also the number of digits for this cage
    _addR *cellPtr;     // arry of Pointers back to board cells
    _comb *ptr;         // point to list of combination records
} _cage;

typedef struct __brd
{
    int board[NUMBERS][NUMBERS];                 // the result board
    int score[NUMBERS][NUMBERS];                 // score board
    unsigned short int bitMask[NUMBERS][NUMBERS]; // bit mask for representing possible values
    int cells;
    int *cBoard;                       // Pointer to Calcudo board
} _brdR;

/********************************************************************************************
*	Utility functions
*********************************************************************************************/
int getVal(unsigned short int bits);
int countSetBits(unsigned short int num);
int readInput(FILE *fp, int *cBoard);
int readBoard(FILE *fp, int *board, char *token);
int readLineB(int *board, char *line);
char getOp(char *str);
unsigned short inDigits(_digit *digits, const char *digitsA, const int nDigits);
unsigned short clearDigit(const int val, _digit *digits, const int nDigits);
void simpleSort(char *digitsS, const char *digits, const int nDigits);
int compDigits(const char *digits1, const _digit *digits2, const int nDigits);
unsigned short resetDIgits(_digit *digits, int nDigits);

/*************************************************************
*	calcu related functions
*********************************************************************************************/
void init_brdR(_brdR *brd);
int nextNumbers(char *digits, const int num);
int calcSum(const char *digits, const int num);
int calcSub(const char *digits, const int num);
int calcMul(const char *digits, const int num);
int calcOr(const char *digits, const int num);
int calcDiv(const char *digits, const int num);
void updateCage(_brdR *b, int cId);
void releaseMem();
_cage **cageIndInit();
_comb *addComb(const short int cID, const char *digits);

int brk(const int cID, const int sum, const int num, const char op);
_cage *cageCreate(const short int id, const int val, const short int nCells, const char op);
int readCage(FILE *fp);
int validateCageId(int *board, int r, int c);
void updateBackPointers(int cId, int r, int c);
void restoreCageStates(const _brdR *board);
int initCalcudoku(int *cBoard);
void resetCombChange();
void printCageState();

void printBoard(int *b);
void printStat(_brdR *b, int r, int c, int v);
void updateLines(_brdR *b, const int r, const int c, const int v);
void updateBlock(_brdR *b, const int r, const int c, const int v);
int setNewValue(_brdR *b, int r, int c, int v);
int findFirstNonSet(_brdR *b, int *r, int *c, int maxOpt);
int initSudo(FILE *fp, _brdR *pBlk, int *sBoard);
int solveBoard(_brdR *board, const int raw, const int col, const int value);
int readSBoard(int *sBoard, FILE *fp);

#endif /* CALCU_H_ */
