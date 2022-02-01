#include "STATS.H"

#include <MEMORY.H>
#include <STDIO.H>

void init_savedata(savedata_t* out)
{
	memset(out, 0, sizeof(savedata_t));
}

int load_data(const char* fpath, savedata_t* out, unsigned thash)
{
	int read = 0;
	FILE* fd = fopen(fpath, "rb");
	if (fd)
	{
		read = fread(out, sizeof(savedata_t), 1, fd);
		if (read && (out->timehash != thash))
			read = 0;
		fclose(fd);
	}
	return read;
}

int save_data(const char* fpath, const savedata_t* in)
{
	FILE* fd = fopen(fpath, "wb");
	if (fd)
	{
		fwrite(in, sizeof(savedata_t), 1, fd);
		fclose(fd);
		return 1;
	}
	return 0;
}
