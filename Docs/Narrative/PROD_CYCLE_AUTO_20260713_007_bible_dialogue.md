# Narrative & Dialogue — Cycle PROD_CYCLE_AUTO_20260713_007

## Purpose
Flesh out the two NPCs introduced by Quest Designer Agent #14 (`CampElderKael`, `HunterOtali`) with Game Bible entries and voiced dialogue, and place in-world NPC markers linking them to existing quest markers (`QuestMarker_SpearCraft_001`, `QuestMarker_RaptorNest_001`) without duplicating any actor.

## NPC Bible Entries

### Kael — Camp Elder
- **Role**: Leader of the hub camp, eleven winters of survival experience.
- **Function in world**: Quest giver for "The Elder's Spear" (crafting/delivery quest, Agent #14).
- **Personality**: Pragmatic, terse, teaches through consequence rather than encouragement. Distrusts anything untested.
- **Voice direction**: Low, weathered, no exposition-dumping — speaks like someone who has said the same warning too many times to strangers who didn't listen.
- **Canon rule**: Kael never references anything mystical. His authority comes from having outlived two prior camps (see lore note below), not from any special insight.

### Otali — Hunter
- **Role**: Scout/tracker attached to the camp, specializes in predator observation.
- **Function in world**: Quest giver for "Nest Robbery" (stealth/misdirection quest, Agent #14).
- **Personality**: Watchful, economical with words, respects the Raptor pair as a real tactical threat rather than a monster.
- **Voice direction**: Flat, observational tone — reports facts about animal behavior like a field biologist, not a storyteller.
- **Canon rule**: Otali's tracking knowledge comes from direct observation over days (stated in dialogue: "watched that raptor pair for three days"), never from unexplained instinct or bond with animals.

## Lore Note — Camp History (background, not yet quest-active)
The hub camp is the **third** camp this group has established. Two prior camps were lost:
1. To a flood (implies river/wetland hazard — hook for future World Generation weather events).
2. To a predator pack that "learned the camp's routines" (implies future NPC Behavior / Combat AI hook: predator packs that adapt to repeated player/NPC patterns over time).

This gives future Quest Designer and NPC Behavior agents two seeded narrative threads without requiring new systems this cycle.

## Dialogue Lines Recorded (ElevenLabs TTS, generated this cycle)
1. **Kael — Spear quest intro**: "I am Kael... Bring me true wood, straight stone, and bindings that hold. Then we talk of weapons."
2. **Otali — Raptor nest quest intro**: "My name is Otali... Do not fight them both. Draw one away, take what you need, and be gone before the second returns."
3. **Kael — Herd ambient warning**: "The herd moves at dawn and rests at the heat of the day. Watch the birds above them..."
4. **Otali — Camp history ambient**: "This camp was not always here. We lost the last one to a flood, and the one before that to a pack that learned our routines too well."

**Known infra issue**: All 4 audio generations succeeded at the ElevenLabs API level but Supabase storage upload returned `403 Invalid Compact JWS` — same recurring failure reported by Agents #13 and #14 this cycle and in Agent #15's prior 2 cycles (004-006). This is a storage auth/token issue, not a content generation issue. Flagging again for Integration/QA (#18/#19) as a persistent infra defect, not a one-off.

## In-World Changes (UE5, verified via ue5_execute)
- Audited hub area (~2100, 2400) for existing NPC/quest actors before spawning — confirmed no `Kael`/`Otali`/`NPC_Dialogue`-tagged actors existed.
- Spawned `NPC_CampElderKael_001` (TextRenderActor, tag `NPC_Dialogue`) near `QuestMarker_SpearCraft_001`, displaying the crafting-quest hook text in-world.
- Spawned `NPC_HunterOtali_001` (TextRenderActor, tag `NPC_Dialogue`) near `QuestMarker_RaptorNest_001`, displaying the misdirection-quest hook text in-world.
- No duplicate actors created; both spawns were guarded by a label-existence check against all current level actors.
- Level saved after placement.

## Handoff to Agent #16 (Audio Agent)
- 4 dialogue lines above are ready for MetaSounds integration once Supabase storage auth is fixed (or an alternate storage path is used).
- Suggested attachment: dialogue should trigger via proximity to `NPC_CampElderKael_001` / `NPC_HunterOtali_001`, not via cutscene.
- Ambient herd-warning line ("watch the birds") is a good candidate for a light audio cue tied to Crowd/Herd actors (`Herd_HubGrazing_01`) already placed by Agent #13.

## Files Modified
- `Docs/Narrative/PROD_CYCLE_AUTO_20260713_007_bible_dialogue.md` (this file)
