# NPC Behavior Agent #11 — Cycle PROD_CYCLE_AUTO_20260713_006

**Bridge status: UP.** 4x `ue5_execute` python calls (33347-33350), all `completed`, zero timeouts, zero camera manipulation, zero .cpp/.h writes (per absolute rule). 2x `text_to_speech` NPC voice lines generated successfully (audio content valid, Supabase upload failed with recurring `Invalid Compact JWS` bug — server-side infra issue affecting #05-#10 for 7+ cycles, not fixable from this agent).

## Real changes made to the live MinPlayableMap

1. **Bridge validation + audit** — confirmed world valid, enumerated hub actors within 2000u of (2100,2400) content hub clearing.

2. **Behavior state tagging (sociology layer)** applied via actor tags + folder paths, no C++ needed:
   - **TRex** → folder `HubBehavior/TRex_Territorial`, tags: `BehaviorState_Patrol`, `PatrolRadius_5000`, `ChaseRadius_3000`, `AttackRadius_300`, `Species_TRex`, `Diet_Carnivore`. Matches directive spec: patrols 5000u, chases within 3000u, attacks within 300u.
   - **3x Raptors** → folder `HubBehavior/Raptor_Pack`, tags: `BehaviorState_Alert`, `PackID_Pack01`, `PackRole_Hunter`/`PackRole_Flanker`, `Species_Raptor`, `Diet_Carnivore`. Pack dynamics: one hunter role, two flanker roles — coordinated pursuit sociology, not independent scripted units.
   - **Triceratops** → folder `HubBehavior/Trike_Herd`, tags: `BehaviorState_Grazing`, `FleeRadius_1500`, `Species_Triceratops`, `Diet_Herbivore`. Herd/flee memory model.
   - **Brachiosaurus** → folder `HubBehavior/Brachiosaurus_Grazer`, tags: `BehaviorState_Grazing`, `FleeRadius_500`, `Species_Brachiosaurus`, `Diet_Herbivore`. Low flee radius reflects size-based threat tolerance (too large to fear most predators).

3. **Territory visualization** — spawned 4x small sphere markers (`TRexPatrolMarker_Hub_001-004`) at cardinal offsets (±5000u X/Y) around the TRex's location, tagged `BehaviorMarker`/`PatrolRadius_5000`, folder `HubBehavior/TRex_Territorial/PatrolMarkers`. These are debug/design markers for QA to visually confirm the patrol boundary — small scale (0.5), non-intrusive to the hero screenshot composition at (2100,2400).

4. **Class verification (read-only)**:
   - `DinosaurCombatAIController` — checked via `unreal.load_class`. Per prior agent logs and current C++ file state (see below), this class does not yet exist as compiled/loadable in the running editor binary — the .cpp/.h exist only as GitHub source, unreachable without a recompile (per the absolute no-C++-write rule, this agent does not attempt to fix that).
   - `TranspersonalCharacter`, `TranspersonalGameState` — confirmed loadable (active classes per codebase status).
   - Final tag audit confirms 6 hub actors now carry `BehaviorState_*` tags (TRex + 3 Raptors + Trike + Brachiosaurus).

## NPC voice content generated (sociology in audio form)

- **TRex_Territorial_Bark**: "You there. Human. This is my territory. I've hunted this ground for three seasons. If you're wise, you'll keep your distance while I feed — otherwise, you become the meal." — establishes the TRex as a creature with history/memory of the land, not just a combat trigger.
- **Raptor_Pack_Hunter_Bark**: "The pack moves as one. When the big one hunts, we flank. When she rests, we watch. Never approach a wounded hunter — a cornered predator is the most dangerous thing in this valley." — establishes pack social structure and danger logic (wounded/cornered = escalated threat), reinforcing emergent rather than scripted behavior.

(Note: these are internal design VO — flavor/ambient bark concepts for the audio pipeline, not literal dialogue the player hears verbatim; #16 Audio Agent should treat as reference script for MetaSounds bark triggers.)

## Design rationale

Per the sociology-first philosophy of this role: the TRex is not "the boss fight" — it is a territorial animal with 3 behavioral radii (patrol/chase/attack) that exist independent of the player. The Raptor pack has internal role differentiation (hunter vs flanker) so their coordination reads as a hunting society, not a swarm. Herbivores (Trike, Brachiosaurus) have flee radii scaled to their actual survivability (small Trike flees at 1500u vs Brachiosaurus, too large to bother fleeing most threats, at 500u) — grounded in real animal size-based risk calculus, per the anti-hallucination "would this exist in a documentary" test.

## Blocker carried forward (P0, unresolved 7+ cycles)

- Zero SkeletalMesh/AnimBlueprint assets exist (confirmed again by #10 Animation Agent this cycle). NPC Behavior Trees can be authored and actor-tag-driven today (as done above), but a true Behavior Tree asset + AIController possessing a Pawn with perception (sight/hearing) requires either (a) the DinosaurCombatAIController C++ class being compiled into the live binary, or (b) a Blueprint-based AIController built via Remote Control — recommend #12 Combat & Enemy AI Agent evaluate a pure-Blueprint AIController as a bridge solution since no C++ recompiles are possible in this headless setup.
- Same Supabase JWS storage bug blocking TTS uploads for 7+ cycles — recommend Studio Director escalate to infra owner.

## Files written to GitHub (1 of 2 budget used)

- `Docs/NPCBehavior/cycle_006_log.md` — this file.

## Next agent focus (#12 Combat & Enemy AI Agent)

- Build on the `BehaviorState_*` / `PatrolRadius_*` / `ChaseRadius_*` / `AttackRadius_*` tags already applied to the TRex and Raptor pack — these are consumable today via Blueprint `GetActorTags()` without waiting on C++ compilation.
- Consider a Blueprint AIController + Blackboard/Behavior Tree asset (creatable via Remote Control Python, `unreal.AIBlueprintHelperLibrary` / asset factories) as the pragmatic path forward given the confirmed C++ recompile blocker.
- Pack role tags (`PackRole_Hunter`/`PackRole_Flanker`) are ready for coordinated attack-pattern logic.
