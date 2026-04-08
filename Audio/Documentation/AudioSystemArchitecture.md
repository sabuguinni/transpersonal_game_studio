# AUDIO SYSTEM ARCHITECTURE
## Transpersonal Game Studio — Jurassic Survival Game
### Audio Agent — Technical Specification v1.0

---

## CORE PHILOSOPHY

**"O som que não existe é muitas vezes mais poderoso do que o som que existe"** — Walter Murch

**"Quando o áudio está perfeito, o jogador não pensa no áudio — pensa no mundo"**

O sistema de áudio deste jogo é construído sobre três pilares fundamentais:

1. **MEDO CONSTANTE** — O jogador deve sempre sentir-se como presa
2. **MUNDO VIVO** — Os dinossauros têm vidas sonoras independentes
3. **SILÊNCIO SUSPEITO** — A ausência de som é uma ferramenta narrativa

---

## TECHNICAL FOUNDATION

### Audio Engine: Unreal Engine 5 Audio Mixer
- **MetaSounds** para música adaptativa e efeitos procedurais
- **Soundscape** para ambientes dinâmicos
- **Audio Modulation** para transições emocionais
- **Spatialization** com binaural processing para imersão 3D

### Performance Targets
- **60fps PC / 30fps Console** — nunca comprometer performance
- **Max 200 concurrent sounds** com LOD automático
- **Memory budget: 512MB** para todos os assets de áudio
- **Latency < 50ms** para feedback imediato

---

## ADAPTIVE MUSIC SYSTEM

### Emotional States Framework

```cpp
enum class EEmotionalState : uint8
{
    Calm_Exploration,      // Exploração segura, descoberta
    Tense_Awareness,       // Algo não está certo
    Active_Danger,         // Predador próximo mas não detectou
    Immediate_Threat,      // Perseguição activa
    Combat_Survival,       // Luta pela vida
    Post_Trauma,          // Após escape de perigo
    Peaceful_Base,        // Segurança da base construída
    Discovery_Wonder      // Encontro com dinossauro pacífico
};
```

### Music Transition Rules

**CALM → TENSE**: Fade out melody, introduce subtle dissonance
**TENSE → DANGER**: Heartbeat rhythm, remove harmony
**DANGER → THREAT**: Full orchestral tension, staccato strings
**THREAT → COMBAT**: Percussive chaos, brass stabs
**COMBAT → POST_TRAUMA**: Sudden silence, then slow emotional rebuild

### MetaSound Implementation Strategy

1. **Horizontal Layering**: Base rhythm + Melody + Harmony + Tension layers
2. **Vertical Sequencing**: Seamless transitions between emotional states
3. **Parameter Modulation**: Real-time adjustment based on gameplay events
4. **Procedural Elements**: Randomized variations to prevent repetition

---

## DINOSAUR AUDIO BEHAVIOUR SYSTEM

### Individual Dinosaur Voice Design

Each dinosaur species has:
- **Unique vocal signature** — recognizable across distance
- **Emotional range** — calm, alert, aggressive, feeding, mating
- **Distance-based filtering** — realistic sound propagation
- **Directional audio** — head movement affects vocalization direction

### Procedural Variation System

```cpp
struct FDinosaurVoiceVariation
{
    float PitchRange;        // ±0.2 semitones
    float TimbreShift;       // Formant frequency adjustment
    float VolumeVariation;   // ±3dB natural variation
    float BreathingPattern;  // Individual breathing rhythm
};
```

### Behavioural Audio Triggers

- **Feeding**: Chewing, swallowing, satisfaction sounds
- **Movement**: Footsteps with weight variation, vegetation disturbance
- **Social**: Communication calls, territorial warnings
- **Environmental**: Reaction to weather, time of day
- **Player Proximity**: Awareness levels affect vocalization intensity

---

## ENVIRONMENTAL AUDIO SYSTEM

### Dynamic Soundscape Layers

