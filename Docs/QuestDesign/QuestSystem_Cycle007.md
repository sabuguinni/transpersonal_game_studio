# Quest & Mission System — Agent #14 — Cycle PROD_CYCLE_AUTO_20260617_007

## Overview
Four complete quest chains built on top of Agent #13's Crowd Simulation infrastructure.
All quest markers are placed as PointLight actors in MinPlayableMap with unique labels.

---

## QUEST 1: "THE MIGRATION" (Main Chain — 5 Stages)

### Concept
The player discovers a massive herbivore migration and must guide/protect the herd from
south plains to the northern water source. This is the backbone quest that teaches the
player about dinosaur ecology, predator zones, and terrain navigation.

### Emotional Arc
Discovery → Wonder → Tension → Danger → Relief/Triumph

### Stages & Actors
| Stage | Label | Location | Objective |
|-------|-------|----------|-----------|
| 1 | Quest_Migration_Stage1_001 | (-2200, -1800, 200) | Find the herd at south plains grazing zone |
| 2 | Quest_Migration_Stage2_001 | (0, -800, 200) | Observe raptor threat from Safe Post B |
| 3 | Quest_Migration_Stage3_001 | (1200, 400, 200) | T-Rex territory — trigger migration north |
| 4 | Quest_Migration_Stage4_001 | (-800, 1000, 200) | Raptor hunt zone — protect stragglers |
| 5 | Quest_Migration_Stage5_001 | (-1800, 2000, 200) | Northern water source — migration complete |

### Mechanics
- **Stage 1**: Player must approach within 800u of GrazingZone_Savanna_001 to spot herd
- **Stage 2**: Player observes from Safe Post B (0, -800) — stealth check, no detection
- **Stage 3**: Player enters T-Rex radius (1800u from MassAI_Exclusion_TRex_001) → triggers stampede
- **Stage 4**: Player must reach raptor zone and create distraction (throw rock/fire) to protect 1 herbivore
- **Stage 5**: Herd reaches GrazingZone_North_001 — quest complete, reward: migration map unlocked

### Reward
- Migration route permanently revealed on map
- Unlock "Tracker" skill: herbivore locations shown on compass
- Resource cache at northern water source (flint, berries, hide)

### Dialogue Hook
TribalElder voice line: "The herd moves east at dawn. Follow the migration trail and you will find water — and safety. But beware the raptors. They hunt the stragglers."
Audio: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781702548785_TribalElder.mp3

---

## QUEST 2: "STAMPEDE WARNING" (Timed Survival Quest)

### Concept
Player accidentally enters T-Rex territory, triggering a herbivore stampede directly
toward camp. Player must race back through the flee corridor before the herd arrives.
Failure = camp destroyed, resources lost.

### Emotional Arc
Panic → Desperation → Relief (if successful) / Devastation (if failed)

### Stages & Actors
| Stage | Label | Location | Objective |
|-------|-------|----------|-----------|
| Trigger | Quest_Stampede_Trigger_001 | (1400, 600, 250) | Enter T-Rex zone — stampede begins |
| CP1 | Quest_Stampede_CP1_001 | (600, -200, 200) | Flee corridor checkpoint 1 |
| CP2 | Quest_Stampede_CP2_001 | (-200, -600, 200) | Flee corridor checkpoint 2 |
| Camp | Quest_Stampede_Camp_001 | (0, 0, 250) | Reach camp before herd arrives |

### Mechanics
- **Timer**: 90 seconds from trigger to camp
- **Obstacle**: Raptor pack at (-800, 1000) may intercept flee route — player must detour
- **Failure condition**: Timer expires → camp takes 50% resource damage
- **Success condition**: Player reaches Quest_Stampede_Camp_001 with >10s remaining

### Reward (success)
- +20% camp defense (herd passes around camp)
- Unlock "Stampede Survivor" — stamina cost reduced 15% when sprinting

### Penalty (failure)
- 50% food/water resources destroyed
- Camp must be partially rebuilt

