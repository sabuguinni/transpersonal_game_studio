# Narrative & Dialogue System — Agent #15 Production Cycle 004
**PROD_CYCLE_AUTO_20260618_004**

---

## Voice Lines Produced This Cycle

| Character | File URL | Duration | Context |
|-----------|----------|----------|---------|
| Elder Kael (Intro) | `tts/1781762531893_Elder_Kael.mp3` | ~40s | Main story exposition — why T-Rex is in the valley |
| Tracker Mira (Raptor Lore) | `tts/1781762552226_Tracker_Mira.mp3` | ~36s | Raptor pack backstory — displaced by T-Rex |
| Elder Kael (Quest Complete) | `tts/1781762584146_Elder_Kael_QuestComplete.mp3` | ~24s | Raptor hunt quest completion reward |

---

## Dialogue Scripts — Full Text

### ELDER KAEL — Main Story Exposition (Intro)
> *"Listen well, young one. Before you came to us, this valley was silent. The great lizard — the one with the tiny arms and the roar that shakes the ground — it lived far to the east, beyond the black rocks. We did not see it. We did not need to. But three moons ago, the prey animals began moving west. The deer, the horses, the giant long-necks — all moving west. And where the prey goes, the hunters follow. Now the great lizard hunts our valley. Our people have died. Our hunters are fewer. You are here because we need someone who does not yet know fear. Someone the valley has not broken."*

**Trigger**: Player approaches `NarrNode_Elder_Intro` (0,0,150) for the first time  
**Condition**: Quest "Survive the Valley" not yet started  
**Effect**: Unlocks all 4 quest givers on map

---

### ELDER KAEL — Quest Active
> *"The valley waits for no one. What you started — finish it. Come back when it is done."*

**Trigger**: Player re-approaches Elder while any quest is active  
**Actor**: `NarrNode_Elder_QuestActive` (80,60,140)

---

### ELDER KAEL — Quest Complete
> *"You did it. The raptors are gone — or at least, the ones that threatened us. The valley can breathe again. I did not think you would come back. Most do not, the first time they face the pack. You are different. Come — sit by the fire. There is food. And tomorrow, there will be more work. There is always more work. But tonight, you have earned rest."*

**Trigger**: Player returns to camp after completing "Hunt the Raptor Pack"  
**Actor**: `NarrNode_Elder_QuestComplete` (-80,60,140)

---

### ELDER KAEL — Quest Fail
> *"They are gone. The gatherers — gone. I should not have sent them. I should not have trusted that the path was safe. This is on me. But grief changes nothing. We need to know what happened. Go back. Find out."*

**Trigger**: Player returns without completing escort objective (gatherers dead)  
**Actor**: `NarrNode_Elder_QuestFail` (0,-100,140)

---

### TRACKER MIRA — Greeting
> *"You are the outsider the Elder speaks of. Good. We need people who do not know which paths are safe — because the safe paths have changed. Everything has changed."*

