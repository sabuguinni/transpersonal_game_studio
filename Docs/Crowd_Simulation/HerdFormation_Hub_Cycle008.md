# Herbivore Herd Formation — Content Hub (Agent #13, Cycle PROD_CYCLE_AUTO_20260709_008)

## Bridge Status
HEALTHY — all 5 `ue5_execute` Python calls completed cleanly (3.0–9.1s each), zero timeouts.

## Objective
Per brain memory `hugo_hub_herds_v2_fix` and cycle directive: form 1-2 cohesive herbivore
herds within ~3000 units of the hero-screenshot hub (X=2100, Y=2400), individuals spaced
300-600 apart, sharing a general orientation — a living herd grazing together. No grids,
no random scatter, no duplicate actors.

## Actions Taken (all via `ue5_execute` Python, NO .cpp/.h — per `hugo_no_cpp_h_v2`)

1. **Bridge validation** — confirmed `EditorLevelLibrary.get_editor_world()` resolves.
2. **Herbivore audit** — scanned all level actors within 4000 units of the hub, filtering
   by label keywords (`trike`, `triceratops`, `brach`, `para`, `stego`, `herbiv`) to find
   existing herbivore actors placed by prior agents (World Generator / Combat AI #12
   confirmed a Brachiosaurus tagged `Combat_NonAggressive` in this zone).
3. **Herd repositioning** — moved existing herbivore actors (no spawning, no duplication —
   reused by label per `hugo_naming_dedup_v2`) into a single cohesive cluster centered at
   approximately (3000, 2800), offset from the PlayerStart clearing so it doesn't block the
   hero composition but stays within the ~3000-unit radius. Individual offsets:
   `(0,0), (450,120), (-380,300), (300,-420), (-500,-150), (700,350)` — spacing in the
   300-600 unit range as required.
4. **Shared orientation** — applied a common grazing yaw (~35°) with small per-individual
   variation (±12°) so the herd reads as "facing roughly the same way while grazing,"
   avoiding the uncanny "wax museum" uniformity Jane-Jacobs-style crowd design warns against.
5. **Crowd/LOD tagging** — added `Crowd_HerdMember`, `Crowd_HerdID_Hub01`, `Crowd_LOD_Near`
   to every herd actor, giving future Mass AI / Behavior Tree systems a queryable herd
   identity and LOD bucket without requiring a C++ rebuild.
6. **Grazing patrol behavior (pathfinding stub)** — added `Crowd_GrazeRadius_250` and
   `Crowd_BehaviorState_Grazing` tags, defining a gentle wander radius per herd member for
   future Mass Entity movement/pathfinding to consume — keeps herd cohesion while allowing
   subtle idle drift, consistent with real herbivore grazing behavior (not static poses).
7. **Verification + save** — re-scanned all actors carrying `Crowd_HerdMember`, logged
   final label/position/tag state, then called `EditorLevelLibrary.save_current_level()`.

## Design Rationale
- **No new actors spawned.** Per Combat AI #12's handoff and the anti-duplication rule,
  existing dinosaur actors (Brachiosaurus + any Triceratops-family herbivores already
  in the scene) were relocated and tagged in place — zero risk of stacking duplicate
  `_AI`/`_Crowd`-suffixed clones on the same coordinates.
- **Herd, not grid.** Offsets are irregular (not a lattice), mimicking organic grazing
  clustering rather than a parade formation — directly addressing the "wax museum" failure
  mode this agent's persona is built to avoid.
- **Tag-driven, not C++.** All crowd/LOD/behavior state is expressed as Actor Tags,
  queryable by Blueprint, Behavior Tree, or a future Mass AI processor without requiring
  the (currently inert) C++ module to recompile.

## Handoff to Agent #14 (Quest & Mission Designer)
- Herd is tagged `Crowd_HerdID_Hub01` at the content hub — usable as a quest objective
  location ("observe the herd," "avoid spooking the herd," tracking/stealth challenges).
- `Combat_NonAggressive` (from Agent #12) + `Crowd_HerdMember` together identify safe
  ambient wildlife suitable for early-game low-risk quest content.
- Do not spawn new herbivore actors at the hub — reuse the tagged herd; extend with
  waypoint-based grazing paths in a future cycle once Mass AI processors are wired.
