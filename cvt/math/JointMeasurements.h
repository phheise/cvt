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

#ifndef CVT_JOINTMEASUREMENTS_H
#define CVT_JOINTMEASUREMENTS_H

#include <map>
#include <set>
#include <vector>
#include <iostream>

namespace cvt
{
	class JointMeasurements
	{
		public:

			typedef std::map< size_t, std::set<size_t> > MapType;
			typedef MapType::iterator					 MapIterType;
			typedef MapType::const_iterator				 ConstMapIterType;

			JointMeasurements();

			void resize( size_t n );
			size_t size() const { return _jointMeasForEntity.size(); }

			void addMeasurementForEntity( size_t e0, size_t e1, size_t m );

			/* Iterators */
			ConstMapIterType secondEntityIteratorBegin( size_t e0 ) const { return _jointMeasForEntity[ e0 ].begin(); }
			ConstMapIterType secondEntityIteratorEnd( size_t e0 )   const { return _jointMeasForEntity[ e0 ].end(); }

			size_t	numBlocks() const { return _numBlocks; }
		private:
			/*
				vector: numEntities
			    map:	id ->				secondEntity
						std::set<size_t>	set of joint measurement ids		
			 */
			std::vector< MapType > _jointMeasForEntity;
			size_t				   _numBlocks;

			void dumpMap() const;

	};

	inline JointMeasurements::JointMeasurements() :
		_numBlocks( 0 )
	{
	}

	inline void JointMeasurements::resize( size_t n )
	{
		_jointMeasForEntity.clear();
		_jointMeasForEntity.resize( n );
		_numBlocks = 0;
	}

}

#endif
