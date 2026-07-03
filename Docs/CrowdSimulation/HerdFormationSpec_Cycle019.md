# Herd Formation Specification — Cycle 019
## Agent #13 — Crowd & Traffic Simulation

### Overview
Two cohesive herbivore herds placed within 3000u of the hero-screenshot hub (X=2100, Y=2400).
All individuals share a general grazing orientation with natural ±20° variation.

---

## Herd 1 — Triceratops (Trike_Savana_001 to 008)
- **Center:** X=1800, Y=2200 (NW of hub, ~360u)
- **Facing:** 35° NE (grazing direction)
- **Count:** 8 individuals
- **Spacing:** 300–550u between individuals
- **Formation:** Natural cluster — lead female at center, flankers, rear guard, front scout
- **Label pattern:** `Trike_Savana_NNN`

| Label | Offset from Center | Role |
|-------|-------------------|------|
| Trike_Savana_001 | (0, 0) | Lead female |
| Trike_Savana_002 | (+380, -120) | Right flank |
| Trike_Savana_003 | (-300, +200) | Left rear |
| Trike_Savana_004 | (+150, +350) | Rear right |
| Trike_Savana_005 | (-200, -280) | Front left |
| Trike_Savana_006 | (+500, +200) | Far right |
| Trike_Savana_007 | (-450, +50) | Far left |
| Trike_Savana_008 | (+200, -400) | Front scout |

---

## Herd 2 — Brachiosaurus (Brach_Savana_001 to 004)
- **Center:** X=2600, Y=2900 (SE of hub, ~620u)
- **Facing:** -20° SW (toward water source)
- **Count:** 4 individuals
- **Spacing:** 500–700u (giants need more space)
- **Formation:** Loose group — wide spacing, slow movement implied
- **Label pattern:** `Brach_Savana_NNN`

---

## Stragglers — Stegosaurus (Stego_Savana_001 to 003)
- **Purpose:** Bridge the two herds visually, add depth to composition
- **Positions:** Between herds at (2050,2550), (2300,2350), (2150,2700)
- **Facing:** 5–25° (casual grazing)

---

## Naming Convention
All crowd actors follow: `Type_Bioma_NNN`
- Type: Species name (Trike, Brach, Stego, TRex, Raptor)
- Bioma: Savana, Floresta, Rio, Planicie
- NNN: 3-digit zero-padded index

## CAP Compliance
- Sun pitch: ≤-30° (enforced to -45° if above threshold)
- Fog: Single ExponentialHeightFog actor (duplicates removed)
- FastSkyLUT: r.SkyAtmosphere.FastSkyLUT 1
- SkyLight: real_time_capture=True

## Next Steps for Agent #14 (Quest & Mission Designer)
- Triceratops herd at (1800, 2200) can serve as "Observe the Herd" quest objective
- Brachiosaurus group at (2600, 2900) ideal for "Track the Giants" mission
- Stego stragglers at hub perimeter = ambient wildlife for immersion
- All actors labeled and positioned — quest triggers can reference by label
