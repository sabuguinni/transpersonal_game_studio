# Audio System — Cycle 001
## Agent #16 — PROD_CYCLE_AUTO_20260618_001

---

## Voice Lines Generated This Cycle

| ID | Character | File | Duration | Script Summary |
|----|-----------|------|----------|----------------|
| VL-016-001 | Elder_Mara_SpringWarning | `tts/1781747456474_Elder_Mara_SpringWarning.mp3` | ~16s | "The spring beyond the stone ridge — pack hunts there now. Three raptors, maybe four. When the ground shakes... that is not thunder. Move." |
| VL-016-002 | PlayerJournal_Entry02_RivalTribe | `tts/1781747459608_PlayerJournal_Entry02_RivalTri.mp3` | ~18s | "Found a spear near eastern ridge — not mine, not from our tribe. Someone else is out here. Left a mark on the rock face." |
| VL-016-003 | PlayerJournal_Entry03_RaptorBehavior | `tts/1781747510020_PlayerJournal_Entry03_RaptorBe.mp3` | ~17s | "Raptors coordinate — one drives, two flank, one waits. They are not animals. They are hunters. Like us." |

---

## Cumulative Voice Line Registry (All Cycles)

### Cycle 012 (Agent #16)
| Character | File | Content |
|-----------|------|---------|
| Tracker_StormWarning | `tts/1781727140975_Tracker_StormWarning.mp3` | Rain coming — predators go still. Move during storm, scent disappears. |
| Narrator_CacheDiscovery | (see cycle 012 log) | Cache discovery narration |

### Cycle 013 (Agent #16)
| Character | File | Content |
|-----------|------|---------|
| Elder_SilenceWarning | `tts/1781732619618_Elder_SilenceWarning.mp3` | "Listen. Do you hear that?" — silence before predator attack |

### Cycle 014 (Agent #16)
| Character | File | Content |
|-----------|------|---------|
| Elder_Kael_SilenceBefore | `tts/1781741546677_Elder_Kael_SilenceBefore.mp3` | "The silence before the storm" — predator warning |

### Cycle 015 (Agent #15 — Narrative)
| Character | File | Content |
|-----------|------|---------|
| Elder_NPC QuestIntro | (see cycle 015 log) | Last hunter briefing, tribe in danger |
| Elder_NPC WaterQuest | (see cycle 015 log) | Spring beyond stone ridge, pack hunting ground |
| Elder_NPC PostHunt | (see cycle 015 log) | Surprise player survived, unlocks valley lore |
| PlayerJournal_Entry01 | (see cycle 015 log) | Warm ashes, other survivor hint |

---

## Audio Zones Placed in MinPlayableMap

| Actor Label | Location | Audio Type | Linked Narrative Trigger |
|-------------|----------|------------|--------------------------|
| AudioZone_Campfire_001 | (-500, 700, 80) | Campfire crackling loop — warm orange light marker | NarrTrigger_ShelterBuilt |
| AudioZone_ElderHut_001 | (0, 200, 80) | Low wind, distant insects — dialogue ambient | NarrTrigger_ElderIntro |
| AudioZone_WaterHole_001 | (200, -1200, 80) | Trickling water, frogs, insects | NarrTrigger_WaterFound |
| AudioZone_EastRidge_001 | (800, -600, 80) | Wind, distant raptor call | NarrJournal_Entry02/03 |
| AudioZone_HuntReturn_001 | (1200, 800, 80) | Heavy breathing, fire crackle | NarrTrigger_HuntComplete |

**Visual coding:** Warm orange = fire/dialogue zones, Cool blue = water zones, Pale yellow = danger/ridge zones

---

## Sound FX References (Freesound.org)

### Storm / Thunder Ambience
| ID | Name | Duration | Preview |
|----|------|----------|---------|
| 607823 | thunder with steady rain | 464s | https://cdn.freesound.org/previews/607/607823_6911631-hq.mp3 |
| 813233 | Thunderstorm pt.2 | 232s | https://cdn.freesound.org/previews/813/813233_1433145-hq.mp3 |
| 465314 | Thunder (crack + birds) | 32s | https://cdn.freesound.org/previews/465/465314_9395330-hq.mp3 |
| 813232 | Thunderstorm pt.1 | 557s | https://cdn.freesound.org/previews/813/813232_1433145-hq.mp3 |
| 503282 | Room tone with hail | 88s | https://cdn.freesound.org/previews/503/503282_6652872-hq.mp3 |

