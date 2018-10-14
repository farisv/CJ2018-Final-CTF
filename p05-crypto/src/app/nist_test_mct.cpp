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
#include "AES256_Base.hpp"
#include <vector>


/*=============================================*
 * Program for testing the AES256 code with the
 * "Mone Carlo Test" files distributed by NIST,
 * which can be downloaded from links in the page
 *
 * http://csrc.nist.gov/groups/STM/cavp/
 *
 * Direct link to the ZIP archive containing files
 * is
 *
 *  http://csrc.nist.gov/groups/STM/cavp/documents/aes/aesmct.zip
 *
 * The program expects a single argument, the name
 * of the file with the tests to be read in.
 *
 * The program may seem to bea bit strange when it comes
 * to allocating arrays of lots and lots of strings for
 * intermediate plaintexts and ciphertexts.  There's lots
 * of room for improvements, especially for reducing use
 * of memory! It's done this way for one single reason:
 * it's the nearest one can do to reproduce the pseudo-code
 * given in the NIST publication
 *
 * http://csrc.nist.gov/groups/STM/cavp/documents/aes/AESAVS.pdf
 *
 * describing how to do the tests. And the hope is that by
 * staying as close as possible to the pseudo-code shown
 * there the likelyhood of making mistakes, possibly
 * resulting in false positives, is minimized. And, of
 * course, it makes it simpler to check if the  code
 * used here does what it's supposed to do.
 *=============================================*/



/*---------------------------------------------*
 * Open the RESPONSE file for a Monte Carlo test.
 * The file is supposed to have a name with the
 * pattern "xxxMCT256.rsp", where 'xxx' is the
 * name of "chaining mode (either "ECB", "CBC",
 * "CFB128", "CFB8" or "OFB").
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
        std::cerr << "Invalid file name' " << fn << "'" << std::endl;
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
        std::cerr << "Invalid file name' " << fn << "'" << std::endl;
        return false;
    }

	if (    strncmp( fn + i + offset, "MCT", 3 )
		 || strcmp( fn + strlen( fn ) - 7, "256.rsp" ) )
    {
        std::cerr << "Invalid file name' " << fn << "'" << std::endl;
        return false;
    }

    ifs.open( fn, std::ifstream::in );
    if ( ifs.fail( ) )
    {
        std::cerr << "Failed to open file '" << fn << "'" << std::endl;
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
 * Remove trailing padding from a string freshly encrypted.
 *---------------------------------------------*/

static
std::string
remove_IV( std::string const & in )
{
    return in.substr( 16 );
}


/*---------------------------------------------*
 * Run encryption test for CBC mode - see NIST's AESAVS
 * documentation for details.
 *---------------------------------------------*/

static
bool
test_cbc_enc ( std::string const & key,
               std::string const & iv,
               std::string const & plaintext,
               std::string const & ciphertext )
{
    AES256 aes( key, AES256::CBC );
    aes.set_IV( iv );

    std::vector< std::string > CT;
    std::vector< std::string > PT;

    PT.push_back( plaintext );

    for ( int i = 0; i < 1000; ++i )
    {
        CT.push_back( remove_IV( aes.encrypt( PT.at( i ), true ) ) );

        if ( i == 0 )
            PT.push_back( iv );
        else
            PT.push_back( CT.at( i - 1 ) );
    }

    return CT.back( ) == ciphertext;
}


/*---------------------------------------------*
 * Run decryption test for CBC mode - see NIST's AESAVS
 * documentation for details.
 *---------------------------------------------*/

static
bool
test_cbc_dec ( std::string const & key,
               std::string const & iv,
               std::string const & plaintext,
               std::string const & ciphertext )
{
    AES256 aes( key, AES256::CBC );
    aes.set_IV( iv );

    std::vector< std::string > CT;
    std::vector< std::string > PT;

    PT.push_back( ciphertext );

    for ( int i = 0; i < 1000; ++i )
    {
        CT.push_back( aes.decrypt( aes.get_IV( ) + PT.at( i ), true ) );

        if ( i == 0 )
            PT.push_back( iv );
        else
            PT.push_back( CT.at( i - 1 ) );
    }

    return CT.back( ) == plaintext;
}


