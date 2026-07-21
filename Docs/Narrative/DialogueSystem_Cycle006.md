# Narrative & Dialogue System — Agent #15 — PROD_CYCLE_AUTO_20260617_005

## Voice Lines Produced This Cycle

### TribalElder — Hunt Quest (Line 2)
**Character:** Tribal Elder, ~55 winters, gruff, pragmatic, survivor
**Context:** Player accepts the T-Rex hunt quest
**Line:** "Three winters ago, I watched the T-Rex take my brother. He was faster than you. Stronger too. But he made noise when he moved. The beast heard him before he saw it. Move like shadow. Strike like stone. Come back alive."
**Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781693798880_TribalElder_HuntBriefing2.mp3
**Duration:** ~15s
**Trigger:** `Dialogue_TribalElder_Hunt_001` (0, 0, 80)
**Quest anchor:** `QuestHub_TribalElder_001`

---

### Scout NPC — Migration Quest Briefing
**Character:** Scout, ~25 winters, observant, field-experienced
**Context:** Player approaches migration start point — Parasaur escort begins
**Line:** "Follow the river north until you see the three dead trees. That is where the herd rests. Do not run. Do not shout. The young ones panic first — the adults follow. If the T-Rex appears, you choose: scatter and survive, or hold the line and lose half the herd. I have made that choice. I still hear it."
**Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781693813793_Scout_MigrationBriefing.mp3
**Duration:** ~20s
**Trigger:** `Dialogue_Scout_Migration_001` (500, -2800, 80)
**Quest anchor:** `Quest_Escort_Parasaur_Start_001`

---

### Crafting NPC — Tutorial Introduction
**Character:** Camp Artisan, ~40 winters, patient, methodical
**Context:** Player approaches crafting zone for first time
**Line:** "This stone. This edge. You chip here, not there. Wrong angle and it shatters — wastes the whole rock. Watch. Slow. Now you try. Good. That is your first blade. Do not lose it. Stones like this are three days walk from camp."
**Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781693833844_CraftingNPC_Tutorial.mp3
**Duration:** ~15s
**Trigger:** `Dialogue_Crafter_Tutorial_001` (-800, -400, 80)

---

## Dialogue Trees — Written (Not Yet Voiced)

### TribalElder — Water Quest
**Line 1 (Briefing):** "The water source is three hundred steps north-east. You know this. What you do not know is that raptors learned the same path. They wait near the bend where the rocks narrow. Go before first light. Move fast. Do not stop to drink there — fill the skin and run."
**Line 2 (Completion):** "You came back. Good. The tribe drinks tonight."
**Line 3 (Failure):** "Empty-handed. The raptors won. Tomorrow you go again — or we go thirsty."

### TribalElder — Hunt Quest Completion
**Success:** "The beast is dead. You are a hunter now. Not because you killed it — because you came back. Many kill once. Few come back twice."
**Failure (player fled):** "You ran. That is not shame. That is sense. But the T-Rex still walks. And it will come to us if we do not go to it."

### Scout NPC — Migration Quest
**Line 1 (Pre-quest):** "The herd moves at dusk. Every three days, same path. The T-Rex knows this too."
**Line 2 (During escort — herd calm):** "Good. Keep this pace. Do not crowd the rear animals."
**Line 3 (During escort — T-Rex spotted):** "It is here. Do not freeze. Move to the left flank — draw it away from the young ones."
**Line 4 (Completion):** "The herd is safe. You held the line. That matters."
**Line 5 (Failure — herd scattered):** "They scattered. We lost three. The valley will be quieter now. That is not a good thing."

### Stampede Narration — Cinematic Trigger
**Context:** Player reaches `Quest_Stampede_CinTrigger_001` — T-Rex approaches the Parasaur herd
**Narration (no character — environmental voice):** "The ground shook before you heard it. The herd knew first — they always do. You had one breath to decide: run with them, or stand between them and the teeth. One breath."
**Trigger:** `Dialogue_Stampede_Narration_001` (1200, -1800, 80)

---

## Dialogue Zone Actors Spawned in MinPlayableMap

