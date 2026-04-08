# CHARACTER VISUAL STYLE GUIDE
## Transpersonal Game Studio — Jurassic Survival
### Character Artist Agent — Versão 1.0

---

## PRINCÍPIOS VISUAIS FUNDAMENTAIS

### 1. REALISMO EMOCIONAL
Cada personagem deve transmitir uma história através da aparência física:
- **Marcas do tempo**: cicatrizes, rugas de expressão, mãos calejadas
- **Vestígios de vida**: roupas desgastadas, cabelo desalinhado, postura corporal
- **Diversidade genuína**: não há dois rostos iguais, cada NPC tem características únicas

### 2. COERÊNCIA TEMPORAL
Período pré-histórico com realismo moderno:
- **Roupas**: materiais naturais (couro, fibras vegetais, peles)
- **Acessórios**: ossos, pedras, madeira trabalhada
- **Ferramentas**: primitivas mas funcionais
- **Sem anacronismos**: nada de metal trabalhado ou tecidos sintéticos

### 3. SOBREVIVÊNCIA VISUAL
Todos os personagens mostram sinais de vida dura:
- **Pele**: bronzeada pelo sol, com pequenas cicatrizes
- **Músculos**: definição natural de quem trabalha fisicamente
- **Postura**: alerta, sempre pronto para reagir
- **Olhar**: vigilante, conhecedor dos perigos

---

## ARQUÉTIPOS DE PERSONAGENS

### PROTAGONISTA — Dr. [Nome a definir]
**Conceito**: Paleontologista moderno perdido no tempo
- **Idade**: 35-40 anos
- **Físico**: Atlético mas não musculoso, mãos de académico
- **Roupas**: Roupas de exploração modernas (deteriorando-se ao longo do jogo)
- **Expressão**: Determinação misturada com vulnerabilidade
- **Particularidade**: Mantém alguns objectos do presente (relógio, óculos)

### TRIBAIS NATIVOS
**Conceito**: Humanos que sempre viveram nesta época
- **Diversidade étnica**: Representação global autêntica
- **Físico**: Músculos funcionais, cicatrizes de caça/sobrevivência
- **Roupas**: Couro trabalhado, ossos decorativos, pinturas corporais
- **Expressão**: Sabedoria ancestral, conexão com a natureza
- **Hierarquia visual**: Líderes têm mais ornamentos, jovens têm menos marcas

### OUTROS DESLOCADOS TEMPORAIS
**Conceito**: Pessoas de diferentes épocas que também foram transportadas
- **Variedade histórica**: Viking, samurai, explorador vitoriano, etc.
- **Adaptação**: Roupas originais misturadas com materiais locais
- **Estado psicológico**: Diferentes graus de adaptação/loucura
- **Conhecimento**: Cada um traz habilidades da sua época

---

## ESPECIFICAÇÕES TÉCNICAS METAHUMAN

### CONFIGURAÇÃO BASE
- **LOD System**: 3 níveis (Close-up, Medium, Distance)
- **Texture Resolution**: 4K para protagonista, 2K para NPCs principais, 1K para crowd
- **Bone Count**: Otimizado para UE5 (máximo 150 bones por personagem)
- **Animation Compatibility**: Skeleton compatível com Game Animation Sample

### VARIAÇÃO PROCEDURAL
- **Face Morphing**: 15 parâmetros únicos por NPC
- **Skin Variation**: 8 tons de pele base + weathering overlays
- **Hair System**: Groom assets com 5 estilos base + variações
- **Clothing System**: Modular pieces para combinações únicas

### PERFORMANCE TARGETS
- **Protagonista**: 80k triangles (LOD0), 40k (LOD1), 15k (LOD2)
- **NPCs Principais**: 50k triangles (LOD0), 25k (LOD1), 8k (LOD2)
- **Crowd NPCs**: 25k triangles (LOD0), 12k (LOD1), 4k (LOD2)
- **Simultaneous Characters**: Até 50 personagens únicos em cena

---

## PIPELINE DE CRIAÇÃO

### ETAPA 1: CONCEITO
1. Definir personalidade e história do personagem
2. Escolher arquétipo base
3. Definir características físicas únicas
4. Aprovar conceito visual

### ETAPA 2: METAHUMAN CREATION
1. Criar base no MetaHuman Creator
2. Customizar features faciais
3. Ajustar proporções corporais
4. Definir skin materials

### ETAPA 3: CUSTOMIZAÇÃO
1. Criar clothing assets
2. Adicionar props e acessórios
3. Implementar hair/groom variations
4. Aplicar weathering e damage

### ETAPA 4: INTEGRAÇÃO
1. Export para UE5
2. Setup de Animation Blueprint
3. Teste de performance
4. Integração com gameplay systems

---

## DIRETRIZES DE QUALIDADE

### MUST HAVES
- Cada personagem deve ser reconhecível à distância
- Expressões faciais devem funcionar em todas as LODs
- Roupas devem reagir fisicamente ao movimento
- Diversidade étnica e de idade autêntica

### NICE TO HAVES
- Damage system dinâmico (roupas rasgam, cicatrizes aparecem)
- Weather reaction (cabelo molhado, pele brilhante de suor)
- Emotional state reflection (postura muda com mood)

### NEVER DOS
- Personagens genéricos ou "clone-like"
- Anacronismos históricos óbvios
- Over-sexualization de qualquer personagem
- Stereotypes étnicos ou culturais

---

*Character Visual Style Guide v1.0*
*Character Artist Agent — Transpersonal Game Studio*