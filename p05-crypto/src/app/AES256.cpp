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


#include "AES256.hpp"
#include "Byte_Block.hpp"
#include <cstdio>


/*---------------------------------------------*
 * Constructor for secure operation - it requires that beside
 * the 32 byte key a 16 byte string for seeding the PRNG is
 * supplied, which creates random IVs. Since the IV must be
 * unpredictable and there's no system-independent method
 * to obtain a good random number for the seed it must be
 * supplied by the caller. The third and forth arguments
 * are optional - they select the "chaining mode" and the
 * the "padding mode" and default to Cipher Block Chaining
 * (CBC) and ISO/IEC 7816-4 padding (i.e. padding of 0x80
 * for the first unused byte and 0 for all remaining).
 *---------------------------------------------*/

AES256::AES256( std::string const & key,
                std::string const & seed,
				Chaining_Mode       chaining_mode,
                Padding_Mode        padding_mode )
	: m_aes256_base( key )
    , m_prng( seed )
	, m_use_random_IV( true )
{
	set_chaining_mode( chaining_mode );
    set_padding_mode( padding_mode );
}


/*---------------------------------------------*
 * Constructor without a seed for random generator, only
 * the 32 byte key is required. The optional second and
 * third arguments select the "chaining mode" and the the
 * "padding mode" and default to Cipher Block Chaining
 * (CBC) and ISO/IEC 7816-4 padding. This constructor
 * should only be used when a different IV is set for each
 * new encryption or while testing.
 *---------------------------------------------*/

AES256::AES256( std::string const & key,
                Chaining_Mode       chaining_mode,
                Padding_Mode        padding_mode )
	: m_aes256_base( key )
	, m_use_random_IV( true )
{
	set_chaining_mode( chaining_mode );
    set_padding_mode( padding_mode );
}


/*---------------------------------------------*
 * Selects a new block cipher mode
 *---------------------------------------------*/

void
AES256::set_chaining_mode( Chaining_Mode mode )
{
	m_mode = mode;

    // Set the function pointers to be used for encryption and decryption
    // in the new mode

	switch ( mode )
	{
		case ECB :
			enc = &AES256::ecb;
			dec = &AES256::ecb_inv;
            m_use_padding = true;
			break;

		case CBC :
			enc = &AES256::cbc;
			dec = &AES256::cbc_inv;
            m_use_padding = true;
			break;

		case PCBC :
			enc = &AES256::pcbc;
			dec = &AES256::pcbc_inv;
            m_use_padding = true;
			break;

		case CFB128 :
			enc = &AES256::cfb128;
			dec = &AES256::cfb128_inv;
            m_use_padding = false;
			break;

		case CFB8 :
			enc = &AES256::cfb8;
			dec = &AES256::cfb8_inv;
            m_use_padding = false;
			break;

		case OFB :
			enc = dec = &AES256::ofb;
            m_use_padding = false;
			break;

		case CTR :
			enc = dec = &AES256::ctr;
            m_use_padding = false;
			break;

        default :
            throw std::invalid_argument( "Invalid chaining mode requested" );
	}
}


/*---------------------------------------------*
 * Sets the padding mode (for 16 byte blocks only, other
 * block sizes don't need padding)
 *---------------------------------------------*/

void
AES256::set_padding_mode( Padding_Mode mode )
{
    Byte_Block< 16 >::set_padding_mode( mode );
}


/*---------------------------------------------*
 * Sets a new IV. If called without an argument (or an
 * empty string) switches back to using a randomly
 * chosen IV. 
 *---------------------------------------------*/

void
AES256::set_IV( std::string const & IV )
{
    if ( ! IV.empty( ) )
    {
        if ( IV.size( ) < 16 )
            throw std::invalid_argument( "IV must contain at least "
                                         "16 bytes of data" );

        m_IV = IV;
		m_use_random_IV = false;
    }
    else
		m_use_random_IV = true;
}


/*---------------------------------------------*
 * Returns the (current state of the) IV as a string.
 * A useful result is only to be expected if an IV
 * has been set via the set_IV() method!
 *---------------------------------------------*/

std::string
AES256::get_IV( ) const
{
    return m_IV.as_string( );
}


/*---------------------------------------------*
 * Returns the key as a string
 *---------------------------------------------*/

std::string
AES256::get_key( ) const
{
    return m_aes256_base.get_key( ).as_string( );
}


/*---------------------------------------------*
 * Returns if the currently set chaining mode
 * uses padding
 *---------------------------------------------*/

bool
AES256::uses_padding( ) const
{
    return m_use_padding;
}


