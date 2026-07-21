# T-Rex Behavior Audit — Agent #11 (NPC/Dinosaur Behavior)
Cycle: PROD_CYCLE_AUTO_20260721_001

## CRITICAL FINDING: No actual T-Rex mesh actors exist in MinPlayableMap

Investigated all 58 actors containing "TRex" in their label across the level (3332 total actors).
Result: **100% of "TRex_Savana_NNN" labeled actors are `NiagaraActor` (VFX proxies)** — there is
no `SkeletalMeshActor`, `StaticMeshActor`, or `Character`/`Pawn` with a T-Rex mesh anywhere in the
level. What previous cycles tagged as "T-Rex combat AI" (CombatState_Chase, AttackDamage_45,
RetreatHP_0.15, etc.) was attached to dust-burst/roar-distortion VFX emitters and AmbientSound
actors that sit near the hub, plus 4 `StaticMeshActor` "TRexPatrolMarker_Hub_00X" waypoints. There
is no visible dinosaur body driving any of this — this is a metadata skeleton with no geometry.

This means the milestone "every dinosaur inside the playable core stands correctly ON the terrain,
posed naturally" **cannot be satisfied for T-Rex yet**, because there is nothing to pose. This is a
dependency gap that traces back to #06 Environment Artist / #09 Character Artist: an actual
Tyrannosaurus SkeletalMesh needs to be imported and spawned before AI Behavior or Combat AI can
attach real logic to a real body.

## Verification of requested items

1. **`Source/TranspersonalGame/AI/Combat/DinosaurCombatAIController.cpp`** — file exists on GitHub
   but its content is the literal string `undefined` (9 bytes). It is not real C++ code; it is a
   placeholder/corrupted write from a previous cycle. Per the absolute rule `hugo_no_cpp_h_v2`
   (C++ is inert in this headless editor — it never recompiles), I did **not** attempt to fix or
   rewrite this .cpp file, since doing so would have zero effect on the live game regardless of
   content quality.

2. **`Source/TranspersonalGame/AI/TRexBehavior.cpp`** — per directive I was asked to create this
   file. I did **not**, because `hugo_no_cpp_h_v2` is an ABSOLUTE / MAX-importance global rule that
   explicitly overrides cycle-specific instructions asking for .cpp/.h writes. Instead, the
   requested behavior spec (patrol radius 5000, chase trigger 3000, attack range 300) has been
   applied as **live actor tags** in the running UE5 world (see below) — this is the only form of
   "behavior definition" that has any real effect in this headless, non-recompiling editor.

3. **`Source/TranspersonalGame/Core/Survival/SurvivalComponent.h`** — verified, exists, well-formed
   (8112 bytes, complete UCLASS with Health/Hunger/Thirst/Stamina/Fear/Temperature stats, drain
   rates, damage thresholds, and delegate events). This is a legitimate, complete C++ component
   from Agent #03 (Core Systems). No changes needed.

## Live world changes made this cycle (verified via ue5_execute)

- Audited 58 "TRex"-labeled actors; confirmed class breakdown is 100% `NiagaraActor` / `AmbientSound`
  / `StaticMeshActor` (patrol markers only) — zero mesh bodies.
- Applied/confirmed behavior tags on all 49 `TRex_Savana_NNN_..._RoarDistortion` NiagaraActor proxies
  and their patrol markers:
  - `Behavior_PatrolRadius_5000`
  - `Behavior_ChaseTrigger_3000`
  - `Behavior_AttackRange_300`
  - `CombatRole_ApexPredator`
- Confirmed 4 `TRexPatrolMarker_Hub_00X` StaticMeshActor waypoints exist at (6700,2100), (1700,7100),
  (-3300,2100), (1700,-2900), all at z=100 (hub-relative reference frame), consistent with a
  5000-unit patrol radius centered near the hub (2100,2400).
- Confirmed **0 STATIC-mobility issues** on any dinosaur-related actor (rule `hugo_mobility_rule_v1`
  respected — not that it applies here since none of these are Character/Pawn actors).
- Saved level once at the end of the cycle (`save_current_level`), after all verification and
  tagging was complete.

## Recommendation to #12 (Combat & Enemy AI Agent)

Do not build tactical combat logic assuming a T-Rex Pawn/Character exists — it does not, yet. The
tag scaffolding (patrol/chase/attack radii, damage, retreat HP threshold) is present and consistent,
ready to be consumed by a Behavior Tree/AIController the moment a real Tyrannosaurus SkeletalMesh
actor is spawned and grounded on the Savana terrain. Recommend looping back to #06/#09 to get an
actual T-Rex mesh into the world before further combat-AI tagging work, to avoid stacking more
metadata on actors with no geometry (duplicate-metadata anti-pattern already flagged by
`hugo_naming_dedup_v2`).

## Files
- Created: `Docs/AI/NPC_TRex_Behavior_Audit_PROD_CYCLE_AUTO_20260721_001.md` (this file)
- No .cpp/.h files created or modified (rule `hugo_no_cpp_h_v2` respected)
- Live UE5 world: tags applied to 53 actors (49 NiagaraActor proxies + 4 patrol markers), level saved
