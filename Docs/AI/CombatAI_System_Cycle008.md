# Combat & Enemy AI System — Production Cycle 008
**Agent #12 — Combat & Enemy AI**
**Cycle:** PROD_CYCLE_AUTO_20260618_008

---

## Overview

This cycle wires Agent #11's NPC Behavior Tree infrastructure to concrete combat mechanics.
All actors are deployed in `/Game/Maps/MinPlayableMap` via UE5 Python Remote Control.

---

## Combat Zones Deployed

### T-Rex Combat System
| Actor Label | Type | Location | Purpose |
|---|---|---|---|
| `Combat_TRex_DamageZone_001` | TriggerBox/Sphere | (3000,1500,150) | 300-unit proximity kill zone — player takes damage on entry |
| `Combat_TRex_SearchZone_001` | TriggerBox | (3200,1800,100) | LKP search area — T-Rex investigates when player breaks LoS |
| `Combat_TRex_ThreatRing_Attack` | TriggerSphere | (3000,1500,100) | Scale 1x — Immediate attack range (~100 units) |
| `Combat_TRex_ThreatRing_Chase` | TriggerSphere | (3000,1500,100) | Scale 3x — Chase activation range (~300 units) |
| `Combat_TRex_ThreatRing_Detect` | TriggerSphere | (3000,1500,100) | Scale 6x — Detection range (~600 units) |

**T-Rex Combat State Machine:**
```
IDLE → [Player enters ThreatRing_Detect] → ALERT
ALERT → [Player enters ThreatRing_Chase] → CHASE
CHASE → [Player enters ThreatRing_Attack] → ATTACK (damage tick: 25hp/s)
CHASE → [Player exits ThreatRing_Chase + enters SearchZone] → SEARCH
SEARCH → [30s elapsed, no player found] → PATROL → IDLE
```

### Raptor Pack Combat System (Coordinated Pincer)
| Actor Label | Type | Location | Purpose |
|---|---|---|---|
| `Combat_Raptor_Alpha_Attack_001` | TriggerBox | (-1500,2500,100) | Alpha drives player forward |
| `Combat_Raptor_Beta_Flank_001` | TriggerBox | (-1200,3000,100) | Beta flanks from right |
| `Combat_Raptor_Gamma_Ambush_001` | TriggerBox | (-1800,2800,100) | Gamma ambushes from left |
| `Combat_Raptor_AlphaCharge_Marker` | TargetPoint | (-1500,2500,200) | Alpha charge direction (facing 180°) |
| `Combat_Raptor_BetaFlank_Marker` | TargetPoint | (-1200,3000,200) | Beta flank direction (facing 270°) |
| `Combat_Raptor_GammaAmbush_Marker` | TargetPoint | (-1800,2800,200) | Gamma ambush direction (facing 90°) |

**Raptor Pincer Logic:**
```
Alpha enters BTZone_Raptor_Hunt_001 → DRIVES player toward Beta+Gamma
Beta activates BTZone_Raptor_Flank_001 → CUTS OFF right escape
Gamma activates Combat_Raptor_Gamma_Ambush_001 → CUTS OFF left escape
If player enters all 3 zones simultaneously → PACK_ATTACK (damage: 15hp/s each)
If player reaches high ground (Z > 400) → PACK_RETREAT (raptors cannot climb)
```

### Brachiosaurus Stampede System
| Actor Label | Type | Location | Purpose |
|---|---|---|---|
| `Combat_Brachio_StampedeZone_001` | TriggerBox | (500,-3000,100) | Entry triggers stampede warning |
| `Combat_Brachio_StampedePath_001` | TargetPoint | (500,-3000,100) | Stampede waypoint 1 |
| `Combat_Brachio_StampedePath_002` | TargetPoint | (800,-2500,100) | Stampede waypoint 2 |
| `Combat_Brachio_StampedePath_003` | TargetPoint | (1100,-2000,100) | Stampede waypoint 3 |
| `Combat_Brachio_StampedePath_004` | TargetPoint | (1400,-1500,100) | Stampede waypoint 4 |

