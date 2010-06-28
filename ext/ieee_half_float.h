/*
 * ieee_half_float.h - Conversion between double from/to half-precision float.
 *
 * Copyright (c) 2010, Diego Che
 *
 * Adapted from the implementation of James Tursa.
 *
 *******************************************************************************
 *
 * Filename:    ieeehalfprecision.c
 * Programmer:  James Tursa
 * Version:     1.0
 * Date:        March 3, 2009
 * Copyright:   (c) 2009 by James Tursa, All Rights Reserved
 *
 *  This code uses the BSD License:
 *
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions are 
 *  met:
 *
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the distribution
 *      
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef RCL_IEEE_HALF_FLOAT_H_
#define RCL_IEEE_HALF_FLOAT_H_

#include <math.h>
#include "capi.h"

#define Expect_Integer(ro) \
    do { \
        if (!FIXNUM_P(ro) && TYPE(ro) != T_BIGNUM) { \
            rb_raise(rb_eTypeError, "Expected %s is an Integer.", #ro); \
        } \
    } while (0)

#define Expect_Float(ro) \
    do { \
        if (TYPE(ro) != T_FLOAT && !FIXNUM_P(ro) && TYPE(ro) != T_BIGNUM) { \
            rb_raise(rb_eTypeError, "Expected %s is of type Float.", #ro); \
        } \
    } while (0)

// Convert a Ruby object to half precision float.
inline cl_half Extract_Half(VALUE ro)
{
    Expect_Float(ro);

    double v = NUM2DBL(ro);
    uint32_t *xp = (uint32_t *)&v; xp++;    // Skip low mantissa bits.
    uint32_t x = *xp;
    
    TRACE("To convert %F to half, and effective bits is %x.\n", v, x);
    
    cl_half h;
    if ((x & 0x7FFFFFFFu) == 0) {   // signed zero.
        h = (cl_half)(x >> 16);
    } else {
        uint32_t xs, xe, xm;   // sign bit, exponent bits and mantissa bits.
        xs = x & 0x80000000u;
        xe = x & 0x7FF00000u;
        xm = x & 0x000FFFFFu;
        
        if( xe == 0 ) {  // Denormal will underflow, return a signed zero
            h = (cl_half) (xs >> 16);
        } else if( xe == 0x7FF00000u ) {  // Inf or NaN (all the exponent bits are set)
            if( xm == 0 ) { // If mantissa is zero ...
                h = (cl_half)((xs >> 16) | 0x7C00u); // Signed Inf
            } else {
                h = (cl_half)0xFE00u; // NaN, only 1st mantissa bit set
            }
        } else { // Normalized number
            uint16_t hs, he, hm;
            int hes;
            
            hs = (uint16_t)(xs >> 16); // Sign bit
            hes = ((int)(xe >> 20)) - 1023 + 15; // Exponent unbias the double, then bias the halfp
            if ( hes >= 0x1F ) {  // Overflow
                h = (uint16_t)((xs >> 16) | 0x7C00u); // Signed Inf
            } else if ( hes <= 0 ) {  // Underflow
                if ((10 - hes) > 21) {  // Mantissa shifted all the way off & no rounding possibility
                    hm = (uint16_t)0u;  // Set mantissa to zero
                } else {
                    xm |= 0x00100000u;  // Add the hidden leading bit
                    hm = (uint16_t) (xm >> (11 - hes)); // Mantissa
                    if ((xm >> (10 - hes)) & 0x00000001u) { // Check for rounding
                        hm += (uint16_t) 1u; // Round, might overflow into exp bit, but this is OK
                    }
                }
                h = (hs | hm); // Combine sign bit and mantissa bits, biased exponent is zero
            } else {
                he = (uint16_t) (hes << 10); // Exponent
                hm = (uint16_t) (xm >> 10); // Mantissa
                if (xm & 0x00000200u) // Check for rounding
                    h = (hs | he | hm) + (uint16_t)1u; // Round, might overflow to inf, this is OK
                else
                    h = (hs | he | hm);  // No rounding
            }
        }
    }

    return h;
}

inline VALUE rcl_half_float_new(cl_half h)
{
    double v = 0;
    uint32_t *f = ((uint32_t *)&v); f++; // Leave low mantissa bits as 0.

    if( (h & 0x7FFFu) == 0 ) {  // Signed zero
        *f = ((uint32_t) h) << 16;  // Return the signed zero
    } else { // Not zero
        uint16_t hs, he, hm;
        uint32_t xs, xe, xm;
        int32_t xes;
        
        hs = h & 0x8000u;  // Pick off sign bit
        he = h & 0x7C00u;  // Pick off exponent bits
        hm = h & 0x03FFu;  // Pick off mantissa bits
        if (he == 0) {  // Denormal will convert to normalized
            int e = -1; // The following loop figures out how much extra to adjust the exponent
            do {
                e++;
                hm <<= 1;
            } while ((hm & 0x0400u) == 0); // Shift until leading bit overflows into exponent bit
            xs = ((uint32_t)hs) << 16; // Sign bit
            xes = ((int32_t)(he >> 10)) - 15 + 1023 - e; // Exponent unbias the halfp, then bias the double
            xe = (uint32_t)(xes << 20); // Exponent
            xm = ((uint32_t)(hm & 0x03FFu)) << 10; // Mantissa
            *f = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
        } else if (he == 0x7C00u) {  // Inf or NaN (all the exponent bits are set)
            if (hm == 0) { // If mantissa is zero ...
                *f = (((uint32_t)hs) << 16) | ((uint32_t)0x7FF00000u); // Signed Inf
            } else {
                *f = (uint32_t)0xFFF80000u; // NaN, only the 1st mantissa bit set
            }
        } else {
            xs = ((uint32_t)hs) << 16; // Sign bit
            xes = ((int32_t)(he >> 10)) - 15 + 1023; // Exponent unbias the halfp, then bias the double
            xe = (uint32_t)(xes << 20); // Exponent
            xm = ((uint32_t)hm) << 10; // Mantissa
            *f = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
        }
    }
    
    return rb_float_new(v);
}

#endif // RCL_IEEE_HALF_FLOAT_H_
