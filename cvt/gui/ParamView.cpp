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


#include <cvt/gui/ParamView.h>
#include <cvt/util/Delegate.h>

namespace cvt {

    ParamView::ParamView( ParamSet& pset, size_t paramIdx ) :
        _pset( pset ),
        _paramIdx( paramIdx ),
        _nameLabel( "" ),
        _valueLabel( "" )
    {
        const ParamInfo* pinfo = _pset.paramInfo( _paramIdx );
        _paramHandle = _pset.paramHandle( pinfo->name );

        Delegate<void (float)> d( this, &ParamView::handleSliderChange );
        _slider.valueChanged.add( d );

        // the gui elements:
        WidgetLayout wl;
        wl.setRelativeLeftRight( 0.01f, 0.49f );
        wl.setRelativeTopBottom( 0.01f, 0.49f );

        String label( pinfo->name.c_str() );
        _nameLabel.setLabel( label );

        addWidget( &_nameLabel, wl );
        wl.setRelativeLeftRight( 0.51f, 0.99f );
        addWidget( &_valueLabel, wl );

        wl.setRelativeLeftRight( 0.01f, 0.99f );
        wl.setRelativeTopBottom( 0.51f, 0.9f );
        addWidget( &_slider, wl );
        initializeSliderRanges();
    }

    void ParamView::handleSliderChange( float v )
    {
        // update the parameter and the label
        const ParamInfo* _pinfo = _pset.paramInfo( _paramIdx );

        String label;
        switch( _pinfo->type ){
            case PTYPE_UINT8:
                _pset.setArg( _paramHandle, ( uint8_t )v );
                label.sprintf( "%u", ( uint8_t )v );
                break;
            case PTYPE_UINT16:
                _pset.setArg( _paramHandle, ( uint16_t )v );
                label.sprintf( "%u", ( uint16_t )v );
                break;
            case PTYPE_UINT32:
                _pset.setArg( _paramHandle, ( uint32_t )v );
                label.sprintf( "%u", ( uint32_t )v );
                break;
            case PTYPE_UINT64:
                _pset.setArg( _paramHandle, ( uint64_t )v );
                label.sprintf( "%u", ( uint64_t )v );
                break;
            case PTYPE_INT8:
                _pset.setArg( _paramHandle, ( int8_t )v );
                label.sprintf( "%d", ( int8_t )v );
                break;
            case PTYPE_INT16:
                _pset.setArg( _paramHandle, ( int16_t )v );
                label.sprintf( "%d", ( int16_t )v );
                break;
            case PTYPE_INT32:
                _pset.setArg( _paramHandle, ( int32_t )v );
                label.sprintf( "%d", ( int32_t )v );
                break;
            case PTYPE_INT64:
                _pset.setArg( _paramHandle, ( int64_t )v );
                label.sprintf( "%d", ( int64_t )v );
                break;
            case PTYPE_FLOAT:
                _pset.setArg( _paramHandle, v );
                label.sprintf( "%0.1g", v );
                break;
            case PTYPE_DOUBLE:
                _pset.setArg( _paramHandle, ( double )v );
                label.sprintf( "%0.1g", v );
                break;
            default:
                break;
        }
        _valueLabel.setLabel( label );
    }

    void ParamView::initializeSliderRanges()
    {
        const ParamInfo* _pinfo = _pset.paramInfo( _paramIdx );
        switch( _pinfo->type ){
            case PTYPE_UINT8:
                {
                    const ParamInfoTyped<uint8_t>* p = ( const ParamInfoTyped<uint8_t>* )_pinfo;
                    _slider.setMaximum( ( float )p->maxValue() );
                    _slider.setMinimum( ( float )p->minValue() );
                    _slider.setValue( ( float )p->defaultValue() );
                }
                break;
            case PTYPE_UINT16:
                {
                    const ParamInfoTyped<uint16_t>* p = ( const ParamInfoTyped<uint16_t>* )_pinfo;
                    _slider.setMaximum( ( float )p->maxValue() );
                    _slider.setMinimum( ( float )p->minValue() );
                    _slider.setValue( ( float )p->defaultValue() );
                }
                break;
            case PTYPE_UINT32:
                {
                    const ParamInfoTyped<uint32_t>* p = ( const ParamInfoTyped<uint32_t>* )_pinfo;
                    _slider.setMaximum( ( float )p->maxValue() );
                    _slider.setMinimum( ( float )p->minValue() );
                    _slider.setValue( ( float )p->defaultValue() );
                }
                break;
            case PTYPE_UINT64:
                {
                    const ParamInfoTyped<uint64_t>* p = ( const ParamInfoTyped<uint64_t>* )_pinfo;
                    _slider.setMaximum( ( float )p->maxValue() );
                    _slider.setMinimum( ( float )p->minValue() );
                    _slider.setValue( ( float )p->defaultValue() );
                }
                break;
            case PTYPE_INT8:
                {
                    const ParamInfoTyped<int8_t>* p = ( const ParamInfoTyped<int8_t>* )_pinfo;
                    _slider.setMaximum( ( float )p->maxValue() );
                    _slider.setMinimum( ( float )p->minValue() );
                    _slider.setValue( ( float )p->defaultValue() );
                }
                break;
            case PTYPE_INT16:
                {
                    const ParamInfoTyped<int16_t>* p = ( const ParamInfoTyped<int16_t>* )_pinfo;
                    _slider.setMaximum( ( float )p->maxValue() );
                    _slider.setMinimum( ( float )p->minValue() );
                    _slider.setValue( ( float )p->defaultValue() );
                }
                break;
            case PTYPE_INT32:
                {
                    const ParamInfoTyped<int32_t>* p = ( const ParamInfoTyped<int32_t>* )_pinfo;
                    _slider.setMaximum( ( float )p->maxValue() );
                    _slider.setMinimum( ( float )p->minValue() );
                    _slider.setValue( ( float )p->defaultValue() );
                }
                break;
            case PTYPE_INT64:
                {
                    const ParamInfoTyped<int64_t>* p = ( const ParamInfoTyped<int64_t>* )_pinfo;
                    _slider.setMaximum( ( float )p->maxValue() );
                    _slider.setMinimum( ( float )p->minValue() );
                    _slider.setValue( ( float )p->defaultValue() );
                }
                break;
            case PTYPE_FLOAT:
                {
                    const ParamInfoTyped<float>* p = ( const ParamInfoTyped<float>* )_pinfo;
                    _slider.setMaximum( p->maxValue() );
                    _slider.setMinimum( p->minValue() );
                    _slider.setValue( p->defaultValue() );
                }
                break;
            case PTYPE_DOUBLE:
                {
                    const ParamInfoTyped<double>* p = ( const ParamInfoTyped<double>* )_pinfo;
                    _slider.setMaximum( ( float )p->maxValue() );
                    _slider.setMinimum( ( float )p->minValue() );
                    _slider.setValue( ( float )p->defaultValue() );
                }
                break;
            default:
                break;
        }

    }

}
