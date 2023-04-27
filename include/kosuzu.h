#ifndef KOSUSU_H
#define KOSUSU_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* define & enum ------------------------------------------------------------*/
enum KOSUZU_NODETYPE {
	KOSUZU_NODETYPE_INT,
	KOSUZU_NODETYPE_UINT,
	KOSUZU_NODETYPE_USERDATA,
	KOSUZU_NODETYPE_FOLDER
};
#define KOSUZU_NODE_INVALID (-1)
#define KOSUZU_FILE_MAXOPEN (16)

#define KOSUZU_SEEK_SET (0)
#define KOSUZU_SEEK_CUR (1)
#define KOSUZU_SEEK_END (2)

/* types --------------------------------------------------------------------*/
typedef struct KOSUZU_SAVEENTRY {
	const char *name;
	const char *arc_path;
	int is_file;
	int out_type;
	union {
		int32_t		value_sint;
		uint32_t	value_uint;
		struct {
			const char *src_filename;
		};
		struct {
			const void *udata_ptr;
			size_t udata_size;
		};
	} i;
} KOSUZU_SAVEENTRY;
typedef struct KOSUZU_SAVEINFO {
	void **out_buffer;
	const KOSUZU_SAVEENTRY *entries;
	size_t entry_count;
	const char *out_filename;

	int flag_verbose;
} KOSUZU_SAVEINFO;
typedef struct KOSUZU_SAVEQUEUE {
	KOSUZU_SAVEENTRY *entry_ptr;
	size_t entry_count;
	size_t entry_max;
} KOSUZU_SAVEQUEUE;

typedef struct KOSUZU_FILEHEADER {
	char magic[8];
	uint32_t node_count;
	uint32_t tree_count;
	uint32_t data_size;

	uint32_t offset_nodes;
	uint32_t offset_trees;
	uint32_t offset_data;
} KOSUZU_FILEHEADER;
typedef struct KOSUZU_FILENODE {
	uint32_t name_hash;
	uint32_t node_type;
	union {
		int32_t		value_sint;
		uint32_t	value_uint;

		struct {
			uint32_t udata_offset;
			uint32_t udata_size;
		};

		struct {
			uint32_t folder_treeindex;
			uint32_t folder_size;
		};
	} d;	
} KOSUZU_FILENODE;

typedef struct KOSUZU_NODE {
	uint32_t name_hash;
	uint16_t node_type;
	int16_t node_index;
	union {
		int32_t		value_sint;
		uint32_t	value_uint;

		struct {
			uint32_t udata_offset;
			uint32_t udata_size;
		};

		struct {
			uint32_t folder_treeindex;
			uint32_t folder_size;
		};
	} d;	
} KOSUZU_NODE;
typedef struct KOSUZU_FILE {
	struct KOSUZU_ARCHIVE *archive_ptr;
	const KOSUZU_NODE *file_node;
	size_t file_offset;
	size_t file_size;

	int is_open;
} KOSUZU_FILE;
typedef struct KOSUZU_ARCHIVE {
	size_t node_count;
	size_t tree_count;
	size_t data_offset;

	size_t fldr_current;

	KOSUZU_NODE *data_nodes;
	uint16_t *data_trees;
	FILE *file_ptr;
	int did_openFile;

	KOSUZU_FILE file_list[KOSUZU_FILE_MAXOPEN];
} KOSUZU_ARCHIVE;

