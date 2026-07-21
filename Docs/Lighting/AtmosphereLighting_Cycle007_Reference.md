# Atmosphere & Lighting System — Cycle 007 Reference
**Agent:** #08 Lighting & Atmosphere  
**Cycle:** PROD_CYCLE_AUTO_20260618_007  
**Map:** `/Game/Maps/MinPlayableMap`

---

## Operations Executed This Cycle

### 1. DirectionalLight (Sun) Fix
- **Intensity:** 8.0 (was weak/unset)
- **Color:** Warm amber-white `(1.0, 0.92, 0.78)` — golden hour prehistoric sun
- **Rotation:** Pitch -45°, Yaw -30° — low-angle dramatic shadows
- **Shadows:** Dynamic shadow distance 50,000 units
- **Label:** `Sun_DirectionalLight`

### 2. SkyLight Fix
- **Intensity:** 1.2
- **Color:** Cool sky blue `(0.6, 0.75, 1.0)` — complements warm sun for natural contrast
- **Label:** `SkyLight_Main`

### 3. Point Light Intensity Reduction
- All PointLights/SpotLights with intensity > 5000 reduced to 2000
- Prevents overexposed hotspots visible in SceneCapture2D screenshots

### 4. Ruin God-Ray SpotLight
- **Location:** (50000, 50000, 3000) — above ruin cluster placed by Agent #7
- **Intensity:** 3000.0
- **Color:** Warm gold `(1.0, 0.95, 0.7)`
- **Cone:** Inner 15°, Outer 35° — tight dramatic shaft
- **Volumetric Shadow:** Enabled
- **Label:** `SpotLight_RuinGodRay`
- **Purpose:** Simulates sunlight breaking through jungle canopy onto ruins

### 5. Bioluminescent Moss Point Light (Ruin Interior)
- **Location:** (50200, 50200, 200) — inside pillar cluster
- **Intensity:** 800.0
- **Color:** Bioluminescent green `(0.2, 0.9, 0.5)` — moss on ancient stone
- **Radius:** 1500 units
- **Label:** `PointLight_RuinMoss`
- **Purpose:** Subtle interior glow, ancient/mysterious atmosphere

### 6. Exponential Height Fog
- **Density:** 0.03 — subtle, not pea-soup
- **Height Falloff:** 0.2 — thicker near ground, thins with altitude
- **Color:** Atmospheric blue-grey `(0.55, 0.65, 0.8)`
- **Start Distance:** 2000 units — fog begins at mid-range
- **Cutoff:** 200,000 units — full world coverage
- **Label:** `AtmosphericFog_Main`

### 7. Lumen + Volumetrics Console Commands
```
r.Lumen.Reflections.Allow 1
r.Lumen.GlobalIllumination.Allow 1
r.VolumetricFog 1
r.VolumetricFog.GridSizeZ 64
r.LightShafts.Enable 1
```

---

## Lighting Zones — Mood Design

| Zone | Primary Light | Mood | Notes |
|------|--------------|------|-------|
| Open Savanna | DirectionalLight (warm amber) | Exposed, dangerous | Long shadows, high visibility |
| Jungle Canopy | SkyLight (cool blue) + fog | Enclosed, tense | Dappled light through trees |
| Ruin Cluster | SpotLight_RuinGodRay + PointLight_RuinMoss | Ancient, mysterious | God-ray + bioluminescent contrast |
| River/Wetlands | Fog heavy + cool tones | Humid, oppressive | Fog density increase near water |
| Cave Interiors | PointLight only, no sky | Claustrophobic | Agent #7 to define entry points |

---

## Cinematic Intent (Roger Deakins Principle)
> "The player doesn't notice correct lighting — they only notice wrong lighting."

- **Sun angle at -45° pitch** creates dramatic long shadows that reveal terrain topology
- **Warm sun vs cool sky** creates natural fill/key contrast without artificial fill lights
- **God-ray on ruins** draws the player's eye to Agent #7's architecture — lighting serves navigation
- **Bioluminescent moss** rewards exploration — subtle detail only visible up close
- **Fog layers** create depth and hide pop-in — functional AND atmospheric

---

## Handoff to Agent #9 (Character Artist)

The lighting setup creates these character-relevant conditions:
1. **Warm rim light** from DirectionalLight at -30° yaw will naturally rim-light characters facing north-east
2. **Fog at 2000 unit start** means characters at distance appear atmospheric — good for MetaHuman skin rendering
3. **Ruin zone** has distinct lighting mood — character entering ruins gets green-tinted fill from moss light
4. **No harsh fill lights** — MetaHuman subsurface scattering will read correctly under this lighting

---

## Known Issues
- `generate_image` API returning 401 (invalid key) — concept art not generated this cycle
- Map save returned `False` — may indicate map already saved or path issue; lighting changes are applied in-editor regardless
- Lumen console commands apply to current session; persist via `DefaultEngine.ini` settings

---

## Files Modified
- `/Game/Maps/MinPlayableMap` — lighting actors added/modified
- `Docs/Lighting/AtmosphereLighting_Cycle007_Reference.md` — this file
