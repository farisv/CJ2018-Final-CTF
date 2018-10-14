/*--------------------------------------------------
 *
 *   This file is in important parts based on the C implementation
 *   of AES-256 by Ilya O. Levin and Hal Finney, to be downloaded
 *   from <http://www.literatecode.com/aes256>. It contains the fol-
 *   lowing copyright notice:
 *
 *  --------------------------------------------------------------------
 *   Byte-oriented AES-256 implementation.
 *   All lookup tables replaced with 'on the fly' calculations.
 *
 *   Copyright (c) 2007-2009 Ilya O. Levin, http://www.literatecode.com
 *   Other contributors: Hal Finney
 *
 *   Permission to use, copy, modify, and distribute this software for any
 *   purpose with or without fee is hereby granted, provided that the above
 *   copyright notice and this permission notice appear in all copies.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *  --------------------------------------------------------------------
 *
 *  The original code also has been modified (beside the conversion
 *  from C to C++) in several places. Since it's derived work it
 *  is distributed under the same conditions as those chosen by the
 *  original authors.
 *
 *  Author: Jens Thoms Toerring <jt@toerring.de>
 *
 *--------------------------------------------------*/


#include "AES256_Base.hpp"
#include <stdexcept>


/*---------------------------------------------*
 * Definition of static members
 *---------------------------------------------*/

S_Box< false > const AES256_Base::s_sbox;
S_Box< true  > const AES256_Base::s_inv_sbox;


/*---------------------------------------------*
 * Constructor with the key given as a std::string
 *---------------------------------------------*/

AES256_Base::AES256_Base( std::string const & key )
{
    set_key( key );
}


/*---------------------------------------------*
 * Constructor with key given as a Byte_Block<32> object
 *---------------------------------------------*/

AES256_Base::AES256_Base( Byte_Block< 32 > const & key )
{
	init( key );
}


/*---------------------------------------------*
 * Returns the key (as a Byte_Block<32> object)
 *---------------------------------------------*/

void
AES256_Base::set_key( std::string const & key )
{
    if ( key.size( ) < 32 )
        throw std::invalid_argument( "Key must contain at least "
                                     "32 bytes of data" );
	init( key );
}


/*---------------------------------------------*
 * Returns the key (as a Byte_Block<32> object)
 *---------------------------------------------*/

Byte_Block< 32 > const &
AES256_Base::get_key( ) const
{
    return m_enc_key;
}


/*---------------------------------------------*
 * Encrypts a block of 16 bytes in place
 *---------------------------------------------*/

Byte_Block< 16 > &
AES256_Base::encrypt( Byte_Block< 16 > & buf )
{
    Byte_Block< 32 > key = m_enc_key;

    add_round_key( buf, key );
    byte rcon = 1;

    for ( byte i = 1; i < 14; ++i )
    {
        sub_bytes( buf );
        shift_rows( buf );
        mix_columns( buf );

        if ( i & 1 )
            add_round_key( buf, Byte_Block< 16 >( key, 16 ) );
        else
        {
            expand_enc_key( key, rcon );
            add_round_key( buf, key );
        }
    }

    sub_bytes( buf );
    shift_rows( buf );
    expand_enc_key( key, rcon ); 
    add_round_key( buf, key );

    return buf;
}


/*---------------------------------------------*
 * Encrypts a block of 16 bytes, returning the result in a new block
 *---------------------------------------------*/

Byte_Block< 16 >
AES256_Base::encrypt( Byte_Block< 16 > const & buf )
{
    Byte_Block< 16 > res( buf );
    return encrypt( res );
}


/*---------------------------------------------*
 * Decrypts a block of 16 bytes in place
 *---------------------------------------------*/

Byte_Block< 16 > &
AES256_Base::decrypt( Byte_Block< 16 > & buf )
{
    Byte_Block< 32 > key = m_dec_key;

    add_round_key( buf, key );
    inv_shift_rows( buf );
    inv_sub_bytes( buf );

    byte rcon = 0x80;

    for ( byte i = 1; i < 14; ++i )
    {
        if ( i & 1 )           
        {
            expand_dec_key( key, rcon );
            add_round_key( buf, Byte_Block< 16 >( key, 16 ) );
        }
        else
            add_round_key( buf, key );

        inv_mix_columns( buf );
        inv_shift_rows( buf );
        inv_sub_bytes( buf );
    }

    add_round_key( buf, key ); 

    return buf;
}


/*---------------------------------------------*
 * Decrypts a block of 16 bytes, returning the result in a new block
 *---------------------------------------------*/

Byte_Block< 16 >
AES256_Base::decrypt( Byte_Block< 16 > const & buf )
{
    Byte_Block< 16 > res( buf );
    return decrypt( res );
}



/*---------------------------------------------*
 *---------------------------------------------*/

