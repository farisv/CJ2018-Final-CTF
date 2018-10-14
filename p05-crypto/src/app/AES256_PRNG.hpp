/*
 *  Copyright (C) 2014 Jens Thoms Toerring
 *
 *  This file is part of AES256.
 *
 *  AES256 is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 *  AES256 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#pragma once
#if ! defined AES256_PRNG_hpp_
#define AES256_PRNG_hpp_


#include "AES256_Base.hpp"
#include "Byte_Block.hpp"


/*---------------------------------------------*
 * Pseudo-random generator using AES-256. Produces pseudo-random
 * bytes that can be obtained either as a std::string', via a
 * buffer of 16 'unsigned char' or a Byte_Block object.
 *
 * The way it works tries to follow the "Recommendation for Random
 * Number Generation Using Deterministic Random Bit Generators"
 * in the NIST publication SP 800-90A (see CTR_BRBG).
 *
 * When used for anything that requires a cryptographically secure
 * random generator (not that I claim that this implementation is
 * cryptographically secure!) at least a 16 byte seed, obtained
 * from a random source must be set, better also a 32 byte long
 * random key!
 *
 * This isn't a very fast generator (it's slower than reading from
 * '/dev/urandom' using 'dd' on my machine) and hasn't been tested
 * extensively (and may not be cryptographically secure). All that
 * can be claimed is that some checks with the 'ent' program (see
 * <http://www.fourmilab.ch/random/>) on the quality of the random
 * bytes produced seem to indicate it's not doing much worse than
 * '/dev/urandom'.
 *
 * Please note that in its current form without a proper vetting
 * by an expert this random generator is NOT suitable for security
 * critical applications! And any bets are off if not a good (i.e.
 * in no way predictable) seed is supplied! If you should consider
 * using it for security sensitive purposes you also should change
 * the values of the key and the counter and keep them secret. And
 * you will have to carefully analyze the code for any possible
 * security flaws!
 *---------------------------------------------*/

class AES256_PRNG
{
	typedef  unsigned char  byte;

  public :

    AES256_PRNG( std::string const & seed = std::string( ),
                 std::string const & key  = std::string( ) );

    // Reseed the generator

    void
    reseed( std::string const & seed );

    // Get 'size' bytes in a std::string

    std::string
    get( size_t size );

    // Get 'size' bytes into a (user supplied) buffer
    
    byte *
    get_buffer( byte   * buf,
                size_t   size );

    // Get a N bytes into a (user supplied) Byte_Block object 

	template< size_t N >
	void
	get_block( Byte_Block< N > & dst )
	{
		get_buffer( dst, N );
	}


  private :

    void
    fill_pool( );

    // AES256_Base instance for encryption

    AES256_Base m_aes256_base;

    // Counter

    Byte_Block< 16 > m_counter;

    // Ring buffer ("pool") size - this must be an integer multiple of 16

    static size_t const s_pool_size = 64;

    // Ring buffer ("pool") for holding pre-computed random bytes

    Byte_Block< s_pool_size > m_pool;

    // Low amd high filling markers for the buffer

    size_t m_low;
    size_t m_high;
    
    // Initial value of counter

    static byte s_counter[ 16 ];

    // Default key for encrypting with AES-256

    static byte s_key[ 32 ];
};


#endif


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
