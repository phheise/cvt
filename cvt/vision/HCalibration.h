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

#ifndef CVT_HCALIBRATION_H
#define CVT_HCALIBRATION_H

#include <cvt/math/Math.h>
#include <Eigen/Core>
#include <Eigen/SVD>
#include <Eigen/Cholesky>

namespace cvt {
	class HCalibration {
		public:
			HCalibration();

			void reset();
			void addHomography( const Eigen::Matrix<double,3,3>& matrix );
			bool calibration( Eigen::Matrix<double,3,3>& calib  );
			size_t size() const;

		private:
			HCalibration( const HCalibration& );
			HCalibration& operator=( const HCalibration& );

			size_t _n;
			Eigen::Matrix<double,6,6> _V;
	};

	inline HCalibration::HCalibration() : _n( 0 )
	{
		_V.setZero();
	}

	inline void HCalibration::reset()
	{
		_n = 0;
		_V.setZero();
	}

	inline size_t HCalibration::size() const
	{
		return _n;
	}

	inline void HCalibration::addHomography( const Eigen::Matrix<double,3,3>& h )
	{
		Eigen::Matrix<double,6,1> v00;
		Eigen::Matrix<double,6,1> v01;
		Eigen::Matrix<double,6,1> v11;

		_n++;

		/*
			v_ij = hi0 hj0;  hi0 hj1 + hi1 hj0;  hi1 hj1;   hi2 hj0 + hi0 hj2;   hi2 hj1 + hi1 hj2;   hi2 hj2
		 */

		v00( 0 ) = h( 0, 0 ) * h( 0, 0 );
		v00( 1 ) = 2.0 * h( 0, 0 ) * h( 1, 0 );
		v00( 2 ) = h( 1, 0 ) * h( 1, 0 );
		v00( 3 ) = 2.0 * h( 0, 0 ) * h( 2, 0 );
		v00( 4 ) = 2.0 * h( 1, 0 ) * h( 2, 0 );
		v00( 5 ) = h( 2, 0 ) * h( 2, 0 );

		v01( 0 ) = h( 0, 0 ) * h( 0, 1 );
		v01( 1 ) = h( 0, 0 ) * h( 1, 1 ) + h( 1, 0 ) * h( 0, 1 );
		v01( 2 ) = h( 1, 0 ) * h( 1, 1 );
		v01( 3 ) = h( 2, 0 ) * h( 0, 1 ) + h( 0, 0 ) * h( 2, 1 );
		v01( 4 ) = h( 2, 0 ) * h( 1, 1 ) + h( 1, 0 ) * h( 2, 1 );
		v01( 5 ) = h( 2, 0 ) * h( 2, 1 );

		v11( 0 ) = h( 0, 1 ) * h( 0, 1 );
		v11( 1 ) = 2.0 * h( 0, 1 ) * h( 1, 1 );
		v11( 2 ) = h( 1, 1 ) * h( 1, 1 );
		v11( 3 ) = 2.0 * h( 0, 1 ) * h( 2, 1 );
		v11( 4 ) = 2.0 * h( 1, 1 ) * h( 2, 1 );
		v11( 5 ) = h( 2, 1 ) * h( 2, 1 );

		_V += ( v01 * v01.transpose() );
		v00 -= v11;
		_V += ( v00 * v00.transpose() );
	}


	inline bool HCalibration::calibration( Eigen::Matrix<double,3,3>& calib  )
	{
		if( _n < 3 )
			return false;

		Eigen::JacobiSVD<Eigen::Matrix<double,6,6> > svd( _V , Eigen::ComputeFullU | Eigen::ComputeFullV );
		Eigen::Matrix<double,6,1> b = svd.matrixV().col( 5 );

/*		Eigen::Matrix3d A;
		A( 0, 0 ) = b( 0 );
		A( 0, 1 ) = A( 1, 0 ) = b( 1 );
		A( 1, 1 ) = b( 2 );
		A( 0, 2 ) = A( 2, 0 ) = b( 3 );
		A( 1, 2 ) = A( 2, 1 ) = b( 4 );
		A( 2, 2 ) = b( 5 );

		Eigen::LLT<Eigen::Matrix3d > llt( A );
		Eigen::Matrix3d L = llt.matrixL().inverse().transpose();
		calib = L / L( 2, 2 ); */

		double lambda;
		calib.setZero();
		calib( 1, 2 ) = ( b( 1 ) * b( 3 ) - b( 0 ) * b( 4 ) ) / ( b( 0 ) * b( 2 ) - Math::sqr( b( 1 ) ) );
		lambda = b( 5 ) - ( Math::sqr( b( 3 ) )  + calib( 1, 2 ) * ( b( 1 ) * b( 3 ) - b( 0 ) * b( 4 ) ) ) / b( 0 );
		calib( 0, 0 ) = Math::sqrt( lambda / b( 0 ) );
		calib( 1, 1 ) = Math::sqrt( ( lambda * b( 0 ) ) / ( b( 0 ) * b( 2 ) - Math::sqr( b( 1 ) ) ) );
		calib( 0, 1 ) = ( -b( 1 ) * Math::sqr( calib( 0, 0 ) ) * calib( 1, 1 ) ) / lambda;
		calib( 0, 2 ) = calib( 0, 1 ) * calib( 1, 2 ) / calib( 1, 1 ) - b( 3 ) * Math::sqr( calib( 0, 0 ) ) / lambda;
		calib( 2, 2 ) = 1.0f;
		return true;
	}
}



#endif
