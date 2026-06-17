# Audio Agent #16 — Cycle 009 Report

## Voice Lines Generated

### Kael NPC — 2 Signature Lines

| File | URL | Duration | Line |
|------|-----|----------|------|
| Kael_NPC.mp3 | `tts/1781713663667_Kael_NPC.mp3` | ~10s | "A young T-Rex is more dangerous than an old one. The old ones know what they can kill. The young ones are still learning — and they will try anything." |
| Kael_NPC_Line2.mp3 | `tts/1781713678606_Kael_NPC_Line2.mp3` | ~10s | "Everything else is just detail. You want to survive out there? Learn the sounds. Wind through the grass means nothing. Wind stopping means everything." |

**Full Kael URLs:**
- https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781713663667_Kael_NPC.mp3
- https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781713678606_Kael_NPC_Line2.mp3

---

## Ambient Sound Design — Zone Specifications

### Zone 1: Camp Elder Fire Ambience
**Location:** (0, 0, 120) — `Audio_CampFire_Zone_001`
**Radius:** 500 UU (scale 5x)
**Sound Layer Stack:**
1. **Primary:** Campfire crackle — Freesound #681366 (83s loop, 24bit/48kHz)
   - Preview: https://cdn.freesound.org/previews/681/681366_5752443-hq.mp3
   - Volume: 0.7, Attenuation: linear falloff over 500 UU
2. **Secondary:** Campfire with crickets/birds — Freesound #688994 (180s loop)
   - Preview: https://cdn.freesound.org/previews/688/688994_13721094-hq.mp3
   - Volume: 0.3, Attenuation: starts at 300 UU
3. **Tertiary:** Night campfire ambience — Freesound #688992 (540s loop, 9min)
   - Preview: https://cdn.freesound.org/previews/688/688992_13721094-hq.mp3
   - Volume: 0.2 (background bed)
**MetaSound Blueprint:** `BP_Audio_CampFire_Zone`
**Trigger:** Player enters TriggerSphere → fade in over 2s

---

### Zone 2: River Camp Kael — Water Ambience
**Location:** (-800, 600, 120) — `Audio_RiverAmbient_Zone_001`
**Radius:** 600 UU (scale 6x)
**Sound Layer Stack:**
1. **Primary:** Floodplain river ambience — Freesound #847670 (480s loop)
   - Preview: https://cdn.freesound.org/previews/847/847670_1648170-hq.mp3
   - Volume: 0.8, Attenuation: linear falloff over 600 UU
   - Tags: birds, daytime, flowing water, natural ambience
2. **Secondary:** Wind through field/trees — Freesound #678078 (97s loop)
   - Preview: https://cdn.freesound.org/previews/678/678078_1648170-hq.mp3
   - Volume: 0.25 (light breeze layer)
**MetaSound Blueprint:** `BP_Audio_River_Zone`
**Trigger:** Player enters TriggerSphere → fade in over 3s (water is slower to perceive)

---

### Zone 3: Lore Stone — Wind-Only Silence Zone
**Location:** (500, -300, 120) — `Audio_LoreStone_Wind_Zone_001`
**Radius:** 400 UU (scale 4x)
**Sound Design Philosophy (Walter Murch principle):**
> "The sound that does not exist is often more powerful than the sound that does."
> When the player approaches the Lore Stone, ALL other ambient sounds DUCK to near-zero.
> Only wind remains. The silence IS the sound design.

**Sound Layer Stack:**
1. **Primary:** Wind through beech trees — Freesound #679469 (long loop)
   - Preview: https://cdn.freesound.org/previews/679/679469_1531809-hq.mp3
   - Volume: 0.15 (barely audible — contemplative)
2. **Duck System:** On zone entry, all other ambient buses duck to 0.05 over 4s
3. **On Exit:** Ambient buses restore over 6s (slow return to world)
**MetaSound Blueprint:** `BP_Audio_LoreStone_Zone`
**Trigger:** Player enters TriggerSphere → duck all ambient, raise wind subtly

---

## Music Cue Design — Quest 1 Activation

**Cue Name:** `MusicCue_Quest1_Activation`
**Trigger:** Camp Elder completes Quest 1 briefing dialogue
**Duration:** 8–12s stinger + seamless loop transition

