# Combat & Enemy AI — Cycle PROD_CYCLE_AUTO_20260710_006

## Agent #12 — Combat & Enemy AI Agent

### Bridge Status: OK
All 4 `ue5_execute` Python calls succeeded against the live `MinPlayableMap` (3-15s each).

## Production Actions (real, live UE5 edits)

### 1. Bridge Validation
Confirmed editor world loaded and Remote Control responsive before making changes.

### 2. Tactical Combat AI Tagging (no duplication)
Audited all dinosaur actors in the content hub area (X=2100, Y=2400) placed by prior agents
(#09 Character Artist, #11 NPC Behavior). Instead of spawning duplicate AI-specific actors
(the exact anti-pattern flagged in project memory — e.g. `Trike_QuestArea_001_AI` stacking on
existing `Trike_*`), this cycle **reused existing actors by label lookup** and applied Actor Tags
to drive Behavior Tree / Blackboard state without new geometry:

| Species Prefix | Tags Applied | Design Intent |
|---|---|---|
| `TRex_*` | `CombatAI`, `State_Idle`, `AggroRadius_2500`, `Apex_Predator` | Solo ambush predator, large detection radius, high burst damage, low pursuit stamina — forces player to break line of sight rather than out-run it. |
| `Raptor_*` | `CombatAI`, `State_Idle`, `AggroRadius_1800`, `Pack_Hunter` | Coordinated flanking behavior — 3 raptors from #11's handoff now share a pack tag so a future Behavior Tree can implement flank/distract/strike roles. |
| `Trike_*` | `CombatAI`, `State_Idle`, `AggroRadius_800`, `Defensive_Herbivore` | Non-aggressive unless cornered; short aggro radius, charges only when player is within melee range of its territory — teaches "not all dinosaurs are threats" pacing. |

This creates the tagging foundation for a future Behavior Tree combat state machine
(`State_Idle` → `State_Alert` → `State_Attack` → `State_Retreat`) without needing new C++
classes in this headless editor (C++ changes do not recompile here per project rule).

### 3. Verification Pass
Re-queried all actors carrying the `CombatAI` tag and logged their distance to the content hub
center (2100, 2400) plus their full tag list, confirming tagging persisted correctly in the live
level (not just in the transient Python session).

### 4. Tactical Encounter Volumes
Spawned 3 new `TriggerBox` actors (`CombatEncounter_Hub_001/002/003`) around the hub clearing at
(2400,2600), (1800,2200), (2100,2700) — positioned to bracket the existing dinosaur cluster from
three angles. Tagged `CombatEncounterZone` + `TacticalAmbush`. These are lightweight, engine-native
trigger volumes (no custom C++) intended to later drive:
- Player-proximity aggro escalation (raptor pack encirclement)
- Ambush ecology moments (T-Rex breaking cover when player lingers)
- Camera/audio cue triggers for tension building (Naughty Dog-style "enemy notices you" beats)

Level was saved after all changes (`EditorLevelLibrary.save_current_level()`).

## Asset Generation Attempted
- **Concept art** (Velociraptor pack hunting formation, National Geographic documentary style):
  generation succeeded but **Supabase storage upload failed (HTTP 403 — Invalid Compact JWS)**.
  This is an infrastructure auth issue unrelated to prompt content; flagging for Hugo/orchestrator
  to check Supabase storage token expiry.
- **T-Rex combat vocalization** (growl/hiss/roar SFX cue): ElevenLabs TTS generation succeeded,
  same storage upload 403 failure. Raw audio was generated but not persisted to a public URL this
  cycle.

## Technical Decisions & Justification
- Chose **Actor Tags over new Actor spawns** to encode AI combat state — respects the project's
  explicit anti-duplication rule and keeps the hub readable in the hero screenshot composition
  (dense forest + recognizable dinosaurs, no extra AI-marker clutter).
  the underlying UCLASS work (BehaviorTree assets, BTTask/BTService nodes) requires either
  Blueprint-only setup via Python (feasible next cycle) or C++ that cannot compile in this headless
  editor — deferred to keep this cycle's changes 100% live and verifiable.
- Trigger volumes use stock `AEngine.TriggerBox` (rule-compliant: reuse existing UE5 classes,
  no custom actor classes needed for this pass).

## Dependencies / Inputs Needed
- From **#11 NPC Behavior Agent**: confirmation of Behavior Tree asset location (if any exists)
  so combat states can hook into shared Blackboard keys instead of a separate combat-only tree.
- From **Hugo/Infra**: fix Supabase storage JWT/token so generated concept art & SFX persist.

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Tagged 5+ existing dinosaur actors (TRex/Raptor/Trike) with CombatAI state tags and species-specific aggro radii — no duplicate actors created.
- [UE5_CMD] Verified tag persistence and logged distances-to-hub for all CombatAI-tagged actors.
- [UE5_CMD] Spawned 3 TriggerBox `CombatEncounter_Hub_00X` tactical ambush volumes bracketing the hub clearing, saved level.
- [ASSET] Concept art + T-Rex combat SFX generated but blocked by storage 403 (infra issue, flagged).
- [FILE] This documentation file (Docs/CombatAI/Cycle_006_TacticalCombatAI.md).
- [NEXT] #13 Crowd & Traffic Simulation should build pack-coordination logic on top of the `Pack_Hunter` tag on Raptor actors; a future Combat AI cycle should convert `State_Idle`/`AggroRadius_*` tags into an actual Blueprint Behavior Tree once BT asset location is confirmed by #11.
