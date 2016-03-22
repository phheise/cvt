#include <cvt/gfx/IFilter.h>
#include <cvt/util/Plugin.h>
#include <cvt/util/PluginManager.h>
#include <cvt/cl/CLKernel.h>

#include "BCS.h"

namespace cvt {

	static ParamInfoTyped<Image*> _pinput( "Input" );
	static ParamInfoTyped<float>  _pbrightness( "Brightness", -1.0f, 1.0f, 0.0f );
	static ParamInfoTyped<float>  _pcontrast( "Contrast", -2.0f, 2.0f, 1.0f );
	static ParamInfoTyped<float>  _psaturation( "Saturation", -2.0f, 2.0f, 0.0f );
	static ParamInfoTyped<Image*> _poutput( "Output", NULL, false );

	static ParamInfo* _bcs_params[] =
	{
		&_pinput,
		&_pbrightness,
		&_pcontrast,
		&_psaturation,
		&_poutput
	};

	class BCS : public IFilter
	{
		public:
			BCS();
			~BCS();
			void apply( const ParamSet* attribs, IFilterType iftype ) const;

		private:
			BCS( const BCS& );
	};

	BCS::BCS() : IFilter( "BCS", _bcs_params, 5, IFILTER_OPENCL )
	{
	}

	BCS::~BCS()
	{
	}

	void BCS::apply( const ParamSet* attribs, IFilterType iftype ) const
	{

#define RLUM 0.3086f
#define GLUM 0.6094f
#define BLUM 0.0820f

		if( iftype == IFILTER_OPENCL ) {
			CLKernel _clkernel;
			size_t w, h;
			Image* src = attribs->arg<Image*>( 0 );
			Image* dst = attribs->arg<Image*>( 4 );
		//	dst.reallocate( src, src->memType );
			w = src->width();
			h = src->height();
			Vector4f rvec, gvec, bvec;
			float b = attribs->arg<float>( 1 );
			float c = attribs->arg<float>( 2 );
			float s = attribs->arg<float>( 3 );
			rvec.set( c * ( ( 1.0f - s ) * RLUM + s ), c * ( 1.0f - s ) * GLUM, c * ( 1.0f - s ) * BLUM, b  );
			gvec.set( c * ( 1.0f - s ) * RLUM, c * ( ( 1.0f - s ) * GLUM +s ), c * ( 1.0f - s ) * BLUM, b  );
			bvec.set( c * ( 1.0f - s ) * RLUM, c * ( 1.0f - s ) * GLUM, c * ( ( 1.0f - s ) * BLUM + s ), b  );

			std::string log;
			_clkernel.build( "BCS", _BCS_source, strlen( _BCS_source ), log );

			_clkernel.setArg( 0, *dst );
			_clkernel.setArg( 1, *src );
			_clkernel.setArg( 2, rvec );
			_clkernel.setArg( 3, gvec );
			_clkernel.setArg( 4, bvec );
			_clkernel.run( cl::NDRange( w, h ), cl::NDRange( 1, 1 ) );
		}
	}

}

static void _init( cvt::PluginManager* pm )
{
	cvt::IFilter* _BCS = new cvt::BCS();
	pm->registerPlugin( _BCS );
}


CVT_PLUGIN( _init )
/*extern "C" {
	cvt::PluginInfo _cvtplugin = { 0x43565450, 0, 1, ( void ( * )( cvt::PluginManager* ) ) _init };
}*/