**Usage:** Freesound ID 607823 recommended for looping storm ambience during rain gameplay events (Tracker_StormWarning voice line context). ID 465314 for one-shot thunder crack SFX.

---

## Audio Design Principles (This Game)

### What We Are Building
- **Survival audio** — every sound tells the player something about danger, resources, or navigation
- **Diegetic priority** — sounds exist in the world first, UI sounds second
- **Silence as information** — sudden silence = predator nearby (Elder_SilenceWarning voice line)
- **Layered ambience** — base layer (wind/insects) + mid layer (water/fire) + event layer (creature calls)

### Prohibited Audio Content
- No meditation tones, singing bowls, or spiritual ambience
- No mystical reverb or "sacred" sound design
- No supernatural audio cues
- All sounds must be explainable by natural prehistoric world

### Elder Mara Voice Profile
- Deep, gravelly female voice (~60 years old)
- Dry acoustic — no reverb, no echo
- Measured pace — she does not rush
- Occasional pauses for dramatic weight
- ElevenLabs voice: Rachel (default) — acceptable for prototype; target: custom voice with more age/gravel

### Player Journal Voice Profile  
- Younger male voice, 25-35 years old
- Slightly breathless — written under stress
- Factual tone — observational, not emotional
- Short sentences — field notes style

---

## MetaSounds Architecture (UE5 Design Intent)

### Adaptive Music System
```
MetaSound_AdaptiveScore
├── Layer_Base (always playing)
│   ├── Percussion_Heartbeat (low BPM, bone drum)
│   └── Wind_Ambient (procedural, varies with weather)
├── Layer_Tension (fades in when predator nearby)
│   ├── Percussion_Rapid (stone on stone, accelerating)
│   └── String_Drone (animal sinew instrument)
├── Layer_Combat (full intensity)
│   ├── Percussion_Frantic
│   └── Breath_Heavy (player exertion)
└── Layer_Safe (campfire zone)
    ├── Fire_Crackle_Loop
    └── Insects_Night_Loop
```

### Trigger Conditions
| State | Audio Layer | Trigger |
|-------|-------------|---------|
| Exploration | Base only | Default |
| Predator 200m | Base + Tension 30% | Proximity |
| Predator 50m | Base + Tension 100% | Proximity |
| Combat | Base + Combat | Attack initiated |
| Campfire zone | Base + Safe | AudioZone_Campfire overlap |
| Storm event | Base + Storm override | Weather system |

---

## Queued for Next Audio Cycle

### Voice Lines Still Needed (from Agent #15 queue)
1. **Elder_Mara — Rival Tribe Warning**: "There are others. Not like us. They do not share fire."
2. **Elder_Mara — T-Rex Migration**: "When the great one moves north, everything moves with it. Stay south."
3. **PlayerJournal_Entry04**: Day 12 — found the rival tribe's camp, abandoned in haste
4. **Tracker_NPC — Hunt Brief**: Pre-hunt tactical briefing, raptor pack location

### SFX Still Needed
- Campfire crackling loop (Freesound search returned empty — try different query)
- Raptor call (distant, 200m range trigger)
- Heavy footstep ground rumble (T-Rex proximity)
- Stone knapping (crafting SFX)
- Water drinking (survival mechanic feedback)

---

## Files Created This Cycle
- `Docs/Audio/AudioSystem_Cycle001.md` — this file
- 3 TTS voice lines (Supabase URLs above)
- 5 audio zone actors in MinPlayableMap (PointLight markers)
- 5 Freesound storm references catalogued

## Dependencies
- **From Agent #15**: DialogueSystem_Cycle001.md — voice line scripts ✅ received
- **To Agent #17 VFX**: AudioZone actor positions for VFX particle emitter placement (campfire particles at AudioZone_Campfire_001 location -500, 700, 80)
- **To Agent #19 Integration**: All TTS URLs ready for Blueprint audio component assignment
