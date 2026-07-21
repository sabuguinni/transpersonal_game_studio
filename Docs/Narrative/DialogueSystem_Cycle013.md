# Narrative & Dialogue System — Production Cycle 013
**Agent #15 — Narrative & Dialogue Agent**
**Cycle:** PROD_CYCLE_AUTO_20260617_013

---

## Voice Lines Produced This Cycle

| Character | File | URL | Duration | Trigger |
|-----------|------|-----|----------|---------|
| Elder_QuestComplete_Q1 | `tts/1781732486542_Elder_QuestComplete_Q1.mp3` | [link](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781732486542_Elder_QuestComplete_Q1.mp3) | ~28s | Q1 completion return |
| Elder_QuestComplete_Q3 | `tts/1781732492077_Elder_QuestComplete_Q3.mp3` | [link](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781732492077_Elder_QuestComplete_Q3.mp3) | ~22s | Q3 completion return |
| Scout_QuestComplete_Q2 | `tts/1781732507318_Scout_QuestComplete_Q2.mp3` | [link](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781732507318_Scout_QuestComplete_Q2.mp3) | ~20s | Q2 fail/partial return |

---

## Dialogue Trees — Full Text

### NPC: Elder (NPC_Elder_QuestGiver_001 / DialogueTrigger_Elder_001)
**Position:** (-200, -300, 100) in MinPlayableMap

#### Q1 — "Follow the Herd" (GIVE)
> *"The herd is moving. Something spooked them from the north. Follow their trail — whatever scared a hundred Triceratops is worth knowing about."*
- **Tone:** Urgent, pragmatic. No sentimentality.
- **Player response options:** [Go now] / [Ask about danger] / [Not yet]
- **If [Ask about danger]:** *"If it's a predator, it's bigger than anything you've faced. If it's not a predator — that's worse. Go find out."*

#### Q1 — "Follow the Herd" (COMPLETE — SUCCESS)
> *"The herd moved north three days ago. We followed the trail — bones, trampled ground, the smell of fear in the mud. At the end of it, we found the nest. A T-Rex nest, with eggs the size of your torso. That is what scattered a hundred Triceratops. Now you know what we are dealing with. Stay out of the northern valley until the eggs hatch and she moves on. Or don't. Your choice. But don't say I didn't warn you."*
- **Tone:** Measured, experienced. Respect for the player's survival.
- **Reward trigger:** Knowledge of T-Rex nest location unlocked on map

#### Q3 — "The Great Migration" (GIVE)
> *"The corridor opens in two days. The Brachiosaurus move south every season — same path, same timing. You need to cross before they do, or wait three days on the other side. There is no third option. The flint deposits are on the far bank. We need them. Cross when I tell you."*
- **Tone:** Commanding. This is not a request.
- **Player response options:** [Understood] / [What if I wait?] / [How wide is the corridor?]
- **If [What if I wait?]:** *"Then you wait three days with nothing to eat on the far bank. Your choice."*
- **If [How wide is the corridor?]:** *"Wide enough that you cannot run it end to end. Narrow enough that one wrong step puts you under a foot the size of this camp. Move fast, stay low, follow the gaps."*

#### Q3 — "The Great Migration" (COMPLETE — SUCCESS)
> *"You made it through the corridor. Good. Most don't, first time. The Brachiosaurus don't mean to kill you — they just don't see you. That's worse. Now you know the rhythm of the crossing: wait for the gap after the lead female passes, move fast, stay low. Remember it. We cross that corridor every season. Every. Season."*
- **Tone:** Quiet respect. This is the Elder acknowledging the player has learned something real.
- **Reward trigger:** Migration corridor timing knowledge — permanent map annotation

---

### NPC: Scout/Tracker (NPC_Scout_Tracker_001 / DialogueTrigger_Scout_001)
**Position:** (-350, -200, 100) in MinPlayableMap

#### Q2 — "Scavenger's Trail" (GIVE)
> *"Stay low. The pterosaurs are circling something dead to the east. Where they gather, there are bones — and bones mean tools, if you get there before the ground scavengers do."*
- **Tone:** Clipped, tactical. Scout doesn't waste words.
- **Player response options:** [On it] / [How far?] / [What kind of bones?]
- **If [How far?]:** *"Half a day's run. Move now, not after you've eaten."*
- **If [What kind of bones?]:** *"Big ones. Whatever died out there was large. Large bones make large tools. Go."*

#### Q2 — "Scavenger's Trail" (COMPLETE — FAIL/PARTIAL)
> *"You came back empty handed. That tells me something. The pterosaurs were gone when you got there — ground scavengers beat you to the bones. Next time, move faster. When you see them circling, you have maybe half a day before the carcass is stripped. Half a day. That's your window. Don't waste it."*
- **Tone:** No sympathy. Practical lesson, not a reprimand.
- **Trigger condition:** Player returns without bone tools

