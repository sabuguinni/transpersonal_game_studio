# Narrative & Dialogue Agent #15 — Elder Hunter Dialogue Tree
## Cycle: PROD_CYCLE_AUTO_20260618_003

---

## ELDER HUNTER — NPC PROFILE

**Role:** Recurring mentor NPC. Appears at camp between quests. Gives briefings, hints, and success/failure reactions.
**Location in map:** `NarrActor_ElderCamp_Fire` — (0, -800, 120) — orange firelight marker
**Voice:** Gravelly, slow, deliberate. Economy of words. Never explains what the player can observe themselves.
**Design principle:** He speaks in observations, not instructions. He trusts the player to interpret.

---

## QUEST 1 — "Track the Migration"

### Briefing (player approaches camp before Q1)
> *"Tracks. Fresh ones. The herd passed through here not long ago. Follow the trail — they move with the river, always with the river. You will know you are close when the ground smells of them."*

**Audio:** [Elder_Hunter_Q1.mp3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756395123_Elder_Hunter_Q1.mp3)
**Map marker:** `NarrActor_Q1_ElderBriefing` — (-500, -800, 120) — cyan light

### Hint (player can ask mid-quest)
> *"The river bends north past the tall rocks. The herd does not cross there — the current is too strong. They follow the eastern bank."*

### Success (player reaches Q1_MigrationEnd)
> *"You found them. Good. Now you know their path. That knowledge is worth more than any spear."*

**Map marker:** `NarrActor_Q1_Success` — (7000, 2000, 120) — green light
**Audio:** [Elder_Hunter_JournalIntro.mp3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756493668_Elder_Hunter_JournalIntro.mp3)

### Failure (player loses trail)
> *"The trail went cold. It happens. The herd does not wait for us. Rest. Try again at first light — the dew holds tracks longer than dry ground."*

---

## QUEST 2 — "Raptor Pack Territory"

### Briefing (player approaches camp before Q2)
> *"Five of them. They hunt together — that is what makes them dangerous. Break the formation. Kill the scout first, before it calls to the others. After that, the pack loses its eyes."*

**Audio:** [Elder_Hunter_Q2.mp3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756397347_Elder_Hunter_Q2.mp3)
**Map marker:** `NarrActor_Q2_ElderBriefing` — (-800, 1200, 120) — purple light

### Hint (player asks mid-quest)
> *"The flankers move wide — wider than you expect. Do not watch the one in front of you. Watch the edges. That is where the kill comes from."*

### Success (all 5 raptors eliminated)
> *"You survived. Good. Most do not, the first time. The raptors are gone from that territory — for now. But they will return. They always return. Next time, you will be ready. Next time, you will not need me to tell you where they hide."*

**Map marker:** `NarrActor_Q2_Success` — (-1800, 1200, 120) — green light
**Audio:** [Elder_Hunter_Q2_Success.mp3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756503057_Elder_Hunter_Q2_Success.mp3)

### Failure (player killed)
> *"The pack took you. That is not weakness — that is information. You know now where the ambush point is. Use that."*

---

## QUEST 3 — "Watering Hole Ambush"

### Briefing (player approaches camp before Q3)
> *"The watering hole has fed our people for generations. But the great beasts come here too — they know the smell of water as well as we do. Move downwind. Stay low. Strike only when the moment is perfect. A rushed hunter feeds the predators. A patient hunter feeds the tribe."*

**Audio:** [Elder_Hunter_Q3.mp3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756491228_Elder_Hunter_Q3.mp3)
**Map marker:** `NarrActor_Q3_ElderBriefing` — (1500, -200, 120) — yellow light

### Hint (player asks mid-quest)
> *"North approach. Always north. The wind comes from the south — they will not smell you if you come from the north. The east and west are open ground. The south is death."*

### Success (prey secured)
> *"The tribe eats tonight. That is all that matters. Everything else — the fear, the waiting, the cold — none of it matters when the tribe eats."*

**Map marker:** `NarrActor_Q3_Success` — (2000, -800, 120) — green light

### Failure (player detected)
> *"They scattered. The hunt is over for today. The animals remember. They will be more cautious tomorrow. So must you."*

---

## JOURNAL ENTRIES — "The Lost Hunter"

A previous hunter who followed the migration and never returned. His journal entries are scattered along the Q1 migration trail. They build a parallel narrative — a cautionary tale that foreshadows the dangers the player will face.

### Entry 01 — `NarrActor_Journal_Entry01` — (1000, 2000, 120)
**Text (carved into stone/bark):**
> *"Day three. The tracks are clear. The herd is large — larger than any I have seen. I count at least forty. Maybe more. The Elder will want to know this."*

### Entry 02 — `NarrActor_Journal_Entry02` — (2500, 3000, 120)
**Text:**
> *"Day seven. Something is following the herd. Not us — something else. I found claw marks on the trees. Deep ones. Whatever made them is bigger than a raptor."*

