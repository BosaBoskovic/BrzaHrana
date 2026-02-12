#version 330 core

in vec4 channelCol;
in vec2 channelTex;

out vec4 outCol;

uniform sampler2D uTex;
uniform bool useTex;
uniform vec4 uColorMod;
uniform bool useVertexColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightIntensity;
uniform bool lightEnabled;

uniform vec3 viewPos;

// NOVO: preska?emo svetlo kad je true
uniform bool ignoreLight;

in vec3 FragPos;
in vec3 Normal;

void main()
{
    vec4 baseColor;

    if (useTex) {
        baseColor = texture(uTex, channelTex);
        if (baseColor.a < 0.1)
            discard;
    } else if (useVertexColor) {
        baseColor = channelCol;
    } else {
        baseColor = vec4(1.0, 1.0, 1.0, 1.0);
    }

    vec4 finalColor = baseColor * uColorMod;

    // Ako je svetlo ON i ne ignorišemo ga
    if(lightEnabled && !ignoreLight)
    {
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);

        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor * lightIntensity;

        vec3 ambient = 0.2 * lightColor; // malo svetla ?ak i kad nije direktno osvetljeno
        vec3 result = (ambient + diffuse) * finalColor.rgb;

        outCol = vec4(result, finalColor.a);
    }
    else
    {
        outCol = finalColor;
    }

}
