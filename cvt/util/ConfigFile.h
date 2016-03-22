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


#ifndef CVT_CONFIGFILE_H
#define CVT_CONFIGFILE_H

#include <cvt/util/String.h>
#include <map>

namespace cvt {

    class ConfigFile
    {
        public:
            ConfigFile( const String& filename );

            template <class T>
            T   valueForName( const String& name, T defaultValue, const String& group = "" );

            void save( const String& filename ) const;

            /**
             *  Saving to original filename
             */
            void save() const;

        private:           
            typedef std::map<String, String> MapType;            
            typedef std::map<String, MapType> GroupMapType;

            String       _fileName;
            GroupMapType _groups;

            template <class T>
            T   valueFromString( const String& strVal ) const;

            template <class T>
            String  stringFromValue( const T& val ) const;

            void writeGroup( std::ofstream& out, const MapType& map ) const;
    };

    template <class T>
    inline T ConfigFile::valueForName( const String& name, T defaultValue, const String& group )
    {
        String queryGrp = group;
        if( queryGrp == "" ){
            queryGrp = "default";
        }

        GroupMapType::iterator groupIter = _groups.find( queryGrp );
        if( groupIter == _groups.end() ){
            // group not present -> create
            groupIter = _groups.insert( std::make_pair( queryGrp, MapType() ) ).first;
        }

        // groupIter is the map with the values:
        MapType::const_iterator valIter = groupIter->second.find( name );
        if( valIter == groupIter->second.end() ){
            // not present: add default value:
            groupIter->second.insert( std::make_pair( name, stringFromValue( defaultValue ) ) );
            return defaultValue;
        } else {
            return valueFromString<T>( valIter->second );
        }
    }
}

#endif // CONFIGFILE_H
