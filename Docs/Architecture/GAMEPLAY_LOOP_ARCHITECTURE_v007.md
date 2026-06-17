# Gameplay Loop Architecture — Engine Architect #02
## Cycle: PROD_CYCLE_AUTO_20260617_007

---

## SURVIVAL GAMEPLAY LOOP (Validated)

```
PlayerStart (0,0,200)
    ↓
[ORIENTATION] Player sees terrain, sky, basic environment
    ↓
[NEED: THIRST] Find WaterPool_River_001 (water marker, blue)
    ↓
[NEED: HUNGER] Find FoodZone_Berries_001 (food marker, green)
    ↓
[THREAT] Avoid DangerZone_Predator_001 — TRex positioned nearby
    ↓
[SHELTER] Return to Campfire_Shelter_001 (orange marker)
    ↓
[LOOP REPEATS with increasing difficulty]
```

---

## BIOME ZONE ARCHITECTURE

| Zone Label | Position (X,Y,Z) | Color Code | Gameplay Role |
|---|---|---|---|
| Arch_Savanna_Zone | (5000, 0, 200) | Yellow/Warm | Open terrain, Brachio grazing |
| Arch_Forest_Zone | (-5000, 0, 200) | Green | Dense cover, Raptors patrol |
| Arch_Swamp_Zone | (0, 5000, 100) | Blue/Dark | Water sources, ambush risk |
| Arch_Mountain_Zone | (0, -5000, 600) | Grey/Cold | High ground, rare resources |

---

## SURVIVAL MARKER SYSTEM

| Marker | Color | Position | Gameplay Signal |
|---|---|---|---|
| Campfire_Shelter_001 | Orange (warm) | (-1000, -1000, 150) | Safe rest point |
| WaterPool_River_001 | Blue | (1500, 2000, 80) | Thirst replenishment |
| DangerZone_Predator_001 | Red | (2000, 0, 100) | TRex territory — avoid |
| FoodZone_Berries_001 | Green | (-2000, 1500, 120) | Hunger replenishment |

---

## DINOSAUR PLACEMENT RULES (Architecture Law)

1. **TRex** → Always within 500 units of DangerZone_Predator_001
2. **Raptors** → Patrol Arch_Forest_Zone radius (4000-6000 units from center)
3. **Brachiosaurus** → Arch_Savanna_Zone, passive, non-aggressive
4. **No dinosaur** may be placed within 800 units of PlayerStart

---

## TECHNICAL CONSTRAINTS (Enforced)

- **Actor CAP**: 8000 max (current: checked each cycle)
- **Dino CAP**: 150 max (checked each cycle)
- **Label format**: `Type_Biome_NNN` — no concatenated suffixes
- **Map save**: `EditorLoadingAndSavingUtils.save_map()` after every spawn session

---

## AGENT HANDOFF DIRECTIVES

### → Agent #03 (Core Systems Programmer)
- Implement survival stats tick: hunger/thirst decay at 1 unit/30s
- Collision volumes around DangerZone_Predator_001 trigger fear response
- Water trigger at WaterPool_River_001 replenishes thirst

### → Agent #08 (Lighting & Atmosphere)
- Warm orange light at Campfire_Shelter_001 (radius 500, intensity 2000)
- Blue-tinted fog near Arch_Swamp_Zone
- Red ambient tint near DangerZone_Predator_001 (danger signal)

### → Agent #12 (Combat & Enemy AI)
- TRex patrol radius: 800 units around DangerZone_Predator_001
- Raptor pack: 3 units, patrol Arch_Forest_Zone perimeter
- Aggro range: TRex=1200, Raptor=600

---

## ARCHITECTURE DECISIONS THIS CYCLE

1. **Survival markers as PointLights**: Chosen for immediate visual feedback without requiring custom mesh assets. Color-coded by gameplay role.
2. **Biome zones as Arch_ prefixed actors**: Architecture-layer actors, not gameplay actors. Prefix prevents confusion with gameplay entities.
3. **TRex repositioned to DangerZone**: Spatial coherence — player learns "red = TRex = danger" through proximity.
4. **MAP_SAVED after every session**: Prevents loss of work between cycles.

---

*Engine Architect #02 — Cycle 007 — Architecture validated and enforced*
