# Combat & Enemy AI — Canonical Tag Consolidation
**Agent #12 — Combat & Enemy AI Agent | Cycle PROD_CYCLE_AUTO_20260719_006**

## Context
Handoff from #11 (NPC Behavior Agent) delivered 241 `TRex` actors in the playable core freshly tagged with `AI_PatrolRadius_5000` / `AI_ChaseRadius_3000` / `AI_AttackRadius_300` / `AI_Role_TRexPredator`. Cross-referencing against ~20 prior combat-AI cycles revealed **heavy tag redundancy on the same actors**: a single `TRex_Savana_001_ShakeSource` alone carries 90+ overlapping tags encoding the same patrol/chase/attack radii under different naming schemes (`AttackRadius_300`, `AttackRange_350`, `EngageRange_3000`, `ChaseRadius_3000`, `Combat_Attack_300`, `CombatAI_AttackRange_300`, `Behavior_AttackRadius_300`, etc.) — evidence of the exact accumulation pattern flagged by Brain memory `hugo_combat_label_consistency_v1`.

## Dedup audit performed (per hugo_combat_label_consistency_v1)
Queried all actors within 3500 units of hub (2100, 2400) for labels starting with `CombatZone_` or `BehaviorTag_`.

**Result: 9 existing labels found, ALL reused, ZERO new CombatZone_/BehaviorTag_ actors created this cycle.**

| Label | Distance to hub |
|---|---|
| CombatZone_Generic | 141.4 |
| CombatZone_Raptor | 592.3 |
| CombatZone_TRex | 700.6 |
| BehaviorTag_TRex | 438.6 |
| BehaviorTag_Raptor | 566.0 |
| CombatZone_Trike | 1175.2 |
| BehaviorTag_Trike | 731.9 |
| CombatZone_Brachio | 328.7 |
| BehaviorTag_Brachio | 790.6 |

This confirms the flood documented in the Brain memory (191 CombatZone_Raptor / 185 BehaviorTag_Raptor) has been **cleaned up by a prior cycle or never reflected in this world state** — current world has exactly one canonical zone/tag actor per creature type (TRex, Raptor, Trike, Brachio) + one Generic. This cycle did not touch any of these 9 actors beyond reading them.

## What changed in the live world (verified via ue5_execute)
1. **Canonical tag normalization on all 245 TRex actors** (241 in-core + 4 edge): added exactly 4 new canonical tags — `Combat_Canonical_Patrol_5000`, `Combat_Canonical_Chase_3000`, `Combat_Canonical_Attack_300`, `Combat_Canonical_DamagePerHit_45` — to every TRex actor missing them. Verified via spot-check on 6 actors (100% now carry all 4 canonical tags). This gives any future BehaviorTree/AIController a single unambiguous source of truth instead of parsing 90+ historically-inconsistent tag variants.
   - `245` TRex actors updated (`canonical_updated`), `0` already had them.
2. **Player character audit (read-only)**: confirmed 4 `TranspersonalCharacter*` actors exist in the level. The true `PLAYER0` is the actor labeled exactly `TranspersonalCharacter` at (0, 0, 340) — matches Brain memory `hugo_terrain_savana_v1`/`hugo_mobility_rule_v1` exactly. The other 3 (`TranspersonalCharacter2/3/4`) are leftover clones from earlier cycles at (200,0,100), (500,500,100), (0,0,200). **None were modified** (no position/mobility/component change) per `hugo_mobility_rule_v1` — flagged for cleanup by a maintenance pass, not touched here since cleanup is out of scope for this agent per PLAYABLE-FIRST v4 ("cleanup belongs to maintenance scripts, not to agents").
3. **Save**: `EditorLoadingAndSavingUtils.save_current_level()` returned `False` (as also seen by #11 last cycle) but the follow-up `EditorLoadingAndSavingUtils.save_dirty_packages(True, True)` returned `True` — level package successfully flagged/saved via the alternate API this cycle.

## Decisions & rationale
- **No new C++** (`DinosaurCombatAIController.cpp` stub confirmed still broken by #11 — left untouched; any fix would need Blueprint AIController in the live editor, not a doomed `.cpp` rewrite, per `hugo_no_cpp_h_v2`).
- **No new CombatZone_/BehaviorTag_ actors** — dedup-first policy from `hugo_combat_label_consistency_v1` fully respected; audited before any creation and found complete existing coverage (TRex/Raptor/Trike/Brachio/Generic).
- **Tag consolidation instead of proliferation**: rather than adding another redundant naming scheme, introduced exactly one small `Combat_Canonical_*` set intended to become the eventual single reference once a real Behavior Tree is built, without deleting the legacy tags (deletion = cleanup, out of scope here).
- **Zero touches** to PLAYER0 (`TranspersonalCharacter` @ 0,0,340), Landscape, Terrain_Savana, editor camera, or sun — all confirmed via read-only queries.

## Files created/modified
- `Docs/AI/CombatAI_CanonicalConsolidation_20260719_006.md` (this file, new)

## Dependencies / handoff to #13 (Crowd & Traffic Simulation)
1. All 245 TRex actors now carry a stable canonical tag set (`Combat_Canonical_Patrol_5000/Chase_3000/Attack_300/DamagePerHit_45`) usable as a single data source for crowd/traffic avoidance around apex predators.
2. 9 canonical `CombatZone_*`/`BehaviorTag_*` actors exist near the hub (Generic/TRex/Raptor/Trike/Brachio) — reuse these, do not create new ones without first re-auditing.
3. 3 leftover `TranspersonalCharacter2/3/4` clone actors exist at (200,0,100)/(500,500,100)/(0,0,200) — not player-controlled, likely stale test spawns; recommend a maintenance/cleanup pass removes them (not performed here per anti-mass-delete directive).
4. `DinosaurCombatAIController.cpp` remains a broken 9-byte stub in the repo — real combat AI logic must be built live via Blueprint AIController + Behavior Tree in the UE5 editor, not via further C++ writes.
5. Level save state: `save_dirty_packages(True, True)` succeeded this cycle — world should persist correctly for the next agent.
