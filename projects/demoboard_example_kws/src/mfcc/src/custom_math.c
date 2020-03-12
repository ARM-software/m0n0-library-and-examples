
/*
 * Copyright (c) 2020, Arm Limited
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include "custom_math.h"
#include "arm_math.h"
#include <errno.h>

int32_t RoundUpToNearestPowerOfTwo(int32_t n) {
	n--;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	return n+1;
}


//take the log of a positive 32 bit integer, return result as Q16.16
q31_t log_32(int32_t x) {

	// https://www.quinapalus.com/efunc.html
	int32_t t,y;

	y=0xa65af;
	if(x<0x00008000) x<<=16, y-=0xb1721;
	if(x<0x00800000) x<<= 8, y-=0x58b91;
	if(x<0x08000000) x<<= 4, y-=0x2c5c8;
	if(x<0x20000000) x<<= 2, y-=0x162e4;
	if(x<0x40000000) x<<= 1, y-=0x0b172;
	t=x+(x>>1); if((t&0x80000000)==0) x=t,y-=0x067cd;
	t=x+(x>>2); if((t&0x80000000)==0) x=t,y-=0x03920;
	t=x+(x>>3); if((t&0x80000000)==0) x=t,y-=0x01e27;
	t=x+(x>>4); if((t&0x80000000)==0) x=t,y-=0x00f85;
	t=x+(x>>5); if((t&0x80000000)==0) x=t,y-=0x007e1;
	t=x+(x>>6); if((t&0x80000000)==0) x=t,y-=0x003f8;
	t=x+(x>>7); if((t&0x80000000)==0) x=t,y-=0x001fe;
	x=0x80000000-x;
	y-=x>>15;
	return y;
}


//http://www.codecodex.com/wiki/Calculate_an_integer_square_root
// typedef unsigned short int uint16_t;
uint32_t	// OR uint16 OR uint8_t
	sqrt_int32 (uint32_t n) // OR isqrt16 ( uint16 n ) OR	isqrt8 ( uint8_t n ) - respectively [ OR overloaded as isqrt (uint16_t?? n) in C++ ]
	{
		register uint32_t // OR register uint16 OR register uint8_t - respectively
			root, remainder, place;

		root = 0;
		remainder = n;
		place = 0x40000000; // OR place = 0x4000; OR place = 0x40; - respectively

		while (place > remainder)
			place = place >> 2;
		while (place)
		{
			if (remainder >= root + place)
			{
				remainder = remainder - root - place;
				root = root + (place << 1);
			}
			root = root >> 1;
			place = place >> 2;
		}
		return root;
}

/*
 * Fake arm_sqrt_q15 call
 * Original arm_sqrt_q15 uses flating point approximation
 */
arm_status arm_sqrt_q31(
	q31_t in,
	q31_t * pOut)
{
	*pOut = sqrt_int32(in);
	return (ARM_MATH_SUCCESS);
}
