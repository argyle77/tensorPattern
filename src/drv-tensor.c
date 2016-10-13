/*

  Foo.

*/

#define _REENTRANT

#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#ifdef __APPLE__
    #include <sys/uio.h>
#else
    #include <sys/io.h>
#endif

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

#include "drv-tensor.h"
#include "tensor.h"

const char *Tensor_Section1_def[] = {
  "10.5.1.15","10.5.1.9","10.5.1.19",
  "10.5.1.20","10.5.1.6","10.5.1.7"
};

const char *Tensor_Section2_def[] = {
  "10.5.1.23","10.5.1.17","10.5.1.18",
  "10.5.1.22","10.5.1.16","10.5.1.13"
};

const char *Tensor_Section3_def[] = {
  "10.5.1.25","10.5.1.1","10.5.1.21",
  "10.5.1.24","10.5.1.14","10.5.1.3"
};


const char *Tensor_IPs_1[9];
const char *Tensor_IPs_2[9];

int tensor_sections_order[3] = {1,2,3};

int tensor_sock;

/*--------------------------------------------------------------------------*/

static int tensor_init_ips(const char **Tensor_Section1, const char **Tensor_Section2, const char **Tensor_Section3)
{
  int i;

  for(i=0;i<3;i++) {
    int section;
    const char **ssrc;

    section = tensor_sections_order[i];
    if((section < 1) ||
       (section > 3))
      section = i+1;

    switch(section) {
    case 1:
      ssrc = Tensor_Section1;
      break;
    case 2:
      ssrc = Tensor_Section2;
      break;

    default:
    case 3:
      ssrc = Tensor_Section3;
      break;
    }

    Tensor_IPs_1[i*3 + 0] = ssrc[0];
    Tensor_IPs_1[i*3 + 1] = ssrc[1];
    Tensor_IPs_1[i*3 + 2] = ssrc[2];
    Tensor_IPs_2[i*3 + 0] = ssrc[3];
    Tensor_IPs_2[i*3 + 1] = ssrc[4];
    Tensor_IPs_2[i*3 + 2] = ssrc[5];
  }

  return(0);
}

int tensor_init(const char **Tensor_Section1, const char **Tensor_Section2, const char **Tensor_Section3) {

  int i;

  tensor_init_ips(Tensor_Section1, Tensor_Section2, Tensor_Section3);

  if ((tensor_sock=socket(PF_INET,SOCK_DGRAM,0))==-1) {
    return -1;
  }
  i=1;
  if (setsockopt(tensor_sock,SOL_SOCKET,SO_BROADCAST,&i,sizeof(i))) {
    close(tensor_sock);
    return -1;
  }

  return 0;

}

/*--------------------------------------------------------------------------*/

int tensor_build_one(unsigned char *buf,int bufmax,unsigned char *fb,int span,
          int xstart,int ystart,int width,int height) {

  int bufpos,x,y;

  if (width*height>=bufmax-22) {
    fprintf(stderr,"tensor_send_one(): region too big\n");
    return 0;
  }

  bufpos=0;
  buf[bufpos++]=0; // dmx start code

  for (y=ystart;y<ystart+height;++y) {
    for (x=xstart;x<xstart+width;++x) {
      buf[bufpos++]=fb[y*span+x];
    }
  }

  return bufpos;

}

/*--------------------------------------------------------------------------*/

#define TENSOR_SEND_BUF_SIZE 2222

int tensor_landscape_p=0;

void tensor_send(unsigned char *fb) {

  // fb is a 3xTENSOR_WIDTHxTENSOR_HEIGHT array of unsigned chars

  int i,j;
  unsigned char buf[TENSOR_SEND_BUF_SIZE+666];
  struct sockaddr_in dest;

  // Landscape xform
  if(tensor_landscape_p) {
    int x,y;
    unsigned char landscape_fb[TENSOR_BYTES];

    for(x=0;x<TENSOR_HEIGHT;x++) {
      for(y=0;y<TENSOR_WIDTH;y++) {
        int r,g,b;
        int tx,ty;

        r = fb[y*TENSOR_HEIGHT*3 + x*3 + 0];
        g = fb[y*TENSOR_HEIGHT*3 + x*3 + 1];
        b = fb[y*TENSOR_HEIGHT*3 + x*3 + 2];

        tx = TENSOR_WIDTH - y - 1;
        ty = x;

        landscape_fb[ty*TENSOR_WIDTH*3 + tx*3 + 0] = r;
        landscape_fb[ty*TENSOR_WIDTH*3 + tx*3 + 1] = g;
        landscape_fb[ty*TENSOR_WIDTH*3 + tx*3 + 2] = b;
      }
    }

    fb = landscape_fb;
  }

  KiNET_DMXout *kk=(KiNET_DMXout *)buf;
  kk->hdr.magic=KINET_MAGIC;
  kk->hdr.ver=KINET_VERSION;
  kk->hdr.type=KTYPE_DMXOUT;
  kk->hdr.seq=0;
  kk->port=0;
  kk->timerVal=0;
  kk->uni=0;
  kk->flags=0x01; // send acks, please
  dest.sin_family=AF_INET;
  dest.sin_port=htons(KINET_UDP_PORT);

  for (j=0;j<9;++j) {

    i=tensor_build_one(buf+sizeof(KiNET_DMXout),TENSOR_SEND_BUF_SIZE,
           fb,TENSOR_WIDTH*3,j*3*3,0,3*3,10);
    dest.sin_addr.s_addr=inet_addr(Tensor_IPs_1[j]);
    if (dest.sin_addr.s_addr) {
      sendto(tensor_sock,buf,sizeof(KiNET_DMXout)+i,0,
        (struct sockaddr *)&dest,sizeof(dest));
    }
    i=tensor_build_one(buf+sizeof(KiNET_DMXout),TENSOR_SEND_BUF_SIZE,
           fb,TENSOR_WIDTH*3,j*3*3,10,3*3,10);
    dest.sin_addr.s_addr=inet_addr(Tensor_IPs_2[j]);
    if (dest.sin_addr.s_addr)
      sendto(tensor_sock,buf,sizeof(KiNET_DMXout)+i,0,
       (struct sockaddr *)&dest,sizeof(dest));

  }

}

