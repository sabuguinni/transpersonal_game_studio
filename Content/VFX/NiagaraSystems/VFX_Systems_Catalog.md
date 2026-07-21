# VFX Systems Catalog — Transpersonal Game Studio
## Agent #17 VFX — Cycle 004 (DEGRADED MODE: Bridge DOWN, APIs DOWN)

---

## NIAGARA SYSTEM DEFINITIONS

All systems follow naming convention: NS_[Category]_[Effect]
All materials follow: M_VFX_[Effect]

---

### CATEGORY 1 — FIRE & COMBUSTION

#### NS_Fire_Campfire
- **Purpose**: Player campfire — central survival mechanic visual
- **Emitters**: 3 (Flames, Embers, Smoke)
- **Particle count**: ~800 active particles
- **LOD 0** (< 10m): Full flames + embers + smoke + heat distortion
- **LOD 1** (10-30m): Flames + smoke only, reduced emitter rate
- **LOD 2** (30-80m): Single billboard sprite, animated texture
- **LOD 3** (> 80m): Culled
- **Parameters exposed**:
  - FireIntensity (0.0-1.0): scales emission rate and flame height
  - WindDirection (FVector): affects smoke drift
  - FuelAmount (0.0-1.0): reduces intensity as fuel depletes
- **Collision**: Flames collide with terrain (depth buffer collision)
- **Light**: Spawns a PointLight component, radius 400cm, intensity 800 lm, flicker curve

#### NS_Fire_ForestFire
- **Purpose**: Spreading wildfire — environmental hazard
- **Emitters**: 4 (Large Flames, Smoke Column, Embers, Ash)
- **Particle count**: ~3000 active particles
- **LOD 0** (< 50m): Full system
- **LOD 1** (50-150m): Reduced emitters
- **LOD 2** (> 150m): Smoke billboard only
- **Parameters**: SpreadRadius, WindStrength, BurnIntensity

---

### CATEGORY 2 — WEATHER

#### NS_Weather_Rain_Light
- **Purpose**: Light drizzle — common weather state
- **Emitters**: 2 (Droplets, Ground Splash)
- **Particle count**: ~2000 active
- **Spawn**: Box emitter above camera, follows player
- **LOD**: 3 levels based on rain density
- **Parameters**: RainDensity, WindAngle, DropletSize

#### NS_Weather_Rain_Heavy
- **Purpose**: Tropical downpour — reduces visibility, affects movement
- **Emitters**: 4 (Heavy Droplets, Splash, Mist, Puddle Ripples)
- **Particle count**: ~8000 active
- **Parameters**: Intensity, WindForce, MistDensity

#### NS_Weather_Fog_Ground
- **Purpose**: Morning ground fog — atmospheric, low visibility
- **Emitters**: 1 (Volumetric fog sheets)
- **Particle count**: ~200 large billboard particles
- **Height**: 0-200cm above terrain
- **Parameters**: FogDensity, FogColor, FogHeight

#### NS_Weather_Ash_Fall
- **Purpose**: Volcanic ash fallout — triggered by distant eruption events
- **Emitters**: 2 (Ash particles, Cinder flakes)
- **Particle count**: ~1500 active
- **Parameters**: AshDensity, WindDirection, AshColor (gray to black)

---

### CATEGORY 3 — DINOSAUR VFX

#### NS_Dino_Footstep_Heavy
- **Purpose**: Large dinosaur footstep impact (T-Rex, Brachiosaurus)
- **Emitters**: 3 (Dust Cloud, Dirt Chunks, Ground Crack)
- **Trigger**: Spawned at foot bone location on AnimNotify
- **Particle count**: ~300 per footstep (burst)
- **LOD 0** (< 30m): Full dust + chunks + crack decal
- **LOD 1** (30-80m): Dust only
- **LOD 2** (> 80m): Culled
- **Surface variants**: Dirt, Mud, Rock, Grass, Sand
- **Parameters**: FootSize (scales dust radius), ImpactForce (scales particle velocity)

