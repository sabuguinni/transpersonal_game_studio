# Combat AI System Specification

## Overview
The Combat AI system provides tactical enemy behavior for dinosaurs in the Transpersonal Game prehistoric survival world. It implements realistic predator behaviors including ambush tactics, pack coordination, and territorial defense.

## Core Components

### 1. Combat Zones (TriggerBox Actors)
Spatial regions that define combat encounter areas and AI behavior contexts.

**Properties**:
- `zone_type`: ambush | pack_hunt | defense | patrol
- `aggro_radius`: Detection range in UE5 units (1500-3000)
- `threat_level`: low | medium | high | extreme
- `linked_markers`: Array of TargetPoint references for AI navigation

**Current Zones**:
- TRex_Ambush_001 (3000, 5000, 200) — Radius 2000
- Raptor_Hunt_001 (-2000, 3000, 150) — Radius 1500
- Tricera_Territory_001 (1000, -4000, 180) — Radius 1800

### 2. AI State Markers (TargetPoint Actors)
Waypoints that define tactical positions for AI navigation and behavior.

**Marker Types**:
- **Patrol**: Idle/search behavior waypoints
- **Attack**: Optimal attack positions relative to target
- **Flank**: Coordinated multi-angle approach points
- **Defend**: Territorial defense positions
- **Retreat**: Fallback positions when health < 30%

**Current Markers**:
- TRex_Patrol_001, TRex_Attack_001
- Raptor_Flank_001, Raptor_Flank_002
- Tricera_Defend_001

### 3. Dinosaur Combat AI Profiles

#### T-Rex — Ambush Predator
```
aggro_range: 3000 units
attack_damage: 50 HP
speed_multiplier: 1.2x
behavior: ambush_predator

States:
- IDLE: Patrols between patrol markers
- ALERT: Player detected within aggro range, moves to attack position
- CHARGE: Sprints toward player at 1.2x speed
- ATTACK: Melee attack when within 300 units
- RETREAT: Falls back if health < 20%
```

#### Velociraptor — Pack Hunter
```
aggro_range: 2000 units
attack_damage: 25 HP
speed_multiplier: 1.8x
behavior: pack_hunter

States:
- IDLE: Patrols in loose formation
- ALERT: Pack coordinates on detection
- FLANK: Individuals move to flank markers
- ATTACK: Synchronized strikes from multiple angles
- RETREAT: Pack regroups if 50% casualties
```

#### Triceratops — Territorial Defense
```
aggro_range: 1500 units
attack_damage: 40 HP
speed_multiplier: 0.8x
behavior: territorial_defense

States:
- IDLE: Stands at defend marker
- ALERT: Faces threat, lowers horns
- CHARGE: Short-range charge if player enters territory
- ATTACK: Gore attack within 400 units
- RETREAT: Does not retreat, fights to death in territory
```

## Combat Flow

### Detection Phase
1. Dinosaur AI checks for player within `aggro_range` every 0.5 seconds
2. Line-of-sight check using UE5 raycast
3. If detected, transition to ALERT state
4. Broadcast alert to nearby dinosaurs of same species (pack coordination)

### Engagement Phase
1. AI selects optimal marker based on behavior type:
   - Ambush: Move to attack marker
   - Pack: Coordinate flank positions
   - Defense: Hold defend marker
2. Navigate to marker using UE5 NavMesh
3. When within attack range, execute attack animation
4. Apply damage to player on animation hit frame

### Combat Calculations
```cpp
// Damage formula
float FinalDamage = BaseDamage * DifficultyMultiplier * (1.0f - PlayerArmor);

// Attack cooldown
float AttackCooldown = BaseAttackSpeed / SpeedMultiplier;

// Aggro priority (if multiple players)
float AggroPriority = (PlayerHealth / PlayerMaxHealth) * DistanceWeight;
```

### Retreat Phase
1. Check health threshold:
   - T-Rex: < 20% HP
   - Raptor: < 30% HP or 50% pack casualties
   - Triceratops: Never retreats
2. Navigate to nearest retreat marker
3. Idle at retreat marker for 30 seconds (heal 10% HP)
4. Re-engage if player pursues

## Audio Integration
Combat AI triggers audio cues on state transitions:

- **ALERT**: Warning growl (CombatWarning_TRex.mp3, CombatWarning_Raptors.mp3)
- **CHARGE**: Aggressive roar
- **ATTACK**: Impact sound + damage grunt
- **RETREAT**: Wounded cry

## Visual Feedback
- Red outline on ALERT state
- Damage numbers on successful hit
- Threat indicator UI when within aggro range
- Blood particle effects on attack

## Performance Optimization
- Max 20 active combat AI simultaneously
- AI updates at 10 Hz (0.1s tick rate)
- Pathfinding uses cached NavMesh queries
- Aggro checks use spatial hashing (not brute force)

## Future Enhancements
1. **Behavior Tree Integration**: Replace state machine with UE5 Behavior Trees
2. **Dynamic Difficulty**: Adjust aggro/damage based on player skill
3. **Environmental Tactics**: Use terrain for ambushes (tall grass, cliffs)
4. **Injury System**: Limping, reduced speed when wounded
5. **Fear Mechanic**: Dinosaurs flee from fire/loud noises

## Testing Checklist
- [ ] T-Rex detects player at 3000 units
- [ ] Raptors coordinate flank attacks
- [ ] Triceratops defends territory without chasing
- [ ] Damage applies correctly to player health
- [ ] Audio cues trigger on state changes
- [ ] AI navigates to markers without getting stuck
- [ ] Retreat behavior activates at health thresholds
- [ ] Pack coordination works with 3+ raptors

---

**Status**: OPERATIONAL  
**Version**: 1.0  
**Last Updated**: PROD_CYCLE_AUTO_20260617_002
