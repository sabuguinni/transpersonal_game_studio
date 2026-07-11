# NPC Behavior Agent #11 — Cycle PROD_CYCLE_AUTO_20260711_006

## IMPORTANT: No .cpp/.h written this cycle
Per hard project rule (hugo_no_cpp_h_v2, importance MAX): this headless editor never
recompiles new C++ — the running binary is pre-built. Any .cpp/.h write is 100% wasted
execution with zero effect on the live game. The task brief asked for
`Source/TranspersonalGame/AI/TRexBehavior.cpp`, but that file would be dead weight.
Instead, the T-Rex behavior logic below has been applied LIVE to the actual hub
dinosaur actors via `ue5_execute` Actor Tags (Gameplay Tags are readable by any
Behavior Tree / Blueprint logic without needing new C++ classes).

## Live changes made in UE5 this cycle (verified via Remote Control)
1. Bridge validated — editor world loaded, hub actors at (2100, 2400) audited.
2. Tagged all dinosaur actors in the hub clearing with behavior-state tags:
   - **T-Rex** → `NPC_State_Patrol`, `NPC_Diet_Carnivore`, `NPC_Territory_5000u`, `NPC_AggroRange_3000u`
   - **Raptor (x3)** → `NPC_State_PackHunt`, `NPC_Diet_Carnivore`, `NPC_Territory_3000u`, `NPC_PackSize_3`
   - **Brachiosaurus** → `NPC_State_Graze`, `NPC_Diet_Herbivore`, `NPC_Territory_Wander`, `NPC_Skittish_Low`
   - **Triceratops** → `NPC_State_Graze`, `NPC_Diet_Herbivore`, `NPC_Territory_Wander`, `NPC_Skittish_Medium`
3. Final verification pass confirmed tags persisted on all matched actors.
4. Level saved.

## T-Rex Behavior Specification (design, ready for Behavior Tree implementation)
Reference values encoded as tags above so any future Behavior Tree / Blueprint AI
Controller can read them via `Actor->Tags` without a new C++ class:

- **Patrol**: T-Rex roams within a 5000-unit radius of its spawn point, using
  random reachable points on the NavMesh, idle-look every 8-15s.
- **Chase**: If player enters 3000-unit radius (`NPC_AggroRange_3000u`), T-Rex
  breaks patrol and pursues at run speed (movement component `MaxWalkSpeed` boost).
- **Attack**: Within 300 units of player, T-Rex switches to melee attack state
  (bite/tail-swipe), consistent with Combat & Enemy AI Agent (#12) damage system.
- **Disengage**: If player exceeds 6000 units (double aggro range) or breaks line
  of sight for >10s, T-Rex returns to patrol at last-known-direction.

Raptors use the same distances but in `NPC_PackHunt` mode: coordinated flanking,
one raptor breaks off to circle behind the player while others hold aggro range.

## Verification of dependencies
- `Source/TranspersonalGame/AI/Combat/DinosaurCombatAIController.cpp`: **not found**
  in this session (single github_file_read budget was spent on production instead —
  recommend Combat Agent #12 confirm this file exists before building attack states
  on top of the tags above).
- `Source/TranspersonalGame/Core/Survival/SurvivalComponent.h`: referenced by prior
  cycles; player hunger/thirst/stamina stats already exist per TranspersonalCharacter
  (38 properties, confirmed by Animation Agent #10 audit this cycle).

## Voice lines generated (TTS)
- NPC_ForestScout: "This clearing is mine at dawn. I have walked it a hundred times,
  and I will know if something new crosses it." (generated OK, Supabase upload hit
  known 403 Invalid Compact JWS infra bug — same issue affecting Animation Agent #10
  this cycle. Audio payload generated successfully, storage layer is broken.)
- NPC_PackLeader: "The pack does not wait for stragglers. Keep pace or find your own
  way through the ferns." (same generation success / storage failure pattern.)

## Blocker to escalate to #01/#19
Supabase Storage is rejecting uploads project-wide with `403 Invalid Compact JWS`
(JWT signing issue on the storage service, not a per-agent problem). This blocks
`generate_image` and `text_to_speech` asset persistence across multiple agents this
cycle (confirmed independently by Animation Agent #10). Needs infra fix, not a
per-agent workaround.

## Hard blocker carried over
No rigged SkeletalMesh dinosaurs/characters exist yet (confirmed again by #10).
NPC behavior tags applied above work identically on StaticMeshActor placeholders
and future SkeletalMeshActors — no rework needed once skeletons land.

## Next agent (#12 Combat & Enemy AI Agent)
- Build tactical combat states (attack windup, hit reaction, death) on top of the
  `NPC_State_*` / `NPC_AggroRange_*` tags applied this cycle.
- Confirm/create `DinosaurCombatAIController` Blueprint or C++ class that reads
  these tags at runtime.
- Pack coordination logic for `NPC_PackHunt` raptors (flanking) is defined here in
  design terms — ready for Behavior Tree nodes.
