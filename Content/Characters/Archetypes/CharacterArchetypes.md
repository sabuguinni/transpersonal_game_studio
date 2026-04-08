# CHARACTER ARCHETYPES — JURASSIC SURVIVAL GAME

## CONCEITO VISUAL CENTRAL
Cada personagem é um retrato de sobrevivência. Rostos marcados pelo tempo, roupas adaptadas, corpos moldados pela necessidade. Ninguém aqui chegou ontem — todos carregam a história de como sobreviveram num mundo que não foi feito para humanos.

---

## PROTAGONISTA — DR. [NOME A DEFINIR]

**Arquétipo**: Paleontologista moderno transportado no tempo
**Idade**: 35-45 anos
**Aparência Base**: Caucasiano/Misto, altura média, constituição académica

### Evolução Visual
- **Início**: Roupas modernas rasgadas, sapatos inadequados, pele clara
- **Meio-jogo**: Roupas híbridas (moderno + improvisado), pele bronzeada, músculos definidos
- **Final**: Quase indistinguível de um tribal, mas mantém elementos únicos (óculos reparados, relógio parado)

### Elementos Únicos
- Cicatrizes específicas de dinossauros (garra no braço, mordida na perna)
- Mãos calejadas de construir ferramentas
- Olhar sempre alerta — nunca relaxa completamente
- Postura ligeiramente curvada (carregar peso, esconder-se)

---

## NPCS — ARQUÉTIPOS PRINCIPAIS

### 1. LÍDER TRIBAL
**Conceito**: O alfa que mantém o grupo vivo
- **Físico**: Musculatura desenvolvida, cicatrizes de liderança, postura dominante
- **Idade**: 40-55 anos
- **Roupa**: Peles de predadores, ossos como ornamentos, armas sempre visíveis
- **Variações**: 3 etnias diferentes, ambos os géneros
- **Personalidade Visual**: Olhar direto, nunca baixa a guarda, movimentos calculados

### 2. XAMÃ TRIBAL
**Conceito**: O guardião do conhecimento ancestral
- **Físico**: Mais magro, mãos delicadas, olhos intensos
- **Idade**: 50-70 anos
- **Roupa**: Penas, pinturas corporais, amuletos de osso
- **Variações**: Preferência por etnias indígenas, ambos os géneros
- **Personalidade Visual**: Movimentos fluidos, sempre observando, sorri raramente

### 3. GUERREIRO TRIBAL
**Conceito**: A força bruta do grupo
- **Físico**: Máxima definição muscular, cicatrizes de combate, mãos grandes
- **Idade**: 20-35 anos
- **Roupa**: Mínima — apenas o essencial, armas improvisadas sempre à mão
- **Variações**: Todas as etnias, ligeira preferência masculina (70/30)
- **Personalidade Visual**: Tensão constante, movimentos explosivos, nunca desarmado

### 4. ARTESÃO TRIBAL
**Conceito**: O construtor e reparador
- **Físico**: Mãos calejadas e manchadas, postura ligeiramente curvada
- **Idade**: 30-50 anos
- **Roupa**: Bolsas de ferramentas, dedos protegidos, roupas práticas
- **Variações**: Todas as etnias, ambos os géneros
- **Personalidade Visual**: Sempre mexe em algo, olhar focado, movimentos precisos

### 5. CRIANÇA TRIBAL
**Conceito**: O futuro que precisa de ser protegido
- **Físico**: Pequeno mas resistente, olhos grandes e curiosos
- **Idade**: 8-16 anos
- **Roupa**: Versões pequenas das roupas adultas, sempre um pouco grandes
- **Variações**: Todas as etnias, ambos os géneros
- **Personalidade Visual**: Movimentos rápidos, esconde-se atrás dos adultos, brinca com ossos

### 6. SOBREVIVENTE SOLITÁRIO
**Conceito**: Quem escolheu viver sozinho
- **Físico**: Magro mas resistente, cabelo e barba descuidados
- **Idade**: 25-60 anos (varia muito)
- **Roupa**: Remendos sobre remendos, tudo funcional, nada estético
- **Variações**: Todas as etnias, ligeira preferência masculina (60/40)
- **Personalidade Visual**: Evita contacto visual, movimentos furtivos, sempre pronto para fugir

### 7. COMERCIANTE
**Conceito**: Quem encontrou valor na troca
- **Físico**: Bem alimentado (relativamente), mãos cuidadas
- **Idade**: 30-50 anos
- **Roupa**: Múltiplas bolsas, ornamentos de várias tribos, roupas coloridas
- **Variações**: Todas as etnias, ambos os géneros
- **Personalidade Visual**: Sorriso fácil, gestos expansivos, olhos sempre calculando

---

## SISTEMA DE VARIAÇÃO INDIVIDUAL

### Variações Físicas Obrigatórias
1. **Altura**: ±15% da base do arquétipo
2. **Peso/Músculo**: ±25% da base do arquétipo
3. **Largura facial**: 7 variações por arquétipo
4. **Formato do nariz**: 5 variações por arquétipo
5. **Tamanho dos olhos**: 4 variações por arquétipo
6. **Definição da mandíbula**: 6 variações por arquétipo

### Marcas de Sobrevivência (Procedurais)
- **Cicatrizes**: 12 tipos diferentes, posicionamento aleatório
- **Queimaduras solares**: 4 intensidades
- **Calos e marcas de trabalho**: 8 padrões diferentes
- **Dentes**: 3 níveis de desgaste/danos
- **Cabelo**: Sempre com aspecto de "cortado com faca"

### Roupas e Acessórios
- **Nível de desgaste**: 5 estágios (novo → destruído)
- **Remendos**: 15 tipos diferentes de reparos
- **Acessórios únicos**: Cada NPC tem 1-3 itens únicos
- **Pinturas corporais**: 8 padrões tribais diferentes
- **Ornamentos**: Ossos, dentes, penas — sempre com significado

---

## DIRETRIZES DE IMPLEMENTAÇÃO

### MetaHuman Creator Setup
1. **Base Presets**: 3 por arquétipo (variação étnica)
2. **Morph Targets**: Configurar para máxima variação facial
3. **Skin Textures**: 4 níveis de weathering por preset
4. **Hair Systems**: Sempre "survival style" — nunca perfeito

### Material Considerations
- **Skin Shaders**: Suporte para dirt/weathering layers
- **Clothing Materials**: Sistema de desgaste procedural
- **Scar System**: Decals aplicáveis em runtime
- **Dirt/Grime**: Vertex painting para variação

### Performance Targets
- **LOD System**: 4 níveis para NPCs, 6 para protagonista
- **Texture Resolution**: 2K para protagonista, 1K para NPCs
- **Polygon Budget**: 50K tris para protagonista, 25K para NPCs
- **Draw Calls**: Máximo 3 por personagem (body, hair, accessories)

---

## NEXT STEPS PARA ANIMATION AGENT

O Animation Agent (#10) deve focar em:

1. **Personality Through Movement**: Cada arquétipo tem padrões de movimento únicos
2. **Survival Animations**: Movimentos que mostram adaptação ao ambiente
3. **Interaction Systems**: Como diferentes arquétipos interagem entre si
4. **Combat Readiness**: Todos os NPCs devem parecer capazes de lutar se necessário
5. **Environmental Adaptation**: Animações que respondem ao terreno irregular

Cada personagem deve sentir-se como alguém que viveu neste mundo por anos, não como um modelo 3D a interpretar um papel.

---

*Character Artist Agent — Transpersonal Game Studio*
*CYCLE: PROD_JURASSIC_001*