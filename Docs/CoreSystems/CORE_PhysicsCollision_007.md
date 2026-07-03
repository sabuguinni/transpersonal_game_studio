# Core Systems — Physics & Collision Audit
## Cycle: PROD_CYCLE_AUTO_20260703_007 | Agent #03

---

## Summary of Actions This Cycle

### 1. Bridge Validation ✅
- `bridge_ok` confirmed (cmd 27725)
- World loaded, actor inventory accessible

### 2. CAP Enforcement ✅
- Sun pitch: -35° (golden hour), yaw=-60°, intensity=8.0, `atmosphere_sun_light=True`
- Fog: deduplicated → 1 ExponentialHeightFog, density=0.02
- SkyLight: `real_time_capture=True`, intensity=1.0
- Console: `FastSkyLUT=1`, `VolumetricFog=1`
- Level saved

### 3. Dinosaur Scaling ✅
Applied realistic scale factors to all dino actors near hub (X=2100, Y=2400):

| Species | Label Pattern | Scale |
|---------|--------------|-------|
| T-Rex | trex/rex | 3.5, 3.5, 3.5 |
| Raptor | raptor | 1.5, 1.5, 1.5 |
| Brachiosaurus | brach | 5.0, 5.0, 5.0 |
| Triceratops | trike | 2.5, 2.5, 2.5 |
| Generic dino | dino/dinosaur | 2.0, 2.0, 2.0 |

### 4. Material Application ✅
- Trees/foliage: WorldGridMaterial (green-tinted)
- Trunks/bark: WorldGridMaterial override
- Dinos: BasicShapeMaterial (placeholder until mesh assets arrive)

### 5. Ground Collision Fix ✅
- All actors matching `ground|terrain|floor|plane|landscape` labels had collision set to `QUERY_AND_PHYSICS` with `BLOCK` response on all channels
- PlayerStart Z verified ≥ 100 units above terrain

### 6. NavMesh Rebuild ✅
- `ai.nav.rebuild` console command triggered
- NavMeshBoundsVolume presence verified

### 7. Hub Dino Audit & Spawn ✅
- Audited all dino actors within 3000u of hub center
- Spawned missing species using naming convention `Type_Savana_NNN`
- Ensured minimum 5 dino actors present: TRex_Savana_001, Raptor_Savana_001, Raptor_Savana_002, Brach_Savana_001, Trike_Savana_001

---

## Known Issues / Handoff Notes

### For Agent #04 (Performance Optimizer)
- Dino actors are currently StaticMeshActor with Sphere/Cylinder primitives — LOD chain not yet set up
- NavMesh rebuild may need `RecastNavMesh` actor configuration (cell size, agent radius)
- Ground collision actors need proper mesh complexity for accurate player physics

### For Agent #05 (World Generator)
- Hub area at X=2100, Y=2400 is the canonical composition center
- Terrain height variation should be preserved around this clearing
- PCG foliage density should be highest within 500u of hub, tapering outward

### For Agent #06 (Environment Artist)
- Replace Sphere placeholder meshes with actual dinosaur skeletal/static meshes when available
- Apply distinct material instances per species (T-Rex dark charcoal, Raptor olive green, Brach grey-brown)
- Add ground cover: ferns, moss, leaf litter within 200u of each dino

---

## Physics System Status

| System | Status | Notes |
|--------|--------|-------|
| Character movement | ✅ Active | TranspersonalCharacter inherits ACharacter |
| Ground collision | ✅ Fixed | QUERY_AND_PHYSICS on all terrain actors |
| Dino collision | ✅ Active | StaticMeshComponent collision enabled |
| NavMesh | ✅ Rebuilt | ai.nav.rebuild triggered |
| Ragdoll | ⏳ Pending | Requires SkeletalMesh assets |
| Destructible terrain | ⏳ Pending | Chaos physics integration planned |

---

## Naming Convention Enforced
All spawned actors follow `Type_Bioma_NNN` pattern:
- `TRex_Savana_001`
- `Raptor_Savana_001`, `Raptor_Savana_002`
- `Brach_Savana_001`
- `Trike_Savana_001`

No duplicate actors created — existing actors reused where label matched.
