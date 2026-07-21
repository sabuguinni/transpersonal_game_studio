# Narrative & Dialogue Agent #15 — Cycle 009 Report

## Voice Lines Produced (4 total)

| Character | URL | Duration | Context |
|-----------|-----|----------|---------|
| CampElder_QuestGiver (full) | `tts/1781713538376_CampElder_QuestGiver.mp3` | ~32s | Quest 1 briefing — T-Rex valley crossing |
| Tracker_NPC (full) | `tts/1781713552730_Tracker_NPC.mp3` | ~21s | Quest 3 — raptor tracks, follow north |
| RiverCamp_Kael | `tts/1781713563954_RiverCamp_Kael.mp3` | ~21s | Quest 1 completion — Kael introduction |
| Narrator_LoreEntry | `tts/1781713569186_Narrator_LoreEntry.mp3` | ~28s | Tribal oral history — lore stone entry |

---

## Dialogue Trees

### NPC: Camp Elder (actor: `Narr_Camp_Elder_001`)
**Location:** (0, 0, 120) — Main camp centre

#### STATE: IDLE (before Quest 1 accepted)
```
[PLAYER approaches within 300 units]
Elder: "You are restless. Good. Restless people survive longer."
[PLAYER interacts]
Elder: "The scouts have returned. The T-Rex has crossed into our valley."
  → [OPTION A] "Tell me more."
      Elder: "The T-Rex crossed the ridge three nights ago. My grandfather saw the same tracks — wide as a man's chest, deep as a fist in wet clay. You want to warn the river camp? Then listen carefully. You do not run. You do not shout. You move like water — slow, low, and quiet."
      → QUEST 1 ACTIVATED: "Warn the Camp"
  → [OPTION B] "I am not ready."
      Elder: "No one is ever ready. That is why we go anyway."
      → [EXIT]
```

#### STATE: QUEST 1 ACTIVE
```
[PLAYER interacts]
Elder: "The river camp does not know yet. Every hour you wait is an hour they are exposed."
  → [OPTION A] "I am on my way."
      Elder: "Move before dark. The T-Rex hunts by sound at night — your footsteps carry further."
  → [OPTION B] "What if I encounter the T-Rex?"
      Elder: "Find a rock. A tree. Anything solid. Press yourself against it. Wait. Breathe. Survive."
```

#### STATE: QUEST 1 COMPLETE
```
[PLAYER returns after warning river camp]
Elder: "You came back. And Kael's people are safe. You did well."
  → Elder: "Kael is a good man. Stubborn, but good. He will remember this."
  → QUEST 2 UNLOCKED: "Survive the T-Rex"
  → Elder: "Now — the harder task. The T-Rex is still in the valley. We need to know where it sleeps."
```

---

### NPC: Tracker Kael (actor: `Narr_Tracker_Kael_001`)
**Location:** (-800, 600, 120) — River camp / Quest 3 anchor
**Character:** Male, ~50 years old. Pragmatic, dry humour. Has survived three T-Rex encounters. Respects competence above all.

#### STATE: FIRST MEETING (Quest 1 completion trigger)
```
Kael: "You made it. I did not think you would."
Kael: "My name is Kael. I have lived by this river for thirty years and I have never seen the herd scatter like that."
Kael: "Whatever is in that valley, it is not just one T-Rex. There are more. We need to talk."
  → [OPTION A] "How many?"
      Kael: "At least two. Maybe three. The tracks overlap — different sizes. One is old, one is young."
      Kael: "A young T-Rex is more dangerous than an old one. It has not learned patience yet."
  → [OPTION B] "What do you need from me?"
      Kael: "I need someone who can move quietly and observe without being observed."
      Kael: "I have been tracking a raptor pack. Something scared them off a fresh kill. I want to know what."
      → QUEST 3 UNLOCKED: "Witness the Scatter"
```

#### STATE: QUEST 3 BRIEFING
```
Kael: "Three toes. Deep. The raptor was running — not hunting, running."
Kael: "Something bigger scared it off its own kill. Follow the tracks north, past the second river bend."
Kael: "That is where you will find what frightened it. And friend — if you hear the ground shake before you see anything, do not wait to look. Move."
  → [OPTION A] "I understand."
      Kael: "Good. Come back with what you saw. Do not try to fight it — just watch."
  → [OPTION B] "What if the T-Rex is there?"
      Kael: "Then you have found your answer. And you had better be very quiet about it."
```

#### STATE: QUEST 3 COMPLETE
```
Kael: "So it was the old one. I thought so."
Kael: "The raptors will not go back to that territory for a season. That is useful to know."
Kael: "You have good eyes. And better instincts. Stay close to camp tonight — I want to show you something on the map."
  → UNLOCKS: Kael as permanent camp NPC with daily patrol route information
```

---

### LORE STONE (actor: `Narr_LoreStone_001`)
**Location:** (500, -300, 120) — Near camp perimeter
**Interaction:** Player examines carved stone markers

