# AUDIO ARCHITECTURE DESIGN
## Transpersonal Game Studio — Jurassic Survival Game
### Audio Agent — Production Cycle 001

---

## CONCEITO SONORO CENTRAL

**"O silêncio é suspeito. O som que não existe é muitas vezes mais poderoso do que o som que existe."**

O jogador deve sentir-se constantemente como presa num mundo que não lhe pertence. O design sonoro é construído em camadas de tensão constante, onde cada som ambiente pode mascarar uma ameaça real.

---

## ARQUITECTURA TÉCNICA

### MetaSounds Core System
- **Adaptive Music Engine**: Sistema de música procedural que reage ao estado emocional do jogador
- **Environmental Audio Processor**: Gestão de ambientes sonoros por bioma
- **Dinosaur Behavioral Audio**: Sons únicos para cada espécie com variações individuais
- **Survival Tension Modulator**: Sistema que modula intensidade baseado em perigo percebido

### Audio Submix Structure
```
Master Submix
├── Music Submix
│   ├── Ambient Music
│   ├── Tension Music
│   └── Action Music
├── SFX Submix
│   ├── Dinosaur Sounds
│   ├── Environment Sounds
│   ├── Player Actions
│   └── UI Sounds
├── Dialogue Submix
│   └── Player Internal Monologue
└── Ambient Submix
    ├── Forest Ambience
    ├── Water Ambience
    └── Weather Ambience
```

---

## PALETA EMOCIONAL DE ESTADOS MUSICAIS

### 1. EXPLORAÇÃO CALMA
**Emoção**: Maravilha cautelosa
**Instrumentação**: Cordas graves sustentadas, madeiras suaves, percussão orgânica subtil
**Dinâmica**: pp a mp
**Harmonia**: Modais, evitando resoluções definitivas

### 2. TENSÃO CRESCENTE
**Emoção**: Ansiedade, suspeita
**Instrumentação**: Tremolo em cordas, respirações de madeiras, percussão irregular
**Dinâmica**: mp a mf
**Harmonia**: Dissonâncias controladas, clusters

### 3. PERIGO IMINENTE
**Emoção**: Medo, adrenalina
**Instrumentação**: Staccato agressivo, metais baixos, percussão tribal intensa
**Dinâmica**: f a ff
**Harmonia**: Atonalidade, intervalos aumentados

### 4. CONFRONTO DIRETO
**Emoção**: Terror, luta pela sobrevivência
**Instrumentação**: Orquestra completa, síntese agressiva, samples orgânicos distorcidos
**Dinâmica**: ff a fff
**Harmonia**: Clusters extremos, microtonalidade

### 5. ESCAPE/PERSEGUIÇÃO
**Emoção**: Desespero urgente
**Instrumentação**: Ostinatos rápidos, arpejos ascendentes, percussão acelerada
**Dinâmica**: f constante
**Harmonia**: Sequências cromáticas, modulações abruptas

### 6. ALÍVIO TEMPORÁRIO
**Emoção**: Exaustão, segurança frágil
**Instrumentação**: Cordas divisi, harmonias abertas, reverb longo
**Dinâmica**: p a mp
**Harmonia**: Acordes suspensos, resoluções incompletas

---

## SISTEMA DE DINOSSAUROS — AUDIO DESIGN

### Categorização por Comportamento Sonoro

#### PREDADORES GRANDES (T-Rex, Allosaurus)
- **Respiração**: Grave, pesada, audível a 50+ metros
- **Passos**: Impacto sísmico, frequências sub-bass
- **Vocalizações**: Rugidos profundos, 20-200Hz range
- **Variações**: Cada indivíduo tem pitch signature único

#### HERBÍVOROS GRANDES (Brontosaurus, Triceratops)
- **Digestão**: Sons internos constantes, gorgolejos
- **Movimento**: Arrastar de pés, quebra de vegetação
- **Comunicação**: Chamadas baixas, infrassônicas
- **Mastigação**: Texturas orgânicas, folhagem

