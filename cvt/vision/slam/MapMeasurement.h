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

#ifndef CVT_MAP_MEASUREMENT_H
#define CVT_MAP_MEASUREMENT_H

#include <Eigen/Core>
#include <cvt/io/xml/XMLSerializable.h>
#include <cvt/io/xml/XMLNode.h>
#include <cvt/io/xml/XMLElement.h>
#include <cvt/io/xml/XMLText.h>

#include <cvt/util/EigenBridge.h>
#include <cvt/math/Vector.h>
#include <cvt/math/Matrix.h>

namespace cvt
{
	class MapMeasurement : public XMLSerializable
	{
		public:
			EIGEN_MAKE_ALIGNED_OPERATOR_NEW

			MapMeasurement();
			MapMeasurement( const MapMeasurement& other );

			XMLNode* serialize() const;
			void deserialize( XMLNode* node );
	
			// the point
			Eigen::Vector2d	point;

			// information is inverse covariance
			Eigen::Matrix2d	information;
	};


	inline MapMeasurement::MapMeasurement() :
		point( 0, 0 ),
		information( Eigen::Matrix2d::Identity() )
	{
	}

	inline MapMeasurement::MapMeasurement( const MapMeasurement& other ) :
		XMLSerializable(),
		point( other.point ),
		information( other.information )
	{
	}


	inline XMLNode* MapMeasurement::serialize() const
	{
		XMLElement* mapMeas = new XMLElement( "PointMeasurement" );

		XMLElement* n = new XMLElement( "Point2d" );
		Vector2d p2d;
		EigenBridge::toCVT( p2d, point );
		n->addChild( new XMLText( p2d.toString() ) );
		mapMeas->addChild( n );

		n = new XMLElement( "Information" );
		Matrix2d inf;
		EigenBridge::toCVT( inf, information );
		n->addChild( new XMLText( inf.toString() ) );

		mapMeas->addChild( n );

		return mapMeas;
	}

	inline void MapMeasurement::deserialize( XMLNode* node )
	{
		if( node->name() != "PointMeasurement" )
			throw CVTException( "this is not a PointMeasurement node" );

		XMLNode * n;
		n = node->childByName( "Point2d" );
		if( n != NULL ){
			Vector2d v = Vector2d::fromString( n->child( 0 )->value() );
			EigenBridge::toEigen( point, v );
		}

		n = node->childByName( "Information" );
		if( n != NULL ){
			Matrix2d m = Matrix2d::fromString( n->child( 0 )->value() );
			EigenBridge::toEigen( information, m );
		}
	}
}

#endif
