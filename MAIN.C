/*
	WOR-DOS: WORDLE for MS-DOS
	C89 with Borland Turbo-C++ 3
	by vitawrap @ github (2022)
*/

#include <STDLIB.H>
#include <TIME.H>

#include <STDIO.H>
#include <CONIO.H>

#include "WORDLIST.H"
#include "STATS.H"

#define VER_MAJ 1
#define VER_MIN 1

/* Text mode attributes */
const char attr_exists  = 0x6F;
const char attr_unknown = 0x4F;
const char attr_match	= 0x2F;
const char attr_mismatch= 0x1F;
const char attr_input	= 0x78;

#define ROW_OFFSET 16

void print_word(char* seq, char tattr, int col, int wid)
{
	int i;
	const char* clword = NULL;
	int rof = ROW_OFFSET;
	int cof = (col*2)+3;
	char creg = 0;
	char seqstr[LETTER_COUNT+1] = {0};
	int len = 0;

	char ltrcount[26] = {0};
	char attrs[LETTER_COUNT] = {0};
	int matchcnt = 0;

	if (!seq) return;

	strncpy(seqstr, seq, LETTER_COUNT);
	strlwr(seqstr);

	len = strlen(seqstr);

	/* find_word is called before this
	   we know we don't need to sanitize
	   our input word besides strlwr.
	 */
	if (wid != -1)
	{
		clword = get_word(wid);
		memset(attrs, attr_mismatch, sizeof(attrs));

		/* first establish a letter count list */
		for (i = 0; i < LETTER_COUNT; ++i)
			++ltrcount[clword[i] - 97];

		/* remove 1 to letters with a pos match */
		for (i = 0; i < len; ++i)
		{
			creg = seqstr[i];
			if (creg == clword[i])
			{
				--ltrcount[creg - 97];
				attrs[i] = attr_match;
				++matchcnt;
			}
		}

		/* pass 2 if we didnt match the word */
		if (matchcnt < LETTER_COUNT)
		{
			for (i = 0; i < len; ++i)
			{
				creg = seqstr[i];
				if (ltrcount[creg - 97] && (attrs[i] != attr_match))
				{
					attrs[i] = attr_exists;
					--ltrcount[creg - 97];
				}
			}
		}
	}

	/* then color appropriately */
	for (i = 0; i < LETTER_COUNT; ++i)
	{
		creg = seqstr[i];

		window(rof+(i*2), cof, rof+1+(i*2), cof+1);
		if (wid == -1)
			textattr(tattr);
		else
		{
			textattr(attrs[i]);
		}
		/* wid should be -1 when we are missing chars */
		putch(creg? creg - 32 : ' ');
	}

	/* show cursor */
	if (len < LETTER_COUNT)
	{
		window(rof+(len*2), cof, rof+1+(len*2), cof+1);
		gotoxy(1, 1);
	}

	/* debug */
	/*
	window(1,1,40,2);
	gotoxy(1,1);
	cputs(seqstr);
	*/
}

unsigned day_hash()
{
	time_t timer;
	struct tm* gmt;
	unsigned thash = 0;

	timer = time(NULL);
	gmt = gmtime(&timer);
	thash = gmt->tm_yday;		/* lower 9 bits */
	thash |= gmt->tm_year << 9; /* higher 7 bits */

	return thash;
}

int main(int argc, char* argv[])
{
	unsigned char creg = 0xFF;
	unsigned char clast= 0xFF;
	int ltr = 0;
	unsigned id;
	unsigned thash = day_hash();
	char attempts[MAX_ATTEMPTS][LETTER_COUNT+1] = {0};
	int attempt = 0;
	int success = 0;
	int i = 0;
	int quitreq = 0;
	int dispred = 0;
	int serz = 0;

	savedata_t sdata = {0};

	/* must do this fist */
	setup_word_map();

	srand(thash);
	id = rand() % WORD_COUNT;

	memset(attempts, 0, sizeof(attempts));

	/* check on saved data */
	serz = load_data("w.sav", &sdata, thash);
	if (serz)
	{
		memcpy(attempts, sdata.attempts, sizeof(attempts));
		attempt = sdata.attempt;
		success = sdata.success;
	}

	lowvideo();
	textmode(C40);
	textattr(0x0F);
	clrscr();

	gotoxy(17, 1);
	cputs("WOR-DOS");

	while ((attempt < MAX_ATTEMPTS) && !quitreq && !success)
	{
		/* print all attempts */
		for (i = 0; i < attempt; ++i)
			print_word(attempts[i], attr_match, i, id);

		/* print current attempt */
		print_word(attempts[attempt],dispred?attr_unknown:attr_input, i, -1);
		dispred = 0;

		while (!quitreq)
		{
			if (kbhit())
			{
				clast = creg;
				creg = getch();

				/* are we quitting? (esc) */
				if (creg == 27)
				{
					quitreq = 1;
					break;
				}

				/* are we removing a letter? */
				if ((creg == 8) && (ltr > 0))
				{
					--ltr;
					attempts[attempt][ltr] = '\x0';
					break;
				}

				/* or are we submitting a word? */
				if ((creg == '\r') && (ltr == LETTER_COUNT))
				{
					if (find_word(attempts[attempt]) != -1)
					{
						if (!strcmp(attempts[attempt],get_word(id)))
						{
							success = 1;
							quitreq = 1;
						} else {
							ltr = 0;
						}
						++attempt;
					}
					else
					{
						dispred = 1;
					}
					break;
				}

				/* are we giving a letter? */
				creg = tolower(creg);
				if (islower(creg) && (ltr < LETTER_COUNT))
				{
					attempts[attempt][ltr]	= creg;
					attempts[attempt][ltr+1]= '\x0'; /* :-) */
					++ltr;
					break;
				}
			}
		}
	}

	/* end screen, display all attempts, no input. */
	for (i = 0; i < attempt; ++i)
		print_word(attempts[i], attr_match, i, id);

	/* clear current attempt buffer */
	if (attempt < MAX_ATTEMPTS)
		memcpy(attempts[i], 0, sizeof(attempts[0]));

	/* serialize game data */
	memcpy(sdata.attempts, attempts, sizeof(attempts));
	sdata.attempt = attempt;
	if (sdata.timehash != thash) /* upd. stats only after a game */
	{
		if (success)
			++sdata.streak;
		else
			sdata.streak = 0;
		sdata.wins += success;
		sdata.losses += !success;
	}
	sdata.success = success;
	sdata.timehash= thash;
	serz = save_data("w.sav", &sdata);

	/* display stats and quit message */
	textattr(0x0F);
	window(1,16,40,25);
	if (success || (attempt == MAX_ATTEMPTS))
	{
		cprintf("You %s the word!\r\nCheck again tomorrow!\r\n",
			success? "\aFOUND": "DID NOT find"
		);
		cprintf("Attempts: %c/%i\r\n",
			success? attempt + 48 : 'X', MAX_ATTEMPTS);
		cprintf("Streak: %i\r\n", sdata.streak);
		cprintf("All-time W: %i L: %i\r\n", sdata.wins, sdata.losses);
		cprintf("Win ratio: %.3f%%\r\n",
			((float)sdata.wins / (sdata.wins + sdata.losses)) * 100.f);
	}

	/* demake credits :-) */
	textattr(0x1E);
	cprintf("\n WOR-DOS %i.%i by @viwrap \r\n", VER_MAJ, VER_MIN);

	getch();
	textmode(C80);
	textattr(0x0F);
	normvideo();
	return 0;
}