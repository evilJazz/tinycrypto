/*
 ---------------------------------------------------------------------------
 Copyright (c) 1998-2008, Brian Gladman, Worcester, UK. All rights reserved.
 Copyright (c) 2013, Igor Saric. All rights reserved.
 Copyright (c) 2014-2017, Andre Beckedorf, Oldenburg, Germany. All rights reserved.

 LICENSE TERMS

 The redistribution and use of this software (with or without changes)
 is allowed without the payment of fees or royalties provided that:

  1. source code distributions include the above copyright notice, this
     list of conditions and the following disclaimer;

  2. binary distributions include the above copyright notice, this list
     of conditions and the following disclaimer in their documentation;

  3. the name of the copyright holder is not used to endorse products
     built using this software without specific written permission.

 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness
 and/or fitness for purpose.
 ---------------------------------------------------------------------------
*/

#include "tinyaes.h"

#include <stdlib.h>

#include <QUuid>
#include <QIODevice>

#define N_ROW                   4
#define N_COL                   4
#define N_BLOCK   (N_ROW * N_COL)
#define N_MAX_ROUNDS           14

#define WPOLY   0x011b
#define BPOLY     0x1b
#define DPOLY   0x008d

#define f1(x)   (x)
#define f2(x)   ((x << 1) ^ (((x >> 7) & 1) * WPOLY))
#define f4(x)   ((x << 2) ^ (((x >> 6) & 1) * WPOLY) ^ (((x >> 6) & 2) * WPOLY))
#define f8(x)   ((x << 3) ^ (((x >> 5) & 1) * WPOLY) ^ (((x >> 5) & 2) * WPOLY) \
                          ^ (((x >> 5) & 4) * WPOLY))
#define d2(x)   (((x) >> 1) ^ ((x) & 1 ? DPOLY : 0))

#define f3(x)   (f2(x) ^ x)
#define f9(x)   (f8(x) ^ x)
#define fb(x)   (f8(x) ^ f2(x) ^ x)
#define fd(x)   (f8(x) ^ f4(x) ^ x)
#define fe(x)   (f8(x) ^ f4(x) ^ f2(x))

#define s_box(x)     sbox[(x)]
#define is_box(x)    isbox[(x)]
#define gfm2_sb(x)   gfm2_sbox[(x)]
#define gfm3_sb(x)   gfm3_sbox[(x)]
#define gfm_9(x)     gfmul_9[(x)]
#define gfm_b(x)     gfmul_b[(x)]
#define gfm_d(x)     gfmul_d[(x)]
#define gfm_e(x)     gfmul_e[(x)]

#define block_copy_nn(d, s, l)    memcpy(d, s, l)
#define block_copy(d, s)          memcpy(d, s, N_BLOCK)

#define sb_data(w) {    /* S Box data values */                            \
    w(0x63), w(0x7c), w(0x77), w(0x7b), w(0xf2), w(0x6b), w(0x6f), w(0xc5),\
    w(0x30), w(0x01), w(0x67), w(0x2b), w(0xfe), w(0xd7), w(0xab), w(0x76),\
    w(0xca), w(0x82), w(0xc9), w(0x7d), w(0xfa), w(0x59), w(0x47), w(0xf0),\
    w(0xad), w(0xd4), w(0xa2), w(0xaf), w(0x9c), w(0xa4), w(0x72), w(0xc0),\
    w(0xb7), w(0xfd), w(0x93), w(0x26), w(0x36), w(0x3f), w(0xf7), w(0xcc),\
    w(0x34), w(0xa5), w(0xe5), w(0xf1), w(0x71), w(0xd8), w(0x31), w(0x15),\
    w(0x04), w(0xc7), w(0x23), w(0xc3), w(0x18), w(0x96), w(0x05), w(0x9a),\
    w(0x07), w(0x12), w(0x80), w(0xe2), w(0xeb), w(0x27), w(0xb2), w(0x75),\
    w(0x09), w(0x83), w(0x2c), w(0x1a), w(0x1b), w(0x6e), w(0x5a), w(0xa0),\
    w(0x52), w(0x3b), w(0xd6), w(0xb3), w(0x29), w(0xe3), w(0x2f), w(0x84),\
    w(0x53), w(0xd1), w(0x00), w(0xed), w(0x20), w(0xfc), w(0xb1), w(0x5b),\
    w(0x6a), w(0xcb), w(0xbe), w(0x39), w(0x4a), w(0x4c), w(0x58), w(0xcf),\
    w(0xd0), w(0xef), w(0xaa), w(0xfb), w(0x43), w(0x4d), w(0x33), w(0x85),\
    w(0x45), w(0xf9), w(0x02), w(0x7f), w(0x50), w(0x3c), w(0x9f), w(0xa8),\
    w(0x51), w(0xa3), w(0x40), w(0x8f), w(0x92), w(0x9d), w(0x38), w(0xf5),\
    w(0xbc), w(0xb6), w(0xda), w(0x21), w(0x10), w(0xff), w(0xf3), w(0xd2),\
    w(0xcd), w(0x0c), w(0x13), w(0xec), w(0x5f), w(0x97), w(0x44), w(0x17),\
    w(0xc4), w(0xa7), w(0x7e), w(0x3d), w(0x64), w(0x5d), w(0x19), w(0x73),\
    w(0x60), w(0x81), w(0x4f), w(0xdc), w(0x22), w(0x2a), w(0x90), w(0x88),\
    w(0x46), w(0xee), w(0xb8), w(0x14), w(0xde), w(0x5e), w(0x0b), w(0xdb),\
    w(0xe0), w(0x32), w(0x3a), w(0x0a), w(0x49), w(0x06), w(0x24), w(0x5c),\
    w(0xc2), w(0xd3), w(0xac), w(0x62), w(0x91), w(0x95), w(0xe4), w(0x79),\
    w(0xe7), w(0xc8), w(0x37), w(0x6d), w(0x8d), w(0xd5), w(0x4e), w(0xa9),\
    w(0x6c), w(0x56), w(0xf4), w(0xea), w(0x65), w(0x7a), w(0xae), w(0x08),\
    w(0xba), w(0x78), w(0x25), w(0x2e), w(0x1c), w(0xa6), w(0xb4), w(0xc6),\
    w(0xe8), w(0xdd), w(0x74), w(0x1f), w(0x4b), w(0xbd), w(0x8b), w(0x8a),\
    w(0x70), w(0x3e), w(0xb5), w(0x66), w(0x48), w(0x03), w(0xf6), w(0x0e),\
    w(0x61), w(0x35), w(0x57), w(0xb9), w(0x86), w(0xc1), w(0x1d), w(0x9e),\
    w(0xe1), w(0xf8), w(0x98), w(0x11), w(0x69), w(0xd9), w(0x8e), w(0x94),\
    w(0x9b), w(0x1e), w(0x87), w(0xe9), w(0xce), w(0x55), w(0x28), w(0xdf),\
    w(0x8c), w(0xa1), w(0x89), w(0x0d), w(0xbf), w(0xe6), w(0x42), w(0x68),\
    w(0x41), w(0x99), w(0x2d), w(0x0f), w(0xb0), w(0x54), w(0xbb), w(0x16) }

