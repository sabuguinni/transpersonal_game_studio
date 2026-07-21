# Quest & Mission Designer — Agent #14 — Cycle 008
## Transpersonal Game Studio — Prehistoric Survival Game

---

## QUEST SYSTEM OVERVIEW

### Architecture Philosophy
Every quest is a survival scenario with a clear emotional arc:
- **Setup**: The world presents a danger or opportunity
- **Tension**: Player must navigate real risk (predators, terrain, resources)
- **Resolution**: Success changes the player's position in the world

All quests use **point light actors** as in-world objective markers (visible in editor, replaceable with proper UI in production).

---

## ACTIVE QUESTS THIS CYCLE

### QUEST 1 — "Follow the Herd"
**Type**: Migration / Exploration  
**Risk Level**: Medium  
**Emotional Arc**: Hope → Uncertainty → Relief

**Objective Markers** (green lights):
| Actor Label | Location | Stage |
|---|---|---|
| Quest_FollowHerd_Obj1 | (-2200, -1800, 250) | Find herd tracks in southern savana |
| Quest_FollowHerd_Obj2 | (0, 500, 250) | Cross the river with the herd |
| Quest_FollowHerd_Obj3 | (2500, 3000, 250) | Reach northern plains |
| Quest_FollowHerd_Obj4 | (5000, 6000, 250) | **GOAL** — Establish camp near herd |

**Integration with Agent #13**:
- Uses `MigrationWP_001` through `MigrationWP_006` as path spine
- Herd density data from CrowdSimulationManager informs safe passage windows
- Quest fails if player triggers `ScatterTrigger_Savana_001` before reaching Obj2

**Rewards**:
- New camp location unlocked (northern plains)
- Brachiosaur herd becomes passive to player
- Resource bonus: 3x meat, 2x hide

---

### QUEST 2 — "Survive the Stampede"
**Type**: Survival / Escape  
**Risk Level**: High  
**Emotional Arc**: Panic → Desperation → Triumph

**Objective Markers** (orange lights):
| Actor Label | Location | Stage |
|---|---|---|
| Quest_Stampede_Obj1 | (-3000, -2500, 250) | Enter savana — herd is already agitated |
| Quest_Stampede_Obj2 | (1200, -800, 250) | Reach river bank before stampede hits |
| Quest_Stampede_Obj3 | (-1500, 2000, 250) | **GOAL** — Climb to high ground, survive |

**Integration with Agent #13**:
- `ScatterTrigger_Savana_001` fires when T-Rex enters zone
- Crowd scatter event activates: 200+ herbivores flee simultaneously
- Player has ~45 seconds to reach high ground before trampling damage

**Failure Condition**: Player caught in stampede path = death (instant kill zone)

**Rewards**:
- Stampede path leaves behind 5x carcasses (food + hide)
- T-Rex location revealed on mental map for 10 minutes

---

### QUEST 3 — "Hunt the Hunter"
**Type**: Hunt / Combat  
**Risk Level**: Extreme  
**Emotional Arc**: Determination → Fear → Catharsis

**Objective Markers** (red lights):
| Actor Label | Location | Stage |
|---|---|---|
| Quest_HuntTRex_Track1 | (3000, -1000, 250) | Find T-Rex tracks in northern territory |
| Quest_HuntTRex_Track2 | (4500, 500, 250) | Confirm T-Rex patrol territory boundary |
| Quest_HuntTRex_Ambush | (5500, 2000, 250) | **GOAL** — Set ambush at patrol choke point |

**Integration with Agent #13**:
- Uses `PatrolZone_TRex_North_001` boundary as territory reference
- T-Rex patrol route from Agent #12 (Combat AI) defines ambush window
- Player must wait for T-Rex to face away before striking

**Tactical Notes**:
- Spear throw range: 8m
- T-Rex detection radius: 15m (sound), 25m (sight)
- Recommended: 3 spears, stone axe, full stamina

**Rewards**:
- T-Rex tooth (rare crafting material — advanced weapon tier)
- Tribal elder dialogue unlocked: "You are no longer prey"
- Northern territory becomes accessible for exploration

---

### QUEST 4 — "Stealth Through the Ridge"
**Type**: Stealth / Exploration  
**Risk Level**: High  
**Emotional Arc**: Tension → Concentration → Release

**Objective Markers** (purple lights):
| Actor Label | Location | Stage |
|---|---|---|
| Quest_StealthRidge_Obj1 | (-500, 4000, 250) | Enter Carnotaurus ridge territory |
| Quest_StealthRidge_Obj2 | (1000, 5500, 250) | Navigate mid-passage (Carno patrols here) |
| Quest_StealthRidge_Obj3 | (2500, 7000, 250) | **GOAL** — Exit ridge, reach northern valley |

**Integration with Agent #13**:
- `PatrolZone_Carno_Ridge_001` defines the danger corridor
- Carnotaurus has 20m hearing radius — player must crouch-walk
- Wind direction mechanic: upwind = safe, downwind = detected

**Failure Condition**: Carnotaurus detects player = combat or flee
**Alternative Path**: Go around ridge (+2 hours game time, costs stamina)

**Rewards**:
- Northern valley access (new biome)
- Carno patrol pattern memorized (shown on mental map)

