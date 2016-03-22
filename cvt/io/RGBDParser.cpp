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

#include <cvt/io/RGBDParser.h>

namespace cvt
{

    RGBDParser::RGBDParser( const String& folder, double maxStampDiff ) :
        _maxStampDiff( maxStampDiff ), // this is 50ms
        _folder( folder ),
        _idx( 0 )
    {
        if( _folder[ _folder.length() - 1 ] != '/' )
            _folder += "/";

        // intrinsics:
        String calibFile( _folder + "calib.xml" );
        if( FileSystem::exists( calibFile ) ){
            _calib.load( calibFile );
        } else {
            String msg;
            msg.sprintf( "Could not find calibration data for dataset: %s", calibFile.c_str() );
            throw CVTException( msg.c_str() );
        }

        std::vector<double> rgbStamps;
        std::vector<double> depthStamps;

        String assocFile = _folder + "associated.txt";
        bool associated = false;
        if( FileSystem::exists( assocFile ) ){
            // load the associated
            std::cout << "Using preassociated file" << std::endl;
            loadDepthAndRGB( rgbStamps, depthStamps );
            associated = true;
        } else {
            loadRGBFilenames( rgbStamps );
            loadDepthFilenames( depthStamps );
        }
        loadGroundTruth();

        std::cout << "RGB: "    << _rgbFiles.size() << std::endl;
        std::cout << "Depth: "  << _depthFiles.size() << std::endl;
        std::cout << "Stamps: " << _stamps.size() << std::endl;

        sortOutData( rgbStamps, depthStamps, associated );
        std::cout << "RGB: " << _rgbFiles.size() << std::endl;
        std::cout << "Depth: " << _depthFiles.size() << std::endl;
        std::cout << "Stamps: " << _stamps.size() << std::endl;
    }

    void RGBDParser::next()
    {
        if( _idx >= _stamps.size() ){
            std::cout << "End of data !" << std::endl;
            return;
        }
        _sample.stamp	= _stamps[ _idx ];
        _sample.rgb.load( _rgbFiles[ _idx ] );
        _sample.depth.load( _depthFiles[ _idx ] );
        _sample.orientation = _orientations[ _idx ];
        _sample.position = _positions[ _idx ];
        _sample.poseValid = _poseValid[ _idx ];
        _idx++;
    }

    void RGBDParser::loadGroundTruth()
    {
        String file;
        file.sprintf( "%sgroundtruth.txt", _folder.c_str() );

        Data gtFile;
        FileSystem::load( gtFile, file );

        DataIterator iter( gtFile );

        Quaterniond q;
        Vector3d    p;
        double stamp;
        while( readNext( q, p, stamp, iter ) ){
            _orientations.push_back( q );
            _positions.push_back( p );
            _stamps.push_back( stamp );
        }
    }

    void RGBDParser::loadRGBFilenames( std::vector<double> & rgbStamps )
    {
        String file;
        file.sprintf( "%srgb.txt", _folder.c_str() );

        Data rgbFile;
        FileSystem::load( rgbFile, file );
        DataIterator iter( rgbFile );

        String name;
        double stamp;
        while( readNextFilename( name, stamp, iter ) ){
            rgbStamps.push_back( stamp );
            _rgbFiles.push_back( _folder + name );
        }
    }

    void RGBDParser::loadDepthAndRGB( std::vector<double>& rgbStamps, std::vector<double>& depthStamps )
    {
        _rgbFiles.clear();
        _depthFiles.clear();

        String file = _folder + "associated.txt";
        Data assocFile;
        FileSystem::load( assocFile, file );
        DataIterator iter( assocFile );

        while( iter.hasNext() ){
            std::vector<String> tokens;
            iter.tokenizeNextLine( tokens, " " );

            if( tokens.size() == 4 ){
                double t = tokens[ 0 ].toDouble();
                if( tokens[ 1 ].hasPrefix( "rgb" ) ){
                    // rgb then depth
                    rgbStamps.push_back( t );
                    _rgbFiles.push_back( _folder + tokens[ 1 ] );
                    depthStamps.push_back( tokens[ 2 ].toDouble() );
                    _depthFiles.push_back( _folder + tokens[ 3 ] );
                } else {
                    depthStamps.push_back( t );
                    _depthFiles.push_back( _folder + tokens[ 1 ] );
                    rgbStamps.push_back( tokens[ 2 ].toDouble() );
                    _rgbFiles.push_back( _folder + tokens[ 3 ] );
                }
            }
        }
    }

    void RGBDParser::loadDepthFilenames( std::vector<double>& depthStamps )
    {
        String file;
        file.sprintf( "%sdepth.txt", _folder.c_str() );

        Data depthFile;
        FileSystem::load( depthFile, file );
        DataIterator iter( depthFile );

        String name;
        double stamp;
        while( readNextFilename( name, stamp, iter ) ){
            depthStamps.push_back( stamp );
            _depthFiles.push_back( _folder + name );
        }
    }

