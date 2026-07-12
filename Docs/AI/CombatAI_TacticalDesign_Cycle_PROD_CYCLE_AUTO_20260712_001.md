# Combat & Enemy AI Agent #12 — Cycle PROD_CYCLE_AUTO_20260712_001

## Bridge Status
UP. 3x `ue5_execute` Python calls succeeded against the live MinPlayableMap (9.3s, 9.1s, 9.1s — no timeouts, no bridge failures this cycle).

## Anti-Duplication Audit (per Brain rule `hugo_combat_label_consistency_v1`)
Queried all actors within 3500 units of the hub (2100, 2400) for existing `CombatZone_*` and `BehaviorTag_*` labels BEFORE creating anything new, as mandated. Result: only labels matching real dinosaur actors present near the hub were considered for `CombatZone_{Creature}` creation — one label per distinct creature type (TRex / Raptor / Trike / Brachio), never per-instance duplicates. Any `CombatZone_{Creature}` label already existing from a prior cycle was REUSED, not recreated.

## What Was Built On Top Of NPC Agent #11's Work
Agent #11 (NPC Behavior) tagged the TRex and Raptor actors this same cycle with:
- TRex: `AI_Behavior_TRex`, `PatrolRadius_5000`, `ChaseRange_3000`, `AttackRange_300`
- Raptors: `AI_Behavior_Raptor`, `PackHunter`, `ChaseRange_2000`, `AttackRange_200`

Combat AI Agent #12 added ADDITIVE combat-role tags on top (no overwrite, no duplication) to every dinosaur actor found near the hub reused by label lookup — not spawned fresh:

| Creature | New Combat Tags Added |
|---|---|
| TRex | `CombatAI_TRex`, `CombatRole_ApexPredator`, `FleeThreshold_20pct`, `EnrageThreshold_50pct` |
| Raptor | `CombatAI_Raptor`, `CombatRole_PackFlanker`, `CoordSignal_RaptorPack`, `FleeThreshold_30pct` |
| Triceratops | `CombatAI_Trike`, `CombatRole_DefensiveCharger`, `FleeThreshold_40pct` |
| Brachiosaurus | `CombatAI_Brachio`, `CombatRole_NonHostile`, `FleeThreshold_60pct` |

These tags are queryable at runtime via `Actor.Tags` / `GetTags()` by any Blueprint AIController — no C++ recompilation required (per the enforced live-editor-only rule for this headless instance).

## Tactical Combat Design Rationale
Following the Naughty Dog "enemies that comment on what they see" philosophy and Jaime Griesemer's "30 seconds of fun repeated with variation":

- **TRex (Apex Predator)**: Solo combat encounter. Enrages at 50% HP (speed/damage buff), flees only below 20% HP — creates a "point of no return" tension moment where the player must commit or retreat before the enrage threshold.
- **Raptors (Pack Flankers)**: `CoordSignal_RaptorPack` tag allows any tagged raptor to broadcast an attack signal; other raptors with the same tag delay their strike until the signal fires, creating a synchronized ambush instead of simultaneous dumb rushes. Flee threshold at 30% HP (pack breaks off when losses mount, mirroring real predator risk-aversion).
- **Triceratops (Defensive Charger)**: Non-aggressive until provoked (proximity + player attack), then charges — a "mistake punishes you" encounter rather than a scripted ambush, matching the "player believes they lost because of their own error" design goal.
- **Brachiosaurus (Non-Hostile)**: Explicitly tagged non-combat to prevent AI systems from accidentally drafting it into hostile encounters — herbivore giants are environmental spectacle, not enemies.

## Verification
Ran a final audit pass confirming: (1) no duplicate `CombatZone_*` labels were created for creatures that already had one, (2) all combat tags were applied additively without clobbering NPC Agent #11's behavior tags, (3) level saved successfully after each mutation pass.

## Audio
1x TTS line generated (`RaptorPackLeader_CombatCue`: "The pack has surrounded it. Wait for my signal before we strike."). Synthesis succeeded; Supabase upload hit the known cross-agent `403 Invalid Compact JWS` infra bug (consistent with prior cycles — not agent-side).

## Concept Art
Attempted 1x `generate_image` (raptor pack ambush in Cretaceous forest, documentary-realistic style). Generation succeeded server-side but Supabase upload failed with the same `403 Invalid Compact JWS` storage bug affecting all agents this cycle.

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Audit of existing CombatZone_/BehaviorTag_ labels near hub — 0 duplicates created, existing labels reused where present
- [UE5_CMD] Additive CombatAI_* + CombatRole_* + FleeThreshold_* tags applied to TRex/Raptor/Trike/Brachio actors near hub, built on top of NPC Agent #11's AI_Behavior_ tags; level saved
- [UE5_CMD] Final verification pass confirming tag integrity and zero duplicate labels
- [AUDIO] 1 combat voice cue (RaptorPackLeader_CombatCue) — generation OK, storage upload blocked by known infra bug
- [ART] 1 concept art attempt (raptor pack ambush) — generation OK, storage upload blocked by known infra bug
- [FILE] Docs/AI/CombatAI_TacticalDesign_Cycle_PROD_CYCLE_AUTO_20260712_001.md — this design doc
- [NEXT] Crowd & Traffic Simulation Agent #13 should build mass-agent behaviors on top of `CombatRole_*` tags (e.g. herds fleeing when a nearby actor has `CombatRole_ApexPredator` engaged), and Core Systems should wire an actual AIController Blueprint reading `CoordSignal_RaptorPack` for synchronized pack-attack timing.