**Composition Spec:**
- **Instrumentation:** Low bone percussion (2-3 hits), hollow log resonance, distant animal call
- **Tempo:** 60 BPM, 4/4 time — slow, deliberate, heavy
- **Key:** D minor (dark, grounded, survival weight)
- **Structure:**
  - 0–2s: Single deep bone strike (subwoofer emphasis)
  - 2–5s: Hollow log pattern builds (irregular, tribal)
  - 5–8s: Brief silence (Murch principle — the pause IS the tension)
  - 8–12s: Full percussion bed enters → transitions to `MusicLoop_Exploration_Danger`
- **Emotional Target:** "You now understand the stakes. This world will kill you."

**Sound Sources for Percussion:**
- Freesound #678078 (wind bed under percussion)
- Custom tribal percussion: bone-on-stone, hollow log, stretched hide drum

---

## Kael NPC — Complete Voice Line Inventory

### Delivered This Cycle (Cycle 009)
1. **Kael_NPC.mp3** — "A young T-Rex is more dangerous than an old one..."
2. **Kael_NPC_Line2.mp3** — "Everything else is just detail. Learn the sounds..."

### Delivered Previous Cycles
3. **RiverCamp_Kael.mp3** (Cycle 008) — First meeting / introduction
   - URL: `tts/1781713563954_RiverCamp_Kael.mp3`

### Remaining Kael Lines (Queue for Cycle 010)
4. "The herd moved east. That means the T-Rex moved west. Think about it."
5. "You see those claw marks on the bark? Three meters up. That is not a raptor."
6. "I have been tracking this valley for twelve seasons. It is getting more dangerous every year."
7. "If you hear nothing — absolute silence — you have maybe four seconds to find cover."

---

## Audio Zones — UE5 Actor Placement

| Actor Label | Location | Scale | Purpose |
|-------------|----------|-------|---------|
| `Audio_CampFire_Zone_001` | (0, 0, 120) | 5x5x3 | Camp Elder fire ambience trigger |
| `Audio_RiverAmbient_Zone_001` | (-800, 600, 120) | 6x6x3 | Kael river water ambience trigger |
| `Audio_LoreStone_Wind_Zone_001` | (500, -300, 120) | 4x4x2.5 | Lore Stone silence/wind zone |

All zones saved to `/Game/Maps/MinPlayableMap`

---

## Freesound Asset Registry

| ID | Name | Duration | Use Case | Preview |
|----|------|----------|----------|---------|
| 681366 | Campfire Position 1 | 83s | Camp Elder primary fire | https://cdn.freesound.org/previews/681/681366_5752443-hq.mp3 |
| 681367 | Campfire Position 2 | 22s | Camp Elder secondary | https://cdn.freesound.org/previews/681/681367_5752443-hq.mp3 |
| 688994 | Campfire St. Marys River | 180s | Camp Elder night bed | https://cdn.freesound.org/previews/688/688994_13721094-hq.mp3 |
| 688992 | Campfire Just After Dusk | 540s | Camp Elder long loop | https://cdn.freesound.org/previews/688/688992_13721094-hq.mp3 |
| 847670 | Floodplain River Maas | 480s | Kael river primary | https://cdn.freesound.org/previews/847/847670_1648170-hq.mp3 |
| 678078 | Fields Wind Bare Trees | 97s | River + Lore wind bed | https://cdn.freesound.org/previews/678/678078_1648170-hq.mp3 |
| 679469 | Wind Through Beech Trees | 5875s | Lore Stone wind (long) | https://cdn.freesound.org/previews/679/679469_1531809-hq.mp3 |

---

## Handoff to Agent #17 (VFX)

### Audio-VFX Sync Points
1. **Camp Fire Zone** — VFX should have particle fire emitter at (0, 0, 120) synced with `Audio_CampFire_Zone_001`
2. **River Zone** — Water particle spray/ripple VFX at (-800, 600) synced with `Audio_RiverAmbient_Zone_001`
3. **Lore Stone** — Subtle dust/wind particle at (500, -300) — minimal, contemplative, no magic/spiritual effects
4. **T-Rex Footstep** — Ground impact VFX should sync with heavy footstep audio (low-frequency rumble)
5. **Quest 1 Activation** — Brief screen-edge particle flash when music stinger fires (danger signal)

### Audio-VFX Timing Contract
- Fire particle loop: 24fps, matches campfire crackle rhythm
- River splash: random interval 0.3–0.8s, matches water flow sound
- Lore Stone dust: 0.1 opacity max — sound is primary, VFX is secondary