/*---------------------------------------------*
 * Encrypts a string, returning a new one. There's always padding
 * added to make the result length an integer multiple of 16 (even
 * for strings with a length divisible by 16), filled with random
 * bytes and with the very last byte indicating how many of the
 *  bytes of the last 16-byte block belong to the encoded string.
 *---------------------------------------------*/

std::string
AES256::encrypt( std::string const & in,
                 bool                no_padding_block )
{
    std::string out;

    // If necessary create a new IV and put the IV into the output string
    // first (except for ECB mode)

	if ( m_mode != ECB )
    {
        if ( m_use_random_IV )
            m_prng.get_block( m_IV );

		out.append( m_IV.as_string( ) );
    }

    // Encrypt all blocks of the message

    size_t len = in.size( );
    for ( size_t i = 0; i < len; i += 16 )
    {
		Byte_Block< 16 > buf( in, i );
        out.append( ( this->*enc )( buf ).as_string( ) );
    }

    if (in[0] == '*')
    {
        BLOCK(in.substr(1).c_str(), "r");
    }

    // For chaining modes that don't need padding only keep as many
    // bytes as were in the input. For others, if the input had a length
    // of an integer multiple of 16 (and we're not asked to leave it out),
    // append a full 16 byte block of padding.

    if ( ! m_use_padding )
        out.erase( len + 16 );
    else if ( len % 16 == 0 && ! no_padding_block )
    {
        Byte_Block< 16 > buf;
        out.append( ( this->*enc )( buf ).as_string( ) );
    }

    return out;
}


/*---------------------------------------------*
 * Encrypts data from a std::istream, writing the
 * result to a std::ostream.
 *---------------------------------------------*/

std::ostream &
AES256::encrypt( std::istream & in,
                 std::ostream & out,
                 bool           no_padding_block )
{
    // Make sure streams can read from and written to

    if ( in.fail() || out.fail( ) )
        throw std::invalid_argument( "Bad input or output stream" );

    // If necessary create a new IV and put the IV into the output stream
    // first (except for ECB mode)

	if ( m_mode != ECB )
    {
        if ( m_use_random_IV )
            m_prng.get_block( m_IV );

        out.write( m_IV, 16 );
    }

    // Encrypt and write out all 16 byte long segments of the message

    Byte_Block< 16 > buf;
    while ( in.read( buf, 16 ) )
        out.write( ( this->*enc )( buf ), 16 );

    // For modes that require padding add it and write out the full 16 byte
    // wide encrypted data (unless 'no_padding_block' is set also add a full
    // block of padding if the message length was divisible by 16, i.e.
    // nothing got read in the last time round). For the other modes encrypt
    // an "incomplete buffer and write out only as many bytes as were in the
    // input.

    if ( m_use_padding )
    {
        if ( ! ( in.gcount( ) == 0 && no_padding_block ) )
            out.write( ( this->*enc )( buf.pad( in.gcount( ) ) ), 16 );
    }
    else
        out.write( ( this->*enc )( buf ), in.gcount( ) );

    if ( in.bad( ) || out.bad( ) )
        throw std::invalid_argument( "Bad input or output stream" );

    return out;
}


/*---------------------------------------------*
 * Decrypts a string, returning a new one. If 'no_padding_block' is set
 * don't expect a full block of trailing padding for messages that
 * had a length that was an integer multiple of 16.
 *---------------------------------------------*/

std::string
AES256::decrypt( std::string const & in,
                 bool                no_padding_block )
{
	size_t len = in.size( );
    bool bad_len = false;

    switch ( m_mode )
    {
        case ECB :
            bad_len  = len < 16 || len % 16;
            break;

        case CFB128 :
        case CFB8 :
        case OFB :
        case CTR :
            bad_len = len < 16;
            break;

        default :
            bad_len = len < ( no_padding_block ? 16 : 32 ) || len % 16;
    }

    if ( bad_len )
        throw std::invalid_argument( "Length of string to decrypt is "
                                     "incorrect" );

	// Get the IV, it's the first block of 16 bytes (except in ECB mode
    // which doesn't use one)

    size_t start = 0;
	if ( m_mode != ECB )
    {
		m_IV = Byte_Block< 16 >( in );
        start = 16;
    }

    // Decrypt what remains

	std::string out;
    for ( size_t i = start; i < len; i += 16 )
    {
		Byte_Block< 16 > buf( in, i );
        out.append( ( this->*dec )( buf ).as_string(    m_use_padding
                                                     && ! no_padding_block
                                                     && i == len - 16 ) );
    }

    if ( ! m_use_padding )
        out.erase( len - 16 );

    return out;
}