#define isb_data(w) {   /* inverse S Box data values */                    \
    w(0x52), w(0x09), w(0x6a), w(0xd5), w(0x30), w(0x36), w(0xa5), w(0x38),\
    w(0xbf), w(0x40), w(0xa3), w(0x9e), w(0x81), w(0xf3), w(0xd7), w(0xfb),\
    w(0x7c), w(0xe3), w(0x39), w(0x82), w(0x9b), w(0x2f), w(0xff), w(0x87),\
    w(0x34), w(0x8e), w(0x43), w(0x44), w(0xc4), w(0xde), w(0xe9), w(0xcb),\
    w(0x54), w(0x7b), w(0x94), w(0x32), w(0xa6), w(0xc2), w(0x23), w(0x3d),\
    w(0xee), w(0x4c), w(0x95), w(0x0b), w(0x42), w(0xfa), w(0xc3), w(0x4e),\
    w(0x08), w(0x2e), w(0xa1), w(0x66), w(0x28), w(0xd9), w(0x24), w(0xb2),\
    w(0x76), w(0x5b), w(0xa2), w(0x49), w(0x6d), w(0x8b), w(0xd1), w(0x25),\
    w(0x72), w(0xf8), w(0xf6), w(0x64), w(0x86), w(0x68), w(0x98), w(0x16),\
    w(0xd4), w(0xa4), w(0x5c), w(0xcc), w(0x5d), w(0x65), w(0xb6), w(0x92),\
    w(0x6c), w(0x70), w(0x48), w(0x50), w(0xfd), w(0xed), w(0xb9), w(0xda),\
    w(0x5e), w(0x15), w(0x46), w(0x57), w(0xa7), w(0x8d), w(0x9d), w(0x84),\
    w(0x90), w(0xd8), w(0xab), w(0x00), w(0x8c), w(0xbc), w(0xd3), w(0x0a),\
    w(0xf7), w(0xe4), w(0x58), w(0x05), w(0xb8), w(0xb3), w(0x45), w(0x06),\
    w(0xd0), w(0x2c), w(0x1e), w(0x8f), w(0xca), w(0x3f), w(0x0f), w(0x02),\
    w(0xc1), w(0xaf), w(0xbd), w(0x03), w(0x01), w(0x13), w(0x8a), w(0x6b),\
    w(0x3a), w(0x91), w(0x11), w(0x41), w(0x4f), w(0x67), w(0xdc), w(0xea),\
    w(0x97), w(0xf2), w(0xcf), w(0xce), w(0xf0), w(0xb4), w(0xe6), w(0x73),\
    w(0x96), w(0xac), w(0x74), w(0x22), w(0xe7), w(0xad), w(0x35), w(0x85),\
    w(0xe2), w(0xf9), w(0x37), w(0xe8), w(0x1c), w(0x75), w(0xdf), w(0x6e),\
    w(0x47), w(0xf1), w(0x1a), w(0x71), w(0x1d), w(0x29), w(0xc5), w(0x89),\
    w(0x6f), w(0xb7), w(0x62), w(0x0e), w(0xaa), w(0x18), w(0xbe), w(0x1b),\
    w(0xfc), w(0x56), w(0x3e), w(0x4b), w(0xc6), w(0xd2), w(0x79), w(0x20),\
    w(0x9a), w(0xdb), w(0xc0), w(0xfe), w(0x78), w(0xcd), w(0x5a), w(0xf4),\
    w(0x1f), w(0xdd), w(0xa8), w(0x33), w(0x88), w(0x07), w(0xc7), w(0x31),\
    w(0xb1), w(0x12), w(0x10), w(0x59), w(0x27), w(0x80), w(0xec), w(0x5f),\
    w(0x60), w(0x51), w(0x7f), w(0xa9), w(0x19), w(0xb5), w(0x4a), w(0x0d),\
    w(0x2d), w(0xe5), w(0x7a), w(0x9f), w(0x93), w(0xc9), w(0x9c), w(0xef),\
    w(0xa0), w(0xe0), w(0x3b), w(0x4d), w(0xae), w(0x2a), w(0xf5), w(0xb0),\
    w(0xc8), w(0xeb), w(0xbb), w(0x3c), w(0x83), w(0x53), w(0x99), w(0x61),\
    w(0x17), w(0x2b), w(0x04), w(0x7e), w(0xba), w(0x77), w(0xd6), w(0x26),\
    w(0xe1), w(0x69), w(0x14), w(0x63), w(0x55), w(0x21), w(0x0c), w(0x7d) }

