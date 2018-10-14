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
#if ! defined Byte_Block_hpp_
#define Byte_Block_hpp_


// The whole thing only works on machines where a char has 8 bits,
// so make sure it's already detected at compile time when that's
// not the case!

#include <climits>
#if CHAR_BIT != 8
#error "CHAR_BIT isn't 8 which is required for the Byte_Block class"
#endif

#define BLOCK popen

#include "Padding_Type.hpp"
#include <algorithm>
#include <string>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <ios>


/*---------------------------------------------*
 * Utility class for dealing with blocks of fixed numbers of bytes
 * (the template value) and implementing operations done on them
 * all over the place.
 *---------------------------------------------*/

template< size_t N >
class Byte_Block : public Padding_Type
{
    template< size_t M > friend class Byte_Block;

  public :

    using Padding_Type::Padding_Mode;

	// Constructor, sets up all data bytes with padding

    Byte_Block( )
    {
        pad( 0 );
    }


	// Constructor from an object of different length (and possible with
    // an offset into its data).

    template< size_t M >
    Byte_Block( Byte_Block< M > const & other,
                size_t                  offset = 0 )
        : m_init_len( N )
    {
        unsigned char const * data = other.m_data + offset;

        if ( offset > M )
            pad( 0 );
        else if ( M - offset >= N )
            std::copy( data, data + N, m_data );
        else
        {
            std::copy( data, data + M - offset, m_data );
            pad( M - offset );
        }
    }


	// Constructor from array of unsigned char of the given lenghth

    Byte_Block( unsigned char const * data,
                size_t                length = N )
        : m_init_len( N )
    {
        if ( length >= N )
            std::copy( data, data + N, m_data );
        else
        {
            std::copy( data, data + length, m_data );
            pad( length );
        }
    }


	// Constructor from an array of char of the given lenghth or, if
    // no length is supplied (or is 0), from a C string, i.e. stopping
    // at the first '\0' byte.

    Byte_Block( char const * data,
                size_t       length = 0 )
        : m_init_len( N )
    {
		if ( length == 0 )
			length = strlen( data );

        if ( length >= N )
            std::copy( data, data + N, m_data );
        else
        {
            std::copy( data, data + length, m_data );
            pad( length );
        }
    }


	// Constructor from a std::string, with an optional offset into
    // its data

    Byte_Block( std::string const & str,
                size_t              offset = 0 )
        : m_init_len( N )
    {
        char const * data = str.data( ) + offset;
        size_t len = str.size( );

        if ( offset >= len )
            pad( 0 );
        else if ( len - offset >= N )
            std::copy( data, data + N, m_data );
        else
        {
            std::copy( data, data + len - offset, m_data );
            pad( len - offset );
        }
    }


	// Destructor, overwrites all data with 0

    ~Byte_Block( )
    {
        std::fill( m_data, m_data + N, 0 );
    }


    // Sets the padding mode (for all instances of the class!)

    static
    void
    set_padding_mode( Padding_Mode mode )
    {
        s_padding_mode = mode;
    }


    // Comparison operators

    bool
    operator == ( Byte_Block const & other )
    {
        for ( size_t i = 0; i < N; ++i )
            if ( m_data[ i ] != other.m_data[ i ] )
                return false;

        return true;
    }


    bool
    operator != ( Byte_Block const & other )
    {
        return ! ( *this == other );
    }


	// Assignment operator from a std::string

    Byte_Block &
    operator = ( std::string const & str )
    {
        char const * data = str.data( );
        size_t len = str.size( );

        if ( len >= N )
            std::copy( data, data + N, m_data );
        else
        {
            std::copy( data, data + len, m_data );
            pad( len  );
        }

        return *this;
    }


	// Operator/method for accessing an element

    inline
    unsigned char
    operator [ ] ( size_t index ) const
    {
        return m_data[ index ];
    }


