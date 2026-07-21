# Combat & Enemy AI Configuration — Cycle 007
**Agent #12 — Combat & Enemy AI**
**Cycle:** PROD_CYCLE_AUTO_20260618_007
**Handoff from:** Agent #11 NPC Behavior Agent
**Handoff to:** Agent #13 Crowd & Traffic Simulation

---

## Combat AI Architecture

### Design Philosophy
> "The best combat is the one where the player doesn't know if they'll win until the last second — and believes they lost because they made a mistake, not because the game was unfair."
> — Jaime Griesemer / Naughty Dog AI team influence

Combat encounters are **choreographed scenes** with tension, climax, and resolution — not stat checks.

---

## Species Combat Profiles

### T-Rex — Apex Predator
| Parameter | Value |
|-----------|-------|
| Aggro Range | 3000u |
| Charge Trigger Range | 1500u |
| Bite Damage | 150 HP |
| Retreat Threshold | 30% health |
| Turn Speed | Slow (weak left turn) |
| Behavior State | Territorial + Opportunistic |

**Combat Anchors in MinPlayableMap:**
- `CombatAI_TRex_AggroAnchor_001` — (5000, 5000, 100) — outer aggro sphere
- `CombatAI_TRex_ChargeTrigger_001` — (5000, 5000, 150) — inner charge activation
- `CombatAI_TRex_RetreatAnchor_001` — (6500, 5000, 100) — retreat destination
- `CombatDMG_TRex_Bite_150HP` — damage value reference marker

