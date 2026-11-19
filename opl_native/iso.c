#include "iso.h"
#include <string.h>
#include <strings.h>
static iso_reader read_sector;
static int joliet;
static unsigned char buf[2048] __attribute__((aligned(64)));
static uint32_t le(const unsigned char *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static void record(const unsigned char *p,IsoFile *f){f->lba=le(p+2);f->size=le(p+10);f->flags=p[25];f->pos=0;}
int iso_mount(iso_reader r,IsoFile *root){unsigned i; read_sector=r;joliet=0;for(i=16;i<80;i++){if(!r(i,buf)||memcmp(buf+1,"CD001",5))return 0;if(buf[0]==255)return 0;if(buf[0]==1){if(buf[156]<34||buf[128]!=0||buf[129]!=8)return 0;record(buf+156,root);return !!(root->flags&2);}}return 0;}
int iso_next(IsoFile *d,IsoFile *e,char name[256]){while(d->pos<d->size){unsigned off=d->pos%2048,n,k;unsigned char *p;if(!read_sector(d->lba+d->pos/2048,buf))return -1;p=buf+off;n=*p;if(!n){unsigned step=2048-off;d->pos+=step>d->size-d->pos?d->size-d->pos:step;continue;}if(n<34||n>2048-off||n>d->size-d->pos)return -1;d->pos+=n;k=p[32];if(!k||k>n-33)return -1;if(k==1&&p[33]<2)continue;if(p[25]&128)continue;if(joliet){unsigned j=0,i;for(i=0;i+1<k && j<253;i+=2){unsigned c=(p[33+i]<<8)|p[34+i];if(c<128)name[j++]=c;else if(c<2048){name[j++]=0xc0|(c>>6);name[j++]=0x80|(c&63);}else{name[j++]=0xe0|(c>>12);name[j++]=0x80|((c>>6)&63);name[j++]=0x80|(c&63);}}name[j]=0;}else{memcpy(name,p+33,k);name[k]=0;}{char *s=strchr(name,';');if(s)*s=0;}record(p,e);return 1;}return 0;}
int iso_open(IsoFile dir,const char *name,IsoFile *out){char n[256];int r;dir.pos=0;while((r=iso_next(&dir,out,n))>0)if(!strcasecmp(n,name))return 1;return r;}
int iso_seek(IsoFile *f,uint32_t p){if(p>f->size)return 0;f->pos=p;return 1;}
int iso_read(IsoFile *f,void *dest,unsigned size){unsigned done=0;unsigned char *p=dest;if(f->pos>f->size)return -1;if(size>f->size-f->pos)size=f->size-f->pos;while(done<size){unsigned off=f->pos%2048,n=2048-off;if(n>size-done)n=size-done;if(!read_sector(f->lba+f->pos/2048,buf))return done?(int)done:-1;memcpy(p+done,buf+off,n);done+=n;f->pos+=n;}return done;}

int iso_mount_joliet(IsoFile *root){unsigned i;for(i=16;i<80;i++){if(!read_sector(i,buf)||memcmp(buf+1,"CD001",5)||buf[0]==255)return 0;if(buf[0]==2&&buf[88]==0x25&&buf[89]==0x2f&&(buf[90]==0x40||buf[90]==0x43||buf[90]==0x45)&&buf[156]>=34&&buf[128]==0&&buf[129]==8){record(buf+156,root);joliet=1;return !!(root->flags&2);}}return 0;}
