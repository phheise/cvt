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

#ifndef CVT_ICOMPONENTS_H
#define CVT_ICOMPONENTS_H

#include <cvt/gfx/Image.h>
#include <cvt/gfx/IMapScoped.h>
#include <cvt/geom/PointSet.h>
#include <cvt/gfx/IComponents.h>
#include <cvt/util/Exception.h>

namespace cvt {

	template<typename T>
	class IComponents {
		public:
			IComponents( );
			IComponents( const Image& img );
			~IComponents();

			size_t				   size() const;
			PointSet<2,T>&		   operator[]( int i );
			const PointSet<2,T>&   operator[]( int i ) const;
			void				   add( const PointSet<2,T>& ptset );

			void				   clear();

			void				   extract( const Image& img );

		private:
			void traceComponent( PointSet<2,T>& ptset, IMapScoped<uint8_t>& map, int width, int height );
			void traceComponent( PointSet<2,T>& ptset, IMapScoped<float>& map, int width, int height );

			std::vector<PointSet<2,T> > _components;
	};

	template<typename T>
	inline IComponents<T>::IComponents()
	{
	}

	template<typename T>
	inline IComponents<T>::IComponents( const Image& img )
	{
		extract( img );
	}

	template<typename T>
	inline IComponents<T>::~IComponents()
	{
	}

	template<typename T>
	inline size_t IComponents<T>::size() const
	{
		return _components.size();
	}

	template<typename T>
	inline PointSet<2,T>& IComponents<T>::operator[]( int i )
	{
		return _components[ i ];
	}

	template<typename T>
	inline const PointSet<2,T>& IComponents<T>::operator[]( int i ) const
	{
		return _components[ i ];
	}

	template<typename T>
	inline void	IComponents<T>::add( const PointSet<2,T>& ptset )
	{
		_components.push_back( ptset );
	}

	template<typename T>
	inline void	IComponents<T>::clear()
	{
		_components.clear();
	}


	template<typename T>
	inline void IComponents<T>::traceComponent( PointSet<2,T>& ptset, IMapScoped<uint8_t>& map, int width, int height )
	{
		int dir[][2] = { { -1, -1 }, { 0, -1 }, { 1, -1 }, { 1,  0 }, { 1,  1 }, { 0,  1 }, { -1,  1 }, { -1,  0 } };

		bool onContour = true;
		size_t current = 0;
		int newx, newy;
		uint8_t* pixel = 0;

		int cx = ptset[ current ].x;
		int cy = ptset[ current ].y;

		while ( onContour )
		{
			for ( size_t i = 0, order = 0; i < 8; ++i )
			{
				newx	= Math::clamp( cx + dir[ order ][ 0 ], 0, width-1 );
				newy	= Math::clamp( cy + dir[ order ][ 1 ], 0, height-1 );

				pixel	= map.base() + newy * map.stride() + newx;
				order	= ( order + ( *pixel == 0 ) ) % 8;

				if ( *pixel != 0 )
				{
					ptset.add( Vector2<T>( newx, newy ) );
					*pixel = 0;
				}
			}

			current++;
			onContour = current < ptset.size();
			if ( onContour )
			{
				cx = ( int ) ptset[ current ].x;
				cy = ( int ) ptset[ current ].y;
			}
		}
	}

	template<typename T>
	inline void IComponents<T>::traceComponent( PointSet<2,T>& ptset, IMapScoped<float>& map, int width, int height )
	{
		int dir[][2] = { { -1, -1 }, { 0, -1 }, { 1, -1 }, { 1,  0 }, { 1,  1 }, { 0,  1 }, { -1,  1 }, { -1,  0 } };

		bool onContour = true;
		size_t current = 0;
		int newx, newy;
		float* pixel = 0;

		int cx = ptset[ current ].x;
		int cy = ptset[ current ].y;

		while ( onContour )
		{
			for ( size_t i = 0, order = 0; i < 8; ++i )
			{
				newx	= Math::clamp( cx + dir[ order ][ 0 ], 0, width-1 );
				newy	= Math::clamp( cy + dir[ order ][ 1 ], 0, height-1 );

				pixel	= ( float* ) ( ( uint8_t* ) map.base() + newy * map.stride() + newx * sizeof( float ) );
				order	= ( order + ( *pixel == 0 ) ) % 8;

				if ( *pixel != 0 )
				{
					ptset.add( Vector2<T>( newx, newy ) );
					*pixel = 0.0f;
				}
			}

			current++;
			onContour = current < ptset.size();
			if ( onContour )
			{
				cx = ( int ) ptset[ current ].x;
				cy = ( int ) ptset[ current ].y;
			}
		}
	}

	template<typename T>
	inline void IComponents<T>::extract( const Image& img )
	{
		if( img.format().formatID == IFORMAT_GRAY_UINT8 ) {
			Image tmp( img ); // create copy to leave img untouched
			IMapScoped<uint8_t> mapsrc( tmp );
			int width = tmp.width();
			int height = tmp.height();
			for( int y = 0; y < height; y++ ) {
				uint8_t* ptr = mapsrc.ptr();
				for( int x = 0; x < width; x++ ) {
					if( *ptr != 0 ) {
						*ptr = 0;
						_components.push_back( PointSet<2,T>() );
						_components.back().add( Vector2<T>( x, y ) );
						traceComponent( _components.back(), mapsrc, width, height );
					}
					ptr++;
				}
				mapsrc++;
			}
		} else if( img.format().formatID == IFORMAT_GRAY_FLOAT ) {
			Image tmp( img ); // create copy to leave img untouched
			IMapScoped<float> mapsrc( tmp );
			int width = tmp.width();
			int height = tmp.height();
			for( int y = 0; y < height; y++ ) {
				float* ptr = mapsrc.ptr();
				for( int x = 0; x < width; x++ ) {
					if( *ptr != 0 ) {
						*ptr = 0;
						_components.push_back( PointSet<2,T>() );
						_components.back().add( Vector2<T>( x, y ) );
						traceComponent( _components.back(), mapsrc, width, height );
					}
					ptr++;
				}
				mapsrc++;
			}
		} else {
			throw CVTException( "Unsupported image format!" );
		}
	}
}
#endif
