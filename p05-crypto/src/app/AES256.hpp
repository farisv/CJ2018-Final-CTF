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
#if ! defined AES256_hpp_
#define AES256_hpp_


#include "AES256_Base.hpp"
#include "AES256_PRNG.hpp"
#include "Padding_Type.hpp"
#include <iostream>
#include <stdexcept>


template< size_t N > class Byte_Block;


/*---------------------------------------------*
 * Class implementing the encryption and decryption of arbitrary
 * long blocks of data, in the form of C++ std::strings (which
 * can hold binary data).
 *
 * Several different block chaining modes are implemented,
 * currently these are ECB, CBC, PCBC, CFB-128. CFB-8, OFB
 * and CTR. The default, when none is explicitly selected,
 * is CBC.
 *
 * Also different padding modes can be selected (if the
 * chaining mode requires padding) can be selected. Per
 * default, messages with lengths divisible by 16 are
 * (for the modes requiring padding) padded with a full
 * 16 byte long block.
 *
 * Except in ECB mode, which doesn't use an initialization
 * vector the (unencrpted) IV is prepended to the encrypted
 * data.
 *---------------------------------------------*/


class AES256 : public Padding_Type
{
  public :

    typedef unsigned char byte;

	enum Chaining_Mode
	{
		ECB,
		CBC,
		PCBC,
		CFB,
        CFB128 = CFB,
        CFB8,
		OFB,
		CTR
	};

    using Padding_Type::Padding_Mode;

    // Construtor, requires a std::string with a 32-byte key and,
    // for secure operation, an (at least) 16 byte string to seed
    // the random generator used for creating a secure IV etc. It
    // also accepts a block cipher mode (which defaults to CBC)

    AES256( std::string const & key,
            std::string const & seed,
            Chaining_Mode       chaining_mode = CBC,
            Padding_Mode        padding_mode = ISO7816_4 );

    // Constructor without seed for random generator, should only
    // be used when a new IV is set before each encryption or for
    // testing purposes.

    AES256( std::string const & key,
            Chaining_Mode       chaining_mode = CBC,
            Padding_Mode        padding_mode = ISO7816_4 );

    // Selects the chaining mode

	void
	set_chaining_mode( Chaining_Mode mode );

    // Selects the padding mode

    void
    set_padding_mode( Padding_Mode mode );

    // Allows to set am  initialization vector. When called with
    // an empty string (or no argument) it switches back to usind
    // a new, randomly chosen IV for each encryption.

    void
    set_IV( std::string const & IV = std::string( ) );


    // Returns the (current state of the ) IV - only use this if an IV
    // has been set via set_IV()

    std::string
    get_IV( ) const;


    // Returns the key

    std::string
    get_key( ) const;


    // Returns if current chaining mode uses padding

    bool
    uses_padding( ) const;


    // Encrypts a std::string (returned as a new string). Per default for
    // modes with padding a complete block of padding is added if the
    // message length is divisible by 16 - switch this off by setting
    // 'no_padding_block'.

    std::string
    encrypt( std::string const & in,
             bool                no_padding_block = false );

    // Encrypts a std::istream, outputs to a std::ostream

    std::ostream &
    encrypt( std::istream & in,
             std::ostream & out,
             bool           no_padding_block = false );

    // Decrypts an encrypted std::string (returned as a new string)
    // If 'no_padding_block' is set the method expects that a full
    // block of padding (for messages that originally had a length
    // divisible by 16) is NOT present.

    std::string
    decrypt( std::string const & in,
             bool                no_padding_block = false );

    // Decrypts a std::istream, outputs to a std::ostream

    std::ostream &
    decrypt( std::istream & in,
             std::ostream & out,
             bool           no_padding_block = false );

  private :

    // ECB mode encryption method

    Byte_Block< 16 > &
    ecb( Byte_Block< 16 > & buf );

    // ECB mode decryption method

    Byte_Block< 16 > &
    ecb_inv( Byte_Block< 16 > & buf );

    // CBC mode encryption method

    Byte_Block< 16 > &
    cbc( Byte_Block< 16 > & buf );

    // CBC mode decryption method

    Byte_Block< 16 > &
    cbc_inv( Byte_Block< 16 > & buf );

    // PCBC mode encryption method

    Byte_Block< 16 > &
    pcbc( Byte_Block< 16 > & buf );

    // PCBC mode decryption method

	Byte_Block< 16 > &
	pcbc_inv( Byte_Block< 16 > & buf );

    // CFB128 mode encryption method

    Byte_Block< 16 > &
    cfb128( Byte_Block< 16 > & buf );

    // CFB128 mode decryption method

	Byte_Block< 16 > &
	cfb128_inv( Byte_Block< 16 > & buf );

    // CFB-8 mode encryption method

    Byte_Block< 16 > &
    cfb8( Byte_Block< 16 > & buf );

    // CFB-8 mode decryption method

	Byte_Block< 16 > &
	cfb8_inv( Byte_Block< 16 > & buf );

    // OFB mode encryption and decryption method

    Byte_Block< 16 > &
    ofb( Byte_Block< 16 > & buf );

    // CTR mode encryption and decryption method

    Byte_Block< 16 > &
    ctr( Byte_Block< 16 > & buf );


    // Current IV

	Byte_Block< 16 > m_IV;

    // Basic encryption class instance

	AES256_Base m_aes256_base;

    // Current block cipher mode

	Chaining_Mode m_mode;

    // Pointers to functions to be used in current block cipher mode

	Byte_Block< 16 > & ( AES256::* enc )( Byte_Block< 16 > & );
	Byte_Block< 16 > & ( AES256::* dec )( Byte_Block< 16 > & );

    // Pseudo-random generator to be used for generating IVs

    AES256_PRNG m_prng;

    // Set when a new, randomly generated IV is to be used for each encryption

	bool m_use_random_IV;

    // Set for modes that require padding

    bool m_use_padding;
};


#endif


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
