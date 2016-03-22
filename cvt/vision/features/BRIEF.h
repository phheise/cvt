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

#ifndef CVT_BRIEF_H
#define CVT_BRIEF_H

#include <cvt/gfx/Image.h>
#include <cvt/gfx/IMapScoped.h>
#include <cvt/vision/ImagePyramid.h>
#include <cvt/vision/features/FeatureSet.h>
#include <cvt/vision/features/FeatureDescriptor.h>
#include <cvt/vision/features/FeatureDescriptorExtractor.h>
#include <cvt/vision/features/MatchBruteForce.h>

namespace cvt {

	template<size_t N>
	class BRIEF : public FeatureDescriptorExtractor
	{
		public:
			typedef FeatureDescriptorInternal<N, uint8_t, FEATUREDESC_CMP_HAMMING> Descriptor;

			BRIEF( size_t boxradius = 3 );
			BRIEF( const BRIEF& brief );
			~BRIEF();

			size_t						size() const;
			BRIEF<N>*					clone() const;
			FeatureDescriptor&			operator[]( size_t i );
			const FeatureDescriptor&	operator[]( size_t i ) const;

			void clear();
			void extract( const Image& img, const FeatureSet& features );
			void extract( const ImagePyramid& pyr, const FeatureSet& features );

			void matchBruteForce( std::vector<FeatureMatch>& matches, const FeatureDescriptorExtractor& other, float distThresh ) const;

			void matchInWindow( std::vector<MatchingIndices>& matches,
								const std::vector<FeatureDescriptor*>& other,
								float maxFeatureDist,
								float maxDescDistance ) const;

			void matchInWindow( std::vector<MatchingIndices>& matches,
								const RowLookupTable& rlt,
								const std::vector<FeatureDescriptor*>& other,
								float maxFeatureDist,
								float maxDescDistance ) const;

            void scanLineMatch( std::vector<FeatureMatch>& matches,
                                const std::vector<const FeatureDescriptor*>& left,
                                float minDisp,
                                float maxDisp,
                                float maxDescDist,
                                float maxLineDist ) const;

            void scanLineMatch( std::vector<FeatureMatch>& matches,
                                const RowLookupTable& rlt,
                                const std::vector<const FeatureDescriptor*>& left,
                                float minDisp,
                                float maxDisp,
                                float maxDescDist,
                                float maxLineDist ) const;

		private:
			struct DistFunc {
				DistFunc() : _simd( SIMD::instance() )
				{
				}

				float operator()( const Descriptor& a, const Descriptor& b ) const
				{
					return _simd->hammingDistance( a.desc, b.desc, N );
				}

				SIMD* _simd;
			};


            template <class ImgT>
            void extractInternal( const ImagePyramid& pyr, const FeatureSet& features );

            template <class ImgT>
            void extractInternal( const Image& img, const FeatureSet& features );

            template <class ImgT>
            void descriptor( Descriptor& feature, const Vector2f& pt, const IMapScoped<ImgT>& map );

			const size_t			_boxradius;
			std::vector<Descriptor> _features;

	};

#include <cvt/vision/features/BRIEFPattern.h>

	typedef BRIEF<16> BRIEF16;
	typedef BRIEF<32> BRIEF32;
	typedef BRIEF<64> BRIEF64;

	template<size_t N>
	inline BRIEF<N>::BRIEF( size_t boxradius ) :
		_boxradius( boxradius )
	{
	}

	template<size_t N>
	inline BRIEF<N>::BRIEF( const BRIEF<N>& other ) :
		_boxradius( other._boxradius ),
		_features( other._features )
	{
	}

	template<size_t N>
	inline BRIEF<N>::~BRIEF()
	{
	}

	template<size_t N>
	inline size_t BRIEF<N>::size() const
	{
		return _features.size();
	}

	template<size_t N>
	inline BRIEF<N>* BRIEF<N>::clone() const
	{
		BRIEF<N>* bcopy = new BRIEF<N>( _boxradius );
		bcopy->_features = _features;
		return bcopy;
	}

	template<size_t N>
	inline FeatureDescriptor& BRIEF<N>::operator[]( size_t i )
	{
		return _features[ i ];
	}

	template<size_t N>
	inline const FeatureDescriptor& BRIEF<N>::operator[]( size_t i ) const
	{
		return _features[ i ];
	}

	template<size_t N>
	inline void BRIEF<N>::clear()
	{
		_features.clear();
	}

	template<size_t N>
    inline void BRIEF<N>::extract( const ImagePyramid& pyr, const FeatureSet& features )
    {
        if( pyr[ 0 ].channels() != 1 || ( pyr[ 0 ].format() != IFormat::GRAY_UINT8 && pyr[ 0 ].format() != IFormat::GRAY_FLOAT ) )
            throw CVTException( "Unimplemented" );

        if( pyr[ 0 ].format() == IFormat::GRAY_FLOAT )
            extractInternal<const float>( pyr[ 0 ], features );
        else if( pyr[ 0 ].format() == IFormat::GRAY_UINT8 )
            extractInternal<const uint8_t>( pyr[ 0 ], features );
    }

