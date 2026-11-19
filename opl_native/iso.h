#ifndef DG_ISO_H
#define DG_ISO_H
#include <stdint.h>
typedef int (*iso_reader)(uint32_t, void *);
typedef struct { uint32_t lba,size,pos; unsigned flags; } IsoFile;
int iso_mount(iso_reader reader, IsoFile *root);
int iso_next(IsoFile *dir, IsoFile *entry, char name[256]);
int iso_open(IsoFile dir,const char *name,IsoFile *out);
int iso_read(IsoFile *file,void *dest,unsigned size);
int iso_seek(IsoFile *file,uint32_t pos);
#endif
