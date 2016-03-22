#include "PlyLoader.h"

#include <cvt/io/FileSystem.h>
#include <cvt/util/DataIterator.h>

namespace cvt {

	String PlyLoader::_name = "PLY";
	String PlyLoader::_extensions[ 2 ] = { ".ply", ".PLY" };


	enum PlyFormat { PLY_ASCII, PLY_BIN_LE, PLY_BIN_BE };
	enum PlyPropertyType { PLY_U8, PLY_U16, PLY_U32,
		PLY_S8, PLY_S16, PLY_S32,
		PLY_FLOAT, PLY_DOUBLE, PLY_LIST };

	struct PlyProperty {
		String name;
		PlyPropertyType type;
		PlyPropertyType lsizetype; /* type of list size */
		PlyPropertyType ltype; /* type of list elements */
	};

	struct PlyElement {
		String name;
		size_t size;
		std::vector<PlyProperty> properties;
		bool hasProperty( const String& name ) const;
	};

	inline bool PlyElement::hasProperty( const String& name ) const
	{
		for( std::vector<PlyProperty>::const_iterator it = properties.begin(); it != properties.end(); ++it ) {
			if( ( *it ).name == name )
				return true;
		}
		return false;
	}

	static inline size_t PlyTypeSize( PlyPropertyType type )
	{
		switch( type ) {
			case PLY_U8:
			case PLY_S8: return 1;
			case PLY_U16:
			case PLY_S16: return 2;
			case PLY_U32:
			case PLY_S32:
			case PLY_FLOAT: return 4;
			case PLY_DOUBLE: return 8;
			default: return 0;
		}
	}

	static bool PlyDiscardPropertyAscii( DataIterator& d, const PlyProperty& p )
	{
		String ws( " \r\n\t" ), dummy;

		if( p.type != PLY_LIST ) {
			/* discard single token */
			if( !d.nextToken( dummy, ws ) )
				return false;
		} else {
			/* discard list elements */
			long nlist;

			nlist = d.nextLong();
			if( nlist < 0 )
				return 0;
			while( nlist-- ) {
				if( !d.nextToken( dummy, ws ) )
					return false;
			}
		}
		return true;
	}

	static bool PlyDiscardElementAscii( DataIterator& d, const PlyElement& e )
	{
		bool fastdiscard = true;
		String ws( " \r\n\t" ), dummy;

		for( std::vector<PlyProperty>::const_iterator it = e.properties.begin();
			it != e.properties.end(); ++it ) {
			if( ( *it ).type == PLY_LIST ) {
				fastdiscard = false;
				break;
			}
		}

		if( fastdiscard ) {
			size_t n = e.properties.size() * e.size;
			while( n-- ) {
				if( !d.nextToken( dummy, ws ) )
					return false;
			}
		} else {
			size_t n = e.size;
			while( n-- ) {
				for( std::vector<PlyProperty>::const_iterator it = e.properties.begin();
					it != e.properties.end(); ++it )
					if( !PlyDiscardPropertyAscii( d, *it ) )
						return false;
			}
		}
		return true;
	}