### Dialogue Hook
TribalWarrior voice line: "Stampede! The ground shakes — run! Get to high ground before the herd crushes everything in its path!"
Audio: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781702560056_TribalWarrior.mp3

---

## QUEST 3: "SAFE PASSAGE" (Escort Quest)

### Concept
A juvenile Parasaurolophus has been separated from the herd and is injured. Player must
escort it through the raptor danger zone back to the migration corridor without being
detected by the raptor pack.

### Emotional Arc
Compassion → Tension → Stealth/Strategy → Connection

### Stages & Actors
Uses existing actors:
- Start: GrazingZone_Savanna_001 (-2200, -1800) — find injured juvenile
- Danger: MassAI_DangerZone_Raptor_001 (-800, 1000) — raptor hunt zone (600u radius)
- Safe Post: Quest_RaptorObs_Post_001 (-1400, 800) — plan route around raptors
- End: Migration_Waypoint_003 (mid-corridor) — rejoin herd

### Mechanics
- **Juvenile AI**: Follows player at 80% speed, panics if raptor within 400u
- **Stealth window**: Raptors patrol 3 fixed routes — player must time crossing
- **Distraction option**: Throw meat/bone to redirect raptor alpha away from corridor
- **Fail condition**: Juvenile detected → raptors attack, juvenile flees randomly

### Reward
- Juvenile rejoins herd — herd size +1 (affects future migration quests)
- Unlock "Herbivore Bond" — herbivores no longer flee from player within 200u
- Rare hide drop from grateful herd (Parasaur hide — crafting material)

---

## QUEST 4: "RAPTOR PACK OBSERVATION" (Knowledge Quest)

### Concept
Player must observe the raptor pack's hunting behavior from safe observation posts
without being detected. Completing all 3 observation phases unlocks tactical knowledge
about raptor attack patterns — directly improving combat effectiveness.

### Emotional Arc
Curiosity → Tension → Understanding → Tactical Empowerment

### Stages & Actors
| Stage | Label | Location | Objective |
|-------|-------|----------|-----------|
| Post | Quest_RaptorObs_Post_001 | (-1400, 800, 200) | Reach safe observation post |
| Observe Alpha | Quest_RaptorObs_Alpha_001 | (-800, 1200, 200) | Watch raptor alpha frontal charge |
| Complete | Quest_RaptorObs_Complete_001 | (-1400, 800, 350) | 3 hunt phases observed — return to camp |

### Observation Phases (3 required)
1. **Flanking maneuver**: Beta + Gamma raptors circle prey while Alpha charges — 45s observation
2. **Pack communication**: Raptors use vocalizations to coordinate — player learns call patterns
3. **Kill technique**: Observe how raptors disable prey — reveals weak points in raptor combat

### Mechanics
- **Detection radius**: Player must stay >600u from MassAI_DangerZone_Raptor_001
- **Observation timer**: Each phase requires 45s of uninterrupted observation
- **Interruption**: If player moves during observation, phase resets

### Reward
- Unlock "Raptor Patterns" knowledge — combat UI shows raptor attack telegraphs
- Raptor weak point revealed: spine between shoulder blades (+150% damage)
- Unlock crafting recipe: Raptor Deterrent (smoke bundle — raptors avoid 300u radius)

---

## ACTOR INVENTORY (All Quest Actors in MinPlayableMap)

### Quest Migration (5 actors)
- Quest_Migration_Stage1_001 → (-2200, -1800, 200) — Green (0.2, 0.8, 0.1)
- Quest_Migration_Stage2_001 → (0, -800, 200) — Amber (0.8, 0.6, 0.0)
- Quest_Migration_Stage3_001 → (1200, 400, 200) — Orange-Red (0.9, 0.3, 0.0)
- Quest_Migration_Stage4_001 → (-800, 1000, 200) — Red (0.8, 0.2, 0.0)
- Quest_Migration_Stage5_001 → (-1800, 2000, 200) — Blue (0.0, 0.5, 1.0)

