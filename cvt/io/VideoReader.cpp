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

#include <cvt/io/VideoReader.h>

#include <cvt/io/FileSystem.h>
#include <cvt/util/SIMD.h>
#include <cvt/util/Exception.h>

#include <iostream>

extern "C" {
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
}

namespace cvt {

	VideoReader::VideoReader( const String & fileName, bool autoRewind ):
		_formatContext( 0 ),
		_codecContext( 0 ),
		_avStream( 0 ),
		_streamIndex( -1 ),
		_avFrame( 0 ),
		_frame( 0 ),
		_width( 0 ),
		_height( 0 ),
		_format( IFormat::BGRA_UINT8 ),
		_autoRewind( autoRewind )
	{
		if( !FileSystem::exists( fileName ) ){
			String message( "File does not exist: " );
			message += fileName;
			throw CVTException( message.c_str() );
		}
		av_register_all();

#if LIBAVFORMAT_VERSION_MAJOR == 52
        if( av_open_input_file( &_formatContext, fileName.c_str(), NULL, 0, NULL ) != 0 )
            throw CVTException( "Could not open file!" );

        if( av_find_stream_info( _formatContext ) < 0 )
            throw CVTException( "Could not find stream information" );
#else /* 54 */
		if( avformat_open_input( &_formatContext, fileName.c_str(), NULL, NULL ) < 0 )
			throw CVTException( "Could not open file!" );
		if( avformat_find_stream_info( _formatContext, NULL ) < 0 )
			throw CVTException( "Could not find stream information" );
#endif

		// find first video stream:
		for( unsigned int i = 0; i < _formatContext->nb_streams; i++ ){
			if( _formatContext->streams[ i ]->codec->codec_type == AVMEDIA_TYPE_VIDEO ){
				_streamIndex = i;
				_avStream = _formatContext->streams[ i ];
				break;
			}
		}
		if( _streamIndex == -1 )
			throw CVTException( "No video stream found in file" );

		_codecContext = _avStream->codec;
		_codec = avcodec_find_decoder( _codecContext->codec_id );

		if( _codec == NULL ){
			throw CVTException( "No appropriate codec found" );
		}

#if LIBAVCODEC_VERSION_MAJOR == 52
        if( avcodec_open( _codecContext, _codec ) < 0 )
            throw CVTException( "Could not open codec!" );
#else /* 54 */
		if( avcodec_open2( _codecContext, _codec, NULL ) < 0 )
			throw CVTException( "Could not open codec!" );
#endif

		_width = _codecContext->width;
		_height = _codecContext->height;

		updateFormat();

		_avFrame = avcodec_alloc_frame();
	}

	VideoReader::~VideoReader()
	{
		if( _frame )
			delete _frame;

		av_free( _avFrame );
		avcodec_close( _codecContext );
#if LIBAVFORMAT_VERSION_MAJOR == 52
		av_close_input_file( _formatContext );
#else /* silently assume 54*/
		avformat_close_input( &_formatContext );
#endif
	}

	void VideoReader::updateFormat()
	{
		// alloc frame:
		switch( _codecContext->pix_fmt ){
			case PIX_FMT_BGRA:
				_format = IFormat::BGRA_UINT8;
				break;
			case PIX_FMT_RGBA:
				_format = IFormat::RGBA_UINT8;
				break;
			case PIX_FMT_GRAY8:
				_format = IFormat::GRAY_UINT8;
				break;
			case PIX_FMT_GRAY16LE:
				_format = IFormat::GRAY_UINT16;
				break;
			case PIX_FMT_YUV422P:
				_format = IFormat::YUYV_UINT8;
				break;
			case PIX_FMT_UYVY422:
				_format = IFormat::UYVY_UINT8;
				break;
			case PIX_FMT_YUV420P:
				_format = IFormat::BGRA_UINT8;
				break;
			default:
				std::cout << "Pixelformat:" << (int)_codecContext->pix_fmt << std::endl;
				throw CVTException( "Cannot map Pixelformat to CVT Format!" );
		}
	}

	bool VideoReader::nextFrame( size_t )
	{
		int	frameFinished;

		AVPacket packet;
		int ret;
		while( ( ret = av_read_frame( _formatContext, &packet ) ) >= 0 ) {
			// Is this a packet from the video stream?
			if( packet.stream_index == _streamIndex ) {
				// Decode video frame
				avcodec_decode_video2( _codecContext, _avFrame, &frameFinished, &packet );

				// Did we get a video frame?
				if(frameFinished) {
					// decoded a new frame lying in _avFrame
					if( _codecContext->pix_fmt == PIX_FMT_YUV420P ) {
						if( !_frame )
							_frame = new Image( _width, _height, _format );

						SIMD* simd = SIMD::instance();

						uint8_t * dOrig;
						const uint8_t * src = _avFrame->data[ 0 ];
						const uint8_t *srcu;
						const uint8_t *srcv;

						size_t stridedst;
						uint8_t * dst = _frame->map( &stridedst );
						dOrig = dst;

						srcu = _avFrame->data[ 1 ];
						srcv = _avFrame->data[ 2 ];

						size_t n = _height >> 1;
						while( n-- ) {
							simd->Conv_YUV420u8_to_BGRAu8( dst, src, srcu, srcv, _width );
							src += _avFrame->linesize[ 0 ];
							dst += stridedst;
							simd->Conv_YUV420u8_to_BGRAu8( dst, src, srcu, srcv, _width );
							src += _avFrame->linesize[ 0 ];
							dst += stridedst;
							srcu += _avFrame->linesize[ 1 ];
							srcv += _avFrame->linesize[ 2 ];
						}
						_frame->unmap( dOrig );
					} else {
						delete _frame;
						_frame = new Image( _width, _height, _format, _avFrame->data[ 0 ], _avFrame->linesize[ 0 ] );
					}
					break;
				}
			}

			// Free the packet that was allocated by av_read_frame
			av_free_packet( &packet );
		}
		av_free_packet( &packet );

		if( ret < 0 && _autoRewind )
			this->rewind();
		return true;
	}

	void VideoReader::rewind()
	{
		av_seek_frame( _formatContext, _streamIndex, 0, AVSEEK_FLAG_BACKWARD );
	}

	size_t VideoReader::numFrames() const
	{
		// TODO: nb_frames might not be set ...
		if( _avStream->nb_frames == -1 ){
			return 0;
		}
		return ( size_t )_avStream->nb_frames;
	}
}

