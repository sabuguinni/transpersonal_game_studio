# Narrative & Dialogue System — Cycle 001
## Agent #15 — PROD_CYCLE_AUTO_20260618_001

---

## TRIBE LORE — ASHROCK TRIBE

**Setting:** A valley in the late Cretaceous. The Ashrock tribe has survived here for three generations by hunting smaller prey and avoiding the great predators. Three moons ago, a T-Rex pack migrated into the valley and drove the tribe from their main camp. The elders are too old to flee further. The young hunters are dead or scattered. The player is the last capable hunter.

**Tribe Status:**
- 6 elders remaining at base camp (near origin)
- 3 children too young to hunt
- 2 injured scouts who cannot walk
- 0 active hunters (player is the only one)

**Central Tension:** The tribe has food for 4 days. The T-Rex pack has settled near the river — blocking the main hunting grounds. The player must find alternative food sources, secure water, and eventually drive the predators away or find a new valley.

---

## ELDER NPC — DIALOGUE TREE

### NPC: Elder Mara (lead elder, female, ~60 years old)
**Personality:** Pragmatic, direct, no sentimentality. Speaks in short sentences. Has seen many hunters die. Does not waste words.

### DIALOGUE NODE 01 — First Meeting (NarrTrigger_ElderIntro)
**Trigger:** Player enters 150-unit radius of Elder NPC
**Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781747321767_Elder_NPC.mp3

**Script:**
> "You are the last hunter of the Ashrock tribe. Three moons ago, the great predators drove our people from the valley. Now only the elders remain, too old to run. If you fail to bring meat before the cold night falls, we will not see another dawn. Go east. Track the raptors. Survive."

**Player Response Options:**
- [GESTURE: Nod] → Quest FIRST_HUNT activates
- [GESTURE: Point west] → Elder responds: "West is T-Rex territory. You will not return."
- [GESTURE: Sit down] → Elder responds: "There is no time to rest. Go."

---

### DIALOGUE NODE 02 — Water Quest (NarrTrigger_WaterFound area)
**Trigger:** Player talks to Elder after First Hunt complete
**Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781747337313_Elder_NPC.mp3

**Script:**
> "The water hole to the south is drying up. I have seen this before — when the great lizards move, the rivers follow them. We must find a new source before the dry season comes. Take this hollow gourd. Fill it at the spring beyond the stone ridge. Do not linger — the ridge is hunting ground for the pack."

**Player Response Options:**
- [GESTURE: Take gourd] → Quest WATER_SOURCE activates
- [GESTURE: Point at injured scout] → Elder: "He cannot walk. You are the only one."

---

### DIALOGUE NODE 03 — Post-Hunt Return (NarrTrigger_HuntComplete)
**Trigger:** Player returns after completing First Hunt quest
**Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781747346259_Elder_NPC.mp3

**Script:**
> "You came back. I did not think you would. The last three hunters who went east — they did not return. Sit. Eat. Tomorrow I will tell you about the valley before the great ones came. About what we lost. About what we are still fighting to keep."

**Unlocks:** Journal Entry 02 (valley history)

---

