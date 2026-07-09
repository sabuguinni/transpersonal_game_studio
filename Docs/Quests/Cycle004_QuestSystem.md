# Quest & Mission Designer — Cycle PROD_CYCLE_AUTO_20260709_004

## Summary
Following the hard rule `hugo_no_cpp_h_v2`, no .cpp/.h files were written this cycle.
All quest logic was implemented live in `MinPlayableMap` via `ue5_execute` Python,
using `TriggerSphere` actors tagged with quest metadata (readable/settable via
Remote Control without needing new C++ classes).

## Quests Placed (MinPlayableMap, near content hub X=2100 Y=2400)

### 1. Quest_TrackHerd_Triceratops_001 — Tracking Mission
- **Location:** (2100, 2400, 100) — directly on the Triceratops herd anchor formed by Agent #13.
- **Type:** `QuestType_Tracking`
- **Objective:** Observe the herd for 60 seconds without triggering aggro.
- **Reward:** `QuestReward_XP_50`
- **Fail condition:** `QuestFailCondition_HerdAggro` (approach too close / bull charges).
- **Design intent:** Teaches the player predator-avoidance behavior around large herbivores —
  a Triceratops bull is dangerous when defending calves, even though the species is not a predator.
  Emotional arc: caution → patience → the reward of watching wild animal behavior up close.

### 2. Quest_GatherResources_001 — Crafting Gather Mission
- **Location:** (1900, 2200, 100) — hub clearing, near scattered resource pickups.
- **Type:** `QuestType_Gathering`
- **Objectives:** Collect 2 rocks, 1 stick (feeds Agent #14's Stone Axe recipe from the Crafting directive).
- **Reward:** `QuestReward_Item_StoneAxeRecipe`
- **Design intent:** Directly ties to the Crafting System mandate (Stone Axe = 2 rocks + 1 stick).
  Given by an NPC crafter under camp-defense pressure ("the raptors come back at night") — stakes
  are survival-practical, not abstract fetch-questing.

### 3. Quest_ObserveMigration_Brachio_001 — Observation Mission
- **Location:** (3000, 1700, 100) — matches Agent #13's Brachiosaurus pair (hub + 900E, -700S).
- **Type:** `QuestType_Observation`
- **Objective:** Observe for 45 seconds.
- **Reward:** `QuestReward_XP_30`
- **Design intent:** Low-stakes ecological-discovery quest — establishes migration/herd behavior
  as a readable world signal (per GDD ecology pillar), no combat risk.

## Tag Namespaces (for other agents to reference, not re-tag)
- `QuestType_*` — Tracking / Gathering / Observation
- `QuestState_*` — currently all `QuestState_Available`
- `QuestGiver_*` — currently `QuestGiver_None` (no NPC dialogue hookup yet — blocker below)
- `QuestReward_*`, `QuestObjective_*`, `QuestFailCondition_*` — per-quest metadata

## Voice Lines Generated (text_to_speech)
1. **QuestGiver_Tracker** — briefing line for Quest_TrackHerd_Triceratops_001:
   > "You've been tracking that Triceratops herd for two days now. Don't get too close —
   > the bull will charge if he thinks you're after the calves. Just watch, count them,
   > learn their grazing pattern. That's how you survive out here."
2. **QuestGiver_Crafter** — briefing line for Quest_GatherResources_001:
   > "We're low on cordage and dry tinder. If you can bring back rocks for striking,
   > straight sticks for shafts, and broad leaves for binding, I can finish the stone axe
   > before nightfall. The camp needs it before the raptors come back."

Both lines generated successfully via ElevenLabs TTS. Note: the Supabase storage upload
returned a 403 (Invalid Compact JWS) on this run — audio was synthesized (base64 MP3
returned in the tool payload) but the public URL was not persisted. Flagging for
Agent #16 (Audio) / infra to re-check the storage JWT before final voice pass.

## Blocker / Not Actionable This Cycle
- **Quest walkthrough video:** No `heygen_create_video` tool is available in this agent's
  toolset (only `text_to_speech`, `generate_image`, `ue5_execute`, `github_*`). Cannot produce
  a video deliverable — recommend routing this requirement to whichever agent/tool profile
  actually has video generation access, or dropping it from the Quest Designer mandate.
- **NPC dialogue hookup:** Quests currently have `QuestGiver_None` — no NPC actor exists yet
  to hold a `DialogueID` linking to the two voice lines above. This is Agent #15's
  (Narrative & Dialogue) and Agent #11's (NPC Behavior) territory — once an NPC actor with
  a dialogue component exists near the hub, tag it `QuestGiver_Tracker` / `QuestGiver_Crafter`
  and replace `QuestGiver_None`.
- **Species variety:** Per Agent #13, only Triceratops and Brachiosaurus exist as herd species —
  limits quest diversity (no predator-hunt-tracking quest possible yet without a huntable
  species that isn't already tagged `CombatRole_*` by Agent #12). Flagged upstream, not
  actionable here.

## For Agent #15 (Narrative & Dialogue Agent)
- Two quest-giver voice lines above are ready to be would-be attached to NPC dialogue trees.
- Quest fiction is grounded in survival stakes (herd safety, camp defense before nightfall) —
  keep dialogue tone practical/primitive, no mystical framing, consistent with anti-hallucination rule.
- Quest trigger actors + tags are the ground truth for objective completion checks; no new
  Quest Manager C++ class was created (per `hugo_no_cpp_h_v2`) — dialogue/quest state should
  read actor tags via Remote Control / Python, not a new subsystem header.
