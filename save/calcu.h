
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

typedef struct __comb
{
    unsigned short allDigits;       // list of digits constructing the sun
    unsigned short remainingDigits; // remaining unassigned digits
    struct __comb *next;            // next combination record
} _comb;

typedef struct __cage
{
    int value;          // value of cage
    int nCells;         // number of cells in this cage
    _addR *cellPtr;     // arry of Pointers back to board cells
    _comb *ptr;         // point to list of combination records
} _cage;

typedef struct __brd
{
    char board[NUMBERS][NUMBERS];                 // the result board
    char score[NUMBERS][NUMBERS];                 // score board
    unsigned short int bitMask[NUMBERS][NUMBERS]; // bit mask for representing possible values
    char cells;
    char *cBoard;                       // Pointer to Calcudo board
} _brdR;

/********************************************************************************************
*	Utility functions
*********************************************************************************************/
char getVal(unsigned short int bits);
char countSetBits(unsigned short int num);
int readInput(FILE *fp, char *iBoard, char *cBoard);
int readBoard(FILE *fp, char *board, char *token);
int readLineB(char *board, char *line);
char getOp(char *str);
/*************************************************************
*	calcu related functions
*********************************************************************************************/
void init_brdR(_brdR *brd);
int nextNumbers(int *digits, const int num);
int calcSum(const int *digits, const int num);
int calcSub(const int *digits, const int num);
int calcMul(const int *digits, const int num);
int calcDiv(const int *digits, const int num);
char getVal(unsigned short int bits);
void updateCage(_brdR *b, int cId);
void releaseMem();
_cage **cageIndInit();
_comb *addComb(short int cID, unsigned short digits);
unsigned short genBMask(const int *digits, const int num);
int brk(const int cID, const int sum, const int num, const char op);
_cage *cageCreate(const short int id, const int val, const short int nCells, const char op);
int readCage(FILE *fp);
int validateCageId(char *board, int r, int c);
void updateBackPointers(int cId, int r, int c);
void restoreCageStates(const _brdR *board);
int initCalcudoku(_brdR *pBlk, char *iBoard, char *cBoard);
void resetCombChange();
void printCageState();

void printBoard(char *b);
void printStat(_brdR *b, int r, int c, int v);
void updateLines(_brdR *b, const int r, const int c, const char v);
void updateBlock(_brdR *b, const int r, const int c, const char v);
int setNewValue(_brdR *b, int r, int c, char v);
int findFirstNonSet(_brdR *b, int *r, int *c, int maxOpt);
int initSudo(FILE *fp, _brdR *pBlk, char *sBoard);
int solveBoard(_brdR *board, const int raw, const int col, const char value);
int readSBoard(char *sBoard, FILE *fp);

#endif /* CALCU_H_ */
