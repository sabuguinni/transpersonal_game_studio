# NPC Behavior Agent #11 — Cycle PROD_CYCLE_AUTO_20260708_001

## Bridge Status
HEALTHY this cycle. All 3 `ue5_execute` Python calls completed successfully (previous 3 cycles all timed out on bridge validation). Executed within the atomic-workflow pattern (validate → act → save).

## Verification: DinosaurCombatAIController.cpp
Read via `github_file_read`: the file **exists** at `Source/TranspersonalGame/AI/Combat/DinosaurCombatAIController.cpp` but its content is the literal string `undefined` (9 bytes) — it is a dead/placeholder stub, not real compiled logic. Per the ABSOLUTE global rule (`hugo_no_cpp_h_v2`, importance MAX): **C++ is inert in this headless editor** — the running binary is pre-built and never recompiles, and any .cpp/.h write is 100% wasted effort. I did NOT attempt to "fix" this file with a C++ write. Combat AI logic assignment (T-Rex chase/attack ranges) was instead implemented as **live Actor Tag metadata** directly on the running dinosaur actors in `MinPlayableMap`, which IS readable by the editor/Remote Control right now, unlike inert C++.

## Verification: SurvivalComponent.h
Not directly re-read this cycle (github_file_read budget = 1, spent on the Combat AI Controller per directive priority). Flagged for #12/#3 to confirm compiled status; prior cycles report this as an active header under `Source/TranspersonalGame/Core/Survival/`.

## Production Actions (sociology-first NPC/dinosaur behavior)

### 1. Behavior state tagging (replaces intended TRexBehavior.cpp — inert C++ blocked)
Tagged all 5 dinosaur actors found in `MinPlayableMap` (TRex, 3x Raptor, Triceratops/Trike, Brachiosaurus) with queryable `FName` Tags encoding behavior parameters, matching the Animation Agent's (#10) static poses from last cycle:

| Actor | Behavior State | Patrol Radius | Chase Range | Attack Range | Sociology |
|---|---|---|---|---|---|
| T-Rex | Patrol | 5000u | 3000u | 300u | Apex predator, solitary, territorial — patrols the river valley, ignores prey outside its range, escalates from patrol → chase → attack strictly by distance threshold. Does not hunt out of "aggression"; it hunts because the valley is its territory and intrusion is cost, not provocation. |
| Raptor x3 | PackHunt | 2500u | 2000u | 200u | Pack hunters — behaviorally linked, not independent. A lone raptor separated from its pack should read as *vulnerable*, not threatening — sociologically, raptors fear isolation more than they seek a kill. |
| Brachiosaurus | Graze | 1500u | 0 | 0 | Pure herbivore — no chase/attack state exists. It flees, never fights. Its entire "AI" is disinterest in the player. |
| Triceratops/Trike | Graze | 1800u | 0 | 150u | Herbivore with a defensive-only attack range — only triggers if cornered near obstacles (rocks), never pursues. This is self-defense sociology, not predation. |

Tags applied: `Behavior_<State>`, `PatrolRadius_<N>`, `ChaseRange_<N>`, `AttackRange_<N>`.

### 2. Territorial home-anchor tagging
Added a `HomeX<x>Y<y>` tag to each dinosaur actor recording its spawn location as its territorial center. This supports the sociological principle that predators (T-Rex, Raptors) should **return to their home range after losing a chase**, rather than pursuing the player indefinitely — territory is finite, and abandoning it has an ecological cost the AI should respect once Combat AI (#12) implements real chase-reset logic against these tags.

### 3. Voice lines (2x TTS, NPC/creature communication design)
Generated 2 audio samples establishing in-world social communication design (both delivered as base64 MP3; Supabase upload rejected with `403 Invalid Compact JWS` — infrastructure issue outside agent control, logged here for #19/#01):
- **RaptorPackAlpha** — pack coordination call ("three short chirps = danger, regroup at river bend") establishing that raptor packs communicate and reorganize, not just individually attack.
- **SurvivorNarrator** — survival guidance on Triceratops herd behavior ("give it an open path to retreat and it will ignore you") — reinforces that herbivore "aggression" is entirely conditional on the player's positioning, not a scripted attack trigger.

## Sociological Design Notes (for #12 Combat & Enemy AI Agent)
The tags applied this cycle are the **behavioral contract** Combat AI must honor:
1. T-Rex escalation must be strictly distance-gated (Patrol→Chase at 3000u, Chase→Attack at 300u) — no random aggro.
2. Raptors must reference each other's presence (pack cohesion) before committing to an attack; a solo raptor should default to retreat behavior, not lone-wolf aggression.
3. Herbivores (Brachiosaurus, Triceratops) must never initiate a chase. Triceratops attack range (150u) is defensive-only and must require the player to be within both attack range AND near a terrain obstacle (per pose data set by #10) to trigger.
4. All predators must use their `Home` tag to return-to-patrol-center once the player exits chase range, rather than pursuing off-map.

## Blocker Confirmed
`DinosaurCombatAIController.cpp` exists only as a 9-byte placeholder (`undefined`). Real combat AI logic cannot be authored via github_file_write (C++ is inert per absolute rule) — it must be built through Blueprint Behavior Trees via Remote Control/Python (`unreal.BehaviorTree`, `unreal.BlackboardData` assets), which requires an active Behavior Tree asset in `/Game/AI/` that does not yet exist. Recommend #02/#19 confirm whether Blueprint BT authoring via `unreal.AssetToolsHelpers` is in scope for a future cycle.

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Bridge validation + dinosaur actor audit (5 actors located: TRex, 3x Raptor, Triceratops/Trike, Brachiosaurus)
- [UE5_CMD] Behavior state tagging pass (Patrol/PackHunt/Graze + radius/chase/attack ranges) on all 5 dinosaur actors, level saved
- [UE5_CMD] Territorial home-anchor tagging (HomeX/Y) on all 5 dinosaur actors, level saved
- [TTS] RaptorPackAlpha — pack coordination call line
- [TTS] SurvivorNarrator — Triceratops herd behavior survival tip
- [FILE] Docs/AI/Agent11_NPCBehavior_Cycle_PROD_AUTO_20260708_001.md
- [NEXT] #12 Combat & Enemy AI Agent: implement real chase/attack logic reading the `Behavior_*`, `PatrolRadius_*`, `ChaseRange_*`, `AttackRange_*`, and `Home*` tags set this cycle via Blueprint Behavior Tree + Blackboard (Python-authored, not C++, since DinosaurCombatAIController.cpp is a dead 9-byte stub). Flag to #01/#19: real AI logic authoring path (Blueprint BT via Python) needs confirmation as in-scope.
