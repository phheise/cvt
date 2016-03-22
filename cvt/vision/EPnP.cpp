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

#include <cvt/vision/EPnP.h>

namespace cvt
{
	template <typename T>
	EPnP<T>::EPnP( const PointSet<3, T> & pointSet ) : _points3D( pointSet )
	{
		// controlpoints
		computeControlPoints( pointSet );
		computeBarycentricCoords( pointSet );
	}

	template <typename T>
	void EPnP<T>::solve( Matrix4<T> & transform, const PointSet<2, T> & pointSet, const Matrix3<T> & K ) const
	{
		Eigen::Matrix<T, 12, 12> A;

		// build the matrix (M^T*M in the paper)
		buildSystem( A, pointSet, K );

		// compute the svd: we know that it's symmetric and square, so no preconditioning!
		Eigen::JacobiSVD<Eigen::Matrix<T, 12, 12>, Eigen::NoQRPreconditioner> svd( A, Eigen::ComputeFullU | Eigen::ComputeFullV );
		const Eigen::Matrix<T, 12, 12> & V = svd.matrixV();
		const Eigen::Matrix<T, 12, 1> & v0 = V.col( 11 );
		const Eigen::Matrix<T, 12, 1> & v1 = V.col( 10 );
		const Eigen::Matrix<T, 12, 1> & v2 = V.col(  9 );
		const Eigen::Matrix<T, 12, 1> & v3 = V.col(  8 );

		// distances of the control points
		Eigen::Matrix<T, 6, 1> controlPointDistances, cpDistSqr;
		computeControlPointsDelta( controlPointDistances );

		Matrix4<T> Tout[ 3 ];
		T		   err[ 3 ];
		Matrix4<T> K44;
		K44.setZero();

		for( size_t i = 0; i < 3; i++ )
			for( size_t k = 0; k < 3; k++ )
				K44[ i ][ k ] = K[ i ][ k ];
		K44[ 3 ][ 3 ] = 1;

		Eigen::Matrix<T, 6, 10> constraintMat;
		fillConstraintMatrix( constraintMat, v0, v1, v2, v3 );
		cpDistSqr = controlPointDistances.array() * controlPointDistances.array();

		Eigen::Matrix<T, 12, 1> combinedV;
		Eigen::Matrix<T, 4, 1> betas;

		// N=2;
		solveBetaN2( betas, constraintMat, cpDistSqr );
		combinedV = betas[ 0 ] * v0 + betas[ 1 ] * v1;
		computePose( Tout[ 0 ], combinedV, _controlPoints );
		err[ 0 ] = reprojectionError( Tout[ 0 ], K44, _points3D, pointSet );

		// N=3;
		solveBetaN3( betas, constraintMat, cpDistSqr );
		combinedV = betas[ 0 ] * v0 + betas[ 1 ] * v1 + betas[ 2 ] * v2;
		computePose( Tout[ 1 ], combinedV, _controlPoints );
		err[ 1 ] = reprojectionError( Tout[ 1 ], K44, _points3D, pointSet );

		// N=4;
		solveBetaN4( betas, constraintMat, cpDistSqr );
		combinedV = betas[ 0 ] * v0 + betas[ 1 ] * v1 + betas[ 2 ] * v2 + betas[ 3 ] * v3;
		computePose( Tout[ 2 ], combinedV, _controlPoints );
		err[ 2 ] = reprojectionError( Tout[ 2 ], K44, _points3D, pointSet );

		size_t i = 0;
		if( err[ 1 ] < err[ 0 ] ){
			if( err[ 2 ] < err[ 1 ] )
				i = 2;
			else
				i = 1;
		} else {
			if( err[ 2 ] < err[ 0 ] )
				i = 2;
		}

		transform = Tout[ i ];
	}

