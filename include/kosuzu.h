#ifndef KOSUZU_H
#define KOSUZU_H

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
	struct KOSUZU_RECORD *archive_ptr;
	const KOSUZU_NODE *file_node;
	size_t file_offset;
	size_t file_size;

	int is_open;
} KOSUZU_FILE;
typedef struct KOSUZU_RECORD {
	size_t node_count;
	size_t tree_count;
	size_t data_offset;

	size_t fldr_current;

	KOSUZU_NODE *data_nodes;
	uint16_t *data_trees;
	FILE *file_ptr;
	int did_openFile;

	KOSUZU_FILE file_list[KOSUZU_FILE_MAXOPEN];
} KOSUZU_RECORD;
typedef struct KOSUZU_NODECHECK_ENTRY {
	const char *name;
	size_t type;
} KOSUZU_NODECHECK_ENTRY;

/* functions ----------------------------------------------------------------*/

// savqueue.c
int kosuzu_savequeue_setup(KOSUZU_SAVEQUEUE *queue,KOSUZU_SAVEENTRY entries[],const size_t entry_max);
int kosuzu_savequeue_addEntry(KOSUZU_SAVEQUEUE *queue,const KOSUZU_SAVEENTRY *entry);
int kosuzu_savequeue_addFolder(KOSUZU_SAVEQUEUE *queue,const char *arc_path,const char *name);
int kosuzu_savequeue_addUdata(KOSUZU_SAVEQUEUE *queue,const char *arc_path,const char *name,const void *data,size_t data_size);
int kosuzu_savequeue_addFile(KOSUZU_SAVEQUEUE *queue,const char *arc_path,const char *name,const char *filename);
int kosuzu_savequeue_addSint(KOSUZU_SAVEQUEUE *queue,const char *arc_path,const char *name,const int32_t num);
int kosuzu_savequeue_addUint(KOSUZU_SAVEQUEUE *queue,const char *arc_path,const char *name,const uint32_t num);
int kosuzu_savequeue_saveFile(KOSUZU_SAVEQUEUE *queue,const char *filename);

// save.cpp
int kosuzu_save(void **out_buffer, const KOSUZU_SAVEENTRY entries[],const size_t entry_count);
int kosuzu_saveFile(const char *out_filename,const KOSUZU_SAVEENTRY entries[],const size_t entry_count);
int kosuzu_saveEX(const KOSUZU_SAVEINFO *save_info);

// record.c
int kosuzu_recordOpen(KOSUZU_RECORD *archive,FILE *file_ptr);
int kosuzu_recordOpenFile(KOSUZU_RECORD *archive,const char *src_filename);
int kosuzu_recordClose(KOSUZU_RECORD *archive);

int kosuzu_recordChdir(KOSUZU_RECORD *archive,const char *dir_name);
const KOSUZU_NODE *kosuzu_recordNodeFind(KOSUZU_RECORD *archive,const char *name);
const KOSUZU_NODE *kosuzu_recordNodeGet(KOSUZU_RECORD *archive,size_t index);
const KOSUZU_NODE *kosuzu_recordNodeGetCurFldr(KOSUZU_RECORD *archive);
KOSUZU_FILE *kosuzu_record_fileOpen(KOSUZU_RECORD *archive,const char *name);
int kosuzu_record_nodeCheck(KOSUZU_RECORD *record,KOSUZU_NODECHECK_ENTRY name_list[]);

// file.c
int kosuzu_file_close(KOSUZU_FILE *file);
int kosuzu_file_seek(KOSUZU_FILE *file,long int offset,int whence);
size_t kosuzu_file_read(KOSUZU_FILE *file,void *output,size_t size);
int kosuzu_file_eof(KOSUZU_FILE *file);

// misc.c
uint32_t kosuzu_hashString(const char *str);

#ifdef __cplusplus
}
#endif

/* C++ portion --------------------------------------------------------------*/
#ifdef __cplusplus
struct CKosuzuNode : public KOSUZU_NODE {
	public:
		constexpr auto read_int() const -> int { return d.value_sint; }
		constexpr auto read_uint() const -> uint32_t { return d.value_uint; }
};
class CKosuzuRecord : public KOSUZU_RECORD {
	public:
		int close() {
			return kosuzu_recordClose(this); 
		}
		int open_file(const char *filename) {
			return kosuzu_recordOpenFile(this,filename);
		}

		int chdir(const char *dir_name) { 
			return kosuzu_recordChdir(this,dir_name);
		}
		auto node_find(const char *name) -> const CKosuzuNode* {
			return (CKosuzuNode*)kosuzu_recordNodeFind(this,name);
		}
		auto node_check(KOSUZU_NODECHECK_ENTRY name_list[]) -> int {
			return kosuzu_record_nodeCheck(this,name_list);
		}
		KOSUZU_FILE *file_open(const char *name) { 
			return kosuzu_record_fileOpen(this,name); 
		}
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
};

#endif

#endif

