## building
lua not required but i used it for building

	lua build.lua build_lib build_tests

builds library and test program. omit any of the 2 last keywords if you want
to build only the test or the library.

## usage
```c++
KOSUZU_ARCHIVE archive = {};
if(kosuzu_archiveOpenFile(&archive,"character.ksz")) {
	kosuzu_archiveChdir(&archive,"cg");
	const KOSUZU_FILENODE *node = kosuzu_archiveFileSeek(&archive,"walk.bmp");
	if(node) {
		FILE *out_file = fopen("output.bmp","wb");
		char file_buffer;
		for(size_t i=0; i<node->d.udata_size; i++) {
			fread(&file_buffer,1,sizeof(char),archive.file_ptr);
			fwrite(&file_buffer,1,sizeof(char),out_file);
		}
		fclose(out_file);
	}

	kosuzu_archiveClose(&archive);
}
```

reads a file 'cg\walk.bmp' from the archive 'character.ksz', and then writes
it back to a file on disk 'output.bmp'.

```c++
KOSUZU_ARCHIVE archive = {};
if(kosuzu_archiveOpenFile(&archive,"map.ksz")) {
	const int idx_width = kosuzu_archiveNodeFind(&archive,"width");
	const int idx_height = kosuzu_archiveNodeFind(&archive,"height");

	// check if nodes were actually found
	if((idx_width != -1) && (idx_height != -1)) {
		const KOSUZU_FILENODE *width = kosuzu_archiveNodeGet(&archive,idx_width);
		const KOSUZU_FILENODE *height = kosuzu_archiveNodeGet(&archive,idx_height);
		
		printf("width: %d\n",width->d.value_int);
		printf("height: %d\n",height->d.value_int);
	}

	kosuzu_archiveClose(&archive);
}
```

