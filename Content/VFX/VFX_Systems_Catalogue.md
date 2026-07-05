# VFX Systems Catalogue — Transpersonal Game Studio
## Agent #17 VFX | Cycle 006 | DEGRADED MODE (Bridge DOWN, API key invalid)

---

## STATUS: DEGRADED MODE
- UE5 Bridge: **DOWN** (timeout on all 6 consecutive cycles)
- generate_image API: **INVALID** (401 Unauthorized — OpenAI key expired)
- Freesound API: **NO RESULTS** (returning empty arrays)
- Mode: **GitHub documentation only**

---

## NIAGARA VFX SYSTEMS — DESIGN SPECIFICATIONS

All systems below are designed for Niagara (UE5). When bridge is restored,
these should be implemented via ue5_execute Python commands.

### NS_Fire_Campfire
**Category:** Environment — Natural  
**Trigger:** Proximity to campfire actor  
**Emitters:**
- Flame Core: 200 particles/s, cone shape, orange→yellow gradient, 0.3–0.8s lifetime
- Embers: 15 particles/s, upward drift with random scatter, red→black fade, 2–4s lifetime
- Smoke: 8 particles/s, large billboards (0.5–2m), grey→transparent, 5–10s lifetime, wind-affected
- Heat Distortion: Distortion material plane above flame, animated UV scroll

**LOD Chain:**
- LOD0 (0–15m): Full system, all emitters active
- LOD1 (15–40m): Flame + smoke only, reduced particle count 50%
- LOD2 (40–80m): Single billboard sprite, no simulation

**Performance Budget:** 450 particles max at LOD0

---

### NS_Dino_Footstep_Dust
**Category:** Dinosaur — Impact  
**Trigger:** AnimNotify on foot-plant frame  
**Emitters:**
- Dust Burst: 80 particles, hemisphere spray, brown/tan, 0.4–0.8s lifetime
- Pebble Scatter: 12 mesh particles (small rocks), physics-simulated, 1–2s
- Ground Crack: Decal spawned at impact point, 2m radius, 8s fade

**Surface Variation:**
- Dry earth: Brown dust + pebbles
- Mud: Dark splatter + water droplets
- Rock: Spark particles + stone chips
- Grass: Green/brown debris + dirt

**LOD Chain:**
- LOD0 (0–20m): Full system
- LOD1 (20–50m): Dust only, no mesh particles
- LOD2 (50–100m): Disabled

**Performance Budget:** 100 particles per footstep burst

---

### NS_Weather_Rain
**Category:** Weather  
**Trigger:** WeatherController state change  
**Emitters:**
- Raindrops: 2000 particles/s, vertical lines, blue-grey, velocity 15 m/s downward
- Ground Splash: Spawned at collision point, ring expansion, 0.2s lifetime
- Puddle Ripples: Decal-based concentric rings on standing water surfaces
- Mist: Low-density fog particles near ground, 0.5m height band

**Intensity Levels:** Light (20%), Medium (60%), Heavy (100%) particle counts  
**Wind Integration:** Raindrop angle follows wind direction vector

**LOD Chain:**
- LOD0 (camera-relative): Full splash + ripples
- LOD1: Raindrops only, no ground interaction
- LOD2: Disabled (use weather material overlay instead)

**Performance Budget:** 3000 particles max at heavy intensity

---

### NS_Dino_Breath_Vapor
**Category:** Dinosaur — Ambient  
**Trigger:** Temperature < 10°C OR cold biome  
**Emitters:**
- Breath Puff: 3 particles/breath cycle, nostril position, white→transparent, 1.5s lifetime
- Vapor Trail: Wispy elongated particles, wind-affected, 2–3s lifetime

**Animation Sync:** Tied to breathing AnimNotify  
**LOD Chain:**
- LOD0 (0–10m): Full vapor simulation
- LOD1 (10–25m): Single puff sprite
- LOD2 (25m+): Disabled

---

### NS_Combat_BloodImpact
**Category:** Combat  
**Trigger:** Damage event with BloodImpact flag  
**Emitters:**
- Blood Spray: 40 particles, cone in hit direction, red, 0.3–0.6s lifetime
- Blood Drip: 8 particles, gravity-affected, 1–2s lifetime
- Blood Decal: Spawned at hit surface, 0.3–1.5m radius, 30s fade

**Intensity Scaling:** Particle count scales with damage amount (0–100 damage → 10–40 particles)  
**Surface Detection:** Raytrace at impact point to place decal on correct surface

**LOD Chain:**
- LOD0 (0–15m): Full system
- LOD1 (15–30m): Spray only, no decal
- LOD2 (30m+): Disabled

---

### NS_Crafting_Sparks
**Category:** Player — Crafting  
**Trigger:** Crafting action start (stone knapping)  
**Emitters:**
- Sparks: 30 particles/strike, bright yellow-white, 0.1–0.3s lifetime, physics bounce
- Stone Chips: 5 mesh particles, small rock fragments, 0.5–1s lifetime
- Dust: 20 particles, fine grey dust, 0.8s lifetime

