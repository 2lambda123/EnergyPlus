// Numeric Support Functions
//
// Project: Objexx Fortran-C++ Library (ObjexxFCL)
//
// Version: 4.2.0
//
// Language: C++
//
// Copyright (c) 2000-2017 Objexx Engineering, Inc. All Rights Reserved.
// Use of this source code or any derivative of it is restricted by license.
// Licensing is available from Objexx Engineering, Inc.:  http://objexx.com

// ObjexxFCL Headers
#include <ObjexxFCL/numeric.hh>

// C++ Headers
#include <cstring>

namespace ObjexxFCL {

std::size_t
SIZEOF( char const * x )
{
	return std::strlen( x );
}

} // ObjexxFCL