#### NS_Dino_Footstep_Light
- **Purpose**: Small dinosaur footstep (Raptor, Compsognathus)
- **Emitters**: 2 (Small Dust, Leaf Scatter)
- **Particle count**: ~80 per footstep (burst)
- **Parameters**: FootSize, SurfaceType

#### NS_Dino_Breath_Cold
- **Purpose**: Visible breath vapor in cold/dawn environments
- **Emitters**: 1 (Steam puff)
- **Trigger**: Spawned at mouth bone, synced with breathing animation
- **Particle count**: ~50 per breath
- **Parameters**: BreathIntensity, Temperature (fades above 20°C)

#### NS_Dino_Roar_Shockwave
- **Purpose**: Air distortion ring from T-Rex roar
- **Emitters**: 2 (Distortion ring, Dust kick)
- **Effect**: Radial distortion material on a plane mesh, expands outward
- **Trigger**: AnimNotify on roar animation peak
- **Radius**: 0 to 1500cm over 0.8 seconds
- **Parameters**: RoarIntensity, DistortionStrength

#### NS_Dino_Blood_Impact
- **Purpose**: Blood spray on hit — combat feedback
- **Emitters**: 3 (Blood spray, Blood drip, Blood decal projector)
- **Particle count**: ~150 per hit (burst)
- **LOD**: Culled beyond 40m
- **Parameters**: HitForce, BloodColor (species-specific), WoundSize

#### NS_Dino_WaterSplash
- **Purpose**: Dinosaur entering/exiting water bodies
- **Emitters**: 3 (Splash crown, Foam, Ripple rings)
- **Particle count**: ~500 burst
- **Parameters**: BodyMass (scales splash size), EntryVelocity

---

### CATEGORY 4 — PLAYER & COMBAT

#### NS_Player_Footstep_Dust
- **Purpose**: Player footstep dust on dry terrain
- **Emitters**: 1 (Small dust puff)
- **Particle count**: ~30 per step
- **Parameters**: SurfaceType, MovementSpeed

#### NS_Combat_SpearImpact
- **Purpose**: Spear hitting dinosaur or terrain
- **Emitters**: 3 (Impact spark, Dust/Blood, Debris)
- **Particle count**: ~200 burst
- **Parameters**: ImpactSurface (rock/flesh/wood), ImpactVelocity

#### NS_Combat_RockThrow
- **Purpose**: Thrown rock impact effect
- **Emitters**: 2 (Dust burst, Rock fragments)
- **Particle count**: ~100 burst

#### NS_Player_Crafting_Sparks
- **Purpose**: Flint-knapping sparks when crafting stone tools
- **Emitters**: 1 (Bright sparks)
- **Particle count**: ~50 burst per strike
- **Parameters**: StrikeForce

#### NS_Player_Cooking_Smoke
- **Purpose**: Smoke from cooking meat over campfire
- **Emitters**: 2 (Thin smoke, Steam)
- **Particle count**: ~100 active
- **Parameters**: CookProgress (0-1), MeatType

---

### CATEGORY 5 — ENVIRONMENT

#### NS_Env_WaterfallSpray
- **Purpose**: Waterfall base spray and mist
- **Emitters**: 3 (Splash, Mist cloud, Foam)
- **Particle count**: ~2000 active
- **Parameters**: WaterfallWidth, FlowRate, MistRadius

#### NS_Env_RiverRipple
- **Purpose**: Water surface disturbance from objects
- **Emitters**: 1 (Ripple rings)
- **Trigger**: On object entering water volume
- **Parameters**: ObjectRadius, RippleSpeed

#### NS_Env_Pollen
- **Purpose**: Floating pollen particles in forest clearings
- **Emitters**: 1 (Slow-drifting pollen specks)
- **Particle count**: ~300 active, very slow movement
- **Parameters**: PollenDensity, WindStrength

