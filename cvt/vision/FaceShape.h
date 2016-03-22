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

#ifndef CVT_FACESHAPE_H
#define CVT_FACESHAPE_H

#include <cvt/gfx/Image.h>
#include <cvt/vision/MeasurementModel.h>
#include <cvt/math/Matrix.h>
#include <cvt/math/CostFunction.h>
#include <cvt/io/Resources.h>
#include <cvt/gfx/GFX.h>
#include <stdio.h>

//#define GTLINEINPUT 1

namespace cvt {
	template<typename T>
	class FaceShape : public MeasurementModel< T,
				  FaceShape,
				  Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>,
				  Eigen::Matrix<T, Eigen::Dynamic, 1>,
				  Eigen::Matrix<T, Eigen::Dynamic, 1>,
				  T >
	{
		public:
			FaceShape();
			~FaceShape();
			void	updateInput( const Image * img ){ _currI = img; updateInputData(); };
			void	setTransform( T scale, T angle, T tx, T ty );
			void	setTransform( const Matrix3<T> t );
			void	drawCurrent( GFX* g ) const;
			void    draw( GFX* g, Matrix3<T>& transform, Eigen::Matrix<T, Eigen::Dynamic, 1 >& p ) const;

			void	apply( const Eigen::Matrix<T, Eigen::Dynamic, 1> & delta );
			T		buildLSSystem( Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> & A, Eigen::Matrix<T, Eigen::Dynamic, 1> & b, const CostFunction<T, T> & costFunc );
//			T		evaluateCosts( const CostFunction<T, T> & costFunc );

			Matrix3<T> transform() const { return _transform; };
			Eigen::Matrix<T, Eigen::Dynamic, 1> weights() const { return _p; };
			void resetWeights() { _p.setZero(); }

		private:
			void updateInputData();
			void updateCurrent();
			void loadFaceShape( const char* path );
			bool sampleNormal( uint8_t* dxptr, uint8_t* dyptr, int _x, int _y, int _dx, int _dy, size_t dxstride, size_t dystride, Vector2<T>& norm, T& dist );
			bool sampleNormal2( uint8_t* dxptr, uint8_t* dyptr, int _x, int _y, int _dx, int _dy, size_t dxstride, size_t dystride, Vector2<T>& norm, T& dist );

			bool sampleNormalCanny( const uint8_t* cptr, uint8_t* dxptr, uint8_t* dyptr, int _x, int _y, int _dx, int _dy, size_t cstride, size_t dxstride, size_t dystride, Vector2<T>& norm, T& dist );

			const Image* _currI;
			IKernel _kdx, _kdy;
			Image _dx;
			Image _dy;
			Image _canny;
			size_t _ptsize;
			size_t _pcsize;
			size_t _lsize;
			uint32_t* _lines;
			T _costs;
			Matrix3<T> _transform;
			Eigen::Matrix<T, Eigen::Dynamic, 1 > _p;
			Eigen::Matrix<T, Eigen::Dynamic, 1 > _mean;
			Eigen::Matrix<T, Eigen::Dynamic, 1 > _regcovar;
			Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic > _pc;
			Eigen::Matrix<T, Eigen::Dynamic, 1 > _pts;
	};

	template<typename T>
	inline FaceShape<T>::FaceShape() : _currI( NULL), _kdx( IKernel::HAAR_HORIZONTAL_3 ), _kdy( IKernel::HAAR_VERTICAL_3 ),
									  _ptsize( 0 ), _pcsize( 0 ), _lsize( 0 ), _lines( 0 ), _costs( 0 )
	{
		_transform.setIdentity();

		std::string dataFolder( CVT_DATA_FOLDER );
		std::string inputFile(dataFolder + "/face.data");
		loadFaceShape( inputFile.c_str() );
	}

	template<typename T>
	inline FaceShape<T>::~FaceShape()
	{
		delete[] _lines;
	}

