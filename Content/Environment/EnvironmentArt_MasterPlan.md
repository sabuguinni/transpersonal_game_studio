# ENVIRONMENT ART — MASTER PLAN
## Transpersonal Game Studio — Environment Artist #06
## CYCLE_ID: PROD_JURASSIC_002

---

## VISÃO GERAL

Este documento define a abordagem completa de environment art para o mundo jurássico. Cada decisão é baseada na premissa central: **o jogador deve sentir que está num lugar que existiu antes de ele chegar e que vai continuar a existir depois de ele sair**.

---

## PRINCÍPIOS FUNDAMENTAIS

### 1. Narrativa Ambiental Silenciosa
Cada prop, cada cluster de vegetação, cada detalhe conta uma história sem palavras. Uma bota abandonada perto de um rio com ossos de animal próximos = história de um encontro que correu mal.

### 2. Ecossistemas Funcionais
A vegetação não é decoração — é um sistema vivo. Árvores grandes criam sombra onde crescem plantas de sombra. Rios suportam vegetação ribeirinha. Cada planta tem uma razão ecológica para estar onde está.

### 3. Variação Procedural com Controlo Artístico
Usamos PCG e Foliage Tools para densidade e distribuição, mas cada área importante é refinada à mão para garantir que a história ambiental é clara.

### 4. Performance Através de LOD Inteligente
Nanite para geometria complexa, LOD chains para foliage, culling baseado em distância e importância narrativa.

---

## ESTRUTURA POR BIOMA

### FOREST (Floresta Tropical)
**Identidade:** Denso, vivo, imprevisível
**Cor dominante:** Verde tropical — múltiplos tons
**Luz:** Filtrada pela copa das árvores

#### Vegetação Principal:
- **Árvores de copa alta** (15-25m) — criam o tecto da floresta
- **Árvores de sub-bosque** (8-15m) — camada intermédia
- **Arbustos densos** (1-4m) — cobertura ao nível do solo
- **Plantas de sombra** — samambaias, musgos, plantas rasteiras
- **Lianas e trepadeiras** — conectam verticalmente o ecossistema

#### Props Narrativos:
- Troncos caídos com fungos e musgo
- Rochas cobertas de vegetação
- Poças de água da chuva
- Pegadas na lama (dinossauros)
- Ninhos abandonados em árvores altas

### SWAMP (Pântano)
**Identidade:** Húmido, fechado, sem horizonte
**Cor dominante:** Verdes escuros, castanhos, cinzentos
**Água:** Presente em todo o lado

#### Vegetação Principal:
- **Árvores de pântano** — troncos grossos, raízes expostas
- **Vegetação aquática** — nenúfares, juncos, plantas emergentes
- **Musgo abundante** — cobre tudo
- **Plantas carnívoras** — detalhes únicos do bioma

#### Props Narrativos:
- Troncos semi-submersos
- Rochas cobertas de limo
- Bolhas de gás no pântano
- Ossos antigos semi-enterrados
- Nevoeiro permanente (VFX + ambiente)

### SAVANA
**Identidade:** Aberto, exposto, sem esconderijo
**Cor dominante:** Amarelos e castanhos dourados
**Luz:** Forte e directa

#### Vegetação Principal:
- **Árvores isoladas** — acácias espalhadas
- **Erva alta** — move com o vento
- **Arbustos esparsos** — resistentes à seca
- **Plantas suculentas** — adaptadas ao calor

#### Props Narrativos:
- Ossos branqueados pelo sol
- Rochas isoladas (pontos de referência)
- Trilhos de animais na erva
- Buracos de água secos
- Ninhos no chão

### DESERT (Deserto)
**Identidade:** Vasto, seco, sem sombra
**Cor dominante:** Areia — amarelos e ocres
**Vegetação:** Mínima

#### Vegetação Principal:
- **Cactos** — várias espécies e tamanhos
- **Plantas desérticas** — pequenas, resistentes
- **Vegetação de oásis** — apenas perto de água

#### Props Narrativos:
- Ossos completamente secos
- Rochas esculpidas pelo vento
- Dunas com padrões de vento
- Pegadas na areia (temporárias)
- Oásis raros com vegetação densa

