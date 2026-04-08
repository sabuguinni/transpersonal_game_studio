# ARQUITECTURA TÉCNICA — TRANSPERSONAL GAME STUDIO
## Jurassic Survival Game — Engine Architecture Document
**Versão:** 1.0  
**Data:** Março 2026  
**Responsável:** Engine Architect (Agente #02)  
**Aprovação:** OBRIGATÓRIA para todos os agentes técnicos

---

## VISÃO ARQUITECTURAL

### Conceito Base
- **Género:** Survival de mundo aberto
- **Escala:** Regional (4-8km²)
- **Período:** Jurássico/Cretáceo
- **Plataforma Primária:** PC Windows (60fps)
- **Plataforma Secundária:** Consolas (30fps)
- **Engine:** Unreal Engine 5.3+

### Pilares Técnicos
1. **Performance Primeiro** — 60fps PC, 30fps consola é inviolável
2. **Escalabilidade** — Suporta mundos até 8km² sem degradação
3. **Modularidade** — Cada sistema é independente e substituível
4. **Robustez** — Zero crashes em builds de produção

---

## ARQUITECTURA DE SISTEMAS

### Core Engine Stack
```
┌─────────────────────────────────────────────────────────────┐
│                    GAME LAYER                               │
├─────────────────────────────────────────────────────────────┤
│  Survival │ Dinosaur │ World  │ Player │ Quest │ Audio │ VFX │
│  Systems  │    AI    │  Gen   │ Systems│ System│ Engine│ Eng │
├─────────────────────────────────────────────────────────────┤
│                  FRAMEWORK LAYER                            │
├─────────────────────────────────────────────────────────────┤
│  Physics  │ Collision│ Memory │ Thread │ Asset │ Render│ Net │
│  Engine   │ Detection│  Mgmt  │  Pool  │  Mgmt │ Pipeline│ Mgr │
├─────────────────────────────────────────────────────────────┤
│                   UNREAL ENGINE 5                           │
├─────────────────────────────────────────────────────────────┤
│ Nanite │ Lumen │ World │ Chaos │ MetaSound │ Niagara │ VSM  │
│        │       │ Part. │ Physics│           │         │      │
└─────────────────────────────────────────────────────────────┘
```

### Módulos Principais
1. **TranspersonalGame** — Módulo principal do jogo
2. **CoreSystems** — Física, colisão, ragdoll, destruição
3. **WorldGeneration** — PCG, terrenos, biomas
4. **DinosaurAI** — Comportamento, rotinas, memória
5. **SurvivalSystems** — Fome, sede, crafting, base building
6. **AudioEngine** — MetaSounds, música adaptativa
7. **VFXSystems** — Niagara, weather, particles

---

## WORLD PARTITION ARCHITECTURE

### Configuração Obrigatória
- **World Partition:** ATIVADO para todos os níveis
- **Cell Size:** 512m x 512m (optimizado para 4-8km²)
- **Loading Range:** 1024m (2x cell size)
- **One File Per Actor:** ATIVADO
- **Data Layers:** OBRIGATÓRIO para organização

### Estrutura de Células
```
Grid Layout (8km x 8km):
┌────┬────┬────┬────┬────┬────┬────┬────┐
│ 0,0│ 1,0│ 2,0│ 3,0│ 4,0│ 5,0│ 6,0│ 7,0│
├────┼────┼────┼────┼────┼────┼────┼────┤
│ 0,1│ 1,1│ 2,1│ 3,1│ 4,1│ 5,1│ 6,1│ 7,1│
├────┼────┼────┼────┼────┼────┼────┼────┤
│ 0,2│ 1,2│ 2,2│ 3,2│ 4,2│ 5,2│ 6,2│ 7,2│
└────┴────┴────┴────┴────┴────┴────┴────┘
```

### Data Layers Obrigatórias
- **DL_Terrain** — Paisagem base
- **DL_Vegetation** — Árvores, plantas, foliage
- **DL_Structures** — Rochas, cavernas, ruínas
- **DL_Dinosaurs** — Spawn points e rotas dos dinossauros
- **DL_Resources** — Pontos de recursos para crafting
- **DL_Audio** — Ambient sounds, audio triggers
- **DL_Lighting** — Luzes dinâmicas e atmosfera

---

## NANITE & RENDERING PIPELINE

### Nanite Configuration
- **OBRIGATÓRIO** para toda a geometria estática
- **Minimum Triangle Count:** 1000+ para activar Nanite
- **Fallback Mesh:** Sempre configurado para ray-tracing
- **LOD Strategy:** Nanite automático (sem LODs manuais)

### Rendering Stack
```
Frame Rendering Pipeline:
┌─────────────────┐
│   Scene Setup   │ ← Camera, frustum, culling
├─────────────────┤
│ Nanite Raster   │ ← Geometry rendering
├─────────────────┤
│ Lumen GI        │ ← Global illumination
├─────────────────┤
│ Virtual Shadows │ ← High-res shadows
├─────────────────┤
│ Post Processing │ ← TAA, bloom, tone mapping
├─────────────────┤
│   UI Overlay    │ ← HUD, menus
└─────────────────┘
```

### Performance Targets
- **PC (Epic Settings):** 60fps @ 1440p
- **PC (High Settings):** 60fps @ 1080p
- **Console (High Settings):** 30fps @ 4K upscaled
- **Console (Medium Settings):** 60fps @ 1080p upscaled

---

## LUMEN GLOBAL ILLUMINATION

### Configuration
- **Dynamic GI:** ATIVADO (substitui lightmaps)
- **Software Ray Tracing:** Fallback para hardware não suportado
- **Hardware Ray Tracing:** Preferido quando disponível
- **Scene View Distance:** 400m (optimizado para survival)
- **Final Gather Quality:** High (para interiores de cavernas)

### Lighting Strategy
- **Primary Light:** Directional Light (sol)
- **Sky Light:** Atmospheric scattering
- **Local Lights:** Point/Spot lights para cavernas e fogueiras
- **Emissive Materials:** Cristais, lava, bioluminescência

---

## VIRTUAL SHADOW MAPS

### Configuration
- **VSM:** ATIVADO por defeito
- **Page Pool Size:** 128MB (ajustável por scalability)
- **Clipmap Levels:** 6-22 (64cm a 40km)
- **SMRT Ray Count:** 8 rays (Epic), 4 rays (High)
- **Caching:** ATIVADO para geometria estática

### Shadow Quality Tiers
- **Epic:** 16k VSM, 8 SMRT rays, full caching
- **High:** 16k VSM, 4 SMRT rays, limited caching
- **Medium:** 8k VSM, 2 SMRT rays, minimal caching
- **Low:** Traditional shadow maps

---

## CHAOS PHYSICS ARCHITECTURE

### Physics Configuration
- **Chaos Physics:** ATIVADO (substitui PhysX)
- **Substeps:** 4 (60fps), 2 (30fps)
- **Solver Iterations:** 8 position, 4 velocity
- **Collision Channels:** Custom setup para dinossauros

### Collision Channels
```cpp
// Custom Collision Channels
ECC_Dinosaur        = ECC_GameTraceChannel1
ECC_Player          = ECC_GameTraceChannel2  
ECC_Environment     = ECC_GameTraceChannel3
ECC_Projectile      = ECC_GameTraceChannel4
ECC_Interaction     = ECC_GameTraceChannel5
ECC_Audio           = ECC_GameTraceChannel6
```

### Destruction System
- **Chaos Destruction:** Para árvores, rochas, estruturas
- **Fracture Depth:** 3 níveis máximo
- **Debris Lifetime:** 30 segundos
- **Max Debris Count:** 500 simultâneos

---

## MASS AI FRAMEWORK

### Configuration
- **Mass Entity:** ATIVADO para crowds de dinossauros
- **Max Entities:** 50,000 simultâneos
- **LOD Distances:** 100m/500m/1000m/2000m
- **Update Frequency:** 60Hz/30Hz/10Hz/1Hz por LOD

### AI Behaviour Stack
```
┌─────────────────────────────────────┐
│        Behaviour Trees              │ ← Individual dinosaur logic
├─────────────────────────────────────┤
│        State Trees                  │ ← High-level states
├─────────────────────────────────────┤
│        Mass Simulation              │ ← Crowd behaviour
├─────────────────────────────────────┤
│        Navigation Mesh             │ ← Pathfinding
└─────────────────────────────────────┘
```

---

## METASOUNDS AUDIO ARCHITECTURE

### Audio Pipeline
- **MetaSounds:** OBRIGATÓRIO para toda a música e SFX
- **Spatial Audio:** 3D positioning com atenuação realista
- **Adaptive Music:** Baseada em tensão, localização, hora do dia
- **Dynamic Range:** -23 LUFS (broadcast standard)

### Audio Categories
- **Music:** Adaptive orchestral score
- **Ambience:** Forest sounds, wind, water
- **Dinosaur SFX:** Roars, footsteps, breathing
- **Player SFX:** Footsteps, crafting, inventory
- **Environmental:** Weather, fire, water

---

## NIAGARA VFX SYSTEM

### VFX Architecture
- **Niagara:** OBRIGATÓRIO para todos os efeitos
- **LOD Chain:** 3 níveis (Near/Medium/Far)
- **GPU Simulation:** Para particles > 1000
- **CPU Simulation:** Para particles < 1000

### Effect Categories
- **Weather:** Rain, fog, wind particles
- **Environmental:** Dust, leaves, water splashes
- **Magical:** Gem effects, teleportation
- **Combat:** Blood, impact effects
- **Atmospheric:** God rays, volumetric fog

---

## MEMORY ARCHITECTURE

### Memory Pools
```cpp
// Memory Layout (8GB target)
Engine Core:        1.5GB
World Streaming:    2.0GB
Audio:             512MB
Textures:          2.5GB
Meshes:            1.0GB
AI/Gameplay:       512MB
Reserve:           512MB
```

### Streaming Strategy
- **Texture Streaming:** Virtual textures para paisagens
- **Audio Streaming:** Compressed audio com 3-second buffer
- **Mesh Streaming:** Nanite automático
- **Animation Streaming:** LOD-based animation compression

---

## PERFORMANCE MONITORING

### Profiling Tools
- **Unreal Insights:** Primary profiling tool
- **GPU Profiler:** Frame time analysis
- **Memory Profiler:** Allocation tracking
- **Network Profiler:** Multiplayer (futuro)

### Performance Metrics
```cpp
// Target Frame Times
Epic (60fps):    16.67ms
High (60fps):    16.67ms  
Medium (30fps):  33.33ms
Low (30fps):     33.33ms

// Breakdown
Render Thread:   12ms max
Game Thread:     14ms max
GPU:            15ms max
```

---

## SCALABILITY FRAMEWORK

### Scalability Groups
- **ViewDistance:** 0.5x / 0.75x / 1.0x / 1.25x
- **AntiAliasing:** Off / FXAA / TAA / TSR
- **PostProcessing:** Low / Medium / High / Epic
- **Textures:** 512 / 1024 / 2048 / 4096
- **Effects:** 25% / 50% / 75% / 100%
- **Foliage:** 0.5x / 0.75x / 1.0x / 1.25x

### Platform Presets
```ini
[PC_Epic]
ViewDistance=1.25
AntiAliasing=3 (TSR)
PostProcessing=3
Textures=3
Effects=3
Foliage=1.25

[Console_High]
ViewDistance=1.0
AntiAliasing=2 (TAA)
PostProcessing=2
Textures=2
Effects=2
Foliage=1.0
```

---

## BUILD PIPELINE

### Build Configurations
- **Debug:** Full symbols, no optimization
- **Development:** Partial optimization, logging
- **Test:** Full optimization, test features
- **Shipping:** Maximum optimization, no debug

### Packaging Pipeline
```bash
# Standard build process
1. Source compilation (C++)
2. Blueprint compilation
3. Asset cooking
4. Texture compression
5. Audio compression
6. Pak file generation
7. Platform packaging
```

---

## REGRAS TÉCNICAS INVIOLÁVEIS

### 1. Performance Rules
- **60fps PC é obrigatório** — qualquer feature que quebra isto é rejeitada
- **30fps consola é obrigatório** — sem excepções
- **16GB RAM máximo** — incluindo OS overhead
- **Loading times < 10 segundos** — entre áreas do mundo

### 2. World Partition Rules
- **Todos os níveis usam World Partition** — sem excepções
- **Cell size 512m** — não modificar sem aprovação do Engine Architect
- **One File Per Actor obrigatório** — para colaboração de equipa
- **Data Layers organizados** — seguir convenção definida

### 3. Rendering Rules
- **Nanite obrigatório** — para toda a geometria estática > 1000 tris
- **Lumen obrigatório** — sem lightmaps estáticos
- **Virtual Shadow Maps obrigatório** — sem shadow maps tradicionais
- **Material complexity < 300 instructions** — para performance

### 4. Audio Rules
- **MetaSounds obrigatório** — sem audio cues tradicionais
- **Spatial audio obrigatório** — para imersão
- **-23 LUFS target** — para consistência de volume
- **Compressed audio** — OGG Vorbis para música, WAV para SFX curtos

### 5. Memory Rules
- **8GB target total** — incluindo OS
- **Streaming obrigatório** — para assets > 10MB
- **Garbage collection < 5ms** — sem hitches perceptíveis
- **Memory pools** — para allocations frequentes

### 6. Code Rules
- **Epic C++ Standards** — sem desvios
- **Modular architecture** — cada sistema independente
- **Interface-based design** — para testabilidade
- **Performance-first** — optimização não é opcional

---

## APROVAÇÃO E DISTRIBUIÇÃO

**Este documento é OBRIGATÓRIO para todos os agentes técnicos.**

Qualquer desvio destas especificações requer aprovação explícita do Engine Architect e Studio Director.

**Próximo Agente:** Core Systems Programmer (#03)  
**Inputs Necessários:** Esta arquitectura técnica completa  
**Outputs Esperados:** Implementação dos sistemas base (física, colisão, ragdoll, destruição)

---

*Documento criado pelo Engine Architect — Agente #02*  
*Transpersonal Game Studio — Março 2026*