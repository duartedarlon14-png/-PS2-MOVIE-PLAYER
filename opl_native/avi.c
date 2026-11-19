#include "avi.h"
#include <string.h>

#define TAG(a,b,c,d) ((unsigned)(a)|((unsigned)(b)<<8)|((unsigned)(c)<<16)|((unsigned)(d)<<24))

static unsigned u(const unsigned char *p) {
    return p[0]|(unsigned)p[1]<<8|(unsigned)p[2]<<16|(unsigned)p[3]<<24;
}

static unsigned w(const unsigned char *p) {
    return p[0]|(unsigned)p[1]<<8;
}

static int get(IsoFile *f,unsigned p,void *b,unsigned n) {
    return iso_seek(f,p)&&iso_read(f,b,n)==(int)n;
}

static int walk(IsoFile *f,AviInfo *a,unsigned p,unsigned end,int depth,int movi,unsigned *stream,unsigned *kind) {
    unsigned char b[64];

    if(depth>8)return 0;

    while(end-p>=8) {
        unsigned t,n,q,next;

        if(!get(f,p,b,8))return 0;

        t=u(b);
        n=u(b+4);
        q=p+8;

        if(n>end-q)return 0;

        next=q+n;

        if(n&1) {
            if(next==end)return 0;
            next++;
        }

        if(t==TAG('L','I','S','T')) {
            unsigned l,k=0;

            if(n<4||!get(f,q,b,4))return 0;

            l=u(b);

            if(l==TAG('s','t','r','l')) {
                unsigned id=(*stream)++;
                if(!walk(f,a,q+4,q+n,depth+1,0,&id,&k))return 0;
            }
            else if(l==TAG('h','d','r','l')||
                    l==TAG('m','o','v','i')||
                    l==TAG('r','e','c',' ')) {
                if(!walk(f,a,q+4,q+n,depth+1,
                    movi||l==TAG('m','o','v','i'),
                    stream,kind))return 0;
            }
        }
        else if(!movi&&t==TAG('s','t','r','h')&&n>=36) {
            if(!get(f,q,b,36))return 0;

            *kind=u(b);

            if(*kind==TAG('v','i','d','s')&&a->video==~0u) {
                a->video=*stream;
                a->codec=u(b+4);
                a->scale=u(b+20);
                a->rate=u(b+24);
            }

            if(*kind==TAG('a','u','d','s')&&a->audio==~0u)
                a->audio=*stream;
        }
        else if(!movi&&t==TAG('s','t','r','f')) {
            if(*kind==TAG('v','i','d','s')&&*stream==a->video&&n>=20) {
                if(!get(f,q,b,20))return 0;

                a->width=u(b+4);
                a->height=u(b+8);
                a->codec=u(b+16);
            }

            if(*kind==TAG('a','u','d','s')&&*stream==a->audio&&n>=8) {
                if(!get(f,q,b,8))return 0;

                a->format=w(b);
                a->channels=w(b+2);
                a->hz=u(b+4);
            }
        }
        else if(movi) {
            unsigned x=t&255;
            unsigned y=(t>>8)&255;
            unsigned id,suffix=t>>16;

            if(x>='0'&&x<='9'&&y>='0'&&y<='9') {
                id=(x-'0')*10+y-'0';

                if(id==a->video&&
                    (suffix==('d'|('c'<<8))||
                     suffix==('d'|('b'<<8)))) {

                    if(n&&a->frames<3&&
                        !get(f,q,b,n<sizeof b?n:sizeof b))
                        return 0;

                    if(n&&!a->first_size) {
                        a->first_pos=q;
                        a->first_size=n;
                    }

                    a->frames++;
                    a->video_bytes+=n;
                }

                if(id==a->audio&&suffix==('w'|('b'<<8))) {

                    if(n&&a->audio_blocks<3&&
                        !get(f,q,b,n<sizeof b?n:sizeof b))
                        return 0;

                    a->audio_blocks++;
                    a->audio_bytes+=n;
                }
            }

            if(a->frames>=30&&
                (a->audio==~0u||a->audio_blocks>=10))
                return 2;
        }

        p=next;
    }

    return p==end;
}

int avi_scan(IsoFile f,AviInfo *a) {
    unsigned char b[12];
    unsigned n,s=0,k=0;

    memset(a,0,sizeof *a);

    a->video=a->audio=~0u;

    if(!get(&f,0,b,12)||
        u(b)!=TAG('R','I','F','F')||
        u(b+8)!=TAG('A','V','I',' '))
        return 0;

    n=u(b+4);

    if(n<4||n>f.size-8)return 0;

    return walk(&f,a,12,n+8,0,0,&s,&k)>0&&a->frames>0;
}
