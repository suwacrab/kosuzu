#include "kosuzu.h"

int kosuzu_savequeue_setup(KOSUZU_SAVEQUEUE *queue,KOSUZU_SAVEENTRY entries[],const size_t entry_max) {
	if(!queue) return false;
	if(!entries) return false;

	return false;
}
int kosuzu_savequeue_addFile(KOSUZU_SAVEQUEUE *queue,const char *arc_path,const char *name,const char *filename) {
	return false;
}
int kosuzu_savequeue_addFolder(KOSUZU_SAVEQUEUE *queue,const char *arc_path,const char *name) {
	return false;
}
int kosuzu_savequeue_addSint(KOSUZU_SAVEQUEUE *queue,const char *arc_path,const char *name,const int32_t num) {
	return false;
}
int kosuzu_savequeue_addUint(KOSUZU_SAVEQUEUE *queue,const char *arc_path,const char *name,const uint32_t num) {
	return false;
}

