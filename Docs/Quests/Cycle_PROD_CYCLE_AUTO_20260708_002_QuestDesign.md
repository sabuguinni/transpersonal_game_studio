# Quest & Mission Designer — Cycle PROD_CYCLE_AUTO_20260708_002

## Bridge Status
HEALTHY — 3/3 `ue5_execute` Python calls completed without timeout (audit → spawn → verify).

## Compliance Note (overrides generic mandate text)
Per `hugo_no_cpp_h_v2` (importance MAX, no exceptions): this agent does **not** write .cpp/.h files.
C++ is inert in this headless editor instance (pre-built binary, no recompilation).
All quest logic this cycle was implemented live via `ue5_execute` (TriggerBox actors + tags),
which is queryable by Blueprints/Remote Control today and can be wired to a future
QuestManager subsystem once compiled code changes are actually deployable.

## What Was Built This Cycle

### 1. Actor & Tag Audit (read-only, no mutation)
Confirmed Agent #13's live herd tags are present and queryable:
- `Crowd_HerdA_Triceratops` — herd cluster near (2500, 2100)
- `Crowd_HerdB_Grazers` — herd cluster near (1500, 2900)
- `Crowd_Behavior_Fleeing` / `Crowd_Behavior_Grazing` — reactive state tags driven by Combat AI threat proximity

### 2. Three New Quest Trigger Zones (TriggerBox actors, label-checked before spawn — zero duplicates)

| Actor Label | Location | Tag | Quest Concept |
|---|---|---|---|
| `Quest_ObserveHerd_Triceratops_001` | (2450, 2050, 100) | `Quest_ObserveHerd` | **"Watch, Don't Hunt"** — Observation-only quest. Player must approach the Triceratops herd without weapons drawn, stay in cover, and log the grazing pattern. Rewards patience over aggression — a deliberate design counterpoint to the game's combat-heavy hunting quests. |
| `Quest_StampedeHazard_001` | (2200, 2200, 100) | `Quest_StampedeHazard` | **"Ground Shakes"** — Environmental hazard/warning zone sitting between the herd and the player hub. When `Crowd_Behavior_Fleeing` flips true on HerdA members (set by Agent #13 from Combat AI predator-proximity state), this zone becomes an active danger corridor — the player must relocate or be caught in a stampede. This converts Agent #13's reactive crowd tag into a concrete, readable-by-player game event. |
| `Quest_TrackGrazers_HerdB_001` | (1600, 2800, 100) | `Quest_TrackGrazers` | **"Follow the Herd"** — Migration-tracking quest. Player follows Herd B toward water/forage, logging 3 resting points along the trail. Rewards exploration and observation of ecology (per GDD realism mandate — no mystical tracking, just following visible animal movement). |

All three were spawned only after confirming their labels did not already exist in the level (naming-dedup rule honored — zero duplicate actors introduced).

### 3. Emotional Arc Design Rationale (Pawel Sasko / Jonathan Blow lens)
- **"Watch, Don't Hunt"** exists specifically because every other quest in the current pipeline is kill/gather. A quest whose *mechanic* is "put your weapon away" is itself an affirmation about the game world: not every dinosaur encounter has to end in violence. Completing it should feel like restraint rewarded, not a mechanical checkbox.
- **"Ground Shakes"** reuses Agent #13's live threat-state signal as narrative tension instead of inventing a new scripted trigger — the danger is real and computed, not staged. This keeps player trust: the world reacts because of what's actually happening (a nearby predator), not a hidden script.
- **"Follow the Herd"** ties directly to survival mechanics (water/forage discovery) so the emotional payoff (successfully tracking a herd to a resource) also has mechanical value — no reward is purely narrative fluff.

### 4. Voice Lines Generated (text_to_speech)
- `QuestGiver_Tracker` — briefing line for "Watch, Don't Hunt," instructing the player to observe the Triceratops herd without engaging and warns about stampede risk.
- `QuestGiver_Elder` — briefing line for "Follow the Herd," directing the player to track Herd B toward the river and log three resting points.
(Note: audio upload to Supabase Storage returned a 403/Invalid JWS on this run — synthesis itself succeeded; storage layer needs a token refresh on the platform side. Flagging for #16 Audio Agent / infra owner.)

### 5. Verification Pass
Re-queried the level for all three quest actors post-spawn: confirmed labels, positions, and tags match spec. Total `Quest_*` actors in map counted for handoff tracking.

## Deliverables Summary
- [UE5_CMD] Actor/tag audit of HerdA/HerdB/Fleeing state (read-only)
- [UE5_CMD] Spawned 3 TriggerBox quest zones (ObserveHerd, StampedeHazard, TrackGrazers), dedup-checked, level saved
- [UE5_CMD] Verification pass confirming all 3 actors + tags in map
- [TTS] 2 quest-giver voice lines generated (Tracker + Elder briefings) — storage upload failed (infra issue, not agent error)
- [DOC] This file

## Known Limitation
No `heygen_create_video` tool was available in this session's toolset despite being named in the mandate — cannot produce a "quest walkthrough video" without it. Recommend the walkthrough be captured via UE5 in-editor sequencer/screenshot pipeline (Agent #17 VFX or Agent #19 Integration) once camera-safe tooling exists (per `hugo_no_camera_v2`, this agent will not touch the viewport camera itself).

## Handoff to #15 (Narrative & Dialogue Agent)
- Three quest concepts now have concrete world placement and mechanical triggers: `Quest_ObserveHerd`, `Quest_StampedeHazard`, `Quest_TrackGrazers`.
- Need full dialogue trees for `QuestGiver_Tracker` and `QuestGiver_Elder` NPCs — voice line drafts above are placeholders for tone/pacing, not final VO.
- Suggest Narrative Agent confirm these NPCs' names/backstory fit the Bible before final voice lines are locked.

## Handoff to #16 (Audio Agent)
- TTS synthesis succeeded but Supabase storage upload failed (403 Invalid JWS) — needs an infra fix, not a re-run of synthesis.

## Handoff to Next Quest Design Cycle
- Wire `Quest_StampedeHazard_001` to actually read `Crowd_Behavior_Fleeing` live (currently a static snapshot trigger volume; needs a Blueprint OnTick or event binding for real-time reactivity once compiled code can be deployed).
- Consider a 4th quest once Agent #12's dinosaur combat AI stabilizes: an ambush/ranged-hunt quest using `CombatAI_State_*` tags directly.
