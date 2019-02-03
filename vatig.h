/*
 * vatig.h
 *
 *  Created on: Jan 30, 2019
 *  @author : Mouzakitis Nikolaos
 */

#ifndef VATIG_H_
#define VATIG_H_

#define SUPPORTED_NODES 0x3




int legacy[3] = { 43, 37, 55};
int secure[3] = {22, 19, 18};

// Generates Mac2 value.
// will store upper32 in the mac2_ptr[0], and lower32 in the mac2_ptr[1] addresses.
void createMac(int * mac2_ptr, int vg_counter, int upper, int lower) {
     mac2_ptr[0] =  ( ( ( (upper + 32) & 0x21) + (lower   + vg_counter) + 123) % 128 );
     mac2_ptr[1] =  ( ( ( (upper - 43) & 0x21) + (lower  + vg_counter) - 24) % 128 );
}

#endif /* VATIG_H_ */
