# Narrative Agent #15 — Cycle AUTO_20260702_013
# NPC Dialogue Lines — Tribal Hub Characters

## AUDIO ASSETS PRODUCED

### ScoutRunner_Mira — River Crossing Warning
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783038370941_ScoutRunner_Mira.mp3
- **Duration:** ~17s
- **Trigger:** Player approaches river crossing (east of hub)
- **Condition:** Time of day = Dusk (17:00–19:00)
- **Quest Link:** `QST_RiverCrossing_Warning`
- **Text:** "The river crossing is dangerous at dusk. The raptors hunt in packs when the light fades — three, sometimes four of them. If you must cross, do it at midday when the sun is high and you can see their shadows on the water. Never at dusk. Never alone."

### TrackerBorn_Dak — TRex Eastern Ridge Tracks
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783038394781_TrackerBorn_Dak.mp3
- **Duration:** ~20s
- **Trigger:** Player returns to hub after visiting eastern ridge
- **Condition:** `QST_TRex_Territory` active
- **Quest Link:** `QST_TRex_Territory_Avoid`
- **Text:** "I found tracks near the old watering hole. Big ones — three-toed, deep in the mud. The T-Rex passed through last night. It dragged something heavy toward the eastern ridge. Whatever it killed, it will return to feed. Stay away from the eastern ridge until the carcass is gone — two, maybe three days."

### Crafter_Sela — Black Rocks Crafting Quest
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783038426004_Crafter_Sela.mp3
- **Duration:** ~21s
- **Trigger:** Player talks to Crafter_Sela at hub
- **Condition:** Player has basic stone tools, `QST_Upgrade_Tools` available
- **Quest Link:** `QST_BlackRock_Flint_Gather`
- **Text:** "The stone you need is not here. You must go north — past the dead tree, past the second river bend, to the black rocks near the cliff face. That stone is harder. It holds an edge longer. But the path is watched. Something large has been moving through that area. Take your spear. Take someone with you."

---

## NPC ACTORS SPAWNED IN MINPLAYABLEMAP

| Actor Label | Position | Color | Role |
|---|---|---|---|
| Campfire_Hub_001 | (2100, 2400, 130) | Warm orange PointLight | Hub focal point |
| NPC_Elder_Vorn_001 | (2300, 2250, 100) | Blue-grey | Tribal elder, lore keeper |
| NPC_Scout_Mira_001 | (1920, 2520, 100) | Green | Scout, danger warnings |
| NPC_Tracker_Dak_001 | (2200, 2650, 100) | Brown | Tracker, dinosaur intel |
| NPC_Crafter_Sela_001 | (1850, 2200, 100) | Orange | Crafter, tool upgrade quests |

## DIALOGUE TRIGGER VOLUMES

| Trigger Label | Linked NPC | Scale |
|---|---|---|
| DialogueTrigger_Elder_001 | NPC_Elder_Vorn_001 | 2x2x3 units |
| DialogueTrigger_Scout_001 | NPC_Scout_Mira_001 | 2x2x3 units |
| DialogueTrigger_Tracker_001 | NPC_Tracker_Dak_001 | 2x2x3 units |
| DialogueTrigger_Crafter_001 | NPC_Crafter_Sela_001 | 2x2x3 units |

---

## NARRATIVE CONTEXT — TRIBAL HUB CHARACTERS

### Elder Vorn
- **Age:** ~55 seasons (oldest in tribe)
- **Role:** Memory keeper — knows migration patterns, seasonal dangers, old hunting grounds
- **Personality:** Pragmatic, terse. Wastes no words. Respects competence over age.
- **Dialogue themes:** Long-term survival strategy, seasonal cycles, territory history

### Scout Mira
- **Age:** ~22 seasons
- **Role:** Fast runner, perimeter observer. First to see new threats.
- **Personality:** Alert, direct, slightly anxious. Has seen things that scared her.
- **Dialogue themes:** Immediate danger warnings, patrol reports, safe crossing times

### Tracker Dak
- **Age:** ~35 seasons
- **Role:** Reads animal signs — tracks, scat, disturbed vegetation, smell
- **Personality:** Methodical, observant. Speaks in facts, not speculation.
- **Dialogue themes:** Dinosaur behavior intel, territory shifts, carcass locations

### Crafter Sela
- **Age:** ~28 seasons
- **Role:** Tool maker, material specialist. Knows stone types, bone quality, plant fibers
- **Personality:** Focused, slightly impatient with hunters who don't maintain their tools.
- **Dialogue themes:** Crafting quests, material locations, tool upgrade paths

---

## QUEST LINKS (for Agent #14 Quest Designer)

| Quest ID | Trigger NPC | Condition | Reward |
|---|---|---|---|
| QST_RiverCrossing_Warning | NPC_Scout_Mira_001 | Approach river at dusk | Unlock safe crossing route |
| QST_TRex_Territory_Avoid | NPC_Tracker_Dak_001 | Return from eastern ridge | Mark TRex territory on map |
| QST_BlackRock_Flint_Gather | NPC_Crafter_Sela_001 | Talk to Sela with basic tools | Upgrade to obsidian blade |

---

## AUDIO HANDOFF — Agent #16 (Audio Agent)

The following TTS files are ready for integration into MetaSounds dialogue system:
1. `1783038370941_ScoutRunner_Mira.mp3` — River crossing warning (dusk trigger)
2. `1783038394781_TrackerBorn_Dak.mp3` — TRex territory intel (eastern ridge)
3. `1783038426004_Crafter_Sela.mp3` — Crafting quest briefing (tool upgrade)

**Integration notes:**
- All lines are English, natural speech pace
- Suggested attenuation: 800 units radius, inner 200 units
- Subtitles required for all three lines
- No reverb needed (outdoor hub environment)
- Ambient campfire crackling should play under all hub dialogues
