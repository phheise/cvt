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

#include <cvt/gui/ParamSetView.h>
#include <cvt/gui/Slider.h>

namespace cvt
{

        ParamSetView::ParamSetView( ParamSet& pset )
        {
            _paramViews.resize( pset.size() );

            WidgetLayout wl;
            wl.setRelativeLeftRight( 0.01f, 0.99f );
            float relTop = 0.0f;
            float relStep = 1.0f / pset.size();
            for( size_t i = 0; i < pset.size(); i++ ){
                _paramViews[ i ] = new ParamView( pset, i );

                wl.setRelativeTopBottom( relTop, relTop + relStep );
                addWidget( _paramViews[ i ], wl );
                relTop += relStep;
            }
        }

        ParamSetView::~ParamSetView()
        {
            for( size_t i = 0; i < _paramViews.size(); i++ ){
                if( _paramViews[ i ] )
                    delete _paramViews[ i ];
            }
            _paramViews.clear();
        }
}