	template <typename T>
	void EPnP<T>::computeControlPoints( const PointSet<3, T> & ptSet )
	{
		// the centroid
		PCA<T> pca( 3 );

		Eigen::Matrix<T, Eigen::Dynamic, 1> p( 3, 1 );

		Vector3<T> mean = ptSet.mean();
		_controlPoints.add( mean );

		for( size_t i = 0; i < ptSet.size(); i++ ){
			p[ 0 ] = ptSet[ i ].x;
			p[ 1 ] = ptSet[ i ].y;
			p[ 2 ] = ptSet[ i ].z;
			pca.addSample( p );
		}

		pca.mean( p );
		Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> comp( 3, 3 );
		Eigen::Matrix<T, Eigen::Dynamic, 1> sVals( 3 );
		pca.principleComponents( comp, sVals );

		_controlPoints.add( Vector3<T>( ( T )0, ( T )0, ( T )0 ) );
		_controlPoints.add( Vector3<T>( ( T )0, ( T )0, ( T )0 ) );
		_controlPoints.add( Vector3<T>( ( T )0, ( T )0, ( T )0 ) );

		T scale0 = Math::sqrt( sVals[ 0 ] );

		_controlPoints[ 1 ].x = scale0 * comp( 0, 0 ) + p[ 0 ];
		_controlPoints[ 1 ].y = scale0 * comp( 1, 0 ) + p[ 1 ];
		_controlPoints[ 1 ].z = scale0 * comp( 2, 0 ) + p[ 2 ];

		T scale1 = Math::sqrt( sVals[ 1 ] );
		_controlPoints[ 2 ].x = scale1 * comp( 0, 1 ) + p[ 0 ];
		_controlPoints[ 2 ].y = scale1 * comp( 1, 1 ) + p[ 1 ];
		_controlPoints[ 2 ].z = scale1 * comp( 2, 1 ) + p[ 2 ];

		T scale2 = Math::sqrt( sVals[ 2 ] );
		_controlPoints[ 3 ].x = scale2 * comp( 0, 2 ) + p[ 0 ];
		_controlPoints[ 3 ].y = scale2 * comp( 1, 2 ) + p[ 1 ];
		_controlPoints[ 3 ].z = scale2 * comp( 2, 2 ) + p[ 2 ];

		_pcInv[ 0 ][ 0 ] = comp( 0, 0 ) / scale0; _pcInv[ 0 ][ 1 ] = comp( 1, 0 ) / scale0;	_pcInv[ 0 ][ 2 ] = comp( 2, 0 ) / scale0;
		_pcInv[ 1 ][ 0 ] = comp( 0, 1 ) / scale1; _pcInv[ 1 ][ 1 ] = comp( 1, 1 ) / scale1;	_pcInv[ 1 ][ 2 ] = comp( 2, 1 ) / scale1;
		_pcInv[ 2 ][ 0 ] = comp( 0, 2 ) / scale2; _pcInv[ 2 ][ 1 ] = comp( 1, 2 ) / scale2;	_pcInv[ 2 ][ 2 ] = comp( 2, 2 ) / scale2;
	}

	template <typename T>
	void EPnP<T>::computeBarycentricCoords( const PointSet<3, T> & ptSet )
	{
		_barycentricCoords.clear();
		_barycentricCoords.reserve( ptSet.size() );
		Vector3<T> tmp;
		T alpha0;
		for( size_t i = 0; i < ptSet.size(); i++ ){
			tmp = _pcInv * ( ptSet[ i ] - _controlPoints[ 0 ] );
			alpha0 = ( T )1 - tmp[ 0 ] - tmp[ 1 ] - tmp[ 2 ];
			_barycentricCoords.push_back( Vector4<T>( alpha0, tmp.x, tmp.y, tmp.z ) );
		}
	}

	template<typename T>
	void EPnP<T>::buildSystem( Eigen::Matrix<T, 12, 12> & A, const PointSet<2, T> & points2D, const Matrix3<T> & K ) const
	{
		// build the matrix:
		A.setZero();

		Eigen::Matrix<T, 12, 1> l0, l1;
		for( size_t i = 0; i < points2D.size(); i++ ){
			for( size_t k = 0; k < 4; k++ ){
				l0[ k * 3 ]		= _barycentricCoords[ i ][ k ] * K[ 0 ][ 0 ];
				l0[ k * 3 + 1 ] = _barycentricCoords[ i ][ k ] * K[ 0 ][ 1 ];
				l0[ k * 3 + 2 ] = _barycentricCoords[ i ][ k ] * ( K[ 0 ][ 2 ] - points2D[ i ].x );
				l1[ k * 3 ]		= ( T )0;
				l1[ k * 3 + 1 ] = _barycentricCoords[ i ][ k ] * K[ 1 ][ 1 ];
				l1[ k * 3 + 2 ] = _barycentricCoords[ i ][ k ] * ( K[ 1 ][ 2 ] - points2D[ i ].y );
			}

			A += l0 * l0.transpose();
			A += l1 * l1.transpose();
		}

	}

