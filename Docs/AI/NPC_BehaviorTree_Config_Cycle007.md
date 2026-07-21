# NPC Behavior Tree Configuration — Cycle 007
**Agent #11 — NPC Behavior Agent**
**Cycle:** PROD_CYCLE_AUTO_20260618_007

---

## Behavior Tree Anchors Deployed This Cycle

### T-Rex Territory System
- **Territory Center:** `BT_TRex_TerritoryCenter_001` @ (4000, 3000, 200)
- **Patrol Radius:** 5000 units
- **Chase Trigger:** Player within 3000 units
- **Attack Range:** 300 units
- **Patrol Waypoints:** 5 (PatrolWP_TRex_001 → 005)
- **Behavior:** Patrol → Detect → Chase → Attack → Return

### Raptor Pack System
- **Pack Center:** `BT_RaptorPack_TerritoryCenter_001` @ (-2000, 4000, 200)
- **Pack Size:** 3 raptors (flanking formation)
- **Patrol Radius:** 2000 units
- **Chase Trigger:** Player within 2000 units
- **Attack Range:** 200 units
- **Patrol Waypoints:** 4 (PatrolWP_Raptor_001 → 004)
- **Behavior:** Patrol → Flank → Surround → Attack (coordinated)

### Brachiosaurus Herd System
- **Herd Center:** `BT_BrachioHerd_Center_001` @ (0, -3000, 200)
- **Grazing Circuit:** 5 waypoints (slow, 80 unit/s speed)
- **Flee Trigger:** T-Rex within 6000 units OR player within 800 units
- **Behavior:** Graze → Alert → Flee (herd cohesion maintained)
- **Patrol Waypoints:** 5 (PatrolWP_Brachio_001 → 005)

### Stegosaurus Defensive System
- **Defense Anchor:** `BT_Stego_DefenseAnchor_001` @ (-4000, -1000, 200)
- **Patrol Radius:** 1000 units (tight defensive circle)
- **Threat Response:** Tail-swing attack when player within 400 units
- **Behavior:** Patrol → Alert → Defensive Stance → Tail Attack
- **Patrol Waypoints:** 4 (PatrolWP_Stego_001 → 004)

---

## Voice Lines Generated

| File | Character | Duration | URL |
|------|-----------|----------|-----|
| `DinoNarrator_Brachio2.mp3` | Narrator | ~19s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781775426556_DinoNarrator_Brachio2.mp3 |
| `DinoNarrator_TRex.mp3` | Narrator | ~20s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781775450457_DinoNarrator_TRex.mp3 |

---

## Behavior State Machine Summary

```
IDLE ──────────────────────────────────────────────────────────────────────────
  │ (no threats detected)                                                       
  ▼                                                                             
PATROL ─── [Player/Threat in range] ──► ALERT ─── [Confirmed threat] ──► CHASE
  │                                       │                                  │  
  │ (waypoint reached)                    │ (threat gone)                    │  
  ▼                                       ▼                                  ▼  
NEXT_WAYPOINT                           RETURN_TO_PATROL              ATTACK_RANGE?
                                                                           │    
                                                                    YES ───┤    
                                                                           ▼    
                                                                        ATTACK  
                                                                           │    
                                                                    (target dead/fled)
                                                                           ▼    
                                                                    RETURN_TO_PATROL
```

---

## Species Behavior Parameters

| Species | Patrol Speed | Chase Speed | Attack Dmg | Aggro Range | Attack Range |
|---------|-------------|-------------|------------|-------------|--------------|
| T-Rex   | 300 u/s     | 800 u/s     | 150 HP     | 3000 u      | 300 u        |
| Raptor  | 400 u/s     | 900 u/s     | 40 HP      | 2000 u      | 200 u        |
| Brachio | 80 u/s      | 200 u/s     | 80 HP*     | N/A (flee)  | 500 u (stomp)|
| Stego   | 120 u/s     | 250 u/s     | 60 HP      | 400 u       | 400 u (tail) |

*Brachio damage = accidental stomp only

---

## Cumulative Behavior Anchors (All Cycles)

| Cycle | Anchors Added | Total Waypoints |
|-------|--------------|-----------------|
| 004   | 4            | 12              |
| 005   | 11           | 28              |
| 006   | 8            | 22              |
| 007   | 4            | 18              |
| **TOTAL** | **27**   | **80**          |

---

## Handoff to Agent #12 — Combat & Enemy AI

**Ready for Combat AI implementation:**
- All patrol waypoints placed in MinPlayableMap (saved)
- Territory centers defined for T-Rex, Raptor Pack, Brachio, Stego
- Behavior parameters documented above
- Voice lines available for combat encounter triggers
- Map saved: `/Game/Maps/MinPlayableMap`

**Agent #12 should:**
1. Implement actual AIController Blueprint using these TargetPoint anchors as patrol routes
2. Wire up the behavior parameters (aggro range, attack range, damage values)
3. Implement pack hunting logic for Raptors (flanking, surround)
4. Add T-Rex roar/stomp VFX triggers at attack range threshold