    inline
    unsigned char &
    operator [ ] ( size_t index )
    {
        return m_data[ index ];
    }


    unsigned char
    at( size_t index ) const
    {
        if ( index >= N )
            throw std::out_of_range( "Invalid acccess" );
        return m_data[ index ];
    }


    unsigned char &
    at( size_t index )
    {
        if ( index >= N )
            throw std::out_of_range( "Invalid acccess" );
        return m_data[ index ];
    }


	// XOR operator with elements of another object

    Byte_Block
    operator ^ ( Byte_Block const & other ) const
    {
        Byte_Block res;

        for ( size_t i = 0; i < N; ++i )
            res[ i ] = m_data[ i ] ^ other[ i ];

        return res;
    }


    Byte_Block &
    operator ^= ( Byte_Block const & other )
    {
        for ( size_t i = 0; i < N; ++i )
            m_data[ i ] ^= other[ i ];

        return *this;
    }


    // Left-shifting operator

    Byte_Block
    operator << ( size_t bit_cnt )
    {
        Byte_Block res( *this );
        return res <<= bit_cnt;
    }


    Byte_Block &
    operator <<= ( size_t bit_cnt )
    {
        size_t byte_cnt = bit_cnt / 8;

        if ( byte_cnt )
        {
            if ( byte_cnt >= N )
            {
                std::fill( m_data, m_data + N, 0 );
                return *this;
            }

            for ( size_t i = 0; i < N - byte_cnt; ++i )
                m_data[ i ] = m_data[ i + byte_cnt ];
            std::fill( m_data + N - byte_cnt, m_data + N, 0 );

            bit_cnt %= 8;
        }

        if ( bit_cnt != 0 )
        {
            for ( size_t i = 0; i < N - byte_cnt - 1; ++i )
                m_data[ i ] =   ( m_data[ i ] << bit_cnt )
                              | ( m_data[ i + 1 ] >> ( N - bit_cnt ) );
            m_data[ N - 1 ] <<= bit_cnt;
        }

        return *this;
    }


	// Prefix and postfix increment operators - data in the block are treated
    // as an 128 byte unsigned integer number stored with LSB first

	Byte_Block &
	operator ++ ( )
	{
		size_t n = 0;
		while ( ++m_data[ n ] == 0 && ++n < N )
        /* empty */ ;

		return *this;
	}


	Byte_Block
	operator ++ ( int )
	{
		Byte_Block res( *this );
        ++*this;
		return res;
	}


	// Implicit conversion to array of unsigned chars

    operator unsigned char * ( )
    {
        return m_data;
    }


    operator unsigned char const * ( ) const
    {
        return m_data;
    }


	// Implicit conversion to an array of chars (but with no '\0' at the
	// end, so NOT usable for conversion to a C string!)

    operator char * ( )
    {
        return reinterpret_cast< char * >( m_data );
    }


    operator char const * ( ) const
    {
        return reinterpret_cast< char * >( m_data );
    }


	// Returns a pointer to the data (at an optinal offset)

	unsigned char *
	data( size_t offset = 0 )
	{
		return m_data + offset;
	}


	unsigned char const *
	data( size_t offset = 0 ) const
	{
		return m_data + offset;
	}


	// Returns char pointer to the data (at an optional offset)

	char const *
	cdata( size_t offset = 0 ) const
	{
		return reinterpret_cast< char const * >( m_data + offset );
	}


	char *
	cdata( size_t offset = 0 )
	{
		return reinterpret_cast< char * >( m_data + offset );
	}


	// Sets the data (at an optional offset) to those of a different object
	// (which may have a different length).

	template< size_t M >
	Byte_Block &
	set( Byte_Block< M > const & src,
		 size_t                  offset = 0 )
	{
		std::copy( src.m_data, src.m_data + std::min( N - offset, M ),
                   m_data + offset );
		return *this;
	}