	// N=2: we need to select comb. between 00, 01 and 11
	template <typename T>
	void EPnP<T>::solveBetaN2( Eigen::Matrix<T, 4,  1> & betas,
							   const Eigen::Matrix<T, 6, 10> & C,
							   const Eigen::Matrix<T, 6,  1> & dSqr ) const
	{
		Eigen::Matrix<T, 6, 3> L;

		L.template block<6, 2>( 0, 0 ) = C.template block<6, 2>( 0, 0 );
		L.col( 2 ) = C.col( 4 );

		Eigen::Matrix<T, 3, 1> x;

		x = L.jacobiSvd( Eigen::ComputeFullU | Eigen::ComputeFullV ).solve( dSqr );

		if( x[ 0 ] < 0 ){
			betas[ 0 ] = Math::sqrt( -x[ 0 ] );
			betas[ 1 ] = ( x[ 2 ] < 0 )? Math::sqrt( -x[ 2 ] ) : 0;
		} else {
			betas[ 0 ] = Math::sqrt( x[ 0 ] );
			betas[ 1 ] = ( x[ 2 ] > 0 )? Math::sqrt( x[ 2 ] ) : 0;
		}

		if( x[ 1 ] < 0 )
			betas[ 0 ] = -betas[ 0 ];

		betas[ 2 ] = betas[ 3 ] = 0;
	}


	// N=3: we need to select comb. between 00, 01, 02, 11, 12
	template <typename T>
	void EPnP<T>::solveBetaN3( Eigen::Matrix<T, 4,  1> & betas,
							   const Eigen::Matrix<T, 6, 10> & C,
							   const Eigen::Matrix<T, 6,  1> & dSqr ) const
	{
		Eigen::Matrix<T, 6, 5> L;

		L.template block<6, 3>( 0, 0 ) = C.template block<6, 3>( 0, 0 );
		L.template block<6, 2>( 0, 3 ) = C.template block<6, 2>( 0, 4 );

		Eigen::Matrix<T, 5, 1> x;
		x = L.jacobiSvd( Eigen::ComputeFullU | Eigen::ComputeFullV ).solve( dSqr );

		if( x[ 0 ] < 0 ){
			betas[ 0 ] = Math::sqrt( -x[ 0 ] );
			betas[ 1 ] = ( x[ 3 ] < 0 )? Math::sqrt( -x[ 3 ] ) : 0;
		} else {
			betas[ 0 ] = Math::sqrt( x[ 0 ] );
			betas[ 1 ] = ( x[ 3 ] > 0 )? Math::sqrt( x[ 3 ] ) : 0;
		}

		if( x[ 1 ] < 0 )
			betas[ 0 ] = -betas[ 0 ];

		betas[ 2 ] = x[ 2 ] / betas[ 0 ];
	}

	// N=4: we need to select comb. between 00, 01, 02, 03
	template <typename T>
	void EPnP<T>::solveBetaN4( Eigen::Matrix<T, 4,  1> & betas,
							   const Eigen::Matrix<T, 6, 10> & C,
							   const Eigen::Matrix<T, 6,  1> & dSqr ) const
	{
		Eigen::Matrix<T, 6, 4> L;
		L.template block<6, 4>( 0, 0 ) = C.template block<6, 4>( 0, 0 );
		Eigen::Matrix<T, 4, 1> x;
		x = L.jacobiSvd( Eigen::ComputeFullU | Eigen::ComputeFullV ).solve( dSqr );

		if( x[ 0 ] < 0 ){
			betas[ 0 ] = Math::sqrt( -x[ 0 ] );
			betas[ 1 ] = -x[ 1 ] / betas[ 0 ];
			betas[ 2 ] = -x[ 2 ] / betas[ 0 ];
			betas[ 3 ] = -x[ 3 ] / betas[ 0 ];
		} else {
			betas[ 0 ] = Math::sqrt( x[ 0 ] );
			betas[ 1 ] = x[ 1 ] / betas[ 0 ];
			betas[ 2 ] = x[ 2 ] / betas[ 0 ];
			betas[ 3 ] = x[ 3 ] / betas[ 0 ];
		}
	}