#define mm_data(w) {    /* basic data for forming finite field tables */   \
    w(0x00), w(0x01), w(0x02), w(0x03), w(0x04), w(0x05), w(0x06), w(0x07),\
    w(0x08), w(0x09), w(0x0a), w(0x0b), w(0x0c), w(0x0d), w(0x0e), w(0x0f),\
    w(0x10), w(0x11), w(0x12), w(0x13), w(0x14), w(0x15), w(0x16), w(0x17),\
    w(0x18), w(0x19), w(0x1a), w(0x1b), w(0x1c), w(0x1d), w(0x1e), w(0x1f),\
    w(0x20), w(0x21), w(0x22), w(0x23), w(0x24), w(0x25), w(0x26), w(0x27),\
    w(0x28), w(0x29), w(0x2a), w(0x2b), w(0x2c), w(0x2d), w(0x2e), w(0x2f),\
    w(0x30), w(0x31), w(0x32), w(0x33), w(0x34), w(0x35), w(0x36), w(0x37),\
    w(0x38), w(0x39), w(0x3a), w(0x3b), w(0x3c), w(0x3d), w(0x3e), w(0x3f),\
    w(0x40), w(0x41), w(0x42), w(0x43), w(0x44), w(0x45), w(0x46), w(0x47),\
    w(0x48), w(0x49), w(0x4a), w(0x4b), w(0x4c), w(0x4d), w(0x4e), w(0x4f),\
    w(0x50), w(0x51), w(0x52), w(0x53), w(0x54), w(0x55), w(0x56), w(0x57),\
    w(0x58), w(0x59), w(0x5a), w(0x5b), w(0x5c), w(0x5d), w(0x5e), w(0x5f),\
    w(0x60), w(0x61), w(0x62), w(0x63), w(0x64), w(0x65), w(0x66), w(0x67),\
    w(0x68), w(0x69), w(0x6a), w(0x6b), w(0x6c), w(0x6d), w(0x6e), w(0x6f),\
    w(0x70), w(0x71), w(0x72), w(0x73), w(0x74), w(0x75), w(0x76), w(0x77),\
    w(0x78), w(0x79), w(0x7a), w(0x7b), w(0x7c), w(0x7d), w(0x7e), w(0x7f),\
    w(0x80), w(0x81), w(0x82), w(0x83), w(0x84), w(0x85), w(0x86), w(0x87),\
    w(0x88), w(0x89), w(0x8a), w(0x8b), w(0x8c), w(0x8d), w(0x8e), w(0x8f),\
    w(0x90), w(0x91), w(0x92), w(0x93), w(0x94), w(0x95), w(0x96), w(0x97),\
    w(0x98), w(0x99), w(0x9a), w(0x9b), w(0x9c), w(0x9d), w(0x9e), w(0x9f),\
    w(0xa0), w(0xa1), w(0xa2), w(0xa3), w(0xa4), w(0xa5), w(0xa6), w(0xa7),\
    w(0xa8), w(0xa9), w(0xaa), w(0xab), w(0xac), w(0xad), w(0xae), w(0xaf),\
    w(0xb0), w(0xb1), w(0xb2), w(0xb3), w(0xb4), w(0xb5), w(0xb6), w(0xb7),\
    w(0xb8), w(0xb9), w(0xba), w(0xbb), w(0xbc), w(0xbd), w(0xbe), w(0xbf),\
    w(0xc0), w(0xc1), w(0xc2), w(0xc3), w(0xc4), w(0xc5), w(0xc6), w(0xc7),\
    w(0xc8), w(0xc9), w(0xca), w(0xcb), w(0xcc), w(0xcd), w(0xce), w(0xcf),\
    w(0xd0), w(0xd1), w(0xd2), w(0xd3), w(0xd4), w(0xd5), w(0xd6), w(0xd7),\
    w(0xd8), w(0xd9), w(0xda), w(0xdb), w(0xdc), w(0xdd), w(0xde), w(0xdf),\
    w(0xe0), w(0xe1), w(0xe2), w(0xe3), w(0xe4), w(0xe5), w(0xe6), w(0xe7),\
    w(0xe8), w(0xe9), w(0xea), w(0xeb), w(0xec), w(0xed), w(0xee), w(0xef),\
    w(0xf0), w(0xf1), w(0xf2), w(0xf3), w(0xf4), w(0xf5), w(0xf6), w(0xf7),\
    w(0xf8), w(0xf9), w(0xfa), w(0xfb), w(0xfc), w(0xfd), w(0xfe), w(0xff) }

typedef quint8  aes_result;

static const quint8  sbox[256]  =  sb_data(f1);
static const quint8  isbox[256] = isb_data(f1);

static const quint8  gfm2_sbox[256] = sb_data(f2);
static const quint8  gfm3_sbox[256] = sb_data(f3);

static const quint8  gfmul_9[256] = mm_data(f9);
static const quint8  gfmul_b[256] = mm_data(fb);
static const quint8  gfmul_d[256] = mm_data(fd);
static const quint8  gfmul_e[256] = mm_data(fe);

typedef struct
{
    quint8  ksch[(N_MAX_ROUNDS + 1) * N_BLOCK];
    quint8  rnd;
} aes_context;

// algorithm
void xor_block( void *d, const void *s )
{
    ((quint32*)d)[ 0] ^= ((quint32*)s)[ 0];
    ((quint32*)d)[ 1] ^= ((quint32*)s)[ 1];
    ((quint32*)d)[ 2] ^= ((quint32*)s)[ 2];
    ((quint32*)d)[ 3] ^= ((quint32*)s)[ 3];
}