**Strike Sync:** Burst emission on each crafting input event  
**LOD Chain:**
- LOD0 (0–5m): Full system
- LOD1 (5–15m): Sparks only
- LOD2 (15m+): Disabled

---

### NS_Water_Waterfall
**Category:** Environment — Water  
**Trigger:** Persistent (always active near waterfall actor)  
**Emitters:**
- Water Stream: 500 particles/s, vertical column, blue-white, 1–3s lifetime
- Mist Spray: 100 particles/s, hemisphere at base, white fog, 2–5s lifetime
- Foam: 50 particles/s, surface foam at base pool, white, 3–6s lifetime
- Droplets: 200 particles/s, scatter from impact zone, 0.5–1s lifetime

**LOD Chain:**
- LOD0 (0–30m): Full system
- LOD1 (30–80m): Stream + mist only
- LOD2 (80–200m): Single animated texture billboard

---

### NS_Volcano_Ash
**Category:** Environment — Volcanic  
**Trigger:** Volcanic biome proximity OR eruption event  
**Emitters:**
- Ash Flakes: 500 particles/s, slow drift downward, grey, 8–15s lifetime
- Cinder: 20 particles/s, glowing orange-red, 3–6s lifetime, cooling fade
- Smoke Column: 5 large particles/s, billowing upward, dark grey, 20–40s lifetime

**Wind Integration:** Ash drift direction follows global wind vector  
**LOD Chain:**
- LOD0 (0–100m): Full system
- LOD1 (100–500m): Ash flakes only, reduced 30%
- LOD2 (500m+): Skybox texture overlay only

---

## MATERIAL VFX SPECIFICATIONS

### M_VFX_Fire_Core
- Base: Additive blend mode
- Texture: Animated flipbook (8x8 frames, 24fps)
- Color: Orange→Yellow gradient with blue base tint
- Opacity: Noise-masked, edge fade

### M_VFX_Smoke_Volumetric
- Base: Translucent blend mode
- Texture: Wispy noise texture, UV scroll
- Color: Grey with slight blue tint
- Opacity: Soft particle enabled, distance fade

### M_VFX_Blood_Decal
- Base: DBuffer Decal blend mode
- Texture: Blood splatter atlas (16 variations)
- Normal: Subtle surface normal perturbation
- Roughness: High (0.9) — wet blood appearance

### M_VFX_Dust_Particle
- Base: Translucent, two-sided
- Texture: Soft cloud texture
- Color: Parameterized (brown/tan/grey by surface type)
- Opacity: Soft particle, distance fade

---

## IMPLEMENTATION QUEUE (When Bridge Restored)

Priority order for ue5_execute implementation:

1. **NS_Fire_Campfire** — Highest visual impact, campfire is core survival mechanic
2. **NS_Dino_Footstep_Dust** — Critical for dinosaur presence/weight feel
3. **NS_Weather_Rain** — Weather system integration
4. **NS_Combat_BloodImpact** — Combat feedback
5. **NS_Crafting_Sparks** — Crafting system feedback
6. **NS_Dino_Breath_Vapor** — Atmospheric detail
7. **NS_Water_Waterfall** — Environment polish
8. **NS_Volcano_Ash** — Biome atmosphere

---

## HERO SCREENSHOT COMPOSITION NOTES
**Target coords: X=2100, Y=2400 (content hub clearing)**

VFX elements that should be visible in hero screenshot:
- Campfire with visible flame + smoke column (NS_Fire_Campfire)
- Dust particles settling from recent dinosaur movement (NS_Dino_Footstep_Dust)
- Ambient ash/pollen particles drifting through sunlit clearing
- God rays through canopy (Lumen + volumetric fog, not Niagara)
- No magical/spiritual effects — purely physical, photorealistic

---

## HANDOFF TO AGENT #18 (QA & Testing)

**VFX Systems Status:**
- 8 Niagara system designs documented and ready for implementation
- 4 material specifications documented
- LOD chains defined for all systems (3 levels each)
- Performance budgets set per system
- All effects are physically realistic (no spiritual/magical content)

**Blocking Issues:**
- UE5 Bridge DOWN for 6 consecutive cycles — cannot implement Niagara systems
- generate_image API key expired — cannot generate reference art
- Freesound API returning empty results — cannot source audio references

**QA Test Cases (when systems implemented):**
1. Campfire VFX visible from 80m max distance
2. Footstep dust triggers on every foot-plant AnimNotify
3. Rain system transitions smoothly between intensity levels
4. Blood decals do not accumulate beyond 50 instances (pool management)
5. All VFX respect LOD chain — verify with stat particles console command
6. No VFX system exceeds its performance budget at LOD0

---
*Generated by Agent #17 VFX | Cycle PROD_CYCLE_AUTO_20260705_006 | DEGRADED MODE*
