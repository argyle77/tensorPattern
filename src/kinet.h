#ifndef KINET_H
#define KINET_H

#define KINET_UDP_PORT 6038

#define KINET_VERSION 0x0001

// KiNET magic numbers
#define KINET_MAGIC                 0x4adc0104
#define KINET_MORE_MAGIC          0xdeadbeef
#define KINET_DEEP_MAGIC          0xc001d00d
#define KINET_MAGIC_HASH          0x69000420

#define KTYPE_DMXOUT                  0x0101
#define KTYPE_SET 0x0103
#define KSET_ACKS 1

#define KTYPE_KDM_SER9OUT 0xfb02
#define KTYPE_KDM_TICKER 0xfb03
#define KTYPE_KDM_DMXSTROBE 0xfb04

typedef struct {
  unsigned long magic;
  unsigned short ver;
  unsigned short type;
  unsigned long seq;
} KiNET_Hdr;

typedef struct {
  KiNET_Hdr hdr;
  unsigned char port;
  unsigned char flags;
  unsigned short timerVal;
  unsigned long uni;
} KiNET_DMXout;

typedef struct {
  KiNET_Hdr hdr;
  unsigned short var;
  unsigned short pad;
  unsigned long val;
} KiNET_Set;

typedef struct {
  KiNET_Hdr hdr;
  unsigned long status;
} KiNET_Ack;

typedef struct {
  KiNET_Hdr hdr;
  unsigned char port;
  unsigned char flags;
  unsigned short timerVal;
  unsigned long uni;
  // need to add serial data here
} KiNET_KDM_Ser9Out;

typedef struct {
  KiNET_Hdr hdr;
  unsigned short tickval;
  unsigned short pad;
  unsigned long uni;
} KiNET_KDM_Ticker;

typedef struct {
  KiNET_Hdr hdr;
  unsigned char duration;
  unsigned char flags;
  unsigned short interval;
} KiNET_KDM_DMXStrobe;

#endif /* KINET_H */
