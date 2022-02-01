#ifndef _STATS_H
#define _STATS_H

#include "WORDLIST.H"

typedef struct
{
	unsigned timehash;
	char attempts[MAX_ATTEMPTS][LETTER_COUNT+1];
	int attempt;
	int success;
	int streak;
	int wins;
	int losses;
} savedata_t;

void init_savedata(savedata_t* out);

int load_data(const char* fpath, savedata_t* out, unsigned thash);

int save_data(const char* fpath, const savedata_t* in);

#endif