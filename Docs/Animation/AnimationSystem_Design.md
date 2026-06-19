# Animation System Design — Transpersonal Game Studio
## Agent #10 — Animation Agent | Cycle PROD_CYCLE_AUTO_20260618_012

---

## Overview

This document defines the complete animation architecture for the prehistoric survival game.
Every character moves with weight, intention, and biological authenticity.
The animation system is built on UE5's Motion Matching framework with procedural IK layering.

---

## Player Character Locomotion State Machine

### States

| State | Condition | Blend Time | Notes |
|-------|-----------|------------|-------|
| Idle | Speed < 10 | 0.25s | Breathing, micro-movements, alert scanning |
| Walk | 10 < Speed < 180 | 0.2s | Weight-forward, cautious prehistoric gait |
| Run | 180 < Speed < 300 | 0.15s | Arms pumping, forward lean |
| Sprint | Speed > 300 | 0.1s | Full exertion, survival desperation |
| Crouch_Idle | Crouched + Speed < 10 | 0.3s | Stealth posture, head scanning |
| Crouch_Walk | Crouched + Speed > 10 | 0.25s | Silent movement, arms low |
| Jump_Start | JumpPressed | 0.1s | Explosive push-off |
| Jump_Loop | InAir + VelZ > 0 | 0.05s | Apex float |
| Fall | InAir + VelZ < -200 | 0.1s | Arms out for balance |
| Land_Soft | Landing + Speed < 200 | 0.2s | Knee bend absorption |
| Land_Hard | Landing + Speed > 200 | 0.3s | Full body impact recovery |
| Swim | InWater | 0.4s | Primitive breaststroke |
| Climb | OnLedge | 0.2s | Scramble up rocky surface |
| Exhausted | Stamina < 15% | 0.5s | Hunched, labored breathing overlay |
| Injured | Health < 30% | 0.4s | Limping, arm clutching wound |

### Movement Parameters (Configured via Python)
- **MaxWalkSpeed**: 300 cm/s
- **MaxWalkSpeedCrouched**: 150 cm/s
- **JumpZVelocity**: 420 cm/s
- **GravityScale**: 1.75 (heavier feel, prehistoric weight)
- **MaxAcceleration**: 2048 cm/s²
- **BrakingFrictionFactor**: 2.0
- **RotationRate**: 540°/s yaw
- **OrientRotationToMovement**: true

---

## Blend Spaces

### BS_Locomotion_Ground (2D Blend Space)
- **X Axis**: Direction (-180° to 180°) — for strafing
- **Y Axis**: Speed (0 to 600 cm/s)
- **Samples**:
  - (0, 0) → Idle
  - (0, 150) → Walk_Forward
  - (0, 300) → Run_Forward
  - (0, 500) → Sprint_Forward
  - (90, 150) → Walk_Right
  - (-90, 150) → Walk_Left
  - (180, 150) → Walk_Backward
  - (90, 300) → Run_Right
  - (-90, 300) → Run_Left

### BS_Crouch_Locomotion (1D Blend Space)
- **Axis**: Speed (0 to 200 cm/s)
- **Samples**:
  - (0) → Crouch_Idle
  - (100) → Crouch_Walk
  - (200) → Crouch_Run

---

## Procedural IK System

### Foot IK
- **Two-bone IK** on both legs
- **Trace distance**: 80cm downward from foot bone
- **Max step height**: 45cm
- **Blend weight**: 1.0 on ground, 0.0 in air (blends over 0.15s)
- **Hip adjustment**: Pelvis shifts down when one foot is lower than the other
- **Terrain adaptation**: Rotates foot to match slope normal

### Hand IK (Weapon/Tool Holding)
- **Two-bone IK** on both arms when holding tools
- **IK target**: Weapon grip socket
- **Left hand IK**: Follows left-hand grip socket on weapon mesh

### Look-At IK (Head/Spine)
- **Spine01, Spine02, Neck, Head** bones rotate toward look target
- **Max angle**: 70° horizontal, 40° vertical
- **Blend**: 0.6 weight (not fully locked to target — natural feel)
- **Priority**: Predator threats > prey > environment > forward

---

## Dinosaur Animation System

