# Narrative Dialogue Database — Cycle PROD_CYCLE_AUTO_20260624_002
## Agent #15 — Narrative & Dialogue Agent

---

## ACTIVE DIALOGUE TRIGGERS (MinPlayableMap)

### NarrTrigger_CampEntrance
- **Location**: (200, 0, 50)
- **Speaker**: Elder Mara
- **Line**: "The raptors left tracks near the eastern ridge last night. Three sets — a hunting pack. Do not go east alone."
- **Condition**: First time player enters camp radius
- **Priority**: HIGH

### NarrTrigger_RiverCrossing
- **Location**: (800, 400, 30)
- **Speaker**: Scout Dren
- **Line**: "Brachiosaurus herd, moving north. Forty, maybe fifty of them. Where the big ones go, the predators follow."
- **Condition**: Player reaches river crossing
- **Priority**: MEDIUM

### NarrTrigger_AncientKill
- **Location**: (-600, 300, 40)
- **Speaker**: Environmental (no speaker — player discovery)
- **Line**: [NO VOICE — visual discovery] Massive bones half-buried in mud. The skull is three metres wide. Whatever killed this was bigger.
- **Condition**: Player approaches ancient kill site
- **Priority**: LOW

---

## NPC ROSTER (Active in Map)

### Elder Mara — NarrNPC_ElderMara
- **Role**: Tribal elder, knowledge keeper (practical — tracks, seasons, animal behaviour)
- **Location**: Camp area (150, -80, 120)
- **Personality**: Blunt, experienced, no patience for recklessness
- **Key Dialogue Lines**:
  1. "You survived the night. Good. Now make yourself useful."
  2. "The T-Rex hunts by smell, not sight. Stay downwind."
  3. "We lost two hunters last season. Their bones are still out there. Learn from them."
  4. "Rain is coming. The raptors go quiet before a storm — that is when they are most dangerous."

### Scout Dren — NarrNPC_ScoutDren
- **Role**: Young scout, fast runner, territorial knowledge
- **Location**: River crossing area (350, 120, 120)
- **Personality**: Nervous energy, observant, speaks in short bursts
- **Key Dialogue Lines**:
  1. "River's high. Crossing will be slow. Slow is dead."
  2. "Saw three raptors on the ridge. They were watching the camp."
  3. "The big one — the T-Rex — it has a territory. Stay out of the valley."
  4. "Fresh tracks. Big. Heading south. We need to move."

---

## NARRATIVE ARC — ACT 1 (Survival Foundation)

### Beat 1: Arrival (Player Start)
Player wakes alone. No tribe. No tools. Must find shelter before dark.
- **Objective**: Survive first night
- **Tone**: Disorientation, vulnerability

### Beat 2: First Contact
Player finds Elder Mara's camp. Small group of survivors.
- **Objective**: Prove worth to the tribe (bring food/materials)
- **Tone**: Cautious hope, pragmatic evaluation

### Beat 3: The Threat
Raptor pack attacks camp perimeter. Scout Dren is injured.
- **Objective**: Drive back the raptors / rescue Dren
- **Tone**: Urgent, dangerous, first real combat

### Beat 4: The Territory Question
T-Rex is expanding its territory. Camp is in its path.
- **Objective**: Find new camp location OR find way to deter the T-Rex
- **Tone**: Strategic, existential stakes

---

## DIALOGUE SYSTEM ARCHITECTURE

### Trigger Types
- **ProximityTrigger**: Player enters radius → dialogue fires
- **EventTrigger**: Game event (combat, discovery) → dialogue fires
- **TimeTrigger**: Time of day → ambient dialogue fires
- **QuestTrigger**: Quest state change → dialogue fires

### Voice Line Status
- text_to_speech API: QUOTA_EXCEEDED (2 credits remaining)
- Fallback: All lines stored as text in this database
- Audio Agent (#16) to implement MetaSounds dialogue playback from text data

### Integration Points
- Quest Agent (#14): Quest objectives reference NPC dialogue nodes
- Audio Agent (#16): Voice line playback via MetaSounds
- NPC Behavior Agent (#11): Dialogue conditions tied to NPC state

---

## FILES CREATED THIS CYCLE
- `Content/Narrative/DialogueDatabase_Cycle024.md` — this file
- UE5: NarrTrigger_CampEntrance spawned at (200,0,50)
- UE5: NarrTrigger_RiverCrossing spawned at (800,400,30)
- UE5: NarrTrigger_AncientKill spawned at (-600,300,40)
- UE5: NarrNPC_ElderMara text label at (150,-80,120)
- UE5: NarrNPC_ScoutDren text label at (350,120,120)

---
*Generated: PROD_CYCLE_AUTO_20260624_002 | Agent #15 Narrative & Dialogue*
