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

#ifndef CVT_FEATURESET_H
#define CVT_FEATURESET_H

#include <cvt/vision/features/Feature.h>
#include <cvt/geom/PointSet.h>
#include <vector>

namespace cvt {

	class FeatureSet
	{
		public:
			typedef std::vector<Feature>::iterator			iterator;
			typedef std::vector<Feature>::const_iterator	const_iterator;

			FeatureSet();
			~FeatureSet();

			void			add( const Feature& feature );
			void			clear();

			size_t			size() const;
			Feature&		operator[]( size_t i );
			const Feature&	operator[]( size_t i ) const;
			FeatureSet&		operator= (const FeatureSet& other);

			void			toPointSet2f( PointSet2f& ptset ) const;
			void			setFeatures( const Feature* f, size_t n );

			void			filterNMS( int radius, bool sortPosition );
			void			filterANMS( int radius, float threshold, bool sortPosition );
			void			filterBest( size_t n, bool sortScore );

            /**
             * @brief filterGrid - Perform a gridfiltering operation over features.
             *  The basic idea is to get a better spread of features over the entire image
             *  even when some features might have a lower score than spatially-independant /
             *  global filtering only by score.
             * @param imageWidth        Width of the input image
             * @param imageHeight       Height of the input image
             * @param cellCountX        Amount of cells in X direction
             * @param cellCountY        Amount of cells in Y direction
             * @param featuresPerCell   How many features to keep per cell
             * @remarks Range-checking is disabled for speed reasons, if you perform this
             *          method on a FeatureSet containing features with coordinates outside
             *          the specified image width/height this WILL segfault!
             */
            void            filterGrid( size_t imageWidth, size_t imageHeight,
                                        size_t cellCountX, size_t cellCountY,
                                        size_t featuresPerCell );

			void			sortPosition();

			iterator		begin()			{ return _features.begin(); }
			const_iterator	begin() const	{ return _features.begin(); }
			iterator		end()			{ return _features.end(); }
			const_iterator	end()	const	{ return _features.end(); }
			const Feature&  back()	const	{ return _features.back(); }

			class CmpScore
			{
				public:
					bool operator()( const Feature& f1, const Feature& f2 )
					{
						return f1.score > f2.score;
					}
			};

			class CmpPos
			{
				public:
					bool operator()( const Feature& f1, const Feature& f2 )
					{
						if( f1.pt.y == f2.pt.y )
							return f1.pt.x < f2.pt.x;
						return f1.pt.y < f2.pt.y;
					}
			};

			class CmpX
			{
				public:
					bool operator()( const Feature& f1, const Feature& f2 )
					{
						return f1.pt.x < f2.pt.x;
					}
			};

			class CmpY
			{
				public:
					bool operator()( const Feature& f1, const Feature& f2 )
					{
						return f1.pt.y < f2.pt.y;
					}
			};

			class CmpPosi
			{
				public:
					bool operator()( const Feature& f1, const Feature& f2 )
					{
						if( ( int )f1.pt.y == ( int )f2.pt.y )
							return ( int )f1.pt.x < ( int )f2.pt.x;
						return ( int )f1.pt.y < ( int )f2.pt.y;
					}
			};

			class CmpXi
			{
				public:
					bool operator()( const Feature& f1, const Feature& f2 )
					{
						return ( int ) f1.pt.x < ( int ) f2.pt.x;
					}
			};

			class CmpYi
			{
				public:
					bool operator()( const Feature& f1, const Feature& f2 )
					{
						return ( int )f1.pt.y < ( int )f2.pt.y;
					}
			};

		private:
			std::vector<Feature> _features;
	};


	inline FeatureSet::FeatureSet()
	{
	}

	inline FeatureSet::~FeatureSet()
	{
	}

	inline void FeatureSet::add( const Feature& feature )
	{
		_features.push_back( feature );
	}

	inline void FeatureSet::clear()
	{
		_features.clear();
	}

	inline size_t FeatureSet::size() const
	{
		return _features.size();
	}

	inline Feature& FeatureSet::operator[]( size_t i )
	{
		return _features[ i ];
	}

	inline const Feature& FeatureSet::operator[]( size_t i ) const
	{
		return _features[ i ];
	}

    inline FeatureSet& FeatureSet::operator= ( const FeatureSet& other )
	{
		// no check for self-assignment necessary because no raw pointers in class
        _features.assign( other.begin(), other.end() );
        return *this;
	}

	inline void FeatureSet::toPointSet2f( PointSet2f& ptset ) const
	{
		ptset.clear();
		for( size_t i = 0; i < size(); i++ )
			ptset.add( ( *this )[ i ].pt );
	}

	inline void FeatureSet::setFeatures( const Feature* f, size_t n )
	{
		_features.assign( f, f + n );
	}

    class FeatureSetWrapper
	{
		public:
			FeatureSetWrapper( FeatureSet& set, float scale = 1.0f, size_t octave = 0 ) :
				_featureset( set ),
				_scale( scale ),
				_octave( octave )
			{}

			void operator()( float x, float y, float score = 0.0f )
			{
				_featureset.add( Feature( x * _scale, y * _scale, 0, _octave, score ) );
			}


		private:
			FeatureSet& _featureset;
			float		_scale;
			size_t		_octave;

			FeatureSetWrapper( const FeatureSet& );
	};

}

#endif