	static bool PlyReadProperty( DataIterator& d, PlyProperty& p )
	{
		String strtype;
		String strname;
		String ws( " \r\n\t" );

		if( !d.nextToken( strtype, ws ) )
			return false;


		/* type */
		if( strtype == "uchar" )
			p.type  = PLY_U8;
		else if( strtype == "ushort" )
			p.type  = PLY_U16;
		else if( strtype == "uint" )
			p.type  = PLY_U32;
		else if( strtype == "char" )
			p.type  = PLY_S8;
		else if( strtype == "short" )
			p.type  = PLY_S16;
		else if( strtype == "int" )
			p.type  = PLY_S32;
		else if( strtype == "float" )
			p.type  = PLY_FLOAT;
		else if( strtype == "double" )
			p.type  = PLY_DOUBLE;
		else if( strtype == "list" )
			p.type  = PLY_LIST;
		else
			return false;


		if( p.type != PLY_LIST ) {
			/* name */
			if( !d.nextToken( p.name, ws ) )
				return false;
		} else {
			/* list type and name */
			PlyPropertyType type;

			if( !d.nextToken( strtype, ws ) )
				return false;

			if( strtype == "uchar" )
				type  = PLY_U8;
			else if( strtype == "ushort" )
				type  = PLY_U16;
			else if( strtype == "uint" )
				type  = PLY_U32;
			else if( strtype == "char" )
				type  = PLY_S8;
			else if( strtype == "short" )
				type  = PLY_S16;
			else if( strtype == "int" )
				type  = PLY_S32;
			else
				return false;

			p.lsizetype = type;

			if( !d.nextToken( strtype, ws ) )
				return false;

			if( strtype == "uchar" )
				type  = PLY_U8;
			else if( strtype == "ushort" )
				type  = PLY_U16;
			else if( strtype == "uint" )
				type  = PLY_U32;
			else if( strtype == "char" )
				type  = PLY_S8;
			else if( strtype == "short" )
				type  = PLY_S16;
			else if( strtype == "int" )
				type  = PLY_S32;
			else if( strtype == "float" )
				type  = PLY_FLOAT;
			else if( strtype == "double" )
				type  = PLY_DOUBLE;
			else
				return false;

			p.ltype = type;

			if( !d.nextToken( p.name, ws ) )
				return false;
		}

		//std::cout << p.name << std::endl;

		return true;
	}

	static bool PlyReadElement( DataIterator& d, PlyElement& e )
	{
		String name, str;
		String ws( " \r\n\t" );
		long size;

		if( !d.nextToken( name, ws ) )
			return false;

		size = d.nextLong();

		if( size < 0 )
			return false;

		e.name = name;
		e.size = ( size_t ) size;

		//std::cout << "Element: " << name << " " << size << std::endl;
		while( 1 ) {
			const uint8_t* cpos = d.pos();

			if( !d.nextToken( str, ws ) )
				return false;

			if( str == "comment"  ) {
				d.skipInverse( "\n" );
			} else if( str == "property"  ) {
				e.properties.resize( e.properties.size() + 1 );
				if( !PlyReadProperty( d, e.properties[ e.properties.size() - 1 ] ) )
					return false;
			} else {
				d -= ( d.pos() - cpos );
				return true;
			}
		}
	}

	static bool PlyReadHeader( DataIterator& d, std::vector<PlyElement>& elements, PlyFormat& format )
	{
		String str;
		String ws( " \r\n\t" );

		if( !d.nextToken( str, ws ) )
			return false;
		if( str != "ply" )
			return false;

		if( !d.nextToken( str, ws ) )
			return false;
		if( str != "format" )
			return false;

		if( !d.nextToken( str, ws ) )
			return false;

		/* which format */
		if( str == "ascii" )
			format = PLY_ASCII;
		else if( str == "binary_big_endian" )
			format = PLY_BIN_BE;
		else if( str == "binary_little_endian" )
			format = PLY_BIN_LE;
		else
			return false;

		/* skip version */
		if( !d.nextToken( str, ws ) )
			return false;

		/* get elments and theirs properties */
		while( 1 ) {
			if( !d.nextToken( str, ws ) )
				return false;

			if( str == "comment" ) {
				d.skipInverse( "\n" );
			} else if( str == "element" ) {
				elements.resize( elements.size() + 1 );
				if( !PlyReadElement( d, elements[ elements.size() - 1 ] ) )
					return false;
			} else if( str == "end_header" )
				break;
		}

		return true;
	}

	static bool PlyReadVertexAscii( DataIterator& d, std::vector<Vector3f>& vertices, std::vector<Vector3f>& normals, const PlyElement& e )
	{
		size_t n = e.size;
		String dummy;
		String ws( " \r\n\t" );
		double x , y, z, nx, ny, nz;

		x = y = z = nx = ny = nz = 0;

		while( n-- ) {
			for( std::vector<PlyProperty>::const_iterator it = e.properties.begin();
				it != e.properties.end(); ++it ) {
				if( it->name == "x" ) {
					x = d.nextDouble();
				} else if( it->name == "y" ) {
					y = d.nextDouble();
				} else if( it->name == "z" ) {
					z = d.nextDouble();
				} else	if( it->name == "nx" ) {
					nx = d.nextDouble();
				} else if( it->name == "ny" ) {
					ny = d.nextDouble();
				} else if( it->name == "nz" ) {
					nz = d.nextDouble();
				} else {
					/* discard */
					if( !d.nextToken( dummy, ws ) )
						return false;
				}
			}
			vertices.push_back( Vector3f( x, y, z ) );
			normals.push_back( Vector3f( nx, ny, nz ) );
		}

		return true;
	}

