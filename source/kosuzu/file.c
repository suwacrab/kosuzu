#include <stdbool.h>
#include <stdio.h>
#include "kosuzu.h"

static bool file_checkOK(KOSUZU_FILE *file);

/* I/O functions ------------------------------------------------------------*/
int kosuzu_file_close(KOSUZU_FILE *file) {
	if(!file_checkOK(file)) return false;
	if(!file->is_open) return false;
	file->is_open = false;
	return true;
}
/*int kosuzu_file_seek(KOSUZU_FILE *file,long int offset,int whence) {
	if(!file_checkOK(file)) return false;
	if(!file->is_open) return false;
	return true;
}*/
size_t kosuzu_file_read(KOSUZU_FILE *file,void *output,size_t size) {
	if(!file_checkOK(file)) return 0;
	if(!file->is_open) return 0;

	const KOSUZU_ARCHIVE *archive = file->archive_ptr;
	size_t read_size = size;

	/* make sure to not read past file ------------------*/
	if(kosuzu_file_eof(file)) return 0;
	if((size + file->file_offset) > file->file_size) {
		read_size = file->file_size - file->file_offset;
	}

	/* read data ----------------------------------------*/
	fread(output,sizeof(char),read_size,archive->file_ptr);
	file->file_offset += read_size;
	return read_size;
}

int kosuzu_file_eof(KOSUZU_FILE *file) {
	if(!file_checkOK(file)) return false;
	if(!file->is_open) return false;

	if(file->file_offset >= file->file_size) return true;
	return false;
}

/* misc functions -----------------------------------------------------------*/
static bool file_checkOK(KOSUZU_FILE *file) {
	if(!file) return false;
	if(!file->archive_ptr) return false;
	if(!file->file_node) return false;
	return true;
}

