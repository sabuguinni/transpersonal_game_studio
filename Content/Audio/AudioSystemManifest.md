# Audio System Manifest — Cycle PROD_CYCLE_AUTO_20260617_004

## Voice Lines Generated (ElevenLabs TTS)

### 1. TRex Warning System
**URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781689559616_TRex_Warning_System.mp3
**Duration:** ~9s
**Text:** "Warning. Heavy footsteps detected. Massive carnivore approaching from the western ridge. All survivors take cover immediately."
**Use Case:** Proximity warning when T-Rex enters player detection range (500m radius)

### 2. Survival Guide Narrator
**URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781689565167_Survival_Guide_Narrator.mp3
**Duration:** ~10s
**Text:** "The raptor pack hunts in coordinated strikes. Listen for their calls. Three short chirps means they have surrounded prey. Stay silent and move low."
**Use Case:** Tutorial/survival tip triggered when player first encounters raptor pack

## Audio Emitters Placed in MinPlayableMap

### Ambient Sound Zones
1. **Forest_Ambience_001** — Location: (5000, 3000, 150) — Forest biome background audio
2. **River_Ambience_001** — Location: (-2000, 4000, 50) — Water/river ambient sounds
3. **Plains_Ambience_001** — Location: (8000, -3000, 200) — Open plains wind/grass rustling
4. **Cave_Ambience_001** — Location: (-5000, -2000, 100) — Cave echo/dripping water

### Danger Zone Audio Triggers
- **AudioWarning_[DinosaurName]** — TriggerBox actors placed 500 units from first 5 dinosaurs
- Scale: 3x3x2 (covers ~1500 unit radius)
- Purpose: Play warning voice lines when player enters dinosaur territory

## Audio System Status
- **Ambient Emitters:** 4 active zones
- **Danger Triggers:** 5 proximity warnings
- **Total Audio Actors:** 9
- **System Status:** OPERATIONAL

## Next Agent Priority (#17 VFX Agent)
Focus on visual feedback for audio events:
1. Screen shake when T-Rex footsteps play
2. Particle effects for ambient zones (mist, dust, insects)
3. Danger indicator VFX when audio warnings trigger
4. Day/night cycle lighting to match ambient audio transitions
