"""Build the original character meshes and pack artwork into glTF. Requires NumPy and Inkscape."""
import json
import math
from pathlib import Path
import struct
import subprocess
import tempfile

import numpy as np

ROOT = Path(__file__).resolve().parent
OUTPUT = ROOT.parent / "assets/models"
PI = math.pi
sin, cos = math.sin, math.cos


class Model:
    def __init__(self):
        self.vertices = []
        self.uv = []
        self.triangles = []

    def patch(self, columns, rows, surface, outward):
        """Sample a parametric surface, orienting its triangles toward the exterior."""
        first = len(self.vertices)
        for row in range(rows + 1):
            for col in range(columns + 1):
                xyz, uv = surface(col / columns, row / rows)
                self.vertices.append(xyz)
                self.uv.append(uv)
        for row in range(rows):
            for col in range(columns):
                a = first + row * (columns + 1) + col
                b = a + columns + 1
                for indices in [(a, a + 1, b), (a + 1, b + 1, b)]:
                    p, q, r = (np.array(self.vertices[i]) for i in indices)
                    normal = np.cross(q - p, r - p)
                    if np.linalg.norm(normal) < 1e-9:
                        continue
                    if np.dot(normal, outward((col+.5)/columns, (row+.5)/rows)) < 0:
                        indices = indices[::-1]
                    self.triangles.append(indices)

    def write(self, name, texture):
        points = np.asarray(self.vertices, dtype='<f4')
        indices = np.asarray(self.triangles, dtype='<u2')
        # Share normals across UV seams without merging the texture coordinates.
        _, weld = np.unique(points.round(5), axis=0, return_inverse=True)
        normals = np.zeros_like(points)
        p, q, r = points[indices].transpose(1,0,2)
        face = np.cross(q-p, r-p)
        for corner in indices.T:
            np.add.at(normals, weld[corner], face)
        normals = normals[weld]
        normals /= np.maximum(np.linalg.norm(normals,axis=1,keepdims=True), 1e-10)
        streams = [points.tobytes(), normals.tobytes(), np.asarray(self.uv,dtype='<f4').tobytes(),
                   indices.tobytes(), texture]
        binary = bytearray()
        views = []
        for stream in streams:
            views.append(dict(buffer=0,byteOffset=len(binary),byteLength=len(stream)))
            binary.extend(stream)
            binary.extend(b'\0'*(-len(binary)%4))
        access = [dict(bufferView=i,componentType=5126,count=len(points),type=t)
                  for i,t in enumerate(['VEC3','VEC3','VEC2'])]
        access[0].update(min=points.min(0).tolist(),max=points.max(0).tolist())
        access.append(dict(bufferView=3,componentType=5123,count=indices.size,type='SCALAR'))
        doc=dict(asset=dict(version='2.0'),scene=0,scenes=[dict(nodes=[0])],nodes=[dict(mesh=0,name=name)],
                 meshes=[dict(primitives=[dict(attributes=dict(POSITION=0,NORMAL=1,TEXCOORD_0=2),indices=3,material=0)])],
                 materials=[dict(pbrMetallicRoughness=dict(baseColorTexture=dict(index=0),metallicFactor=0,roughnessFactor=.45))],
                 textures=[dict(source=0)],images=[dict(bufferView=4,mimeType='image/png')],
                 buffers=[dict(byteLength=len(binary))],bufferViews=views,accessors=access)
        header=json.dumps(doc,separators=(',',':')).encode()
        header+=b' '*(-len(header)%4)
        (OUTPUT/(name+'.glb')).write_bytes(struct.pack('<4sIII4s',b'glTF',2,28+len(header)+len(binary),len(header),b'JSON')+
            header+struct.pack('<I4s',len(binary),b'BIN\0')+binary)


def pin():
    model=Model()
    # Outer radius by height, including a recessed foot and a rounded crown.
    profile=[(0,0),(0,.215),(.018,.239),(.07,.25),(.16,.28),(.30,.34),(.45,.382),
             (.59,.4),(.73,.395),(.89,.365),(1.04,.31),(1.18,.243),(1.31,.182),
             (1.43,.14),(1.53,.136),(1.60,.153),(1.69,.19),(1.77,.222),(1.85,.228),
             (1.92,.209),(1.98,.167),(2.024,.101),(2.047,.042),(2.05,0)]
    def surface(u,v):
        h,r=profile[round(v*(len(profile)-1))]
        a=(u-.5)*2*PI
        return (r*sin(a),h,-r*cos(a)),(u,1-h/2.05)
    model.patch(64,len(profile)-1,surface,lambda u,v:(sin((u-.5)*2*PI), -1 if v<.045 else .4 if v>.8 else 0,-cos((u-.5)*2*PI)))
    return model


