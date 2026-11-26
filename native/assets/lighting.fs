#version 330
in vec2 uv;
in vec3 normal;
in vec3 world;
in vec3 localPos;
in vec4 color;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform int woodMode;
uniform vec3 eye;
uniform vec4 pinShadows[10];
uniform vec3 ballShadow;
uniform float sceneTime;
uniform vec2 boardLight;
uniform float laneRun;
uniform vec3 keyDirection, keyTint;
uniform float roofLight;
uniform int aquarium;
uniform vec3 finLoad;
out vec4 finalColor;
// GLSL pow(x,n) is undefined for x<0, even for integer-valued n.
// Signed floor reflections previously produced NaNs on some Windows drivers.
float square(float x) {return x*x;}
float fourth(float x) {float q=x*x;return q*q;}
float hash(float n) { return fract(sin(n)*43758.5453); }
float noise(vec3 p) {
    vec3 i=floor(p), f=fract(p); f=f*f*(3.0-2.0*f);
    float n=dot(i,vec3(1.0,57.0,113.0));
    return mix(mix(mix(hash(n),hash(n+1.0),f.x),mix(hash(n+57.0),hash(n+58.0),f.x),f.y),
               mix(mix(hash(n+113.0),hash(n+114.0),f.x),mix(hash(n+170.0),hash(n+171.0),f.x),f.y),f.z);
}
void main() {
    vec3 n=normalize(normal), v=normalize(eye-world);
    vec2 sampleUV=uv;
    float holeLight=1.0;
    if (woodMode==7) {
        vec3 b=normalize(localPos);
        sampleUV=vec2(.5+atan(-b.x,-b.z)/6.283185,acos(clamp(b.y,-1.,1.))/3.141593);
        if (b.z<0.) {
            vec2 p=vec2(-b.x,-b.y);
            float hole=min(length(p-vec2(.46,-.32)),min(length(p-vec2(.65,-.19)),length(p-vec2(.44,.01))));
            float bevel=smoothstep(.048,.084,hole);
            holeLight=mix(.038,1.,bevel)+.22*exp(-square((hole-.084)/.009));
        }
    }
    if (woodMode==6 && uv.x<.48 && uv.y>.65) {
        // Move the painted irises within the eye islands and close the upper lids on a blink.
        float socket=1.-smoothstep(.7,1.,length((uv-vec2(.246,.81))/vec2(.19,.18)));
        sampleUV.x+=socket*(.021*finLoad.y+.018*finLoad.x*sign(localPos.x));
    }
    vec3 tex=texture(texture0,sampleUV).rgb*colDiffuse.rgb*color.rgb;
    if (woodMode==6 && uv.x<.48 && uv.y>.65) {
        float blink=pow(max(0.,cos(sceneTime*1.49)),180.);
        float lid=.736+.181*blink+.019*finLoad.x;
        tex=mix(tex,vec3(.09,.35,.43),1.-smoothstep(lid-.005,lid+.005,uv.y));
    }
    vec3 key=normalize(keyDirection);
    float diffuse=max(dot(n,key),0.0);
    float rim=pow(1.0-clamp(dot(n,v),0.0,1.0),3.0);
    bool floorMaterial=woodMode==1 || woodMode==5;
    float broad=noise(world*.42), detail=noise(world*2.4);
    if(woodMode==3) {
        // Broad painted mineral patches, fine mottling and cool turquoise veins.
        tex*=.64+broad*.65+detail*.16;
        float vein=pow(1.0-abs(sin(world.y*.85+world.z*.38+broad*5.0)),8.0);
        tex=mix(tex,vec3(.27,.69,.73),vein*.24);
    }
    vec3 lit=tex*(vec3(.29,.33,.39)+diffuse*.91*keyTint);
    lit+=tex*max(dot(n,normalize(vec3(.7,.2,.6))),0.0)*vec3(.09,.20,.30);
    float gloss=pow(max(dot(n,normalize(key+v)),0.0),floorMaterial?100.0:62.0);
    lit+=vec3(.85,.91,1.0)*gloss*(floorMaterial?.48:.46)+rim*vec3(.035,.07,.09);
    if(woodMode==7) {
        // Two long diffusers reflected in clear resin; the team paint rolls beneath them.
        vec3 r=reflect(-v,n);
        float boxLight=1.-smoothstep(.88,1.08,max(abs(r.x+.32)/.36,abs(r.y-.72)/.095));
        float strip=1.-smoothstep(.78,1.15,max(abs(r.x-.61)/.085,abs(r.y-.48)/.30));
        lit+=max(-r.z,0.)*(boxLight*vec3(.77,.73,.65)+strip*vec3(.30,.37,.46));
    }
    if(woodMode==7 || woodMode==9 || woodMode==12) {
        vec3 reflected=reflect(-v,n);
        float horizon=exp(-square((reflected.y-.15)/.12));
        lit+=keyTint*horizon*(.05+.14*rim);
        if(woodMode==12) lit+=vec3(.56,.87,.94)*(.44*gloss+.19*rim);
        if(woodMode==9) {
            float rub=1.-smoothstep(.0,.15,abs(localPos.y-.64+.027*sin(localPos.x*24.)));
            lit*=1.-.10*rub*noise(localPos*41.);
        }
    }
    if(woodMode==1) {
        // Broad catwalk reflections at the Trop and raking sunlight in the open parks.
        float roofBand=exp(-square(sin(world.z*.21+world.x*.13)*3.8));
        float beamEdge=exp(-square((abs(world.x)-2.7)/1.65));
        lit+=keyTint*roofBand*beamEdge*roofLight*.17;
        lit*=mix(vec3(1.),keyTint,.19);
        float plank=floor((world.x+30.0)*3.4);
        float grain=sin(world.x*165.0+noise(vec3(world.x*8.0,0,world.z*.7))*3.0);
        float aa=1.0-smoothstep(.4,2.5,fwidth(world.x*165.0));
        float ribbon=sin(world.x*24.0+2.8*noise(vec3(plank,0,world.z*.19)));
        lit*=.90+.12*hash(plank)+grain*.022*aa+.023*ribbon*aa;
        // A soft reflection across the waxed pocket boards under the deck lights.
        float varnish=exp(-square((world.x+1.1)/2.8))*smoothstep(.0,.9,1.-abs(n.z));
        lit+=vec3(.024,.016,.009)*varnish;
        float seam=fract((world.x+30.0)*3.4);
        float lineWidth=max(.012,fwidth(world.x*3.4));
        float boardLine=smoothstep(0.0,lineWidth,seam)*smoothstep(0.0,lineWidth,1.0-seam);
        lit*=mix(.88+.12*boardLine,.98,smoothstep(.18,.7,lineWidth));
        float end=fract((world.z+hash(plank)*7.0)/4.5);
        lit*=mix(.93,1.0,smoothstep(0.0,max(.004,fwidth(world.z/4.5)),end));
    }
    if(woodMode==5) {
        float streak=noise(vec3(world.x*3.0,0,world.z*.12));
        lit=mix(lit,vec3(.28,.63,.77),streak*.32);
        float seam=abs(fract(world.x*.33+.5)-.5);
        lit+=vec3(.18,.24,.26)*exp(-seam*100.0)*.35;
        lit*=vec3(.82,.91,.94);
        // Shallow blue glass carries soft cross-light from the exhibit mullions.
        float sweep=sin(world.z*.21+world.x*.76+sceneTime*.28);
        lit+=vec3(.025,.073,.083)*sweep;
        float glint=1.0-smoothstep(.015,.18,abs(sweep+.24*sin(world.x*1.62-sceneTime*.41)));
        lit+=vec3(.055,.085,.065)*glint;
        float caustic=.5+.5*sin(world.x*2.8+world.z*.38+sin(world.z*.72-sceneTime*1.4)*.7);
        float ripple=.5+.5*sin(world.x*1.5-world.z*.52+sceneTime*.96);
        lit+=vec3(.07,.14,.17)*(pow(caustic*ripple,5.0));
    }
    if(floorMaterial) {
        float shadow=0.0;
        // The soft cast shadow follows the park lights. A tighter core anchors each foot.
        vec2 castDirection=-key.xz/max(.42,key.y);
        for(int i=0;i<10;++i) {
            vec2 axis=pinShadows[i].zw;
            vec2 offset=world.xz-pinShadows[i].xy;
            float along=clamp(dot(offset,axis)/max(.001,dot(axis,axis)),0.,1.);
            vec2 d=(offset-axis*along)/vec2(.50,.85);
            shadow+=exp(-dot(d,d)*2.0)*mix(.58,.36,along);
            vec2 penumbra=offset-axis*along-castDirection*.47;
            shadow+=.17*exp(-dot(penumbra,penumbra)*1.46);
        }
        vec2 footprint=woodMode==5 ? vec2(1.46,1.63) : ballShadow.y*vec2(.90,1.13);
        vec2 d=(world.xz-ballShadow.xz)/footprint;
        shadow+=exp(-dot(d,d)*1.8)*.58;
        vec2 castOffset=(world.xz-ballShadow.xz-castDirection*.52)/footprint;
        shadow+=.20*exp(-dot(castOffset,castOffset)*1.3);
        shadow+=.15*exp(-square((abs(world.x)-4.94)/.16));
        if(woodMode==1) shadow+=exp(-dot(d,d)*9.)*.24;
        lit*=1.0-min(shadow,.75);
        // Soft elongated ceiling-light glints in the varnish, not scene reflections.
        vec3 r=reflect(-v,n);
        if(r.y>.01) {
            vec3 hit=world+r*((7.0-world.y)/r.y);
            float bands=exp(-square(sin((hit.z-1.0)*3.14159/11.0)*5.0));
            float across=exp(-fourth(hit.x/2.0));
            float fresnel=.12+.35*pow(1.0-clamp(dot(n,v),0.0,1.0),4.0);
            lit+=vec3(1.0,.87,.62)*bands*across*fresnel;
        }
        float screenReach=exp(-square((world.z-boardLight.y)/24.0)-square(world.x/4.0));
        lit+=vec3(.15,.28,.49)*boardLight.x*screenReach;
        float edge=exp(-square((abs(world.x)-4.8)/.6));
        lit+=vec3(.03,.16,.22)*edge;
    }
    if(woodMode==9) {
        // Lacquer over painted maple picks up a warm key and a cool strip of fill.
        vec3 refl=reflect(-v,n);
        float softbox=smoothstep(.86,.98,dot(refl,normalize(vec3(.65,.4,-.55))));
        lit+=softbox*vec3(.22,.21,.16)+rim*vec3(.026,.052,.064);
    }
    if(woodMode==8) {
        float blades=noise(vec3(world.x*12.0,0.,world.z*8.0));
        blades=mix(blades,.5,smoothstep(.25,1.3,length(fwidth(world.xz)*vec2(12.,8.))));
        lit*=.80+.24*broad+.18*blades;
        lit+=vec3(.04,.025,.003)*smoothstep(.55,.88,blades);
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
    if(woodMode==10) {
        // Looking up through the tunnel roof, broad bands of sunlight drift across the water.
        vec2 water=world.xz*vec2(.56,.22);
        water+=vec2(.65*sin(water.y+sceneTime*.30),.52*sin(water.x*.9-sceneTime*.24));
        float crest=abs(sin(water.x+water.y)+.72*sin(water.y*1.64-water.x));
        float caustic=1.0-smoothstep(.025,.21,crest);
        float shafts=.5+.5*sin(world.z*.29-world.x*.81 + broad*2.4);
        lit=mix(vec3(.027,.16,.24),vec3(.12,.49,.54),shafts*.65+detail*.24);
        lit+=caustic*vec3(.06,.19,.17);
        lit=mix(tex*.68,lit,smoothstep(4.2,8.0,world.y));
    }
    if(woodMode==5 || woodMode==10 || woodMode==4) {
        float course=(world.z-2.)/laneRun;
        float forest=smoothstep(.26,.38,course)*(1.-smoothstep(.64,.76,course));
        lit*=mix(vec3(1.),vec3(.35,.52,.63),forest);
        float daylight=smoothstep(.68,.87,course);
        float shafts=.5+.5*sin(world.x*1.35+world.z*.31+sceneTime*.19);
        lit+=daylight*vec3(.105,.073,.024)*shafts;
    }
    if(woodMode==13) {
        // Orange-peel enamel with primer showing through along rolled steel edges.
        vec3 d=abs(localPos);
        float bevel=max(min(d.x,d.y),max(min(d.y,d.z),min(d.x,d.z)));
        float wear=smoothstep(.46,.498,bevel)*smoothstep(.42,.72,detail);
        float patina=noise(world*vec3(1.8,.30,2.1));
        lit=lit*(.81+.23*patina+.10*detail)+keyTint*gloss*.18;
        lit=mix(lit,vec3(.43,.40,.30),wear*.65);
    }
    if(woodMode==11) lit=mix(lit,vec3(1.,.47,.049),.56)+vec3(.085,.041,.004)*broad;
    if(aquarium==1 && world.y<8.) {
        vec2 water=world.xz*vec2(1.32,.43)+world.y*vec2(.71,-.49);
        float ridge=abs(sin(water.x+sceneTime*.88+sin(water.y))+
                        .69*sin(water.y*1.67-sceneTime*.72+water.x*.41));
        float shaft=(1.-smoothstep(.025,.20,ridge))*(.28+.72*max(n.y,0.));
        lit+=vec3(.065,.19,.17)*shaft;
    } else if(aquarium==0) {
        float pool=exp(-square((world.z-boardLight.y)/25.)-square(world.x/17.));
        lit+=vec3(.06,.15,.32)*pool*boardLight.x*(.34+.66*abs(n.z));
        float grassBounce=max(0.,-n.y)*exp(-abs(world.y)*.31);
        lit+=tex*vec3(.027,.072,.034)*grassBounce;
    }
    float fog=1.0-exp(-length(eye-world)*.0035);
    lit=mix(lit,vec3(.09,.17,.24),fog);
    finalColor=vec4(lit*holeLight,colDiffuse.a*color.a);
}