	template <typename T>
	void EPnP<T>::fillConstraintMatrix( Eigen::Matrix<T, 6, 10> & C,
										const Eigen::Matrix<T, 12, 1> & v0,
										const Eigen::Matrix<T, 12, 1> & v1,
										const Eigen::Matrix<T, 12, 1> & v2,
										const Eigen::Matrix<T, 12, 1> & v3 ) const
	{
		// deltas of the components
		Eigen::Matrix<T, 3, 1> d0, d1, d2, d3;

		// c0 - c1
		d0 = v0.template head<3>() - v0.template segment<3>( 3 );
		d1 = v1.template head<3>() - v1.template segment<3>( 3 );
		d2 = v2.template head<3>() - v2.template segment<3>( 3 );
		d3 = v3.template head<3>() - v3.template segment<3>( 3 );
		C( 0, 0 ) = d0.dot( d0 );
		C( 0, 1 ) = 2 * d0.dot( d1 );
		C( 0, 2 ) = 2 * d0.dot( d2 );
		C( 0, 3 ) = 2 * d0.dot( d3 );
		C( 0, 4 ) = d1.dot( d1 );
		C( 0, 5 ) = 2 * d1.dot( d2 );
		C( 0, 6 ) = 2 * d1.dot( d3 );
		C( 0, 7 ) = d2.dot( d2 );
		C( 0, 8 ) = 2 * d2.dot( d3 );
		C( 0, 9 ) = d3.dot( d3 );

		// c0 - c2
		d0 = v0.template head<3>() - v0.template segment<3>( 6 );
		d1 = v1.template head<3>() - v1.template segment<3>( 6 );
		d2 = v2.template head<3>() - v2.template segment<3>( 6 );
		d3 = v3.template head<3>() - v3.template segment<3>( 6 );
		C( 1, 0 ) = d0.dot( d0 );
		C( 1, 1 ) = 2 * d0.dot( d1 );
		C( 1, 2 ) = 2 * d0.dot( d2 );
		C( 1, 3 ) = 2 * d0.dot( d3 );
		C( 1, 4 ) = d1.dot( d1 );
		C( 1, 5 ) = 2 * d1.dot( d2 );
		C( 1, 6 ) = 2 * d1.dot( d3 );
		C( 1, 7 ) = d2.dot( d2 );
		C( 1, 8 ) = 2 * d2.dot( d3 );
		C( 1, 9 ) = d3.dot( d3 );

		// c0 - c3
		d0 = v0.template head<3>() - v0.template tail<3>();
		d1 = v1.template head<3>() - v1.template tail<3>();
		d2 = v2.template head<3>() - v2.template tail<3>();
		d3 = v3.template head<3>() - v3.template tail<3>();
		C( 2, 0 ) = d0.dot( d0 );
		C( 2, 1 ) = 2 * d0.dot( d1 );
		C( 2, 2 ) = 2 * d0.dot( d2 );
		C( 2, 3 ) = 2 * d0.dot( d3 );
		C( 2, 4 ) = d1.dot( d1 );
		C( 2, 5 ) = 2 * d1.dot( d2 );
		C( 2, 6 ) = 2 * d1.dot( d3 );
		C( 2, 7 ) = d2.dot( d2 );
		C( 2, 8 ) = 2 * d2.dot( d3 );
		C( 2, 9 ) = d3.dot( d3 );

		// c1 - c2
		d0 = v0.template segment<3>( 3 ) - v0.template segment<3>( 6 );
		d1 = v1.template segment<3>( 3 ) - v1.template segment<3>( 6 );
		d2 = v2.template segment<3>( 3 ) - v2.template segment<3>( 6 );
		d3 = v3.template segment<3>( 3 ) - v3.template segment<3>( 6 );
		C( 3, 0 ) = d0.dot( d0 );
		C( 3, 1 ) = 2 * d0.dot( d1 );
		C( 3, 2 ) = 2 * d0.dot( d2 );
		C( 3, 3 ) = 2 * d0.dot( d3 );
		C( 3, 4 ) = d1.dot( d1 );
		C( 3, 5 ) = 2 * d1.dot( d2 );
		C( 3, 6 ) = 2 * d1.dot( d3 );
		C( 3, 7 ) = d2.dot( d2 );
		C( 3, 8 ) = 2 * d2.dot( d3 );
		C( 3, 9 ) = d3.dot( d3 );

		// c1 - c3
		d0 = v0.template segment<3>( 3 ) - v0.template tail<3>();
		d1 = v1.template segment<3>( 3 ) - v1.template tail<3>();
		d2 = v2.template segment<3>( 3 ) - v2.template tail<3>();
		d3 = v3.template segment<3>( 3 ) - v3.template tail<3>();
		C( 4, 0 ) = d0.dot( d0 );
		C( 4, 1 ) = 2 * d0.dot( d1 );
		C( 4, 2 ) = 2 * d0.dot( d2 );
		C( 4, 3 ) = 2 * d0.dot( d3 );
		C( 4, 4 ) = d1.dot( d1 );
		C( 4, 5 ) = 2 * d1.dot( d2 );
		C( 4, 6 ) = 2 * d1.dot( d3 );
		C( 4, 7 ) = d2.dot( d2 );
		C( 4, 8 ) = 2 * d2.dot( d3 );
		C( 4, 9 ) = d3.dot( d3 );

		// c2 - c3
		d0 = v0.template segment<3>( 6 ) - v0.template tail<3>();
		d1 = v1.template segment<3>( 6 ) - v1.template tail<3>();
		d2 = v2.template segment<3>( 6 ) - v2.template tail<3>();
		d3 = v3.template segment<3>( 6 ) - v3.template tail<3>();
		C( 5, 0 ) = d0.dot( d0 );
		C( 5, 1 ) = 2 * d0.dot( d1 );
		C( 5, 2 ) = 2 * d0.dot( d2 );
		C( 5, 3 ) = 2 * d0.dot( d3 );
		C( 5, 4 ) = d1.dot( d1 );
		C( 5, 5 ) = 2 * d1.dot( d2 );
		C( 5, 6 ) = 2 * d1.dot( d3 );
		C( 5, 7 ) = d2.dot( d2 );
		C( 5, 8 ) = 2 * d2.dot( d3 );
		C( 5, 9 ) = d3.dot( d3 );
	}

