#include "Psybrus.hlsl"

////////////////////////////////////////////////////////////////////////
// VS_INPUT
struct VS_INPUT
{
	float4 Position_		: POSITION;
	float4 Normal_			: NORMAL;
	float4 Tangent_			: TANGENT;
	float4 Colour_			: COLOR0;
	float4 TexCoord0_		: TEXCOORD0;

#if defined( PERM_MESH_SKINNED_3D )
	float4 BlendIndices_	: BLENDINDICES;
	float4 BlendWeights_	: BLENDWEIGHTS;

#elif defined( PERM_MESH_PARTICLE_3D )
	float4 VertexOffset_	: TEXCOORD8;

#elif defined( PERM_MESH_INSTANCED_3D )
	float4 WorldMatrix0_	: TEXCOORD4;
	float4 WorldMatrix1_	: TEXCOORD5;
	float4 WorldMatrix2_	: TEXCOORD6;
	float4 WorldMatrix3_	: TEXCOORD7;
#endif
};

////////////////////////////////////////////////////////////////////////
// VS_OUTPUT
struct VS_OUTPUT
{
	float4 Position_	: SV_POSITION;
	float4 Normal_		: NORMAL;
	float4 Tangent_		: TANGENT;
	float4 Colour_		: COLOR0;
	float4 TexCoord0_	: TEXCOORD0;

#if defined( PERM_MESH_SKINNED_3D )
	float4 BlendIndices_	: BLENDINDICES;
	float4 BlendWeights_	: BLENDWEIGHTS;

#elif defined( PERM_MESH_PARTICLE_3D )
	float4 VertexOffset_	: TEXCOORD8;

#elif defined( PERM_MESH_INSTANCED_3D )
	float4 WorldMatrix0_	: TEXCOORD4;
	float4 WorldMatrix1_	: TEXCOORD5;
	float4 WorldMatrix2_	: TEXCOORD6;
	float4 WorldMatrix3_	: TEXCOORD7;
#endif
};

////////////////////////////////////////////////////////////////////////
// GS_OUTPUT
struct GS_OUTPUT
{
	float4 Position_	: SV_POSITION;
	float4 Normal_		: NORMAL;
	float4 Tangent_		: TANGENT;
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
	o.Position_ = i.Position_;
	o.Normal_ = i.Normal_;
	o.Tangent_ = i.Tangent_;
	o.Colour_ = i.Colour_;
	o.TexCoord0_ = i.TexCoord0_;

#if defined( PERM_MESH_SKINNED_3D )
	o.BlendIndices_	= i.BlendIndices_;
	o.BlendWeights_ = i.BlendWeights_;

#elif defined( PERM_MESH_PARTICLE_3D )
	o.VertexOffset_ = i.VertexOffset_;

#elif defined( PERM_MESH_INSTANCED_3D )
	o.WorldMatrix0_	= i.WorldMatrix0_;
	o.WorldMatrix1_ = i.WorldMatrix1_;
	o.WorldMatrix2_ = i.WorldMatrix2_;
	o.WorldMatrix3_ = i.WorldMatrix3_;
#endif
	return o;
}

#if PSY_BACKEND_TYPE != PSY_BACKEND_TYPE_GLSL_ES

////////////////////////////////////////////////////////////////////////
// geometryMain
[maxvertexcount(32)]
void geometryMain( triangle VS_OUTPUT Input[3],
                   inout TriangleStream< GS_OUTPUT > OutputStream )
{
	for( float j = -4; j <= 4; j += 1.0 )
	{
		for( int i = 0; i < 3; ++i )
		{
			GS_OUTPUT Output;
			float4 WorldPosition;
			PSY_MAKE_WORLD_SPACE_VERTEX( WorldPosition, Input[i].Position_, Input[i] );

			WorldPosition.z += j * 10.0;

			PSY_MAKE_CLIP_SPACE_VERTEX( Output.Position_, WorldPosition );

			Output.Normal_ = Input[i].Normal_;
			Output.Tangent_ = Input[i].Tangent_;
			Output.Colour_ = Input[i].Colour_;
			Output.TexCoord0_ = Input[i].TexCoord0_;

			OutputStream.Append( Output );
		}

		OutputStream.RestartStrip();
	}
}

#endif // PSY_BACKEND_TYPE != PSY_BACKEND_TYPE_GLSL_ES

////////////////////////////////////////////////////////////////////////
// pixelMain
PS_OUTPUT pixelMain( GS_OUTPUT i )
{
	PS_OUTPUT o = (PS_OUTPUT)0;
	o.Colour_ = i.Colour_;
	return o;
}
