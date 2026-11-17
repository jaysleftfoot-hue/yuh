import * as T from 'three';
const mat = (color, roughness=.65) => new T.MeshStandardMaterial({color, roughness});
export function mesh(geometry, color, x=0,y=0,z=0) { const m=new T.Mesh(geometry,mat(color)); m.position.set(x,y,z); m.castShadow=true; m.receiveShadow=true; return m; }
export function box(w,h,d,color,x,y,z) { return mesh(new T.BoxGeometry(w,h,d),color,x,y,z); }
export function makePin(color) {
 const g=new T.Group();
 const profile=[[0,0],[.24,0],[.38,.2],[.4,.5],[.29,.85],[.16,1.12],[.17,1.32],[.27,1.48],[.28,1.66],[.18,1.8],[0,1.84]].map(p=>new T.Vector2(...p));
 g.add(mesh(new T.LatheGeometry(profile,24),0xfff2d5));
 g.add(mesh(new T.CylinderGeometry(.18,.18,.16,20),color,0,1.16,0));
 g.add(mesh(new T.SphereGeometry(.3,20,12,0,Math.PI*2,0,Math.PI/2),color,0,1.68,0));
 g.add(box(.55,.06,.4,color,0,1.69,.22));
 for(const x of [-.1,.1]) { g.add(mesh(new T.SphereGeometry(.055,12,8),0x172333,x,1.51,.25)); }
 g.add(box(.13,.035,.03,0x9e493b,0,1.39,.245));
 return g;
}
export function makeBall() {
 const g=new T.Group(); g.add(mesh(new T.SphereGeometry(.5,40,24),0xfff8e8));
 for(const side of [-1,1]) {
  const pts=[];
  for(let i=0;i<=100;i++){const t=i/100*Math.PI*2; pts.push(new T.Vector3(.34*Math.cos(t),.34*Math.sin(t),side*.369));}
  g.add(mesh(new T.TubeGeometry(new T.CatmullRomCurve3(pts),100,.012,6,false),0xbf363b));
  for(let i=0;i<28;i++){const t=i/28*Math.PI*2; const s=box(.08,.017,.018,0xbf363b,.34*Math.cos(t),.34*Math.sin(t),side*.377); s.rotation.z=t+.7;g.add(s);}
 }
 return g;
}
export function makeStadium(color) {
 const g=new T.Group();
 g.add(box(110,.4,110,0x246b4b,0,-.5,-10));
 for(let i=0;i<12;i++)g.add(box(80,.01,4, i%2?0x2d7951:0x286e47,0,-.28,10-i*4));
 g.add(box(15,.1,34,0xa76e47,0,-.18,-13));
 g.add(box(10,.2,30,0xd6a666,0,-.02,-13));
 for(let i=0;i<20;i++)g.add(box(.018,.01,30,0xb88b50,-5+i*.5,.09,-13));
 for(const x of [-5.4,5.4]) {g.add(box(.7,.15,30,0x263c43,x,-.06,-13));g.add(box(.15,.3,30,0xf4d79a,x+Math.sign(x)*.45,.05,-13));}
 g.add(box(10,.015,.1,0x483729,0,.1,-2));
 for(let i=-3;i<=3;i++){const marker=mesh(new T.ConeGeometry(.1,.3,3),0x423c37,i,.11,-9);marker.rotation.x=-Math.PI/2;g.add(marker);}
 for(let tier=0;tier<6;tier++) {
  for(const side of [-1,1])g.add(box(8,1,48, tier%2?0x344b65:color,side*(17+tier*2),tier*1.2,-12));
  g.add(box(65,1,3,tier%2?0x344b65:color,0,tier*1.2,-34-tier*2));
 }
 for(let i=0;i<450;i++){const side=i%2?1:-1,tier=i%6;g.add(mesh(new T.SphereGeometry(.16,6,4),[0xf4c278,0xe75c4a,0xe9e7d8,0x71b0bb][i%4],side*(15+tier*2+(i%3)),tier*1.2+.75,-32+(i*7%45)));}
 g.add(box(12,5,.7,0x18352f,0,7,-35));
 const canvas=document.createElement('canvas');canvas.width=1024;canvas.height=400;const ctx=canvas.getContext('2d');ctx.fillStyle='#102c28';ctx.fillRect(0,0,1024,400);ctx.textAlign='center';ctx.fillStyle='#ffe3a0';ctx.font='bold 85px sans-serif';ctx.fillText('BIG LEAGUE',512,140);ctx.font='bold 100px sans-serif';ctx.fillText('BOWLING',512,260);ctx.font='28px sans-serif';ctx.fillStyle='#fff';ctx.fillText('TEN PINS. ONE BALLPARK.',512,340);const sign=new T.Mesh(new T.PlaneGeometry(11.6,4.6),new T.MeshBasicMaterial({map:new T.CanvasTexture(canvas)}));sign.position.set(0,7,-34.6);g.add(sign);
 for(const x of [-23,23])for(const z of [-29,8]){
  g.add(box(.25,15,.25,0x889b9b,x,7,z));g.add(box(5,1.4,.4,0xfff5d0,x,14,z));
 }
 return g;
}
