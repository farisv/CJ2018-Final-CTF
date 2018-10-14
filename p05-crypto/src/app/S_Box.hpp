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
#if ! defined S_Box_hpp_
#define S_Box_hpp_


// If the the macro 'CALC_RIJNDAEL_SBOX' is defined the S-box (or its
// inverse) is calculated, put into a table and values are then obtained
// from the table. If 'ON_THE_FLY_RIJNDAEL_SBOX' is defined instead the
// values of the S-boxes are always computed when needed. And when the
// macro 'TABLED_RIJNDAEL_SBOX' (or none) is defined a static table is
// used. Obly one (or none) of these macros can be defined at a time.

#if    ( defined CALC_RIJNDAEL_SBOX && defined ON_THE_FLY_RIJNDAEL_SBOX )   \
    || ( defined CALC_RIJNDAEL_SBOX && defined TABLED_RIJNDAEL_SBOX )       \
    || ( defined TABLED_RIJNDAEL_SBOX && defined ON_THE_FLY_RIJNDAEL_SBOX )
#error "Invalid combination of macro settings"
#endif

#if    ! defined CALC_RIJNDAEL_SBOX        \
    && ! defined ON_THE_FLY_RIJNDAEL_SBOX  \
    && ! defined TABLED_RIJNDAEL_SBOX
#define TABLED_RIJNDAEL_SBOX
#endif


// The whole thing only works on machines where a char has 8 bits,
// so make sure it's already detected at compile time when that's
// not the case!

#include <climits>
#if CHAR_BIT != 8
#error "CHAR_BIT isn't 8 which is required for the Byte_Block class"
#endif


typedef unsigned char  byte;

#include <iostream>

/*---------------------------------------------*
 *---------------------------------------------*/

template< bool Inverted >
class S_Box
{
  public :

    // If the table with the S-box values is to be calculated we need to
    // do that in the constructor (but only once since it's a static table).

#if defined CALC_RIJNDAEL_SBOX
    S_Box( )
    {
        if ( ! s_is_initialized )
        {
            byte i = 0;

            do
                s_sbox[ i ] = calc_element( i );
            while ( ++i );

            s_is_initialized = true;
        }
    }
#endif


    // If a table for the S-box values isused  we've just to look up the
    // value in the table, otherwise we've got to calculate it each time.

#if defined TABLED_RIJNDAEL_SBOX || defined CALC_RIJNDAEL_SBOX
    inline
    byte
    operator [ ] ( byte index ) const
    {
        return s_sbox[ index ];
    }
#else
    inline
    byte
    operator [ ] ( byte index ) const
    {
        return calc_element( index );
    }
#endif


  private :

    // Unless we use the pre-set table of the S-box values we need a function
    // for calculating its elements

#if ! defined TABLED_RIJNDAEL_SBOX
    byte
    calc_element( byte x ) const;
#endif

#if defined TABLED_RIJNDAEL_SBOX
    static byte const s_sbox[ ];
#endif

#if defined CALC_RIJNDAEL_SBOX
    static byte s_sbox[ 256 ];
    static bool s_is_initialized;
#endif
};


#if defined CALC_RIJNDAEL_SBOX

template< bool Inverted > byte S_Box< Inverted >::s_sbox[ 256 ];

template< bool Inverted > bool S_Box< Inverted >::s_is_initialized = false;

#endif


#endif


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