/*---------------------------------------------*
 * Run encryption test for CFB8 mode - see NIST's AESAVS
 * documentation for details.
 *---------------------------------------------*/

static
bool
test_cfb8_enc( std::string const & key,
               std::string const & iv,
               std::string const & plaintext,
               std::string const & ciphertext )
{
    AES256 aes( key, AES256::CFB8 );
    aes.set_IV( iv );

    std::vector< std::string > CT;
    std::vector< std::string > PT;

    PT.push_back( plaintext );

    for ( int i = 0; i < 1000; ++i )
    {
        CT.push_back( remove_IV( aes.encrypt( PT[ i ] ) ) );

        if ( i < 16 )
            PT.push_back( iv.substr( i, 1 ) );
        else
            PT.push_back( CT.at( i - 16 ) );
    }

    return CT.back( ) == ciphertext;
}


/*---------------------------------------------*
 * Run decryption test for CFB8 mode - see NIST's AESAVS
 * documentation for details.
 *---------------------------------------------*/

static
bool
test_cfb8_dec( std::string const & key,
               std::string const & iv,
               std::string const & plaintext,
               std::string const & ciphertext )
{
    AES256 aes( key, AES256::CFB8 );
    aes.set_IV( iv );

    std::vector< std::string > CT;
    std::vector< std::string > PT;

    PT.push_back( ciphertext );

    for ( int i = 0; i < 1000; ++i )
    {
        CT.push_back( aes.decrypt( aes.get_IV( ) + PT[ i ], true ) );

        if ( i < 16 )
            PT.push_back( iv.substr( i, 1 ) );
        else
            PT.push_back( CT.at( i - 16 ) );
    }

    return CT.back( ) == plaintext;
}


/*---------------------------------------------*
 * Run encryption test for CFB128 mode - see NIST's AESAVS
 * documentation for details.
 *---------------------------------------------*/

static
bool
test_cfb128_enc( std::string const & key,
                 std::string const & iv,
                 std::string const & plaintext,
                 std::string const & ciphertext )
{
    AES256 aes( key, AES256::CFB128 );
    aes.set_IV( iv );

	std::vector< std::string > PT;
	std::vector< std::string > CT;

	PT.push_back( plaintext );

	for ( int i = 0; i < 1000; ++i )
	{
		CT.push_back( remove_IV( aes.encrypt( PT[ i ] ) ) );

		if ( i == 0 )
			PT.push_back( iv );
		else
			PT.push_back( CT[ i - 1 ] );
	}

	return CT.back( ) == ciphertext;
}


/*---------------------------------------------*
 * Run decryption test for CFB128 mode - see NIST's AESAVS
 * documentation for details.
 *---------------------------------------------*/

static
bool
test_cfb128_dec( std::string const & key,
                 std::string const & iv,
                 std::string const & plaintext,
                 std::string const & ciphertext )
{
    AES256 aes( key, AES256::CFB128 );
    aes.set_IV( iv );

	std::vector< std::string > PT;
	std::vector< std::string > CT;

	PT.push_back( ciphertext );

	for ( int i = 0; i < 1000; ++i )
	{
		CT.push_back( aes.decrypt( aes.get_IV( ) + PT[ i ], true ) );

		if ( i == 0 )
			PT.push_back( iv );
		else
			PT.push_back( CT[ i - 1 ] );
	}

	return CT.back( ) == plaintext;
}


/*---------------------------------------------*
 * Run encryption test for ECB mode - see NIST's AESAVS
 * documentation for details.
 *---------------------------------------------*/

static
bool
test_ecb_enc( std::string const & key,
              std::string const &,
              std::string const & plaintext,
              std::string const & ciphertext )
{
    AES256 aes( key, AES256::ECB );

    std::vector< std::string > PT;
    std::vector< std::string > CT;

    PT.push_back( plaintext );

    for ( int i = 0; i < 1000; ++i )
    {
        CT.push_back( aes.encrypt( PT.at( i ), true ) );
		PT.push_back( CT.back( ) );
	}

	return CT.back( ) == ciphertext;
}


/*---------------------------------------------*
 * Run decryption test for ECB mode - see NIST's AESAVS
 * documentation for details.
 *---------------------------------------------*/

