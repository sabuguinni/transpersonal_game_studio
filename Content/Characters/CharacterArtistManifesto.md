# CHARACTER ARTIST MANIFESTO
## Transpersonal Game Studio — Character Artist Agent #09
## Cycle: PROD_JURASSIC_002

---

## FILOSOFIA CENTRAL

**"Aparência é personagem. A forma como alguém veste, a postura que tem, as marcas que o tempo deixou — tudo isto é biografia visual."**

Cada personagem neste mundo Jurássico tem uma história escrita no rosto, na roupa, nas mãos. Não criamos modelos 3D — criamos pessoas que existiram antes do jogador os conhecer.

---

## PRINCÍPIOS FUNDAMENTAIS

### 1. AUTENTICIDADE VISUAL
- Cada rosto conta uma história antes de dizer uma palavra
- Nenhum NPC deve parecer um clone — clones quebram a ilusão de mundo real
- As marcas do tempo, cicatrizes, e imperfeições são biografia visual

### 2. DIVERSIDADE GENUÍNA
- Variação étnica realista e respeitosa
- Diferentes idades, constituições físicas, e expressões
- Cada personagem tem características únicas identificáveis

### 3. CONTEXTUALIZAÇÃO TEMPORAL
- Roupas e equipamentos refletem a situação de sobrevivência
- Sinais de desgaste, reparos improvisados, adaptação ao ambiente
- Nada de materiais ou estilos que não existiriam no contexto

### 4. EXPRESSÃO EMOCIONAL
- Postura corporal que reflete personalidade e estado mental
- Expressões faciais que sugerem história pessoal
- Detalhes que humanizam (cicatrizes, rugas, mãos calejadas)

---

## METODOLOGIA METAHUMAN

### CONFIGURAÇÃO BASE
- Utilização do MetaHuman Creator para base realista
- Customização procedural para variações únicas
- Sistema de tags para organização de arquétipos

### PIPELINE DE CRIAÇÃO
1. **Conceito** → Definir biografia e personalidade
2. **MetaHuman Base** → Selecionar preset próximo
3. **Customização** → Ajustar características únicas
4. **Vestuário** → Adicionar roupas contextuais
5. **Detalhamento** → Cicatrizes, desgaste, personalização
6. **Validação** → Teste em diferentes condições de luz

### SISTEMA DE VARIAÇÃO
- Parâmetros procedurais para características faciais
- Sistema de materiais para variação de pele, cabelo, olhos
- Templates de vestuário adaptáveis por contexto

---

## ARQUÉTIPOS DEFINIDOS

### PROTAGONISTA
**Dr. [Nome] - Paleontologista**
- Idade: 35-40 anos
- Físico: Constituição média, mãos de quem trabalha no campo
- Vestuário: Roupas de expedição científica, adaptadas para sobrevivência
- Personalidade Visual: Determinado mas cauteloso, intelectual prático

### NPCS POTENCIAIS
**Sobreviventes Ocasionais** (se existirem na narrativa)
- Variação de idades, etnias, e backgrounds
- Roupas improvisadas com materiais do ambiente
- Sinais claros de luta pela sobrevivência

---

## ESPECIFICAÇÕES TÉCNICAS

### METAHUMAN SETTINGS
- **LOD System**: 3 níveis para performance otimizada
- **Texture Resolution**: 2K para close-ups, 1K para distância
- **Bone Count**: Otimizado para animação facial e corporal
- **Material Slots**: Organizados por tipo (pele, cabelo, roupas, acessórios)

### NAMING CONVENTION
```
MH_[Archetype]_[Gender]_[Variation]_[LOD]
Exemplo: MH_Protagonist_M_01_LOD0
```

### FILE ORGANIZATION
```
Content/Characters/
├── MetaHuman/
│   ├── Protagonist/
│   ├── Survivors/
│   └── Templates/
├── Materials/
│   ├── Skin/
│   ├── Hair/
│   ├── Clothing/
│   └── Accessories/
├── Blueprints/
│   ├── Character_Base/
│   ├── Character_Protagonist/
│   └── Character_NPC/
└── Data/
    ├── CharacterArchetypes.json
    ├── VariationParameters.json
    └── ClothingCatalog.json
```

---

## DIRETRIZES DE QUALIDADE

### VISUAL FIDELITY
- Consistência com o style guide visual (B3)
- Realismo científico — não fantasia
- Adequação ao contexto Jurássico/Cretáceo

### PERFORMANCE
- Otimização para 60fps PC / 30fps console
- LOD apropriado para diferentes distâncias
- Texture streaming eficiente

### NARRATIVE INTEGRATION
- Alinhamento com a premissa do jogo
- Suporte à sensação de "medo constante"
- Credibilidade como pessoas reais

---

## ENTREGÁVEIS DESTA SESSÃO

1. **Protagonista Completo** - Dr. [Nome] totalmente modelado
2. **Sistema de Variação** - Templates para NPCs futuros
3. **Material Library** - Catálogo de materiais reutilizáveis
4. **Blueprint Framework** - Sistema técnico para characters
5. **Documentation** - Guias para outros agentes

---

*"Cada personagem que criamos tem uma história escrita no rosto, na roupa, nas mãos. Somos os retratistas deste mundo."*

**Character Artist Agent #09**  
**Transpersonal Game Studio — Março 2026**