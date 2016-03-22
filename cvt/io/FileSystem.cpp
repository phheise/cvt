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

#include "FileSystem.h"
#include "util/Exception.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>

#include <cvt/util/CVTTest.h>

namespace cvt {

	bool FileSystem::exists( const String & path )
	{
		struct stat attr;
		if ( stat( path.c_str(), &attr ) == -1 )
			return false;
		else
			return true;
	}

	void FileSystem::touch( const String& file )
	{
		int fd;
		if( ( fd = open( file.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP ) ) < 0 ){
			throw CVTException( "Could not create file" );
		}
		close( fd );
	}

	void FileSystem::rename( const String & from, const String & to )
	{
		if ( ::rename( from.c_str(), to.c_str() ) < 0 )
			throw CVTException("Could not rename file");
	}

	bool FileSystem::isFile( const String & file )
	{
		struct stat attr;
		if( stat( file.c_str(), &attr ) == -1 )
			CVTException( "Given path does not exist" );
		return S_ISREG( attr.st_mode );
	}

	bool FileSystem::isDirectory( const String & dir )
	{
		struct stat attr;
		if( stat( dir.c_str(), &attr ) == -1 )
			CVTException( "Given path does not exist" );
		return S_ISDIR( attr.st_mode );
	}

	void FileSystem::mkdir( const String & name )
	{
		if( ::mkdir( name.c_str(), 0770 ) == -1 ){
			String msg( "mkdir error: " );
			msg += strerror( errno );
			throw CVTException( msg.c_str() );
		}
	}

	void FileSystem::ls( const String & path, std::vector<String> & entries )
	{
		entries.clear();
		if( !exists( path ) )
			return;

		DIR * dirEntries = opendir( path.c_str() );
		if( dirEntries == NULL )
			return;

		struct dirent * entry = NULL;

		while( ( entry = readdir( dirEntries ) ) != NULL ) {
			String entryName( entry->d_name );

			if( entryName == "." ||
					entryName == ".." )
				continue;

			entries.push_back( entryName );
		}
		closedir( dirEntries );
	}

	void FileSystem::filesWithExtension( const String & _path, std::vector<String>& result, const String & ext )
	{
		String path = _path;

		if( path[ path.length()-1 ] != '/' )
			path += '/';

		if( !exists( path ) ){
			String message( "Path not found: " );
			message += path;
			throw CVTException( "Path not found: " );
		}

		DIR * dirEntries = opendir( path.c_str() );
		if( dirEntries == NULL ){
			String message( "Directory not readable: " );
			message += path;
			CVTException( message.c_str() );
		}

		struct dirent * entry = NULL;

		while( ( entry = readdir( dirEntries ) ) != NULL ) {
			String entryName( path );
			entryName += entry->d_name;

			if( isFile( entryName ) ) {
				if( entryName.length() >= ext.length() ){
					// get the extension of the entry
					String entryExt = entryName.substring( entryName.length() - ext.length(), ext.length() );
					if( entryExt == ext ){
						result.push_back( entryName );
					}
				}
			}
		}
		closedir( dirEntries );
	}

	size_t FileSystem::size( const String& path )
	{
		struct stat buf;
		if( !stat( path.c_str(), &buf ) ) {
			return buf.st_size;
		}
		return 0;
	}

	bool FileSystem::load( Data& d, const String& path, bool zerotermination )
	{
		size_t len;
		if( ( len = size( path ) ) ) {
			int fd;
			if( ( fd = open( path.c_str(), O_RDONLY ) ) < 0 )
				return false;
			d.allocate( len + ( zerotermination? 1 : 0 ) );
			if( read( fd, d.ptr(), len ) != ( ssize_t ) len ) {
				close( fd );
				return false;
			}
			close( fd );
			if( zerotermination )
				d.ptr()[ len ] = '\0';
			return true;
		}
		return false;
	}

	bool FileSystem::save( const String& path, const Data& d )
	{
		int fd;
		if( ( fd = open( path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP ) ) < 0 )
			return false;
		if( write( fd, d.ptr(), d.size() ) != ( ssize_t ) d.size() ) {
			close( fd );
			return false;
		}
		close( fd );
		return true;
	}


	BEGIN_CVTTEST( filesystem )
		bool result = true;
	String dataFolder( CVT_DATA_FOLDER );
	bool b = FileSystem::exists( "/usr/include" );
	b &= FileSystem::exists( dataFolder );
	b &= !FileSystem::exists( "bliblabluiamnothere" );
	CVTTEST_PRINT( "exists: ", b );
	result &= b;


	b = FileSystem::isDirectory( "/usr/include" );
	String f;
	f = dataFolder; f += "/lena.png";
	b &= !FileSystem::isDirectory( f );
	CVTTEST_PRINT( "isDirectory: ", b );
	result &= b;

	b = !FileSystem::isFile( "/usr/include" );
	b &= FileSystem::isFile( f );
	CVTTEST_PRINT( "isFile: ", b );
	result &= b;

	String f2( dataFolder ); f2 += "/blubb.png";
	FileSystem::rename( f, f2 );
	b = FileSystem::exists( f2 );
	b &= !FileSystem::exists( f );
	FileSystem::rename( f2, f );
	b &= !FileSystem::exists( f2 );
	b &= FileSystem::exists( f );
	CVTTEST_PRINT( "rename: ", b );
	result &= b;

	return result;
	END_CVTTEST
}
