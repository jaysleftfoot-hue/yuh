import * as T from 'three';
import {makeBall,makePin,makeStadium} from './assets3d.js';
const colors={ARI:0xa71930,ATL:0xce1141,BOS:0xbd3039,CHC:0x0e3386,LAD:0x005a9c,NYY:0x132448,SF:0xfd5a1e,NYM:0x002d72,PHI:0xe81828,SEA:0x005c5c};
export class PlaceholderRenderer {
 constructor(canvas) {
  this.canvas=canvas; this.webgl=new T.WebGLRenderer({canvas,antialias:true});this.webgl.setPixelRatio(Math.min(devicePixelRatio,1.5));this.webgl.shadowMap.enabled=false;this.webgl.shadowMap.type=T.PCFSoftShadowMap;this.webgl.toneMapping=T.ACESFilmicToneMapping;
  this.scene=new T.Scene();this.scene.background=new T.Color(0x8cb4c5);this.scene.fog=new T.Fog(0x8cb4c5,55,130);
  this.camera=new T.PerspectiveCamera(49,1,.1,180);
  this.scene.add(new T.HemisphereLight(0xe7f5ff,0x6b6442,2.4));const sun=new T.DirectionalLight(0xffdfac,3);sun.position.set(-15,30,12);sun.castShadow=true;sun.shadow.mapSize.set(2048,2048);Object.assign(sun.shadow.camera,{left:-35,right:35,top:35,bottom:-35});this.scene.add(sun);
  this.ball=makeBall();this.scene.add(this.ball);this.pinMeshes=new Map();this.setTeam('ARI');this.clock=new T.Clock();
  const points=[new T.Vector3(),new T.Vector3()];this.guide=new T.Line(new T.BufferGeometry().setFromPoints(points),new T.LineDashedMaterial({color:0xffffff,dashSize:.3,gapSize:.2}));this.scene.add(this.guide);
 }
 setTeam(id) {this.team=id;const color=colors[id]??0x184d76;if(this.stadium)this.scene.remove(this.stadium);this.stadium=makeStadium(color);this.scene.add(this.stadium);for(const p of this.pinMeshes.values())this.scene.remove(p);this.pinMeshes.clear();this.color=color;}
 draw(sim,shot,preview) {
  const w=this.canvas.clientWidth,h=this.canvas.clientHeight;
  if(this.width!==w||this.height!==h){this.width=w;this.height=h;this.webgl.setSize(w,h,false);this.camera.aspect=w/h;this.camera.updateProjectionMatrix();}
  const time=this.clock.getElapsedTime();const ids=new Set(sim.pins.map(p=>p.id));
  for(const [id,m]of this.pinMeshes)if(!ids.has(id)){this.scene.remove(m);this.pinMeshes.delete(id);}
  for(const p of sim.pins){let m=this.pinMeshes.get(p.id);if(!m){m=makePin(this.color);this.pinMeshes.set(p.id,m);this.scene.add(m);}m.position.set(p.x-5,.12,-p.y);m.rotation.x=T.MathUtils.lerp(m.rotation.x,p.down?-Math.PI/2:0,.17);m.rotation.z=p.down?Math.sin(p.id)*.4:Math.sin(time*2+p.id)*.025;m.position.y+=p.down?.1:Math.sin(time*3+p.id)*.025;}
  const b=preview?{x:shot.position,y:2}:sim.ball;this.ball.visible=!!b;if(b){this.ball.position.set(b.x-5,.62,-b.y);this.ball.rotation.x=-b.y*2;}
  const follow=!preview&&b?Math.min(b.y*.55,12):0;const target=new T.Vector3(preview?0:(b?.x-5)*.25,6.5,10-follow);this.camera.position.lerp(target,.06);this.camera.lookAt(0,1,-18-follow*.3);
  this.guide.visible=preview;
  if(preview){const points=[];const speed=11+shot.power*13;for(let d=0;d<22;d+=.3){const t=d/(Math.cos(shot.aim*.16)*speed);const x=shot.position-5+Math.sin(shot.aim*.16)*speed*t+.425*shot.curve*t*t;if(Math.abs(x)>4.5)break;points.push(new T.Vector3(x,.15,-2-d));}this.guide.geometry.dispose();this.guide.geometry=new T.BufferGeometry().setFromPoints(points);this.guide.computeLineDistances();}
  this.webgl.render(this.scene,this.camera);
 }
}