### SNOWY ROCKSIDE (Montanhas Nevadas)
**Identidade:** Isolado, silencioso, hostil
**Cor dominante:** Branco, cinzento, azul frio
**Clima:** Frio intenso, neve frequente

#### Vegetação Principal:
- **Coníferas resistentes** — pinheiros adaptados ao frio
- **Arbustos baixos** — resistentes ao vento
- **Plantas alpinas** — pequenas, junto ao solo
- **Líquenes** — nas rochas

#### Props Narrativos:
- Rochas cobertas de gelo
- Troncos caídos preservados pelo frio
- Pegadas na neve (temporárias)
- Cavernas com gelo
- **A gema final** — localização secreta

---

## SISTEMA DE MATERIAIS

### Materiais Base por Bioma:
1. **M_Forest_Ground** — terra húmida, folhas decompostas
2. **M_Forest_Rock** — rochas com musgo e humidade
3. **M_Swamp_Water** — água turva com reflexos
4. **M_Swamp_Mud** — lama com bolhas de gás
5. **M_Savana_Grass** — erva seca dourada
6. **M_Desert_Sand** — areia com padrões de vento
7. **M_Snow_Ground** — neve com pegadas
8. **M_Snow_Ice** — gelo com reflexos

### Sistema de Blending:
- Landscape Layer Blend para transições suaves
- Height-based blending para realismo
- Wetness maps para áreas húmidas
- Wind direction maps para efeitos de erosão

---

## FOLIAGE SYSTEM

### Densidade por Bioma:
- **Forest:** 80-100% cobertura
- **Swamp:** 60-80% cobertura
- **Savana:** 20-40% cobertura
- **Desert:** 5-15% cobertura
- **Snow:** 10-30% cobertura

### LOD Strategy:
- **LOD 0:** 0-50m — máximo detalhe
- **LOD 1:** 50-150m — detalhe médio
- **LOD 2:** 150-500m — baixo detalhe
- **LOD 3:** 500m+ — impostors/billboards

### Culling Rules:
- Foliage pequena: cull a 200m
- Árvores médias: cull a 500m
- Árvores grandes: cull a 1000m
- Props narrativos: nunca cull se visíveis

---

## NARRATIVE PROPS SYSTEM

### Categorias de Props:
1. **Story Props** — contam histórias específicas
2. **Ecosystem Props** — suportam a lógica ecológica
3. **Navigation Props** — ajudam orientação
4. **Atmosphere Props** — criam mood

### Regras de Colocação:
- Props nunca aparecem aleatoriamente
- Cada prop tem contexto ecológico ou narrativo
- Clustering natural — coisas relacionadas aparecem juntas
- Weathering apropriado ao bioma

---

## PERFORMANCE TARGETS

### Poly Budget por Bioma (viewport completo):
- **Forest:** 2M triangles (densidade alta)
- **Swamp:** 1.5M triangles (água + vegetação)
- **Savana:** 800K triangles (espaços abertos)
- **Desert:** 500K triangles (mínima vegetação)
- **Snow:** 1M triangles (rochas + vegetação esparsa)

### Memory Budget:
- Texturas: 2GB total para environment
- Meshes: 1GB total para foliage e props
- Materials: 500MB para todos os materiais base

---

## FERRAMENTAS E WORKFLOW

### Primary Tools:
1. **UE5 Foliage Mode** — distribuição base de vegetação
2. **PCG (Procedural Content Generation)** — patterns e distribuição
3. **Landscape Materials** — terreno e blending
4. **Nanite** — geometria complexa de rochas e props
5. **Lumen** — iluminação global dinâmica

### Secondary Tools:
1. **World Partition** — streaming de conteúdo
2. **Virtual Texturing** — otimização de texturas
3. **Material Layer System** — variações de materiais
4. **Decal System** — detalhes e variação

---

## PRÓXIMOS PASSOS

1. **Criar materiais base** para cada bioma
2. **Definir foliage library** — árvores, arbustos, plantas
3. **Estabelecer prop library** — rochas, troncos, detalhes
4. **Configurar PCG rules** para distribuição procedural
5. **Implementar LOD system** para performance
6. **Criar narrative prop templates** para storytelling

---

*Environment Art Master Plan — Agente #06*
*Criado para CYCLE_ID: PROD_JURASSIC_002*
*Transpersonal Game Studio — Março 2026*