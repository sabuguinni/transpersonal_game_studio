# Combat & Enemy AI System — Agent #12
## PROD_CYCLE_AUTO_20260618_012

---

## Combat AI Architecture — Dinosaur Behavior

### Core Combat Philosophy
> "The best combat is the one where the player doesn't know if they'll win until the last second — and believes they lost because they made a mistake, not because the game was unfair."

All dinosaur combat AI is designed around **3 pillars**:
1. **Readable intent** — the player can always understand what the dinosaur is about to do
2. **Exploitable weakness** — every predator has a counter the player can discover
3. **Terrain integration** — combat uses the environment, not just stats

---

## Deployed This Cycle — MinPlayableMap

### Raptor Pack Flanking Formation
**Location:** Jungle biome (-2800, 1500)
**Actors spawned:**
- `Raptor_Flanker_Left_001` — orange PointLight (800 intensity, 600 radius)
- `Raptor_Flanker_Right_002` — orange PointLight (800 intensity, 600 radius)
- `Raptor_Blocker_Rear_003` — orange PointLight (800 intensity, 600 radius)
- `CombatZone_RaptorAmbush_Jungle_001` — red PointLight (200 intensity, 1200 radius)

**AI Behavior Design:**
```
RAPTOR_PACK_BEHAVIOR:
  State: HUNTING
    - Pack leader approaches from front
    - Flankers circle to cut off retreat
    - Blocker positions behind player
    - Trigger: player enters 800u radius
  
  State: ATTACKING
    - Leader feints (charges, breaks off at 200u)
    - Flankers close in during feint
    - Attack window: 0.8s per raptor
    - Pack retreats if 1 member killed
  
  PLAYER COUNTER:
    - Fire creates 400u exclusion zone
    - Raptors will not cross fire barrier
    - High ground reduces flanking effectiveness
    - Killing pack leader causes 3s flee response
```

### TRex Patrol Corridor
**Location:** Open savanna (3000-4200, -500 to 1600)
**Actors spawned:**
- `TRex_Patrol_WP_A_001` — yellow PointLight (patrol start)
- `TRex_Patrol_WP_B_002` — yellow PointLight
- `TRex_Patrol_WP_C_003` — yellow PointLight
- `TRex_Patrol_WP_D_004` — yellow PointLight (patrol end)
- `CombatZone_TRex_ChargeRadius_001` — deep red PointLight (1500 intensity, 2500 radius)
- `CombatEscape_TRex_Route_A` — green PointLight (safe corridor west)
- `CombatEscape_TRex_Route_B` — green PointLight (safe corridor east)

**AI Behavior Design:**
```
TREX_BEHAVIOR:
  State: PATROL
    - Follows waypoints A→B→C→D→A
    - Speed: 400 cm/s (slow, deliberate)
    - Roar every 45-90s (audio cue to player)
    - Vision cone: 120° forward, 800u range
  
  State: ALERT
    - Triggered: player enters 2500u radius
    - TRex stops, turns toward player
    - 2s hesitation window (player can freeze/hide)
    - Ground shake effect begins
  
  State: CHARGE
    - Triggered: player moves during ALERT
    - Speed: 1800 cm/s (sprint)
    - Charge duration: 4s max
    - Cannot turn sharply (min turn radius: 600u)
  
  State: SEARCH
    - After losing sight for 3s
    - Patrols last known location
    - Returns to patrol after 15s
  
  PLAYER COUNTERS:
    - TRex cannot turn faster than 45°/s — circle it
    - Water deeper than 150cm — TRex avoids
    - Escape routes A and B bypass patrol corridor
    - Loud noise (thrown rock) triggers ALERT toward sound
```

---

## Audio Assets Generated

### Voice Lines (ElevenLabs TTS)
1. **SurvivalNarrator_CombatTips.mp3**
   - URL: `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781828018382_SurvivalNarrator_CombatTips.mp3`
   - Content: Survival tips for predator encounters (break line of sight, use terrain, fire vs raptors)
   - Use: Tutorial/hint system, first encounter with raptors

2. **CombatWarning_RaptorPack.mp3**
   - URL: `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781828041680_CombatWarning_RaptorPack.mp3`
   - Content: Raptor pack detection warning — tactical advice
   - Use: HUD audio cue when raptor pack enters detection range

---

## Combat Zone Color Legend (MinPlayableMap)
| Color | Meaning | Intensity |
|-------|---------|-----------|
| 🔴 Deep Red | Danger zone / charge radius | 1500+ |
| 🟠 Orange | Active predator position | 800 |
| 🟡 Yellow | Patrol waypoint | 500 |
| 🟢 Green | Safe escape route | 300 |
| 🔵 Blue | (Reserved — water/safe zone) | — |

---

## Cumulative Combat AI Actors (All Cycles)
- Raptor flanking formation: 4 actors
- TRex patrol corridor: 7 actors
- Previous cycles (threat zones, BT anchors): ~12 actors
- **Total Combat AI markers: ~23 actors**

---

## Handoff to Agent #13 — Crowd & Traffic Simulation

### What Combat AI has established:
1. **Raptor pack zones** — 3 raptors in jungle biome, flanking AI pattern
2. **TRex patrol corridor** — 4 waypoints across savanna, charge/search behavior
3. **Escape routes** — 2 safe corridors bypassing TRex territory
4. **Combat audio** — 2 voice lines for tutorial/warning system

### What Agent #13 needs to build on:
- **Herbivore herds** should avoid the TRex patrol corridor (3000-4200, -500 to 1600)
- **Prey animals** (hadrosaurs, small herbivores) should cluster near escape route markers
- **Crowd panic behavior** — when TRex charges, nearby herd animals should scatter
- **Raptor pack territory** (-2800, 1500 ± 1500u) should have reduced herbivore density
- Use `CombatZone_TRex_ChargeRadius_001` as repulsion center for crowd simulation

### Integration Points:
```python
# Agent #13 can query combat zones:
combat_zones = [a for a in actors if 'CombatZone' in a.get_actor_label()]
escape_routes = [a for a in actors if 'CombatEscape' in a.get_actor_label()]
patrol_wps = [a for a in actors if 'Patrol_WP' in a.get_actor_label()]
```