#### Q2 — "Scavenger's Trail" (COMPLETE — SUCCESS)
> *"Good. Those bones will make cutting edges that last a season. You read the birds right — that's harder than it sounds. Most people see pterosaurs and think danger. You need to learn to think: opportunity. Same animal, different context."*
- **Tone:** Approving, but teaching continues. Scout never fully relaxes.
- **Reward trigger:** Bone tool crafting recipe unlocked

---

## Quest 4 — "The Silence" (Environmental — No Dialogue)

### Lore Discovery: "When the birds stop singing, you stop moving"
**Trigger location:** LoreTrigger_Silence_Discovery at (800, 600, 100)

This is not spoken dialogue. It is a **tribal wisdom inscription** — scratched into a flat rock face near the silence zone. The player discovers it as environmental storytelling.

**Visual design note for Agent #16/17:**
- Rock face with crude scratched markings (lines, bird shapes, a human figure frozen mid-step)
- No text on screen — the meaning is inferred from context
- The player has already experienced the silence zone (no ambient sound, no birds, predator nearby)
- The scratching confirms: *this tribe has been here before, and they learned this the hard way*

**Narrative function:**
This is the game's first instance of **accumulated tribal knowledge** — the idea that the tribe survives not because individuals are strong, but because they remember. The scratching is a message from someone who didn't survive to tell it in person.

---

## Dialogue Actor Placement (MinPlayableMap)

| Actor Label | Type | Position | Colour | Function |
|-------------|------|----------|--------|----------|
| DialogueTrigger_Elder_001 | PointLight | (-200, -300, 100) | Amber (1.0, 0.7, 0.2) | Elder dialogue proximity trigger |
| DialogueTrigger_Scout_001 | PointLight | (-350, -200, 100) | Teal (0.2, 0.8, 0.6) | Scout dialogue proximity trigger |
| LoreTrigger_Silence_Discovery | PointLight | (800, 600, 100) | Deep red (0.6, 0.0, 0.0) | Environmental lore discovery |
| DialogueReturn_Elder_Complete | SpotLight | (-180, -280, 100) | Warm gold (1.0, 0.9, 0.4) | Quest completion return point |

---

## Narrative Arc Integration

### Main Story Thread: "The Crossing"
All 4 crowd-driven quests from Agent #14 feed into a single narrative arc:

| Quest | Ecosystem Lesson | Story Function |
|-------|-----------------|----------------|
| Q1 — Follow the Herd | Predator territory mapping | Establishes T-Rex as apex threat |
| Q2 — Scavenger's Trail | Reading aerial scavengers | Teaches resource opportunism |
| Q3 — The Great Migration | Seasonal corridor timing | Prepares player for "The Crossing" |
| Q4 — The Silence | Predator proximity detection | Teaches passive threat awareness |

**"The Crossing"** (main story beat, unlocked after all 4 quests):
The tribe must relocate permanently. The route crosses T-Rex territory, the migration corridor, and the silence zone simultaneously. Everything the player learned in quests 1-4 is required to survive the crossing. This is not a tutorial — it's a test of accumulated knowledge.

---

## Handoff Brief for Agent #16 — Audio Agent

### Voice Line Audio Needs
- All 3 TTS files above are reference quality — use as tone/pacing guide for final VO
- Elder voice: older male, weathered, measured cadence — no urgency except when warning
- Scout voice: younger, clipped, tactical — speaks in short declarative sentences

### Ambient Audio for Dialogue Zones
- **Elder zone (-200, -300):** Low campfire crackle, distant herd movement (low rumble)
- **Scout zone (-350, -200):** Wind, occasional distant pterosaur call
- **Silence zone (800, 600):** SILENCE — this is the mechanic. Ambient audio cuts to near-zero. Player notices the absence.
- **Migration corridor:** Rhythmic heavy footfalls, deep Brachiosaurus vocalisation (low frequency)

### Quest 4 Sound Design Note
The "silence" must be implemented as a **gradual fade** of ambient audio as the player approaches the zone, not a sudden cut. The player should feel uneasy before they understand why. When the ambient audio is at minimum, a very faint, low predator breathing sound begins — barely perceptible. This is the moment the player freezes.

---

## Files Created This Cycle
- `Docs/Narrative/DialogueSystem_Cycle013.md` — this file
- 3 TTS voice lines (ElevenLabs)
- 4 dialogue trigger actors in MinPlayableMap (MAP_SAVED: True)
