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

#ifndef CVT_SPARSE_BLOCK_MATRIX_H
#define CVT_SPARSE_BLOCK_MATRIX_H

namespace cvt
{
	template<size_t bRows, size_t bCols>
	class SparseBlockMatrix
	{
		public:
			typedef typename Eigen::Matrix<double, bRows, bCols> BlockMatType;
			
			SparseBlockMatrix();
			~SparseBlockMatrix();


			void resize( size_t numRowBlocks, size_t numColBlocks );

			bool containsBlock( size_t row, size_t col ) const;

			BlockMatType&	block( size_t row, size_t col );

			size_t			numBlockRows() const { return _numRows; }
			size_t			numBlockCols() const { return _numCols; }

		private:
			int*	_filled;
			size_t	_numRows;
			size_t	_numCols;
			std::vector<BlockMatType, Eigen::aligned_allocator<BlockMatType> >	_blocks;
	};

	template <size_t bRows, size_t bCols>
	inline SparseBlockMatrix<bRows, bCols>::SparseBlockMatrix() :
		_filled( 0 ),
		_numRows( 0 ),
		_numCols( 0 )
	{
	}
	
	template <size_t bRows, size_t bCols>
	inline SparseBlockMatrix<bRows, bCols>::~SparseBlockMatrix() 
	{
		if( _filled ){
			delete[] _filled;
		}
	}

	template <size_t bRows, size_t bCols>
	inline bool SparseBlockMatrix<bRows, bCols>::containsBlock( size_t row, size_t col ) const
	{
		return _filled[ row * _numCols + col ] != -1;
	}

	template <size_t bRows, size_t bCols>
	inline Eigen::Matrix<double, bRows, bCols>& SparseBlockMatrix<bRows, bCols>::block( size_t r, size_t c )
	{
		int* a = _filled + ( r * _numCols + c );

		if( *a == -1 ){
			*a = _blocks.size();
			_blocks.push_back( BlockMatType() );
			_blocks.back().setZero();
		}

		return _blocks[ *a ];
	}
	
	template <size_t bRows, size_t bCols>
	inline void SparseBlockMatrix<bRows, bCols>::resize( size_t rows, size_t cols ) 	
	{
		if( _numRows == rows && _numCols == cols ) {
			_blocks.clear();
			size_t n = _numRows * _numCols;
			memset( _filled, -1, sizeof( int ) * n );
			return;
		} else {
			delete[] _filled;
			_numRows = rows; 
			_numCols = cols;
			size_t n = _numRows * _numCols;
			_filled = new int[ n ];
			memset( _filled, -1, sizeof( int ) * n );
			_blocks.clear();
		}
	}
}

#endif
