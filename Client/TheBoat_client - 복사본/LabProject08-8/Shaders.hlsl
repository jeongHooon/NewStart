cbuffer cbPlayerInfo : register(b0)
{
	matrix		gmtxPlayerWorld : packoffset(c0);
};

cbuffer cbCameraInfo : register(b1)
{
	matrix		gmtxView : packoffset(c0);
	matrix		gmtxProjection : packoffset(c4);
	float3		gvCameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b2)
{
	matrix		gmtxGameObject : packoffset(c0);
	uint		gnMaterial : packoffset(c4);
};

#include "Light.hlsl"

struct VS_DIFFUSED_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
};

struct VS_DIFFUSED_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

VS_DIFFUSED_OUTPUT VSDiffused(VS_DIFFUSED_INPUT input)
{
	VS_DIFFUSED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.color = input.color;

	return(output);
}

float4 PSDiffused(VS_DIFFUSED_OUTPUT input) : SV_TARGET
{
	return(input.color);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
VS_DIFFUSED_OUTPUT VSPlayer(VS_DIFFUSED_INPUT input)
{
	VS_DIFFUSED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxPlayerWorld), gmtxView), gmtxProjection);
	output.color = input.color;

	return(output);
}

float4 PSPlayer(VS_DIFFUSED_OUTPUT input) : SV_TARGET
{
	return(input.color);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D gtxtTexture : register(t0);
Texture2D gtxtTextures : register(t5);
SamplerState gWrapSamplerState : register(s0);
SamplerState gClampSamplerState : register(s1);

struct VS_TEXTURED_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};

struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_TEXTURED_OUTPUT VSTextured(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

VS_TEXTURED_OUTPUT VSTexturedUI(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSTextured(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtTexture.Sample(gWrapSamplerState, input.uv);
	/*if (cColor.a <= 0.1f)
	{
		clip(-1);
	}*/
	
	return(cColor);
}
float4 PSShadowPlayer(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtTexture.Sample(gWrapSamplerState, input.uv);
	if(cColor.a <= 0)
		return(float4(0.0, 0.0, 0.0, 0.0f));
    return(float4(0.1, 0.1, 0.1, 0.8));
}
#define _WITH_VERTEX_LIGHTING

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

struct VS_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
#ifdef _WITH_VERTEX_LIGHTING
	float4 color : COLOR;
#endif
};

VS_LIGHTING_OUTPUT VSLighting(VS_LIGHTING_INPUT input)
{
	VS_LIGHTING_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
#ifdef _WITH_VERTEX_LIGHTING
	output.normalW = normalize(output.normalW);
	output.color = Lighting(output.positionW, output.normalW);
#endif
	return(output);
}

float4 PSLighting(VS_LIGHTING_OUTPUT input) : SV_TARGET
{
#ifdef _WITH_VERTEX_LIGHTING
	float4 cIllumination = input.color;
#else
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW);
#endif
	return(cIllumination);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_TEXTURED_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

struct VS_TEXTURED_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	//	nointerpolation float3 normalW : NORMAL;
	float2 uv : TEXCOORD;
#ifdef _WITH_VERTEX_LIGHTING
	float4 color : COLOR;
#endif
};

VS_TEXTURED_LIGHTING_OUTPUT VSTexturedLighting(VS_TEXTURED_LIGHTING_INPUT input)
{
	VS_TEXTURED_LIGHTING_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;
#ifdef _WITH_VERTEX_LIGHTING
	output.normalW = normalize(output.normalW);
	output.color = Lighting(output.positionW, output.normalW);
#endif
	return(output);
}

float4 PSTexturedLighting(VS_TEXTURED_LIGHTING_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtTexture.Sample(gWrapSamplerState, input.uv);
#ifdef _WITH_VERTEX_LIGHTING
	float4 cIllumination = input.color;
#else
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW);
#endif
	return(lerp(cColor, cIllumination, 0.5f));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D gtxtTerrainBaseTexture : register(t1);