#### NS_Env_Insects
- **Purpose**: Ambient insect swarms near vegetation/water
- **Emitters**: 1 (Swarming insect sprites)
- **Particle count**: ~200 active, circular swarm behavior
- **Parameters**: SwarmRadius, SwarmDensity

#### NS_Env_LavaGlow
- **Purpose**: Distant volcanic lava glow particles
- **Emitters**: 2 (Lava sparks, Ash plume)
- **Particle count**: ~500 active
- **Visibility**: Only active when VolcanicActivity > 0.5
- **Parameters**: EruptionIntensity, LavaColor

#### NS_Env_GodRays
- **Purpose**: Volumetric light shafts through forest canopy
- **Implementation**: Material-based (not particle) — animated UVs on translucent plane mesh
- **Parameters**: LightAngle, Intensity, Color

---

## MATERIAL DEFINITIONS

### M_VFX_Fire_Core
- **Type**: Translucent, Additive blend
- **Textures**: FireNoise (Tiling), FireMask (Animated UV)
- **Parameters**: FlameColor, EmissiveIntensity, OpacityFalloff

### M_VFX_Smoke
- **Type**: Translucent, Alpha Composite
- **Textures**: SmokeSheet (flipbook 8x8)
- **Parameters**: SmokeColor, Density, FadeDistance

### M_VFX_Dust
- **Type**: Translucent, Alpha Composite
- **Textures**: DustParticle (flipbook 4x4)
- **Parameters**: DustColor, Opacity, LifetimeFade

### M_VFX_Blood
- **Type**: Translucent, Alpha Composite
- **Textures**: BloodSplat (atlas)
- **Parameters**: BloodColor, Saturation, Opacity

### M_VFX_WaterSplash
- **Type**: Translucent, Additive
- **Textures**: WaterDroplet, FoamTexture
- **Parameters**: WaterColor, Foam Intensity, Refraction

### M_VFX_Distortion
- **Type**: Translucent, Refraction
- **Textures**: DistortionNormal (animated)
- **Parameters**: DistortionStrength, Radius, FadeEdge

---

## PERFORMANCE BUDGET

| System | LOD0 Particles | LOD1 Particles | LOD2 | GPU Cost (ms) |
|--------|---------------|---------------|------|--------------|
| NS_Fire_Campfire | 800 | 300 | Billboard | 0.8 |
| NS_Weather_Rain_Heavy | 8000 | 3000 | Disabled | 2.1 |
| NS_Dino_Footstep_Heavy | 300 burst | 100 burst | Culled | 0.3 |
| NS_Dino_Roar_Shockwave | 200 | 50 | Culled | 0.4 |
| NS_Env_WaterfallSpray | 2000 | 800 | Billboard | 1.2 |
| **TOTAL BUDGET** | | | | **< 4ms GPU** |

---

## IMPLEMENTATION NOTES

### UE5 Niagara Setup
- All systems use GPU simulation for particle physics
- Collision uses Depth Buffer collision (not scene query) for performance
- LOD transitions use Niagara's built-in Scalability settings
- All systems registered in NiagaraEffectsManager (Blueprint class)

### Spawn Integration
- Footstep VFX: Triggered via AnimNotify_NiagaraEffect on character/dino skeletons
- Combat VFX: Triggered via GameplayAbility or HitResult callbacks
- Weather VFX: Managed by WeatherController Blueprint, follows player camera
- Ambient VFX: Placed as NiagaraActor in level, auto-LOD

### Blueprint Manager
- BP_VFX_Manager: Central manager spawning/pooling all VFX
- Exposes: SpawnEffect(SystemAsset, Location, Rotation, Parameters)
- Pool size: 50 pre-allocated NiagaraComponents per common effect type

---

*Generated by Agent #17 VFX — Cycle 004 — DEGRADED MODE (Bridge DOWN, APIs DOWN)*
*Status: Documentation consolidated. Awaiting bridge restoration for live UE5 implementation.*
