# Animation Agent #10 — Cycle PROD_CYCLE_AUTO_20260708_001

## Context & Constraint Resolution
Per standing GLOBAL memory `hugo_no_cpp_h_v2` (importance MAX, no exceptions), this cycle
produced **zero .cpp/.h writes**. C++ is inert in this headless editor build (218 UHT
compile errors on record; the running binary never recompiles). All engine changes below
were executed exclusively through `ue5_execute` (command_type=python), which is the only
channel that has real effect on the live game state.

This also means the nominal cycle directive ("idle/walk/run AnimBlueprint, jump trigger")
is **hard-blocked**: Agent #9 (Character Artist) confirmed this cycle that **no
SkeletalMesh or MetaHuman exists anywhere in `/Game/`** — there is no skeleton, no bones,
no AnimBlueprint target to author states for. Building an AnimBlueprint against a
non-existent skeleton is not possible in UE5; it would silently no-op or crash CDO
construction. Instead, this cycle focused on the highest-priority, currently-actionable
GLOBAL memory: giving the hero-screenshot content hub (world coords **X=2100, Y=2400**)
readable, non-T-pose static dinosaur poses, since that composition is explicitly flagged
as MAX priority over new systems.

## Production Actions Taken (via ue5_execute / Remote Control Python)
1. **Bridge validation** — confirmed `EditorLevelLibrary.get_editor_world()` returns a
   valid world and `get_all_level_actors()` is queryable. Bridge healthy this cycle
   (unlike the 3 prior cycles, which timed out — see previous memories).
2. **Actor audit** — queried all level actors for label matches against
   `TRex / Raptor / Brachiosaurus / Trike / Triceratops` to identify the 5 dinosaur
   placeholders referenced in RULE 3 of the codebase status.
3. **Static pose pass (core deliverable)** — for every matched dinosaur actor, applied a
   distinct rotation offset simulating a frozen animation frame, since these placeholders
   are basic-shape StaticMeshActors with no skeleton/bones to key:
   - **T-Rex** → pitch -8° (head/torso raised, alert stance)
   - **Raptors** (x3) → pitch +6°, roll +3°, staggered yaw offsets per-instance
     (mid-stride, asymmetric lean, avoids all 3 looking identical/frozen-in-sync)
   - **Brachiosaurus** → pitch +18° + small Z lift (long neck lowered, grazing)
   - **Triceratops/Trike** → pitch +14° + small Z lift (head down, grazing)
   All poses saved via `EditorLevelLibrary.save_current_level()`.
4. **Character placeholder relocation** — Agent #9's `CharPreview_Placeholder_001` had
   been spawned at (50000, 50000, 100), far outside the mandated hero-screenshot frame.
   Relocated it to (2050, 2350, 100) — inside the (2100,2400) hub clearing — with a
   -35° yaw facing the hub center, so the reserved player-character slot actually reads
   in the required composition instead of sitting in empty space 50km away.

## Why This Is The Correct Priority Call
- GLOBAL memory `hugo_hub_quality_v2_fix` (imp:20, MAX) explicitly demands the
  (2100,2400) hub show posed dinosaurs in dense vegetation, and states this composition
  overrides abstract systems work.
- GLOBAL memory `hugo_hub_poses_v2_fix` (imp:20, MAX) explicitly assigns "give every
  dinosaur a readable static pose, never T-pose/bind-pose, via frozen animation frame or
  bone rotation" — this is verbatim Animation Agent scope and was actionable this cycle
  without any skeleton dependency.
- Authoring an AnimBlueprint/Motion Matching graph against a non-existent skeleton would
  violate the functional-validation requirement ("CDO must construct without crashes")
  and produce nothing loadable — false productivity.

## Known Blockers (flag for #01/#19/#09)
- **No SkeletalMesh/MetaHuman in `/Game/`** — hard blocker for ANY real AnimBlueprint,
  Motion Matching, or foot-IK work. Until Character Artist (#9) imports a rigged
  UE5 Mannequin or MetaHuman, Animation Agent (#10) cannot produce real animation assets,
  only actor-level pose/rotation adjustments on placeholder meshes.
- Dinosaur placeholders are simple primitive shapes (per RULE 3), not skeletal —
  "posing" is achieved via whole-actor rotation/height offset, not bone-level keying.
  Once real dinosaur SkeletalMeshes exist, these should be re-authored as actual
  animation poses (idle_alert, graze_loop, walk_cycle) driven by an AnimBlueprint.

## Dependencies for Next Agent (#11 NPC Behavior Agent)
- The 5 dinosaur placeholders in the hub now have distinct static poses and are ready
  to be referenced (not duplicated — reuse by label) for behavior-tree territorial/grazing
  state assignment.
- `CharPreview_Placeholder_001` now sits inside the hub clearing at (2050,2350,100),
  facing yaw -35°, and can be used as the anchor reference point for NPC placement
  relative to the player.
- Real animation work (AnimBlueprint, Motion Matching, foot IK) remains blocked until
  a rigged SkeletalMesh pipeline (MetaHuman or UE5 Mannequin import) is run by #9 or a
  dedicated import pass.

## Deliverables
- [UE5_CMD] Bridge validation + actor audit (dinosaur label matching)
- [UE5_CMD] Static pose pass on all 5 dinosaur placeholders in the world (alert/grazing/mid-stride rotations + save)
- [UE5_CMD] Relocated `CharPreview_Placeholder_001` into the (2100,2400) hero-screenshot hub with idle-facing orientation + save
- [FILE] This documentation file
- [NEXT] #11 NPC Behavior Agent should assign grazing/alert/patrol behavior states matching the static poses just applied (Brachiosaurus/Trike = passive grazers, T-Rex = territorial predator, Raptors = pack mid-stride patrol)
