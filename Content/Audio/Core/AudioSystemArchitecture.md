# Audio System Architecture — Jurassic Survival Game
## Transpersonal Game Studio — Audio Agent #16
## CYCLE_ID: PROD_JURASSIC_001

---

## CORE DESIGN PRINCIPLES

### 1. EMOTIONAL AUDIO STATES
- **PREY MODE** (default): Jogador vulnerável, mundo ameaçador
- **OBSERVATION MODE**: Jogador escondido, observando dinossauros
- **DANGER MODE**: Predador próximo, tensão máxima
- **DOMESTICATION MODE**: Interacção pacífica com herbívoros
- **EXPLORATION MODE**: Descoberta de novos territórios

### 2. ADAPTIVE MUSIC SYSTEM
Música que muda baseada em:
- Proximidade de predadores (sistema de ameaça)
- Hora do dia (ciclo circadiano)
- Bioma actual (floresta densa, planície, rio)
- Estado emocional do jogador (stress, calma, curiosidade)
- Actividade dos dinossauros (caça, descanso, migração)

### 3. PROCEDURAL SOUNDSCAPE
- **Ambient Layers**: Vento, folhas, insectos, água
- **Dynamic Wildlife**: Sons de dinossauros baseados em comportamento IA
- **Interactive Foley**: Passos adaptativos ao terreno
- **Spatial Audio**: Posicionamento 3D preciso para sobrevivência

---

## METASOUNDS ARCHITECTURE

### MASTER SUBMIX CHAIN
```
Sound Sources → Source Effects → Spatialization → Submixes → Master Effects → Output
```

### CORE SUBMIXES
1. **Music_Submix**: Música adaptativa
2. **Ambience_Submix**: Sons ambientes e atmosfera
3. **Dinosaur_Submix**: Vocalizações e movimentos de dinossauros
4. **Player_Submix**: Foley do jogador e UI
5. **Interactive_Submix**: Objectos interactivos e ferramentas
6. **Dialogue_Submix**: Vozes e narração (ElevenLabs)

### DYNAMIC MIXING RULES
- **Danger State**: Music -6dB, Ambience -3dB, Dinosaur +3dB
- **Stealth State**: Player -12dB, Ambience +6dB, Music -9dB
- **Domestication State**: Music +3dB (warmer), Dinosaur -6dB (friendly)

---

## THREAT DETECTION SYSTEM

### PROXIMITY ZONES
- **SAFE ZONE** (50m+): Música exploratória, ambience relaxado
- **AWARENESS ZONE** (20-50m): Transição musical, ambience reduzido
- **DANGER ZONE** (5-20m): Música tensão, respiração do jogador
- **CRITICAL ZONE** (0-5m): Música suspensa, heartbeat, predator focus

### AUDIO TRIGGERS
- Pegadas de predadores → Low-frequency rumble
- Quebra de galhos → Directional audio cue
- Respiração de dinossauros → Spatial breathing sounds
- Movimento de vegetação → Rustling intensity based on creature size

---

## DINOSAUR AUDIO BEHAVIOUR

### SPECIES-SPECIFIC SIGNATURES
- **T-Rex**: Sub-bass roar (20-60Hz), ground tremor, heavy breathing
- **Velociraptor**: Sharp clicks, coordinated pack calls, stealth movement
- **Triceratops**: Low rumbling, herd communication, defensive snorts
- **Pteranodon**: Wing flaps, aerial screeches, dive warnings

### BEHAVIOURAL AUDIO STATES
- **Hunting**: Predator calls, coordinated movement sounds
- **Feeding**: Tearing sounds, satisfied rumbles, bone cracks
- **Resting**: Gentle breathing, occasional snores, settling sounds
- **Alert**: Sharp intakes, movement preparation, warning calls
- **Territorial**: Dominance roars, ground stomping, threat displays

---

## ADAPTIVE MUSIC COMPOSITION

### LAYERED ORCHESTRATION
- **Foundation**: Ambient drones, nature textures
- **Rhythm**: Heartbeat-sync percussion for tension
- **Melody**: Sparse, haunting themes
- **Harmony**: Dissonance for danger, consonance for safety
- **Dynamics**: Real-time volume and intensity shifts

### MUSICAL THEMES
1. **Survival Theme**: Minor pentatonic, sparse instrumentation
2. **Wonder Theme**: Major modes, orchestral swells for discovery
3. **Predator Theme**: Atonal clusters, percussive attacks
4. **Domestication Theme**: Warm strings, gentle woodwinds
5. **Ancient World Theme**: Ethnic instruments, primordial textures

### TRANSITION SYSTEM
- **Horizontal Re-sequencing**: Change musical sections based on gameplay
- **Vertical Remixing**: Add/remove layers based on emotional intensity
- **Crossfading**: Smooth transitions between emotional states
- **Stingers**: Musical punctuation for specific events

---

## TECHNICAL IMPLEMENTATION

### METASOUNDS GRAPHS
1. **AdaptiveMusicController**: Main music state machine
2. **ThreatDetectionAudio**: Proximity-based audio triggers
3. **DinosaurVocalizer**: Species-specific sound generation
4. **EnvironmentalAmbience**: Biome-based soundscape
5. **PlayerAudioController**: Foley and feedback systems

### AUDIO MEMORY MANAGEMENT
- **Streaming**: Large ambient loops and music stems
- **Compressed**: Dinosaur vocalizations and one-shots
- **Uncompressed**: Critical UI sounds and stingers
- **Procedural**: Wind, water, and particle-based effects

### PERFORMANCE TARGETS
- **Total Audio Memory**: <150MB on console
- **Concurrent Voices**: Max 64 (32 3D, 32 2D)
- **CPU Usage**: <5% of audio thread
- **Latency**: <50ms for interactive sounds

---

## INTEGRATION POINTS

### WITH OTHER SYSTEMS
- **AI Behaviour Trees**: Dinosaur audio state sync
- **Weather System**: Dynamic ambience adaptation
- **Time of Day**: Circadian audio cycles
- **Player Health**: Stress-induced audio effects
- **Inventory System**: Tool-specific audio feedback

### BLUEPRINT INTERFACES
- **AudioStateManager**: Central audio state controller
- **ThreatLevelInterface**: Danger level communication
- **BiomeAudioInterface**: Environmental audio switching
- **DinosaurAudioInterface**: Creature-specific audio triggers

---

*Audio System Architecture v1.0*
*Created by Audio Agent #16 — Transpersonal Game Studio*