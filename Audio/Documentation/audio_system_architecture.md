# ARQUITECTURA DE ÁUDIO — TRANSPERSONAL GAME STUDIO
## Jogo de Sobrevivência Jurássico
**Versão 1.0 — Março 2026**

---

## FILOSOFIA CENTRAL

**"O som que não existe é muitas vezes mais poderoso do que o som que existe."** — Walter Murch

O áudio neste jogo não é decoração. É o sistema nervoso do mundo. Cada som comunica informação vital sobre:
- Proximidade de predadores
- Estado emocional do mundo
- Oportunidades de sobrevivência
- Mudanças ambientais críticas

### PRINCÍPIOS FUNDAMENTAIS

1. **TENSÃO CONSTANTE** — O silêncio é suspeito. Quando a floresta fica quieta, algo perigoso está próximo.

2. **MÚSICA INVISÍVEL** — A música adaptativa muda o estado emocional do jogador sem que ele perceba conscientemente.

3. **INFORMAÇÃO SONORA** — Cada som tem significado de sobrevivência. Nenhum som é apenas estético.

4. **ESPACIALIZAÇÃO PRECISA** — O jogador deve conseguir localizar ameaças apenas pelo som.

---

## ARQUITECTURA TÉCNICA

### SISTEMA DE CAMADAS ADAPTATIVAS

```
CAMADA 1: AMBIENTE BASE
├── Vento através da vegetação
├── Água corrente (rios, chuva)
├── Insectos e pequenos animais
└── Ressonância da paisagem

CAMADA 2: VIDA SELVAGEM ATIVA
├── Dinossauros herbívoros (pastoreio, movimento)
├── Predadores (respiração, passos distantes)
├── Aves (alertas, migração)
└── Vida aquática

CAMADA 3: TENSÃO DINÂMICA
├── Silêncios súbitos (predador próximo)
├── Intensificação gradual (perigo crescente)
├── Alívio (área segura)
└── Climax (confronto direto)

CAMADA 4: MÚSICA ADAPTATIVA
├── Estados emocionais base
├── Transições baseadas em gameplay
├── Temas de personagens/criaturas
└── Progressão narrativa
```

### ESTADOS MUSICAIS ADAPTATIVOS

#### EXPLORAÇÃO SEGURA
- **Tom:** Maravilhamento cauteloso
- **Instrumentação:** Cordas suaves, madeiras, percussão orgânica
- **Dinâmica:** pp a mf
- **Trigger:** Nenhuma ameaça detectada por 60+ segundos

#### TENSÃO CRESCENTE
- **Tom:** Ansiedade controlada
- **Instrumentação:** Drones graves, metais distantes, percussão irregular
- **Dinâmica:** mp a f
- **Trigger:** Movimento de predador detectado (>100m)

#### PERIGO IMINENTE
- **Tom:** Medo agudo
- **Instrumentação:** Clusters dissonantes, percussão agressiva, respiração
- **Dinâmica:** f a ff
- **Trigger:** Predador em range de ataque (<50m)

#### CONFRONTO
- **Tom:** Adrenalina pura
- **Instrumentação:** Orquestra completa, ritmos tribais, elementos eletrônicos
- **Dinâmica:** ff a fff
- **Trigger:** Combate ativo

#### FUGA
- **Tom:** Pânico controlado
- **Instrumentação:** Ostinatos rápidos, metais urgentes, percussão de perseguição
- **Dinâmica:** f a ff
- **Trigger:** Jogador correndo de predador

#### SEGURANÇA TEMPORÁRIA
- **Tom:** Alívio cauteloso
- **Instrumentação:** Redução gradual para cordas e madeiras
- **Dinâmica:** f decrescendo para mp
- **Trigger:** Jogador escondido ou em base

---

## SISTEMA DE ESPACIALIZAÇÃO 3D

### ATENUAÇÃO BASEADA EM REALISMO
- **Herbívoros grandes:** Audíveis até 500m (passos, respiração)
- **Predadores:** Audíveis até 200m (movimento furtivo)
- **Ambiente:** Audível até 1km (vento, água, ecos)
- **Jogador:** Audível até 50m (passos, equipamento)

### OCLUSÃO E REVERB DINÂMICOS
- **Floresta densa:** Reverb curto, oclusão alta
- **Clareiras:** Reverb médio, propagação livre
- **Canyons:** Reverb longo, ecos múltiplos
- **Cavernas:** Reverb muito longo, frequências graves amplificadas

---

## SOUND DESIGN ESPECÍFICO

### DINOSSAUROS — ASSINATURAS SONORAS

#### HERBÍVOROS (Não-ameaçadores)
- **Triceratops:** Respiração profunda, mastigação rítmica, passos pesados
- **Parasaurolophus:** Chamadas melódicas, movimento em grupo
- **Ankylosaurus:** Arrastar de cauda blindada, respiração lenta

