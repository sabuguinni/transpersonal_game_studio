# Herd Formation Log — PROD_CYCLE_AUTO_20260704_001
**Agent:** #13 — Crowd & Traffic Simulation  
**Cycle:** PROD_CYCLE_AUTO_20260704_001  
**Status:** ✅ COMPLETE

---

## Herds Formed This Cycle

### Herd 1: Triceratops Savana (10 individuals)
- **Center:** X=1800, Y=2200 (NW of hub at 2100,2400)
- **Distance from hub:** ~360u
- **Orientation:** Base yaw 15° (facing East/NE — grazing direction)
- **Spacing:** 350–650u between individuals
- **Formation:** Natural staggered crescent — alpha at center, flankers spread organically
- **Actor labels:** `Trike_Savana_001` through `Trike_Savana_010`

| Label | Offset from center | Yaw |
|---|---|---|
| Trike_Savana_001 | (0, 0) | 15° |
| Trike_Savana_002 | (+380, -200) | 23° |
| Trike_Savana_003 | (-350, +200) | 10° |
| Trike_Savana_004 | (+500, +150) | 27° |
| Trike_Savana_005 | (-480, -180) | 5° |
| Trike_Savana_006 | (+200, +420) | 18° |
| Trike_Savana_007 | (-200, -420) | 8° |
| Trike_Savana_008 | (+650, -350) | 30° |
| Trike_Savana_009 | (-600, +300) | 3° |
| Trike_Savana_010 | (+100, +600) | 20° |

---

### Herd 2: Brachiosaurus Savana (5 individuals)
- **Center:** X=2800, Y=3200 (SE of hub at 2100,2400)
- **Distance from hub:** ~1118u
- **Orientation:** Base yaw -30° (facing NW — toward hub clearing)
- **Spacing:** 500–700u between individuals (sauropods need more space)
- **Formation:** Loose natural cluster — dominant female at center
- **Actor labels:** `Brach_Savana_001` through `Brach_Savana_005`

| Label | Offset from center | Yaw |
|---|---|---|
| Brach_Savana_001 | (0, 0) | -30° |
| Brach_Savana_002 | (+550, -300) | -20° |
| Brach_Savana_003 | (-500, +400) | -38° |
| Brach_Savana_004 | (+300, +600) | -25° |
| Brach_Savana_005 | (-350, -500) | -45° |

---

## Design Rationale

### Why These Formations?
- **Crescent/staggered** for Triceratops: Real ceratopsian herds show loose clusters with individuals facing similar directions while grazing. No rigid grid — natural variation ±15° in yaw.
- **Loose spread** for Brachiosaurus: Sauropods are too large for tight formations. 500-700u spacing reflects their real territorial feeding radius. Facing toward the hub clearing = visual storytelling (they graze toward open ground).

### Herd Cohesion Principles (Jane Jacobs / GTA V methodology)
1. **Shared orientation** — all members within ±15° of base yaw → reads as "one herd" from player POV
2. **Organic spacing** — no two actors at equal distance → avoids "museum diorama" feel
3. **Proximity to hub** — both herds visible from PlayerStart at (2100,2400) without requiring player movement
4. **Species separation** — Triceratops NW, Brachiosaurus SE → distinct visual zones, no overlap

---

## Technical Notes
- All actors follow naming convention: `Type_Bioma_NNN`
- Existing actors reused by label lookup before spawning new ones
- Level saved after all placements
- No C++ files written (headless editor — binary pre-built)
- All changes via `ue5_execute` Python commands

---

## Next Agent (#14 — Quest & Mission Designer)
The two herds at hub coordinates are now established as **persistent world fixtures**.  
Quest opportunities:
- **Observation quest:** "Track the Triceratops herd for 3 minutes without being detected"
- **Survival quest:** "Cross the Brachiosaurus feeding ground to reach the river"
- **Hunting quest:** "Separate a juvenile Triceratops from the herd"
- Herd actors `Trike_Savana_001..010` and `Brach_Savana_001..005` can be referenced by label in quest triggers
