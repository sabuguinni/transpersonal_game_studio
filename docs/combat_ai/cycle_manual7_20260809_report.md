# Combat & Enemy AI Agent (#12) — Cycle Report PROD_CYCLE_MANUAL7_20260809

## Objective
Complete a combat-semantics tagging pass on existing `Dino_*` / `TRexPatrolMarker_*` actors without moving, rotating, rescaling or deleting any of them (hard rule for this agent), and verify the gate constraints R30 (predator-prey pairs ≥5000uu) and R31 (≤1 creature within 3000uu of PlayerStart) remain satisfied.

## Audit findings (read-only, verified via `ue5_execute` python)
- **38** `Dino_*` actors total (previous cycle noted 37; recount confirms 38 due to a duplicate label discovered below).
- Species breakdown: Velociraptor ×2, Triceratops ×14, Ankylosaurus ×10, Parasaurolophus ×6, Pachycephalosaurus ×1, Tsintaosaurus ×2, Protoceratops ×1, Brachiosaurus ×1, TRex ×1.
- **4** `TRexPatrolMarker_Hub_00[1-4]` confirmed at (6700,2100,100), (1700,7100,100), (-3300,2100,320), (1700,-2900,100) — read only, untouched.
- PlayerStart_Hub_001 at (1200,1200,300.77) — untouched (infrastructure rule respected).
- R30 (predator-prey ≥5000uu): **0 violations** found and confirmed after changes.
- R31 (creatures within 3000uu of PlayerStart): **1** (`Dino_Triceratops_6` at 2026uu) — within the ≤1 limit, no action needed.

## Key discovery: duplicate label collision
Two distinct actors both carry the label `Dino_Ankylosaurus_1`:
1. `SkeletalMeshActor_33` at (-37023, -138.5, 1356.6) — static creature, already fully tagged with combat semantics (13 tags: `CombatRole_ArmoredDefender`, `Combat_Telegraph_0.7`, `Combat_Damage_35`, `Combat_Range_220`, `Combat_Stamina_20`, `Combat_Recover_3.0`, etc).
2. `BP_Creature_Ankylosaurus_C_0` at (7000, -6500, -621.5) — a **movable** creature (Blueprint pawn, part of the "pastor" mobile-creature system from earlier cycles), which had **zero tags**.

This duplicate label caused a false "missing combat tags" read on the first pass. Corrected:
- Reverted the static `SkeletalMeshActor_33` to its original clean 13-tag set (removed 5 duplicate/conflicting tags I had mistakenly added in a first pass — e.g. two conflicting `Combat_Telegraph` values). Verified by independent reread: tag list matches pre-existing pattern exactly, position unchanged (-37023.0, -138.5, 1356.6, byte-identical before/after).
- Tagged the movable `BP_Creature_Ankylosaurus_C_0` with the equivalent combat/territory tag set (`Species_Ankylosaurus`, `CombatRole_ArmoredDefender`, `AI_Role_ArmoredHerbivore_Defensive`, `ThreatAwareness_low`, `CombatAI_FleeHealthPct_40`, `Combat_Telegraph_0.7`, `Combat_Damage_35`, `Combat_Range_220`, `Combat_Stamina_20`, `Combat_Recover_3.0`, `Behavior_StandGround_TailSwing_OnCornered`, `Movable_PastorManaged`). Location/rotation verified byte-identical before/after (7000, -6500, -621.5, rot 0/0/0) — no movement occurred, consistent with the hard rule against relocating creatures.

## Verification performed (all via separate `ue5_execute` calls, per anti-hallucination rule)
1. Pre-check: enumerated all 3464 level actors, isolated 38 `Dino_*` + 4 `TRexPatrolMarker_*`.
2. Full pairwise distance audit for R30/R31 — 0 and 1 respectively, both within gate limits.
3. Discovered and diagnosed the duplicate-label collision via `get_path_name()` (unique per GLOBAL_MEMORY rule that `get_name()` is not unique across levels).
4. Corrective tagging pass with `actor.modify(True)` before `set_editor_property('tags', ...)` (required for the change to be marked dirty).
5. `save_dirty_packages(True, True)` → returned `True`.
6. Independent reread in a **separate** `ue5_execute` call (not the same call that wrote) confirmed both tag sets persisted correctly, and re-ran R30/R31 checks post-save: 0 violations, 1 within-3000 creature — no regression.

## What was NOT done (explicitly out of scope / respected hard limits)
- No `Dino_*` actor was moved, rotated, rescaled, or deleted.
- No C++ (.cpp/.h) files were created — this cycle's directive is strictly tag/zone/parameter work via `ue5_execute` python.
- No HUD, hunger, inventory, or death/respawn systems touched (belongs to #14/#3).
- No new `CombatZone_*` / `BehaviorTag_*` TextRenderActor duplicates created — per the consistency rule from prior cycles, this cycle worked purely on the `tags` array of existing actors, avoiding further accumulation of that pattern.
- Did not touch PlayerStart, Landscape, Terrain_Savana sublevel, viewport camera, or the player pawn.

## Files created/modified
- `docs/combat_ai/cycle_manual7_20260809_report.md` (this report — documentation only, no code).

## Handoff to #13 (Crowd & Traffic Simulation)
- The 38 `Dino_*` actors are fully tagged with combat/territory/threat semantics (species, combat role, damage, range, stamina, recovery, telegraph timing, flee thresholds).
- One creature (`BP_Creature_Ankylosaurus_C_0`) is **movable** (pawn-based, "pastor"-managed) and now carries the `Movable_PastorManaged` tag for downstream systems to distinguish it from the 37 static `SkeletalMeshActor`-based creatures — important for #13's mass-agent simulation to know which entities already have independent movement logic vs. which are candidates for crowd/traffic behavior.
- Gate constraints R30 (predator-prey spacing) and R31 (PlayerStart proximity) are both green; no further combat-positioning work is needed this cycle.
- Recommend #13 cross-reference the `Herd_*` tags (e.g. `Herd_HubGrazing_01`, `Herd_AnkyPatrol_01`) already present on Triceratops/Ankylosaurus actors when building crowd/herd movement groups, to avoid creating duplicate herd groupings.
