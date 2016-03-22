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

#ifndef CVT_PARAMSET_H
#define CVT_PARAMSET_H

#include <cvt/util/ParamInfo.h>
#include <cvt/util/Exception.h>
#include <iostream>

namespace cvt
{
    class ParamSet;
    std::ostream & operator<<( std::ostream & out, const ParamSet& pSet );

    class ParamSet
    {
        public:
            /**
             * \brief Constructor for a ParamSet
             * \param pInfos	array of ParamInfo
             * \param n		size of array
             * \param genOffsets	if true, the offsets will be generated and set during construction
             *                      if e.g. you want to store your params in a struct,
             *                      you should specify the offsets yourself!
             */
            ParamSet( ParamInfo** pInfos, size_t n, bool genOffsets = true );

            ~ParamSet();

            /**
             *  \brief get handle to a parameter with the given name
             *  \param name name of the parameter
             *  \return the handle (id) which can be used to access the parameter
             */
            size_t paramHandle( const std::string & name );

            /* set parameter */
            template <class T>
            void setArg( size_t handle, T value, size_t localIndex = 0 );

            template <class T>
            T arg( size_t handle, size_t localIndex = 0 ) const;

            template <typename T>
            T * ptr();

            template <class T>
            const std::string & selectionString( size_t handle );
            size_t  size() const { return _numParameters; }

            const ParamInfo* paramInfo( size_t i ) const;

        private:
            uint8_t *	_parameterMem;
            ParamInfo**	_pInfos;
            size_t		_numParameters;

            friend std::ostream& operator<< ( std::ostream &, const ParamSet& );
    };

    template<class T>
    inline void ParamSet::setArg( size_t handle, T value, size_t localIndex )
    {
        ParamInfo * pInfo = _pInfos[ handle ];

        if( localIndex > pInfo->count )
            throw CVTException( "Parameter \"" + pInfo->name + "\" local index out of bounds!" );

        if( !PTypeCheck<T>::check( pInfo->type ) ){
            throw CVTException( "Parameter \"" + pInfo->name + "\" types do not match!" );
        }

        // Type T is ensured here!
        ParamInfoTyped<T>* pInfoT =  ( ParamInfoTyped<T>* )pInfo;
        if( pInfo->hasRange ){
            if( value < pInfoT->minValue() ){
                value = pInfoT->minValue();
            } else if( value > pInfoT->maxValue() ) {
                value = pInfoT->maxValue();
            }
        }

        *( T* )( _parameterMem + pInfo->offset + localIndex * sizeof( T ) ) = value;
    }

    template <class T>
    inline T ParamSet::arg( size_t handle, size_t localIndex ) const
    {
        ParamInfo * pInfo = _pInfos[ handle ];

        if( localIndex > pInfo->count )
            throw CVTException( "Parameter \"" + pInfo->name + "\" local index out of bounds!" );

        if( !PTypeCheck<T>::check( pInfo->type ) ){
            throw CVTException( "Parameter \"" + pInfo->name + "\" types do not match!" );
        }

        return *( T* )( _parameterMem + pInfo->offset + localIndex * sizeof( T ) );
    }

    template <class T>
    inline T * ParamSet::ptr()
    {
        return ( T* )_parameterMem;
    }

    template<>
    inline const std::string & ParamSet::selectionString<Selection>( size_t handle )
    {
        ParamInfo * p = _pInfos[ handle ];
        if( p->type != PTYPE_SELECTION ){
            throw CVTException( "Not a selection type" );
        }

        Selection current = this->arg<Selection>( handle );

        return ( ( ParamInfoTyped<Selection>* )p )->description( current );
    }


    inline const ParamInfo* ParamSet::paramInfo( size_t i ) const
    {
        if( i >= _numParameters )
            throw CVTException( "Out of parameter bounds!" );
        return _pInfos[ i ];
    }
}

#endif