### Quest Stampede Warning (4 actors)
- Quest_Stampede_Trigger_001 → (1400, 600, 250) — Red (1.0, 0.1, 0.0)
- Quest_Stampede_CP1_001 → (600, -200, 200) — Orange (1.0, 0.5, 0.0)
- Quest_Stampede_CP2_001 → (-200, -600, 200) — Orange (1.0, 0.5, 0.0)
- Quest_Stampede_Camp_001 → (0, 0, 250) — Green (0.0, 1.0, 0.5)

### Quest Raptor Observation (3 actors)
- Quest_RaptorObs_Post_001 → (-1400, 800, 200) — Blue (0.3, 0.3, 1.0)
- Quest_RaptorObs_Alpha_001 → (-800, 1200, 200) — Red (0.8, 0.0, 0.0)
- Quest_RaptorObs_Complete_001 → (-1400, 800, 350) — Cyan (0.0, 0.8, 1.0)

**Total Quest Actors This Cycle: 12**
**Total Quest Actors All Cycles: ~20+ (including QuestHub, hunt markers from previous cycles)**

---

## AUDIO ASSETS

| Character | Line | URL |
|-----------|------|-----|
| TribalElder | Migration briefing | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781702548785_TribalElder.mp3 |
| TribalWarrior | Stampede warning | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781702560056_TribalWarrior.mp3 |

---

## HANDOFF TO AGENT #15 — Narrative & Dialogue Agent

### Quest Narrative Needs
1. **"The Migration"** — full dialogue tree for TribalElder quest giver (3 conversation nodes)
2. **"Stampede Warning"** — environmental storytelling: what happened to the camp if player fails?
3. **"Safe Passage"** — emotional beats: player names the juvenile? What does the herd's reaction mean?
4. **"Raptor Pack Observation"** — internal monologue as player watches raptors hunt (3 phases × 2 lines)

### Lore Hooks Available
- Migration route is the same one the tribe has followed for 3 generations — cultural significance
- Raptor pack has a named alpha: "Scarback" (distinctive scar on left flank) — recurring antagonist
- Northern water source is sacred to the tribe — first time player sees it is a story beat
- Stampede failure creates persistent world change — camp damage visible for 5 in-game days

### NPC Voice Lines Needed (Priority)
1. TribalElder: Quest completion line for "The Migration"
2. TribalWarrior: Raptor observation debrief ("Now you know how they think")
3. Ambient NPC: Reaction to stampede damage ("We rebuild. We always rebuild.")

---

## INTEGRATION WITH PREVIOUS SYSTEMS

### Agent #13 Crowd Simulation Dependencies
- Migration_Waypoint_001-006: Quest stages 1-5 use these as backbone
- MassAI_Exclusion_TRex_001: Stage 3 trigger zone
- MassAI_DangerZone_Raptor_001: Stage 4 danger zone + Raptor Obs quest
- Stampede_FleePoint_001-004: Stampede Warning flee corridor
- GrazingZone_Savanna_001: Quest 1 start + Quest 3 start
- GrazingZone_North_001: Quest 1 completion zone

### Agent #12 Combat AI Dependencies
- Raptor alpha/beta/gamma positions used in Observation quest
- T-Rex territory radius used in Stampede trigger
- Combat outcome affects quest state (if player kills T-Rex, stampede quest becomes unavailable)

### Agent #11 NPC Behavior Dependencies
- TribalElder NPC at QuestHub_TribalElder_001 (0, 0, 250) — quest giver
- NPC daily routine: Elder at camp at dawn/dusk, scouts during day
- Quest state tracked via NPC memory system

---

## DESIGN PRINCIPLES APPLIED

1. **Every quest teaches ecology**: Migration = herbivore behavior, Stampede = predator-prey dynamics, Safe Passage = pack hunting, Observation = raptor intelligence
2. **Failure has consequences**: Stampede failure = camp damage (persistent). Raptor detection = quest reset (immediate)
3. **Player agency**: Multiple approaches for each quest (distract, stealth, speed, sacrifice)
4. **Emotional resonance**: Juvenile Parasaur in Safe Passage creates attachment. Raptor Scarback creates recurring antagonist
5. **World integration**: All quests use existing crowd simulation actors — no orphaned systems