---

## QUEST PROGRESSION MAP

```
[Southern Savana — START]
        |
Quest_FollowHerd_Obj1 (find tracks)
        |
Quest_Stampede_Obj1 (danger event)
        |
Quest_FollowHerd_Obj2 (river crossing)
        |
Quest_StealthRidge_Obj1 (ridge entry)
        |
Quest_HuntTRex_Track1 (northern territory)
        |
Quest_FollowHerd_Obj3 (northern plains)
        |
Quest_HuntTRex_Ambush / Quest_StealthRidge_Obj3
        |
[Northern Valley — END ZONE]
```

---

## VOICE LINES PRODUCED

### TribalElder_QuestGiver
**Audio URL**: `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781707900774_TribalElder_QuestGiver.mp3`
**Line**: "The herd has moved on. Three days east, maybe four. If you want to eat this winter, you follow them now — or you stay here and hope the forest is kind to you."
**Quest**: Follow the Herd — briefing

### Narrator_QuestBriefing
**Audio URL**: `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781707914080_Narrator_QuestBriefing.mp3`
**Line**: "You tracked the beast for two days. Now it stands between you and the valley. One wrong step — one sound — and it charges. Hold your breath. Wait for the moment."
**Quest**: Hunt the Hunter — pre-ambush tension moment

---

## ACTOR INVENTORY (This Cycle)

| Actor Label | Type | Location | Quest |
|---|---|---|---|
| Quest_FollowHerd_Obj1 | PointLight (green) | (-2200,-1800,250) | Follow the Herd |
| Quest_FollowHerd_Obj2 | PointLight (green) | (0,500,250) | Follow the Herd |
| Quest_FollowHerd_Obj3 | PointLight (green) | (2500,3000,250) | Follow the Herd |
| Quest_FollowHerd_Obj4 | PointLight (gold) | (5000,6000,250) | Follow the Herd — GOAL |
| Quest_Stampede_Obj1 | PointLight (orange) | (-3000,-2500,250) | Survive the Stampede |
| Quest_Stampede_Obj2 | PointLight (orange) | (1200,-800,250) | Survive the Stampede |
| Quest_Stampede_Obj3 | PointLight (gold) | (-1500,2000,250) | Survive the Stampede — GOAL |
| Quest_HuntTRex_Track1 | PointLight (red) | (3000,-1000,250) | Hunt the Hunter |
| Quest_HuntTRex_Track2 | PointLight (red) | (4500,500,250) | Hunt the Hunter |
| Quest_HuntTRex_Ambush | PointLight (red) | (5500,2000,250) | Hunt the Hunter — GOAL |
| Quest_StealthRidge_Obj1 | PointLight (purple) | (-500,4000,250) | Stealth Through the Ridge |
| Quest_StealthRidge_Obj2 | PointLight (purple) | (1000,5500,250) | Stealth Through the Ridge |
| Quest_StealthRidge_Obj3 | PointLight (gold) | (2500,7000,250) | Stealth Through the Ridge — GOAL |

**Total actors spawned this cycle: 13**
**Map saved: True**

---

## INTEGRATION SPECS FOR AGENT #15 (Narrative & Dialogue)

### Dialogue Triggers Needed
1. **QuestHub_TribalElder_001** (from cycle 005) — needs 4 dialogue trees:
   - Quest offer: "Follow the Herd"
   - Quest offer: "Hunt the Hunter" (unlocks after Follow the Herd complete)
   - Quest offer: "Stealth Through the Ridge" (unlocks after river crossing)
   - Quest completion: "You are no longer prey" (after T-Rex hunt)

2. **NPC Witness Lines** — when player enters danger zones:
   - Entering `PatrolZone_TRex_North_001`: survivor NPC whispers warning
   - Entering `ScatterTrigger_Savana_001`: distant rumble + NPC shout "RUN!"
   - Completing `Quest_StealthRidge_Obj3`: NPC at valley entrance greets player

3. **Environmental Storytelling** — no dialogue, world tells the story:
   - Bones near Quest_HuntTRex_Track1 (previous hunter failed)
   - Trampled vegetation near Quest_Stampede_Obj1 (herd passed recently)
   - Claw marks on rocks near Quest_StealthRidge_Obj1 (Carno territory marking)

### Lore Hooks
- The northern valley was the tribe's original home — lost to T-Rex 3 generations ago
- The "Follow the Herd" quest is a rite of passage for young hunters
- Carnotaurus ridge is called "The Silence" — no one who shouts there survives

---

## TECHNICAL DECISIONS

1. **Point lights as quest markers**: Visible in editor, easily replaced with proper UMG/widget markers in production. Color coding: green=safe, orange=danger, red=combat, purple=stealth, gold=goal.

2. **No spiritual content**: All quests are purely survival-based — tracking, hunting, fleeing, navigating. Zero mystical elements.

3. **Quest progression is geographic**: Player moves south→north following migration corridor. Natural difficulty ramp from medium (Follow Herd) to extreme (Hunt the Hunter).

4. **Integration-first design**: Every quest references specific actors from Agents #12 and #13 (patrol zones, scatter triggers, migration waypoints). No quest exists in isolation.
