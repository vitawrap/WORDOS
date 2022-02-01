#ifndef _WORDLIST_H
#define _WORDLIST_H

#define LETTER_COUNT 5
#define MAX_ATTEMPTS 6

extern int WORD_COUNT;

const char* get_word(int at);

/* requires letter-based rle */
int find_word(char const* w);

void setup_word_map(void);

#endif