	template<typename T>
	inline void FaceShape<T>::loadFaceShape( const char* path )
	{
		FILE* f;
		f = fopen( path, "rb" );
		if( !f )
			throw CVTException( "Could not open file!" );
		uint32_t tmp;
		fread( &tmp, sizeof( uint32_t ), 1, f );
		_ptsize = tmp;
		fread( &tmp, sizeof( uint32_t ), 1, f );
		_pcsize = tmp;

		_mean = Eigen::Matrix<T, Eigen::Dynamic, 1>::Zero( _ptsize * 2 );
		_regcovar = Eigen::Matrix<T, Eigen::Dynamic, 1>::Zero( _pcsize );
		_pc = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Zero( _ptsize * 2, _pcsize );
		_p = Eigen::Matrix<T, Eigen::Dynamic, 1>::Zero( _pcsize );
		_pts = Eigen::Matrix<T, Eigen::Dynamic, 1>::Zero( _ptsize * 2 );

		float ftmp;
		for( size_t i = 0; i < _ptsize * 2; i++ ) {
			fread( &ftmp, sizeof( float ), 1, f );
			_mean( i ) = ftmp;
		}

		for( size_t i = 0; i < _pcsize; i++ ) {
			fread( &ftmp, sizeof( float ), 1, f );
			_regcovar( i ) = 1.0f / ( ftmp * ftmp + 1e-8 );
		}

		for( size_t c = 0; c < _pcsize; c++ ) {
			for( size_t i = 0; i < _ptsize * 2; i++ ) {
				fread( &ftmp, sizeof( float ), 1, f );
				_pc( i, c ) = ftmp;
			}
		}
		fread( &tmp, sizeof( uint32_t ), 1, f );
		_lsize = tmp;
		_lines = new uint32_t[ _lsize * 2 ];
		for( size_t i = 0; i < _lsize * 2; i++ )
			fread( &_lines[ i ], sizeof( uint32_t ), 1, f );
		fclose( f );
		updateCurrent();
	}

	template<typename T>
	inline void FaceShape<T>::setTransform( T s, T angle, T tx, T ty )
	{
		T a = Math::deg2Rad( angle );
		Matrix2f rot( Math::cos( a ), -Math::sin( a ), Math::sin( a ), Math::cos( a ) );
		_transform[ 0 ][ 0 ] = s * rot[ 0 ][ 0 ];
		_transform[ 0 ][ 1 ] = s * rot[ 0 ][ 1 ];
		_transform[ 0 ][ 2 ] = tx;
		_transform[ 1 ][ 0 ] = s * rot[ 1 ][ 0 ];
		_transform[ 1 ][ 1 ] = s * rot[ 1 ][ 1 ];
		_transform[ 1 ][ 2 ] = ty;
		_transform[ 2 ][ 0 ] = 0.0f;
		_transform[ 2 ][ 1 ] = 0.0f;
		_transform[ 2 ][ 2 ] = 1.0f;
		updateCurrent();
	}
	template<typename T>
	inline void FaceShape<T>::setTransform( const Matrix3<T> t )
	{
		_transform = t;
		updateCurrent();
	}


	template<typename T>
	inline void FaceShape<T>::updateCurrent()
	{
		Vector2<T> pt;
		_pts = _mean + _pc * _p;
		for( size_t i = 0; i < _ptsize; i++ ) {
			pt.x = _pts[ i * 2 ];
			pt.y = _pts[ i * 2 + 1 ];
			pt = _transform * pt;
			_pts[ i * 2 ] = pt.x;
			_pts[ i * 2 + 1 ] = pt.y;
		}
	}

