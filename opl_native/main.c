#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <libcdvd.h>
#include <debug.h>
#include <string.h>
#include <strings.h>
#include "iso.h"
static int sector(uint32_t lba,void *p){sceCdRMode m={0};m.trycount=3;m.spindlctrl=1;m.datapattern=0;if(!sceCdRead(lba,1,p,&m))return 0;sceCdSync(0);return sceCdGetError()==0;}
int main(void){IsoFile root,dir,f;char name[256];unsigned char h[12],block[4096];int r,n=0;init_scr();scr_printf("DG MOVIE V0.1.1 - DIAGNOSTICO\nSem reset IOP / sem drivers de armazenamento\n");SifInitRpc(0);scr_printf("RPC OK\n");sceCdInit(SCECdINIT);scr_printf("CD INIT retornou\n");if(!iso_mount(sector,&root)){scr_printf("FALHA PVD ISO9660\n");goto end;}scr_printf("PVD OK\n");if(iso_open(root,"MOVIES",&dir)!=1||!(dir.flags&2)){scr_printf("FALHA pasta MOVIES\n");goto end;}scr_printf("MOVIES OK\n");while((r=iso_next(&dir,&f,name))>0){char *ext=strrchr(name,'.');if(f.flags&2||!ext||(strcasecmp(ext,".AVI")&&strcasecmp(ext,".MP4")))continue;scr_printf("%s LBA:%u bytes:%u\n",name,f.lba,f.size);if(iso_read(&f,h,12)!=12){scr_printf("FALHA leitura cabecalho\n");continue;}scr_printf("Header: %s\n",!memcmp(h,"RIFF",4)&&!memcmp(h+8,"AVI ",4)?"AVI RIFF":!memcmp(h+4,"ftyp",4)?"MP4 FTYP":"OUTRO");if(f.size>=sizeof(block)&&iso_seek(&f,f.size/2)&&iso_read(&f,block,sizeof(block))==(int)sizeof(block))scr_printf("Seek meio + 4096 bytes OK\n");else scr_printf("Seek/leitura FALHOU\n");if(++n>=4)break;}scr_printf("Fim: %d filmes, estado %d\n",n,r);end:scr_printf("Fotografe esta tela. Sem reproducao nesta build.\n");SleepThread();return 0;}
