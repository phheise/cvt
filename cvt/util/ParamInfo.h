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

#ifndef CVT_PARAMINFO_H
#define CVT_PARAMINFO_H

#include <stdint.h>
#include <string>
#include <cvt/gfx/Image.h>
#include <cvt/math/Vector.h>
#include <cvt/math/Matrix.h>
#include <cvt/gfx/Color.h>

namespace cvt
{
	/* Special handled Type */
	struct Selection {
		Selection( size_t s ) : value( s ){}
		Selection( const Selection & s ) : value( s.value ){}
		bool operator<( const Selection & other ) const { return value < other.value; }
		bool operator>( const Selection & other ) const { return value > other.value; }
		size_t value;		
	};

#define X( TYPE, PTYPE ) PTYPE,
	enum ParamType
	{
		#include <cvt/util/internal/ParamTypes.def>
		PTYPE_SELECTION // param types that need special handling need manual editing
	};
#undef X

	template<class T> struct PTypeCheck;

	#define X( TYPE, PTYPE ) template<> struct PTypeCheck<TYPE>{ static const ParamType paramType = PTYPE; static bool check( ParamType other ) { if( other != paramType ) return false; return true; } };
		#include <cvt/util/internal/ParamTypes.def>
		X( Selection, PTYPE_SELECTION )
	#undef X
	
	class ParamInfo
	{		
		public:
			virtual ~ParamInfo(){}

                        /**
                         *  \brief descriptive type
                         */
			ParamType	type;			

                        /**
                         *  \brief descriptive name
                         */
			std::string	name;

			/* count times this type */
			size_t		count;

			/* offset in parameter set */
			size_t		offset;
		
			const bool isInput;
			const bool hasDefault;
			const bool hasRange;
		
			virtual void setDefaultValue( void * ) const = 0;
			virtual const std::string typeString() const = 0;
		
		protected:
			ParamInfo( ParamType t, const std::string & n, size_t c, 
					   size_t o, bool input, bool _hasDefault = false, bool _hasRange = false ) :
				type( t ), name( n ), count( c ), offset( o ), 
				isInput( input ), hasDefault( _hasDefault ), hasRange( _hasRange )
			{}
	};


	inline std::ostream& operator<<( std::ostream& out, const ParamInfo& filter )
	{
		out << "Name: " << filter.name << " Type: " << filter.typeString() << ( filter.isInput ?" ( in )":" ( out )" );
		return out;
	}
	
	template<class T> class ParamInfoTyped;

#define X( TYPE, PTYPE ) \
	template<> class ParamInfoTyped<TYPE> : public ParamInfo \
	{\
		public:\
			ParamInfoTyped( const std::string & n, bool input = true, size_t c = 1, size_t o = 0 ) :\
			ParamInfo( PTYPE, n, c, o, input ) {}\
			/* min / max / default */ \
			ParamInfoTyped( const std::string & n, TYPE min, TYPE max, TYPE _defaultValue, bool input = true, size_t c = 1, size_t o = 0 ) :\
			ParamInfo( PTYPE, n, c, o, input, true, true ), min( min ), max( max ), defaultVal( _defaultValue ) {}\
			/* default */ \
			ParamInfoTyped( const std::string & n, TYPE _defaultValue, bool input = true, size_t c = 1, size_t o = 0 ) :\
			ParamInfo( PTYPE, n, c, o, input, true, false ), defaultVal( _defaultValue ) {}\
			/* min / max */ \
			ParamInfoTyped( const std::string & n, TYPE min, TYPE max, bool input = true, size_t c = 1, size_t o = 0 ) :\
			ParamInfo( PTYPE, n, c, o, input, false, true ), min( min ), max( max ) {}\
			virtual ~ParamInfoTyped() {}\
			TYPE minValue() const { return min; }\
			TYPE maxValue() const { return max; }\
			TYPE defaultValue() const { return defaultVal; }\
			virtual void setDefaultValue( void * ptr ) const { *( ( TYPE* )ptr ) = defaultVal; }\
			\
			TYPE min; TYPE max; TYPE defaultVal;\
			const std::string typeString() const { return #TYPE; } \
	};
	
	#include <cvt/util/internal/ParamTypes.def>
#undef X	
	template<> class ParamInfoTyped<Selection> : public ParamInfo
	{
		public:
		ParamInfoTyped( const std::string & n, size_t numVals, std::string* desc, bool input = true, size_t o = 0 ) :
				ParamInfo( PTYPE_SELECTION, n, 1, o, input, true ),
				min( 0 ), max( numVals - 1 ), defValue( 0 ), values( desc )
			{}
		
		virtual ~ParamInfoTyped() {}
		Selection minValue() const { return min; }
		Selection maxValue() const { return max; }
		Selection defaultValue() const { return defValue; }
		virtual void setDefaultValue( void * ptr ) const { *( ( Selection* )ptr ) = defValue; }
		Selection min; 
		Selection max; 
		Selection defValue;

		const std::string typeString() const { return "Selection"; }
		
		const std::string & description( const Selection & sel ){
			if( sel < min || sel > max )
				throw CVTException( "Selection out of bounds" );
			
			return values[ sel.value ];
		}
				
		private:
			std::string* values;
	};
	
}

#endif