    // read next line from the gtfile
    bool RGBDParser::readNext( Quaterniond& orientation, Vector3d& position, double& stamp, DataIterator& iter )
    {
        if( !iter.hasNext() )
            return false;

        std::vector<String> tokens;
        iter.tokenizeNextLine( tokens, " " );

        while( tokens.size() == 0 || tokens[ 0 ] == "#" ){
            if( !iter.hasNext() )
                return false;
            tokens.clear();
            iter.tokenizeNextLine( tokens, " " );
        }

        // now we should have a valid line with 8 tokens!
        if( tokens.size() != 8 ) std::cout << "File corrupt?!" << std::endl;

        stamp = tokens[ 0 ].toDouble();
        position.x = tokens[ 1 ].toDouble();
        position.y = tokens[ 2 ].toDouble();
        position.z = tokens[ 3 ].toDouble();
        orientation.x = tokens[ 4 ].toDouble();
        orientation.y = tokens[ 5 ].toDouble();
        orientation.z = tokens[ 6 ].toDouble();
        orientation.w = tokens[ 7 ].toDouble();

        return true;
    }

    bool RGBDParser::readNextFilename( String& filename, double& stamp, DataIterator& iter )
    {
        if( !iter.hasNext() )
            return false;

        std::vector<String> tokens;
        iter.tokenizeNextLine( tokens, " " );


        while( tokens.size() == 0 || tokens[ 0 ] == "#" ){
            if( !iter.hasNext() ){
                return false;
            }
            tokens.clear();
            iter.tokenizeNextLine( tokens, " " );
        }

        // now we should have a valid line with 8 tokens!
        if( tokens.size() != 2 ) std::cout << "File corrupt?!" << std::endl;

        stamp = tokens[ 0 ].toDouble();
        filename = tokens[ 1 ];

        return true;
    }

    size_t RGBDParser::findClosestMatchInGTStamps( double val, size_t startIdx )
    {
        size_t bestIdx = startIdx;
        double best = Math::abs( _stamps[ startIdx ] - val );
        for( size_t i = startIdx + 1; i < _stamps.size(); i++ ){
            double cur = Math::abs( _stamps[ i ] - val );
            if( cur < best ){
                best = cur;
                bestIdx = i;
            }
        }
        return bestIdx;
    }

    void RGBDParser::sortOutData( const std::vector<double>& rgbStamps,
                                  const std::vector<double>& depthStamps,
                                  bool rgbAndDepthAssocitated )
    {
        size_t rgbIdx = 0;

        std::vector<double> stamps;
        std::vector<Quaterniond> orientations;
        std::vector<Vector3d>    positions;
        std::vector<String> color;
        std::vector<String> depth;
        std::vector<bool> validity;

        Quaterniond qlast;
        Vector3d    tlast;
        tlast.setZero();
        while( rgbIdx < rgbStamps.size() ){
            // find best depth for this rgb
            size_t bestDepthIdx = rgbIdx;
            if( !rgbAndDepthAssocitated ){
                bestDepthIdx = findClosestMatchInDepthStamps( rgbStamps[ rgbIdx ], 0, depthStamps );
            }
            double depthDist = Math::abs( rgbStamps[ rgbIdx ] - depthStamps[ bestDepthIdx ] );

            if( depthDist < _maxStampDiff ){
                // now we have the match: rgbIdx & depthIdx and should search for the closest gt
                depth.push_back( _depthFiles[ bestDepthIdx ] );
                color.push_back( _rgbFiles[ rgbIdx ] );

                size_t bestGtIdx = findClosestMatchInGTStamps( rgbStamps[ rgbIdx ], 0 );
                if( Math::abs( _stamps[ bestGtIdx ] - rgbStamps[ rgbIdx ] ) < _maxStampDiff ){
                    // take it
                    stamps.push_back( _stamps[ bestGtIdx ] );
                    orientations.push_back( _orientations[ bestGtIdx ] );
                    positions.push_back( _positions[ bestGtIdx ] );
                    validity.push_back( true );
                    tlast = positions.back();
                    qlast = orientations.back();
                } else {
                    stamps.push_back( rgbStamps[ rgbIdx ] );
                    orientations.push_back( qlast );
                    positions.push_back( tlast );
                    validity.push_back( false );
                }
           }
           rgbIdx++;
        }
        std::cout << "Found: " << stamps.size() << " matches" << std::endl;

        _orientations = orientations;
        _positions = positions;
        _poseValid = validity;
        _stamps = stamps;
        _rgbFiles = color;
        _depthFiles = depth;

    }


    size_t RGBDParser::findClosestMatchInDepthStamps( double stamp,
                                                      size_t iter,
                                                      const std::vector<double>& dStamps )
    {
        size_t bestIdx = iter;
        double best = Math::abs( dStamps[ iter ] - stamp );
        double last = best;
        for( size_t i = iter + 1; i < dStamps.size(); i++ ){
            double cur = Math::abs( dStamps[ i ] - stamp );
            if( cur < best ){
                best = cur;
                bestIdx = i;
            }

            // they are ordered, so if we get a worse match once, we won't get any better
            if( cur > last )
                break;
            last = cur;
        }
        return bestIdx;
    }
}