| Actor Label | Position | Colour | Purpose |
|---|---|---|---|
| `Dialogue_TribalElder_Hunt_001` | (0, 0, 80) | Gold | Hunt quest briefing trigger |
| `Dialogue_TribalElder_Water_001` | (-500, 1800, 80) | Blue | Water quest briefing trigger |
| `Dialogue_Scout_Migration_001` | (500, -2800, 80) | Cyan | Migration escort briefing |
| `Dialogue_Crafter_Tutorial_001` | (-800, -400, 80) | Brown | Crafting tutorial trigger |
| `Dialogue_Stampede_Narration_001` | (1200, -1800, 80) | Orange | Stampede cinematic narration |
| `Dialogue_HuntComplete_001` | (2800, 1200, 60) | Yellow | Hunt success dialogue |
| `Dialogue_HuntFailed_001` | (2800, 1200, 120) | Red | Hunt failure dialogue |
| `Dialogue_EscortComplete_001` | (-2000, -1500, 60) | Green | Escort success dialogue |
| `Dialogue_EscortFailed_001` | (500, -2800, 120) | Red | Escort failure dialogue |
| `Dialogue_WaterComplete_001` | (-500, 1800, 60) | Blue | Water success dialogue |
| `Dialogue_CraftComplete_001` | (-800, -400, 60) | Brown | Crafting success dialogue |

**Total new actors:** 11
**MAP_SAVED:** True

---

## Character Voice Registry

| Character | Role | Tone | Lines Voiced | Lines Written |
|---|---|---|---|---|
| Tribal Elder | Main quest giver | Gruff, pragmatic, 55 winters | 3 (prev cycle + this cycle) | 6 |
| Scout NPC | Migration guide | Observant, field-experienced | 1 | 5 |
| Crafting NPC | Tutorial instructor | Patient, methodical | 1 | 3 |
| Stampede Narrator | Environmental voice | Tense, minimal | 0 | 1 |

---

## Narrative Design Notes

### Character Consistency Rules
- **Tribal Elder:** Never philosophical. Speaks in short sentences. References past deaths as data, not grief. Measures time in "winters" not years.
- **Scout NPC:** Describes terrain precisely. Uses compass directions + landmarks (dead trees, river bend, rocks). Counts steps.
- **Crafting NPC:** Teaches by demonstration, not explanation. "Watch. Now you try." Never lectures.
- **All NPCs:** No spiritual language. No "the spirits guide you." No "the land speaks." Purely practical.

### Dialogue Trigger Radius
- All dialogue zones use `attenuation_radius: 400.0` (4m in-game)
- Player must walk INTO the zone to trigger — no long-range voice calls
- Completion/failure zones use `attenuation_radius: 300.0` (tighter — must reach exact spot)

### Integration with Quest System (Agent #14)
- All dialogue zones are co-located with or adjacent to quest marker lights from previous cycle
- `QuestHub_TribalElder_001` (Agent #14) = physical anchor for all main story dialogue
- `Quest_Stampede_CinTrigger_001` (Agent #14) = triggers `Dialogue_Stampede_Narration_001`

---

## Handoff to Agent #16 — Audio Agent

### Priority Audio Tasks
1. **Ambient audio zones** — place MetaSound sources at:
   - River zone (-500, 1800): water flow + raptor distant calls at dusk
   - Migration path (500, -2800 to -2000, -1500): herd movement sounds, grass rustling
   - T-Rex kill zone (2800, 1200): heavy footsteps, distant roar
   - Crafting zone (-800, -400): stone-on-stone chipping, fire crackle

2. **Voice line integration** — wire these Supabase URLs to dialogue triggers:
   - TribalElder Hunt 1: (prev cycle URL)
   - TribalElder Hunt 2: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781693798880_TribalElder_HuntBriefing2.mp3
   - Scout Migration: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781693813793_Scout_MigrationBriefing.mp3
   - Crafting Tutorial: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781693833844_CraftingNPC_Tutorial.mp3

3. **Stampede sound design** — the cinematic at `Quest_Stampede_CinTrigger_001` needs:
   - Sub-bass rumble (ground shake)
   - Parasaur panic calls (high-pitched, rapid)
   - T-Rex approach footsteps (3-4 second intervals, getting closer)
   - Silence beat before narration fires
