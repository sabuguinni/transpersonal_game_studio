# Sistema de Música Adaptativa — Design Document

## Conceito Central
A música nunca deve ser notada conscientemente pelo jogador, mas deve constantemente moldar o seu estado emocional. O sistema usa camadas musicais que se misturam dinamicamente baseadas no estado do jogo.

## Estados Musicais Primários

### 1. CALM (Calmo)
- **Emoção**: Falsa segurança, beleza perigosa
- **Instrumentação**: Cordas suaves, flautas de madeira, harpa
- **Ritmo**: Lento, irregular (60-70 BPM)
- **Harmonia**: Maior, mas com dissonâncias subtis
- **Dinâmica**: pp a mp
- **Função**: Fazer o jogador baixar a guarda

### 2. TENSION (Tensão)
- **Emoção**: Algo não está certo, vigilância
- **Instrumentação**: Cordas com tremolo, percussão sutil
- **Ritmo**: Moderado, sincopado (80-90 BPM)
- **Harmonia**: Acordes suspensos, intervalos de quarta
- **Dinâmica**: mp a mf
- **Função**: Aumentar a atenção sem alarmar

### 3. DANGER (Perigo)
- **Emoção**: Ameaça iminente, adrenalina
- **Instrumentação**: Metais baixos, percussão tribal, cordas staccato
- **Ritmo**: Rápido, driving (100-120 BPM)
- **Harmonia**: Menor, clusters dissonantes
- **Dinâmica**: mf a f
- **Função**: Preparar para ação ou fuga

### 4. PANIC (Pânico)
- **Emoção**: Terror, desespero, fuga
- **Instrumentação**: Orquestra completa, percussão intensa
- **Ritmo**: Muito rápido, irregular (120+ BPM)
- **Harmonia**: Atonalismo, clusters extremos
- **Dinâmica**: f a ff
- **Função**: Máxima tensão emocional

### 5. STEALTH (Furtividade)
- **Emoção**: Concentração, respiração suspensa
- **Instrumentação**: Pads sintéticos, sons ambientais processados
- **Ritmo**: Muito lento, quase estático (40-50 BPM)
- **Harmonia**: Drones, intervalos de quinta
- **Dinâmica**: ppp a p
- **Função**: Foco total na sobrevivência

### 6. DISCOVERY (Descoberta)
- **Emoção**: Maravilhamento perigoso, curiosidade
- **Instrumentação**: Celesta, cordas pizzicato, flautas
- **Ritmo**: Livre, rubato
- **Harmonia**: Modalismo, escalas exóticas
- **Dinâmica**: p a mp
- **Função**: Beleza que esconde perigo

### 7. SAFETY (Segurança)
- **Emoção**: Alívio temporário, respiro
- **Instrumentação**: Cordas quentes, madeiras suaves
- **Ritmo**: Lento, estável (50-60 BPM)
- **Harmonia**: Maior puro, sem dissonâncias
- **Dinâmica**: mp
- **Função**: Momento de pausa antes da próxima tensão

## Camadas Musicais (MetaSound Layers)

### Layer 1: FOUNDATION (Sempre ativa)
- **Conteúdo**: Drones, pads ambientais, texturas de fundo
- **Parâmetros**: 
  - `TensionLevel` (0.0-1.0): Controla dissonância
  - `EnvironmentType` (Enum): Afeta timbre e reverb

### Layer 2: RHYTHM (Ativada em Tension+)
- **Conteúdo**: Percussão sutil, pulsos rítmicos
- **Parâmetros**:
  - `RhythmIntensity` (0.0-1.0): Volume e complexidade
  - `BPM` (40-140): Velocidade baseada no estado

### Layer 3: MELODY (Ativada em Danger+)
- **Conteúdo**: Linhas melódicas, motivos temáticos
- **Parâmetros**:
  - `MelodyComplexity` (0.0-1.0): Densidade de notas
  - `EmotionalValence` (-1.0 a 1.0): Maior/menor

### Layer 4: ORCHESTRATION (Ativada em Panic)
- **Conteúdo**: Orquestração completa, tutti
- **Parâmetros**:
  - `OrchestralDensity` (0.0-1.0): Número de instrumentos
  - `DynamicRange` (0.0-1.0): Contraste dinâmico

## Transições Musicais

### Regras de Transição:
1. **CALM → TENSION**: Fade in de elementos rítmicos (3-5 segundos)
2. **TENSION → DANGER**: Crescendo orquestral (2-3 segundos)
3. **DANGER → PANIC**: Transição imediata (0.5-1 segundo)
4. **PANIC → CALM**: Fade out gradual (8-10 segundos)
5. **Qualquer → STEALTH**: Fade to silence, depois fade in stealth (4-6 segundos)

### Parâmetros de Transição:
- `TransitionSpeed`: Velocidade da mudança (0.1-2.0)
- `CrossfadeTime`: Tempo de crossfade entre camadas
- `StateBlend`: Mistura entre estados durante transição

## Implementação MetaSound

### Nós Principais:
1. **State Controller**: Recebe parâmetros do jogo
2. **Layer Mixer**: Mistura as 4 camadas baseado no estado
3. **Transition Manager**: Controla crossfades suaves
4. **Environment Processor**: Aplica reverb/EQ baseado no ambiente
5. **Dynamic Range Controller**: Compressão adaptativa

### Parâmetros de Input:
- `GameState` (Enum): Estado atual do jogo
- `ThreatLevel` (Float): Nível de ameaça (0.0-1.0)
- `EnvironmentType` (Enum): Tipo de ambiente
- `PlayerHeartRate` (Float): Simulação de batimento cardíaco
- `TimeOfDay` (Float): Hora do dia (0.0-24.0)
- `WeatherIntensity` (Float): Intensidade do clima

### Parâmetros de Output:
- `MasterVolume`: Volume final
- `LowPassCutoff`: Filtro para distância/oclusão
- `ReverbSend`: Quantidade de reverb ambiental

## Considerações Técnicas

### Performance:
- Máximo 4 camadas simultâneas
- Streaming de audio assets baseado na proximidade de estados
- LOD system para reduzir qualidade em hardware limitado

### Memória:
- Samples curtos (2-4 segundos) em loop
- Compressão OGG Vorbis para assets não-críticos
- WAV descomprimido apenas para elementos rítmicos precisos

### Latência:
- Transições de estado devem responder em <100ms
- Crossfades não devem causar clicks ou pops
- Sincronização com eventos visuais críticos

## Referências Sonoras
- **The Last of Us**: Tensão constante, beleza perigosa
- **Alien Isolation**: Minimalismo que amplifica o medo
- **Red Dead Redemption 2**: Música que nunca se sobrepõe ao mundo
- **Subnautica**: Ambiente que conta história através do som