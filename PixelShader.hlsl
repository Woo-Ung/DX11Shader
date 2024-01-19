cbuffer cBuffer
{
    float time;
}
Texture2D shaderTexture;
SamplerState Sampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

struct Interpolants
{
	float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

struct Pixel
{
	float4 color : SV_TARGET;
};

Pixel main(Interpolants In)
{
    Pixel Out;
    Out.color = shaderTexture.Sample(Sampler, In.tex);
    
    //Out.color.g = 1;  //rgb변환
    
    //Out.color.rgb = (Out.color.r + Out.color.g * Out.color.b) / 3; //그레이스케일
    
    //Out.color.r += (sin(time*5) + 1) / 2;
    
    /* float2 p = In.tex;
    //Out.color.rgb += length(p - float2(0.5, 0.5)); //합연산
    //Out.color.rgb *= length(p - float2(0.5, 0.5)); //곱연산
    //Out.color.rgb *= 1 - length(p - float2(0.5, 0.5)); //리버스
    
    //p.x += sin(time * 2);
    //p.y += cos(time * 2);
    //Out.color.rgb += 1 - length(p - float2(0.5, 0.5)); // * (sin(time * 2) * 2) //원크기조절
    
    Out.color.a = 1; */
    
    // rose petal
    In.tex = In.tex * 2 - 1; // -1 to 1
    
    float dist = length(In.tex);
    float size = 1; // petal size
    float petals = 6; // petal count
    float thick = 0.3; // petal thickness
    float inset = sin(time); // petal length
    
    if (dist <= thick * (size + inset * sin(petals * atan2(In.tex.y, In.tex.x))))
    {
        Out.color = float4(1, 1, 1, 1);
    }
    
    return Out;

}