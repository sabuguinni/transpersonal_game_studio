# Combat & Enemy AI Agent #12 — Cycle PROD_CYCLE_AUTO_20260719_004

**Bridge: UP.** 4x `ue5_execute` (3x `python`, 1x `run_console_command`), all `completed`, no timeouts (~3.0s each). Zero `.cpp`/`.h` files written (rule `hugo_no_cpp_h_v2` respected). Zero new `CombatZone_*` / `BehaviorTag_*` actors spawned (rule `hugo_combat_label_consistency_v1` respected — dedup-first policy). PLAYER0, Landscape, Terrain_Savana and camera untouched.

## Anti-duplication audit (mandatory per `hugo_combat_label_consistency_v1`)
Attempted a full audit of existing `CombatZone_*` / `BehaviorTag_*` labels within 3500 units of the hub (2100,2400) before creating anything. **Known bridge limitation** (confirmed again this cycle, consistent with #11's report): `ue5_execute` with `command_type=python` always collapses the return value to `{"ReturnValue": bool}` regardless of what the Python script returns (dict, string, int) — real counts and label lists could not be extracted numerically this cycle. Given the standing memory that ~191 `CombatZone_Raptor` + ~185 `BehaviorTag_Raptor` labels already exist near the hub (pure accumulation, not real content), I treated the risk of duplication as **confirmed high** and took the conservative path:

**Decision: created ZERO new `CombatZone_*`/`BehaviorTag_*` actors this cycle.** Instead of adding more label-actor flood, I implemented combat AI state directly as **native Unreal Actor Tags** on the existing `TRex_Savana_*` actors already grounded by NPC Behavior Agent #11 last cycle. This is reuse-first, doesn't inflate the actor count, and is real gameplay-relevant data (not decorative labels).

## What was done in the live world
1. **TRex discovery**: enumerated all actors labeled `TRex*_Savana_*` and the player (`PLAYER0`, read-only, position never modified).
2. **Combat AI parameters as Actor Tags** (idempotent — old `Combat_*` tags stripped before re-adding, so repeated cycles don't stack duplicates):
   - `Combat_Patrol_5000` — patrol radius (matches #11's handoff note on TRex behavior ranges)
   - `Combat_Chase_3000` — chase engagement radius
   - `Combat_Attack_300` — melee attack radius
   - `Combat_State_<IDLE|PATROL|ALERT|CHASE|ATTACK>` — computed live from actual distance between each TRex and PLAYER0 at tagging time
3. **Verification pass**: re-read tags off the same TRex actors in a follow-up call, confirmed `Combat_` tags present (`found_any=True`), confirmed zero new `CombatZone_/BehaviorTag_` actors were created this cycle.
4. **Single save** (`save all`) at the end, after verification — no mid-work blind saves.

## Design rationale (Naughty Dog / Jaime Griesemer influence)
The tag-based state (`PATROL → ALERT → CHASE → ATTACK`) is designed to be read by a Behavior Tree/Blackboard set up via Blueprint (not C++, per absolute rule) in a later cycle: the TRex should telegraph awareness before committing to a chase (ALERT state = visible head-turn/roar cue, giving the player a fair warning window — "the player loses because of a mistake, not because the game was unfair"). The four-tier distance model (5000/3000/300) mirrors #11's TRexBehavior spec from previous cycles, now expressed as real, queryable, persisted actor data instead of a dead `.cpp` file.

## Known technical limitation (recurring, flagged again to #02/#19)
The RC bridge's coercion of all Python return values to boolean makes it impossible to verify exact counts (how many TRex were tagged, exact distances, exact existing zone/tag actor counts) from within a single cycle. All confirmations here are boolean presence checks (`success:true`, `found_any:true`), not numeric audits. This has now been reported by both #11 and #12 in the same cycle — recommend #02/#19 prioritize a fix (e.g. writing structured audit results to a `Saved/*.json` file that a future cycle's Python can `open()` and `unreal.log()` line-by-line, or exposing a raw-string return path in the bridge).

## Handoff to #13 (Crowd & Traffic Simulation)
- TRex actors in the playable core now carry queryable `Combat_State_*` tags reflecting live proximity to the player — useful if crowd/traffic logic needs to route human NPC movement away from `CHASE`/`ATTACK` state predators.
- No new actors were added to the world this cycle — actor cap discipline maintained.
- Recommend #13 read `Combat_State_*` tags rather than recomputing distance itself, to avoid redundant logic.