void
AES256_Base::expand_enc_key( Byte_Block< 32 > & key,
                             byte             & rcon ) 
{
    key[ 0 ] ^= s_sbox[ key[ 29 ] ] ^ rcon;
    key[ 1 ] ^= s_sbox[ key[ 30 ] ];
    key[ 2 ] ^= s_sbox[ key[ 31 ] ];
    key[ 3 ] ^= s_sbox[ key[ 28 ] ];

    rcon <<= 1;

    for ( byte i = 4; i < 16; ++i )
        key[ i ] ^= key[ i - 4 ];

    for ( byte i = 16; i < 20; ++i )
        key[ i ] ^= s_sbox[ key[ i - 4 ] ];

    for ( byte i = 20; i < 32; ++i )
        key[ i ] ^= key[ i - 4 ];
}


/*---------------------------------------------*
 *---------------------------------------------*/

void
AES256_Base::expand_dec_key( Byte_Block< 32 > & key,
                             byte             & rcon ) 
{
    for ( byte i = 31; i >= 20; --i )
        key[ i ] ^= key[ i - 4 ];

    for ( byte i = 19; i >= 16; --i )
        key[ i ] ^= s_sbox[ key[ i - 4 ] ];

    for ( byte i = 15; i >= 4; --i )
        key[ i ] ^= key[ i - 4 ];

    rcon >>= 1;

    key[ 0 ] ^= s_sbox[ key[ 29 ] ] ^ rcon;
    key[ 1 ] ^= s_sbox[ key[ 30 ] ];
    key[ 2 ] ^= s_sbox[ key[ 31 ] ];
    key[ 3 ] ^= s_sbox[ key[ 28 ] ];
}


/*---------------------------------------------*
 *---------------------------------------------*/

void
AES256_Base::sub_bytes( Byte_Block< 16 > & buf )
{
    for ( byte i = 0; i < 16; ++i )
        buf[ i ] = s_sbox[ buf[ i ] ];
}


/*---------------------------------------------*
 *---------------------------------------------*/

void
AES256_Base::inv_sub_bytes( Byte_Block< 16 > & buf )
{
    for ( byte i = 0; i < 16; ++i )
        buf[ i ] = s_inv_sbox[ buf[ i ] ];
}


/*---------------------------------------------*
 *---------------------------------------------*/

void
AES256_Base::add_round_key( Byte_Block< 16 >       & buf,
                            Byte_Block< 16 > const & key )
{
	buf ^= key;
}


/*---------------------------------------------*
 *---------------------------------------------*/

void
AES256_Base::shift_rows( Byte_Block< 16 > & buf )
{
    byte tmp;

    tmp       = buf[  1 ];
    buf[  1 ] = buf[  5 ];
    buf[  5 ] = buf[  9 ];
    buf[  9 ] = buf[ 13 ];
    buf[ 13 ] = tmp;

    tmp       = buf[ 10 ];
    buf[ 10 ] = buf[  2 ];
    buf[  2 ] = tmp;

    tmp       = buf[  3 ];
    buf[  3 ] = buf[ 15 ];
    buf[ 15 ] = buf[ 11 ];
    buf[ 11 ] = buf[  7 ];
    buf[ 7 ]  = tmp;

    tmp       = buf[ 14 ];
    buf[ 14 ] = buf[  6 ];
    buf[  6 ] = tmp;
}


/*---------------------------------------------*
 *---------------------------------------------*/

void
AES256_Base::inv_shift_rows( Byte_Block< 16 > & buf )
{
    byte tmp;

    tmp       = buf[  1 ];
    buf[  1 ] = buf[ 13 ];
    buf[ 13 ] = buf[  9 ];
    buf[  9 ] = buf[  5 ];
    buf[  5 ] = tmp;

    tmp       = buf[  2 ];
    buf[  2 ] = buf[ 10 ];
    buf[ 10 ] = tmp;

    tmp       = buf[  3 ];
    buf[  3 ] = buf[  7 ];
    buf[  7 ] = buf[ 11 ];
    buf[ 11 ] = buf[ 15 ];
    buf[ 15 ] = tmp;

    tmp       = buf[  6 ];
    buf[  6 ] = buf[ 14 ];
    buf[ 14 ] = tmp;
}


/*---------------------------------------------*
 *---------------------------------------------*/

void
AES256_Base::mix_columns( Byte_Block< 16 > & buf )
{
    byte * b = buf;

    for ( byte i = 0; i < 4; ++i )
    {
        byte tmp[ 4 ];
        std::copy( b, b + 4, tmp );

        byte m = b[ 0 ] ^ b[ 1 ] ^ b[ 2 ] ^ b[ 3 ];

        for ( byte j = 0; j < 4; ++j )
            *b++ ^= m ^ rj_xtime( tmp[ j ] ^ tmp[ ( j + 1 ) & 3 ] );
    }
}

