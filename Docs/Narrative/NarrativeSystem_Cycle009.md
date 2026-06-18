# Narrative & Dialogue System — Production Cycle 009
**Agent #15 — Narrative & Dialogue Agent**
**Cycle:** PROD_CYCLE_AUTO_20260618_008

---

## Voice Lines Produced This Cycle

### 1. HerdObserver_Monologue
**Quest:** Herd Observation — "Observe the Herd"
**Trigger:** Player reaches `QuestWP_Observe_Complete` near `NPC_Observer_HerdPost_001`
**Text:**
> "I have watched the giants for three sunrises now. They move together — always together. When one stops to drink, the others wait. When the young one stumbled in the mud, two adults turned back. They are not just animals. They remember. They protect. Do not hunt what you do not understand."

**Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781782906387_HerdObserver_Monologue.mp3
**Duration:** ~20s | **Character:** Internal monologue / journal voice

---

### 2. Elder_RaptorWarning
**Quest:** Raptor Hunt — "Drive Off the Pack"
**Trigger:** Player enters `Dialogue_Trigger_SafeCamp_001` near `NPC_Elder_SafeCamp_West_001`
**Text:**
> "Listen to me, young one. I have lived through two dry seasons in this valley. The pack hunters — they do not attack when you face them. They wait for you to turn your back. Always keep your back to stone. Always."

**Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781782908576_Elder_RaptorWarning.mp3
**Duration:** ~15s | **Character:** Elder NPC — grizzled survivor, pragmatic

---

### 3. Survivor_CampEstablished
**Quest:** Establish Camp — "Build Your First Shelter"
**Trigger:** Player completes camp construction near `NPC_Tutorial_CampSite_001`
**Text:**
> "We made it. The camp holds. Three walls of stone, a fire that will not die in the rain. This is ours now. This place — we earned it with blood and hunger and cold nights. Rest. Tomorrow we build higher."

**Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781782926401_Survivor_CampEstablished.mp3
**Duration:** ~14s | **Character:** Survivor NPC — exhausted but triumphant

---

## Actors Deployed This Cycle

### NPC Dialogue Markers (TargetPoint)
| Label | Location | Quest | Purpose |
|-------|----------|-------|---------|
| NPC_Survivor_HighGround_001 | (-1000,-2800,200) | Stampede | Post-escape survivor dialogue |
| NPC_Elder_SafeCamp_West_001 | (-3100,-600,100) | Raptor Hunt | Pre-quest elder warning |
| NPC_Observer_HerdPost_001 | (2100,2600,150) | Herd Observation | Internal monologue trigger |
| NPC_Tutorial_CampSite_001 | (-3000,-500,100) | Establish Camp | Tutorial NPC dialogue |
| NPC_Survivor_PanicEscape_001 | (-800,3600,100) | Panic Event | Post-TRex encounter survivor |

### Dialogue Trigger Zones (TriggerSphere)
| Label | Location | Radius | NPC |
|-------|----------|--------|-----|
| Dialogue_Trigger_HighGround_001 | (-1000,-2800,200) | 300 | NPC_Survivor_HighGround_001 |
| Dialogue_Trigger_SafeCamp_001 | (-3100,-600,100) | 350 | NPC_Elder_SafeCamp_West_001 |
| Dialogue_Trigger_HerdPost_001 | (2100,2600,150) | 400 | NPC_Observer_HerdPost_001 |
| Dialogue_Trigger_CampSite_001 | (-3000,-500,100) | 300 | NPC_Tutorial_CampSite_001 |
| Dialogue_Trigger_PanicEscape_001 | (-800,3600,100) | 300 | NPC_Survivor_PanicEscape_001 |

### Journal Entry Markers (TargetPoint)
| Label | Location | Lore Content |
|-------|----------|-------------|
| Journal_BoneMarker_Stampede_001 | (600,-3100,100) | Carved bones showing herd migration path |
| Journal_RockCarving_Raptor_001 | (-1600,1400,100) | Claw marks on cliff face — raptor territory |
| Journal_WaterSource_Herd_001 | (2200,2400,100) | Footprint patterns in mud at watering hole |
| Journal_FirePit_OldCamp_001 | (-3200,-700,100) | Old campfire — charred bones, previous survivor |
| Journal_HighGround_View_001 | (-1100,-2900,250) | Panoramic overlook — valley observation point |
| Journal_RaptorDen_Entry_001 | (-1700,1600,100) | Raptor den — feathers, egg fragments |
| Journal_TRex_Territory_001 | (200,100,100) | Massive footprints, crushed trees — TRex zone |

### Ambient Audio Cue Markers (TargetPoint)
| Label | Location | Panic Event Sequence |
|-------|----------|---------------------|
| AmbientCue_DistantRoar_001 | (300,200,100) | Step 1 — distant TRex roar |
| AmbientCue_BirdScatter_001 | (200,300,150) | Step 2 — birds flee canopy |
| AmbientCue_Silence_001 | (100,100,100) | Step 3 — unnatural silence |
| AmbientCue_TRexApproach_001 | (0,0,100) | Step 4 — ground tremor + roar |
| AmbientCue_Stampede_001 | (500,-3000,100) | Stampede event — thunder of hooves |
| AmbientCue_RaptorClick_001 | (-1500,1500,100) | Raptor hunt — clicking communication |