	template<size_t N>
	inline void BRIEF<N>::extract( const Image& img, const FeatureSet& features )
	{
		if( img.channels() != 1 || ( img.format() != IFormat::GRAY_UINT8 && img.format() != IFormat::GRAY_FLOAT ) )
			throw CVTException( "Unimplemented" );

		if( img.format() == IFormat::GRAY_FLOAT )
            extractInternal<const float>( img, features );
		else if( img.format() == IFormat::GRAY_UINT8 )
            extractInternal<const uint8_t>( img, features );
	}

    template<size_t N>
    template<class ImgT>
    inline void BRIEF<N>::extractInternal( const ImagePyramid& pyr, const FeatureSet& features )
    {
        ImagePyramid boxPyr( pyr.octaves(), pyr.scaleFactor() );
        pyr.boxfilter( boxPyr, _boxradius );

        size_t octaves = pyr.octaves();
        std::vector<IMapScoped<ImgT>*> maps;
        std::vector<float> scales;
        for( size_t i = 0; i < octaves; ++i ){
            maps.push_back( new IMapScoped<ImgT>( boxPyr[ i ] ) );
            scales.push_back( Math::pow( boxPyr.scaleFactor(), ( float )i ) );
        }

        size_t iend = features.size();
        Vector2f vs;
        for( size_t i = 0; i < iend; ++i ) {
            _features.push_back( Descriptor( features[ i ] ) );
            Descriptor& desc = _features.back();
            size_t o = desc.octave;
            vs = desc.pt * scales[ o ];
            descriptor( desc, vs, *maps[ o ] );
        }

        for( size_t i = 0; i < octaves; ++i ){
            delete maps[ i ];
        }
    }

    template<size_t N>
    template<class ImgT>
    inline void BRIEF<N>::extractInternal( const Image& img, const FeatureSet& features )
	{
		Image boximg;
		img.boxfilter( boximg, _boxradius );

        IMapScoped<ImgT> map( boximg );
		size_t iend = features.size();
		for( size_t i = 0; i < iend; ++i ) {
			_features.push_back( Descriptor( features[ i ] ) );
            descriptor( _features.back(), features[ i ].pt, map );
		}
	}

    template<size_t N>
    template<class ImgT>
    inline void BRIEF<N>::descriptor( Descriptor& feature, const Vector2f& pt, const IMapScoped<ImgT>& map )
    {
        int px = ( int ) pt.x;
        int py = ( int ) pt.y;

        #define DOBRIEFTEST( n ) ( map( px + _brief_pattern[ n ][ 0 ], py + _brief_pattern[ n ][ 1 ] ) < map( px + _brief_pattern[ n ][ 2 ], py + _brief_pattern[ n ][ 3 ] ) )
        for( size_t n = 0; n < N; n++ ) {
            uint8_t tests  = 0;

            for( int t = 0; t < 8; t++ ) {
                tests |= ( DOBRIEFTEST( n * 8 + t ) << t );
            }
            feature.desc[ n ] = tests;
        }

    }

	template<size_t N>
	inline void BRIEF<N>::matchBruteForce( std::vector<FeatureMatch>& matches, const FeatureDescriptorExtractor& other, float distThresh ) const
	{
		DistFunc dfunc;
		FeatureMatcher::matchBruteForce<Descriptor, typename BRIEF<N>::DistFunc>( matches, this->_features, ( ( const BRIEF<N>& ) other)._features, dfunc, distThresh );
	}

	template<size_t N>
	inline void BRIEF<N>::matchInWindow( std::vector<MatchingIndices>& matches,
										 const std::vector<FeatureDescriptor*>& other,
										 float maxFeatureDist,
										 float maxDescDistance ) const
	{
		DistFunc dfunc;
		FeatureMatcher::matchInWindow<Descriptor, DistFunc>( matches,
															 other,
															 this->_features,
															 dfunc,
															 maxFeatureDist,
															 maxDescDistance );
	}

    template<size_t N>
    inline void BRIEF<N>::matchInWindow( std::vector<MatchingIndices>& matches,
                                         const RowLookupTable& rlt,
                                         const std::vector<FeatureDescriptor*>& other,
                                         float maxFeatureDist,
                                         float maxDescDistance ) const
	{
		DistFunc dfunc;
		FeatureMatcher::matchInWindow<Descriptor, DistFunc>( matches,
															 rlt,
															 other,
															 this->_features,
															 dfunc,
															 maxFeatureDist,
															 maxDescDistance );
	}

	template<size_t N>
	inline void BRIEF<N>::scanLineMatch( std::vector<FeatureMatch>& matches,
										 const std::vector<const FeatureDescriptor*>& left,
										 float minDisp,
										 float maxDisp,
										 float maxDescDist,
										 float maxLineDist ) const
	{
		DistFunc dfunc;
		FeatureMatcher::scanLineMatch( matches,
									   left,
									   _features,
									   dfunc,
									   minDisp,
									   maxDisp,
									   maxDescDist,
									   maxLineDist );
	}

    template<size_t N>
    inline void BRIEF<N>::scanLineMatch( std::vector<FeatureMatch>& matches,
                                         const RowLookupTable& rlt,
                                         const std::vector<const FeatureDescriptor*>& left,
                                         float minDisp,
                                         float maxDisp,
                                         float maxDescDist,
                                         float maxLineDist ) const
    {
        DistFunc dfunc;
        FeatureMatcher::scanLineMatch( matches,
                                       rlt,
                                       left,
                                       _features,
                                       dfunc,
                                       minDisp,
                                       maxDisp,
                                       maxDescDist,
                                       maxLineDist );
    }
}

#endif