void copy_and_key( void *d, const void *s, const void *k )
{
    ((quint32*)d)[ 0] = ((quint32*)s)[ 0] ^ ((quint32*)k)[ 0];
    ((quint32*)d)[ 1] = ((quint32*)s)[ 1] ^ ((quint32*)k)[ 1];
    ((quint32*)d)[ 2] = ((quint32*)s)[ 2] ^ ((quint32*)k)[ 2];
    ((quint32*)d)[ 3] = ((quint32*)s)[ 3] ^ ((quint32*)k)[ 3];
}

void add_round_key( quint8  d[N_BLOCK], const quint8  k[N_BLOCK] )
{
    xor_block(d, k);
}

void shift_sub_rows( quint8  st[N_BLOCK] )
{
    quint8  tt;

    st[ 0] = s_box(st[ 0]); st[ 4] = s_box(st[ 4]);
    st[ 8] = s_box(st[ 8]); st[12] = s_box(st[12]);

    tt = st[1]; st[ 1] = s_box(st[ 5]); st[ 5] = s_box(st[ 9]);
    st[ 9] = s_box(st[13]); st[13] = s_box( tt );

    tt = st[2]; st[ 2] = s_box(st[10]); st[10] = s_box( tt );
    tt = st[6]; st[ 6] = s_box(st[14]); st[14] = s_box( tt );

    tt = st[15]; st[15] = s_box(st[11]); st[11] = s_box(st[ 7]);
    st[ 7] = s_box(st[ 3]); st[ 3] = s_box( tt );
}

void inv_shift_sub_rows( quint8  st[N_BLOCK] )
{
    quint8  tt;

    st[ 0] = is_box(st[ 0]); st[ 4] = is_box(st[ 4]);
    st[ 8] = is_box(st[ 8]); st[12] = is_box(st[12]);

    tt = st[13]; st[13] = is_box(st[9]); st[ 9] = is_box(st[5]);
    st[ 5] = is_box(st[1]); st[ 1] = is_box( tt );

    tt = st[2]; st[ 2] = is_box(st[10]); st[10] = is_box( tt );
    tt = st[6]; st[ 6] = is_box(st[14]); st[14] = is_box( tt );

    tt = st[3]; st[ 3] = is_box(st[ 7]); st[ 7] = is_box(st[11]);
    st[11] = is_box(st[15]); st[15] = is_box( tt );
}

void mix_sub_columns( quint8  dt[N_BLOCK] )
{
    quint8  st[N_BLOCK];
    block_copy(st, dt);

    dt[ 0] = gfm2_sb(st[0]) ^ gfm3_sb(st[5]) ^ s_box(st[10]) ^ s_box(st[15]);
    dt[ 1] = s_box(st[0]) ^ gfm2_sb(st[5]) ^ gfm3_sb(st[10]) ^ s_box(st[15]);
    dt[ 2] = s_box(st[0]) ^ s_box(st[5]) ^ gfm2_sb(st[10]) ^ gfm3_sb(st[15]);
    dt[ 3] = gfm3_sb(st[0]) ^ s_box(st[5]) ^ s_box(st[10]) ^ gfm2_sb(st[15]);

    dt[ 4] = gfm2_sb(st[4]) ^ gfm3_sb(st[9]) ^ s_box(st[14]) ^ s_box(st[3]);
    dt[ 5] = s_box(st[4]) ^ gfm2_sb(st[9]) ^ gfm3_sb(st[14]) ^ s_box(st[3]);
    dt[ 6] = s_box(st[4]) ^ s_box(st[9]) ^ gfm2_sb(st[14]) ^ gfm3_sb(st[3]);
    dt[ 7] = gfm3_sb(st[4]) ^ s_box(st[9]) ^ s_box(st[14]) ^ gfm2_sb(st[3]);

    dt[ 8] = gfm2_sb(st[8]) ^ gfm3_sb(st[13]) ^ s_box(st[2]) ^ s_box(st[7]);
    dt[ 9] = s_box(st[8]) ^ gfm2_sb(st[13]) ^ gfm3_sb(st[2]) ^ s_box(st[7]);
    dt[10] = s_box(st[8]) ^ s_box(st[13]) ^ gfm2_sb(st[2]) ^ gfm3_sb(st[7]);
    dt[11] = gfm3_sb(st[8]) ^ s_box(st[13]) ^ s_box(st[2]) ^ gfm2_sb(st[7]);

    dt[12] = gfm2_sb(st[12]) ^ gfm3_sb(st[1]) ^ s_box(st[6]) ^ s_box(st[11]);
    dt[13] = s_box(st[12]) ^ gfm2_sb(st[1]) ^ gfm3_sb(st[6]) ^ s_box(st[11]);
    dt[14] = s_box(st[12]) ^ s_box(st[1]) ^ gfm2_sb(st[6]) ^ gfm3_sb(st[11]);
    dt[15] = gfm3_sb(st[12]) ^ s_box(st[1]) ^ s_box(st[6]) ^ gfm2_sb(st[11]);
  }