	template<typename T>
	inline void FaceShape<T>::updateInputData()
	{
#ifndef GTLINEINPUT
		_dx.reallocate( *_currI );
		_dy.reallocate( *_currI );
		_currI->convolve( _dx, _kdx, IKernel::GAUSS_VERTICAL_3 );
		_currI->convolve( _dy, IKernel::GAUSS_HORIZONTAL_3, _kdy);
/*		Image blax( _dx );
		blax.mul( _dx );
		Image blay( _dy );
		blay.mul( _dy );
		blax.add( blay );
		blax.mul( 100.0f );
		ImageIO::savePNG( blax, "DXDY.png" );
		ImageIO::savePNG( _dx, "DX.png" );
		ImageIO::savePNG( _dy, "DY.png" );*/
#endif
	}


	template<typename T>
	inline void	FaceShape<T>::apply( const Eigen::Matrix<T, Eigen::Dynamic, 1> & delta )
	{
		T s1 = 1 + delta( 0 );
//		T angle = delta( 1 );
		T s2 = delta( 1 );
		T tx = delta( 2 );
		T ty = delta( 3 );
		Matrix3<T> TT;
//		Matrix2<T> rot( Math::cos( angle ), -Math::sin( angle ), Math::sin( angle ), Math::cos( angle ) );
//		TT[ 0 ][ 0 ] = s1 * rot[ 0 ][ 0 ];
//		TT[ 0 ][ 1 ] = s1 * rot[ 0 ][ 1 ];
//		TT[ 0 ][ 2 ] = tx;
//		TT[ 1 ][ 0 ] = s1 * rot[ 1 ][ 0 ];
//		TT[ 1 ][ 1 ] = s1 * rot[ 1 ][ 1 ];
		TT[ 0 ][ 0 ] = s1;
		TT[ 0 ][ 1 ] = -s2;
		TT[ 0 ][ 2 ] = tx;
		TT[ 1 ][ 0 ] = s2;
		TT[ 1 ][ 1 ] = s1;
		TT[ 1 ][ 2 ] = ty;
		TT[ 2 ][ 0 ] = 0.0f;
		TT[ 2 ][ 1 ] = 0.0f;
		TT[ 2 ][ 2 ] = 1.0f;

		if( delta.rows() == 4 )
			_transform = TT * _transform;
		else
			_p += delta;
//		_p.block( _pcsize - 5 , 0, 5, 1 ).setZero();
		updateCurrent();
	}


