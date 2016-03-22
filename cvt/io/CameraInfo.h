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

#ifndef CVT_CAMERAINFO_H
#define CVT_CAMERAINFO_H

#include <vector>
#include <fstream>
#include <cvt/gfx/Image.h>
#include <cvt/io/CameraMode.h>
#include <cvt/io/CameraModeSet.h>
#include <cvt/util/String.h>

namespace cvt {

	enum CameraType
	{
		CAMERATYPE_V4L2,
		CAMERATYPE_QTKIT,
		CAMERATYPE_DC1394,
		CAMERATYPE_UEYE,
		CAMERATYPE_OPENNI,
		CAMERATYPE_OPENNI2
	};

	class CameraInfo
	{
		public:
			CameraInfo();
			CameraInfo( const CameraInfo& );
			CameraInfo& operator=( const CameraInfo& other );

			const String & name() const;
			void setName( const String & name );

			const String & identifier() const;
			void setIdentifier( const String & id );

			CameraType type() const;
			void setType( CameraType type );

			size_t index() const;
			void setIndex( size_t index );

			const CameraMode & mode( size_t index ) const;
			CameraMode bestMatchingMode( const IFormat & format, size_t width, size_t height, size_t fps ) const;
			void addMode( const CameraMode & m );
			size_t numModes() const;
			const CameraModeSet & modeSet() const;

		private:
			String		_name;
			String		_identifier;
			CameraType	_type;
			size_t		_index;
			CameraModeSet _modeSet;
	};

	inline CameraInfo::CameraInfo()
	{
	}

	inline CameraInfo::CameraInfo( const CameraInfo& other ) :
		_name( other._name ),
		_identifier( other._identifier ),
		_type( other._type ),
		_index( other._index ),
		_modeSet( other._modeSet )
	{}

	inline CameraInfo& CameraInfo::operator=( const CameraInfo& other )
	{
		_name = other.name();
		_identifier = other.identifier();
		_type = other.type();
		_index = other.index();
		_modeSet = other.modeSet();
		return *this;
	}

	inline const String& CameraInfo::name() const
	{
		return _name;
	}

	inline void CameraInfo::setName( const String & name )
	{
		_name = name;
	}

	inline const String & CameraInfo::identifier() const
	{
		return _identifier;
	}

	inline void CameraInfo::setIdentifier( const String & id )
	{
		_identifier = id;
	}

	inline CameraType CameraInfo::type() const
	{
		return _type;
	}

	inline void CameraInfo::setType( CameraType type )
	{
		_type = type;
	}

	inline size_t CameraInfo::index() const
	{
		return _index;
	}

	inline void CameraInfo::setIndex( size_t index )
	{
		_index = index;
	}

	inline const CameraMode & CameraInfo::mode( size_t index ) const
	{
		return _modeSet.mode( index );
	}


	inline CameraMode CameraInfo::bestMatchingMode( const IFormat & format, size_t width, size_t height, size_t fps ) const
	{
		return _modeSet.bestMatching( format, width, height, fps );
	}

	inline void CameraInfo::addMode( const CameraMode & m )
	{
		_modeSet.add( m );
	}

	inline size_t CameraInfo::numModes() const
	{
		return _modeSet.size();
	}

	inline const CameraModeSet & CameraInfo::modeSet() const
	{
		return _modeSet;
	}

	std::ostream& operator<<( std::ostream &out, const CameraInfo &info );
}

#endif
