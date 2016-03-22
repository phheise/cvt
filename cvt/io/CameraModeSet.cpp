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

#include <cvt/io/CameraModeSet.h>
#include <cvt/math/Math.h>

namespace cvt {

	CameraModeSet::CameraModeSet()
	{
	}

	CameraModeSet::CameraModeSet( const CameraModeSet& other ) :
		_modes( other._modes )
	{
	}

	CameraModeSet::~CameraModeSet()
	{
	}

	CameraModeSet& CameraModeSet::operator=( const CameraModeSet& other )
	{
		_modes = other._modes;
		return *this;
	}

	void CameraModeSet::add( const CameraMode & m )
	{
		_modes.push_back( m );
	}


	CameraModeSet CameraModeSet::filterFormat( const IFormat & format ) const
	{
		CameraModeSet ret;
		for( size_t i = 0; i < _modes.size(); i++ ){
			const CameraMode & current = _modes[ i ];

			if( current.format == format )
				ret.add( current );
		}

		return ret;

	}

	CameraModeSet CameraModeSet::filterSize( size_t minWidth, size_t minHeight ) const
	{
		CameraModeSet ret;
		for( size_t i = 0; i < _modes.size(); i++ ){
			const CameraMode & current = _modes[ i ];

			if( current.width >= minWidth &&
			   current.height >= minHeight )
				ret.add( current );
		}

		return ret;

	}

	CameraModeSet CameraModeSet::filterFPS( size_t minFPS ) const
	{
		CameraModeSet ret;
		for( size_t i = 0; i < _modes.size(); i++ ){
			const CameraMode & current = _modes[ i ];

			if( current.fps >= minFPS )
				ret.add( current );
		}

		return ret;
	}

	CameraModeSet CameraModeSet::filter( const IFormat & format, size_t minWidth, size_t minHeight, size_t minFPS ) const
	{
		CameraModeSet ret;
		for( size_t i = 0; i < _modes.size(); i++ ){
			const CameraMode & current = _modes[ i ];

			if( current.format == format &&
			   current.width >= minWidth &&
			   current.height >= minHeight &&
			   current.fps >= minFPS )
				ret.add( current );
		}

		return ret;
	}

	size_t CameraModeSet::size() const
	{
		return _modes.size();
	}

	const CameraMode & CameraModeSet::mode( size_t index ) const
	{
        if( index >= _modes.size() ){
			throw CVTException( "Mode index out of bounds" );
		}

		return _modes[ index ];
	}

	CameraMode CameraModeSet::bestMatching( const IFormat & format, size_t width, size_t height, size_t fps  ) const
	{
		size_t dist, bestDist = 100000, bestIdx = 0;
		for( size_t i = 0; i < this->size(); i++ ) {
			const CameraMode & mode = this->mode( i );
			if( mode.format == format )
				dist = 0;
			else
				dist = 1;
			dist += Math::sqr( mode.width - width ) +
					Math::sqr( mode.height - height ) +
					Math::sqr( mode.fps - fps );

			if( dist < bestDist ){
				bestIdx = i;
				bestDist = dist;
			}
		}

		return this->mode( bestIdx );
	}

	std::ostream& operator<<( std::ostream &out, const CameraModeSet & info )
	{
		out << "\tModes:\n\t\t";
		for( size_t i = 0; i < info.size(); i++ ){
			out << info.mode( i ) << ", ";
			if( i != 0 && ( (i+1) % 2 ) == 0 && ( i + 1 ) < info.size() ){
				out << "\n\t\t";
			}
		}

		return out;
	}
}
