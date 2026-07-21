# NPC Narrative Identities — Cycle 006
Agent #15 — Narrative & Dialogue Agent

## Purpose
Give the 4 quest-giver triggers spawned by Agent #14 this cycle concrete voice,
backstory, and personality. Tags applied live in `MinPlayableMap` via
`ue5_execute` (actor.tags on each TriggerSphere) so downstream agents (#16 Audio,
#18 QA) can look up NPC identity by actor tag without a separate database.

## NPC Roster

### Kael — Hunter Elder
- **Tag applied to:** `Quest_TrackingHunt_Raptor_001`
- **Role:** Veteran tracker, survived multiple raptor encounters. Teaches
  practical wind-awareness and pack-hunting danger, not mysticism.
- **Voice line (generated, ElevenLabs, upload failed — see Infra Flag below):**
  > "Name's Kael. Been tracking that raptor pack for six days. They're not
  > stupid — they'll circle downwind of you if you let them. Stay near the
  > rocks, keep your spear ready, and never take your eyes off the tall grass.
  > That's rule one out here."
- **Narrative function:** Establishes raptor pack behavior as a known, studied
  threat (ties to #12 Combat AI flanking behavior) rather than a mysterious
  force. Reinforces realism.

### Nera — Camp Forager
- **Tag applied to:** `Quest_HerdObservation_001` AND `Quest_ResourceGathering_Flint_001`
  (same NPC gives both quests — reduces NPC sprawl per naming/dedup rule)
- **Role:** Resource specialist, calm and pragmatic, respects herd space.
- **Voice lines (generated):**
  > "Flint ridge is good today — the herd's grazing far enough away we can
  > work without spooking them. Fill your pouch, but move slow. A
  > Brachiosaurus doesn't care about you, but a startled one can flatten a
  > camp without even trying."
- **Narrative function:** Bridges the "do no harm" observation quest and the
  "gather flint" quest under one consistent voice — teaches the player that
  herbivore megafauna are a hazard-by-accident, not a monster.

### Orun — Tribal Leader
- **Tag applied to:** `Quest_CampDefense_001`
- **Role:** Leader of the home camp. Grants the player standing/belonging
  based on demonstrated competence, not any spiritual awakening.
- **Voice line (generated):**
  > "You came back. Good. Most don't, not after their first real hunt. Sit.
  > Eat. Tomorrow you start earning your place at this fire, not just a spot
  > to sleep."
- **Narrative function:** Closes the loop on Cycle 006's 4 quests — reframes
  "defend the camp" as the moment the player transitions from lone survivor
  to tribe member. This is the emotional beat McKee-style pressure point:
  competence under threat earns belonging.

## Supporting Lore (world narration, not tied to a specific trigger)
> "This valley used to belong to no one. Then the first families came down
> from the highlands, chasing the herds south before the cold. We stayed
> because the river never freezes here. That's the only reason any of us are
> alive to argue about it."

This line establishes the tribe's founding pressure (climate + food scarcity),
consistent with realistic survival narrative — no mystical origin, no
"chosen one" framing.

## Cross-Agent Vocabulary Used
- Reused #14's TriggerSphere quest actors and tag vocabulary (`QuestType_*`,
  `QuestTarget_*`) rather than creating parallel narrative-only markers.
- Reused #12/#13's predator/herd perception tags for consistency when writing
  Kael and Nera's dialogue (their warnings match actual AI behavior, not
  invented lore).

## Implementation
- 2 `ue5_execute` Python calls this cycle:
  1. Bridge validation + audit of existing `Quest_*` / `Narr_*` / `NPC_*`
     actors in `MinPlayableMap` (zero duplicates found before tagging).
  2. Applied 4 NPC identity tags to the existing quest trigger actors via
     `actor.tags` and saved the level (`save_current_level`).
- No new actors spawned — per `hugo_naming_dedup_v2`, NPC identity was
  attached to Agent #14's existing triggers instead of spawning duplicate
  "Quest_Narrative_001" style actors.
- No `.cpp`/`.h` files touched — headless editor never recompiles C++ per
  `hugo_no_cpp_h_v2`. All narrative logic lives as live tagged actor data,
  readable by any agent via `get_all_level_actors()`.

## Infra Flag (carried forward from #14, confirmed again this cycle)
All 4 `text_to_speech` calls this cycle succeeded at generation but failed at
Supabase Storage upload:
```
403 Unauthorized — Invalid Compact JWS
```
This is the second consecutive cycle with this exact failure (also hit by
Agent #14). Confirmed studio-wide, not agent-specific. Audio data (base64 MP3)
was generated successfully and is embedded in the tool call result payload if
manual recovery is needed. Recommend #01/#19 rotate/refresh the Supabase
storage token before next cycle.

## Dependencies for #16 Audio Agent
- 4 dialogue lines with defined speaker identity (Kael, Nera x2, Orun) ready
  for MetaSounds implementation once storage token is fixed.
- 1 ambient narration line (Valley Lore) suitable for a background/intro
  audio moment near the hub.
- Recommend #16 re-run TTS generation once storage infra is confirmed fixed,
  using the scripts above verbatim to keep continuity with tags already
  placed on the triggers.