void inv_mix_sub_columns( quint8  dt[N_BLOCK] )
{
    quint8  st[N_BLOCK];
    block_copy(st, dt);

    dt[ 0] = is_box(gfm_e(st[ 0]) ^ gfm_b(st[ 1]) ^ gfm_d(st[ 2]) ^ gfm_9(st[ 3]));
    dt[ 5] = is_box(gfm_9(st[ 0]) ^ gfm_e(st[ 1]) ^ gfm_b(st[ 2]) ^ gfm_d(st[ 3]));
    dt[10] = is_box(gfm_d(st[ 0]) ^ gfm_9(st[ 1]) ^ gfm_e(st[ 2]) ^ gfm_b(st[ 3]));
    dt[15] = is_box(gfm_b(st[ 0]) ^ gfm_d(st[ 1]) ^ gfm_9(st[ 2]) ^ gfm_e(st[ 3]));

    dt[ 4] = is_box(gfm_e(st[ 4]) ^ gfm_b(st[ 5]) ^ gfm_d(st[ 6]) ^ gfm_9(st[ 7]));
    dt[ 9] = is_box(gfm_9(st[ 4]) ^ gfm_e(st[ 5]) ^ gfm_b(st[ 6]) ^ gfm_d(st[ 7]));
    dt[14] = is_box(gfm_d(st[ 4]) ^ gfm_9(st[ 5]) ^ gfm_e(st[ 6]) ^ gfm_b(st[ 7]));
    dt[ 3] = is_box(gfm_b(st[ 4]) ^ gfm_d(st[ 5]) ^ gfm_9(st[ 6]) ^ gfm_e(st[ 7]));

    dt[ 8] = is_box(gfm_e(st[ 8]) ^ gfm_b(st[ 9]) ^ gfm_d(st[10]) ^ gfm_9(st[11]));
    dt[13] = is_box(gfm_9(st[ 8]) ^ gfm_e(st[ 9]) ^ gfm_b(st[10]) ^ gfm_d(st[11]));
    dt[ 2] = is_box(gfm_d(st[ 8]) ^ gfm_9(st[ 9]) ^ gfm_e(st[10]) ^ gfm_b(st[11]));
    dt[ 7] = is_box(gfm_b(st[ 8]) ^ gfm_d(st[ 9]) ^ gfm_9(st[10]) ^ gfm_e(st[11]));

    dt[12] = is_box(gfm_e(st[12]) ^ gfm_b(st[13]) ^ gfm_d(st[14]) ^ gfm_9(st[15]));
    dt[ 1] = is_box(gfm_9(st[12]) ^ gfm_e(st[13]) ^ gfm_b(st[14]) ^ gfm_d(st[15]));
    dt[ 6] = is_box(gfm_d(st[12]) ^ gfm_9(st[13]) ^ gfm_e(st[14]) ^ gfm_b(st[15]));
    dt[11] = is_box(gfm_b(st[12]) ^ gfm_d(st[13]) ^ gfm_9(st[14]) ^ gfm_e(st[15]));
  }

// Set the cipher key for the pre-keyed version
aes_result aes_set_key( const quint8 key[], size_t keylen, aes_context ctx[1] )
{
    quint8  cc, rc, hi;

    switch( keylen )
    {
    case 128:
        keylen = 16;
        break;
    case 192:
        keylen = 24;
        break;
    case 256:
        keylen = 32;
        break;
    default:
        ctx->rnd = 0;
        return -1;
    }
    block_copy_nn(ctx->ksch, key, keylen);
    hi = (keylen + 28) << 2;
    ctx->rnd = (hi >> 4) - 1;
    for( cc = keylen, rc = 1; cc < hi; cc += 4 )
    {   quint8  tt, t0, t1, t2, t3;

        t0 = ctx->ksch[cc - 4];
        t1 = ctx->ksch[cc - 3];
        t2 = ctx->ksch[cc - 2];
        t3 = ctx->ksch[cc - 1];
        if( cc % keylen == 0 )
        {
            tt = t0;
            t0 = s_box(t1) ^ rc;
            t1 = s_box(t2);
            t2 = s_box(t3);
            t3 = s_box(tt);
            rc = f2(rc);
        }
        else if( keylen > 24 && cc % keylen == 16 )
        {
            t0 = s_box(t0);
            t1 = s_box(t1);
            t2 = s_box(t2);
            t3 = s_box(t3);
        }
        tt = cc - keylen;
        ctx->ksch[cc + 0] = ctx->ksch[tt + 0] ^ t0;
        ctx->ksch[cc + 1] = ctx->ksch[tt + 1] ^ t1;
        ctx->ksch[cc + 2] = ctx->ksch[tt + 2] ^ t2;
        ctx->ksch[cc + 3] = ctx->ksch[tt + 3] ^ t3;
    }
    return 0;
}

