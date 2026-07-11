# Hub Quest Triggers — Cycle PROD_CYCLE_AUTO_20260711_005 (Agent #14 Quest & Mission Designer)

## Context
Building on Agent #13's herd formation work this cycle (`Herd_HubGrazing_01` cluster near 2500,2100),
three quest trigger volumes were placed live in `MinPlayableMap` via `ue5_execute` Python (idempotent —
each checked for an existing actor with the same label before spawning, per `hugo_naming_dedup_v2`).

## Quests Implemented (TriggerSphere actors, live in world)

### 1. Quest_ObserveHerd_001 — Observation Mission
- **Location:** (2500, 2100, 100), radius 700
- **Tags:** `Quest_ObserveHerd`, `QuestType_Observation`
- **Design intent:** References the herbivore herd formed by Agent #13. Objective: approach quietly,
  observe grazing behavior without triggering flight response. Rewards patience over aggression —
  survival-realistic, not combat-gated. Fails the "documentary test": this is what a wildlife
  observer would actually do near a wild herd.
- **Emotional arc:** Tension (proximity to large animals) → stillness → quiet reward (learning
  migration/grazing patterns feeds into future Migration quests).

### 2. Quest_TrackRaptor_001 — Hunt/Tracking Mission
- **Location:** offset +200/+150 from nearest existing Raptor actor (fallback 2800,2600 if none found), radius 600
- **Tags:** `Quest_TrackRaptor`, `QuestType_Hunt`
- **Design intent:** Track and observe a pack predator without being detected — a precursor mission
  to an eventual ambush/ranged-weapon hunt quest. Tests player stealth/positioning against a real
  territorial predator, not a scripted "boss."

### 3. Quest_GatherResources_001 — Crafting Mission
- **Location:** (1900, 2200, 100), radius 500, near existing resource pickups (rocks/sticks/leaves) spawned in prior cycles
- **Tags:** `Quest_GatherResources`, `QuestType_Crafting`
- **Design intent:** Directly ties into the Stone Axe / Campfire / Water Container recipes already
  scattered near the hub. Objective: gather enough rocks, sticks, and leaves to complete one full
  recipe before nightfall (day/night pressure creates urgency without artificial timers).

## Voice Lines Generated (text_to_speech)
Two quest-giver lines were generated (audio storage upload failed with a 403/Invalid JWS error on
the backend — this is an infrastructure issue on the storage side, not a content issue; raw MP3
bytes were successfully synthesized by ElevenLabs in both cases):
1. **CampElder_QuestGiver** — introduces the Observe the Herd quest, sets tone of caution and respect
   for wildlife, foreshadows the tracking/hunt quest.
2. **CraftingMentor_QuestGiver** — introduces the Gather Resources quest, references the specific
   materials (stone, sticks, leaves) needed for crafting recipes established by Agent #14/#03 in
   prior cycles.

## Anti-hallucination compliance
All three quests and both voice lines are grounded in survival mechanics (observation, stealth
hunting, resource gathering) — zero spiritual/meditation/mystical content, consistent with the
project's anti-hallucination rule.

## Next agent (#15 Narrative & Dialogue Agent) should focus on
- Write full dialogue trees for `CampElder_QuestGiver` and `CraftingMentor_QuestGiver` NPCs,
  anchoring them to physical NPC actors near the hub (currently only trigger volumes exist, no
  NPC pawns yet — that's a gap for #09/#11 to fill with an actual NPC actor at the hub).
- Confirm whether `Quest_TrackRaptor_001`'s attached Raptor actor is a live AI-controlled pawn
  or a static placeholder — hunt quest logic will differ significantly depending on this.
- Audio storage 403 error should be flagged to infrastructure/Agent #19 for the ElevenLabs→Supabase
  upload pipeline (JWS signature issue), independent of quest content quality.
