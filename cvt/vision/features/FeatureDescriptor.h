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

#ifndef CVT_FEATUREDESCRPTOR_H
#define CVT_FEATUREDESCRPTOR_H

#include <cvt/util/Exception.h>
#include <cvt/util/SIMD.h>
#include <cvt/vision/features/Feature.h>

namespace cvt {
	enum FeatureDescriptorComparator {
		FEATUREDESC_CMP_SSD		= 0,
		FEATUREDESC_CMP_SAD		= 1,
		FEATUREDESC_CMP_HAMMING = 2
	};

	struct FeatureDescriptor : public Feature
	{
		FeatureDescriptor( float x, float y, float a, size_t o, float sc ) : Feature( x, y, a, o, sc )
		{
		}

		FeatureDescriptor( const Feature& f ) : Feature( f )
		{
		}

		virtual ~FeatureDescriptor()
		{
		}

		virtual size_t length() const = 0;
		virtual const uint8_t* ptr() const = 0;
		virtual FeatureDescriptorComparator compareType() const = 0;
		virtual FeatureDescriptor* clone() const = 0;
	};

	template<size_t N, typename T, FeatureDescriptorComparator CMPTYPE>
	class FeatureDescriptorDistance;

	template<size_t N, typename T, FeatureDescriptorComparator CMPTYPE>
	struct FeatureDescriptorInternal : public FeatureDescriptor
	{
		FeatureDescriptorInternal( const FeatureDescriptorInternal<N,T,CMPTYPE>& other ) : FeatureDescriptor( ( const Feature& ) other )
		{
			SIMD::instance()->Memcpy( desc, other.desc, N * sizeof( T ) );
		}

		FeatureDescriptorInternal( const Feature& f ) : FeatureDescriptor( f )
		{
		}

		FeatureDescriptorInternal( float x, float y, float a, size_t o, float sc ) : FeatureDescriptor( x, y, a, o, sc )
		{
		}

		virtual ~FeatureDescriptorInternal()
		{
		}

		virtual size_t length() const
		{
			return sizeof( T ) * N;
		}

		virtual const uint8_t* ptr() const
		{
			return ( uint8_t* ) desc;
		}

		virtual FeatureDescriptorComparator compareType() const
		{
			return CMPTYPE;
		}

		virtual FeatureDescriptorInternal<N, T, CMPTYPE>* clone() const
		{
			return new FeatureDescriptorInternal<N, T, CMPTYPE>( *this );
		}

		float distance( const FeatureDescriptorInternal<N,T,CMPTYPE>& desc ) const
		{
			return FeatureDescriptorDistance<N,T,CMPTYPE>::distance( *this, desc );
		}

		T desc[ N ];
	};


	template<size_t N, typename T, FeatureDescriptorComparator CMPTYPE>
	struct FeatureDescriptorDistance
	{
		static float distance( const FeatureDescriptorInternal<N,T,CMPTYPE>& a, const FeatureDescriptorInternal<N,T,CMPTYPE>& b )
		{
			throw CVTException( "Error" );
			return 0;
		}
	};

	/* float, SSD */
	template<size_t N>
	struct FeatureDescriptorDistance<N, float, FEATUREDESC_CMP_SSD>
	{
		static float distance( const FeatureDescriptorInternal<N,float, FEATUREDESC_CMP_SSD>& a, const FeatureDescriptorInternal<N,float, FEATUREDESC_CMP_SSD>& b )
		{
			return SIMD::instance()->SSD( a.desc, b.desc, N );
		}
	};

	/* uint8_t, SSD */
	template<size_t N>
	struct FeatureDescriptorDistance<N, uint8_t, FEATUREDESC_CMP_SSD>
	{
		static float distance( const FeatureDescriptorInternal<N,uint8_t, FEATUREDESC_CMP_SSD>& a, const FeatureDescriptorInternal<N,uint8_t, FEATUREDESC_CMP_SSD>& b )
		{
			return SIMD::instance()->SSD( a.desc, b.desc, N );
		}
	};

	/* float, SAD */
	template<size_t N>
	struct FeatureDescriptorDistance<N, float, FEATUREDESC_CMP_SAD>
	{
		static float distance( const FeatureDescriptorInternal<N,float, FEATUREDESC_CMP_SAD>& a, const FeatureDescriptorInternal<N,float, FEATUREDESC_CMP_SAD>& b )
		{
			return SIMD::instance()->SAD( a.desc, b.desc, N );
		}
	};

	/* uint8_t, SAD */
	template<size_t N>
	struct FeatureDescriptorDistance<N, uint8_t, FEATUREDESC_CMP_SAD>
	{
		static float distance( const FeatureDescriptorInternal<N,uint8_t, FEATUREDESC_CMP_SAD>& a, const FeatureDescriptorInternal<N,uint8_t, FEATUREDESC_CMP_SAD>& b )
		{
			return SIMD::instance()->SAD( a.desc, b.desc, N );
		}
	};

	/* uint8_t, hamming */
	template<size_t N>
	struct FeatureDescriptorDistance<N, uint8_t, FEATUREDESC_CMP_HAMMING>
	{
		static float distance( const FeatureDescriptorInternal<N,uint8_t, FEATUREDESC_CMP_HAMMING>& a, const FeatureDescriptorInternal<N,uint8_t, FEATUREDESC_CMP_HAMMING>& b )
		{
			return SIMD::instance()->hammingDistance( a.desc, b.desc, N );
		}
	};

}

#endif