### T-Rex Locomotion
- **Idle**: Slow head sway, tail counterbalance, nostril flare
- **Walk**: 3-4 mph, bipedal, tail horizontal for balance
- **Run**: 12-15 mph, forward lean, massive ground impact
- **Attack**: Lunge + bite montage, 1.2s total
- **Roar**: Full-body expansion, 2.5s, triggers fear response in player

### Raptor Pack Locomotion
- **Idle**: Alert scanning, weight shifting, feather ruffling
- **Walk**: Precise foot placement, head bobbing
- **Run**: 25 mph, agile, direction changes mid-stride
- **Jump_Attack**: Leap + claw rake, 0.8s montage
- **Flank**: Coordinated circling behavior, synchronized with pack AI

### Brachiosaurus Locomotion
- **Walk**: Slow, 4-legged, massive ground tremor on each step
- **Graze**: Neck extends down, slow head sweep
- **Alert**: Head raises, slow turn toward threat
- **Flee**: Rare — thunderous gallop, causes screen shake

---

## Animation Montages

### Player Montages
| Montage | Duration | Slot | Trigger |
|---------|----------|------|---------|
| AM_Attack_Spear | 1.1s | UpperBody | LMB with spear |
| AM_Attack_Club | 0.9s | UpperBody | LMB with club |
| AM_Throw_Rock | 0.7s | UpperBody | RMB with rock |
| AM_Craft_Start | 2.0s | FullBody | Open crafting menu |
| AM_Eat_Food | 1.8s | FullBody | Consume food item |
| AM_Drink_Water | 1.5s | FullBody | Drink from source |
| AM_Death | 3.0s | FullBody | Health reaches 0 |
| AM_Interact | 0.6s | UpperBody | E key on interactable |
| AM_Pickup | 0.8s | FullBody | Pick up ground item |
| AM_HideInBush | 1.2s | FullBody | Stealth hide trigger |

---

## Animation Zones (Placed in MinPlayableMap)

| Zone Label | Location | Trigger | Animation Override |
|-----------|----------|---------|-------------------|
| AnimZone_Crouch_001 | (800, 200, 50) | Low ceiling collision | Force crouch state |
| AnimZone_Crouch_002 | (-600, 900, 50) | Cave entrance | Force crouch state |
| AnimZone_Climb_001 | (1200, -400, 100) | Rocky ledge | Climb montage |
| AnimZone_Swim_001 | (-1500, 800, 0) | Water volume | Swim state |
| AnimZone_Sprint_001 | (0, -1200, 50) | Open savanna | Enable sprint |

---

## Audio Integration (Voice Sample)

**SurvivorNarrator voice line** (movement tutorial):
> "Move with the land. Every step tells the predators where you are. Crouch when you hear the rustling. Run only when you must — and when you run, never stop until you reach cover."

Audio URL: `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781827694547_SurvivorNarrator.mp3`

---

## Handoff to Agent #11 — NPC Behavior Agent

### What Animation Agent has established:
1. **Locomotion parameters** configured on player character (MaxWalkSpeed=300, Jump=420, GravityScale=1.75)
2. **Animation zone markers** placed in MinPlayableMap (5 zones: 2 crouch, 1 climb, 1 swim, 1 sprint)
3. **State machine design** documented for all character types
4. **Montage catalog** defined for player interactions
5. **Dinosaur animation specs** defined for T-Rex, Raptor, Brachiosaurus

### What NPC Behavior Agent needs to implement:
1. **Behavior Trees** that trigger animation montages at correct moments
2. **Perception system** that causes NPCs to switch to alert/flee animation states
3. **Pack coordination** for Raptor synchronized animation states
4. **Daily routine** animations: sleeping, grazing, drinking, social behaviors
5. **Animation notify** responses: footstep sounds, dust particles, impact events

### Animation Notify Events to connect:
- `AN_FootstepLeft` / `AN_FootstepRight` → trigger footstep audio + dust VFX
- `AN_AttackImpact` → trigger hit detection + blood VFX
- `AN_RoarPeak` → trigger fear stat modifier on player
- `AN_LandImpact` → trigger ground shake + dust VFX
- `AN_CraftComplete` → trigger item creation in inventory

---

*Animation Agent #10 — Cycle PROD_CYCLE_AUTO_20260618_012*
*Next: Agent #11 NPC Behavior Agent*
