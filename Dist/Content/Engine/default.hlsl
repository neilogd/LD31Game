#include "Psybrus.hlsl"

////////////////////////////////////////////////////////////////////////
// VS_INPUT
#define VS_INPUT		VertexDefault

////////////////////////////////////////////////////////////////////////
// VS_OUTPUT
struct VS_OUTPUT
{
	float4 Position_	: SV_POSITION;
	float4 Normal_		: NORMAL;
	float4 Colour_		: COLOR0;
	float4 TexCoord0_	: TEXCOORD0;
};


////////////////////////////////////////////////////////////////////////
// PATCH_OUTPUT
struct PATCH_OUTPUT
{
    float Edges_[3] 	: SV_TessFactor;
    float Inside_		: SV_InsideTessFactor;
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

#if PSY_BACKEND_TYPE != PSY_BACKEND_TYPE_GLSL_ES

////////////////////////////////////////////////////////////////////////
// geometryMain
[maxvertexcount(3)]
void geometryMain( triangle VS_OUTPUT Input[3],
                   inout TriangleStream< VS_OUTPUT > OutputStream )
{
	for( int i = 0; i < 3; ++i )
	{
		VS_OUTPUT Output = Input[i];
		OutputStream.Append( Output );
	}
}

////////////////////////////////////////////////////////////////////////
// hullMain
PATCH_OUTPUT hullConst()
{
	PATCH_OUTPUT o;

	o.Edges_[0] = 1.0f;
	o.Edges_[1] = 1.0f;
	o.Edges_[2] = 1.0f;
	o.Inside_ = 1.0f;
	
	return o;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("hullConst")]
VS_OUTPUT hullMain( InputPatch< VS_OUTPUT, 3 > Patch,
                    uint PointID : SV_OutputControlPointID )
{
	VS_OUTPUT o;

	o.Position_ = Patch[PointID].Position_;
	o.Normal_ = Patch[PointID].Normal_;
	o.Colour_ = Patch[PointID].Colour_;
	o.TexCoord0_ = Patch[PointID].TexCoord0_;

	return o;
}

////////////////////////////////////////////////////////////////////////
// domainMain
[domain("tri")]
VS_OUTPUT domainMain( PATCH_OUTPUT Input, 
                      float3 CoordUVW : SV_DomainLocation,
                      const OutputPatch< VS_OUTPUT, 3 > Patch )
{
	VS_OUTPUT o;
	float4 Position;

	o.Position_ = CoordUVW.x * Patch[0].Position_ + 
	              CoordUVW.y * Patch[1].Position_ + 
	              CoordUVW.z * Patch[2].Position_;

	o.Colour_ = CoordUVW.x * Patch[0].Colour_ + 
	            CoordUVW.y * Patch[1].Colour_ + 
	            CoordUVW.z * Patch[2].Colour_;

	o.Normal_ = CoordUVW.x * Patch[0].Normal_ + 
	            CoordUVW.y * Patch[1].Normal_ + 
	            CoordUVW.z * Patch[2].Normal_;

	o.TexCoord0_ = CoordUVW.x * Patch[0].TexCoord0_ + 
	               CoordUVW.y * Patch[1].TexCoord0_ + 
	               CoordUVW.z * Patch[2].TexCoord0_;

	return o;
}

#endif // PSY_BACKEND_TYPE != PSY_BACKEND_TYPE_GLSL_ES


////////////////////////////////////////////////////////////////////////
// pixelMain
PS_OUTPUT pixelMain( VS_OUTPUT i )
{
	PS_OUTPUT o = (PS_OUTPUT)0;
	o.Colour_ = i.Colour_;
	return o;
}