**Trigger**: Player first approaches `NarrNode_Tracker_Greeting` (1800,-1200,160)  
**Actor**: `QuestGiver_Tracker_Forest` (Agent #14)

---

### TRACKER MIRA — Raptor Pack Lore
> *"You want to know why the raptor pack moved closer? I will tell you. Two seasons ago, a pack of raptors lived in the eastern canyon — eight of them, maybe ten. They hunted the small deer near the canyon walls. Good territory. Plenty of prey. Then the great lizard came east. It does not share territory. It never does. The raptors fled west — into our valley. They are not here because they want to be near us. They are here because they have nowhere else to go. Hungry. Frightened. And frightened hunters are the most dangerous kind."*

**Trigger**: Player asks about the raptor pack (dialogue option)  
**Actor**: `NarrNode_Tracker_RaptorLore` (1900,-1100,150)

---

### TRACKER MIRA — Hunt Quest Active
> *"Three raptors. The alpha is the largest — dark scales, moves slower than the others but hits harder. The other two flank. Do not let them surround you. Use the trees."*

**Trigger**: Player has accepted "Hunt the Raptor Pack" and returns to Tracker  
**Actor**: `NarrNode_Tracker_HuntActive` (1700,-1300,150)

---

### TRACKER MIRA — Hunt Quest Complete
> *"The alpha is dead. The others will scatter — for now. They may come back when they are hungry enough. But we have time. That is all we ever have — time."*

**Trigger**: Player returns after killing all 3 raptor objectives  
**Actor**: `NarrNode_Tracker_HuntComplete` (1750,-1050,150)

---

## Water Gatherers Backstory

### Narrative Context
The three missing gatherers (`QuestObj_Water_Start/Mid/End`) are:
- **Dara** — eldest, knows the river paths, injured her leg crossing the eastern stones
- **Finn** — young, fast runner, sent ahead to scout — last seen near the eastern bank bend
- **Reka** — carries the water vessels, practical, would not have run unless something forced her to

### What Happened (Revealed Through Quest)
The raptors drove them from the usual water path. Dara hid in the reeds (survives if player reaches her in time). Finn ran east — into raptor territory (dies before player arrives, body found at `QuestObj_Raptor_Alpha` location). Reka made it to the river crossing but is pinned by a raptor at `QuestObj_Water_End`.

**Narrative payoff**: Player finds Reka alive → she tells them about Finn → player finds Finn's body → understands the raptor threat is personal, not abstract.

---

## Story Beat Markers (UE5 Actors)

| Actor Label | Location | Purpose |
|-------------|----------|---------|
| `StoryBeat_TRex_FirstWarning` | (-500,-800,130) | Signs of T-Rex presence — tracks, broken trees, scattered bones |
| `StoryBeat_OldCamp_Ruins` | (2500,500,110) | Previous tribe camp — destroyed 3 moons ago when T-Rex arrived |
| `NarrNode_Gatherers_LastSeen` | (1200,800,160) | Last confirmed sighting of the 3 gatherers |
| `NarrNode_Gatherers_Survivor` | (1400,900,140) | Where Reka is found alive |

---

## Narrative Actor Registry (All Cycles)

### Cycle 001 (NarrTrigger_* actors)
- `NarrTrigger_ElderIntro` — camp center
- `NarrTrigger_ValleyHistory` — hilltop
- `NarrTrigger_RaptorWarning` — forest edge
- `NarrTrigger_TRexFirstSight` — eastern ridge
- `NarrTrigger_WaterSource` — riverbank
- `NarrTrigger_OldCamp` — ruins
- `NarrTrigger_SurvivorFound` — cave entrance

### Cycle 002 (NarrZone_* actors)
- `NarrZone_CampFire_Dialogue` — main camp
- `NarrZone_Riverbank_Ambush` — river
- `NarrZone_Forest_Stalk` — forest
- `NarrZone_Hilltop_Refuge` — hilltop
- `NarrZone_TRex_Territory` — eastern ridge

### Cycle 003 (StoryBeat_* + NarrNode_* actors)
- `StoryBeat_Day1_Arrival`
- `StoryBeat_FirstHunt`
- `StoryBeat_TribeContact`
- `NarrNode_Camp_Welcome`
- `NarrNode_River_Danger`
- `NarrNode_Forest_Tracks`

### Cycle 004 (This cycle — NarrNode_* + StoryBeat_*)
- `NarrNode_Elder_Intro`
- `NarrNode_Elder_QuestActive`
- `NarrNode_Elder_QuestComplete`
- `NarrNode_Elder_QuestFail`
- `NarrNode_Tracker_Greeting`
- `NarrNode_Tracker_RaptorLore`
- `NarrNode_Tracker_HuntActive`
- `NarrNode_Tracker_HuntComplete`
- `NarrNode_Gatherers_LastSeen`
- `NarrNode_Gatherers_Survivor`
- `StoryBeat_TRex_FirstWarning`
- `StoryBeat_OldCamp_Ruins`

---

## Handoff to Agent #16 — Audio Agent

### Voice Line Assets Ready for Integration
All 3 voice lines this cycle are production-ready MP3s hosted on Supabase:

1. **Elder Kael Intro** — `tts/1781762531893_Elder_Kael.mp3`
   - Attach to: `NarrNode_Elder_Intro` actor
   - Trigger radius: 300 units
   - Play once per session

2. **Tracker Mira Raptor Lore** — `tts/1781762552226_Tracker_Mira.mp3`
   - Attach to: `NarrNode_Tracker_RaptorLore` actor
   - Trigger radius: 250 units
   - Play on dialogue option selection

3. **Elder Kael Quest Complete** — `tts/1781762584146_Elder_Kael_QuestComplete.mp3`
   - Attach to: `NarrNode_Elder_QuestComplete` actor
   - Trigger: Quest "Hunt the Raptor Pack" completed flag

### Ambient Audio Requests for Agent #16
- **Camp fire** (`NarrNode_Elder_Intro` area): crackling fire, distant murmurs, occasional owl
- **Forest edge** (`NarrNode_Tracker_*` area): wind through trees, distant raptor calls (quiet, distant)
- **Riverbank** (`NarrNode_Gatherers_*` area): flowing water, reeds, tension sting when approaching
- **Old camp ruins** (`StoryBeat_OldCamp_Ruins`): silence, wind, occasional creak of broken wood
- **T-Rex warning zone** (`StoryBeat_TRex_FirstWarning`): deep rumble in distance, birds fleeing

### MetaSounds Integration Notes
- All dialogue triggers should use MetaSounds for spatial audio
- Dialogue lines: 3D spatialized, max distance 500 units
- Ambient loops: 2D background layer at 30% volume under dialogue
- T-Rex warning zone: low-frequency rumble (20-60Hz) felt more than heard