Texture2D gtxtTerrainDetailTexture : register(t2);

struct VS_TERRAIN_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

struct VS_TERRAIN_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_OUTPUT output;

#ifdef _WITH_CONSTANT_BUFFER_SYNTAX
	output.position = mul(mul(mul(float4(input.position, 1.0f), gcbGameObjectInfo.mtxWorld), gcbCameraInfo.mtxView), gcbCameraInfo.mtxProjection);
#else
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
#endif
	output.color = input.color;
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;

	return(output);
}

float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
	float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gWrapSamplerState, input.uv0);
	float4 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gWrapSamplerState, input.uv1);
	float4 cColor = input.color * saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));

	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D gtxtSkyBox : register(t3);

float4 PSSkyBox(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtSkyBox.Sample(gClampSamplerState, input.uv);

	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


VS_TEXTURED_OUTPUT VS_UI(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;

	output.position = float4(input.position, 1.0f);
		
	output.uv = input.uv;

	return(output);
}

float4 PS_UI(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtTexture.Sample(gWrapSamplerState, input.uv);
	if (cColor.a <= 0.1f)
	{
		clip(-1);
	}

	return(cColor);
	//return(float4(input.uv.x, input.uv.y, 0.0f, 1.0f));
}

struct INSTANCEDGAMEOBJECTINFO
{
	matrix		m_mtxGameObject;
	uint		gnMaterial;
};


StructuredBuffer<INSTANCEDGAMEOBJECTINFO> gGameObjectInfos : register(t4);

struct VS_INSTANCING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};
struct VS_INSTANCING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 uv : TEXCOORD;
};

VS_INSTANCING_OUTPUT VSInstancing(VS_INSTANCING_INPUT input, uint nInstanceID : SV_InstanceID)
{
	VS_INSTANCING_OUTPUT output;

	//output.position = mul(mul(mul(float4(input.position, 1.0f), gGameObjectInfos[nInstanceID].m_mtxGameObject), gmtxView), gmtxProjection);
	output.normalW = mul(input.normal, (float3x3)gGameObjectInfos[nInstanceID].m_mtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gGameObjectInfos[nInstanceID].m_mtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0), gmtxView), gmtxProjection);

	output.uv = input.uv;

	return(output);
}
float4 PSInstancing(VS_INSTANCING_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtTextures.Sample(gWrapSamplerState, input.uv);
	//float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.5f)
	input.normalW = normalize(input.normalW);
	clip(cColor.a - 0.1f);
	float4 cIllumination = Lighting(input.positionW, input.normalW);
	//float4 cIllumination = float4(0.0f, 0.0f, 0.0f, 0.5f)
	return(lerp(cColor, cIllumination, 0.5f));
	//return (float4(0.0f, 0.0f, 0.0f, 0.5f));
}

//============================================================================

struct VS_SHADOW_OUTPUT
{
	float4 position : SV_POSITION;
};

VS_TEXTURED_OUTPUT VSShadow(VS_INSTANCING_INPUT input, uint nInstanceID : SV_InstanceID)
{
	VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;


	float4 mxf4position = float4(input.position, 1.0f);
	matrix mxf4transform = mul(mul(gGameObjectInfos[nInstanceID].m_mtxGameObject, gmtxView), gmtxProjection);

	output.position = mul(mxf4position, mxf4transform);

	return output;
}

float4 PSShadow(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
	return (float4(0.0f, 0.0f, 0.0f, 0.5f));
}

//=====================================

VS_TEXTURED_OUTPUT VSINST(VS_INSTANCING_INPUT input, uint nInstanceID : SV_InstanceID)
{
	VS_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gGameObjectInfos[nInstanceID].m_mtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSINST(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtTextures.Sample(gWrapSamplerState, input.uv);
	return(cColor);
	//return (float4(0.0f, 0.0f, 0.0f, 0.5f));
}