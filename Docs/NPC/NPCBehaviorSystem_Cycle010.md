# NPC Behavior System — Agent #11 Cycle 010
## Transpersonal Game Studio — Prehistoric Survival Game

---

## CYCLE SUMMARY

**Cycle:** PROD_CYCLE_AUTO_20260617_010  
**Agent:** #11 — NPC Behavior Agent  
**Focus:** NPC survivor characters + behavior trigger zones + voice lines

---

## BEHAVIOR ZONES CREATED

| Label | Position | Purpose |
|-------|----------|---------|
| BehaviorZone_TRex_Territory_001 | (2000, 1500, 100) | T-Rex patrols this area — flee on sight |
| BehaviorZone_RaptorAmbush_001 | (-1200, 800, 100) | Raptor pack ambush point — flanking behavior |
| BehaviorZone_SafeCamp_NPC_001 | (0, 0, 100) | Safe zone — NPCs give dialogue and trade |
| BehaviorZone_WaterSource_001 | (800, -1500, 100) | Water source — herbivores gather, predators lurk |

---

## NPC CHARACTERS PLACED

| Label | Position | Role |
|-------|----------|------|
| NPC_Scout_001 | (-200, 150, 100) | Scout — patrols camp perimeter, warns of predators |
| NPC_Elder_001 | (50, -80, 100) | Elder — stationary, gives lore and survival tips |
| NPC_Hunter_001 | (300, 200, 100) | Hunter — leaves camp to hunt, returns at dusk |
| NPC_Gatherer_001 | (-100, -200, 100) | Gatherer — forages near camp edge, flees predators |

---

## VOICE LINES GENERATED

### Scout NPC (SurvivorNPC_Scout)
**Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781717464170_SurvivorNPC_Scout.mp3  
**Line:** "Stay low. The raptors hunt in packs — they will flank you from the left while the alpha charges from the front. Your only chance is to reach the tree line before they split."  
**Trigger:** Player enters BehaviorZone_RaptorAmbush_001

### Elder NPC (ElderNPC_Tribal)
**Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781717476450_ElderNPC_Tribal.mp3  
**Line:** "I have seen what happens to those who wander past the river bend at dusk. The big one — the one with the scarred eye — it waits there. Every night. It remembers the smell of humans."  
**Trigger:** Player interacts with NPC_Elder_001

---

## NPC BEHAVIOR DESIGN

### Daily Routine System (Design Spec)

```
NPC_Scout_001 — Daily Routine:
  06:00-08:00 → Wake, eat at camp fire
  08:00-18:00 → Patrol perimeter (waypoints: Waypoint_Camp_N, _E, _S, _W)
  18:00-20:00 → Return to camp, report to Elder
  20:00-22:00 → Rest at camp
  22:00-06:00 → Sleep (reduced awareness)
  
  THREAT RESPONSE:
    - Predator within 800u → Alert state → Warn player → Flee to camp
    - Player in danger → Call out warning dialogue
    - Night → Reduced patrol radius (400u from camp)

NPC_Hunter_001 — Daily Routine:
  06:00-07:00 → Prepare weapons at camp
  07:00-16:00 → Hunt in forest biome (radius 1500u from camp)
  16:00-18:00 → Return with resources
  18:00-22:00 → Process resources, socialize at camp
  22:00-06:00 → Sleep
  
  THREAT RESPONSE:
    - T-Rex within 2000u → Immediate flee to camp
    - Raptor within 500u → Combat stance → Call for help
    - Player nearby → Share hunting tips dialogue

NPC_Elder_001 — Daily Routine:
  All day → Stationary at camp center
  Morning → Tends fire, prepares medicine
  Afternoon → Teaches/advises any nearby survivors
  Evening → Tells stories around fire
  
  INTERACTION:
    - Player approach within 200u → Greeting dialogue
    - Player injured → Offer healing
    - Player asks about territory → Map knowledge dialogue

NPC_Gatherer_001 — Daily Routine:
  07:00-12:00 → Forage east of camp (radius 600u)
  12:00-13:00 → Return to camp, rest
  13:00-17:00 → Forage north of camp (radius 600u)
  17:00-18:00 → Return with berries/plants
  
  THREAT RESPONSE:
    - Any predator within 600u → Panic flee to camp
    - Player nearby → Share gathered food
```

### Memory System Design

NPCs remember:
- **Predator sightings:** Location + time of last predator encounter (fades after 2 in-game hours)
- **Player interactions:** Whether player helped/harmed them (persistent)
- **Resource locations:** Where they found food/water (shared with other NPCs)
- **Danger zones:** Areas where tribe members were killed (permanent avoidance)

### Social Dynamics

- NPCs talk to each other at camp (idle dialogue system)
- Hunter shares hunt results with Elder → Elder updates tribe knowledge
- Scout warns entire camp when predator detected → All NPCs respond
- Player reputation affects NPC dialogue options

---

## CUMULATIVE NPC ZONES (All Cycles)

### Cycle 007 — Raptor Patrol Waypoints
- Waypoint_Floresta_011 through Waypoint_Floresta_014 (forest patrol circuit)

### Cycle 008 — Raptor Pack AI Zones  
- Zone_RaptorHunt_001 (central pack hunt zone)
- Multiple raptor behavior zones

### Cycle 009 — Stealth + Behavior Zones
- BehaviorZone_Stealth_001 at (-600, 400, 100)

### Cycle 010 — Territory + NPC Characters (THIS CYCLE)
- 4 behavior trigger zones
- 4 NPC survivor characters
- 2 voice lines

---

## NEXT AGENT (#12 — Combat & Enemy AI)

Build on this foundation:
1. **T-Rex combat AI** — Use BehaviorZone_TRex_Territory_001 as patrol bounds
2. **Raptor flanking logic** — BehaviorZone_RaptorAmbush_001 is the trigger for pack split behavior
3. **NPC combat support** — NPC_Hunter_001 can assist player in combat (call for help system)
4. **Damage system** — NPCs can be killed by dinosaurs (permanent death, affects tribe morale)
5. **Alert propagation** — When Scout detects predator, all NPCs in 1500u radius enter alert state

**Key positions to respect:**
- Safe camp center: (0, 0, 100) — no predators should spawn within 800u
- T-Rex territory starts at (2000, 1500) — keep combat encounters in that region
- Raptor ambush zone: (-1200, 800) — ideal for scripted raptor encounter

---

*Generated by NPC Behavior Agent #11 — PROD_CYCLE_AUTO_20260617_010*
