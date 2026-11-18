#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float longitude;
out vec4 finalColor;
void main() {
    vec2 p = fragTexCoord * 2.0 - 1.0;
    float r2 = dot(p,p);
    if (r2 > 1.0) discard;
    vec3 n = vec3(p.x, -p.y, sqrt(max(0.0, 1.0-r2)));
    vec2 uv = vec2(0.5 + atan(n.x,n.z)/6.283185, acos(n.y)/3.141593);
    vec3 skin = texture(texture0,vec2(fract(uv.x + longitude),uv.y)).rgb;
    float light = 0.30 + 0.70 * max(0.0,dot(n,normalize(vec3(-0.5,0.7,1.2))));
    float shine = pow(max(0.0,dot(n,normalize(vec3(-0.35,0.5,1.0)))),28.0);
    float a = longitude*6.283185;
    vec3 b = vec3(n.x*cos(a)+n.z*sin(a), -n.y, n.z*cos(a)-n.x*sin(a));
    float hole = b.z < 0.0 ? 1.0 : min(length(b.xy-vec2(.46,-.32)), min(length(b.xy-vec2(.65,-.19)), length(b.xy-vec2(.44,.01))));
    vec3 color = skin*light + vec3(0.38)*shine;
    color *= mix(0.12,1.0,smoothstep(0.055,0.085,hole));
    finalColor = vec4(color,1.0) * fragColor * colDiffuse;
}
