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

#ifndef IFILTER_H
#define IFILTER_H

#include <stdint.h>
#include <string>
#include <iostream>
#include <cvt/util/Flags.h>
#include <cvt/util/ParamSet.h>
#include <cvt/util/Plugin.h>

namespace cvt {
	enum IFilterTypeFlags {
					   IFILTER_CPU = ( 1 << 0 ),
					   IFILTER_OPENGL = ( 1 << 1 ),
					   IFILTER_OPENCL = ( 1 << 2 )
					 };

	CVT_ENUM_TO_FLAGS( IFilterTypeFlags, IFilterType )

	class IFilter : public Plugin {
		friend std::ostream& operator<<( std::ostream& out, const IFilter& filter );

		public:
			virtual ParamSet* parameterSet() const { return new ParamSet( _pinfo, _pinfosize ); };
			virtual void apply( const ParamSet* attribs, IFilterType iftype = IFILTER_CPU ) const = 0;
			uint32_t getIFilterType() const { return _iftype; };
			const String& name() const { return _name; };
			virtual ~IFilter() {};

		protected:
			IFilter( const String& name, ParamInfo** pinfo, size_t pinfosize, IFilterType ifiltertype ) : Plugin( PLUGIN_IFILTER ), _iftype( ifiltertype ), _name( name ), _pinfo( pinfo ), _pinfosize( pinfosize ) {};

		private:
			IFilter( const IFilter& ifilter );
			IFilter& operator=( const IFilter& ifilter );

			IFilterType _iftype;
			String _name;
			ParamInfo** _pinfo;
			size_t _pinfosize;
	};

	inline std::ostream& operator<<( std::ostream& out, const IFilter& filter )
	{
		out << "Filter: " << filter.name() << std::endl;
		out << "Parameters: " << std::endl;
		for( size_t i = 0; i < filter._pinfosize; i++ )
			std::cout << "\t" << *( filter._pinfo[ i ] ) << std::endl;
		return out;
	}
}

#endif
