#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"

#ifdef COMPILEPS
    uniform float cDissolve;
#endif

#ifdef VSM_SHADOW
    varying vec3 vTexCoord;
#else
    varying vec2 vTexCoord;
#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    #ifdef VSM_SHADOW
        vTexCoord = vec3(GetTexCoord(iTexCoord), gl_Position.z / gl_Position.w * 0.5 + 0.5);
    #else
        vTexCoord = GetTexCoord(iTexCoord);
    #endif
}

void PS()
{
    #ifdef ALPHAMASK
        vec4 dissolveMapInput = texture2D(sEmissiveMap, vTexCoord.xy);
        if (dissolveMapInput.r < cDissolve)
            discard;
    #endif

    #ifdef VSM_SHADOW
        float depth = vTexCoord.z;
        gl_FragColor = vec4(depth, depth * depth, 1.0, 1.0);
    #else
        gl_FragColor = vec4(1.0);
    #endif
}
