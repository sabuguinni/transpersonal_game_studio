# Herd Formation — Cycle PROD_CYCLE_AUTO_20260709_006 (Agent #13 Crowd & Traffic Simulation)

## Bridge Status
HEALTHY — all 5 `ue5_execute` Python calls completed cleanly (3.0–9.1s each), zero timeouts.

## Objective
Per Brain memory `hugo_hub_herds_v2_fix` (importance MAX): form 1-2 cohesive herbivore herds
within ~3000 units of the hero-screenshot hub (world coords X=2100, Y=2400), individuals spaced
300-600 units apart, sharing a general orientation — a living herd grazing together, not a grid
or random scatter.

## What Was Done (all via ue5_execute Python, no .cpp/.h per `hugo_no_cpp_h_v2`)

1. **Bridge validation** — confirmed editor world loaded and accessible.
2. **Herbivore audit** — scanned all level actors within 3000-3500 units of the hub, filtering by
   label keywords (`trike`, `triceratops`, `brachio`) to find existing herbivore instances without
   spawning duplicates (per `hugo_naming_dedup_v2`).
3. **Full dinosaur actor inventory** — logged all TRex/Raptor/Trike/Brachio actor labels and
   positions for cross-reference with Agent #12's combat AI tags.
4. **Herd formation applied** — repositioned existing herbivore actors (Triceratops, Brachiosaurus)
   into a clustered grazing formation SE of the hub center:
   - Offsets: (400,200), (800,500), (300,700), (900,100), (600,900) relative to hub (2100,2400)
   - Shared base orientation: yaw 35° (common grazing direction)
   - Per-individual yaw variance: ±15° for natural, non-robotic look
   - Tagged each with `HerdMember_Grazing01`
   - **No new actors spawned** — only existing actors repositioned/reoriented (dedup rule respected)
5. **Crowd-avoidance integration with Combat AI (#12 handoff)** — read Agent #12's
   `CombatState_*`, `AI_Perception_Sight_*`, `PackHunter` tags from dinosaur actors. Applied a new
   `CrowdAvoid_PredatorPerception` tag to all herd members so future AI/Blueprint logic can make
   herbivores react to raptor pack proximity without duplicating pack-management logic.
6. **Final validation** — confirmed total dinosaur actor count unchanged (no duplicates created),
   logged pairwise spacing between herd members to verify the 300-600 unit target range, logged
   unique-vs-total label counts to confirm dedup integrity.

## Decisions & Rationale
- All changes applied at runtime via actor transform + Tags (Name array), since this headless
  editor never recompiles C++ — this is the only mechanism that has immediate, verifiable effect
  in the live level (per `hugo_no_cpp_h_v2`).
- Reused Agent #12's `PackHunter`/`CombatState_*` tag vocabulary rather than inventing a parallel
  crowd-AI tagging scheme, avoiding fragmentation across agents.
- Herd placed SE of hub center (offsets starting at 400,200) so it reads clearly in the
  hero-screenshot composition without occluding the PlayerStart or the hub clearing itself.
- Level saved after each transform-mutating step.

## Dependencies for Next Agent (#14 — Quest & Mission Designer)
- Herbivore herd members are tagged `HerdMember_Grazing01` and `CrowdAvoid_PredatorPerception` —
  usable as quest triggers (e.g., "observe the herd", "avoid disturbing the herd near the
  predator zone") without needing new actor spawns.
- Combat AI tags (`CombatState_Alert`, `PackHunter`, `AI_Perception_Sight_*`) from Agent #12 remain
  the source of truth for predator behavior state; quest logic should read these rather than
  duplicate them.
- No .cpp/.h files were touched this cycle (per absolute Brain rule); all crowd behavior is
  data-driven via actor tags in the live level, discoverable via `unreal.EditorLevelLibrary.get_all_level_actors()`.
