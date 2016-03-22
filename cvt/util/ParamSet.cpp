/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2013, Philipp Heise and Sebastian Klose

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#include <cvt/util/ParamSet.h>

namespace cvt
{
	#define X( TYPE, PTYPE ) sizeof( TYPE ),
	static size_t _PTYPE2SIZE[] =
	{
		#include <cvt/util/internal/ParamTypes.def>
		X( Selection, PTYPE_SELECTION )
	};
	#undef X
	
	#define X( TYPE, PTYPE ) #PTYPE,
	static std::string _PTYPENAMES[] = 
	{
		#include <cvt/util/internal/ParamTypes.def>
		X( Selection, PTYPE_SELECTION )
	};

	ParamSet::ParamSet( ParamInfo** pInfos, size_t n, bool genOffsets ) :
		_parameterMem( 0 ),
		_pInfos( pInfos ),
		_numParameters( n )
	{
		size_t allSize = 0;

		ParamInfo * p;

		if( genOffsets ){			
			for( size_t i = 0; i < _numParameters; i++ ){
				p = pInfos[ i ];
				p->offset = allSize;
				allSize += _PTYPE2SIZE[ p->type ] * p->count;				
			}
		} else {
			// offsets initialized:
			p = pInfos[ n-1 ];
			allSize = p->offset + _PTYPE2SIZE[ p->type ] * p->count;
		}

		_parameterMem = new uint8_t[ allSize ];		
		
		for( size_t i = 0; i < _numParameters; i++ ){
			p = _pInfos[ i ];
			if( p->hasDefault )
				p->setDefaultValue( _parameterMem + p->offset );
		}
	}

	ParamSet::~ParamSet()
	{
		if( _parameterMem )
			delete[] _parameterMem;
	}

	size_t ParamSet::paramHandle( const std::string & name )
	{
		ParamInfo * p;
		for( size_t i = 0; i < _numParameters; i++ ){
			p = _pInfos[ i ];
			if( name == p->name )
				return i;			
		}

		throw CVTException( "Parameter \"" + name + "\" not in parameterset" );
	}

	std::ostream& operator<<( std::ostream& out, const ParamSet& pSet )
	{
		ParamInfo * p; 
		for( size_t i = 0; i < pSet._numParameters; i++ ){
			p = pSet._pInfos[ i ];
			out << "Name: " << p->name << " Type: " << _PTYPENAMES[ p->type ] << " numElements: " << p->count;
			if( p->hasRange ){
				out << " Range available"; 
			}
			if( p->hasDefault ){
				out << " Default value available"; 
			}

			out << std::endl;
		}
		return out;
	}
}
