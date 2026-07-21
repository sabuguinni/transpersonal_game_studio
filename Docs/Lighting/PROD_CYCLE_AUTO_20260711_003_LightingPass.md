# Lighting & Atmosphere Pass — Cycle PROD_CYCLE_AUTO_20260711_003
**Agent #08 — Lighting & Atmosphere**

## Bridge Status
UP throughout. 2x `ue5_execute` (command_id 31620, 31621), both completed cleanly (~3s each), no timeouts.

## Real changes made to the live MinPlayableMap

### 1. Enforced singular daylight lighting rig (per hugo_hub_lighting_v2_fix)
- **DirectionalLight** — deduplicated to exactly ONE actor (`Sun_Main_001` if none existed, otherwise kept first found and destroyed extras). Set to:
  - Intensity: **15000 lux** (within the mandated 10000–75000 daylight band)
  - Rotation: pitch **-45°**, yaw 15°
  - Color: warm white (1.0, 0.93, 0.82)
  - `atmosphere_sun_light = True`
  - Shadow casting enabled, dynamic shadow distance 20000
- **SkyAtmosphere** — deduplicated to exactly ONE actor (`SkyAtmosphere_Main_001` if newly created).
- **SkyLight** — deduplicated to exactly ONE actor (`SkyLight_Main_001`), `real_time_capture = True`, intensity 1.5.
- **ExponentialHeightFog** — deduplicated to exactly ONE actor. Configured for subtle daytime atmospheric depth (not night fog):
  - Fog density: 0.008 (light, does not obscure daylight read)
  - Inscattering color: cool pale blue-white (0.75, 0.82, 0.9)
  - Volumetric fog: enabled, scattering distribution 0.6, directional inscattering exponent 3.0

### 2. Landmark god-ray shaft over Architecture Agent's (#07) new Cretaceous ruin cluster
- Spawned `RuinGodRay_CretaceousRuin_001` — a SpotLight at (50000, 50000, 600), pitched down -70°, warm gold color (1.0, 0.95, 0.75), intensity 80000, narrow cone (outer 22° / inner 8°), attenuation radius 3000, volumetric scattering intensity 4.0, `cast_volumetric_shadow = True`.
- Purpose: turns the ruin vignette into a visually discoverable landmark with a dramatic shaft of light breaking through canopy/fog, per Roger Deakins "light signifies intent" principle — draws the player's eye toward the ruin from a distance.
- Dedup-checked by label prefix before spawn (no duplicate created).

### 3. Hub clearing fill light (per hugo_hub_quality_v2_fix — the (2100,2400) hero-screenshot clearing)
- Spawned `HubFillLight_Hub_001` — a soft non-shadow-casting PointLight at (2100, 2400, 700), warm near-white (1.0, 0.97, 0.9), intensity 5000, attenuation 4000.
- Purpose: fills shadow gaps under dense canopy so dinosaurs and vegetation in the hero composition remain readable in bright daylight, without competing with or duplicating the main sun.

### 4. Verification pass (command 31621)
Logged actor counts for DirectionalLight/SkyAtmosphere/SkyLight/Fog (all confirmed singular), logged sun intensity/rotation, fog density/volumetric flag, and confirmed presence of both the god-ray and hub-fill actors by label. Level save invoked.

## Blocked / Known Infra Issues (reported honestly)
- Both `generate_image` calls (Cretaceous clearing establishing shot + ruin god-ray concept art) **succeeded at the GPT Image 1 API level** but **failed Supabase Storage upload** with `HTTP 403 Invalid Compact JWS` — same recurring Supabase JWT auth fault reported by #06 and #07 in this and prior cycles. Prompts preserved below for retry once auth is fixed.

### Preserved prompts for retry
1. *"Cinematic wide shot of a Cretaceous prehistoric forest clearing at midday, dense green ferns and tall conifer trees, bright warm directional sunlight at 45 degree angle casting long soft shadows, visible dinosaurs (a Triceratops grazing and a distant Brachiosaurus) standing in dappled light, atmospheric haze in the background, volumetric light shafts filtering through canopy gaps, realistic PBR game rendering style, Unreal Engine 5 Lumen global illumination, National Geographic documentary photography aesthetic, no fantasy or mystical elements"*
2. *"Dramatic god-ray light shaft breaking through mist onto ancient tilted stone ruin pillars in a dense prehistoric forest clearing, warm late-morning sunlight, volumetric fog, moss-covered fallen stone blocks scattered on the ground, dense ferns and prehistoric vegetation surrounding the ruin, cinematic Roger Deakins style lighting composition, realistic Unreal Engine 5 Lumen rendering, documentary-realistic tone, no mystical or spiritual imagery"*

## Technical decisions & justification
- Chose SpotLight (not additional DirectionalLight/Rect) for the ruin god-ray to keep exactly ONE DirectionalLight in the scene per the hard rule, while still achieving a directional shaft-of-light effect through narrow cone + volumetric scattering.
- Kept fog density low (0.008) specifically to avoid fighting the mandated bright-daylight read at the hub — heavier fog was rejected as it risked visually darkening the (2100,2400) hero composition.
- All dedup checks performed via `isinstance()` class checks and label-prefix search before any spawn, per hugo_naming_dedup_v2.

## Next agent focus (#09 — Character Artist)
- The lighting rig is now stable and singular across the map: 1 sun (15000 lux, warm, -45° pitch), 1 SkyAtmosphere, 1 SkyLight (real-time capture), 1 light daytime fog, plus 2 accent lights (ruin god-ray, hub fill).
- MetaHuman/NPC characters created this cycle will render correctly under this daylight rig — no additional lighting setup needed for skin-tone/PBR readability.
- Recommend #09 place player-visible NPCs within the (2100,2400) hub clearing to benefit from the new fill light for the hero screenshot composition.
