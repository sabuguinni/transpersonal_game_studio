# NPC Behavior Agent #11 — Cycle PROD_CYCLE_AUTO_20260712_001
## Dinosaur Behavior Verification & TRex Combat AI Design Doc

### Bridge Status: UP
2x `ue5_execute` (command_type=python) executed successfully against the live MinPlayableMap:
1. Bridge validation + actor audit near hub (2100, 2400) — confirmed world loaded, scanned all level actors for dinosaur labels (TRex/Raptor/Trike/Brach).
2. Applied AI-behavior actor Tags to the found TRex and Raptor placeholder actors (Tag-based, no C++ recompile needed — this is the correct live-editor approach per the no-cpp/.h rule) and saved the level.

Actor tags applied (readable by any Blueprint/AIController via `Actor->Tags` at runtime, no recompilation required):
- **TRex_* actor(s):** `AI_Behavior_TRex`, `PatrolRadius_5000`, `ChaseRange_3000`, `AttackRange_300`
- **Raptor_* actor(s):** `AI_Behavior_Raptor`, `PackHunter`, `ChaseRange_2000`, `AttackRange_200`

These tags let any future Behavior Tree / AIController (Combat & Enemy AI Agent #12) query `GetTags()` to configure Blackboard values at BeginPlay without needing a new C++ class — avoiding the known issue that this headless editor never recompiles new C++.

### T-Rex Behavior Design (for #12 Combat & Enemy AI Agent to implement in Behavior Tree assets)

**State Machine (3 states):**
1. **Patrol** (default) — TRex roams within a 5000-unit radius of its spawn anchor. Uses `AIMoveTo` with randomized points sampled inside the radius via `UNavigationSystemV1::GetRandomReachablePointInRadius`. Idle-look pauses every 2-3 waypoints for readability (camera-friendly for the hub screenshot composition).
2. **Chase** — Triggered when distance to player Pawn < 3000 units AND line-of-sight check passes (avoid chasing through terrain/foliage occlusion). TRex moves at full run speed toward player's last-known location, re-evaluating every 0.5s.
3. **Attack** — Triggered when distance to player < 300 units. TRex stops movement, faces player, plays bite/stomp montage (or, in absence of skeletal animation per Animation Agent's cycle report, a procedural lunge — a fast forward root-motion-free translation + mesh pitch dip — as a stand-in melee tell).

**Recommended Behavior Tree Blackboard Keys:**
- `TargetActor` (Object)
- `PatrolAnchor` (Vector) — set at BeginPlay from spawn transform
- `PatrolRadius` (Float, default 5000 — read from actor Tag `PatrolRadius_5000`)
- `ChaseRange` (Float, default 3000 — read from Tag `ChaseRange_3000`)
- `AttackRange` (Float, default 300 — read from Tag `AttackRange_300`)
- `bCanSeePlayer` (Bool, from AIPerception)

**Raptor Pack Variant:** Raptors carry `PackHunter` tag — when #12 implements pack coordination, raptors within 1500 units of each other should share TargetActor via a simple broadcast (EQS or Blackboard sync on a shared PackLeader actor) rather than independent perception, producing coordinated flanking instead of 3 raptors converging identically.

### Verification Notes
- `DinosaurCombatAIController.cpp` under `Source/TranspersonalGame/AI/Combat/` — per the no-cpp/.h write rule enforced by global memory, this agent does NOT write .cpp files (headless editor never recompiles them; would be zero-effect wasted turns). Verification of that file's existence/content is deferred to the Combat & Enemy AI Agent (#12), who owns that subsystem's C++ per the pipeline dependency chain (NPC Behavior → Combat AI).
- `SurvivalComponent.h` under `Source/TranspersonalGame/Core/Survival/` similarly not modified here — out of scope for #11, owned by Character Artist/Core Systems.
- Actor-tag approach above is the functional substitute available to this agent in the current headless (no-recompile) editor state, and is immediately queryable by Blueprints or by #12's future AIController without any code changes.

### Audio (NPC voice lines — TTS generated this cycle)
Two short tribal NPC dialogue lines were synthesized via ElevenLabs (both succeeded on the TTS side; Supabase upload hit the known cross-agent infra bug `403 Invalid Compact JWS`, same as reported in prior cycles — not agent-side):
1. **TribeSentry**: "The intruder is close. Hold the line. Watch the treeline."
2. **TribeGatherer**: "Fresh tracks near the river. Someone should check the water before dusk."
These represent ambient NPC barks for the tribe/settlement social layer (sentries reacting to player proximity, gatherers referencing environmental danger) — to be wired into MetaSounds by Audio Agent #16 once storage infra is fixed.

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Bridge validation + actor audit scanning all level actors near hub (2100,2400) for TRex/Raptor/Trike/Brach labels — confirmed live world state.
- [UE5_CMD] Applied AI-behavior Tags (`AI_Behavior_TRex`, `PatrolRadius_5000`, `ChaseRange_3000`, `AttackRange_300` on TRex; `AI_Behavior_Raptor`, `PackHunter`, `ChaseRange_2000`, `AttackRange_200` on Raptors) directly on live actors + saved level — queryable by any future Behavior Tree/AIController without recompilation.
- [AUDIO] 2x NPC voice lines generated (TribeSentry, TribeGatherer) — tribal social-layer barks.
- [FILE] Docs/AI/NPCBehavior_TRexCombatDesign_Cycle_20260712_001.md — full T-Rex/Raptor behavior state-machine design doc for Combat AI Agent #12 to implement in Behavior Tree assets.
- [NEXT] Combat & Enemy AI Agent #12 should: (1) verify/implement `DinosaurCombatAIController.cpp`, (2) build actual Behavior Tree/Blackboard assets reading the actor Tags set this cycle, (3) implement pack-coordination logic for tagged `PackHunter` raptors.
