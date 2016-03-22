__kernel void BCS( __write_only image2d_t out,  __read_only image2d_t in, float4 rvec, float4 gvec, float4 bvec )
{
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;
	float4 inp, tmpp, outp;
	int2 coord;

	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );
	inp = read_imagef( in, sampler, coord );
	tmpp = inp;
    tmpp.w = 1.0f;	
	outp.x = dot( rvec, tmpp );
	outp.y = dot( gvec, tmpp );
	outp.z = dot( bvec, tmpp );
	outp.w = inp.w;
	write_imagef( out, coord, outp );
}


