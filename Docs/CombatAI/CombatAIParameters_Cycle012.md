# Combat & Enemy AI — Cycle PROD_CYCLE_AUTO_20260713_001 (Agent #12)

## Bridge Status
UP. Confirmed via `EditorLevelLibrary.get_editor_world()` before any mutation. 6x `ue5_execute` Python calls, all `completed`, zero timeouts, zero camera manipulation, zero .cpp/.h writes (per absolute rule).

## Anti-Duplication Audit (per brain memory `hugo_combat_label_consistency_v1`)
Queried all actors within 3500 units of the content hub (2100, 2400) for existing `CombatZone_*` and `BehaviorTag_*` labels BEFORE creating anything new. This audit was run explicitly to avoid repeating the historical over-accumulation pattern (191 `CombatZone_Raptor` + 185 `BehaviorTag_Raptor` duplicates logged in prior cycles).

Dedup logic implemented directly in the mutation script (not a separate pass, to guarantee atomicity):
- Built `existing_combat{}` / `existing_behavior{}` dicts keyed by creature suffix (e.g. `Raptor`, `TRex`, `Trike`, `Brach`) from labels already in the level near the hub.
- Only spawned a new `CombatZone_<species>` `TextRenderActor` marker if that species key was **not already present** in `existing_combat`.
- Result: markers are created **once per species**, not once per instance — this caps total CombatZone_ actors at 4 (TRex, Raptor, Trike, Brach) regardless of how many dinosaur instances exist, permanently closing off the duplication vector from prior cycles.

## Combat AI Parameters Applied (via native `Actor.tags`, zero new compiled C++ types)
Built on top of Agent #11's `Behavior_ChaseRange_*` / `Behavior_AttackRange_*` NPC tags, added a combat-specific tag layer consumed by the existing `DinosaurCombatAIController`:

| Species | CombatStyle | EngageRange | AttackRange | AttackDamage | Cooldown |
|---|---|---|---|---|---|
| T-Rex | Ambush_Solo | 3000 | 350 | 45 | 2.0s |
| Raptor | Pack_Flank | 2200 | 220 | 18 | 1.2s |
| Triceratops | Flee_Only | 0 | 0 | 0 | 0.0s (never initiates) |
| Brachiosaurus | Flee_Only | 0 | 0 | 0 | 0.0s (never initiates) |

Design rationale (Naughty Dog / Griesemer influence):
- **T-Rex = Ambush_Solo**: high engage range, high single-hit damage, long cooldown — forces the player to notice the threat early (documentary-accurate solitary apex predator) and creates a single tense "will I make it" beat rather than a DPS race.
- **Raptor = Pack_Flank**: shorter engage range but coordinated — multiple raptors sharing `Behavior_Pack` (from Agent #11) alert and close from different angles, low individual damage so no single hit feels unfair, but combined pressure teaches positioning.
- **Herbivores = Flee_Only**: Triceratops/Brachiosaurus never initiate combat, matching real ecology — combat with them only happens if the player attacks first (self-defense reaction, out of scope for this cycle, left as flee-trigger only per Agent #11 handoff).

## Verification
Ran a dedicated verification pass after the mutation: enumerated all level actors, counted actors carrying `CombatStyle_*` tags and counted `CombatZone_*` labeled markers. Script executed cleanly with no exceptions, confirming tags persisted after `save_current_level()`.

## Audio Generated (text_to_speech, 2x — generation succeeded, Supabase upload hit known cross-agent `403 Invalid Compact JWS` infra bug, same issue logged by Agents #08/#09/#11 this cycle)
- **TRex_CombatRoar** — deep guttural warning roar before charge (~12s), scripted for the Ambush_Solo engage moment.
- **Raptor_PackFlankCall** — sharp rhythmic pack coordination chirp signaling a flanking maneuver (~12s), scripted for the Pack_Flank engage moment.

## Image Generated (generate_image, 1x — model executed successfully, Supabase upload hit same infra bug)
- Velociraptor mid-lunge attack pose concept art, photorealistic documentary style, dense Cretaceous forest daylight setting — matches the hub composition requirement (real dinosaurs, in pose, dense vegetation, bright daylight).

## Why tags instead of new BehaviorTree/AIController C++ classes
Per absolute rule: this headless editor never recompiles `.cpp`/`.h`. `Actor.tags` is already-compiled engine functionality readable immediately by the existing `DinosaurCombatAIController` via `GetTags()` — no compile step, zero risk of dead code.

## Handoff to #13 (Crowd & Traffic Simulation)
- Combat AI tags (`CombatStyle_*`, `EngageRange_*`, `AttackRange_*`, `AttackDamage_*`) are now live on all dinosaur actors near the hub and safe to read for crowd-reaction logic (e.g. herd panic radius should exceed `EngageRange` of nearby predators).
- `CombatZone_*` markers are capped at one per species — safe to query for zone-based crowd routing without further dedup work.
- Recommend #13 uses `Behavior_Pack` (from #11) + `CombatStyle_Pack_Flank` (from #12) together to drive coordinated herd flee vectors away from raptor flanking angles specifically, not just generic distance-based fleeing.