**Brachio Stampede Logic:**
```
Player enters Combat_Brachio_StampedeZone_001 (within 200 units of BTZone_Brachio_Graze_001)
→ STAMPEDE_WARNING (rumble VFX, screen shake)
→ Brachiosaurus follows StampedePath_001→004 at 800 cm/s
→ Player hit by Brachio during stampede: KNOCKBACK (launch velocity 1200 cm/s) + 40hp damage
→ Player clears path before Brachio reaches them: STAMPEDE_PASS (Brachio returns to GRAZE)
```

---

## Voice Lines (ElevenLabs TTS)

| File | URL | Trigger |
|---|---|---|
| `CombatNarrator_TRexChase.mp3` (~12s) | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781782522009_CombatNarrator_TRexChase.mp3 | Player enters Combat_TRex_ThreatRing_Chase |
| `CombatNarrator_RaptorPincer.mp3` (~13s) | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781782553445_CombatNarrator_RaptorPincer.mp3 | Player enters all 3 Raptor zones simultaneously |

### Voice Line Scripts
**TRexChase:** *"You are bleeding. The T-Rex has you in its sightline. Every step you take, it adjusts. Your only chance — find a gap in the treeline and break its line of sight. Move. Now."*

**RaptorPincer:** *"Three of them. They have split. One in front, two behind you. This is not random — this is a hunt. Drop everything that makes noise. Find high ground. Raptors cannot follow you up a cliff face."*

---

## Integration with Agent #11 NPC Behavior Zones

| Agent #11 Zone | Agent #12 Combat Wire |
|---|---|
| `BTZone_TRex_Aggro_001` | → Activates `Combat_TRex_ThreatRing_Chase` + plays TRexChase voice |
| `BTZone_TRex_Patrol_001` | → Defines boundary for PATROL state, T-Rex returns here after SEARCH |
| `BTZone_Raptor_Hunt_001` | → Triggers Alpha charge + activates Beta/Gamma flankers |
| `BTZone_Raptor_Flank_001` | → Beta flank activation (Combat_Raptor_Beta_Flank_001) |
| `BTZone_Brachio_Graze_001` | → Stampede trigger when player enters within 200 units |
| `NPC_Memory_TRex_LKP_001` | → T-Rex navigates to this point during SEARCH state |
| `NPC_Memory_Raptor_Ambush_001` | → Gamma raptor spawn/wait position |

---

## Damage Values (Combat Balance)

| Dinosaur | Attack | Damage/s | Knockback | Player Counter |
|---|---|---|---|---|
| T-Rex | Bite | 25 hp/s | 2000 cm/s | Break LoS, hide in dense foliage |
| Raptor Alpha | Slash | 15 hp/s | 800 cm/s | High ground (Z > 400) |
| Raptor Beta | Slash | 15 hp/s | 800 cm/s | High ground (Z > 400) |
| Raptor Gamma | Slash | 15 hp/s | 800 cm/s | High ground (Z > 400) |
| Brachiosaurus | Stampede | 40 hp (one-shot) | 1200 cm/s | Clear stampede path |

---

## Next Agent — #13 Crowd & Traffic Simulation

Agent #13 should build on this infrastructure:
1. **Herd dynamics** — Brachiosaurus herd of 5-8 individuals using `Combat_Brachio_StampedePath_*` waypoints
2. **Raptor pack spawning** — Mass AI spawning 3-6 raptors that share the `Combat_Raptor_*` zone awareness
3. **Prey animal crowds** — Herbivore herds that scatter when T-Rex enters `Combat_TRex_ThreatRing_Detect`
4. **Crowd panic system** — When T-Rex is in CHASE state, all nearby herbivores enter FLEE state
5. **Density limits** — Max 5 T-Rex, 15 Raptors, 8 Brachiosaurus in MinPlayableMap simultaneously
