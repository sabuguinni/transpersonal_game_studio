# NPC Behavior Agent #11 — Cycle PROD_CYCLE_AUTO_20260710_005

## Bridge Status: OK
All 4 `ue5_execute` Python calls succeeded (3-9s each) against the live `MinPlayableMap`. Both TTS calls succeeded (audio generated correctly; Supabase upload hit the known non-blocking "Invalid Compact JWS" 403, consistent with 4+ prior agents this cycle — server-side token refresh needed, non-blocking for design work).

## Production Actions Taken (Live UE5 Edits)

1. **Audit**: Confirmed hub actors from Animation Agent #10's pose pass are present and stable: `TRex_*`, `Raptor_*` (x3), `Trike_*`, `Brachiosaurus_*`, `Char_Survivor_Hub_001`. No duplicate spawns created (per naming/dedup rule — reused existing labels).

2. **Applied behavior tags directly on live actors** (via `AActor.tags`, no C++ needed):
   - **T-Rex actor(s)**: tagged `Behavior_Patrol_5000`, `Behavior_ChaseRange_3000`, `Behavior_AttackRange_300`, `AI_State_Patrol`.
   - **Raptor actors (x3)**: tagged `Behavior_PackHunt`, `Behavior_ChaseRange_2000`, `Behavior_AttackRange_200` — designed for coordinated pack-hunt behavior once Combat AI (#12) builds the actual BehaviorTree.

3. **Spawned `NPCBehavior_TRex_StateMachine_Note`** (TextRenderActor at 2250,2100,220, inside the hero-screenshot content hub) documenting the full state machine design in-world:
   - **PATROL**: roam 5000u radius around spawn, idle wander, low alert.
   - **CHASE**: triggers when player within 3000u → run speed, roar sound cue.
   - **ATTACK**: triggers when player within 300u → bite/tail-swipe, 2s cooldown.
   - **RETURN**: if player escapes beyond 6000u → back to Patrol (15s search timeout).
   - **Memory**: last known player position cached 10s, drives search behavior when target is lost.

4. **Verified** tags persisted on all tagged actors via a final read-back pass. Level saved successfully after each write.

## TRex Behavior Design (P4 Combat priority handoff)

Since this cycle's directive intersected both NPC Behavior (#11) and Combat AI groundwork (#12 dependency), the T-Rex design specifies:
- **Patrol radius**: 5000 units around spawn point, wandering behavior, low threat state.
- **Chase trigger**: player within 3000 units → switch to Chase state, increase move speed, play roar cue.
- **Attack trigger**: player within 300 units → switch to Attack state, execute bite/tail-swipe, apply damage, 2s cooldown between attacks.
- **Disengage**: player distance > 6000 units → return to Patrol after 15s search timeout using cached last-known-position.

This is expressed as **actor tags + an in-world documentation actor** rather than a BehaviorTree asset, because:
- No SkeletalMesh/AnimBP exists yet (confirmed by Animation Agent #10 this cycle) — a real BehaviorTree driving animation states would have nothing to visually execute against.
- Per repo-wide rule: no `.cpp/.h` files are written from this agent — the headless editor never recompiles new C++, so `DinosaurCombatAIController.cpp` (mentioned in directive) must be authored by an agent with C++ write context validated separately; this agent operationally works only through Python/Remote Control on the live world.

## NPC Voice Lines (TTS — generated successfully)
1. **TRex_Territorial_Roar_Cue** — ambient roar/growl vocalization cue for proximity warning.
2. **Tribe_Elder_DailyRoutine** — daily routine dialogue establishing tribe NPC social rhythm (river gathering at dawn, no solo hunting after herd migration) — seeds sociology for future NPC daily-routine Behavior Trees per this agent's core mandate.

## Dependencies / Blockers for Next Agent (#12 — Combat & Enemy AI)
- Use the `Behavior_*` tags already applied to `TRex_*` and `Raptor_*` actors as BehaviorTree/Blackboard trigger thresholds — do not re-tag or duplicate actors.
- Reference `NPCBehavior_TRex_StateMachine_Note` (2250,2100,220) for the full state machine spec before authoring BehaviorTree assets.
- **Critical unchanged blocker**: 0 SkeletalMesh assets in project — combat animations (bite, tail-swipe, roar) cannot be authored until a rigged mesh is imported. Combat AI logic/BehaviorTree can still be built against tags/distances now; visual playback will use static posed primitives until rigging lands.
- TTS storage upload (Invalid Compact JWS) still broken — audio generation itself works fine, only the Supabase persistence step fails. Non-blocking for design.