	static bool PlyReadVertexAscii( DataIterator& d, std::vector<Vector3f>& vertices, const PlyElement& e )
	{
		size_t n = e.size;
		String dummy;
		String ws( " \r\n\t" );
		double x , y, z, nx, ny, nz;

		x = y = z = nx = ny = nz = 0;

		while( n-- ) {
			for( std::vector<PlyProperty>::const_iterator it = e.properties.begin();
				it != e.properties.end(); ++it ) {
				if( it->name == "x" ) {
					x = d.nextDouble();
				} else if( it->name == "y" ) {
					y = d.nextDouble();
				} else if( it->name == "z" ) {
					z = d.nextDouble();
				} else {
					/* discard */
					if( !d.nextToken( dummy, ws ) )
						return false;
				}
			}
			vertices.push_back( Vector3f( x, y, z ) );
		}

		return true;
	}


	static bool PlyReadFacesAscii( DataIterator& d, std::vector<unsigned int>& faces, const PlyElement& e )
	{
		size_t n = e.size;
		String dummy;
		String ws( " \r\n\t" );


		while( n-- ) {
			//FIXME: check for only tris or quads
			long nn = d.nextLong();
			while( nn-- ) {
				long i = d.nextLong();
				if( i < 0 )
					return false;
				faces.push_back( ( unsigned int ) i );
			}
		}
		return true;
	}

	void PlyLoader::load( Scene& scene, const String& filename )
	{
		std::vector<PlyElement> elements;
		PlyFormat format;
		std::vector<Vector3f> vertices;
		std::vector<Vector3f> normals;
		std::vector<unsigned int> faces;


		scene.clear();

		Data data;
		FileSystem::load( data, filename );

		DataIterator d( data );
		PlyReadHeader( d, elements, format );

		// FIXME: add support for u,v and red, green, blue properties
		switch( format )
		{
			case PLY_ASCII:
				{
					for( std::vector<PlyElement>::iterator it = elements.begin();it != elements.end(); ++it ) {
						if( it->name == "vertex" ) {
							if( it->hasProperty( "nx" ) && it->hasProperty( "ny" ) && it->hasProperty( "nz" ) ) {
								if( !PlyReadVertexAscii( d, vertices, normals, *it ) )
									return;
							} else {
								if( !PlyReadVertexAscii( d, vertices, *it ) )
									return;
							}
						} else if( it->name == "face" ) {
							if( !PlyReadFacesAscii( d, faces, *it ) )
								return;
						} else {
							if( !PlyDiscardElementAscii( d, *it ) )
								return;
						}
					}
				}
				break;
			case PLY_BIN_LE:
			case PLY_BIN_BE:
				{
					throw CVTException( "Only ASCII PLY supported" );
				}
				break;
		}

		std::cout << vertices.size() << " " << faces.size() << std::endl;
		if( vertices.size() && faces.size() ) {
			SceneMesh* mesh = new SceneMesh( "PLY" );
			mesh->setVertices( &vertices[ 0 ], vertices.size() );
			// FIXME: correct mesh type - either tris or quads
			mesh->setFaces( &faces[ 0 ], faces.size(), SCENEMESH_TRIANGLES );
			if( normals.size() )
				mesh->setNormals( &normals[ 0 ], normals.size() );
			scene.addGeometry( mesh );
		}
	}

}

static void _init( cvt::PluginManager* pm )
{
	cvt::SceneLoader* ply = new cvt::PlyLoader();
	pm->registerPlugin( ply );
}

CVT_PLUGIN( _init )
