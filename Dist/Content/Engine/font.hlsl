#include "Psybrus.hlsl"

////////////////////////////////////////////////////////////////////////
// VS_INPUT
#define VS_INPUT		VertexDefault

////////////////////////////////////////////////////////////////////////
// VS_OUTPUT
struct VS_OUTPUT
{
	float4 Position_	: SV_POSITION;
	float4 Colour_		: COLOR0;
	float4 TexCoord0_	: TEXCOORD0;
};

////////////////////////////////////////////////////////////////////////
// PS_OUTPUT
struct PS_OUTPUT
{
	float4 Colour_		: SV_TARGET;
};

////////////////////////////////////////////////////////////////////////
// vertexMain
VS_OUTPUT vertexMain( VS_INPUT i )
{
	VS_OUTPUT o = (VS_OUTPUT)0;

	float4 WorldPosition;
	PSY_MAKE_WORLD_SPACE_VERTEX( WorldPosition, i.Position_, i );
	PSY_MAKE_CLIP_SPACE_VERTEX( o.Position_, WorldPosition );
	o.Colour_ = i.Colour_;
	o.TexCoord0_ = i.TexCoord0_;
	return o;
}

////////////////////////////////////////////////////////////////////////
// pixelMain
PSY_SAMPLER_2D( DiffuseTex );

PS_OUTPUT pixelMain( VS_OUTPUT i )
{
	PS_OUTPUT o = (PS_OUTPUT)0;
	float4 Colour = PSY_SAMPLE_2D( DiffuseTex, i.TexCoord0_.xy );
	float Factor = smoothstep( AlphaTestParams_.x, AlphaTestParams_.y, Colour.a );

	if( Factor < AlphaTestParams_.z )
	{
		//discard;
	}
	
	o.Colour_ = float4( Colour.xyz, Factor ) * i.Colour_;
	return o;
}
