#include <stdbool.h>
#include <stdio.h>
#include "kosuzu.h"

static bool file_checkOK(KOSUZU_FILE *file) {
	if(!file) return false;
	if(!file->archive_ptr) return false;
	if(!file->file_node) return false;
	return true;
}

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
int kosuzu_file_read(KOSUZU_FILE *file,void *output,size_t size) {
	if(!file_checkOK(file)) return false;
	if(!file->is_open) return false;
	const KOSUZU_ARCHIVE *archive = file->archive_ptr;
	fread(output,sizeof(char),size,archive->file_ptr);
	file->file_offset += size;
	return true;
}

