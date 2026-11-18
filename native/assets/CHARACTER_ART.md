# Character art

baseball-portraits.png is an original AI-generated transparent, vector-style portrait atlas. It is raster artwork, not an editable SVG. Quadrants: Rays player, Cubs helmet batter, umpire, ball boy. Club insignia identify the depicted baseball outfits; characters are fictional. Rival appearances cycle by stadium and do not change AI difficulty. Player portrait uses the Rays cap.

The playable pin, mascot and inflatable are original, self-contained glTF models in models/. Each model includes its own UV coordinates, smooth normals and embedded paint. The pin uses an ivory lacquer finish with baseball stitching and a league crest. The ray has a spotted dorsal surface, ivory underside, sunburst cap, modeled eyes and a tapered tail. Its wing tips and tail flex in the vertex shader during play.

The editable texture drawings and mesh authoring script live in native/art/. To rebuild the three GLBs from the repository root, run `python3 native/art/build_models.py` with NumPy and Inkscape installed. The shipped game loads the GLBs directly.