static
bool
test_ecb_dec( std::string const & key,
              std::string const &,
              std::string const & plaintext,
              std::string const & ciphertext )
{
    AES256 aes( key, AES256::ECB );

    std::vector< std::string > PT;
    std::vector< std::string > CT;

    PT.push_back( ciphertext );

    for ( int i = 0; i < 1000; ++i )
    {
        CT.push_back( aes.decrypt( PT[ i ], true ) );
		PT.push_back( CT.back( ) );
	}

	return CT.back( ) == plaintext;
}


/*---------------------------------------------*
 * Run encryption test for OFB mode - see NIST's AESAVS
 * documentation for details.
 *---------------------------------------------*/

static
bool
test_ofb_enc( std::string const & key,
              std::string const & iv,
              std::string const & plaintext,
              std::string const & ciphertext )
{
    AES256 aes( key, AES256::OFB );
    aes.set_IV( iv );

	std::vector< std::string > PT;
	std::vector< std::string > CT;

	PT.push_back( plaintext );

	for ( size_t i = 0; i < 1000; ++i )
	{
		CT.push_back( remove_IV( aes.encrypt( PT[ i ] ) ) );

		if ( i == 0 )
			PT.push_back( iv );
		else
			PT.push_back( CT[ i - 1 ] );
	}

	return CT.back( ) == ciphertext;
}


/*---------------------------------------------*
 * Run decryption test for OFB mode - see NIST's AESAVS
 * documentation for details.
 *---------------------------------------------*/

static
bool
test_ofb_dec( std::string const & key,
              std::string const & iv,
              std::string const & plaintext,
              std::string const & ciphertext )
{
    AES256 aes( key, AES256::OFB );
    aes.set_IV( iv );

	std::vector< std::string > PT;
	std::vector< std::string > CT;

	PT.push_back( ciphertext );

	for ( size_t i = 0; i < 1000; ++i )
	{
		CT.push_back( aes.decrypt( aes.get_IV( ) + PT[ i ], true ) );

		if ( i == 0 )
			PT.push_back( iv );
		else
			PT.push_back( CT[ i - 1 ] );
	}

	return CT.back( ) == plaintext;
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

	// Pointers to the diverse test functions

    bool ( * tf_enc )( std::string const &,
                       std::string const &,
                       std::string const &,
                       std::string const & );
    bool ( * tf_dec )( std::string const &,
                       std::string const &,
                       std::string const &,
                       std::string const & );

    switch ( mode )
    {
        case AES256::CBC :
            tf_enc = test_cbc_enc;
            tf_dec = test_cbc_dec;
            break;

        case AES256::CFB128 :
            tf_enc = test_cfb128_enc;
            tf_dec = test_cfb128_dec;
            break;

        case AES256::CFB8 :
            tf_enc = test_cfb8_enc;
            tf_dec = test_cfb8_dec;
            break;

        case AES256::ECB :
            tf_enc = test_ecb_enc;
            tf_dec = test_ecb_dec;
            break;

        case AES256::OFB :
            tf_enc = test_ofb_enc;
            tf_dec = test_ofb_dec;
            break;

        default :
            std::cerr << "Something's wrong here" << std::endl;
            return EXIT_FAILURE;
    }

    std::string key, iv, plaintext, ciphertext;
    int count = 0;
    char buf[ 1024 ];
    int fail = 0;
    bool do_encrypt = true;

    while ( ifs.getline( buf, 1024 ) )
    {
        if ( ! *buf || isspace( *buf ) || *buf == '#' )
            continue;

        if ( ! strncmp( buf, "[ENCRYPT]", 9 ) )
            do_encrypt = true;
        else if ( ! strncmp( buf, "[DECRYPT]", 9 ) )
            do_encrypt = false;
        else if ( ! strncmp( buf, "COUNT = ", 8 ) )
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
            if (    (      do_encrypt
                      && ! tf_enc( key, iv, plaintext, ciphertext ) )
                 || (    ! do_encrypt
                      && ! tf_dec( key, iv, plaintext, ciphertext ) ) )
                fail += 1;
            key.clear( );
            iv.clear( );
            plaintext.clear( );
            ciphertext.clear( );
        }
    }

    if ( ! fail )
        std::cout << fn << " => success" << std::endl;
    else
        std::cout << fn << " => " << fail << " out of " << count
                  << " failed" << std::endl;

    return 0;
}


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
