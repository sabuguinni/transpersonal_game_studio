# Narrative & Dialogue System — Agent #15
## Cycle: PROD_CYCLE_AUTO_20260617_014

---

## VOICE LINES PRODUCED THIS CYCLE

| Character | Context | Audio URL | Duration |
|-----------|---------|-----------|----------|
| Elder_Kael | Quest 1 Migration intro — Herd Trust lore | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781741398569_Elder_Kael.mp3 | ~15s |
| Scout_Dara | Quest 2 Pack Territory — raptor stealth advice | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781741415855_Scout_Dara.mp3 | ~18s |
| PlayerJournal_Narrator | Quest 3 Panic Valley — environmental journal entry | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781741430780_PlayerJournal_Narrator.mp3 | ~25s |
| Elder_Kael_RaptorFeathers | Quest 2 reward — raptor feather lore explanation | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781741433526_Elder_Kael_RaptorFeathers.mp3 | ~16s |

---

## DIALOGUE TREES — ALL 4 QUESTS

---

### QUEST 1 — "The Migration"
**Trigger:** Player enters DialogueTrigger_Elder_Migration at (-200, -300)
**NPC:** Elder Kael

#### Dialogue Tree

```
[ELDER_KAEL_MIGRATION_01]
"The herd moves north every dry season. They follow the same path
their ancestors walked. When you move with them — not against them —
they stop seeing you as a threat. That is how trust begins.
Not with words. With patience."

  PLAYER_CHOICE_A: "How do I follow the herd without spooking them?"
    → [ELDER_KAEL_MIGRATION_02A]
    "Stay low. Move at their pace — not faster, not slower.
    If they stop, you stop. If they run, you find cover.
    Never get between a mother and her young."
    → QUEST_OBJECTIVE_SET: Follow herd to river crossing

  PLAYER_CHOICE_B: "Why does the herd go north?"
    → [ELDER_KAEL_MIGRATION_02B]
    "The dry season burns the southern grass. The north still
    has water. The herd has known this longer than our tribe
    has existed. We follow because they are right."
    → QUEST_OBJECTIVE_SET: Follow herd to river crossing

  PLAYER_CHOICE_C: [SKIP — just nod]
    → QUEST_OBJECTIVE_SET: Follow herd to river crossing
    → No additional dialogue
```

**HERD TRUST MECHANIC — LORE GROUNDING:**
The "Herd Trust" flag is earned through repeated non-threatening behaviour near herbivores. Lore explanation: animals in this world recognise individual humans by scent and movement pattern. A human who consistently moves slowly, stays downwind, and does not startle the herd is gradually categorised as "non-threat." This is not supernatural — it is the same mechanism a wildlife photographer uses. The Elder teaches this as practical survival knowledge.

---

### QUEST 2 — "Pack Territory"
**Trigger:** Player enters DialogueTrigger_Scout_PackTerritory at (-350, -200)
**NPC:** Scout Dara

#### Dialogue Tree

```
[SCOUT_DARA_PACK_01]
"I have seen men run from the raptors. I have seen men try to fight them.
Both end the same way. The only ones who survive are the ones who
understand — raptors do not hunt what they cannot see.
Move through the tall grass. Stay downwind. Let them think you are nothing."

  PLAYER_CHOICE_A: "What if they spot me?"
    → [SCOUT_DARA_PACK_02A]
    "Then you run. Not straight — they are faster than you.
    Run toward the herd. The raptors will not follow into
    the herd's territory. Use the big animals as cover."
    → QUEST_OBJECTIVE_SET: Cross raptor territory undetected

  PLAYER_CHOICE_B: "Is there another way around?"
    → [SCOUT_DARA_PACK_02B]
    "Two days east, through the marsh. You will lose a day
    and your boots will never dry. Your choice — but the
    feathers are worth the risk."
    → QUEST_OBJECTIVE_SET: Cross raptor territory (stealth OR detour)

  PLAYER_CHOICE_C: "What are the feathers for?"
    → [SCOUT_DARA_PACK_02C]
    "Ask the Elder. He will tell you more than I know.
    What I know is that they are worth something.
    That is enough for me."
    → QUEST_OBJECTIVE_SET: Cross raptor territory undetected
```

**POST-QUEST REWARD DIALOGUE (DialogueTrigger_Elder_RaptorLore):**
```
[ELDER_KAEL_FEATHERS_01]
"You want to know why the feathers matter? Because the raptor
that wore them was the pack leader. When the pack sees you carry them,
they hesitate. Just for a moment. But in a fight, a moment is everything.
Keep them. Do not lose them."

  → ITEM_GRANTED: RaptorFeathers (passive: raptors hesitate 0.8s before first attack)
  → LORE_UNLOCKED: "Pack Hierarchy" — raptors have alpha hierarchy, disrupting alpha disrupts pack
```

---

### QUEST 3 — "Panic in the Valley"
**Trigger:** Player finds DialogueTrigger_Journal_PanicValley at (0, 1500)
**Type:** Environmental — journal pickup (no NPC, player reads/hears narration)

#### Journal Entry (read aloud by PlayerJournal_Narrator)

```
[JOURNAL_PANIC_VALLEY_01]
"Day twelve. The river is lower than last season. The herd crossed
at the shallow bend — the same place they always cross. I found tracks.
Not just the herd. Something followed them. Big. Single set of prints,
deep in the mud. Whatever it is, it is patient. It waited at the crossing
for two days before moving on. I do not think it is gone.
I think it is watching."

→ LORE_UNLOCKED: "The Watcher" — TRex has been tracking the herd migration route
→ QUEST_OBJECTIVE_SET: Reach high ground before TRex returns to valley
→ FEAR_STAT_MODIFIER: +15 fear (player character reacts to journal discovery)
```