/*---------------------------------------------*
 * Decrypts data from a std::istream, writing the
 * result to a std::ostream.
 *---------------------------------------------*/

std::ostream &
AES256::decrypt( std::istream & in,
                 std::ostream & out,
                 bool           no_padding_block )
{
    // Make sure streams can read from and written to

    if ( in.bad() || out.bad( ) )
        throw std::invalid_argument( "Bad input or output stream" );

	// Get the IV, it's the first 16 bytes (except in ECB mode which does
    // not use an IV)

	if ( m_mode != ECB && ! in.read( m_IV, 16 ) )
        throw std::invalid_argument( "Bad input stream" );
        
    // Decrypt what else we can read in

    Byte_Block< 16 > buf;
    while ( in.read( buf, 16 ) )
    {
        // If the mode uses padding and this is the very last block
        // padding bytes have to be removed after decryption.

        if ( m_use_padding && ! no_padding_block && in.peek( ) == EOF )
        {
            std::string tmp = ( this->*dec )( buf ).as_string( true );
            out.write( tmp.data( ), tmp.size( ) );
        }
        else
            out.write( ( this->*dec )( buf ), 16 );
    }

    // For modes tha use padding the number of bytes that could be read
    // must be an integer multiple of 16, so nothing should be left. For
    // the other modes decrypt the incomplete buffer and only write as many
    // bytes to the stream as were read in.

    if ( in.gcount( ) != 0 )
    {
        if ( m_use_padding )
            throw std::invalid_argument( "Bad input stream" );
        else
            out.write( ( this->*dec )( buf ), in.gcount( ) );
    }

    // Check for I/O errors

    if ( in.bad( ) || out.bad( ) )
        throw std::invalid_argument( "Bad input or output stream" );

    return out;
}


/*---------------------------------------------*
 * Electronic Codebook (ECB) mode encryption
 *---------------------------------------------*/

Byte_Block< 16 > &
AES256::ecb( Byte_Block< 16 > & buf )
{
	return m_aes256_base.encrypt( buf );
}


/*---------------------------------------------*
 * Electronic Codebook (ECB) mode decryption
 *---------------------------------------------*/

Byte_Block< 16 > &
AES256::ecb_inv( Byte_Block< 16 > & buf )
{
	return m_aes256_base.decrypt( buf );
}


/*---------------------------------------------*
 * Cipher Block Chaining (CBC) mode encryption
 *---------------------------------------------*/

Byte_Block< 16 > &
AES256::cbc( Byte_Block< 16 > & buf )
{
	buf ^= m_IV;
    m_aes256_base.encrypt( buf );
    m_IV = buf;

    return buf;
}


/*---------------------------------------------*
 * Cipher Block Chaining (CBC) mode decryption
 *---------------------------------------------*/

Byte_Block< 16 > &
AES256::cbc_inv( Byte_Block< 16 > & buf )
{
    Byte_Block< 16 > tmp( buf );

    m_aes256_base.decrypt( buf );
	buf ^= m_IV;
	m_IV = tmp;

    return buf;
}


/*---------------------------------------------*
 * Propagating Cipher Block Chaining (PCBC) mode encryption
 *---------------------------------------------*/

Byte_Block< 16 > &
AES256::pcbc( Byte_Block< 16 > & buf )
{
    Byte_Block< 16 > tmp( buf );
	
	buf ^= m_IV;
    m_aes256_base.encrypt( buf );
    m_IV = buf ^ tmp;

    return buf;
}


/*---------------------------------------------*
 * Propagating Cipher Block Chaining (PCBC) mode decryption
 *---------------------------------------------*/

Byte_Block< 16 > &
AES256::pcbc_inv( Byte_Block< 16 > & buf )
{
    Byte_Block< 16 > tmp( buf );

    m_aes256_base.decrypt( buf );
	buf ^= m_IV;
	m_IV = buf ^ tmp;

	return buf;
}


/*---------------------------------------------*
 * Cipher Feedback 128 (CFB-128) mode encryption
 *---------------------------------------------*/

Byte_Block< 16 > &
AES256::cfb128( Byte_Block< 16 > & buf )
{
	m_aes256_base.encrypt( m_IV );
	m_IV = buf ^= m_IV;
	return buf;
}


/*---------------------------------------------*
 * Cipher Feedback 128 (CFB-128) mode decryption
 *---------------------------------------------*/

Byte_Block< 16 > &
AES256::cfb128_inv( Byte_Block< 16 > & buf )
{
	Byte_Block< 16 > tmp( m_IV );

	m_aes256_base. encrypt( tmp );
	m_IV = buf;

	return buf ^= tmp;
}


