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

#ifndef CVT_VIDEO_READER
#define CVT_VIDEO_READER

#include <cvt/util/String.h>
#include <cvt/io/VideoInput.h>
#include <cvt/gfx/Image.h>

struct AVFormatContext;
struct AVCodecContext;
struct AVStream;
struct AVCodec;
struct AVFrame;
struct AVPacket;

namespace cvt {

	class VideoReader : public VideoInput
	{
		public:
			VideoReader( const String & fileName, bool autoRewind = true );
			~VideoReader();

			size_t  width() const;
			size_t  height() const;
			const   IFormat & format() const;
			const   Image & frame() const;
			bool    nextFrame( size_t timeout = 0 );
			size_t	numFrames() const;

		private:
			AVFormatContext *	_formatContext;
			AVCodecContext *	_codecContext;
			AVStream *			_avStream;
			AVCodec *			_codec;
			int					_streamIndex;
			AVFrame *			_avFrame;
			Image *				_frame;
			AVPacket *			_packet;
			uint8_t *			_inBuffer;

			size_t				_width;
			size_t				_height;
			IFormat				_format;
			bool				_autoRewind;

			void updateFormat();
			void rewind();
	};

	inline size_t VideoReader::width() const
	{
		return _width;
	}

	inline size_t VideoReader::height() const
	{
		return _height;
	}

	inline const Image & VideoReader::frame() const
	{
		return *_frame;
	}

	inline const IFormat & VideoReader::format() const
	{
		return _format;
	}

}

#endif
