# Crowd & Traffic Simulation Agent #13 — Cycle PROD_CYCLE_AUTO_20260712_001

## Objective
Form living herbivore herds at the hero-screenshot hub (X=2100, Y=2400), reusing existing
dinosaur actors, respecting the `hugo_herd_consistency_v1` rule (never re-tag/reposition
actors that already belong to a prior-cycle herd).

## Workflow Executed
1. **Audit**: Scanned all actors within 3500u of hub for pre-existing `Herd_*` tags.
   Result: none found this cycle — herds had not yet been formed, so new herd creation
   was authorized (not a re-tag/override of prior work).
2. **Candidate selection**: Herbivore actors identified via two signals:
   - Name heuristics (`trike`, `triceratops`, `brachio`, `stego`, `para`)
   - Combat-role tags applied by Agent #12: `CombatRole_DefensiveCharger` (Triceratops),
     `CombatRole_NonHostile` (Brachiosaurus)
3. **Herd formation**: Up to 2 herds created — `Herd_HubGrazing_01`, `Herd_HubGrazing_02`.
   - Members repositioned within ~300u of hub anchor points (still inside 3000u hub radius)
   - Individual spacing: 300–600u (grid-jitter placement, verified via nearest-neighbor
     distance calculation in the verification pass)
   - Shared orientation per herd (yaw 15° / 200°) with small per-individual variance
     (±12°) for an organic grazing appearance, not a rigid grid look
   - Tag applied additively: `HerdRole_Grazing` (existing AI/Combat tags preserved)
4. **Verification**: Re-scanned hub, grouped by `Herd_*` tag, computed nearest-neighbor
   distances per member to confirm the 300–600u spacing target was met. Logged to
   `/tmp/ue5_result_crowd13_verify.txt`.
5. **Crowd LOD tiering** (Mass-AI-style, performance-oriented): All herd members tagged
   by distance from PlayerStart:
   - `CrowdLOD_Near_FullAnim` (≤1500u)
   - `CrowdLOD_Mid_SimplifiedAnim` (≤3000u)
   - `CrowdLOD_Far_Impostor` (>3000u)
   Stale LOD tags stripped before reapplying to prevent duplicate tag buildup across cycles.

## Key Decisions & Rationale
- **No new actors spawned.** All herd members are existing dinosaur actors located by
  label/tag lookup, per the anti-duplication rule (`hugo_naming_dedup_v2`).
- **Additive tagging only.** Herd/LOD tags stack on top of Agent #11 (`AI_Behavior_*`)
  and Agent #12 (`CombatRole_*`, `FleeThreshold_*`) tags — nothing was overwritten.
- **Naming convention locked for future cycles**: `Herd_HubGrazing_NN`. Per
  `hugo_herd_consistency_v1`, any future cycle finding these tags MUST reuse this exact
  prefix rather than inventing a new scheme.
- **No C++ files touched.** This headless editor never recompiles new C++; all changes
  were applied live via `ue5_execute` Python (actor transform + tag mutation), per
  `hugo_no_cpp_h_v2`.

## For Next Agent (#14 — Quest & Mission Designer)
- Herds are tagged `Herd_HubGrazing_01` / `Herd_HubGrazing_02` with member role
  `HerdRole_Grazing` — usable as quest triggers (e.g., "observe the herd without
  disturbing it" or "a predator scatters the herd").
- `CrowdLOD_*` tags are available if quest logic needs to reason about visibility/detail
  tier of background creatures.
- Combat AI's `CoordSignal_RaptorPack` (from Agent #12) + herd positions could combine
  for a "predator ambushes herd" scripted narrative beat.

## Files Changed
- This file only (documentation). No .cpp/.h files created or modified.