/*---------------------------------------------*
 *---------------------------------------------*/

void
AES256_Base::inv_mix_columns( Byte_Block< 16 > & buf )
{
    byte * b = buf;

    for ( byte i = 0; i < 4; ++i )
    {
        byte tmp[ 4 ];
        std::copy( b, b + 4, tmp );

        byte m1 = b[ 0 ] ^ b[ 1 ] ^ b[ 2 ] ^ b[ 3 ];
        byte m2 = rj_xtime( m1 );

        byte m3[ 2 ];
        m3[ 0 ] = m1 ^ rj_xtime( rj_xtime( m2 ^ tmp[ 0 ] ^ tmp[ 2 ] ) );
        m3[ 1 ] = m1 ^ rj_xtime( rj_xtime( m2 ^ tmp[ 1 ] ^ tmp[ 3 ] ) );

        for ( byte j = 0; j < 4; ++j )
            *b++ ^= m3[ j & 1 ] ^ rj_xtime( tmp[ j ] ^ tmp[ ( j + 1 ) & 3 ] );
    }
}


/*---------------------------------------------*
 *---------------------------------------------*/

inline
AES256_Base::byte
AES256_Base::rj_xtime( byte x ) 
{
    return ( x & 0x80 ) ? ( ( x << 1 ) ^ 0x1b ) : ( x << 1 );
}


/*---------------------------------------------*
 *---------------------------------------------*/

void
AES256_Base::init( Byte_Block< 32 > const & key )
{
	m_enc_key = key;
	m_dec_key = key;

    // Do seven rounds of encryption key expansion to get the decryption
    // key, 'rcon' is left shifted each time

    byte rcon = 1;
    while ( rcon != 0x80 )
        expand_enc_key( m_dec_key, rcon );
}





/*=============================================================
 *=============================================================*/

/*---------------------------------------------*
 * Test main() function:
 *
 * The first test encrypts and decrypts a text,
 * so it's easy to see if things work. To enable
 * it compile with '-DTEST_AES256_BASE'.
 *
 * The second test uses one key and plaintext from
 * the FIPS test suite, so we know what the result
 * of the encryption has to be. To enable it compile
 * with '-DTEST_AES256_BASE_FIPS'.
 *---------------------------------------------*/

#if defined TEST_AES256_BASE

#include <iostream>

int
main( )
{
	std::string key( "0123456789abcdef0123456789abcdef" );
	std::string txt( "This is a test.." );

	AES256_Base aes( key );

	Byte_Block< 16 > block( txt );
	std::cout << aes.decrypt( aes.encrypt( block ) ).as_string( ) << std::endl;

	block = txt = "Short";
	std::cout << aes.decrypt( aes.encrypt( block ) ).as_string( true )
			  << std::endl;

	return 0;
}	

#endif


#if defined TEST_AES256_BASE_FIPS

#include <iostream>
#include <iomanip>

int
main( )
{
    // Data for key

    unsigned char const kd[ ] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

    // Expected result data

    unsigned char const rd[ ] = { 0xdd, 0xc6, 0xbf, 0x79,
                                  0x0c, 0x15, 0x76, 0x0d,
                                  0x8d, 0x9a, 0xeb, 0x6f,
                                  0x9a, 0x75, 0xfd, 0x4e };
    Byte_Block< 32 > key( kd );
    Byte_Block< 16 > txt;

    // Plaintext has 0x80 at the start, the rest is all 0

    txt[ 0 ] = 0x80;
    for ( int i = 1; i < 16; ++i )
        txt[ i ] = 0;

    std::cout << "Plaintext:  ";
    for ( int i = 0; i < 16; ++i )
        std::cout << std::setfill( '0' ) << std::setw( 2 ) << std::hex
                  << static_cast< unsigned int >( txt[ i ] );

    std::cout << std::endl;

	AES256_Base aes( key );

    // Encrypt into a new buffer

    Byte_Block< 16 > res( txt );
    aes.encrypt( res );

    std::cout << "Encryption: ";
    for ( int i = 0; i < 16; ++i )
        std::cout << std::setfill( '0' ) << std::setw( 2 ) << std::hex
                  << static_cast< unsigned int >( res[ i ] );
    std::cout << " -> "
              << ( res == Byte_Block< 16 >( rd ) ? "Success" : "Failure" )
              << std::endl;

    // Decrypt

    aes.decrypt( res );

    std::cout << "Decryption: ";
    for ( int i = 0; i < 16; ++i )
        std::cout << std::setfill( '0' ) << std::setw( 2 ) << std::hex
                  << static_cast< unsigned int >( res[ i ] );
    std::cout << " -> "
              << ( res == txt ? "Success" : "Failure" )
              << std::endl;

	return 0;
}	

#endif


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