/* functions ----------------------------------------------------------------*/
int kosuzu_savequeue_setup(KOSUZU_SAVEQUEUE *queue,KOSUZU_SAVEENTRY entries[],const size_t entry_max);
int kosuzu_savequeue_addEntry(KOSUZU_SAVEQUEUE *queue,const KOSUZU_SAVEENTRY *entry);
int kosuzu_savequeue_addFolder(KOSUZU_SAVEQUEUE *queue,const char *arc_path,const char *name);
int kosuzu_savequeue_addUdata(KOSUZU_SAVEQUEUE *queue,const char *arc_path,const char *name,const void *data,size_t data_size);
int kosuzu_savequeue_addFile(KOSUZU_SAVEQUEUE *queue,const char *arc_path,const char *name,const char *filename);
int kosuzu_savequeue_addSint(KOSUZU_SAVEQUEUE *queue,const char *arc_path,const char *name,const int32_t num);
int kosuzu_savequeue_addUint(KOSUZU_SAVEQUEUE *queue,const char *arc_path,const char *name,const uint32_t num);
int kosuzu_savequeue_saveFile(KOSUZU_SAVEQUEUE *queue,const char *filename);

int kosuzu_save(void **out_buffer, const KOSUZU_SAVEENTRY entries[],const size_t entry_count);
int kosuzu_saveFile(const char *out_filename,const KOSUZU_SAVEENTRY entries[],const size_t entry_count);
int kosuzu_saveEX(const KOSUZU_SAVEINFO *save_info);

int kosuzu_archiveOpen(KOSUZU_ARCHIVE *archive,FILE *file_ptr);
int kosuzu_archiveOpenFile(KOSUZU_ARCHIVE *archive,const char *src_filename);
int kosuzu_archiveClose(KOSUZU_ARCHIVE *archive);

int kosuzu_archiveChdir(KOSUZU_ARCHIVE *archive,const char *dir_name);
const KOSUZU_NODE *kosuzu_archiveNodeFind(KOSUZU_ARCHIVE *archive,const char *name);
const KOSUZU_NODE *kosuzu_archiveNodeGet(KOSUZU_ARCHIVE *archive,size_t index);
const KOSUZU_NODE *kosuzu_archiveNodeGetCurFldr(KOSUZU_ARCHIVE *archive);
KOSUZU_FILE *kosuzu_archiveFileOpen(KOSUZU_ARCHIVE *archive,const char *name);

int kosuzu_file_close(KOSUZU_FILE *file);
int kosuzu_file_seek(KOSUZU_FILE *file,long int offset,int whence);
size_t kosuzu_file_read(KOSUZU_FILE *file,void *output,size_t size);
int kosuzu_file_eof(KOSUZU_FILE *file);

uint32_t kosuzu_hashString(const char *str);

#ifdef __cplusplus
}
#endif

/* C++ portion --------------------------------------------------------------*/
#ifdef __cplusplus
class CKosuzuArchive : public KOSUZU_ARCHIVE {
	int open(FILE *fptr) {
		return kosuzu_archiveOpen(this,fptr);
	}	
	int openFile(const char *filename) { 
		return kosuzu_archiveOpenFile(this,filename);
	}
	int close() { return kosuzu_archiveClose(this); }
};

class CKosuzuSaveQueue : public KOSUZU_SAVEQUEUE {
	public:
		int setup(KOSUZU_SAVEENTRY entries[],size_t max) {
			return kosuzu_savequeue_setup(this,entries,max);
		}
		int add_folder(const char *arc_path,const char *name) {
			return kosuzu_savequeue_addFolder(this,arc_path,name);
		}
		int add_file(const char *arc_path,const char *name,const char *filename) {
			return kosuzu_savequeue_addFile(this,arc_path,name,filename);
		}
		int add_udata(const char *arc_path,const char *name,const void *data,size_t data_size) {
			return kosuzu_savequeue_addUdata(this,arc_path,name,data,data_size);
		}
		int add_sint(const char *arc_path,const char *name,int32_t value) {
			return kosuzu_savequeue_addSint(this,arc_path,name,value);
		}
		int add_uint(const char *arc_path,const char *name,uint32_t value) {
			return kosuzu_savequeue_addUint(this,arc_path,name,value);
		}
		int save_file(const char *filename) {
			return kosuzu_savequeue_saveFile(this,filename);
		}
	private:

};

#endif

#endif

