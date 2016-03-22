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

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <sys/stat.h>
#include <errno.h>

static std::string INCLUDEDIR;
static std::string CVT_INCLUDEDIR;

void escapeStream( std::ofstream& output, std::ifstream& input, const std::string& filename );

bool hasEnding( const std::string& fullString, const std::string& ending)
{
	if (fullString.length() >= ending.length()) {
		return  ( fullString.compare(fullString.length() - ending.length(), ending.length(), ending) == 0 );
	} else {
        return false;
    }
}

std::string trimImport( const std::string& str )
{
	const char* ptr = str.c_str();
	const char* endptr = ptr + str.length() - 1;
	while( *ptr == '<' || *ptr == ' ' || *ptr == '\t' || *ptr == '"' )
		ptr++;
	while( *endptr == '>' || *endptr == ' ' || *endptr == '\t' || *endptr == '"' || *endptr == '\r' )
		endptr--;
	if( endptr <= ptr )
		return std::string( "" );
	return std::string( ptr, endptr + 1 - ptr );
}

void importFile( std::ofstream& output, const std::string& _path )
{
    std::ifstream input;
	std::string path( INCLUDEDIR + _path );
    input.open( path.c_str() );

    if( !input.is_open() ) {
	    path = CVT_INCLUDEDIR + _path;
        input.open( path.c_str() );
        if( !input.is_open() ){
            std::cerr << "Failed to import file: \"" << path << "\"" << std::endl;
            ::exit( 1 );
            return;
        }
	}
	std::cout << "Importing " << path << std::endl;
	escapeStream( output, input, _path );
    input.close();
}

void escapeString( std::ofstream& output, const std::string& string )
{
	int len = string.length();
	for( int i = 0; i < len; i++ ) {
		char val = string[ i ];
		switch( val ) {
			case '"': output << "\\\"";
					  break;
			case '\\': output << "\\\\";
					   break;
			default: output << val;
		}
	}
}

void escapeStream( std::ofstream& output, std::ifstream& input, const std::string& filename = "" )
{
    char val;
	unsigned int line = 1;
	std::string hashline;
	std::string importprefix = "import";

	if( !hasEnding( filename, ".vert" ) && !hasEnding( filename, ".frag" ) )
		output << "#line " << line << " \\\"" << filename << "\\\" \\n\" \\" << std::endl << "\"";

	while( !input.eof() ) {
		input.get( val );
		if( !input.eof() ) {
			switch( val ) {
				case '\n': output << "\\n\" \\"  << std::endl << "\"";
						   line++;
						   break;
				case '"': output << "\\\"";
						   break;
				case '\\': output << "\\\\";
						   break;
				case '#':
						   std::getline( input, hashline );
						   line++;
						   if( hashline.substr( 0, importprefix.size() ) == importprefix ) {
							   std::string hashfile = trimImport( hashline.substr( importprefix.size(), std::string::npos ) );
							   importFile( output, hashfile );
							   output << "#line " << line << " \\\"" << filename << "\\\" \\n\" \\" << std::endl << "\"";
						   } else {
							   output << '#';
							   escapeString( output, hashline );
							   output << "\\n\" \\"  << std::endl << "\"";
						   }
						   break;
				default: output << val;
			}
		}
	}

}

int mkpath( std::string s, mode_t mode )
{
    size_t pre=0,pos;
    std::string dir;
    int mdret = 0;

    while( ( pos = s.find_first_of( '/', pre ) ) != std::string::npos ){
        dir = s.substr( 0, pos++ );
        pre = pos;
        if( dir.size() ==0 ) continue; 
        if( ( mdret = mkdir( dir.c_str(), mode ) ) && errno != EEXIST ){
            return mdret;
        }
    }
    return 0;
}

int main( int argc, char** argv )
{

	if( argc != 5 ) {
		std::cerr << "usage: " << argv[ 0 ] << " cl-file header-file name cvt_include_dir" << std::endl;
		return 1;
	}

	//	std::cout << "\n\n\n" << argv[ 0 ] << " " << argv[ 1 ] << " " << argv[ 2 ] << "\n\n\n" << std::endl;

    CVT_INCLUDEDIR = std::string( argv[ 4 ] ) + "/";
	std::string infile( argv[ 1 ] );
	INCLUDEDIR = infile.substr( 0, infile.find_last_of("/") + 1 );
	std::string inbasename = infile.substr( infile.find_last_of("/") + 1, std::string::npos );

	std::ifstream input;
	input.open( argv[ 1 ] );
	if( !input.is_open() ) {
		std::cerr << "Unable to open cl-file!" << std::endl;
		return 1;
	}
	std::ofstream output;
    int r;
    if( (r = mkpath( argv[ 2 ], 0755 )) != 0 ){
       std::cerr << "Could not create path: " << argv[ 2 ] << std::endl;
       std::cerr << "Error: " << errno << std::endl;
       return 1;
    }
	output.open( argv[ 2 ], std::ios::trunc );
	if( !output.is_open() ) {
		std::cerr << "Unable to open header-file!" << std::endl;
		return 1;
	}


	output << "#ifndef CL_" << argv[ 3 ] << "_H" << std::endl;
	output << "#define CL_" << argv[ 3 ] << "_H" << std::endl;
	output << "static const char _" << argv[ 3 ] << "_source[ ] =" << std::endl;
	output << "\"";
	escapeStream( output, input, inbasename );
	output << "\\n\";" << std::endl;
	output << "#endif" << std::endl;
	input.close();
	output.close();

	return 0;
}