**DESIGN NOTE:** This quest has no NPC dialogue — the story is told entirely through the environment. The journal was left by a previous survivor who did not make it. The player never meets this person. This creates dread without exposition.

---

### QUEST 4 — "Herd Defender"
**Trigger:** Player returns to DialogueTrigger_Elder_HerdDefender at (3000, 2000) after completing quest
**NPC:** Elder Kael

#### Dialogue Tree

```
[ELDER_KAEL_DEFENDER_01]
"You drove them off. The young ones are safe."
[BEAT — Elder looks at player for a long moment]
"I have seen many people come through this valley.
Most take what they need and move on. You stayed.
That is not nothing."

  PLAYER_CHOICE_A: "The herd needed help."
    → [ELDER_KAEL_DEFENDER_02A]
    "Yes. And now they know your scent. You will find
    them easier to approach from now on. The valley
    remembers what you did here."
    → HERD_TRUST_FLAG: +2 (significant increase)
    → REWARD: Stone Axe schematic unlocked

  PLAYER_CHOICE_B: "What do I get for it?"
    → [ELDER_KAEL_DEFENDER_02B]
    "Practical. Good. A person who survives asks
    what they get. Here — this is how we make the axe.
    It will serve you better than your hands."
    → HERD_TRUST_FLAG: +1
    → REWARD: Stone Axe schematic unlocked

  PLAYER_CHOICE_C: [Say nothing — just nod]
    → [ELDER_KAEL_DEFENDER_02C]
    "..."
    [Elder nods back. No more words.]
    → HERD_TRUST_FLAG: +2 (Elder respects silence)
    → REWARD: Stone Axe schematic unlocked
```

---

## CHARACTER BIBLES

### Elder Kael
- **Role:** Tribe elder, quest giver for Migration and Herd Defender
- **Age:** ~55-60 (old for this era — earned through competence, not luck)
- **Voice:** Slow, deliberate. Never wastes words. Pauses carry meaning.
- **Knowledge base:** Herd migration patterns, raptor behaviour, seasonal weather, medicinal plants
- **Motivation:** Keep the tribe alive through the dry season. The player is useful — not yet trusted.
- **What he will NOT do:** Give the player anything for free. Every reward is earned. Every piece of advice costs something (time, risk, or obligation).
- **Lore hook:** Kael lost his son to a TRex three seasons ago. He does not talk about it. But it is why he watches the valley so carefully.

### Scout Dara
- **Role:** Tribe scout, quest giver for Pack Territory
- **Age:** ~25-30 (young, fast, pragmatic)
- **Voice:** Clipped, efficient. Gives information in the fewest words possible.
- **Knowledge base:** Raptor territory boundaries, stealth movement, marsh routes, track identification
- **Motivation:** Information is survival. She trades knowledge for knowledge.
- **What she will NOT do:** Accompany the player. She scouts alone. She does not explain herself.
- **Lore hook:** Dara has crossed raptor territory seventeen times. She keeps count. She does not know why.

---

## HERD TRUST SYSTEM — NARRATIVE DESIGN

**Mechanical definition:** Integer flag 0-10. Starts at 0. Affects herbivore aggression radius and NPC dialogue options.

**Lore grounding:** Animals in this world categorise humans by behaviour pattern over time. Consistent non-threatening behaviour near the herd (slow movement, downwind approach, no sudden actions) builds recognition. This is observable animal behaviour — not supernatural.

**Narrative gates:**
- Trust 0-2: Herd scatters when player approaches within 30m
- Trust 3-5: Herd tolerates player at 15m. Elder Kael acknowledges player as "learning"
- Trust 6-8: Herd ignores player at 8m. Scout Dara shares additional raptor territory intel
- Trust 9-10: Player can move through herd. Elder Kael calls player by name in dialogue

**How trust is lost:** Running near herd (-1), startling herd (-2), killing herbivore (-3), bringing predator near herd (-2)

---

## DIALOGUE TRIGGER ACTORS IN MAP

| Actor Label | Position | Color | Quest |
|-------------|----------|-------|-------|
| DialogueTrigger_Elder_Migration | (-200, -300, 120) | Gold | Quest 1 |
| DialogueTrigger_Scout_PackTerritory | (-350, -200, 120) | Cyan | Quest 2 |
| DialogueTrigger_Journal_PanicValley | (0, 1500, 120) | Amber | Quest 3 |
| DialogueTrigger_Elder_HerdDefender | (3000, 2000, 120) | Green | Quest 4 |
| DialogueTrigger_Elder_RaptorLore | (-200, -300, 180) | Purple | Quest 2 reward |

---

## HANDOFF TO AGENT #16 — AUDIO

### Audio needs from this cycle:
1. **Ambient dialogue triggers** — when player enters dialogue zone, play soft ambient sound (rustling grass, distant herd) before NPC speaks
2. **Journal discovery sound** — paper/leather rustling when player picks up journal at (0, 1500)
3. **Herd Trust milestone sounds** — subtle audio cue when trust increases (herd vocalization, calm)
4. **Fear stat spike sound** — when journal triggers +15 fear, heartbeat increase
5. **Voice lines already recorded** — 4 TTS files ready for integration (see table above)

### Integration priority:
- Quest 1 Elder Migration voice line is the FIRST dialogue the player hears — must be clean, no echo
- Journal narration (Quest 3) should have slight reverb — player is reading in open valley
- Scout Dara lines should be whispered/low — she is always aware of being overheard

---

*Agent #15 — Narrative & Dialogue | PROD_CYCLE_AUTO_20260617_014*