// Encrypt a single block of 16 bytes
aes_result aes_encrypt( const quint8 in[N_BLOCK], quint8 out[N_BLOCK], const aes_context ctx[1] )
{
    if( ctx->rnd )
    {
        quint8  s1[N_BLOCK], r;
        copy_and_key( s1, in, ctx->ksch );

        for( r = 1 ; r < ctx->rnd ; ++r )
        {
            mix_sub_columns( s1 );
            add_round_key( s1, ctx->ksch + r * N_BLOCK);
        }
        shift_sub_rows( s1 );
        copy_and_key( out, s1, ctx->ksch + r * N_BLOCK );
    }
    else
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

// CBC encrypt a number of blocks (input and return an IV)
aes_result aes_cbc_encrypt(const quint8 *in, quint8 *out, const size_t size, quint8 iv[N_BLOCK], const aes_context ctx[1] )
{
    if (size % N_BLOCK != 0)
        return EXIT_FAILURE;

    size_t n_block = size / N_BLOCK;

    while(n_block--)
    {
        xor_block(iv, in);
        if(aes_encrypt(iv, iv, ctx) != EXIT_SUCCESS)
            return EXIT_FAILURE;
        memcpy(out, iv, N_BLOCK);
        in += N_BLOCK;
        out += N_BLOCK;
    }
    return EXIT_SUCCESS;
}

// ECB encrypt a number of blocks
aes_result aes_ecb_encrypt(const quint8 *in, quint8 *out, const size_t size, const aes_context ctx[1] )
{
    if (size % N_BLOCK != 0)
        return EXIT_FAILURE;

    size_t n_block = size / N_BLOCK;

    while(n_block--)
    {
        if(aes_encrypt(in, out, ctx) != EXIT_SUCCESS)
            return EXIT_FAILURE;
        in += N_BLOCK;
        out += N_BLOCK;
    }
    return EXIT_SUCCESS;
}

aes_result aes_cbc_encrypt_stream(QIODevice *in, QIODevice *out, quint8 iv[N_BLOCK], const aes_context ctx[1] )
{
    const qint64 size = in->size() - in->pos();

    size_t n_block = size / N_BLOCK;

    quint8  inBlock[N_BLOCK];
    quint8  outBlock[N_BLOCK];

    while(n_block--)
    {
        in->read((char *)inBlock, N_BLOCK);

        xor_block(iv, inBlock);
        if(aes_encrypt(iv, iv, ctx) != EXIT_SUCCESS)
            return EXIT_FAILURE;

        memcpy(outBlock, iv, N_BLOCK);

        out->write((char *)outBlock, N_BLOCK);
    }

    const size_t leftOvers = size % N_BLOCK;
    const quint8 paddingBytes = N_BLOCK - leftOvers;

    in->read((char *)inBlock, leftOvers);
    for (int i = leftOvers; i < N_BLOCK; ++i)
        inBlock[i] = paddingBytes;

    xor_block(iv, inBlock);
    if(aes_encrypt(iv, iv, ctx) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    memcpy(outBlock, iv, N_BLOCK);

    out->write((char *)outBlock, N_BLOCK);

    return EXIT_SUCCESS;
}

aes_result aes_ecb_encrypt_stream(QIODevice *in, QIODevice *out, const aes_context ctx[1] )
{
    const qint64 size = in->size() - in->pos();

    size_t n_block = size / N_BLOCK;

    quint8  block[N_BLOCK];

    while(n_block--)
    {
        in->read((char *)block, N_BLOCK);

        if(aes_encrypt(block, block, ctx) != EXIT_SUCCESS)
            return EXIT_FAILURE;

        out->write((char *)block, N_BLOCK);
    }

    const size_t leftOvers = size % N_BLOCK;
    const quint8 paddingBytes = N_BLOCK - leftOvers;

    in->read((char *)block, leftOvers);
    for (int i = leftOvers; i < N_BLOCK; i++)
        block[i] = paddingBytes;

    if(aes_encrypt(block, block, ctx) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    out->write((char *)block, N_BLOCK);

    return EXIT_SUCCESS;
}

// Decrypt a single block of 16 bytes
aes_result aes_decrypt(const quint8 in[N_BLOCK], quint8 out[N_BLOCK], const aes_context ctx[1] )
{
    if( ctx->rnd )
    {
        quint8  s1[N_BLOCK], r;
        copy_and_key( s1, in, ctx->ksch + ctx->rnd * N_BLOCK );
        inv_shift_sub_rows( s1 );

        for( r = ctx->rnd ; --r ; )
        {
            add_round_key( s1, ctx->ksch + r * N_BLOCK );
            inv_mix_sub_columns( s1 );
        }
        copy_and_key( out, s1, ctx->ksch );
    }
    else
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

// CBC decrypt a number of blocks (input and return an IV)
aes_result aes_cbc_decrypt(const quint8 *in, quint8 *out, const size_t size, quint8 iv[N_BLOCK], const aes_context ctx[1] )
{
    if (size % N_BLOCK != 0)
        return EXIT_FAILURE;

    size_t n_block = size / N_BLOCK;

    quint8  tmp[N_BLOCK];

    while (n_block--)
    {
        memcpy(tmp, in, N_BLOCK);
        if(aes_decrypt(in, out, ctx) != EXIT_SUCCESS)
            return EXIT_FAILURE;
        xor_block(out, iv);
        memcpy(iv, tmp, N_BLOCK);
        in += N_BLOCK;
        out += N_BLOCK;
    }
    return EXIT_SUCCESS;
}

// ECB decrypt a number of blocks
aes_result aes_ecb_decrypt(const quint8 *in, quint8 *out, const size_t size, const aes_context ctx[1] )
{
    if (size % N_BLOCK != 0)
        return EXIT_FAILURE;

    size_t n_block = size / N_BLOCK;

    while (n_block--)
    {
        if(aes_decrypt(in, out, ctx) != EXIT_SUCCESS)
            return EXIT_FAILURE;
        in += N_BLOCK;
        out += N_BLOCK;
    }
    return EXIT_SUCCESS;
}

// CBC decrypt a number of blocks (input and return an IV)
aes_result aes_cbc_decrypt_stream(QIODevice *in, QIODevice *out, quint8 iv[N_BLOCK], const aes_context ctx[1] )
{
    const qint64 size = in->size() - in->pos();

    if (size % N_BLOCK != 0)
        return EXIT_FAILURE;

    size_t n_block = size / N_BLOCK;

    quint8  tmp[N_BLOCK];
    quint8  inBlock[N_BLOCK];
    quint8  outBlock[N_BLOCK];

    while (n_block > 1)
    {
        in->read((char *)inBlock, N_BLOCK);

        memcpy(tmp, inBlock, N_BLOCK);

        if(aes_decrypt(inBlock, outBlock, ctx) != EXIT_SUCCESS)
            return EXIT_FAILURE;

        xor_block(outBlock, iv);

        memcpy(iv, tmp, N_BLOCK);

        out->write((char *)outBlock, N_BLOCK);

        n_block--;
    }

    if (n_block == 1)
    {
        in->read((char *)inBlock, N_BLOCK);

        memcpy(tmp, inBlock, N_BLOCK);

        if(aes_decrypt(inBlock, outBlock, ctx) != EXIT_SUCCESS)
            return EXIT_FAILURE;

        xor_block(outBlock, iv);

        memcpy(iv, tmp, N_BLOCK);

        const quint8 paddingBytes = outBlock[N_BLOCK - 1];
        size_t bytesToChop = 0;

        if (paddingBytes <= N_BLOCK)
        {
            for(int i = 1; i <= paddingBytes; i++)
            {
                if (outBlock[N_BLOCK - i] == paddingBytes)
                    ++bytesToChop;
            }
        }

        if (bytesToChop == paddingBytes)
            out->write((char *)outBlock, N_BLOCK - paddingBytes);
        else
            out->write((char *)outBlock, N_BLOCK);
    }

    return EXIT_SUCCESS;
}

// ECB decrypt a number of blocks
aes_result aes_ecb_decrypt_stream(QIODevice *in, QIODevice *out, const aes_context ctx[1] )
{
    const qint64 size = in->size() - in->pos();

    if (size % N_BLOCK != 0)
        return EXIT_FAILURE;

    size_t n_block = size / N_BLOCK;

    quint8  block[N_BLOCK];

    while (n_block > 1)
    {
        in->read((char *)block, N_BLOCK);

        if(aes_decrypt(block, block, ctx) != EXIT_SUCCESS)
            return EXIT_FAILURE;

        out->write((char *)block, N_BLOCK);

        n_block--;
    }

    if (n_block == 1)
    {
        in->read((char *)block, N_BLOCK);

        if(aes_decrypt(block, block, ctx) != EXIT_SUCCESS)
            return EXIT_FAILURE;

        const quint8 paddingBytes = block[N_BLOCK - 1];
        int bytesToChop = 0;

        if (paddingBytes <= N_BLOCK)
            for(int i = 1; i <= paddingBytes; i++)
                if (block[N_BLOCK - i] == paddingBytes)
                    ++bytesToChop;

        if (bytesToChop == paddingBytes)
            out->write((char *)block, N_BLOCK - paddingBytes);
        else
            out->write((char *)block, N_BLOCK);
    }

    return EXIT_SUCCESS;
}

/* TinyAES */

// encryption with IV

#ifdef ENABLE_MODE_ECB
QByteArray TinyAES::encrypt(const QByteArray &input, const QByteArray &key, const Mode mode)
#else
QByteArray TinyAES::encrypt(const QByteArray &input, const QByteArray &key)
#endif
{
    if (input.size() == 0)
        return QByteArray();

#ifdef ENABLE_MODE_ECB
    if (mode == ECB)
    {
        if (!checkParams(key, QByteArray(N_BLOCK, Qt::Uninitialized)))
            return QByteArray();

        QByteArray in(input);
        addPadding(in);

        const int inSize = in.size();
        QByteArray result(inSize, 0);

        aes_context context;
        aes_set_key((quint8 *)key.constData(), key.size() * 8, &context);
        aes_ecb_encrypt((quint8 *)in.constData(), (quint8 *) result.data(), inSize, &context);

        return result;
    }
    else if (mode == CBC)
    {
#endif
        QByteArray iv = TinyAES::getRandom128Bits();

        if (!checkParams(key, iv))
            return QByteArray();

        QByteArray result = encrypt(input, key, iv);
        return result.prepend(iv);
#ifdef ENABLE_MODE_ECB
    }
    else
    {
        qCritical("Encrypting data with unknown operation mode!");
        return QByteArray();
    }
#endif
}

#ifdef ENABLE_MODE_ECB
bool TinyAES::encrypt(QIODevice *input, QIODevice *output, const QByteArray &key, const Mode mode)
#else
bool TinyAES::encrypt(QIODevice *input, QIODevice *output, const QByteArray &key)
#endif
{
    if ((input->size() - input->pos()) == 0)
        return false;

#ifdef ENABLE_MODE_ECB
    if (mode == ECB)
    {
        if (!checkParams(key, QByteArray(N_BLOCK, Qt::Uninitialized)) || (input->size() - input->pos()) == 0)
            return false;

        aes_context context;
        aes_set_key((quint8 *)key.constData(), key.size() * 8, &context);
        if (aes_ecb_encrypt_stream(input, output, &context) == EXIT_FAILURE)
            return false;

        return true;
    }
    else if (mode == CBC)
    {
#endif
        QByteArray iv = TinyAES::getRandom128Bits();

        if (!checkParams(key, iv))
            return false;

        output->write(iv);
        return encrypt(input, output, key, iv);
#ifdef ENABLE_MODE_ECB
    }
    else
    {
        qCritical("Encrypting data stream with unknown operation mode!");
        return false;
    }
#endif
}

#ifdef ENABLE_MODE_ECB
QByteArray TinyAES::decrypt(QByteArray input, const QByteArray &key, const Mode mode)
#else
QByteArray TinyAES::decrypt(QByteArray input, const QByteArray &key)
#endif
{
    if (input.size() == 0)
        return QByteArray();

#ifdef ENABLE_MODE_ECB
    if (mode == ECB)
    {
        if (!checkParams(key, QByteArray(N_BLOCK, Qt::Uninitialized)) || input.size() == 0)
            return QByteArray();

        const int inputSize = input.size();
        QByteArray result(inputSize, 0);

        aes_context context;
        aes_set_key((quint8 *)key.constData(), key.size() * 8, &context);
        aes_ecb_decrypt((quint8 *)input.constData(), (quint8 *)result.data(), inputSize, &context);

        removePadding(result);

        return result;
    }
    else if (mode == CBC)
    {
#endif
        QByteArray iv = input.left(N_BLOCK);

        if (!checkParams(key, iv))
            return QByteArray();

        input.remove(0, N_BLOCK);
        return decrypt(input, key, iv);
#ifdef ENABLE_MODE_ECB
    }
    else
    {
        qCritical("Decrypting data with unknown operation mode!");
        return QByteArray();
    }
#endif
}

#ifdef ENABLE_MODE_ECB
bool TinyAES::decrypt(QIODevice *input, QIODevice *output, const QByteArray &key, const Mode mode)
#else
bool TinyAES::decrypt(QIODevice *input, QIODevice *output, const QByteArray &key)
#endif
{
    if ((input->size() - input->pos()) == 0)
        return false;

#ifdef ENABLE_MODE_ECB
    if (mode == ECB)
    {
        if (!checkParams(key, QByteArray(N_BLOCK, Qt::Uninitialized)) || (input->size() - input->pos()) == 0)
            return false;

        aes_context context;
        aes_set_key((quint8 *)key.constData(), key.size() * 8, &context);
        if (aes_ecb_decrypt_stream(input, output, &context) == EXIT_FAILURE)
            return false;

        return true;
    }
    else if (mode == CBC)
    {
#endif
        QByteArray iv = input->read(N_BLOCK);

        if (!checkParams(key, iv))
            return false;

        return decrypt(input, output, key, iv);
#ifdef ENABLE_MODE_ECB
    }
    else
    {
        qCritical("Encrypting data stream with unknown operation mode!");
        return false;
    }
#endif
}

// basic encryption
QByteArray TinyAES::encrypt(QByteArray input, const QByteArray &key, const QByteArray &iv)
{
    if (!checkParams(key, iv) || input.size() == 0)
        return QByteArray();

    // add padding
    addPadding(input);

    QByteArray result;
    int inputSize = input.size();
    result.resize(inputSize);

    QByteArray modIv = iv; // since aes_cbc_encrypt will modify the input IV

    aes_context context;
    aes_set_key((quint8 *)key.constData(), key.size() * 8, &context);
    aes_cbc_encrypt((quint8 *)input.constData(), (quint8 *)result.data(), inputSize, (quint8 *)modIv.data(), &context);

    return result;
}

bool TinyAES::encrypt(QIODevice *input, QIODevice *output, const QByteArray &key, const QByteArray &iv)
{
    if (!checkParams(key, iv) || (input->size() - input->pos()) == 0)
        return false;

    QByteArray modIv = iv; // since aes_cbc_encrypt_stream will modify the input IV

    aes_context context;
    aes_set_key((quint8 *)key.constData(), key.size() * 8, &context);
    if (aes_cbc_encrypt_stream(input, output, (quint8 *)modIv.data(), &context) == EXIT_FAILURE)
        return false;

    return true;
}

QByteArray TinyAES::decrypt(const QByteArray &input, const QByteArray &key, const QByteArray &iv)
{
    if (!checkParams(key, iv) || input.size() == 0)
        return QByteArray();

    QByteArray result;
    int inputSize = input.size();
    result.resize(inputSize);

    QByteArray modIv = iv; // since aes_cbc_decrypt will modify the input IV

    aes_context context;
    aes_set_key((quint8 *)key.constData(), key.size() * 8, &context);
    aes_cbc_decrypt((quint8 *)input.constData(), (quint8 *)result.data(), inputSize, (quint8 *)modIv.data(), &context);

    removePadding(result);

    return result;
}

bool TinyAES::decrypt(QIODevice *input, QIODevice *output, const QByteArray &key, const QByteArray &iv)
{
    if (!checkParams(key, iv) || (input->size() - input->pos()) == 0)
        return false;

    QByteArray modIv = iv; // since aes_cbc_decrypt_stream will modify the input IV

    aes_context context;
    aes_set_key((quint8 *)key.constData(), key.size() * 8, &context);
    if (aes_cbc_decrypt_stream(input, output, (quint8 *)modIv.data(), &context) == EXIT_FAILURE)
        return false;

    return true;
}

// helper functions
QByteArray TinyAES::hexStringToByte(QString key)
{
    return QByteArray::fromHex(QString(key).toLatin1());
}

QByteArray TinyAES::getRandom128Bits()
{
    QUuid id = QUuid::createUuid();

    QByteArray bytes(16, Qt::Uninitialized);
    uchar *data = reinterpret_cast<uchar*>(bytes.data());

    memmove(data, &id.data1, sizeof(quint32));
    data += sizeof(quint32);

    memmove(data, &id.data2, sizeof(quint16));
    data += sizeof(quint16);

    memmove(data, &id.data3, sizeof(quint16));
    data += sizeof(quint16);

    for (int i = 0; i < 8; ++i) {
        *(data) = id.data4[i];
        data++;
    }

    return bytes;
}

bool TinyAES::checkParams(const QByteArray &key, const QByteArray &iv)
{
    const int keySize = key.size();
    const int ivSize = iv.size();

    if (keySize != 16 && keySize != 24 && keySize != 32)
        return false;

    if (ivSize != N_BLOCK)
        return false;

    return true;
}

// pkcs#7 padding
void TinyAES::removePadding(QByteArray &input)
{
    const char paddingBytes = input.at(input.size() - 1);
    int bytesToChop = 0;

    if (paddingBytes <= N_BLOCK)
        for(int i = 1; i <= paddingBytes; i++)
        {
            if (input.at(input.size() - i) == paddingBytes)
                ++bytesToChop;
        }

    if (bytesToChop == paddingBytes)
        input.chop(bytesToChop);
}

void TinyAES::addPadding(QByteArray &input)
{
    const int size = input.size();
    const char paddingBytes = N_BLOCK - (size % N_BLOCK);

    if (paddingBytes <= N_BLOCK)
        for (int i = 0; i < paddingBytes; i++)
            input.append(paddingBytes);
}
