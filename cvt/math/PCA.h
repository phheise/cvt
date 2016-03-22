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

#ifndef CVT_PCA_H
#define CVT_PCA_H

#include <Eigen/Core>
#include <Eigen/SVD>

namespace cvt {

	/**
	  \ingroup Math
	*/
	template<typename T>
	class PCA
	{
		public:
			PCA( size_t dimension );
			~PCA();

			size_t dimension() const;

			void addSample( const Eigen::Matrix<T, Eigen::Dynamic, 1>& sample );
			void addSample( const T* sample );
//			void addSample( T* sample, size_t width, size_t stride );

			void mean( Eigen::Matrix<T, Eigen::Dynamic, 1>& m ) const;
			void principleComponents(Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& p) const;
			void principleComponents( Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& p,  Eigen::Matrix<T, Eigen::Dynamic, 1>& svalues ) const;

		private:
			PCA();
			PCA( const PCA& pca );

			size_t _dimension;
			size_t _n;

			Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> _x;
			Eigen::Matrix<T, Eigen::Dynamic, 1>				 _mean;
	};

	template<typename T>
	inline PCA<T>::PCA( size_t dimension ) : _dimension( dimension ), _n( 0 ), _x( ( int ) dimension, ( int ) dimension ), _mean( dimension )
	{
		_mean.setZero();
		_x.setZero();
	}

	template<typename T>
	inline PCA<T>::~PCA()
	{
	}


	template<typename T>
	size_t PCA<T>::dimension() const
	{
		return _dimension;
	}

	template<typename T>
	inline void PCA<T>::addSample( const Eigen::Matrix<T, Eigen::Dynamic, 1>& sample )
	{
		if( ( size_t )sample.rows() != _dimension )
			return;
		_mean += sample;
		_x	  += sample * sample.transpose();
		_n++;
	}

	template<typename T>
	inline void PCA<T>::addSample( const T* data )
	{
		Eigen::Map< Eigen::Matrix<T, Eigen::Dynamic, 1> > sample( ( T* )data, _dimension );
		_mean += sample;
		_x	  += sample * sample.transpose();
		_n++;
	}


	template<typename T>
	inline void PCA<T>::mean( Eigen::Matrix<T, Eigen::Dynamic, 1>& m ) const
	{
		if( ( size_t ) m.rows() != _dimension || !_n )
			return;
		T invn = 1 / ( T ) _n;
		m = _mean * invn;
	}


	template<typename T>
	void PCA<T>::principleComponents(Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& p) const
	{
		if( ( size_t ) p.rows() != _dimension || ( size_t ) p.cols() != _dimension || !_n )
			return;

		T invn = 1 / ( T ) _n;
		Eigen::Matrix<T, Eigen::Dynamic, 1> m( _dimension );
		mean( m );

		p = _x * invn - m * m.transpose();
		Eigen::JacobiSVD<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> > svd( p, Eigen::ComputeFullU | Eigen::ComputeFullV );
		p = svd.matrixU();
	}

	template<typename T>
	void PCA<T>::principleComponents( Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& p,  Eigen::Matrix<T, Eigen::Dynamic, 1>& svalues ) const
	{
		if( ( size_t ) p.rows() != _dimension || ( size_t ) p.cols() != _dimension || !_n )
			return;

		T invn = 1 / ( T ) _n;
		Eigen::Matrix<T, Eigen::Dynamic, 1> m( _dimension );
		mean( m );

		p = _x * invn - m * m.transpose();
		Eigen::JacobiSVD<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> > svd( p, Eigen::ComputeFullU | Eigen::ComputeFullV );
		svalues = svd.singularValues();
		p = svd.matrixU();
	}

	typedef PCA<float> PCAf;
	typedef PCA<double> PCAd;
}

#endif