#### PREDADORES PEQUENOS (Velociraptors, Compsognathus)
- **Movimento**: Passos rápidos, garras em superfícies
- **Comunicação**: Clicks, assobios, chamadas agudas
- **Caça**: Sons de coordenação, respiração ofegante
- **Inteligência**: Padrões sonoros complexos

#### HERBÍVOROS PEQUENOS (Parasaurolophus, Gallimimus)
- **Alerta**: Chamadas de aviso, batidas de pés
- **Fuga**: Sons de pânico, corrida em grupo
- **Alimentação**: Arrancar plantas, mastigação suave
- **Domesticação**: Gradual mudança de padrões sonoros

---

## AMBIENTES SONOROS POR BIOMA

### FLORESTA DENSA
**Base Layer**: Folhagem constante, vento em árvores
**Wildlife Layer**: Insetos pré-históricos, pequenos mamíferos
**Danger Layer**: Quebra de galhos, movimento pesado distante
**Weather Layer**: Chuva filtrada, trovões abafados

### PLANÍCIES ABERTAS
**Base Layer**: Vento constante, capim alto
**Wildlife Layer**: Chamadas de herbívoros distantes
**Danger Layer**: Rugidos ecoando, tremor de terra
**Weather Layer**: Tempestades visíveis, vento forte

### ÁREAS AQUÁTICAS
**Base Layer**: Movimento de água, ondas suaves
**Wildlife Layer**: Respingos de peixes pré-históricos
**Danger Layer**: Movimento subaquático, predadores aquáticos
**Weather Layer**: Chuva na água, tempestades

### CAVERNAS
**Base Layer**: Eco natural, gotejamento
**Wildlife Layer**: Morcegos, pequenos predadores
**Danger Layer**: Respiração amplificada, passos ecoando
**Weather Layer**: Vento através de túneis

---

## SISTEMA DE MÚSICA ADAPTATIVA

### Triggers de Transição
1. **Proximidade de Predadores**: 0-100m range
2. **Estado de Saúde**: Critical/Low/Normal/High
3. **Hora do Dia**: Dawn/Day/Dusk/Night
4. **Atividade do Jogador**: Stealth/Normal/Running/Combat
5. **Descobertas**: New Area/Resource/Threat

### Layers Musicais Dinâmicos
- **Harmonic Foundation**: Sempre presente, define tom emocional
- **Rhythmic Tension**: Adiciona-se com perigo
- **Melodic Fragments**: Emergem em momentos de descoberta
- **Textural Atmosphere**: Responde ao ambiente físico

---

## IMPLEMENTAÇÃO TÉCNICA

### MetaSounds Graphs Principais
1. **AdaptiveMusicController**: Gere transições e layers
2. **DinosaurAudioBehavior**: IA sonora para cada espécie
3. **EnvironmentalProcessor**: Mistura ambientes por localização
4. **TensionModulator**: Calcula e aplica stress sonoro
5. **ProximityDetector**: Detecta ameaças e ajusta audio

### Performance Targets
- **CPU Usage**: <5% em todas as plataformas
- **Memory Usage**: <200MB para todos os assets de audio
- **Latency**: <50ms para todas as transições críticas
- **Concurrent Sounds**: Máximo 64 sources simultâneas

### Quality Settings por Plataforma
- **PC High**: 48kHz, sem compressão, reverb completo
- **PC Medium**: 44.1kHz, compressão leve, reverb reduzido
- **Console**: 44.1kHz, compressão otimizada, LOD automático
- **Mobile**: 22kHz, compressão agressiva, features reduzidas

---

## PIPELINE DE PRODUÇÃO

### Fase 1: Foundation (Atual)
- Arquitectura MetaSounds
- Sistema de música adaptativa base
- Ambientes sonoros core
- Sounds de dinossauros básicos

### Fase 2: Expansion
- Variações individuais de dinossauros
- Sistema de domesticação audio
- Interações complexas predador/presa
- Weather system integration

### Fase 3: Polish
- Mixing e mastering final
- Otimização de performance
- Localização e acessibilidade
- Testing em todas as plataformas

---

*Audio Architecture Design — Transpersonal Game Studio — Março 2026*