### Entry 03 — `NarrActor_Journal_Entry03` — (4000, 4000, 120)
**Text:**
> *"Day nine. I saw it at dusk. It moves like a shadow between the trees. The herd knows it is there — they bunch together at night, the young ones in the centre. Smart animals."*

### Entry 04 (Day 12) — `NarrActor_Journal_Entry04` — (5500, 5000, 120)
**Text:**
> *"Day twelve. The herd has not moved. I have been following the same tracks for three days — they circle back, always back to the river. Something is keeping them here. Something they fear more than us. I will wait one more night. If the tracks do not change by dawn, I go north alone."*

**Audio:** [JournalEntry_LostHunter_Day12.mp3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756506565_JournalEntry_LostHunter_Day12.mp3)

### Final Marker — `NarrActor_Journal_FinalSpear` — (6500, 5800, 120)
**Red light — danger marker**
**Text (spoken by Elder Hunter when player finds the spear):**
> *"He did not return from the valley. We found his spear near the river bend — still lodged in the earth, pointing north. He always said the herd would come back. He was right. Now you must finish what he started."*

**Audio:** [Elder_Hunter_JournalIntro.mp3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756493668_Elder_Hunter_JournalIntro.mp3)

---

## MAP ACTOR INVENTORY (Cycle 003)

| Label | Position | Color | Purpose |
|-------|----------|-------|---------|
| `NarrActor_ElderCamp_Fire` | (0, -800, 120) | Orange | Elder Hunter camp — recurring NPC anchor |
| `NarrActor_Q1_ElderBriefing` | (-500, -800, 120) | Cyan | Q1 briefing trigger |
| `NarrActor_Q2_ElderBriefing` | (-800, 1200, 120) | Purple | Q2 briefing trigger |
| `NarrActor_Q3_ElderBriefing` | (1500, -200, 120) | Yellow | Q3 briefing trigger |
| `NarrActor_Journal_Entry01` | (1000, 2000, 120) | Warm yellow | Journal entry 1 |
| `NarrActor_Journal_Entry02` | (2500, 3000, 120) | Warm yellow | Journal entry 2 |
| `NarrActor_Journal_Entry03` | (4000, 4000, 120) | Warm yellow | Journal entry 3 |
| `NarrActor_Journal_Entry04` | (5500, 5000, 120) | Warm yellow | Journal entry 4 (Day 12) |
| `NarrActor_Journal_FinalSpear` | (6500, 5800, 120) | Red | Lost hunter's spear — narrative climax |
| `NarrActor_Q1_Success` | (7000, 2000, 120) | Green | Q1 success dialogue |
| `NarrActor_Q2_Success` | (-1800, 1200, 120) | Green | Q2 success dialogue |
| `NarrActor_Q3_Success` | (2000, -800, 120) | Green | Q3 success dialogue |

**Total narrative actors this cycle: 12**
**MAP_SAVED: True**

---

## VOICE LINES PRODUCED (Cycle 003)

| Character | Line | Duration | URL |
|-----------|------|----------|-----|
| Elder Hunter | Q3 watering hole briefing | ~19s | [MP3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756491228_Elder_Hunter_Q3.mp3) |
| Elder Hunter | Journal intro / lost hunter lore | ~14s | [MP3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756493668_Elder_Hunter_JournalIntro.mp3) |
| Elder Hunter | Q2 success reaction | ~16s | [MP3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756503057_Elder_Hunter_Q2_Success.mp3) |
| Lost Hunter | Journal Day 12 (read-aloud) | ~19s | [MP3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756506565_JournalEntry_LostHunter_Day12.mp3) |

**Previous cycles (Q1 + Q2 briefings):**
- Q1: [MP3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756395123_Elder_Hunter_Q1.mp3)
- Q2: [MP3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756397347_Elder_Hunter_Q2.mp3)

**Total voice lines across all cycles: 6 (complete set for all 3 quests)**

---

## HANDOFF BRIEF — Agent #16 Audio Agent

### Audio needs from narrative system:
1. **Ambient camp sound** — crackling fire at `NarrActor_ElderCamp_Fire` position
2. **Journal discovery sound** — stone scraping / papyrus rustle when player finds entries
3. **Quest briefing stinger** — short musical cue when Elder Hunter begins speaking
4. **Quest success fanfare** — tribal percussion, brief, not triumphant — more like relief
5. **Lost hunter theme** — melancholic underscore for journal entry zones (Entry01-04 + FinalSpear)
6. **Voice line integration** — all 6 TTS MP3s need to be imported as SoundWave assets and attached to their respective trigger actors

### Spatial audio zones:
- Camp fire radius: ~500 units from (0, -800, 120)
- Journal entries: ~300 unit trigger radius each, sequential unlock (Entry01 → 02 → 03 → 04 → FinalSpear)
- Quest briefings: player must be within 400 units of marker AND facing Elder Hunter direction

---

*Agent #15 — Narrative & Dialogue | PROD_CYCLE_AUTO_20260618_003*
