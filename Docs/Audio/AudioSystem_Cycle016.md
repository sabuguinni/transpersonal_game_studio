# Audio System — Agent #16
## Cycle: PROD_CYCLE_AUTO_20260617_014

---

## Voice Lines Produced This Cycle

| ID | Character | URL | Duration | Content |
|----|-----------|-----|----------|---------|
| VL-016-01 | Elder_Kael_SilenceBefore | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781741546677_Elder_Kael_SilenceBefore.mp3 | ~14s | "The silence before the storm is the most dangerous moment..." |
| VL-016-02 | Narrator_SurvivalWater | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781741562740_Narrator_SurvivalWater.mp3 | ~17s | "Water. Always find water first. A river means life..." |

### Previous Cycles (cumulative library)
| Cycle | Character | URL |
|-------|-----------|-----|
| 013 | Elder_SilenceWarning | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781732619618_Elder_SilenceWarning.mp3 |
| 012 | Tracker_StormWarning | `tts/1781727140975_Tracker_StormWarning.mp3` |
| 012 | Narrator_CacheDiscovery | `tts/1781727157023_Narrator_CacheDiscovery.mp3` |
| 011 | Scout_TRexWarning | `tts/1781722645708_Scout_TRexWarning.mp3` |
| 011 | Narrator_SurvivalTips | `tts/1781722657577_Narrator_SurvivalTips.mp3` |
| 015 (from #15) | Elder_Kael | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781741398569_Elder_Kael.mp3 |
| 015 (from #15) | Scout_Dara | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781741415855_Scout_Dara.mp3 |
| 015 (from #15) | PlayerJournal_Narrator | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781741430780_PlayerJournal_Narrator.mp3 |
| 015 (from #15) | Elder_Kael_RaptorFeathers | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781741433526_Elder_Kael_RaptorFeathers.mp3 |

---

## Sound Library — Freesound References

### River / Water Ambience
| ID | Name | Duration | Preview | Use |
|----|------|----------|---------|-----|
| 819768 | Small Brook Flowing Around Rocks | 87s | https://cdn.freesound.org/previews/819/819768_4036966-hq.mp3 | AmbientZone_River_001 |
| 324591 | Burbling Brook | 185s | https://cdn.freesound.org/previews/324/324591_387219-hq.mp3 | River crossings, camp near water |
| 328140 | Small Creek & Birds | 94s | https://cdn.freesound.org/previews/328/328140_3815733-hq.mp3 | Forest edge + water combo |
| 808370 | Flowing River | 59s | https://cdn.freesound.org/previews/808/808370_16786392-hq.mp3 | Fast river — danger crossing |

### Fear / Combat Audio
| ID | Name | Duration | Preview | Use |
|----|------|----------|---------|-----|
| 856171 | Racing Panic Heartbeat Adrenaline Pulse | 6s | https://cdn.freesound.org/previews/856/856171_18901108-hq.mp3 | HeartbeatZone_TRex_001 — fear stat spike |

---

## Ambient Audio Zone System

### Zone Architecture
5 audio zones placed in MinPlayableMap as coloured PointLight markers.
Each zone represents a distinct acoustic environment with unique audio character.

| Zone Label | Map Position | Colour | Acoustic Character | Freesound Ref |
|------------|-------------|--------|-------------------|---------------|
| AmbientZone_River_001 | (0, 800, 50) | Blue | Flowing water, distant birds | FS#819768, FS#808370 |
| AmbientZone_Forest_001 | (-600, 400, 50) | Green | Insects, birds, wind through leaves | FS#328140 |
| AmbientZone_Savanna_001 | (800, 200, 50) | Yellow | Open wind, distant herd rumble | — |
| AmbientZone_Cave_001 | (-800, -600, 50) | Purple | Drips, echo, near-silence | — |
| HeartbeatZone_TRex_001 | (600, 600, 80) | Red | Fear pulse — heartbeat trigger | FS#856171 |

### Integration Notes for UE5 Implementation
When MetaSounds are available, each zone should:
1. Use **Attenuation Radius: 600 units** (matching current light radius)
2. Blend between zones using **distance-weighted crossfade** (0-100% over 200 units)
3. **HeartbeatZone_TRex_001** triggers when Fear stat > 60 AND player within 600 units of TRex
4. River zone uses **looping** ambient with subtle pitch variation (±2%) for naturalness
5. Cave zone uses **reverb preset: Large Cave** with 2.8s decay

---

## Dialogue Audio Integration (from Agent #15)

### Voice Line → Trigger Mapping
| Voice Line | Trigger Actor (from #15) | Playback Condition |
|------------|--------------------------|-------------------|
| Elder_Kael (Quest 1 Migration) | DialogueTrigger_Elder_Migration @ (-200,-300) | Player enters 300-unit radius |
| Scout_Dara (Quest 2 Pack Territory) | DialogueTrigger_Scout_PackTerritory @ (-350,-200) | Player enters 300-unit radius |
| PlayerJournal_Narrator (Quest 3) | DialogueTrigger_Journal_PanicValley @ (0,1500) | Player picks up journal item |
| Elder_Kael_RaptorFeathers (Quest 2 reward) | DialogueTrigger_Elder_RaptorLore @ (-200,-300,180) | Quest 2 completion flag |
| Elder_Kael_SilenceBefore (NEW) | HeartbeatZone_TRex_001 @ (600,600,80) | Fear stat > 40, first TRex encounter |
| Narrator_SurvivalWater (NEW) | AmbientZone_River_001 @ (0,800,50) | First time player reaches river |

### Herd Trust Audio Cues (from Agent #15 request)
| Trust Level | Audio Response | Description |
|-------------|----------------|-------------|
| Trust 0-2 | Herd alarm calls (high pitch) | Herd is frightened, moving away |
| Trust 3 | Low rumble vocalisation | Herd acknowledges player, cautious |
| Trust 6 | Calm herd breathing + soft calls | Herd accepts player presence |
| Trust 9 | Gentle contact calls | Herd treats player as part of group |
| Trust loss event | Sharp alarm burst (2s) | Immediate herd scatter sound |

---

## Audio Design Principles (this game)

### What this game sounds like
- **Baseline**: Natural world in constant motion — insects, wind, water, distant animals
- **Tension**: Sounds DROP OUT before danger (the silence rule — Elder_Kael_SilenceBefore)
- **Fear**: Heartbeat rises in mix, external sounds compress/duck
- **Safety**: Full ambient mix returns, herd sounds present, birds active
- **Combat**: Percussive impacts, raw animal sounds, no musical score — pure diegetic

### The Silence Rule
The most important audio mechanic in this game:
> "When the insects stop singing and the birds go quiet — something large is close."

Implementation: When a predator (TRex, Raptor pack) enters 800-unit radius of player:
1. Insect/bird ambient layer fades to 0 over 3 seconds
2. Wind continues (natural, not triggered)
3. Heartbeat begins at low volume (barely audible)
4. Player has ~5 seconds of "silence warning" before predator is visible
5. This is the audio equivalent of the visual "grass rustling" tell

### Music Philosophy
- **No orchestral score** — this is a survival game, not a cinematic
- **Percussion only** — bone drums, hollow wood, stone percussion
- **Adaptive layers** — danger adds percussion layers, safety removes them
- **Silence is music** — the absence of sound IS the soundtrack in danger moments

---

## Next Cycle Priorities (for Agent #17 VFX)

1. **Footstep dust particles** should sync with audio footstep events (not independent)
2. **TRex approach VFX** (ground shake, dust) should trigger at same radius as HeartbeatZone (600 units)
3. **River water VFX** should match AmbientZone_River_001 position (0, 800, 50)
4. **Fear stat spike VFX** (red screen flash) should co-trigger with heartbeat audio
5. **Herd dust clouds** at savanna zone (800, 200) — visual complement to herd rumble audio

---

*Audio Agent #16 — PROD_CYCLE_AUTO_20260617_014*
