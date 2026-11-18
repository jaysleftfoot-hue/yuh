#version 330
// FXAA applied to the scene before drawing the interface, keeping text crisp.
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
out vec4 finalColor;
void main() {
    vec2 p=fragTexCoord, texel=1.0/vec2(textureSize(texture0,0));
    vec3 nw=texture(texture0,p+vec2(-1,-1)*texel).rgb;
    vec3 ne=texture(texture0,p+vec2(1,-1)*texel).rgb;
    vec3 sw=texture(texture0,p+vec2(-1,1)*texel).rgb;
    vec3 se=texture(texture0,p+vec2(1,1)*texel).rgb;
    vec3 m=texture(texture0,p).rgb;
    vec3 luma=vec3(.299,.587,.114);
    float lnw=dot(nw,luma),lne=dot(ne,luma),lsw=dot(sw,luma),lse=dot(se,luma),lm=dot(m,luma);
    float lo=min(lm,min(min(lnw,lne),min(lsw,lse))),hi=max(lm,max(max(lnw,lne),max(lsw,lse)));
    if(hi-lo<max(.035,hi*.10)) { finalColor=vec4(m,1);return; }
    vec2 dir=vec2(-((lnw+lne)-(lsw+lse)),(lnw+lsw)-(lne+lse));
    float reduce=max((lnw+lne+lsw+lse)*.03125,.0078125);
    dir=clamp(dir/(min(abs(dir.x),abs(dir.y))+reduce),vec2(-6),vec2(6))*texel;
    vec3 a=.5*(texture(texture0,p+dir*(-1.0/6.0)).rgb+texture(texture0,p+dir*(1.0/6.0)).rgb);
    vec3 b=.5*a+.25*(texture(texture0,p+dir*(-.5)).rgb+texture(texture0,p+dir*.5).rgb);
    float lb=dot(b,luma);
    finalColor=vec4(lb<lo||lb>hi?a:b,1);
}
