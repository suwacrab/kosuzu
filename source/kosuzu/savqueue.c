#include <stdbool.h>
#include "kosuzu.h"

int kosuzu_savequeue_setup(KOSUZU_SAVEQUEUE *queue,KOSUZU_SAVEENTRY entries[],const size_t entry_max) {
	if(!queue) return false;
	if(!entries) return false;

	queue->entry_ptr = entries;
	queue->entry_max = entry_max;
	queue->entry_count = 0;
	return false;
}
int kosuzu_savequeue_addEntry(KOSUZU_SAVEQUEUE *queue,const KOSUZU_SAVEENTRY *entry) {
	if(!queue) return false;
	if(!entry) return false;

	if(queue->entry_count >= queue->entry_max) return false;
	queue->entry_ptr[queue->entry_count++] = *entry;
}
int kosuzu_savequeue_addFile(KOSUZU_SAVEQUEUE *queue,const char *arc_path,const char *name,const char *filename) {
	KOSUZU_SAVEENTRY entry = {
		.name = name,		.arc_path = arc_path,
		.is_file = true,	.out_type = KOSUZU_NODETYPE_USERDATA,
		.i.src_filename = filename
	};
	return kosuzu_savequeue_addEntry(queue,&entry);
}
int kosuzu_savequeue_addFolder(KOSUZU_SAVEQUEUE *queue,const char *arc_path,const char *name) {
	KOSUZU_SAVEENTRY entry = {
		.name = name,		.arc_path = arc_path,
		.out_type = KOSUZU_NODETYPE_FOLDER
	};
	return kosuzu_savequeue_addEntry(queue,&entry);
}
int kosuzu_savequeue_addSint(KOSUZU_SAVEQUEUE *queue,const char *arc_path,const char *name,const int32_t num) {
	KOSUZU_SAVEENTRY entry = {
		.name = name,		.arc_path = arc_path,
		.out_type = KOSUZU_NODETYPE_INT,
		.i.value_sint = num
	};
	return kosuzu_savequeue_addEntry(queue,&entry);
}
int kosuzu_savequeue_addUint(KOSUZU_SAVEQUEUE *queue,const char *arc_path,const char *name,const uint32_t num) {
	KOSUZU_SAVEENTRY entry = {
		.name = name,		.arc_path = arc_path,
		.out_type = KOSUZU_NODETYPE_UINT,
		.i.value_sint = num
	};
	return kosuzu_savequeue_addEntry(queue,&entry);
}

int kosuzu_savequeue_saveFile(KOSUZU_SAVEQUEUE *queue,const char *filename) {
	if(!queue) return false;
	if(!filename) return false;

	return kosuzu_saveFile(filename,queue->entry_ptr,queue->entry_count);
}

