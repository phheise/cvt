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

#include <cvt/math/JointMeasurements.h>
#include <iostream>

namespace cvt
{
	void JointMeasurements::addMeasurementForEntity( size_t e0, size_t e1, size_t m )
	{
		MapType & map = _jointMeasForEntity[ e0 ];
		MapIterType iter = map.find( e1 );
		if( iter == map.end() ){
			std::set<size_t> tmpSet;
			tmpSet.insert( m );
			map.insert( std::pair<size_t, std::set<size_t> >( e1, tmpSet ) );
			_numBlocks++;		
		} else {
			iter->second.insert( m );
		}
	}

	void JointMeasurements::dumpMap() const
	{
		for( size_t i = 0; i < _jointMeasForEntity.size(); i++ ){
			const MapType & map = _jointMeasForEntity[ i ];

			ConstMapIterType mapIter = map.begin();
			const ConstMapIterType mapEnd = map.end();
			
			std::cout << "Constraints for camera " << i << std::endl;
			while( mapIter != mapEnd ){
				std::cout << "\t cam " << mapIter->first << ": ";
				std::set<size_t>::const_iterator pIter = mapIter->second.begin();
				const std::set<size_t>::const_iterator pIterEnd = mapIter->second.end();
				
				while( pIter != pIterEnd ){
					std::cout << *pIter << " "; 
					++pIter;
				}
				std::cout << std::endl;
				++mapIter;
			}
		}
		std::cout << std::endl;
	}
}
