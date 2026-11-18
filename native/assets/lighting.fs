#version 330
in vec2 uv;
in vec3 normal;
in vec3 world;
in vec4 color;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform int woodMode;
uniform vec3 eye;
uniform vec3 pinShadows[10];
uniform vec3 ballShadow;
uniform float sceneTime;
out vec4 finalColor;
float hash(float n) { return fract(sin(n)*43758.5453); }
float noise(vec3 p) {
    vec3 i=floor(p), f=fract(p); f=f*f*(3.0-2.0*f);
    float n=dot(i,vec3(1.0,57.0,113.0));
    return mix(mix(mix(hash(n),hash(n+1.0),f.x),mix(hash(n+57.0),hash(n+58.0),f.x),f.y),
               mix(mix(hash(n+113.0),hash(n+114.0),f.x),mix(hash(n+170.0),hash(n+171.0),f.x),f.y),f.z);
}
void main() {
    vec3 n=normalize(normal), v=normalize(eye-world);
    vec3 tex=texture(texture0,uv).rgb*colDiffuse.rgb*color.rgb;
    vec3 key=normalize(vec3(-.35,.85,-.4));
    float diffuse=max(dot(n,key),0.0);
    float rim=pow(1.0-max(dot(n,v),0.0),3.0);
    bool floorMaterial=woodMode==1 || woodMode==5;
    float broad=noise(world*.42), detail=noise(world*2.4);
    if(woodMode==3) {
        // Broad painted mineral patches, fine mottling and cool turquoise veins.
        tex*=.64+broad*.65+detail*.16;
        float vein=pow(1.0-abs(sin(world.y*.85+world.z*.38+broad*5.0)),8.0);
        tex=mix(tex,vec3(.27,.69,.73),vein*.24);
    }
    vec3 lit=tex*(vec3(.29,.33,.39)+diffuse*vec3(.95,.85,.72));
    lit+=tex*max(dot(n,normalize(vec3(.7,.2,.6))),0.0)*vec3(.09,.20,.30);
    float gloss=pow(max(dot(n,normalize(key+v)),0.0),floorMaterial?100.0:62.0);
    lit+=vec3(.85,.91,1.0)*gloss*(floorMaterial?.48:.46)+rim*vec3(.035,.07,.09);
    if(woodMode==1) {
        float plank=floor((world.x+30.0)*3.4);
        float grain=sin(world.x*165.0+noise(vec3(world.x*8.0,0,world.z*.7))*3.0);
        float aa=1.0-smoothstep(.4,2.5,fwidth(world.x*165.0));
        lit*=.90+.12*hash(plank)+grain*.022*aa;
        float seam=fract((world.x+30.0)*3.4);
        float lineWidth=max(.012,fwidth(world.x*3.4));
        lit*=mix(.88,1.0,smoothstep(0.0,lineWidth,seam)*smoothstep(0.0,lineWidth,1.0-seam));
        float end=fract((world.z+hash(plank)*7.0)/4.5);
        lit*=mix(.93,1.0,smoothstep(0.0,max(.004,fwidth(world.z/4.5)),end));
    }
    if(woodMode==5) {
        float streak=noise(vec3(world.x*3.0,0,world.z*.12));
        lit=mix(lit,vec3(.28,.63,.77),streak*.32);
        float seam=abs(fract(world.x*.33+.5)-.5);
        lit+=vec3(.18,.24,.26)*exp(-seam*100.0)*.35;
        lit+=vec3(.07,.14,.17)*sin(world.z*.24+world.x*.6);
    }
    if(floorMaterial) {
        float shadow=0.0;
        for(int i=0;i<10;++i) {
            vec2 d=(world.xz-pinShadows[i].xz)/vec2(.50,.85);
            shadow+=exp(-dot(d,d)*2.0)*.50;
        }
        vec2 d=(world.xz-ballShadow.xz)/vec2(.8,1.2);
        shadow+=exp(-dot(d,d)*1.8)*.60;
        lit*=1.0-min(shadow,.75);
        // Soft elongated ceiling-light glints in the varnish, not scene reflections.
        vec3 r=reflect(-v,n);
        if(r.y>.01) {
            vec3 hit=world+r*((7.0-world.y)/r.y);
            float bands=exp(-pow(sin((hit.z-1.0)*3.14159/11.0)*5.0,2.0));
            float across=exp(-pow(hit.x/2.0,4.0));
            float fresnel=.12+.35*pow(1.0-max(dot(n,v),0.0),4.0);
            lit+=vec3(1.0,.87,.62)*bands*across*fresnel;
        }
        float edge=exp(-pow((abs(world.x)-4.8)/.6,2.0));
        lit+=vec3(.03,.16,.22)*edge;
    }
    if(woodMode==2) {
        vec2 brick=vec2((abs(n.z)>.5?world.x:world.z)*1.25+floor(world.y*2.8)*.5,world.y*2.8);
        vec2 f=fract(brick);
        float mortar=smoothstep(.02,.07,f.x)*smoothstep(.04,.12,f.y);
        lit*=.8+.25*hash(floor(brick.x)+floor(brick.y)*19.0)+detail*.13;
        lit=mix(vec3(.16,.15,.14),lit,mortar);
    }
    if(woodMode==3) {
        float caustic=pow(.5+.5*sin(world.z*1.7+world.y*2.1+broad*7.0),12.0);
        lit+=vec3(.06,.26,.28)*caustic;
        lit*=.76+.24*smoothstep(0.0,5.0,world.y);
    }
    if(woodMode==4) {
        float wave=sin(world.z*3.0+world.x*2.0+sceneTime*1.1)+sin(world.z*1.5-world.x*3.0-sceneTime);
        lit=tex*(.7+wave*.12)+vec3(.12,.35,.38)*pow(.5+.5*sin(wave*3.0),6.0);
    }
    float fog=1.0-exp(-length(eye-world)*.0035);
    lit=mix(lit,vec3(.09,.17,.24),fog);
    finalColor=vec4(lit,colDiffuse.a*color.a);
}