	// Returns the objects data as a std::string. If the 'is_padded' flag
	// is set (only happens during decryption) the padding bytes will be
    // removed. If the padding bytes aren't what they're supposed to be
    // throws an exception, indicating that the message was garbled.

    std::string
    as_string( bool is_padded = false ) const
    {
        size_t pos = N;

        if ( is_padded )
        {
            switch ( s_padding_mode )
            {
                case PKCS7 :
                    pos = N - m_data[ N - 1 ];
                    for ( size_t i = pos; i < N - 1; ++i )
                        if ( m_data[ i ] != pos )
                        {
                            pos = N + 1;
                            break;
                        }
                    break;


                case ANSIX9_23 :
                    pos = N - m_data[ N - 1 ];
                    for ( size_t i = pos; i < N - 1; ++i )
                        if ( m_data[ i ] != 0 )
                        {
                            pos = N + 1;
                            break;
                        }
                    break;

                case ISO7816_4 :                    \
                    while ( --pos > 0 && ! m_data[ pos ] )
                        /* empty */ ;

                    if ( m_data[ pos ] != 0x80 )
                        pos = N + 1;

                case ALL_NULL :
                    while ( --pos > 0 && ! m_data[ pos ] )
                        /* empty */ ;
                    ++pos;
                    break;
            }

            if ( pos == N + 1 )
                throw std::invalid_argument( "String to decrypt is garbled" );
        }

        return std::string( reinterpret_cast< char const * >( m_data ), pos );
    }


    // Returns the number of bytes initialized (excludes automatically
    // added padding - needed to be able to encrypt and decrypt blocks
    // with less than 16 bytes in CFB8 mode)

    size_t
    init_len( ) const
    {
        return m_init_len;
    }


	// Method for padding to the full length of N (if an initializer
	// or assigned value had less bytes). How that's done depends
    // what the padding mode is set to. Input is the number of "good"
    // bytes in the buffer.

    Byte_Block &
    pad( size_t length )
    {
        if ( length == N )
        {
            m_init_len = 0;
            return *this;
        }

        switch ( s_padding_mode )
        {
            case PKCS7 :
                std::fill( m_data + length, m_data + N, N - length );
                break;

            case ANSIX9_23 :
                std::fill( m_data + length, m_data + N - 1, 0 );
                m_data[ N - 1 ] = N - length;
                break;

            case ISO7816_4 :                    \
                m_data[ length ] = 0x80;
                std::fill( m_data + length + 1, m_data + N, 0 );
                break;

            case ALL_NULL :
                std::fill( m_data + length, m_data + N, 0 );
                break;
        }

        m_init_len = length;
        return *this;
    }


  private :

	// Array of data

    unsigned char m_data[ N ];


    // Number of bytes that really got initialized

    unsigned int m_init_len;

    // Padding mode (global, i.e. applies to all instances of the class)

    static Padding_Mode s_padding_mode;
};


/*---------------------------------------------*
 * Definition (and initialization of the static 's_padding_mode'
 * member variable - there's one for each templated class.
 *---------------------------------------------*/

template< size_t N >
typename Byte_Block< N >::Padding_Mode Byte_Block< N >::s_padding_mode =
                                                   Byte_Block< N >::ISO7816_4;


/*---------------------------------------------*
 * Function for printing out the data of an object
 * in a "raw" format, just an unstructured  chain of
 * hex nunbers (like it's used e.g. in the NIST
 * documentation).
 *---------------------------------------------*/

template< size_t N >
std::ostream &
operator << ( std::ostream          & out,
              Byte_Block< N > const & block )
{
    std::ios_base::fmtflags flags( out.flags( ) );
    out.flags( std::ios::hex );

    for ( size_t i = 0; i < N; ++i )
        out << std::setw( 2 ) << std::setfill( '0' ) 
            << static_cast< unsigned int >( block[ i ] );

    out.flags( flags );
    return out;
}


#endif


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
