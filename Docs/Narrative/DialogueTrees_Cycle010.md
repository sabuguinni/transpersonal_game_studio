# Dialogue Trees — Agent #15 Cycle 010
**PROD_CYCLE_AUTO_20260617_010**

---

## VOICE LINES PRODUCED

| Character | File | Duration | Quest |
|-----------|------|----------|-------|
| CampElder_QuestGiver_Full | `tts/1781717944125_CampElder_QuestGiver_Full.mp3` | ~26s | Quest 1 "The Herd Moves" |
| Tracker_NPC_RaptorWarning | `tts/1781717949320_Tracker_NPC_RaptorWarning.mp3` | ~24s | Quest 2 "Silence in the Plains" |
| TribeElder_HornedWall | `tts/1781717972702_TribeElder_HornedWall.mp3` | ~28s | Quest 3 "The Horned Wall" |
| Survivor_NPC_PackHunters | `tts/1781717977728_Survivor_NPC_PackHunters.mp3` | ~25s | Quest 4 "Pack Hunters" |

---

## NPC DIALOGUE TREES

### NPC_CampElder_001 — Camp Elder (Quest 1 Giver)
**Position:** (200, 200, 150) — warm orange light marker in camp

**GREETING (first encounter):**
> "You are new here. Good. New eyes see things old eyes miss. Sit. I have something to tell you."

**QUEST OFFER — "The Herd Moves":**
> "The herd is moving north. I have seen this before — three times in my life. Each time, the water follows. You must go with them. Stay low, stay quiet, stay downwind. The big ones — the long-necks — they will not harm you if you do not frighten them. But if you run, they run. And when something that large runs, everything in its path dies. Follow the herd. Find the water. Come back alive."

**ACTIVE REMINDER (player returns without completing):**
> "The herd will not wait. Every hour you stay here, they move further north. Go."

**QUEST COMPLETION:**
> "You found it. The water. I knew you would — you move carefully, like someone who wants to live. The others will follow now. You have given us another season."

**FAILURE/RETRY:**
> "You scattered them. It happens. Wait two days — the herd circles back. Then try again. This time, stay further back."

---

### NPC_Tracker_001 — Scout/Tracker (Quest 2 Context)
**Position:** (800, 600, 150) — cool blue-white light near eastern edge

**GREETING:**
> "You are heading east? Stop. Listen to me first."

**QUEST OFFER — "Silence in the Plains":**
> "I tracked them for two days. Three raptors, moving in a triangle pattern — one leads, two flank. They are not random. They are smart. They waited until the old one separated from the herd, then they moved all at once. I barely made it back. If you go east, go in daylight, go fast, and do not stop moving. The moment you slow down — that is when they test you."

**ACTIVE REMINDER:**
> "Still here? The plains do not get safer with time. Go now, while the light holds."

**QUEST COMPLETION:**
> "You made it through their territory. Good. Now you know their pattern. Remember it — they will use it again."

**FAILURE/RETRY:**
> "You froze. I saw it from here. Do not freeze. Movement is life. Try again — they reset their patrol at dawn."

---

### NPC_TribeElder_001 — Tribe Elder (Quest 3 Context)
**Position:** (100, 400, 150) — deep amber light near central camp

**GREETING:**
> "My grandfather crossed that valley. His grandfather before him. It is not impossible. It only looks impossible."

**QUEST OFFER — "The Horned Wall":**
> "My father crossed that valley when the horned ones were still young. He said the trick is noise — they do not like noise from the sides. You make sound to the left, they turn left. You make sound to the right, they turn right. The whole wall of them moves like one animal. Use that. Make them turn, walk through the gap. But do not touch them. Do not ever touch them. One touch and the whole herd becomes a stampede."

**ACTIVE REMINDER:**
> "The horned ones are creatures of habit. They follow the same path at the same time each day. You have until midday before they move again."

**QUEST COMPLETION:**
> "You walked through the wall. My father would have respected that. Beyond the valley — what did you see?"

**FAILURE/RETRY:**
> "A stampede is not the end. They tire quickly. Find high ground, wait, then cross when they settle."

---

### NPC_Survivor_001 — Survivor Witness (Quest 4 Context)
**Position:** (600, 1200, 150) — pale green light near river zone

**GREETING (traumatised, hesitant):**
> "...You came from the north path? You are lucky. Or you are fast."

**QUEST OFFER — "Pack Hunters":**
> "I was with four others when they came. We heard nothing — no warning, no sound. Just shadows moving fast through the tall grass. Then Karo was gone. Just — gone. We ran. Two more did not make it. I hid in the river, underwater, holding my breath until I could not anymore. They do not follow into deep water. Remember that. Deep water. That is the only thing that stops them."

**ACTIVE REMINDER:**
> "If you hear the grass stop moving — run. That is when they are about to strike."

**QUEST COMPLETION:**
> "You made it to camp. Good. Tell the others what you saw. They need to know the raptors have moved closer."

**FAILURE/RETRY:**
> "The river. Always the river. They hate the water. Get there and you survive."

---

## LORE INTEGRATION

### What is beyond the T-Rex territory? (Quest 1 payoff)
The water source north is a large inland lake — **Lake Karo** (named after the survivor's lost companion, retroactively). The lake is fed by glacial melt from the northern mountains. It is the only permanent water source in the region during dry season. The long-neck herd migrates there every year. Beyond the lake: the northern forest, where the climate is cooler and the predator density is lower. This is the long-term goal of the tribe — reach the northern forest before the dry season kills the southern plains.

### Why does the T-Rex territory matter? (Quest 3 payoff)
The valley between the camp and the water source is the T-Rex's core territory. The triceratops herd uses the same valley as a migration corridor. The player must use the triceratops as a distraction/shield to cross — this is the first time the player learns that dinosaur behaviour can be used strategically, not just avoided.

### Raptor pack identity (Quest 2 & 4 continuity)
The three raptors are the same pack across both quests. Quest 2 = first encounter (player observes from distance). Quest 4 = direct confrontation (pack attacks). If player completed Quest 2 carefully, they know the flanking pattern — this gives a mechanical advantage in Quest 4 (they can predict the flankers).

---

## NPC ACTOR LABELS IN MAP
- `NPC_CampElder_001` — (200, 200, 150)
- `NPC_Tracker_001` — (800, 600, 150)
- `NPC_TribeElder_001` — (100, 400, 150)
- `NPC_Survivor_001` — (600, 1200, 150)

---

## HANDOFF TO AGENT #16 — AUDIO AGENT

### Voice Line Integration
All 4 TTS files are in Supabase Storage under `tts/` prefix. Audio Agent should:
1. Import MP3s into `/Game/Audio/Dialogue/NPCs/`
2. Create MetaSound sources for each NPC voice
3. Trigger on proximity (400 unit radius from NPC marker)
4. Quest 1 Elder: warm reverb (campfire ambience)
5. Quest 2 Tracker: minimal reverb (open plains)
6. Quest 3 Tribe Elder: slight cave reverb (elder's shelter)
7. Quest 4 Survivor: slight tremor/breath effect (trauma)

### Ambient Narrative Audio Needs
- Lake Karo (north destination): water ambience, distant long-neck calls
- Raptor territory: unsettling silence, distant grass movement sounds
- Triceratops valley: low rumble, herd breathing, ground vibration
- Camp at night: fire crackle, distant predator calls (far away, not immediate threat)
