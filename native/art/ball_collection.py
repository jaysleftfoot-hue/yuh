"""Build the 64 UV maps in career unlock order. Requires Pillow, NumPy and Inkscape."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import numpy as np
from PIL import Image, ImageColor, ImageFilter

HERE = Path(__file__).resolve().parent
OUT = HERE.parent / "assets/balls"
# MLB mark ID, shell color, accent, illustrated motif (100 x 100 design grid).
# Slash separates the body silhouette from its contrasting inset artwork.
DESIGNS = [
(109,'672238','46ced0','M9 28 35 16 50 24 68 16 90 30 77 64 62 82 49 63 34 85 19 61Z/M18 30 44 37 38 47 22 43ZM81 30 56 37 62 47 78 43ZM27 52 38 58 34 74ZM73 52 61 58 66 74Z'),
(144,'172b52','ff9156','M48 36 Q25 20 14 48 Q7 73 37 89 Q48 96 57 88 Q83 82 85 53 Q87 28 62 31Z/M45 34 Q50 8 80 12 Q73 32 50 35ZM44 41 Q58 54 49 80 Q65 56 48 39Z'),
(110,'30252b','ff882e','M10 71 Q37 51 39 32 Q49 12 69 26 L76 36 93 40 74 46 Q73 74 42 77 L20 87 33 72Z/M31 65 Q46 41 62 45 Q55 63 31 65ZM62 32h6v5h-6Z'),
(111,'134c44','fbdaac','M10 30H90V80H10ZM15 26V13H18V26ZM78 28V9H81V28Z/M18 39H82V44H18ZM20 51h12v11H20Zm22 0h12v11H42Zm23 0h12v11H65Z'),
(112,'173e73','80ae58','M49 89 Q49 67 33 62 Q15 56 13 31 Q36 32 47 50 Q41 30 53 11 Q70 35 58 52 Q72 34 90 38 Q87 65 61 68Z/M50 76 24 42 47 61 53 28 56 60 81 45 56 72Z'),
(145,'222734','c7d2db','M10 73V49L28 38V49L46 36V49L65 40L68 19H77L82 70H90V82H10Z/M18 57h10v7H18Zm18 0h10v7H36Zm18 0h10v7H54ZM60 26l-8-7 4-2-1-9 9 10-4 1Z'),
(113,'851d32','fbc56a','M42 9H58L61 21 71 24 81 19 90 33 81 42 82 53 91 61 83 76 70 73 61 79 58 91H42L39 79 29 75 18 80 10 65 19 56 18 44 9 37 17 22 30 26 40 21Z/M50 29A21 21 0 1 0 51 29ZM50 40A10 10 0 1 1 49 40Z'),
(114,'172e51','f16357','M13 58Q32 29 55 39Q71 48 76 27Q95 52 72 62Q54 70 39 59Q29 52 13 58ZM12 74Q33 60 50 69Q71 78 89 61Q73 90 48 81Q28 70 12 74Z/M15 31 48 18 40 28 62 22 32 40 37 30Z'),
(115,'493466','c4b6ee','M4 77 33 30 44 48 62 15 97 77Z/M21 58 33 36 42 51 35 47 31 52 29 47ZM48 49 62 23 77 49 65 41 60 47 57 39Z'),
(116,'152e47','ff8a43','M7 66 23 59 32 42 59 36 73 49 92 52 94 67 83 70A10 10 0 0 1 63 70H36A10 10 0 0 1 16 70L6 71Z/M33 55 39 46 51 43 50 53ZM55 43 61 42 70 51 55 52ZM8 45h18l-5 4H4Z'),
(117,'1c314e','ff9745','M18 81 30 54 24 48 39 43 Q60 14 84 13 Q82 39 60 59 L55 77 47 69 34 77 40 62Z/M61 29A8 8 0 1 0 62 29ZM15 88 25 67 31 75Z'),
(118,'163e77','ecca64','M10 32 32 48 49 19 68 48 90 32 78 77H23Z/M23 63H79L76 69H25ZM45 50 50 40 55 50 50 58Z'),
(108,'902338','f1cc78','M8 52Q14 26 60 26Q95 27 91 47Q86 70 39 72Q7 72 8 52ZM18 51Q19 61 47 56Q77 52 80 42Q80 34 56 38Q28 42 18 51Z/M69 18 73 7 76 18 86 21 76 24 73 33 70 24 61 22Z'),
(119,'144e88','a4d9ef','M8 78 18 58 29 63 42 33 55 54 65 45 92 78Z/M15 40v-8Q34 30 50 19L86 22 88 28 52 27 37 41 30 40 36 29Z'),
(146,'064760','fc7779','M6 45 23 49Q40 26 67 33L75 19 76 38 96 27 80 49 91 62 72 58Q48 73 22 57L7 64 15 52Z/M31 49Q45 38 64 40Q52 51 37 55ZM24 50h4v4h-4Z'),
(158,'193753','edb954','M31 17Q48 11 68 17Q84 48 68 81Q49 89 30 81Q14 49 31 17Z/M26 30H74V36H26ZM24 60H76V66H24ZM45 20 47 78H49V20Z'),
(142,'182f4b','eb7468','M8 62H92V69H8ZM15 77V29H22V77ZM76 77V29H83V77ZM19 33Q48 72 79 33V40Q49 77 19 40Z/M30 48v15h3V50Zm15 8v8h3v-7Zm18-5v13h3V49Z'),
(121,'163d75','ff8949','M49 38Q28 22 16 42Q5 61 27 82Q38 96 51 87Q68 96 81 75Q98 46 76 35Q61 27 50 39Z/M47 37Q45 19 56 11L61 15Q50 22 53 33ZM54 29Q64 11 84 17Q72 32 54 29Z'),
(147,'243349','d7e0e6','M17 65Q33 63 36 52L27 25 70 19 73 49Q75 58 89 58Q86 69 63 73L29 78Z/M35 28 40 27 48 53 44 54ZM49 26 53 25 61 51 57 51ZM62 24 66 24 73 48 69 50Z'),
(133,'185940','ffce4d','M12 60 18 39Q29 23 47 36Q69 22 78 38L85 44 92 44 87 53 83 51 84 64Q82 77 70 75L69 67Q77 69 76 57L69 53 64 73H54L53 57 35 58 32 74H22L20 52Z/M56 38Q67 33 70 41L64 54 58 49Z'),
(143,'8d223b','8cc1da','M15 74Q29 64 31 44Q32 27 46 24V17H56V25Q71 28 71 45Q74 66 86 73L84 78H16Z/M54 36 47 49 54 54 47 69 60 52 53 48ZM42 82H60Q54 95 45 88Z'),
(134,'26252b','f2bf44','M16 75 20 66H78L85 77 79 81H18ZM29 64 23 42 62 18 78 39 46 61Z/M36 69A10 10 0 1 0 37 69ZM80 22l4-10 2 10 9 1-8 4 4 8-9-6Z'),
(135,'50362a','ffd168','M20 81Q8 65 26 46Q29 62 38 60Q34 45 51 18Q49 40 66 46Q75 50 75 35Q94 61 77 80Z/M38 78Q30 66 47 51Q44 64 54 65L61 53Q72 68 61 79Z'),
(137,'65372e','ff934b','M8 72H92V78H8ZM24 83V21H32V83ZM67 83V20H75V83ZM7 59Q21 53 28 27Q50 70 71 26Q78 50 94 57V62Q81 63 71 39Q48 77 28 39Q20 61 7 65Z/M40 52v20h3V54Zm13 3v17h3V54Z'),
(136,'104c57','8dd0c8','M11 78Q32 74 31 56Q31 34 49 27Q69 19 77 34Q86 50 70 56Q56 61 58 46Q64 54 70 46Q75 37 65 36Q52 34 51 55Q48 84 11 82Z/M20 49Q17 38 26 32Q35 29 36 37Q28 34 27 42ZM57 70Q72 63 83 67L91 75Q73 70 55 81Z'),
(138,'85253a','f0b752','M9 70 36 57Q41 36 54 30L52 16 69 26 76 24 75 37 91 44 75 49Q68 70 43 70L24 84 31 71Z/M36 62Q49 44 61 45Q54 58 36 62ZM61 34 72 38 68 45 59 42Z'),
(139,'183b62','81d0e9','M10 61Q28 27 51 40Q74 55 79 23Q96 46 79 61Q62 77 43 62Q29 50 10 61Z/M18 76Q34 66 49 74Q65 85 81 70Q70 92 48 82Q30 73 18 76ZM22 29l3-14 5 12 12 3-12 4-4 10-4-11-9-3Z'),
(140,'233d79','e96a5a','M50 10 61 38 91 40 68 58 75 88 50 71 25 88 32 58 9 40 39 38Z/M50 20V55L20 43 44 44ZM50 55 68 79 60 56 80 44Z'),
(141,'164d76','89dfa8','M5 71Q31 14 48 31Q58 43 68 31L94 12Q77 55 59 59Q44 62 40 50Q28 38 5 71Z/M20 81Q38 55 48 61Q68 76 88 46Q73 84 51 75Q38 69 20 81Z'),
(120,'852d44','a8bfd9','M49 10 53 37 71 17 61 41 90 34 67 50 89 61 59 57 71 84 52 64 48 92 44 63 25 84 36 58 9 64 31 49 10 37 39 42 27 17 44 36Z/M49 37 53 46 63 50 53 55 49 64 45 54 36 50 45 46Z'),
# Four additional Tampa Bay collectibles.
(139,'103962','ffda62','M48 4 57 36 77 22 66 42 97 49 65 56 78 76 57 64 49 97 42 65 20 79 33 57 4 51 33 43 20 23 40 36Z/M49 16 52 45 81 49 53 53 49 84 46 54 17 51 45 46Z'),
(139,'442868','a9e262','M4 45Q24 51 43 31Q51 23 61 34Q74 44 94 38Q84 62 58 64Q51 67 55 73Q65 85 85 81Q65 94 49 78Q40 67 40 62Q18 65 4 45Z/M16 49Q30 53 42 41Q35 56 24 55ZM62 43Q73 50 83 46Q76 55 67 54Z'),
(139,'075662','66e4cc','M9 53Q22 33 47 32Q56 27 67 39L84 54Q73 56 60 52Q58 67 47 69Q31 68 18 58L6 61Z/M45 61Q37 57 32 48Q45 54 55 50Z M52 66Q52 83 81 77Q64 91 52 78Z'),
(139,'263d77','efae58','M7 69Q24 23 64 28Q81 31 92 51L93 56 8 76Z/M19 64 77 40 78 43 18 69ZM36 49 66 34 68 37 32 54ZM13 78 92 58 92 68 15 89Z')]

def raster(source, output):
    subprocess.run(['inkscape', str(source), '-w', '320', '-o', str(output)], check=True,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    with Image.open(output) as image:
        rgba = image.convert('RGBA')
        return rgba.crop(rgba.getbbox())


def enamel(shell, accent, design, classic):
    w, h = 768, 384
    u, v = np.meshgrid(np.arange(w)/w, np.arange(h)/h)
    a = 2*np.pi*u
    def rgb(color):
        return np.asarray(ImageColor.getrgb('#'+color))
    base, trim = rgb(shell), rgb(accent)
    if classic:
        band = np.abs(v-.5-.32*np.cos(2*a))
        fill = .07+.22*(band<.036)+.64*((band>.040)&(band<.047))
        fill += .09*(np.cos(a*36)>.96)
    else:
        # The wrap continues around the longitude seam and under both emblems.
        swells = np.sin(2*a + 5*v + 1.3*np.sin(a-4*v+design))
        pattern = [swells + 2*(v-.5), np.cos(4*a+8*v+swells),
                   np.sin(a*3+swells*2+8*v), swells+.45*np.cos(12*v-a)][design%4]
        fill = .06+.35*(pattern>.38)+.38*((pattern>.49)&(pattern<.57))
        if design==0:  # Inaugural purple and turquoise snakeskin
            base,trim=rgb('47266b'),rgb('32c5ae')
            scales=np.abs(np.sin(12*a+40*v)*np.sin(12*a-40*v))
            fill=.12+.35*(scales>.67)+.27*(scales>.83)
        elif design==10:  # Houston sunrise jersey bands
            base=np.select([v[...,None] < k for k in (.34,.46,.58,.70)],
                           [rgb(c) for c in ('172c48','b63528','ea5729','ffa63b')],default=rgb('f8d57b'))
            fill=.045+.03*np.sin(2*a)
        elif design==2:  # Camden Yards warehouse brick courses
            mortar=(np.cos(48*np.pi*v)>.95)|(np.cos(16*a+np.pi*(np.floor(v*24)%2))>.98)
            fill=.10+.29*mortar+.17*(swells>.7)
        elif design==3:  # Green Monster scoreboard panel seams
            fill=.05+.18*((np.cos(18*a)>.96)|(np.cos(42*v)>.97))+.16*(swells>.7)
        elif design==18:
            fill=.09+.76*(np.cos(26*a)>.93)
        elif design==15:  # Barrel hoops and staves
            fill=.08+.55*(np.abs(v-.5)>.27)+.10*(np.cos(14*a)>.94)
        elif design==8:
            ridge=np.abs(np.sin(3*a))+.28*np.abs(np.sin(7*a))
            fill=.08+.27*(v>.35+.20*ridge)+.25*(v>.52+.20*ridge)
        elif design in (14,23,26):
            wave=np.cos(37*v+2.6*np.sin(3*a)+.65*np.sin(7*a))
            fill=.09+.25*swells**2+.40*np.maximum(0,wave)**12
    sheen = .96+.04*np.sin(2*a+12*v)+.012*np.cos(a*37)*np.sin(v*329)
    color = ((1-fill[...,None])*base + fill[...,None]*trim)*sheen[...,None]
    return Image.fromarray(np.uint8(color.clip(0,255))).convert('RGBA')


def place(image, emblem, width, height, centers=(0, 384, 768), latitude=.5):
    emblem=emblem.copy()
    emblem.thumbnail((width,height),Image.Resampling.LANCZOS)
    # Paint across the UV seam as well as the facing meridian.
    for x in centers:
        image.alpha_composite(emblem,(x-emblem.width//2,int(image.height*latitude)-emblem.height//2))


def outlined(mark):
    mark=mark.copy()
    mark.thumbnail((320,320),Image.Resampling.LANCZOS)
    pad=Image.new('RGBA',(mark.width+16,mark.height+16))
    pad.alpha_composite(mark,(8,8))
    key=Image.new('RGBA',pad.size,'#fff0cf')
    key.putalpha(pad.getchannel('A').filter(ImageFilter.MaxFilter(11)))
    key.alpha_composite(pad)
    return key


def illustrate(ball, panel):
    """Project the illustration around the front, feathering its edges into enamel."""
    w,h=ball.size
    lon,lat=np.meshgrid((np.arange(w)/w-.5)*2*np.pi, (np.arange(h)/h-.5)*np.pi)
    # Spread the scene across the visible hemisphere, leaving the central subject
    # undistorted at the equator. Wrap its outer brushwork past the silhouette.
    px=np.clip(.5+.49*np.sin(lon*.80),0,1)
    py=np.clip(.5+.49*np.sin(lat),0,1)
    pixels=np.array(panel.convert('RGB').resize((768,512),Image.Resampling.LANCZOS))
    paint=pixels[(py*511).astype(int),(px*767).astype(int)]
    fade=np.clip((2.03-np.abs(lon))/.43,0,1)
    fade=fade*fade*(3-2*fade)
    front=Image.fromarray(paint).convert('RGBA')
    front.putalpha(Image.fromarray(np.uint8(255*fade)))
    return Image.alpha_composite(ball,front)


def panel_from(atlas_path,index,cols,rows):
    with Image.open(atlas_path) as atlas:
        w,h=atlas.size
        x,y=index%cols,index//cols
        return atlas.crop((x*w//cols+2,y*h//rows+2,(x+1)*w//cols-2,(y+1)*h//rows-2))


def build():
    OUT.mkdir(parents=True,exist_ok=True)
    with TemporaryDirectory() as tmp:
        tmp=Path(tmp)
        for i,(club,shell,accent,paths) in enumerate(DESIGNS):
            word=raster(HERE/'team-marks'/f'{club}-wordmark.svg',tmp/'word.png')
            if i<30:
                primary=Image.open(HERE/'team-marks'/f'{club}-primary.png').convert('RGBA')
                primary=primary.crop(primary.getbbox())
                cap=raster(HERE/'team-marks'/f'{club}.svg',tmp/'cap.png')
                ball=enamel(shell,accent,i,True)
                place(ball,outlined(primary),155,152,(384,))
                place(ball,outlined(cap),150,145,(0,768))
                place(ball,outlined(word),132,80,(192,576))
                ball.convert('RGB').save(OUT/f'{i:02}.png')
            outline,inset=paths.split('/')
            svg=f'''<svg xmlns="http://www.w3.org/2000/svg" width="100" height="100">
            <path d="{outline}" fill="#{accent}" stroke="#fff0cd" stroke-width="6" stroke-linejoin="round" fill-rule="evenodd"/>
            <path d="{outline}" fill="#{accent}" stroke="#{shell}" stroke-width="2.5" stroke-linejoin="round" fill-rule="evenodd"/>
            <path d="{inset}" fill="#{shell}" fill-rule="evenodd"/></svg>'''
            path=tmp/'patch.svg'
            path.write_text(svg)
            emblem=raster(path,tmp/'patch.png')
            if i<30:
                panel=panel_from(HERE/'murals'/f'{i//6}.jpg',i%6,3,2)
            else:
                panel=panel_from(HERE/'rays-panoramas.jpg',i-30,2,2)
            ball=illustrate(enamel(shell,accent,i,False),panel)
            place(ball,emblem,156,155,(0,768))
            place(ball,outlined(word),104,50,(0,768),.80)
            ball.convert('RGB').save(OUT/f'{i+30:02}.png')


if __name__=='__main__':
    build()