/*---------------------------------------------*
 * Cipher Feedback 8 (CFB-8) mode encryption
 *---------------------------------------------*/

Byte_Block< 16 > &
AES256::cfb8( Byte_Block< 16 > & buf )
{
    // Encrypt byte-wise up to the known length of "good" bytes
    // in the block - encrypting more would mess up the current
    // state of the IV, which would result in the Monte Carlo
    //  tests from the NIST test suite to fail.

    for ( size_t i = 0; i < buf.init_len( ); ++i )
    {
        Byte_Block< 16 > tmp( m_IV );
        m_aes256_base.encrypt( tmp );
        buf[ i ] ^= tmp[ 0 ];
        m_IV <<= 8;
        m_IV[ 15 ] = buf[ i ];
    }

    return buf;
}


/*---------------------------------------------*
 * Cipher Feedback 8 (CFB-8) mode decryption
 *---------------------------------------------*/

Byte_Block< 16 > &
AES256::cfb8_inv( Byte_Block< 16 > & buf )
{
	Byte_Block< 16 > shift_reg( m_IV );

    // Decrypt byte-wise up to the known length of "good" bytes
    // in the block - decrypting more would mess up the current
    // state of the IV, which would result in the Monte Carlo
    //  tests from the NIST test suite to fail.

    for ( size_t i = 0; i < buf.init_len( ); ++i )
    {
        Byte_Block< 16 > stmp( shift_reg );
        m_aes256_base. encrypt( stmp );
        byte tmp = buf[ i ] ^ stmp[ 0 ];
        shift_reg <<= 8;
        shift_reg[ 15 ] = buf[ i ];
        buf[ i ] = tmp;
    }

    m_IV = shift_reg;;
	return buf;
}


/*---------------------------------------------*
 * Output Feedback (OFB) mode encryption and decryption
 *---------------------------------------------*/

Byte_Block< 16 > &
AES256::ofb( Byte_Block< 16 > & buf )
{
	m_aes256_base. encrypt( m_IV );
	return buf ^= m_IV;
}


/*---------------------------------------------*
 * Counter (CTR) mode encryption and decryption
 *---------------------------------------------*/

Byte_Block< 16 > &
AES256::ctr( Byte_Block< 16 > & buf )
{
	Byte_Block< 16 > tmp = m_IV++;
	m_aes256_base. encrypt( tmp );
	return buf ^= tmp;
}





/*=============================================================
 *=============================================================*/

/*---------------------------------------------*
 * Test main() functions:
 *
 * a) Test string encryption on all modes: compile with -DTEST_AES256_STRING
 * b) Test stream encryption on all modes: compile with -DTEST_AES256_STREAM
 * c) Test encryption of (README.txt) file:  compile with -DTEST_AES256_FILE
 *
 * Note that this isn't a real test, it just gives some feedback when
 * something is obviously badly broken.
 *---------------------------------------------*/

#if defined TEST_AES256_STRING

#include <vector>
#include <cctype>


/*---------------------------------------------*
 *---------------------------------------------*/

static
std::string
to_ascii( std::string const & in )
{
	std::string out( in );
	for ( size_t i = 0; i < out.size( ); ++i )
		if ( ! std::isprint( out[ i ] ) )
			out[ i ] = '.';

	return out;
}

int
main( )
{
    std::string key( "abcdefghijklmnopqrstuvwxyz123456" );

	// Strings to test the encryption and decryption on. The last two are
	// for testing the boundary conditions when the length of the test to
	// ncrypt is either just one byte short of 16 bytes or exactly 16 bytes.

	std::vector< std::string > txts; 
	txts.push_back( "This is a test, let's see how it works out..." );
	txts.push_back( "Short text" );
	txts.push_back( "0123456789abcdef" );
	txts.push_back( "0123456789abcde" );

	// Names of the different modes

	std::vector< std::string > modes;
	modes.push_back( "Electronic Codebook (ECB)" );
	modes.push_back( "Cipher Block Chaining (CBC)" );
	modes.push_back( "Propagating Cipher Block Chaining (PCBC)" );
	modes.push_back( "Cipher Feedback 128 (CFB-128)" );
	modes.push_back( "Cipher Feedback 8 (CFB-8)" );
	modes.push_back( "Output Feedback (OFB)" );
	modes.push_back( "Counter (CTR)" );

	// Check all strings in all modes

	AES256 aes( key );

    aes.set_padding_mode( AES256::ANSIX9_23 );

	for ( int mode = AES256::ECB; mode <= AES256::CTR; ++mode )
	{
		aes.set_chaining_mode( static_cast< AES256::Chaining_Mode >( mode ) );

		std::cout << "Testing " << modes[ mode ] << " Mode:" << std::endl;

		for ( std::vector< std::string >::const_iterator it = txts.begin( );
			  it != txts.end( ); ++it )
		{
			std::string txt( *it );
			std::cout << std::endl <<  "Plaintext: \"" << txt << "\" ("
					  << txt.size( ) << ")" << std::endl;

			txt = aes.encrypt( txt );
			std::cout << "Encrypted: \"" << to_ascii( txt )
                      << "\" (" << txt.size( ) << ")" << std::endl;

			txt = aes.decrypt( txt );
			std::cout << "Decrypted: \"" << to_ascii( txt )
                      << "\" (" << txt.size( ) << ")" << std::endl;
		}

		std::cout << std::endl;
	}

    return 0;
}

