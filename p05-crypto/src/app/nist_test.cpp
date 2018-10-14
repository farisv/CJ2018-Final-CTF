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


#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <string>
#include "AES256.hpp"


/*=============================================*
 * Program for testing the AES256 code with the
 * "Known Answer Test" and "Multi-block Message
 * Test" files distributed by NIST, which can be
 * downloaded from links in the page
 *
 * http://csrc.nist.gov/groups/STM/cavp/
 *
 * Direct links to the ZIP archives containing files
 * are
 *
 *   http://csrc.nist.gov/groups/STM/cavp/documents/aes/KAT_AES.zip
 *   http://csrc.nist.gov/groups/STM/cavp/documents/aes/aesmmt.zip
 *
 * The program expects a single argument, the name
 * of the file with the tests to be read in.
 *=============================================*/


/*---------------------------------------------*
 * Open the RESPONSE file for a "Known Answer Test"
 * (KAT) or "Multi-block Message Test" from the NIST
 * supplied suite of test files for AES256. The file
 * is supposed to have a name starting with the mode
 * (either "ECB", "CBC","CFB128", "CFB8" or "OFB"), and
 * followed by "GFSbox", "KeySbox", "VarKey", "VarTxt"
 * or, for the multi-block message texts, "MMT" and
 * ending in "256.rsp".
 *---------------------------------------------*/

static
bool
open_file( char            const * fn,
           std::ifstream         & ifs,
           AES256::Chaining_Mode & mode )
{
    if ( ! *fn )
    {
        std::cerr << "A file name must be supplied" << std::endl;
        return false;
    }

    size_t i;
    for ( i = strlen( fn ); i > 0; --i )
        if ( fn[ i - 1 ] == '/' )
            break;

    if ( ! fn[ i ] )
    {
        std::cerr << "Invalid file name '" << fn << "'" << std::endl;
        return false;
    }

    int offset = 3;

    if ( ! strncmp( fn + i, "ECB", 3 ) )
        mode = AES256::ECB;
    else if ( ! strncmp( fn + i, "CBC", 3 ) )
        mode = AES256::CBC;
    else if ( ! strncmp( fn + i, "CFB128", 6 ) )
    {
        mode = AES256::CFB128;
        offset = 6;
    }
    else if ( ! strncmp( fn + i, "CFB8", 4 ) )
    {
        mode = AES256::CFB8;
        offset = 4;
    }
    else if ( ! strncmp( fn + i, "CFB1", 4 ) )
    {
        std::cerr << fn << ": CFB1 mode not supported" << std::endl;
        return false;
    }
    else if ( ! strncmp( fn + i, "OFB", 3 ) )
        mode = AES256::OFB;
    else
    {
        std::cerr << "Invalid file name '" << fn << "'" << std::endl;
        return false;
    }

    if (    (    strncmp( fn + i + offset, "GFSbox", 6 )
              && strncmp( fn + i + offset, "KeySbox", 7 )
              && strncmp( fn + i + offset, "VarKey", 6 )
              && strncmp( fn + i + offset, "VarTxt", 6 )
              && strncmp( fn + i + offset, "MMT", 3 ) )
         || strcmp( fn + strlen( fn ) - 7, "256.rsp" ) )
    {
        std::cerr << "Invalid file name' " << fn << "'" << std::endl;
        return false;
    }

    if ( strcmp( fn + strlen( fn ) - 7, "256.rsp" ) )
    {
        std::cerr << "Invalid file name '" << fn << "'" << std::endl;
        return false;
    }

    ifs.open( fn, std::ifstream::in );
    if ( ifs.fail( ) )
    {
        std::cerr << "Failed to open" << fn << std::endl;
        return false;
    }

    return true;
}


/*---------------------------------------------*
 * Converts string with a set of hex numbers represented
 * by chanrs into std::string containing the corresponding
 * values.
 *---------------------------------------------*/

static
std::string
get_byte_string( char const * buf )
{
    std::string res;

    while ( *buf && *buf != '\r' )
    {
        unsigned char x;

        if ( isdigit( *buf ) )
            x = 16 * ( *buf++ - '0' );
        else
            x = 16 * ( *buf++ - 'a' + 10 );

        if ( isdigit( *buf ) )
            x += *buf++ - '0';
        else
            x += *buf++ - 'a' + 10;

        res.push_back( static_cast< char >( x ) );
    }

    return res;
}


/*---------------------------------------------*
 *---------------------------------------------*/

static
bool
test( AES256::Chaining_Mode   mode,
      std::string     const & key,
      std::string     const & iv,
      std::string     const & plaintext,
      std::string     const & ciphertext )
{
    AES256 aes( key, mode );
    aes.set_IV( iv );

    std::string res( aes.encrypt( plaintext, true ) );

    // Remove the IV at the very start

    if ( mode != AES256::ECB )
        res = res.substr( 16 );

    if ( res != ciphertext )
        return false;

    // All modes except ECG need the IV to be prepended.

    if ( mode != AES256::ECB )
        res = iv + ciphertext;
    else
        res = ciphertext;

    return aes.decrypt( res, true ) == plaintext;
}


/*---------------------------------------------*
 *---------------------------------------------*/

int
main( int    argc,
      char * argv[ ] )
{
    if ( argc < 2 )
    {
        std::cerr << "A file name must be supplied" << std::endl;
        return EXIT_FAILURE;
    }

    char const * fn = argv[ 1 ];

    AES256::Chaining_Mode mode;
    std::ifstream ifs;

    if ( ! open_file( fn, ifs, mode ) )
        return EXIT_FAILURE;


    std::string key, iv, plaintext, ciphertext;
    int count = 0;
    char buf[ 1024 ];
    int fail = 0;
    while ( ifs.getline( buf, 1024 ) )
    {
        if ( ! *buf || isspace( *buf ) || *buf == '#' || *buf == '[' )
            continue;

        if ( ! strncmp( buf, "COUNT = ", 8 ) )
        {
            if (    ! key.empty( )
                 || ! iv.empty( )
                 || ! plaintext.empty( )
                 || ! ciphertext.empty( ) )
            {
                std::cerr << "Something's wrong with file '" << fn << "'"
                          << std::endl;
                return EXIT_FAILURE;
            }
            count++;
        }
        else if ( ! strncmp( buf, "KEY = ", 6 ) )
            key = get_byte_string( buf + 6 );
        else if ( ! strncmp( buf, "IV = ", 5 ) )
            iv = get_byte_string( buf + 5 );
        else if ( ! strncmp( buf, "PLAINTEXT = ", 12 ) )
                plaintext = get_byte_string( buf + 12 );
        else if ( ! strncmp( buf, "CIPHERTEXT = ", 13 ) )
                ciphertext = get_byte_string( buf + 13 );
		else
		{
			std::cerr << "Invalid content of file '" << fn << "'" << std::endl;
			return EXIT_FAILURE;
		}

		// When we've found all necessary data run the test

        if (    ! key.empty( )
             && ( mode == AES256::ECB || ! iv.empty( ) )
             && ! plaintext.empty( )
             && ! ciphertext.empty( ) )
        {
            if ( ! test( mode, key, iv, plaintext, ciphertext ) )
                fail += 1;
            key.clear( );
            iv.clear( );
            plaintext.clear( );
            ciphertext.clear( );
        }
    }

    if ( fail )
        std::cout << fn << " => " << fail << " out of " << count
                  << " failed" << std::endl;
    else
        std::cout << fn << " => success" << std::endl;

    return 0;
}


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
