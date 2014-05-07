#ifndef __L4_CACHE_H_
#define __L4_CACHE_H_

/*
  We take the largest size and overflush. Possibly
  this is going to kill performance. Need to fix later.
*/
#define L1_CACHE_BYTES 		32	/* A guess */
#define L1_CACHE_SHIFT_MAX	 6	/* largest L1 which this arch supports */

#endif /* __L4_CACHE_H_ */
