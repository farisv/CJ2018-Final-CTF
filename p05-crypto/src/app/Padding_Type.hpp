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
#if ! defined Padding_Type_hpp_
#define Padding_Type_hpp_


/*---------------------------------------------*
 * Structure declared only in order to have
 * the exact same enumeration in both the
 * AES256 and the Byte_Block class.
 *---------------------------------------------*/

struct Padding_Type
{
    enum Padding_Mode
    {
        PKCS7,           // all padding bytes contain their number
        ANSIX9_23,       // last byte contains their number, all others 0
        ISO7816_4,       // first padding byte is 0x80, all others 0
        ALL_NULL         // all unused bytes set to 0
    };
};


#endif


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
