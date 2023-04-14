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

typedef KOSUZU_FILENODE KOSUZU_NODE;
typedef struct KOSUZU_ARCHIVE {
	size_t node_count;
	size_t tree_count;
	size_t data_offset;

	size_t fldr_current;

	KOSUZU_NODE *data_nodes;
	uint16_t *data_trees;
	FILE *file_ptr;
	int did_openFile;
} KOSUZU_ARCHIVE;

/* functions ----------------------------------------------------------------*/
int kosuzu_save(void **out_buffer, const KOSUZU_SAVEENTRY entries[],const size_t entry_count);
int kosuzu_saveFile(const char *out_filename,const KOSUZU_SAVEENTRY entries[],const size_t entry_count);
int kosuzu_saveEX(const KOSUZU_SAVEINFO *save_info);

int kosuzu_archiveOpen(KOSUZU_ARCHIVE *archive,FILE *file_ptr);
int kosuzu_archiveOpenFile(KOSUZU_ARCHIVE *archive,const char *src_filename);
int kosuzu_archiveClose(KOSUZU_ARCHIVE *archive);

int kosuzu_archiveChdir(KOSUZU_ARCHIVE *archive,const char *dir_name);
int kosuzu_archiveNodeFindIdx(KOSUZU_ARCHIVE *archive,const char *name);
const KOSUZU_NODE *kosuzu_archiveNodeFind(KOSUZU_ARCHIVE *archive,const char *name);
const KOSUZU_NODE *kosuzu_archiveNodeGet(KOSUZU_ARCHIVE *archive,size_t index);
const KOSUZU_NODE *kosuzu_archiveNodeGetCurFldr(KOSUZU_ARCHIVE *archive);
const KOSUZU_NODE *kosuzu_archiveFileSeek(KOSUZU_ARCHIVE *archive,const char *name);

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

#endif

#endif

