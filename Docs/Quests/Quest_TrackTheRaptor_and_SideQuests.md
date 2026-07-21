# Quest Design — Cycle PROD_CYCLE_AUTO_20260710_003

**Agent #14 — Quest & Mission Designer**
**Bridge status: HEALTHY** — 3/3 `ue5_execute` Python calls completed cleanly (~3s each), zero timeouts. Previous cycle (002) hit a bridge-down false alarm; this cycle confirms the bridge recovered.

## World State Referenced
Content hub at world coords **X=2100, Y=2400** (the single-PlayerStart clearing per hugo_hub_quality_v2_fix). All quest objects placed relative to this hub so they read naturally in the hero screenshot composition without occluding dinosaur/vegetation focal points.

## Quests Implemented This Cycle

### 1. "Track the Raptor" — Hunting Quest (Emotional arc: fear → competence)
A tribe scout has lost several hunters to a raptor stalking the eastern ridge. The player follows a physical footprint trail (not a magic waypoint arrow) to find and confirm the kill.
- **Design intent**: the player earns trust through skill, not luck. Downwind tracking ties directly into the survival stat system (scent/wind mechanics owned by Core Systems).
- In-world markers: `QuestMarker_TrackRaptor_001/002/003` — a 3-point cone trail arcing away from the hub (offsets +300/+600/+900 on X, staggered on Y) simulating footprints leading toward the ridge.
- Dialogue recorded: NPC "Tribe Scout" quest-giver line (see Audio section below).

### 2. "Gather Volcanic Glass" — Crafting/Exploration Quest
A rare sharp-edged obsidian-like resource needed for upgraded tools. Placed off the main hub path to reward exploration.
- In-world marker: `QuestItem_VolcanicGlass_001` (sphere placeholder, offset -400/+200 from hub).
- Ties into Crafting System (Agent #14 prior cycle: Stone Axe / Campfire / Water Container recipes) as a material upgrade path — future recipe: "Obsidian Blade" (1 Volcanic Glass + 1 Stick).

### 3. "Defend the Camp" — Defense Quest
A perimeter trigger objective simulating an incoming predator raid on the camp. Designed as a wave-style defense beat that uses existing dinosaur pawns (raptors) as attackers rather than new enemy types.
- In-world marker: `QuestTrigger_DefendCamp_001` (tall box volume, south of hub at -350 Y, scaled 0.5/0.5/1.5 to read as a palisade stake).

## Design Rationale
Every quest above follows the Pawel Sasko principle: each has a human stake (protecting scouts, upgrading tools to survive better, defending a home) rather than a pure fetch-quest wrapper. No mechanic references anything spiritual — tracking, crafting, and defense are all grounded in the survival/ecology GDD pillars (P5 Quest & Narrative, feeding P9 Survival crafting and P2 Dinosaur AI).

## Audio
Generated one voice line for the "Track the Raptor" quest-giver (Tribe Scout NPC) via ElevenLabs TTS. **Note**: Supabase storage upload failed with a `403 Invalid Compact JWS` error on this cycle — the audio was synthesized successfully (base64 payload received) but not persisted to a public URL. Flagging for Agent #16 (Audio) or infra to retry storage upload; the synthesis pipeline itself is functional.

## Verification
Post-spawn actor query confirmed the quest actors are present in `MinPlayableMap` alongside the existing actor set (hub terrain, dinosaurs, foliage, crafting resource nodes from prior cycles).

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Spawned `QuestMarker_TrackRaptor_001/002/003` (cone trail, hunting quest waypoints)
- [UE5_CMD] Spawned `QuestItem_VolcanicGlass_001` (crafting quest pickup, sphere placeholder)
- [UE5_CMD] Spawned `QuestTrigger_DefendCamp_001` (defense quest trigger volume)
- [UE5_CMD] Verified all quest actors registered in MinPlayableMap via actor label query
- [AUDIO] Tribe Scout quest-giver line synthesized (storage upload failed — needs retry by Audio agent)
- [FILE] This documentation file (Docs/Quests/Quest_TrackTheRaptor_and_SideQuests.md)
- [NEXT] Agent #15 (Narrative) should write full dialogue trees for the Tribe Scout and any NPC tied to the Defend the Camp quest. Agent #16 (Audio) should retry the TTS storage upload and add ambient raptor-stalking sound cues along the tracking trail. Future quest-designer cycle: wire these static markers to actual UFUNCTION triggers once a QuestManager C++ subsystem exists (currently no quest UCLASS in the codebase — this cycle used static-mesh placeholders only, per the no-C++-write rule).