	template<typename T>
	inline T FaceShape<T>::buildLSSystem( Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> & A, Eigen::Matrix<T, Eigen::Dynamic, 1> & b, const CostFunction<T, T> & costFunc )
	{
		static bool flip = true;
		Matrix3<T> tnew;
		Vector2<T> pts[ 2 ], n, p, ptmp;
		Eigen::Matrix<T, Eigen::Dynamic, 1> tmp;
		uint8_t* dxptr;
		uint8_t* dyptr;
		const uint8_t* cptr;
		size_t dxstride, dystride, cstride;
		T ftmp;
		size_t i1, i2;
		T weight;
		size_t lines = 0;

	/*	A = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Zero( 4 + _pcsize, 4 + _pcsize );
		b = Eigen::Matrix<T, Eigen::Dynamic, 1>::Zero( 4 + _pcsize );
		tmp = Eigen::Matrix<T, Eigen::Dynamic, 1>::Zero( 4 + _pcsize );*/

		if( flip ) {
			A = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Zero( 4, 4 );
			b = Eigen::Matrix<T, Eigen::Dynamic, 1>::Zero( 4 );
			tmp = Eigen::Matrix<T, Eigen::Dynamic, 1>::Zero( 4 );
		} else {
			A = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Zero( _pcsize, _pcsize );
			b = Eigen::Matrix<T, Eigen::Dynamic, 1>::Zero( _pcsize );
			tmp = Eigen::Matrix<T, Eigen::Dynamic, 1>::Zero( _pcsize );
		}

#ifndef GTLINEINPUT
		dxptr = _dx.map<uint8_t>( &dxstride );
		dyptr = _dy.map<uint8_t>( &dystride );
#else
		cptr = _currI->map<uint8_t>( & cstride );
#endif

#define MAXDIST 15
#define INCR	0.1f
#define COSMAX	0.60f
#define THRESHOLD 0.01f

		_costs = 0;

		for( size_t i = 0; i < _lsize; i++ ) {
			i1 = _lines[ i * 2 ];
			i2 = _lines[ i * 2 + 1 ];

			pts[ 0 ].x = _pts[ i1 * 2 ];
			pts[ 0 ].y = _pts[ i1 * 2 + 1 ];
			pts[ 1 ].x = _pts[ i2 * 2 ];
			pts[ 1 ].y = _pts[ i2 * 2 + 1 ];
			Vector2<T> dp = pts[ 1 ] - pts[ 0 ];
			Vector2<T> n = dp;
			n.normalize();
			ftmp = -n.x;
			n.x = n.y;
			n.y = ftmp;

			Matrix2<T> TTmp( _transform );
			float incr = 5.0f / dp.length();
			incr = Math::clamp( incr, 0.01f, 0.25f );
			for( T alpha = Math::rand( 0.0f, incr ); alpha <= 1; alpha += incr ) {
				p = Math::mix( pts[ 0 ], pts[ 1 ], alpha );
				if( flip ) {
					tmp( 0 ) = n * p;
					tmp( 1 ) = - n.x * p.y + n.y * p.x;
					tmp( 2 ) = n.x;
					tmp( 3 ) = n.y;
				} else {
					for( size_t k = 0; k < _pcsize; k++ ) {
						ptmp.x = Math::mix( _pc( i1 * 2, k ), _pc( i2 * 2, k ), alpha );
						ptmp.y = Math::mix( _pc( i1 * 2 + 1, k ), _pc( i2 * 2 + 1, k ), alpha );
						tmp( k ) = n * ( TTmp * ptmp );
					}
				}
#ifndef GTLINEINPUT
				if( sampleNormal2( dxptr, dyptr, Math::round( p.x ), Math::round( p.y ),
								 Math::round( pts[ 1 ].x - pts[ 0 ].x ), Math::round( pts[ 1 ].y - pts[ 0 ].y ), dxstride, dystride, n, ftmp ) ) {
#else
				if( sampleNormalCanny( cptr, NULL, NULL, Math::round( p.x ), Math::round( p.y ),
								 Math::round( pts[ 1 ].x - pts[ 0 ].x ), Math::round( pts[ 1 ].y - pts[ 0 ].y ), cstride, dxstride, dystride, n, ftmp ) ) {
#endif
					_costs += costFunc.cost( ftmp, weight );
					lines++;

					if( costFunc.isRobust ){
						A += tmp * weight * tmp.transpose();
						b += tmp * ftmp * weight;
					} else {
						A += tmp * tmp.transpose();
						b += tmp * ftmp;
					}
				}
			}
		}

			if( !flip ) {
				tmp = _regcovar;
//				tmp( 0 ) = tmp( 1 ) = tmp( 2 ) = tmp( 3 ) = 0.0f;
				A.diagonal() += 4.0f * tmp;
				tmp.array() *= _p.array();
				b -= 4.0f * tmp;
			} else {
/*				tmp( 0 ) = tmp( 1 ) = 0.0f;
			 	tmp( 2 ) = tmp( 3 ) = 0.0f;
				A.diagonal() += tmp;*/
			}

		flip = !flip;
#ifndef GTLINEINPUT
		_dx.unmap( dxptr );
		_dy.unmap( dyptr );
#else
		_currI->unmap( cptr );
#endif
		return _costs / ( T ) lines;
	}


	template<typename T>
	inline bool FaceShape<T>::sampleNormal( uint8_t* dxptr, uint8_t* dyptr, int _x, int _y, int deltax, int deltay, size_t dxstride, size_t dystride, Vector2<T>& norm, T& dist )
	{
		int dy = - deltax;
		int dx =   deltay;
		ssize_t dxincx, dxincy, dyincx, dyincy;
		int sx, sy;
		int x, y;
		size_t dxbpp, dybpp;
		size_t w, h;
		int err = dx + dy;
		int e2;
		Vector2<T> grad;

		w = _dx.width();
		h = _dx.height();
		dxbpp = _dx.bpp();
		dybpp = _dy.bpp();

		if( deltay < 0 ) {
			dxincx = -dxbpp;
			dyincx = -dybpp;
			sx   = -1;
		} else {
			dxincx = dxbpp;
			dyincx = dybpp;
			sx   = 1;
		}

		if( deltax < 0 ) {
			dxincy = dxstride;
			dyincy = dystride;
			sy   = 1;
		} else {
			dxincy = -dxstride;
			dyincy = -dystride;
			sy   = -1;
		}
		x = y = 0;

		uint8_t* dxptr1 = dxptr + _x * dxbpp + _y * dxstride;
		uint8_t* dxptr2 = dxptr1;
		uint8_t* dyptr1 = dyptr + _x * dybpp + _y * dystride;
		uint8_t* dyptr2 = dyptr1;

		size_t n = MAXDIST;
		float mag;
		while( n-- ) {
			if( ( ( size_t ) ( _x + x ) ) < w && ( ( size_t ) ( _y + y ) ) < h ) {
				grad.x = *( ( float* ) dxptr1 );
				grad.y = *( ( float* ) dyptr1 );
				mag = grad.normalize();
				if( mag >= THRESHOLD && Math::abs( norm * grad ) >= COSMAX ) {
					dist = Math::sqrt( ( T ) ( Math::sqr( x ) + Math::sqr( y ) ) );
					return true;
				}
			}
			if( ( ( size_t ) ( _x - x ) ) < w && ( ( size_t ) ( _y - y ) ) < h ) {
				grad.x = *( ( float* ) dxptr2 );
				grad.y = *( ( float* ) dyptr2 );

				mag = grad.normalize();
				if( mag >= THRESHOLD && Math::abs( norm * grad ) >= COSMAX ) {
					dist = - Math::sqrt( ( T ) ( Math::sqr( x ) + Math::sqr( y ) ) );
					return true;
				}
			}
			e2 = 2 * err;
			if( e2 >= dy ) {
				err += dy;
				dxptr1 += dxincx; dxptr2 -= dxincx;
				dyptr1 += dyincx; dyptr2 -= dyincx;
				x += sx;
			}
			if( e2 <= dx ) {
				err += dx;
				dxptr1 += dxincy; dxptr2 -= dxincy;
				dyptr1 += dyincy; dyptr2 -= dyincy;
				y += sy;
			}
		}
		return false;
	}

	template<typename T>
	inline bool FaceShape<T>::sampleNormal2( uint8_t* dxptr, uint8_t* dyptr, int _x, int _y, int deltax, int deltay, size_t dxstride, size_t dystride, Vector2<T>& norm, T& dist )
	{
		int dy = - deltax;
		int dx =   deltay;
		ssize_t dxincx, dxincy, dyincx, dyincy;
		int sx, sy;
		int x, y;
		size_t dxbpp, dybpp;
		size_t w, h;
		int err = dx + dy;
		int e2;
		Vector2<T> grad;
		T best = 0;
		T bdist;

		w = _dx.width();
		h = _dx.height();
		dxbpp = _dx.bpp();
		dybpp = _dy.bpp();

		if( deltay < 0 ) {
			dxincx = -dxbpp;
			dyincx = -dybpp;
			sx   = -1;
		} else {
			dxincx = dxbpp;
			dyincx = dybpp;
			sx   = 1;
		}

		if( deltax < 0 ) {
			dxincy = dxstride;
			dyincy = dystride;
			sy   = 1;
		} else {
			dxincy = -dxstride;
			dyincy = -dystride;
			sy   = -1;
		}
		x = y = 0;

		uint8_t* dxptr1 = dxptr + _x * dxbpp + _y * dxstride;
		uint8_t* dxptr2 = dxptr1;
		uint8_t* dyptr1 = dyptr + _x * dybpp + _y * dystride;
		uint8_t* dyptr2 = dyptr1;

		size_t n = MAXDIST;
		float mag;
		while( n-- ) {
			if( ( ( size_t ) ( _x + x ) ) < w && ( ( size_t ) ( _y + y ) ) < h ) {
				grad.x = *( ( float* ) dxptr1 );
				grad.y = *( ( float* ) dyptr1 );
				mag = grad.normalize();
//				if( mag >= THRESHOLD && Math::abs( norm * grad ) >= COSMAX ) {
					T d = ( ( T ) ( Math::sqr( x ) + Math::sqr( y ) ) );
					T p = Math::exp( - d / 250 )
						  * ( 1.0 / ( 1.0 + exp( - ( Math::abs( norm * grad ) - 0.65 ) * 15.0 ) ) )
					      * ( 1.0 / ( 1.0 + exp( -( mag - 0.01 ) * 20.0 ) ) );
					if( p > best ) {
						best = p;
						bdist = ( d );
					}
//				}
			}
			if( ( ( size_t ) ( _x - x ) ) < w && ( ( size_t ) ( _y - y ) ) < h ) {
				grad.x = *( ( float* ) dxptr2 );
				grad.y = *( ( float* ) dyptr2 );

				mag = grad.normalize();
//				if( mag >= THRESHOLD && Math::abs( norm * grad ) >= COSMAX ) {
					T d = ( ( T ) ( Math::sqr( x ) + Math::sqr( y ) ) );
					T p = Math::exp( - d / 250 )
						  * ( 1.0 / ( 1.0 + exp( - ( Math::abs( norm * grad ) - 0.65 ) * 15.0 ) ) )
					      * ( 1.0 / ( 1.0 + exp( - ( mag - 0.01 ) * 20.0 ) ) );
					if( p > best ) {
						best = p;
						bdist = -( d );
					}
//				}
			}
			e2 = 2 * err;
			if( e2 >= dy ) {
				err += dy;
				dxptr1 += dxincx; dxptr2 -= dxincx;
				dyptr1 += dyincx; dyptr2 -= dyincx;
				x += sx;
			}
			if( e2 <= dx ) {
				err += dx;
				dxptr1 += dxincy; dxptr2 -= dxincy;
				dyptr1 += dyincy; dyptr2 -= dyincy;
				y += sy;
			}
		}
		if( best >= 0.05 ) {
			dist = bdist;
			return true;
		}
		return false;
	}

	template<typename T>
	inline bool FaceShape<T>::sampleNormalCanny( const uint8_t* cptr, uint8_t* dxptr, uint8_t* dyptr, int _x, int _y, int deltax, int deltay, size_t cstride, size_t dxstride, size_t dystride, Vector2<T>& norm, T& dist )
	{
		int dy = - deltax;
		int dx =   deltay;
		ssize_t incx, incy;
		int sx, sy;
		int x, y;
		size_t bpp, dxbpp, dybpp;
		size_t w, h;
		int err = dx + dy;
		int e2;
		Vector2<T> grad;

		w = _currI->width();
		h = _currI->height();
		bpp = _canny.bpp();
#ifndef GTLINEINPUT
		dxbpp = _dx.bpp();
		dybpp = _dy.bpp();
#endif

		if( deltay < 0 ) {
			incx = -bpp;
			sx   = -1;
		} else {
			incx = bpp;
			sx   = 1;
		}

		if( deltax < 0 ) {
			incy = cstride;
			sy   = 1;
		} else {
			incy = -cstride;
			sy   = -1;
		}
		x = y = 0;

		const uint8_t* cptr1 = cptr + _x * bpp + _y * cstride;
		const uint8_t* cptr2 = cptr + _x * bpp + _y * cstride;

		size_t n = MAXDIST;
		float mag;
		while( n-- ) {
			if( ( ( size_t ) ( _x + x ) ) < w && ( ( size_t ) ( _y + y ) ) < h ) {
				if( *( ( const float* ) cptr1 ) > 0.5f ) {
#ifndef GTLINEINPUT
					grad.x = *( ( float* ) ( dxptr + ( _x + x ) * dxbpp + ( _y + y ) * dxstride ) );
					grad.y = *( ( float* ) ( dyptr + ( _x + x ) * dybpp + ( _y + y ) * dystride ) );
					if( Math::abs( norm * grad ) >= COSMAX ) {
#else
					{
#endif
						dist = Math::sqrt( ( T ) ( Math::sqr( x ) + Math::sqr( y ) ) );
						return true;
					}
				}
			}
			if( ( ( size_t ) ( _x - x ) ) < w && ( ( size_t ) ( _y - y ) ) < h ) {
				if( *( ( const float* ) cptr2 ) > 0.5f ) {
#ifndef GTLINEINPUT
				grad.x = *( ( float* ) ( dxptr + ( _x - x ) * dxbpp + ( _y - y ) * dxstride ) );
				grad.y = *( ( float* ) ( dyptr + ( _x - x ) * dybpp + ( _y - y ) * dystride ) );
				if( Math::abs( norm * grad ) >= COSMAX ) {
#else
					{
#endif
						dist = - Math::sqrt( ( T ) ( Math::sqr( x ) + Math::sqr( y ) ) );
						return true;
					}
				}
			}
			e2 = 2 * err;
			if( e2 >= dy ) {
				err += dy;
				cptr1 += incx; cptr2 -= incx;
				x += sx;
			}
			if( e2 <= dx ) {
				err += dx;
				cptr1 += incy; cptr2 -= incy;
				y += sy;
			}
		}
		return false;
	}

	template<typename T>
	inline void FaceShape<T>::drawCurrent( GFX* g ) const
	{
		Vector2f pts[ 2 ];
		size_t i1, i2;

		for( size_t i = 0; i < _lsize; i++ ) {
			i1 = _lines[ i * 2 ];
			i2 = _lines[ i * 2 + 1 ];
			pts[ 0 ].x = _pts[ i1 * 2 ];
			pts[ 0 ].y = _pts[ i1 * 2 + 1 ];
			pts[ 1 ].x = _pts[ i2 * 2 ];
			pts[ 1 ].y = _pts[ i2 * 2 + 1 ];
			g->drawLines( pts, 1 );
		}
	}

	template<typename T>
	inline void FaceShape<T>::draw( GFX* g, Matrix3<T>& transform, Eigen::Matrix<T, Eigen::Dynamic, 1 >& p ) const
	{
		Vector2f lpts[ 2 ];
		Vector2<T> pt, pt2;
		size_t i1, i2;
		Eigen::Matrix<T, Eigen::Dynamic, 1 > pts;

		pts = _mean + _pc * p;
		for( size_t i = 0; i < _ptsize; i++ ) {
			pt.x = pts[ i * 2 ];
			pt.y = pts[ i * 2 + 1 ];
			pt = transform * pt;
			pts[ i * 2 ] = pt.x;
			pts[ i * 2 + 1 ] = pt.y;
		}

		for( size_t i = 0; i < _lsize; i++ ) {
			i1 = _lines[ i * 2 ];
			i2 = _lines[ i * 2 + 1 ];
			lpts[ 0 ].x = pts[ i1 * 2 ];
			lpts[ 0 ].y = pts[ i1 * 2 + 1 ];
			lpts[ 1 ].x = pts[ i2 * 2 ];
			lpts[ 1 ].y = pts[ i2 * 2 + 1 ];
			g->drawLines( lpts, 1 );
		}
	}
}

#endif
