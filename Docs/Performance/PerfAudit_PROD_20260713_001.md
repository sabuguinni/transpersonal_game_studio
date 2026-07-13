# Performance Optimizer #04 — Audit & Optimization Report
CYCLE: PROD_CYCLE_AUTO_20260713_001

## Bridge Status
OK — 5/5 `ue5_execute` calls completed without timeout/retry (IDs 32945–32949).
Zero `.cpp/.h` written — `hugo_no_cpp_h_v2` respected (47th consecutive cycle).

## Context: Input from Core Systems #03
This cycle, #03 enabled `QUERY_AND_PHYSICS` collision + `BlockAll` profile on all
TRex/Raptor/Trike/Brach placeholders so the player physically collides with them.
My mandate this cycle: validate the **performance cost** of that change and optimize
where possible without reverting the gameplay fix.

## Work Done (5 ue5_execute calls, all live on MinPlayableMap)

### 1. Baseline Actor Census
Counted total actors, tick-enabled actors, StaticMeshComponent count, and dinosaur
actor bounding-box overlap pairs (to catch physics/collision double-processing risk
from overlapping placeholders).

### 2. `stat unit` Enabled
Captured frame time breakdown (Game/Draw/GPU thread) as ongoing baseline reference
for future cycles to compare against.

### 3. Tick & Cull Distance Optimization Pass
- Disabled `bActorTickEnabled` on all static placeholders (Tree_*, Rock_*, TRex_*,
  Raptor_*, Trike_*, Brach_*) that don't run per-frame AI/animation logic yet.
  **Key finding:** collision response (the fix #03 just applied) is handled by the
  physics/collision subsystem, NOT by `Actor::Tick()`. Disabling tick on these
  actors is safe and does not undo #03's collision fix — the player still collides
  with them correctly, but the actors no longer cost per-frame CPU cycles.
- Set `CachedMaxDrawDistance = 15000.0` on all StaticMeshComponents belonging to
  these placeholders to reduce draw calls at range (cheap LOD-adjacent win until
  real LOD chains exist).

### 4. `stat fps` Enabled
Frame rate overlay active for visual confirmation in the live editor.

### 5. Memory/Asset Audit
Counted unique StaticMesh assets vs total instances (duplication/memory-cost
signal), total light components in the scene (Lumen/shadow cost driver), and
re-verified which actors still have tick enabled post-optimization (expected:
only `TranspersonalCharacter`, PlayerController, and manager/subsystem actors —
NOT static props/dinosaurs).

Raw results appended to `/tmp/ue5_result_performance_optimizer.txt` for #19.

## Technical Decisions & Justification
- **Did not revert or weaken #03's collision fix.** Physics collision detection is
  independent of Actor Tick; this is the correct optimization boundary per
  Ericson's principle — cut wasted per-frame CPU work, keep the gameplay-critical
  collision fully intact.
- **Cull distance instead of full LOD chain**: no new meshes/skeletons exist yet
  (per #03's note), so a cheap draw-distance cull is the highest-value, lowest-risk
  win available this cycle. Full LOD chains are a P8 follow-up once real meshes
  land.
- **No new actors spawned** (respects `hugo_naming_dedup_v2`) — pure optimization
  of existing placeholders only.
- **No camera changes** (respects `hugo_no_camera_v2`).

## Dependencies / Next Steps
- **#05 (Procedural World Generator):** any new terrain/biome geometry spawned
  should inherit the same tick-disabled + cull-distance pattern for static
  elements by default.
- **Future #04 cycle:** once real skeletal meshes/animations exist for dinosaurs,
  re-enable tick selectively for animated instances only, and replace flat cull
  distance with a proper 3-tier LOD chain.
- **#19:** cross-reference `/tmp/ue5_result_performance_optimizer.txt` during
  build integration; frame time baseline (`stat unit`/`stat fps`) now recorded
  live for regression comparison in future cycles.
