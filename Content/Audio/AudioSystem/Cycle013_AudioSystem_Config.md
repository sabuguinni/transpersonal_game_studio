# Audio Agent #16 — Cycle AUTO_20260702_013
## Audio System Configuration

### TTS Voice Lines Generated

#### Tracker_Dak_SurvivalLesson (~13s)
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783038498478_Tracker_Dak_SurvivalLesson.mp3
- **Text:** "The forest is never silent. Listen — the insects stop before a predator moves. The birds scatter before the ground shakes. Learn these sounds, and you will live. Ignore them, and you will not."
- **Trigger:** `QST_SurvivalLesson_Begin` → player first enters forest biome
- **Spatial:** 3D attenuation, 800 unit radius, falloff linear

#### ChiefHunter_Kael_TRex_Tactics (~16s)
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783038506365_ChiefHunter_Kael_TRex_Tactics.mp3
- **Text:** "Stay low. Stay quiet. If the T-Rex cannot see you, it cannot hunt you. Move only when the wind is in your face — never at your back. And if it charges — do not run. Find cover. A rock, a tree, a ravine. Running is how you die tired."
- **Trigger:** `QST_TRex_Territory_Avoid` → player enters eastern ridge zone
- **Spatial:** 3D attenuation, 1000 unit radius, falloff inverse square

---

### Audio Source Actors Spawned in MinPlayableMap

| Label | Position | Type | Color | Purpose |
|-------|----------|------|-------|---------|
| AudioSrc_Campfire_Hub_001 | (2100, 2400, 100) | PointLight proxy | Warm orange | Campfire crackle loop |
| AudioSrc_NPC_Elder_Vorn_001 | (2050, 2350, 100) | PointLight proxy | Blue-grey | Elder dialogue spatial origin |
| AudioSrc_NPC_Scout_Mira_001 | (2150, 2350, 100) | PointLight proxy | Green | Scout dialogue spatial origin |
| AudioSrc_NPC_Tracker_Dak_001 | (2050, 2450, 100) | PointLight proxy | Brown | Tracker dialogue spatial origin |
| AudioSrc_NPC_Crafter_Sela_001 | (2150, 2450, 100) | PointLight proxy | Amber | Crafter dialogue spatial origin |

### Audio Trigger Zones Spawned

| Label | Position | Size | Purpose |
|-------|----------|------|---------|
| AudioZone_RaptorCall_River_001 | (1800, 2800, 50) | 600×600×300 | Raptor calls at dusk near river |
| AudioZone_TRex_EasternRidge_001 | (2800, 2400, 50) | 800×800×400 | T-Rex distant rumble on eastern ridge |
| AudioZone_Canyon_LowRumble_001 | (2400, 1800, 50) | 500×500×300 | Canyon echo low frequency rumble |
| AudioZone_DayAmbient_Hub_001 | (2100, 2400, 50) | 2000×2000×500 | Day insects + birds ambient loop |
| AudioZone_NightAmbient_Hub_001 | (2100, 2400, 50) | 2000×2000×500 | Night crickets + distant howls |
| AudioZone_Crafting_Sela_001 | (2150, 2450, 50) | 200×200×150 | Stone knapping SFX near Sela |

---

### MetaSounds Integration Notes (for Blueprint implementation)

#### Campfire Audio (AudioSrc_Campfire_Hub_001)
- **Asset:** `MS_Campfire_Loop` — looping MetaSound source
- **Parameters:** `CrackleIntensity` (0.0–1.0), `WindInfluence` (0.0–1.0)
- **Attenuation:** 3D, 600 unit inner radius, 1200 unit outer radius
- **Priority:** Always-on when player is within 1200 units of hub

#### Predator Audio Zones
- **RaptorCall:** Triggered at `TimeOfDay >= 18:00 AND TimeOfDay <= 20:00`
  - Sound: distant raptor screech, random interval 15–45s
  - Spatial: 2D (non-positional, atmospheric)
- **TRex Rumble:** Triggered when player enters `AudioZone_TRex_EasternRidge_001`
  - Sound: low-frequency ground rumble + distant roar
  - Screen shake: 0.3 magnitude, 0.8s duration
  - Spatial: 3D, 1500 unit radius

#### Dialogue Spatial Audio
- All NPC voice lines use 3D attenuation
- Inner radius: 200 units (full volume)
- Outer radius: 800 units (fade to silence)
- Falloff: Linear
- Subtitle system: enabled, text matches TTS lines exactly

---

### Audio Design Notes

#### Silence as Information (Walter Murch Principle)
The `AudioZone_DayAmbient_Hub_001` implements the "silence before predator" mechanic:
- Normal state: insects + birds playing continuously
- When T-Rex enters 500 unit radius of player: insects STOP (0.3s fade)
- When raptors are within 300 units: birds STOP (0.1s immediate cut)
- Player learns to read silence as danger signal — no UI required

#### Adaptive Music Layers
- **Safe (hub, daytime):** Soft percussion, bone flute, low drone
- **Tension (predator nearby):** Percussion intensifies, strings enter
- **Danger (T-Rex visible):** Full percussion, dissonant tones, heartbeat
- **Combat:** Aggressive percussion, no melody, pure rhythm
- Transitions: 2-4 bar crossfades, never abrupt cuts

---

### Handoff to Agent #17 (VFX Agent)

1. **Campfire VFX:** Niagara fire system at `AudioSrc_Campfire_Hub_001` position (2100, 2400, 100)
   - Fire + ember particles, smoke column rising
   - Sync with `CrackleIntensity` audio parameter for visual-audio coherence

2. **T-Rex footstep dust:** Niagara dust puff at each T-Rex footstep
   - Trigger: `AudioZone_TRex_EasternRidge_001` overlap → spawn dust at ground level
   - Scale with screen shake magnitude

3. **Raptor zone visual:** Subtle fog/mist increase in `AudioZone_RaptorCall_River_001`
   - Atmospheric density +20% when raptor audio triggers

4. **NPC dialogue visual:** Subtle speech indicator above NPC head when dialogue plays
   - Simple Niagara particle burst (3–5 particles) at NPC position