def ray():
    model=Model()
    def section(u):
        x=(u*2-1)*2.03
        w=abs(x)/2.03
        length=(1-w**1.3)**.76
        midz=.08-.84*w**1.8
        midy=.32+.22*w**3+.05*cos(w*PI)
        depth=.46*math.exp(-3.4*w*w)*(1-w**2)**.65+.08*length
        return x,length,midz,midy,depth
    def body(u,v,top):
        x,length,z,y,depth=section(u)
        a=v*2*PI
        z+=length*cos(a)*(1.07 if cos(a)>0 else 1.26)
        y+=depth*sin(a)*(1 if sin(a)>0 else .57)
        # Separate dorsal and ventral islands for the painted markings.
        uv=(.025+u*.95, (.03 if top else .48)+(.5-z/3.2)*(.42 if top else .14))
        return (x,y,z),uv
    for top in (True, False):
        offset = 0 if top else .5
        model.patch(56,24,lambda u,v:body(u,offset+v*.5,top),
                    lambda u,v:((2*u-1)*.28,sin((offset+v*.5)*2*PI),cos((offset+v*.5)*2*PI)))

    def globe(center,scale,rect):
        def surface(u,v):
            a=(u-.5)*2*PI
            b=(v-.5)*PI
            radial=(sin(a)*cos(b),sin(b),cos(a)*cos(b))
            return tuple(center[i]+scale[i]*radial[i] for i in range(3)),(rect[0]+rect[2]*u,rect[1]+rect[3]*(1-v))
        model.patch(32,20,surface,lambda u,v:(sin((u-.5)*2*PI)*cos((v-.5)*PI),sin((v-.5)*PI),cos((u-.5)*2*PI)*cos((v-.5)*PI)))
    for side in [-1,1]:
        globe((side*.335,.759,.54),(.225,.23,.195),(.01,.65,.47,.33))
    # Tail is rooted inside the back, with a continuous upswept taper.
    def tail(u,v):
        a=2*PI*u
        radius=(1-v)**.80*.155
        return (.33*sin(v*3)*v+radius*cos(a),.30+v*v*.46+radius*sin(a),-.84-1.75*v),(.505,.71)
    model.patch(20,32,tail,lambda u,v:(cos(2*PI*u),sin(2*PI*u),0))
    def crown(u,v):
        a=(u-.5)*2*PI
        arc=v*PI/2
        return (.46*cos(arc)*sin(a),.84+.36*sin(arc),.06+.42*cos(arc)*cos(a)),(.55+u*.44,.65+(1-v)*.31)
    model.patch(48,20,crown,lambda u,v:(sin((u-.5)*2*PI)*cos(v*PI/2),sin(v*PI/2),cos((u-.5)*2*PI)*cos(v*PI/2)))
    def brim(u,v):
        a=(u-.5)*2*PI
        radius=cos((v-.5)*PI)
        x=.48*sin(a)*radius
        z=.42+.33*cos(a)*radius
        return (x,.87-.19*x*x-.09*(z-.42)+.035*sin((v-.5)*PI),z),(.505,.735 if v<.5 else .76)
    model.patch(40,12,brim,lambda u,v:(sin((u-.5)*2*PI),10*sin((v-.5)*PI),cos((u-.5)*2*PI)))
    globe((0,1.2,.06),(.051,.034,.051),(.5,.79,0,0))
    # A smile follows the forward cheek surface.
    def smile(u,v):
        t=2*u-1
        a=-.24+.34*t*t+(v-.5)*.19*(1-t*t)
        x,length,z,y,depth=section(.5+.11*t)
        return (x,y+depth*sin(a)*(.57 if a<0 else 1),z+1.07*length*cos(a)+.008),(.505,.825)
    model.patch(28,2,smile,lambda u,v:(0,0,1))
    return model


def tube():
    model = Model()
    def surface(u,v):
        a,b = u*2*PI,v*2*PI
        return ((1.04+.29*cos(b))*sin(a), -.05+.30*sin(b),
                (1.17+.29*cos(b))*cos(a)),(u,v)
    model.patch(64,24,surface,lambda u,v:(sin(2*PI*u)*cos(2*PI*v),sin(2*PI*v),cos(2*PI*u)*cos(2*PI*v)))
    return model

if __name__=='__main__':
    OUTPUT.mkdir(parents=True,exist_ok=True)
    with tempfile.TemporaryDirectory() as tmp:
        for name,model in [('league-pin',pin()),('sunburst-ray',ray()),('rider-tube',tube())]:
            path=Path(tmp)/(name+'.png')
            subprocess.run(['inkscape',str(ROOT/(name+'.svg')),'-o',str(path)],check=True)
            model.write(name,path.read_bytes())
