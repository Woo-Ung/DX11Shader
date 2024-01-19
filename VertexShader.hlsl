cbuffer cBuffer
{
    float time;
}

struct Vertex
{
	float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct Interpolants
{
	float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

Interpolants main( Vertex In )
{
    Interpolants Out;
    
    Out.position = In.position;
    
    ////Out.position += sin(time) * float4(0.1, 0.1, 0.0, 0.0);  //
    //Out.position.x += sin(time * 3 + In.position.x + In.position.y) * 0.1;
    //Out.position.y += cos(time * 3 + In.position.x + In.position.y) * 0.1;
    
    //if(Out.position.y > 0.0)
    //{
    //    Out.position.x += sin(time*3) * 0.1;        
    //}
    
    Out.tex = In.tex;
    
	return Out;
}