#### PREDADORES (Ameaçadores)
- **T-Rex:** Respiração que ecoa, passos sísmicos, rugido que paralisa
- **Velociraptor:** Garras em superfícies, comunicação em matilha, movimento ágil
- **Carnotaurus:** Galope pesado, bufadas agressivas, investidas

### AMBIENTE — CAMADAS PROCEDURAIS

#### DIA
- **Base:** Vento suave, folhas, insetos distantes
- **Atividade:** Movimento de herbívoros, aves, água
- **Dinâmica:** Crescimento e diminuição natural de atividade

#### NOITE
- **Base:** Vento mais intenso, sons noturnos, silêncios súbitos
- **Atividade:** Predadores ativos, sons misteriosos, ecos distantes
- **Dinâmica:** Tensão constante, picos de atividade

#### CHUVA
- **Base:** Chuva em diferentes superfícies, trovões distantes
- **Modificação:** Todos os sons ficam abafados, visibilidade reduzida
- **Gameplay:** Mascaramento de movimento do jogador

---

## IMPLEMENTAÇÃO TÉCNICA

### METASOUNDS — ESTRUTURA MODULAR

#### MASTER METASOUND: ADAPTIVE_MUSIC_CONTROLLER
```
INPUTS:
- ThreatLevel (0.0 - 1.0)
- EnvironmentType (Enum)
- TimeOfDay (0.0 - 24.0)
- WeatherIntensity (0.0 - 1.0)
- PlayerState (Enum)

OUTPUTS:
- Music_Layer_Ambient
- Music_Layer_Tension
- Music_Layer_Action
- Reverb_Send
- Filter_Cutoff
```

#### CREATURE_AUDIO_SYSTEM
```
INPUTS:
- CreatureType (Enum)
- DistanceToPlayer (Float)
- ActivityState (Enum)
- HealthPercentage (0.0 - 1.0)

OUTPUTS:
- Spatialized_Audio
- Footstep_Triggers
- Vocalization_Triggers
- Breathing_Audio
```

#### ENVIRONMENT_AUDIO_MIXER
```
INPUTS:
- BiomeType (Enum)
- WeatherState (Enum)
- TimeOfDay (Float)
- PlayerMovementSpeed (Float)

OUTPUTS:
- Ambient_Layer_Base
- Ambient_Layer_Weather
- Ambient_Layer_Activity
- Occlusion_Filter
```

### SUBMIX ROUTING

```
MASTER SUBMIX
├── MUSIC_SUBMIX
│   ├── Adaptive_Music_Bus
│   └── Stinger_Bus
├── SFX_SUBMIX
│   ├── Creatures_Bus
│   ├── Environment_Bus
│   ├── Player_Bus
│   └── Interactive_Objects_Bus
├── DIALOGUE_SUBMIX
│   └── Scientist_Thoughts_Bus
└── REVERB_SUBMIX
    ├── Forest_Reverb
    ├── Canyon_Reverb
    └── Cave_Reverb
```

---

## PERFORMANCE E OTIMIZAÇÃO

### LOD SYSTEM PARA ÁUDIO
- **LOD 0 (0-50m):** Áudio completo, máxima fidelidade
- **LOD 1 (50-200m):** Redução de frequências altas, menos camadas
- **LOD 2 (200-500m):** Apenas elementos essenciais
- **LOD 3 (500m+):** Culling ou representação minimalista

### MEMORY MANAGEMENT
- **Streaming:** Carregamento dinâmico baseado em proximidade
- **Compression:** Ogg Vorbis para ambiente, PCM para elementos críticos
- **Pooling:** Reutilização de instâncias de áudio similares

### CPU OPTIMIZATION
- **Async Processing:** Decodificação e processamento em threads separadas
- **Culling Inteligente:** Remoção de sons não-audíveis
- **Batch Processing:** Agrupamento de operações similares

---

## FERRAMENTAS DE DESENVOLVIMENTO

### DEBUG TOOLS
- **Audio Visualizer:** Representação visual de todas as camadas ativas
- **Threat Level Monitor:** Display em tempo real do nível de tensão
- **Spatial Audio Debug:** Visualização 3D de fontes sonoras
- **Performance Profiler:** Monitoramento de CPU/Memory usage

### SOUND DESIGNER TOOLS
- **Adaptive Music Tester:** Interface para testar transições musicais
- **Creature Audio Editor:** Ferramenta para ajustar comportamentos sonoros
- **Environment Mixer:** Controle em tempo real de camadas ambientais
- **Emotional State Visualizer:** Gráfico do estado emocional do jogador

---

*Documento criado pelo Audio Agent #16*
*Transpersonal Game Studio — Março 2026*