**LAYER 1: Base Ambience**
- Forest wind through different vegetation types
- Distant geological sounds (waterfalls, rock shifts)
- Atmospheric pressure changes

**LAYER 2: Life Activity**
- Insect populations varying by time/weather
- Small creature movements in underbrush
- Plant sounds (creaking trees, rustling leaves)

**LAYER 3: Dinosaur Ecosystem**
- Distant dinosaur calls creating ecosystem awareness
- Predator-prey interactions audible from afar
- Territorial audio boundaries

**LAYER 4: Weather & Time**
- Rain intensity affecting all other layers
- Day/night transition audio
- Storm approach and retreat

### Occlusion & Propagation

**Advanced Audio Raytracing**:
- Real-time calculation of sound paths around obstacles
- Vegetation density affects high-frequency attenuation
- Cave and canyon reverb simulation
- Distance-based air absorption modeling

---

## PLAYER FEEDBACK AUDIO

### Survival State Audio Cues

**Health Status**:
- Heartbeat intensity correlates with health
- Breathing patterns indicate stamina
- Subtle audio distortion when near death

**Hunger/Thirst**:
- Stomach sounds for hunger
- Dry mouth audio for thirst
- Weakness affecting movement audio

**Fear Level**:
- Enhanced environmental audio sensitivity
- Auditory hallucinations at high fear
- Suppressed breathing when hiding

### Tool & Crafting Audio

**Material Impact Feedback**:
- Wood chopping with realistic impact variation
- Stone knapping with authentic chipping sounds
- Metal crafting with temperature-based audio

**Construction Audio**:
- Building placement with satisfying confirmation
- Structural integrity audio feedback
- Tool wear audio progression

---

## TECHNICAL IMPLEMENTATION

### MetaSound Graph Architecture

```
[Game State Input] → [Emotional State Manager] → [Music Layer Controller]
                                                      ↓
[Environmental Data] → [Soundscape Controller] → [Spatial Audio Mixer]
                                                      ↓
[Dinosaur AI State] → [Creature Audio Manager] → [3D Audio Output]
```

### Audio Memory Management

**Streaming Strategy**:
- Core ambience always loaded (50MB)
- Music stems loaded on-demand (100MB)
- Dinosaur voices cached by proximity (200MB)
- Player feedback sounds resident (25MB)
- Environmental one-shots streamed (137MB)

**LOD System**:
- **LOD 0**: Full quality within 50m
- **LOD 1**: Reduced sample rate 50-200m  
- **LOD 2**: Mono conversion beyond 200m
- **LOD 3**: Culled beyond 500m

### Performance Optimization

**CPU Budget Allocation**:
- Music system: 15% audio thread
- Environmental audio: 35% audio thread
- Dinosaur audio: 30% audio thread
- Player feedback: 10% audio thread
- Spatial processing: 10% audio thread

---

## INTEGRATION POINTS

### With Other Systems

**AI Behaviour Integration**:
- Dinosaur AI states trigger audio behaviour changes
- Audio cues influence AI decision making (predator audio affects prey)

**Weather System Integration**:
- Rain affects audio propagation and creature behaviour
- Wind direction influences sound travel

**Time of Day Integration**:
- Nocturnal vs diurnal creature audio activity
- Music emotional state influenced by time

**Player Construction Integration**:
- Base building affects local soundscape
- Constructed walls provide audio occlusion

---

## QUALITY ASSURANCE METRICS

### Audio Testing Criteria

**Immersion Metrics**:
- Player should identify dinosaur species by audio alone
- Emotional state transitions should feel natural
- No audio repetition noticed in 30-minute sessions

**Performance Metrics**:
- No audio dropouts during peak load
- Consistent 60fps with full audio system active
- Memory usage within 512MB budget

**Accessibility Metrics**:
- Subtitle accuracy for all dinosaur vocalizations
- Visual indicators for directional audio cues
- Hearing-impaired player testing validation

---

*Document Version: 1.0*
*Created by: Audio Agent #16*
*Date: March 2026*
*Next Review: Integration Phase*