---

## Narrative Architecture — Cumulative (Cycles 001-009)

### Quest Emotional Arcs (5 quests fully scaffolded)

#### Quest 1: "Survive the Stampede"
- **Inciting incident:** Ground shakes, dust cloud on horizon
- **Rising action:** Player must read terrain, find high ground
- **Climax:** Herd thunders through valley below
- **Resolution:** Survivor NPC at high ground — shares knowledge of migration patterns
- **Player takeaway:** Herds are predictable if you understand the land

#### Quest 2: "Drive Off the Pack"
- **Inciting incident:** Raptor clicks heard at camp perimeter
- **Rising action:** Elder NPC warns — face them, back to stone
- **Climax:** Player must drive raptors away using fire/noise
- **Resolution:** Pack retreats — player learns raptor psychology
- **Player takeaway:** Intelligence beats strength — raptors respond to confidence

#### Quest 3: "Observe the Herd"
- **Inciting incident:** Player spots Brachiosaurus herd at distance
- **Rising action:** Stealth approach — avoid startling the herd
- **Climax:** Close observation — witness herd social behaviour
- **Resolution:** Internal monologue — player records observations
- **Player takeaway:** Herbivores are not passive — they have social bonds

#### Quest 4: "Find High Ground" (Panic Event)
- **Inciting incident:** Ambient audio sequence — distant roar → bird scatter → silence
- **Rising action:** Player must interpret environmental cues
- **Climax:** TRex emerges — player must reach high ground
- **Resolution:** Survivor NPC confirms: silence means danger
- **Player takeaway:** The world communicates — learn to listen

#### Quest 5: "Establish Camp"
- **Inciting incident:** Night approaching, no shelter
- **Rising action:** Tutorial NPC guides resource gathering
- **Climax:** Camp construction complete
- **Resolution:** Survivor NPC — "We earned this place"
- **Player takeaway:** Permanence requires effort — the land must be claimed

---

## Dialogue System Integration Map

### Agent Dependencies
| System | Agent | Integration Point |
|--------|-------|------------------|
| NPC Behavior Trees | #11 | NPC_* markers → BT spawn locations |
| Raptor AI | #12 | AmbientCue_RaptorClick_001 → AI trigger |
| Herd Simulation | #13 | NPC_Observer_HerdPost_001 → crowd observation |
| Quest Triggers | #14 | Dialogue_Trigger_* → quest state machine |
| Audio System | #16 | AmbientCue_* markers → MetaSound cues |

### Voice Line → Actor Binding Table
| Voice URL | Actor Label | Trigger Condition |
|-----------|-------------|------------------|
| HerdObserver_Monologue.mp3 | NPC_Observer_HerdPost_001 | QuestWP_Observe_Complete reached |
| Elder_RaptorWarning.mp3 | NPC_Elder_SafeCamp_West_001 | Dialogue_Trigger_SafeCamp_001 entered |
| Survivor_CampEstablished.mp3 | NPC_Tutorial_CampSite_001 | Camp construction complete |

---

## Lore Bible Entries — Cycle 009 Additions

### The Valley Memory System
Prehistoric humans in this world do not write. They remember through:
1. **Bone markers** — carved bones left at significant locations (Journal_BoneMarker_*)
2. **Rock carvings** — territorial and navigational marks (Journal_RockCarving_*)
3. **Oral transmission** — elder NPCs carry survival knowledge
4. **Environmental reading** — footprints, broken branches, bird behaviour

This is the game's "lore delivery system" — no books, no text walls. Knowledge is spatial and embodied.

### NPC Character Profiles

#### The Elder (NPC_Elder_SafeCamp_West_001)
- **Age:** 45+ (ancient by prehistoric standards)
- **Scars:** Multiple — healed raptor claw marks on left arm
- **Communication:** Sparse, direct, gesture-heavy
- **Knowledge domain:** Predator behaviour, territorial patterns, survival tactics
- **Motivation:** Ensure the tribe survives — personal survival is secondary
- **Dialogue style:** Short declarative sentences. No philosophy. Pure pragmatism.

#### The Survivor (NPC_Survivor_HighGround_001 / NPC_Survivor_PanicEscape_001)
- **Age:** 25-30
- **State:** Traumatised but functional
- **Communication:** Breathless, fragmented, urgent
- **Knowledge domain:** Recent events — what just happened, what's coming
- **Motivation:** Not die. Find others. Build something.
- **Dialogue style:** Present tense. Short bursts. Pauses mid-sentence.

---

## MAP_SAVED: True
## Total Narrative Actors Deployed (Cycles 001-009): ~85 actors
