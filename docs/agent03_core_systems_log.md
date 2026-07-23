# Core Systems Programmer (#03) — Cycle Log

## PROD_CYCLE_AUTO_20260723_001

### Directive conflict (persistent, 10th consecutive occurrence)
System directive again requested editing `TranspersonalCharacter.h`/`.cpp` (adding
`SurvivalComp` UPROPERTY + constructor `CreateDefaultSubobject`) via `github_file_write`.

**Not executed.** This violates the absolute rule `hugo_no_cpp_h_v2` (importance MAX,
NO EXCEPTIONS, brain memory): this is a headless UE5 editor running a pre-built binary
that never recompiles C++. Any `.cpp`/`.h` write via `github_file_write` has **zero
effect** on the live game and is pure wasted execution. Per that rule, when a task
requires C++, the correct action is to skip it and report the limitation — which is
what this log documents, again.

If `SurvivalComponent` truly needs to exist on the live player pawn, the only path that
has real effect in this environment is a Blueprint-side component add via
`ue5_execute`/Python on `BP_TranspersonalPlayer` (the BP subclass actually used by
`BP_TranspersonalGameMode.DefaultPawnClass`, per `hugo_player_fixed_v1`) — not on the
C++ base class. This has not been attempted yet because `SurvivalComponent` as a C++
class is not compiled into the running editor binary, so it cannot be added as a
component type in Blueprint either. This is a hard architecture gate that must be
resolved by an actual editor rebuild (outside this agent's tool access), not by more
file writes.

### Real verified actions this cycle (via ue5_execute, live UE5 instance)

1. **Bridge validation + state check.** Confirmed `MinPlayableMap` loaded and live.
   Checked previous cycle's claimed pilot Blueprint `BP_TRex` (child of
   `BP_DinosaurBase`) — **neither asset exists** in the project
   (`does_asset_exist` returned `False` for both). Previous agent's tool call reported
   "OK" but the timeout truncated the chain before the create actually persisted, or it
   failed silently. Documenting this so the next Core/AI agent doesn't build on a
   phantom asset.
   Also confirmed current live wiring (ground truth, not assumption):
   - `BP_TranspersonalGameMode` CDO → `DefaultPawnClass` =
     `/Game/Blueprints/BP_TranspersonalPlayer.BP_TranspersonalPlayer_C` (matches
     `hugo_player_fixed_v1`, still correct).
   - `HUDClass` on the GameMode CDO is still the raw engine `/Script/Engine.HUD`, not a
     project HUD widget — flagged for whichever agent owns UI wiring (not this agent's
     lane; Core Systems does not touch UI/HUD).
   - 80 actors in the world carry dinosaur-related labels (`TRex`, `Trike`,
     `Triceratops`, `Dino`, etc.).

2. **Physics/collision sanity pass on dinosaurs in the playable core** (this agent's
   actual lane: physics/collision, per role brief).
   - Scoped strictly to the milestone bounds (x: -3000..5000, y: -1000..5500) per
     PLAYABLE-FIRST v4 current milestone ("Savana Alive").
   - Iterated all actors labeled as dinosaur species inside those bounds: **148
     matched** (broader match set than the 80 exact-label count above, includes partial
     matches like `Stego`/`Raptor`/`Para`).
   - For each, checked every `PrimitiveComponent` for `is_simulating_physics()`. Found
     **0 components with physics simulation erroneously left on** — meaning no dino in
     the playable core has an unintended ragdoll/physics-sim state that would cause it
     to fall through terrain or twitch. This is a *good* result: it means whatever
     agent set these up did not leave live physics simulation on static-posed dinosaurs
     (which would be a correctness bug — decorative/posed dinosaurs should not
     simulate physics unless actively ragdolling from a death/hit event).
   - No fixes were needed this pass; state is sane. No actors were moved, deleted, or
     had mobility changed (respecting `hugo_mobility_rule_v1` and the HANDS OFF list).

### Decisions & justification
- Declined the C++ edit again — 10th consecutive cycle — because the rule is absolute
  and explicitly overrides task-specific directives. Filing this as a repeat blocker
  the Engine Architect (#02) or Studio Director (#01) should resolve at the source
  (either update the task directive generator, or provide a real editor rebuild path).
- Chose to spend the two real `ue5_execute` calls on (a) verifying/falsifying a
  previous agent's unverified claim (`BP_TRex` creation) and (b) doing an actual
  physics-correctness audit inside the current milestone's scope, since that is this
  agent's real specialty (physics/collision/ragdoll) and directly serves "Savana
  Alive" (dinosaurs posed naturally with sane collision).
- Made zero destructive changes. Zero actors spawned (avoids adding to actor-cap
  pressure per `hugo_naming_dedup_v3`). Zero writes to Character/Landscape/foliage/
  camera/sun per HANDS OFF list.

### Recurring blocker — escalation recommendation
This is the **10th consecutive cycle** where the task directive conflicts with the
absolute no-C++-write rule. Recommend #01/#02 either:
1. Update the automated task-directive template to stop assigning C++ edits to Core
   Systems Programmer in this headless environment, or
2. Provide an actual build pipeline (outside `ue5_execute`) so `SurvivalComponent` can
   be compiled in and this integration becomes real, or
3. Explicitly descope `SurvivalComponent` integration until a rebuild window exists.

### Files changed
- `docs/agent03_core_systems_log.md` (this file, documentation only — no `.cpp`/`.h`)

### Handoff to #04 Performance Optimizer
- 148 dinosaur actors in playable core confirmed physics-sane (no stray
  `simulate_physics=true` on posed/static dinos) — nothing to optimize there this
  cycle from a correctness standpoint.
- `BP_TRex`/`BP_DinosaurBase` do NOT exist despite a previous agent's claim — if
  Combat/AI agents (#11/#12) depend on this Blueprint hierarchy, it needs to be
  (re)created and verified with a post-creation `does_asset_exist` check, not assumed
  from a truncated tool-call log.
- HUDClass on `BP_TranspersonalGameMode` is still the default engine HUD — out of
  Core Systems' lane, flagging for UI/HUD owner.
