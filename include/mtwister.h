#ifndef __MTWISTER_H
#define __MTWISTER_H

#define STATE_VECTOR_LENGTH 624
#define STATE_VECTOR_M      397 /* changes to STATE_VECTOR_LENGTH also require changes to this */

typedef struct tagMTRand {
  unsigned long mt[STATE_VECTOR_LENGTH];
  int index;
} mt_rand;

mt_rand mt_seed_rand(unsigned long seed);
unsigned long mt_gen_rand_long(mt_rand* rand);
double mt_gen_rand(mt_rand* rand);

#endif /* __MTWISTER_H */