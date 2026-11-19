# Painted arcade UI

Original generated artwork: arcade-frame.png (rock/turf/baseball outer frame), arcade-slate.png (chipped portrait/score/dialog slate), arcade-button.png (organic green action plaque). Generated for this project; no original game's textures are packaged. ui.hpp draws these textures with native text and real input hit areas. Slate uses nine regions with fixed output bevel widths so shallow score strips do not inherit giant borders.

Reference images retrieved directly from the official Zombie Bowl-o-Rama Steam listing (app 32160), then visually inspected for layout/materials:

- [Versus setup](https://shared.akamai.steamstatic.com/store_item_assets/steam/apps/32160/ss_1cdf66e954705079b12b46dc82389b3f7fc330e5.1920x1080.jpg): turf/rock outer framing, centered lane thumbnail, opposing portrait plaques, central mode selection and paired bottom actions.
- [Gameplay and inset machine](https://shared.akamai.steamstatic.com/store_item_assets/steam/apps/32160/ss_d12fb5e49f6b80f68a98808a5ebf7a5d396eb97f.1920x1080.jpg): shallow chipped slate scores, portrait badges and dimensional overlay over the game.
- User-supplied trophy screenshot: slate modal, bright arcade lettering and one organic action below the award.

The title poster is retained. The rejected full-width navy header, left marketing slogan, stadium/career dashboard and old stitched polygon panel implementation are removed. Setup, locker, scores, help, HUD, pause and match results share painted arcade materials. Baseball portraits and stadium art remain original/separately sourced as documented alongside their assets.

Ball locker previews use ui-ball.fs to map the existing ball skins onto shaded circular ball icons with finger holes; no rectangular texture swatches remain.

## Dugout equipment references

The lane views informed the floating pickup silhouette and the machine view informed the framed equipment display. The baseball case and medals are original game artwork.

- [Pickup on the sunset lane](https://shared.akamai.steamstatic.com/store_item_assets/steam/apps/32160/ss_1637c5101c9c2a943ac6027fcb829e45185b1e06.1920x1080.jpg?t=1688475037)
- [Pickup on the night lane](https://shared.akamai.steamstatic.com/store_item_assets/steam/apps/32160/ss_389a941d4bf864fd13d3e102385e671c5c106c0f.1920x1080.jpg?t=1688475037)
- [Power-up machine](https://shared.akamai.steamstatic.com/store_item_assets/steam/apps/32160/ss_d12fb5e49f6b80f68a98808a5ebf7a5d396eb97f.1920x1080.jpg?t=1688475037)
