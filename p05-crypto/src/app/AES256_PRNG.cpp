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


#include "AES256_PRNG.hpp"


#include <algorithm>
#include <stdexcept>


/*---------------------------------------------*
 * Definitions of static class data members.
 *---------------------------------------------*/

// Initial value for counter

AES256_PRNG::byte AES256_PRNG::s_counter[ ] = { 0xd0, 0x73, 0xb0, 0xb1,
												0xc7, 0xda, 0x04, 0xde,
												0x58, 0x12, 0xa1, 0x4d,
												0xf6, 0x4d, 0x0f, 0xd3 };

// Default value of key

AES256_PRNG::byte AES256_PRNG::s_key[ ]     = { 0x08, 0x11, 0x34, 0xe3,
												0x73, 0x86, 0xc7, 0xc4,
												0xd1, 0x24, 0x3c, 0xb0,
												0x3c, 0xde, 0x0d, 0x73,
												0xea, 0xb2, 0xa6, 0x78,
												0xb7, 0x0b, 0x01, 0x05,
												0x62, 0x93, 0xee, 0x98,
												0xe2, 0xff, 0x46, 0x2f };


/*---------------------------------------------*
 * Constructor, optionally called with a 'seed' of (at
 * least) 16 bytes and a key of (at least) 32 bytes
 *---------------------------------------------*/

AES256_PRNG::AES256_PRNG( std::string const & seed,
                          std::string const & key )
    : m_aes256_base( s_key )
    , m_counter( s_counter )
    , m_low( 0 )
    , m_high( 0 )
{
    // If a seed is supplied XOR it with the counter value as the "nonce".

    if ( ! seed.empty( ) )
    {
        if ( seed.size( ) < 16 )
            throw std::invalid_argument( "Seed must consist of (at least) "
                                         "16 bytes" );
		m_counter ^= seed;
    }

    // If an appropriate key is supplied make the AES256 instance use it.

    if ( ! key.empty( ) )
        m_aes256_base.set_key( key );
}


/*---------------------------------------------*
 * Reseeds the random generator. If called with the
 * same seed the generator will recreate the exact
 * same sequence of bytes as before, so this should
 * only be done for testing purposes!
 *---------------------------------------------*/

void
AES256_PRNG::reseed( std::string const & seed )
{
        if ( seed.size( ) < 16 )
            throw std::invalid_argument( "Seed must consist of (at least) "
                                         "16 bytes" );

        // Set the counter back to its initial value and XOR the value
        // with the new seed. Then "empty" the pool by setting both
        // markers to 0.

		m_counter = Byte_Block< 16 >( s_counter ) ^ seed;
        m_high = m_low = 0;
}


/*---------------------------------------------*
 * Returns a string with 'size' pseudo-random bytes.
 * (This isn't very efficient with the extra allocation
 * and copying involved...)
 *---------------------------------------------*/

std::string
AES256_PRNG::get( size_t size )
{
    byte * buf = new byte [ size ];
    std::string r( reinterpret_cast< char const * >( get_buffer( buf, size ) ),
                   size );
    delete [ ] buf;
    return r;
}


/*---------------------------------------------*
 * Fills a user supplied buffer 'buf' with 'size' pseudo-random
 * bytes. Returns the address of the buffer passed to it.
 *---------------------------------------------*/

unsigned char *
AES256_PRNG::get_buffer( byte   * buf,
                         size_t   size )
{
    byte * ret = buf;
    size_t available = m_high - m_low;

    // Refill the pool if it's completely drained

    if ( available == 0 )
    {
        fill_pool( );
        available = s_pool_size;
    }

    // Now copy data from the pool to the user supplied buffer until it's
    // full, if necessary in chunks and with replenishing the pool in
    // between.

    while ( 1 )
    {
        size_t s = std::min( size, available );
        size_t low = m_low  % s_pool_size;

        if ( s <= s_pool_size - m_low )
            std::copy( m_pool.data( low ), m_pool.data( low + s ), buf );
        else
        {
            std::copy( m_pool.data( low ), m_pool.data( s_pool_size ), buf );
            std::copy( m_pool.data( ), m_pool.data( s - s_pool_size + low ),
                       buf + s_pool_size - low );
        }
        m_low += s;

        if ( size == s )
            return ret;

        buf += s;
        size -= s;
        fill_pool( );
        available = s_pool_size;
    }
}


/*---------------------------------------------*
 * The random generator keeps a pool of pre-computed random bytes.
 * If it's empty this method is called to refill it. It may not be
 * called before the pool is completely empty!
 *
 * Blocks of (16) random bytes are generated by encrypting
 * the counter value, which then is incremented. This is
 * equivalent to CTR mode with user supplied "seed" as the
 * nonce.
 *---------------------------------------------*/

void
AES256_PRNG::fill_pool( )
{
    while ( m_high - m_low < s_pool_size )
    {
		Byte_Block< 16 > tmp( m_counter++ );
        m_aes256_base.encrypt( tmp );
		m_pool.set( tmp, m_high % s_pool_size );
        m_high += 16;
    }

    // Avoid that the low and high markers ever overflow (we can't know
    // how many bytes will be requested and 'size_t' should suffice for
    // quite a long time, but why take chances?)

    m_low  %= s_pool_size;
    m_high  = m_low + s_pool_size;
}




/*=============================================================
 *=============================================================*/

/*---------------------------------------------*
 * Test main() functions:
 *
 * The first test just creates a few random bytes and
 * prints them out in hex. To enable it complile with
 * '-DTEST_AES256_RAND1'.
 *
 * The second creates over 16 MB of data and writes
 * them to a file named 'random.dat'. This file can
 * be used to check the properties of the random
 * numbers generated with tools like 'ent' (see
 * <http://www.fourmilab.ch/random/>). To enable
 * it compile with '-DTEST_AES256_RAND2'.
 *---------------------------------------------*/

#if defined TEST_AES256_RAND1

#include <cstdio>

int main( )
{
    std::string r = AES256_PRNG( ).get( 128 );

    for ( size_t i = 0; i < r.size( ); ++i )
        printf( "0x%02x ", ( unsigned char ) r.data( )[ i ] );
    printf( "\n" );
    return 0;
}

#endif

#if defined TEST_AES256_RAND2

#include <cstdio>

int main( )
{
    AES256_PRNG prng;
	unsigned char buf[ 0x1000 ];
    FILE * fp = fopen( "random.dat", "w" );

    for ( int i = 0; i < 0x1000; ++i )
        fwrite( prng.get_buffer( buf, sizeof buf ), sizeof buf, 1, fp );

    return 0;
}

#endif


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