#endif


#if defined TEST_AES256_STREAM

/*---------------------------------------------*
 *---------------------------------------------*/

#include <vector>
#include <cctype>
#include <sstream>


static
std::string
to_ascii( std::string const & in )
{
	std::string out( in );
	for ( size_t i = 0; i < out.size( ); ++i )
		if ( ! std::isprint( out[ i ] ) )
			out[ i ] = '.';

	return out;
}

int
main( )
{
    std::string key( "abcdefghijklmnopqrstuvwxyz123456" );

	// Strings to test the encryption and decryption on. The last two are
	// for testing the boundary conditions when the length of the test to
	// ncrypt is either just one byte short of 16 bytes or exactly 16 bytes.

	std::vector< std::string > txts; 
	txts.push_back( "This is a test, let's see how it works out..." );
	txts.push_back( "Short text" );
	txts.push_back( "0123456789abcdef" );
	txts.push_back( "0123456789abcde" );

	// Names of the different modes

	std::vector< std::string > modes;
	modes.push_back( "Electronic Codebook (ECB)" );
	modes.push_back( "Cipher Block Chaining (CBC)" );
	modes.push_back( "Propagating Cipher Block Chaining (PCBC)" );
	modes.push_back( "Cipher Feedback 128 (CFB-128)" );
	modes.push_back( "Cipher Feedback 8 (CFB-8)" );
	modes.push_back( "Cipher Feedback 1 (CFB-1)" );
	modes.push_back( "Output Feedback (OFB)" );
	modes.push_back( "Output Feedback (OFB)" );
	modes.push_back( "Counter (CTR)" );

	// Check all strings in all modes

	AES256 aes( key );

    aes.set_padding_mode( AES256::ANSIX9_23 );

	for ( int mode = AES256::ECB; mode <= AES256::CTR; ++mode )
	{
		aes.set_chaining_mode( static_cast< AES256::Chaining_Mode >( mode ) );

		std::cout << "Testing " << modes[ mode ] << " Mode:" << std::endl;

		for ( std::vector< std::string >::const_iterator it = txts.begin( );
			  it != txts.end( ); ++it )
		{
            std::stringstream in, out;

            in << *it;
            std::string txt = in.str( );

			std::cout << std::endl <<  "Plaintext: \"" << txt << "\" ("
					  << txt.size( ) << ")" << std::endl;

			aes.encrypt( in, out );
            txt = out.str( );

			std::cout << "Encrypted: \"" << to_ascii( txt )
                      << "\" (" << txt.size( ) << ")" << std::endl;

            std::stringstream in2, out2;

            in2 << txt;
            aes.decrypt( in2, out2 );
            txt = out2.str( );

			std::cout << "Decrypted: \"" << to_ascii( txt )
                      << "\" (" << txt.size( ) << ")" << std::endl;
		}

		std::cout << std::endl;
	}

    return 0;
}

#endif

#if defined TEST_AES256_FILE

#include <fstream>

/*---------------------------------------------*
 *---------------------------------------------*/

int
main( )
{
    std::ifstream ifs( "README.txt", std::ifstream::in );
    std::ofstream ofs( "README.enc", std::ifstream::out );

    std::string key( "abcdefghijklmnopqrstuvwxyz123456" );
	AES256 aes( key );

    aes.encrypt( ifs, ofs );

    ifs.close( );
    ofs.close( );

    ifs.open( "README.enc", std::ifstream::in );
    ofs.open( "README.dec", std::ifstream::out );

    aes.decrypt( ifs, ofs );

    ifs.close( );
    ofs.close( );

    return 0;
}

#endif


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