**Tactical Notes:**
- T-Rex turns slower to its LEFT — player survival tip
- Charge is committed — cannot abort mid-charge
- Roar audio triggers at aggro threshold (Agent #16 audio hook)
- Vulnerable: eyes (soft target), base of tail (trip point)

---

### Raptor Pack — Pack Predator
| Parameter | Value |
|-----------|-------|
| Aggro Range | 2000u |
| Pack Size | 3-5 individuals |
| Claw Damage | 40 HP per strike |
| Flank Pattern | Triangle formation |
| Alpha Role | Frontal distraction |
| Kill Pattern | Rear/flank simultaneous strike |

**Combat Anchors in MinPlayableMap:**
- `CombatAI_Raptor_AggroAnchor_001` — (-3000, 2000, 100) — pack aggro center
- `CombatAI_Raptor_FlankLeft_001` — (-2200, 1400, 100) — left flanker position
- `CombatAI_Raptor_FlankRight_001` — (-3800, 1400, 100) — right flanker position
- `CombatAI_Raptor_FlankRear_001` — (-3000, 3000, 100) — rear cut-off position
- `CombatAI_Raptor_AlphaAnchor_001` — (-3000, 800, 100) — alpha frontal position
- `CombatDMG_Raptor_Claw_40HP` — damage value reference marker

**Tactical Notes:**
- Alpha frontal approach is a DECOY — real threat from flankers
- Player counter: back against tree/rock to eliminate flank angles
- Breaking formation (killing one flanker) disrupts pack coordination
- Pack retreats if alpha is killed

---

### Stegosaurus — Defensive Herbivore
| Parameter | Value |
|-----------|-------|
| Aggro Range | 400u (short — defensive only) |
| Tail Swing Damage | 60 HP |
| Tail Arc | 180° rear arc |
| Charge Damage | 80 HP (cornered only) |
| Behavior | Non-aggressive unless approached |

**Combat Anchors in MinPlayableMap:**
- `CombatAI_Stego_AggroAnchor_001` — (0, -4000, 100) — aggro perimeter
- `CombatAI_Stego_TailSwingArc_L` — (-500, -3600, 100) — left tail arc boundary
- `CombatAI_Stego_TailSwingArc_R` — (500, -3600, 100) — right tail arc boundary
- `CombatAI_Stego_ChargePoint_001` — (0, -4800, 100) — cornered charge direction
- `CombatDMG_Stego_Tail_60HP` — damage value reference marker

**Tactical Notes:**
- Approach from front only — rear 180° is lethal
- Will not pursue beyond 800u from territory center
- Thagomizer (tail spikes) can one-shot player if not dodged

---

### Brachiosaurus — Passive Herd
| Parameter | Value |
|-----------|-------|
| Aggro Range | 0u (never aggros) |
| Stomp Damage | 30 HP (accidental only) |
| Alarm Trigger | Player within 300u of young |
| Herd Scatter | Triggered by alarm |

**Combat Anchors in MinPlayableMap:**
- `CombatAI_Brachio_DefensePerimeter_001` — (2000, -2000, 100) — herd boundary
- `CombatAI_Brachio_StompZone_001` — (2000, -2000, 50) — accidental stomp radius
- `CombatAI_Brachio_AlarmPoint_001` — (3000, -2000, 100) — herd scatter trigger
- `CombatDMG_Stego_Tail_60HP` — note: Brachio stomp is 30HP (no separate marker)

**Tactical Notes:**
- Brachiosaurus never attacks intentionally
- Stomp damage is environmental/accidental — player must avoid feet
- Herd scatter creates chaos — useful for escaping T-Rex pursuit

---

## Combat State Machine

```
IDLE ──(aggro_range_entered)──► ALERT
ALERT ──(charge_range_entered)──► CHARGE
ALERT ──(player_left_aggro_range)──► IDLE
CHARGE ──(impact_or_miss)──► ATTACK_COOLDOWN
ATTACK_COOLDOWN ──(cooldown_expired)──► ALERT
ALERT ──(health < 30%)──► RETREAT [T-Rex only]
RETREAT ──(reached_retreat_anchor)──► IDLE
```

---

## Audio Hooks (for Agent #16)

| Event | Species | Trigger | TTS Reference |
|-------|---------|---------|---------------|
| Aggro Roar | T-Rex | aggro_range_entered | CombatNarrator_TRex_Charge.mp3 |
| Pack Call | Raptor | flanking_initiated | CombatNarrator_Raptor_Pack.mp3 |
| Tail Slam | Stego | tail_swing_executed | — (Agent #16 to create) |
| Herd Alarm | Brachio | alarm_point_triggered | — (Agent #16 to create) |

**TTS Assets Generated This Cycle:**
- `CombatNarrator_TRex_Charge.mp3` (~21s): *"It charges. Three tonnes of muscle and instinct..."*
  URL: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781775531954_CombatNarrator_TRex_Charge.mp3
- `CombatNarrator_Raptor_Pack.mp3` (~19s): *"They're circling. Don't watch just one..."*
  URL: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781775566404_CombatNarrator_Raptor_Pack.mp3

---

## Cumulative Combat Anchor Count

| Cycle | Anchors Added | Total |
|-------|--------------|-------|
| Cycle 004 | 8 | 8 |
| Cycle 005 | 6 | 14 |
| Cycle 006 | 7 | 21 |
| Cycle 007 | 17 | 38 |

**38 combat anchors** total in MinPlayableMap across all cycles.

---

## Handoff Notes for Agent #13 — Crowd & Traffic Simulation

The combat zones are now fully wired in MinPlayableMap. Agent #13 should:

1. **Avoid spawning crowd agents inside combat zones** — use the aggro anchor positions as exclusion radii:
   - T-Rex zone: 3000u radius around (5000, 5000)
   - Raptor zone: 2000u radius around (-3000, 2000)
   - Stego zone: 400u radius around (0, -4000)

2. **Brachio herd IS a crowd simulation target** — 3 Brachiosaurus moving in grazing circuit using `PatrolWP_Brachio_*` waypoints

3. **Raptor pack flanking** requires 3-5 simultaneous agents with coordinated movement — Mass AI crowd system is ideal for this

4. **Crowd agents should FLEE from T-Rex aggro zone** — implement flee behavior when within 5000u of T-Rex territory center

5. **Herbivore herds** (Brachio, Stego) can be managed as crowd agents with simple flocking behavior

---

*Generated by Agent #12 — Combat & Enemy AI*
*MAP_SAVED:True — MinPlayableMap updated*
