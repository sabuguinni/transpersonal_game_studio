# Dialogue Tree — Elder NPC: "First Tools" Quest
**Agent:** #15 — Narrative & Dialogue  
**Cycle:** PROD_CYCLE_AUTO_20260619_001  
**Quest Context:** Player has just arrived at the tribe camp. Elder initiates the "First Tools" quest.

---

## NPC Profile: The Elder

- **Role:** Tribe memory-keeper and practical leader. Not spiritual — a survivor who has outlived everyone else by being smarter and more cautious.
- **Voice:** Slow, deliberate, low energy. Every word costs something.
- **Motivation:** The tribe is weakening. Two hunters lost this season. He needs the player to prove they can contribute before winter.
- **Actor Label:** `NPC_Elder_001` (amber point light anchor at world origin)

---

## Voice Lines (ElevenLabs TTS — Produced This Cycle)

| ID | Character | Line | Audio URL | Duration |
|----|-----------|------|-----------|----------|
| VL_001 | Elder_Lore_Memory | "She was gone before the sun reached the ridge. Three days now, and no sign. The hunters found her spear near the river — broken clean. Whatever took her, it was fast. We do not speak her name until we know. That is our way." | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781830426453_Elder_Lore_Memory.mp3 | ~15s |
| VL_002 | Hunter_TRex_Warning | "Stay low. Do not run. If you run, it sees you as prey. Move slow, keep the wind at your back, and never — never — look it in the eyes." | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781830428888_Hunter_TRex_Warning.mp3 | ~9s |
| VL_003 | Elder_CraftSuccess | "Good. The axe holds weight. Now you are not just a survivor — you are a maker. The tribe remembers those who make things. They forget those who only take." | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781830435690_Elder_CraftSuccess.mp3 | ~11s |
| VL_004 | Elder_QuestFailureWarning | "The darkness is coming. You have not finished. If the night finds you without fire, without tools — the things that hunt in the dark will find you first. Move. Now." | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781830437873_Elder_QuestFailureWarning.mp3 | ~11s |

---

## Dialogue Tree: "First Tools" — Full Flow

### NODE_001 — Greeting (Quest Offer)
**Trigger:** Player enters `DialogueTrigger_Elder_001` radius (200 units from origin)  
**Condition:** Quest "First Tools" not yet started

> **Elder:** "You came from the east. Alone. That means you are either very fast or very lucky. We will find out which."  
> *(beat)*  
> "The tribe needs tools. Real tools. Not sharpened sticks. Rocks from the riverbank — the flat grey ones. And wood that does not bend. Bring both. I will show you what to do with them."

**Player Response Options:**
- [A] "Where is the riverbank?" → NODE_002a
- [B] "What happened to your other hunters?" → NODE_002b  
- [C] "I understand. I'll go." → NODE_003 (quest starts)

---

### NODE_002a — Directions
> **Elder:** "Follow the sound of water north. You will smell it before you see it. Stay on the high bank — the low bank floods at night and the raptors know it."

→ Continues to NODE_003

---

### NODE_002b — Lost Hunter Lore (triggers VL_001)
**Trigger:** Plays `Elder_Lore_Memory` audio  
> **Elder:** "She was gone before the sun reached the ridge. Three days now, and no sign. The hunters found her spear near the river — broken clean. Whatever took her, it was fast. We do not speak her name until we know. That is our way."  
> *(pause)*  
> "That is why we need tools. Better tools. Go."

→ Continues to NODE_003  
**Side effect:** Unlocks `LoreStone_LostHunter_001` — player can interact with it for additional lore

---

### NODE_003 — Quest Active Acknowledgment
**Trigger:** Quest "First Tools" starts  
> **Elder:** "Rocks and wood. Before dark. Do not make me come looking for you."

**Quest Objectives Activated:**
- Gather 4x Rock (near `Resource_Rock_001–004`)
- Gather 3x Stick (near `Resource_Stick_001–003`)
- Return to `CraftingStation_Campfire_001`

---

### NODE_004 — TRex Territory Warning (proximity to `LoreStone_TRexTerritory_001`)
**Trigger:** Player approaches lore stone at (2200, 1800) — red light marker  
**Plays:** `Hunter_TRex_Warning` audio  
**Source:** Nearby hunter NPC (not Elder)

> **Hunter:** "Stay low. Do not run. If you run, it sees you as prey. Move slow, keep the wind at your back, and never — never — look it in the eyes."

**Game Effect:** Displays on-screen text "T-Rex Territory — Proceed with caution"

---

### NODE_005 — Crafting Success (Quest Complete)
**Trigger:** Player crafts Stone Axe at `CraftingStation_Campfire_001`  
**Plays:** `Elder_CraftSuccess` audio

> **Elder:** "Good. The axe holds weight. Now you are not just a survivor — you are a maker. The tribe remembers those who make things. They forget those who only take."

**Rewards:**
- +25 Reputation with tribe
- Unlocks Quest: "First Hunt"
- Elder now gives additional dialogue options

---

### NODE_006 — Quest Failure State (Night arrives, quest incomplete)
**Trigger:** In-game time reaches nightfall AND quest not complete  
**Plays:** `Elder_QuestFailureWarning` audio

> **Elder:** "The darkness is coming. You have not finished. If the night finds you without fire, without tools — the things that hunt in the dark will find you first. Move. Now."

**Game Effect:** 
- Raptor aggression radius increases by 50%
- Player stamina drain rate increases (fear mechanic)
- Quest timer becomes visible on HUD

---

## Lore Context: Why Stone Axes Matter

The tribe has lost 2 hunters this season — one to a T-Rex near the eastern river, one to unknown causes (the "lost hunter" referenced in VL_001). Their remaining tools are worn down. Without stone axes:
- Cannot process large animal carcasses efficiently
- Cannot cut timber for shelter reinforcement before winter
- Cannot defend against raptors in close quarters

The Elder is not sentimental about this. He is pragmatic. The player is an asset or a liability — nothing more, until proven otherwise.

---

## World Anchor Actors (Placed This Cycle)

| Label | Location | Color | Purpose |
|-------|----------|-------|---------|
| `NPC_Elder_001` | (0, 0, 150) | Amber | Elder NPC position anchor |
| `LoreStone_LostHunter_001` | (800, -400, 100) | Cold Blue | Memory lore trigger |
| `LoreStone_TRexTerritory_001` | (2200, 1800, 100) | Red | Danger zone marker |
| `DialogueTrigger_Elder_001` | (0, 0, 50) | Yellow | Conversation radius |

---

## Dependencies for Audio Agent #16

The following audio assets need ambient/reactive sound design:
1. **Elder voice lines** — low reverb, cave/campfire acoustic space
2. **LoreStone_LostHunter_001** — subtle wind, distant raptor call when activated
3. **LoreStone_TRexTerritory_001** — deep rumble, ground vibration SFX
4. **DialogueTrigger_Elder_001** — soft interaction chime (stone/bone percussion)

Audio URLs above are ready for integration into MetaSounds cue system.