### DIALOGUE NODE 04 — Campfire Quest (NarrTrigger_ShelterBuilt area)
**Trigger:** Player approaches Elder after nightfall without campfire built
**Script (no audio yet — queue for Agent #16):**
> "You feel it? The cold comes fast here. Three nights without fire and the children will not wake. The forest floor has dry branches — I can see them from here. Bring them. I will show you how our people make fire."

**Player Response Options:**
- [GESTURE: Nod] → Quest BUILD_SHELTER activates
- [GESTURE: Shrug] → Elder: "Then we freeze. Your choice."

---

## JOURNAL ENTRIES (unlocked on quest completion)

### Journal Entry 01 — First Raptor Encounter
**Unlocks after:** First Hunt quest complete
**Trigger Actor:** NarrJournal_Entry01 (400, 400, 50)
**Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781747349277_PlayerJournal_Narrator.mp3

**Text:**
> "Journal entry. Third day alone. The raptor tracks lead north now — the pack has moved. I found the remains of a campfire near the ridge. Someone else was here, not long ago. The ashes were still warm. I am not the only survivor in this valley."

**Gameplay significance:** Hints at secondary survivor NPC (future quest hook for Agent #14)

---

### Journal Entry 02 — Valley History
**Unlocks after:** Post-hunt Elder dialogue (Node 03)
**Trigger Actor:** NarrJournal_Entry02 (-300, -400, 50)

**Text (no audio yet — queue for Agent #16):**
> "The elder spoke tonight. She said this valley had a name once — the Ashrock people called it the Green Throat, because of the river that ran through it like a vein. The T-Rex pack arrived with the dry season. They followed the hadrosaur herds north. The hadrosaurs are gone now. But the T-Rex stayed."

**Gameplay significance:** Explains why T-Rex is in the valley, sets up hadrosaur migration mechanic

---

### Journal Entry 03 — Other Survivor
**Unlocks after:** Water Source quest complete
**Trigger Actor:** NarrJournal_Entry03 (800, -600, 50)

**Text (no audio yet — queue for Agent #16):**
> "Near the spring I found a spear. Not ours — the binding is different, river reeds instead of bark. Someone from another tribe made this. They were here recently. The elder does not know of any other survivors. But the spear does not lie."

**Gameplay significance:** Introduces rival/allied tribe mechanic — major story branch for Act 2

---

## WORLD ACTOR PLACEMENT TABLE

| Actor Label | Position (X,Y,Z) | Purpose |
|---|---|---|
| NarrTrigger_ElderIntro | (0, 200, 50) | Elder intro speech trigger |
| NarrTrigger_HuntComplete | (1200, 800, 50) | Post-hunt dialogue trigger |
| NarrTrigger_WaterFound | (200, -1200, 50) | Water quest dialogue trigger |
| NarrTrigger_ShelterBuilt | (-500, 700, 50) | Campfire quest dialogue trigger |
| NarrJournal_Entry01 | (400, 400, 50) | Journal entry 1 pickup |
| NarrJournal_Entry02 | (-300, -400, 50) | Journal entry 2 pickup |
| NarrJournal_Entry03 | (800, -600, 50) | Journal entry 3 pickup |

---

## VOICE LINE ASSET REGISTRY

| Character | Line | Duration | URL |
|---|---|---|---|
| Elder_NPC | Quest intro — last hunter briefing | ~19s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781747321767_Elder_NPC.mp3 |
| Elder_NPC | Water quest — find the spring | ~21s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781747337313_Elder_NPC.mp3 |
| Elder_NPC | Post-hunt return — you survived | ~17s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781747346259_Elder_NPC.mp3 |
| PlayerJournal_Narrator | Journal entry 1 — warm ashes | ~17s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781747349277_PlayerJournal_Narrator.mp3 |

---

## LINES QUEUED FOR AGENT #16 (Audio Agent)

The following lines have scripts but no audio yet. Agent #16 should generate TTS for these:

1. **Elder — Campfire quest:** "You feel it? The cold comes fast here..."
2. **Journal Entry 02:** "The elder spoke tonight. She said this valley had a name once..."
3. **Journal Entry 03:** "Near the spring I found a spear. Not ours..."
4. **Elder — T-Rex warning:** "West is T-Rex territory. You will not return."
5. **Elder — No time:** "There is no time to rest. Go."

---

## DEPENDENCIES

- **From Agent #14:** QuestTrigger actors confirmed in map — dialogue triggers placed adjacent
- **To Agent #16:** 5 voice lines queued (scripts above), Elder voice should be deep/gravelly female
- **To Agent #17:** Dialogue trigger zones need particle effect (subtle glow/dust) to indicate interactable
- **To Agent #11 (NPC Behavior):** Elder NPC needs idle animation at (0, 0, 50) — seated near campfire

---

## NARRATIVE ARCHITECTURE NOTES

**Act 1 (current):** Survival — player proves competence to tribe
**Act 2 (planned):** Discovery — rival tribe contact, valley secrets, T-Rex territory mapping  
**Act 3 (planned):** Conflict — resource war with rival tribe OR alliance against T-Rex pack

**Core narrative loop:** Quest → Complete → Elder dialogue → Journal entry → New quest unlocked

This keeps narrative delivery tied to gameplay actions, not cutscenes.
