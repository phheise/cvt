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

#ifndef CVT_DESCRIPTOR_DATA_BASE_H
#define CVT_DESCRIPTOR_DATA_BASE_H

#include <vector>
#include <cvt/vision/features/FeatureDescriptor.h>
#include <cvt/vision/KLTPatch.h>
#include <cvt/math/GA2.h>

namespace cvt
{
	class DescriptorDatabase
	{
		public:
			typedef GA2<float> PatchPose;
            typedef KLTPatch<7, PatchPose> PatchType;
			DescriptorDatabase();
			~DescriptorDatabase();

			void						clear();
			void						addDescriptor( const FeatureDescriptor& desc, size_t id );
			void						addPatch( PatchType* patch, size_t id );
			const FeatureDescriptor&	descriptor( size_t id ) const;
			const PatchType*			patch( size_t id ) const;

			void descriptorsForIds( std::vector<FeatureDescriptor*>& descriptors,
									const std::vector<size_t>& ids );

			void patchesForIds( std::vector<PatchType*>& descriptors,
									  const std::vector<size_t>& ids );

			void descriptorsAndPatchesForIds( std::vector<FeatureDescriptor*>& descriptors,
											  std::vector<PatchType*>& patches,
											  const std::vector<size_t>& ids );

		private:
			std::vector<FeatureDescriptor*>	_descriptors;
			std::vector<PatchType*>			_patches;
	};

	inline DescriptorDatabase::DescriptorDatabase()
	{
        _descriptors.reserve( 10000 );
        _patches.reserve( 10000 );
	}

	inline DescriptorDatabase::~DescriptorDatabase()
	{
		for( size_t i = 0; i < _descriptors.size(); i++ ){
			if( _descriptors[ i ] != 0 )
				delete _descriptors[ i ];
		}
	}

	inline void DescriptorDatabase::clear()
	{
		_descriptors.clear();
	}

	inline void DescriptorDatabase::addDescriptor( const FeatureDescriptor& d, size_t id )
	{
		size_t numDesc = _descriptors.size();
		if( id < numDesc ){
			// already have this id -> update this descriptor
			if( _descriptors[ id ] != 0 ){
				*_descriptors[ id ] = d;
			} else {
				_descriptors[ id ] = d.clone();
			}
			return;
		}

		if( id == numDesc ){
			_descriptors.push_back( d.clone() );
			return;
		}

		// handle non-consecutive id insertion
		size_t lastId = numDesc;
		_descriptors.resize( ( id+1 ) );
		while( lastId < id )
			_descriptors[ lastId++ ] = 0;
		_descriptors[ id ] = d.clone();
	}

	inline void DescriptorDatabase::addPatch( PatchType* patch, size_t id )
	{
		size_t numPatches = _patches.size();
		if( id < numPatches ){
			// already have this id -> update this descriptor
			if( _patches[ id ] != 0 ){
				delete _patches[ id ];
			}
			_patches[ id ] = patch;
			return;
		}

		if( id == numPatches ){
			_patches.push_back( patch );
			return;
		}

		// handle non-consecutive id insertion
		size_t lastId = numPatches;
		_patches.resize( ( id+1 ) );
		while( lastId < id )
			_patches[ lastId++ ] = 0;
		_patches[ id ] = patch;
	}

	inline const FeatureDescriptor& DescriptorDatabase::descriptor( size_t id ) const
	{
		const FeatureDescriptor* des = _descriptors[ id ];
		if( des == 0 ){
			throw CVTException( "descriptor for requested id does not exist" );
		}
		return *des;
	}

	inline const DescriptorDatabase::PatchType* DescriptorDatabase::patch( size_t id ) const
	{
		const PatchType* p = _patches[ id ];
		if( p == 0 ){
			throw CVTException( "patch for requested id does not exist" );
		}
		return p;
	}

	inline void DescriptorDatabase::descriptorsForIds( std::vector<FeatureDescriptor*>& descriptors,
													   const std::vector<size_t>& ids )
	{
		descriptors.resize( ids.size() );
		for( size_t i = 0; i < ids.size(); ++i ){
			descriptors[ i ] = _descriptors[ ids[ i ] ];
		}
	}

	inline void DescriptorDatabase::patchesForIds( std::vector<PatchType*>& patches,
												   const std::vector<size_t>& ids )
	{
		patches.resize( ids.size() );
		for( size_t i = 0; i < ids.size(); ++i ){
			patches[ i ] = _patches[ ids[ i ] ];
		}
	}

	inline void DescriptorDatabase::descriptorsAndPatchesForIds( std::vector<FeatureDescriptor*>& descriptors,
																 std::vector<PatchType*>& patches,
																 const std::vector<size_t>& ids )
	{
		patches.resize( ids.size() );
		descriptors.resize( ids.size() );
		for( size_t i = 0; i < ids.size(); ++i ){
			size_t idx = ids[ i ];
			patches[ i ] = _patches[ idx ];
			descriptors[ i ] = _descriptors[ idx ];
		}
	}
}

#endif