#### LORE ENTRY 1: "The Valley Crossing"
```
[Narrator voice — tribal oral history format]
"In the time before memory, when the valley was young, the great lizards ruled without challenge. 
Our people learned to read their signs — the broken trees, the silence of birds, the way the 
smaller creatures flee before the ground shakes. This knowledge was not written. It was spoken, 
from elder to child, beside every fire, in every season. To forget it was to die. To remember 
it was to live another day."
```

#### LORE ENTRY 2: "The Three Rules" (unlocked after Quest 1)
```
[Elder voice — instructional]
"The elders taught three rules for T-Rex country:
First — never run. A T-Rex sees movement, not stillness.
Second — never shout. Sound carries further than you think in open ground.
Third — never face it alone. If you must be in its territory, be two. One to watch, one to move."
```

#### LORE ENTRY 3: "Kael's First Hunt" (unlocked after meeting Kael)
```
[Kael voice — personal memory]
"I was fourteen the first time I saw a T-Rex up close. My father pushed me behind a boulder and 
told me not to breathe. I held my breath for what felt like an hour. When it passed, my father 
said: 'Now you know. That is the lesson. Everything else is just detail.'"
```

---

## NPC Character Bible: Kael

**Full name:** Kael (no family name — river people do not use them)
**Age:** ~50
**Role:** River camp tracker, Quest 3 giver, permanent camp NPC after Quest 1
**Personality:** Pragmatic, dry, observational. Does not waste words. Respects competence, not status.
**Backstory:** Born at the river camp. Has tracked every major predator in the valley over 30 years. Lost two hunting partners to T-Rex encounters — this is why he is methodical, not reckless.
**Dialogue style:** Short sentences. Concrete observations. Occasional dark humour. Never philosophical.
**Relationship to player:** Starts neutral → becomes trusted ally after Quest 3 → becomes information source for predator patrol routes

**Key Kael lines (for Audio Agent #16 to record):**
- "You made it. I did not think you would." (Quest 1 completion)
- "Three toes. Deep. The raptor was running." (Quest 3 briefing)
- "A young T-Rex is more dangerous than an old one. It has not learned patience yet." (lore)
- "Everything else is just detail." (lore stone entry 3)

---

## Quest Narrative Integration Map

```
Quest 1: "Warn the Camp"
  Giver: Camp Elder (Narr_Camp_Elder_001)
  Completion NPC: Kael (Narr_Tracker_Kael_001)
  Voice: tts/1781713538376_CampElder_QuestGiver.mp3
  Unlocks: Quest 2 + Kael as NPC

Quest 2: "Survive the T-Rex"
  Giver: Camp Elder (follow-up)
  No new NPC — uses existing T-Rex zone actors from Agent #12
  Unlocks: Quest 3

Quest 3: "Witness the Scatter"
  Giver: Kael (Narr_Tracker_Kael_001)
  Voice: tts/1781713552730_Tracker_NPC.mp3
  Completion: Kael (tts/1781713563954_RiverCamp_Kael.mp3)
  Unlocks: Kael patrol route info + Lore Stone entries 2 & 3
```

---

## UE5 Actors Placed This Cycle

| Label | Location | Type | Purpose |
|-------|----------|------|---------|
| `Narr_Camp_Elder_001` | (0, 0, 120) | PointLight marker | Camp Elder NPC dialogue anchor |
| `Narr_Tracker_Kael_001` | (-800, 600, 120) | PointLight marker | Kael NPC dialogue anchor |
| `Narr_LoreStone_001` | (500, -300, 120) | PointLight marker | Lore stone interaction point |

---

## Handoff to Audio Agent #16

### Priority voice recordings needed:
1. **Camp Elder** — full Quest 1 briefing (32s line recorded: `1781713538376`)
2. **Tracker Kael** — Quest 3 briefing (21s line recorded: `1781713552730`)
3. **Kael** — Quest 1 completion / first meeting (21s line recorded: `1781713563954`)
4. **Narrator** — Lore Entry 1 (28s line recorded: `1781713569186`)

### Ambient audio requests for narrative zones:
- **Camp Elder zone** (0,0,120): Low fire crackle, distant wind, occasional distant dinosaur call
- **River camp zone** (-800,600,120): River water, bird calls, rustling reeds
- **Lore Stone zone** (500,-300,120): Wind only, silence — contemplative

### Music cue requests:
- Quest 1 activation: Tense, low percussion, building
- Quest 1 completion (meeting Kael): Brief resolution chord, then neutral ambient
- Lore Stone interaction: Minimal, sparse — single instrument, no melody

---

## Decisions Made This Cycle

1. **Kael named as permanent NPC** — not a one-off quest giver. Rationale: river camp needs a named face for player to return to. Kael's tracker expertise makes him a natural long-term information source for predator patrol routes.

2. **Lore delivered via stone markers, not cutscenes** — Rationale: Kojima principle — use the act of playing to tell the story. Player must physically walk to the stone and interact. Lore is earned, not broadcast.

3. **Dialogue style: no exposition dumps** — Every line of dialogue contains either a survival instruction, a concrete observation, or a character beat. No philosophical monologues. No spiritual content.

4. **Three lore entries gated behind quest progression** — Prevents information overload at start. Player discovers lore as they earn trust with NPCs.