	template <typename T>
	void EPnP<T>::computeControlPointsDelta( Eigen::Matrix<T, 6, 1> & cpDelta ) const
	{
		cpDelta[ 0 ] = ( _controlPoints[ 0 ] - _controlPoints[ 1 ] ).length();
		cpDelta[ 1 ] = ( _controlPoints[ 0 ] - _controlPoints[ 2 ] ).length();
		cpDelta[ 2 ] = ( _controlPoints[ 0 ] - _controlPoints[ 3 ] ).length();
		cpDelta[ 3 ] = ( _controlPoints[ 1 ] - _controlPoints[ 2 ] ).length();
		cpDelta[ 4 ] = ( _controlPoints[ 1 ] - _controlPoints[ 3 ] ).length();
		cpDelta[ 5 ] = ( _controlPoints[ 2 ] - _controlPoints[ 3 ] ).length();
	}

	template <typename T>
	void EPnP<T>::computePose( Matrix4<T> & transform,
							   const Eigen::Matrix<T, 12, 1> & estimatedCoords,
							   const PointSet<3, T> & controlPoints ) const
	{
		PointSet<3, T> estimated;
		estimated.add( Vector3<T>( estimatedCoords[ 0 ], estimatedCoords[  1 ], estimatedCoords[  2 ] ) );
		estimated.add( Vector3<T>( estimatedCoords[ 3 ], estimatedCoords[  4 ], estimatedCoords[  5 ] ) );
		estimated.add( Vector3<T>( estimatedCoords[ 6 ], estimatedCoords[  7 ], estimatedCoords[  8 ] ) );
		estimated.add( Vector3<T>( estimatedCoords[ 9 ], estimatedCoords[ 10 ], estimatedCoords[ 11 ] ) );

		transform = controlPoints.alignRigid( estimated );
	}

	template <typename T>
	T EPnP<T>::reprojectionError( const Matrix4<T> & transform,
								  const Matrix4<T> & K44,
								  const PointSet<3, T> & p3d,
								  const PointSet<2, T> & p2d ) const
	{
		T error = ( T )0;
		Matrix4<T> P = K44 * transform;
		Vector3<T> tmp;
		Vector2<T> proj;

		for( size_t i = 0, n = p3d.size(); i < n; i++ ){
			tmp = P * p3d[ i ];
			proj[ 0 ] = tmp[ 0 ] / tmp[ 2 ];
			proj[ 1 ] = tmp[ 1 ] / tmp[ 2 ];

			error += ( proj	- p2d[ i ] ).lengthSqr();
		}

		return error;
	}

	template class EPnP<float>;
	template class EPnP<double>;
}
