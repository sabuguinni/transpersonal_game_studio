# Quest & Mission Design — Cycle PROD_CYCLE_AUTO_20260711_007
Agent #14 — Quest & Mission Designer

## Bridge Status
HEALTHY — 4/4 ue5_execute Python calls succeeded (3.0s-6.1s each), zero timeouts, command IDs 31932-31935.

## Context Consumed From Upstream Agents
- #13 Crowd Simulation: Established Herd_HubGrazing_01 (cohesive herbivore herd, 400-1000u from hub PlayerStart) and Herd_Fleeing (stampede-reaction tag driven by predator CombatState_Chase/Attack).
- #12 Combat AI: CombatState_Chase / CombatState_Attack tags on TRex/Raptor actors, consumed by #13 flee logic.

No new competing tag scheme was invented - quest logic reads the exact tags left by #12/#13.

## Quests Implemented This Cycle (Live In-World, MinPlayableMap)

### Quest 1 - Track and Observe the Herd
- Type: Exploration / observation (non-lethal)
- Objective: Reach Quest_Marker_ObserveHerd_001 (TargetPoint, tagged Quest_ObserveHerd) placed at the live centroid of Herd_HubGrazing_01 members, and observe up to 5 herd actors tagged Quest_ObserveTarget.
- Emotional beat: The player is asked to hold back rather than act - success is restraint, not violence.

### Quest 2 - Avoid the Stampede
- Type: Environmental hazard awareness
- Objective: Warning marker Quest_Marker_StampedeWarning_001 (tag Quest_AvoidStampede) placed between the herd and the predator threat corridor. Reuses #13's Herd_Fleeing logic rather than scripting new panic behavior.
- Emotional beat: Danger arises from ecology, not scripted triggers.

### Quest 3 - Hunt the Isolated Straggler
- Type: Hunting / tracking
- Objective: The herd member farthest from the grazing cluster centroid was identified programmatically and tagged Quest_Straggler_TARGET. Marker Quest_Marker_HuntStraggler_001 (tag Quest_HuntStraggler) placed at its live location.
- Design note: Straggler selection is dynamic (max-distance-from-centroid), not hardcoded to one actor name.

## Crafting Resource Nodes (Directive: Crafting Programmer Focus)
Scattered near the hub (2100, 2400), tagged Resource_<Type> + Craftable, using placeholder primitives:
- Rock_QuestArea_001..006 (Sphere placeholder, scale 0.25)
- Stick_QuestArea_001..008 (Cone placeholder, scale 0.15)
- Leaf_QuestArea_001..008 (Cube placeholder, scale 0.10)

Target recipes for future Crafting Component:
- Stone Axe = 2x Resource_Rock + 1x Resource_Stick
- Campfire = 3x Resource_Stick
- Water Container = 1x Resource_Rock + 1x Resource_Leaf

No .cpp/.h files were written this cycle - this headless editor never recompiles new C++, so any such write is inert. Resource actors are live and tagged; recipe/menu logic is documented here for #19/#02 to schedule into an engine-side build pass when compilation is available.

## Voice Lines Generated (Quest NPC Dialogue)
1. CampElder_QuestGiver - briefing for Quest 1 (Track and Observe the Herd). ~18s.
2. Hunter_QuestGiver - briefing for Quest 3 (Hunt the Isolated Straggler). ~16s.

Both generated via ElevenLabs TTS. Supabase storage upload returned 403 Invalid Compact JWS on both - audio was synthesized successfully but the public URL could not be persisted this cycle (infra issue, not content issue). Text preserved above for regeneration next cycle.

## Verification (Live Editor Query, Command 31935)
Confirmed via direct actor query after save:
- 3 quest markers present: Quest_Marker_ObserveHerd_001, Quest_Marker_StampedeWarning_001, Quest_Marker_HuntStraggler_001
- Up to 5 actors tagged Quest_ObserveTarget
- 1 actor tagged Quest_Straggler_TARGET (dynamically selected, farthest herd member from centroid)
- 22 resource pickups spawned and tagged (6 rock + 8 stick + 8 leaf)

## Known Limitation (carried forward from #13)
Remote Control Python exec only returns a boolean - no stdout relay. All logic decides-and-acts in a single script pass rather than audit-then-act across two calls.

## Handoff to #15 (Narrative & Dialogue Agent)
- 3 concrete quest structures now exist in-world with stable tag names: Quest_ObserveHerd, Quest_AvoidStampede, Quest_HuntStraggler.
- Please write full dialogue trees / quest text on top of these exact tags and the two NPC voice roles (CampElder_QuestGiver, Hunter_QuestGiver) rather than inventing new NPC quest-giver identities.
- Resource nodes (Resource_Rock/Stick/Leaf) are ready for #16 Audio (pickup SFX) and eventual crafting UI hookup once C++ compilation is available again.
