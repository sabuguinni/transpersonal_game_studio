# Crowd & Traffic Simulation Agent #13 — Cycle PROD_CYCLE_AUTO_20260708_001

**Bridge status:** HEALTHY — all 5 `ue5_execute` calls completed successfully this cycle (first clean bridge session after 3 consecutive FAIL cycles on record: 20260706_002/004/005).

## Mandate
Form 1-2 cohesive herbivore herds within ~3000 units of hub coords **X=2100, Y=2400**, individuals spaced 300-600 apart, sharing a general grazing orientation. No grids, no random scatter. Reuse existing actors by label before spawning new ones.

## What I did (all via `ue5_execute` Python, zero .cpp/.h — C++ is inert in this headless editor per standing rule `hugo_no_cpp_h_v2`)

### 1. Actor discovery (read-only, no duplication)
Queried all level actors for existing herbivore labels (`Trike`, `Triceratops`, `Brach`, `Parasaur`, `Stego`, `Herbivore`) and their distance from the hub. This confirmed which actors already represent herbivores before touching anything, per the naming/dedup rule (`hugo_naming_dedup_v2`).

### 2. Herd clustering — reused existing actors
Took the existing Triceratops and Brachiosaurus placeholders (already in `MinPlayableMap` per infra baseline) and repositioned them around an anchor point ~400-500 units off the hub center:
- Anchor: `(2500, 2700)` approx, within the 3000-unit radius of `(2100, 2400)`
- 5 offset slots at 300-600 unit spacing: `(0,0)`, `(350,200)`, `(-300,350)`, `(400,-300)`, `(-450,-200)`
- Shared grazing orientation: base yaw ~35°, with ±20° per-individual variance so the herd reads as organic, not robotic-uniform
- Tagged each with `Herd_HubGrazing_NN` for future Mass AI / Behavior Tree pathing reference (per #12's handoff — combat AI reads `Combat_*` tags, crowd sim now exposes `Herd_*` tags on the same actors)

### 3. Herd completion (conditional spawn, only if needed)
Checked if fewer than 3 actors ended up tagged as herd members (since the baseline map has limited herbivore variety — mostly 1 Triceratops + 1 Brachiosaurus per RULE 3 infra notes). If so, duplicated the **existing static mesh** (not a new asset, not a new class) from the source herbivore actor onto 1-2 new `StaticMeshActor` instances, labeled `Trike_HubHerd_NNN` (follows `Type_Bioma_NNN` convention from `hugo_naming_dedup_v2`), placed at a second anchor point still within the 3000-unit hub radius, with matching grazing-yaw variance.

### 4. Verification + save
Final pass re-queried all `Herd_HubGrazing_*` tagged actors, printed their final position/yaw for confirmation, then called `unreal.EditorLevelLibrary.save_current_level()` to persist the herd formation into `MinPlayableMap`.

## Key decisions
- **No camera changes** — did not touch viewport per `hugo_no_camera_v2`.
- **No new C++** — all logic is live actor state (transform + tags), consistent with #11/#12's precedent that Blueprint/Python-level state is the only functional layer in this headless build.
- **Reuse over duplication** — checked existing labels before spawning; only added new `StaticMeshActor` instances when genuinely needed to reach a believable herd size (3+), reusing the existing mesh asset rather than importing/generating new geometry.
- **Grazing orientation logic** — shared base yaw with small per-individual variance mimics Jane Jacobs' "emergent pattern from individual behavior" principle: no two herd members face identically, but the group reads as cohesive at a glance.

## Handoff to #14 (Quest & Mission Designer)
The `Herd_HubGrazing_*` tags are now readable on herbivore actors near the content hub. These are safe to reference for:
- Quest objectives themed around observing/tracking a herd without disturbing it
- Environmental storytelling beats (herd flees if player triggers combat-tagged dinosaurs nearby, per #12's `Combat_*` tags on the same actor set)
- No narrative content authored here — pure spatial/behavioral state only, per scope boundary.

## Files created/modified
- `Docs/Crowd/Agent13_HerdFormation_Cycle_PROD_AUTO_20260708_001.md` (1 of 2 github writes used, zero .cpp/.h)

## Tool calls this cycle
5x `ue5_execute` (python): actor discovery → distance query → herd repositioning+tagging → conditional herd completion spawn → final verification+save. All returned `success: true`.
