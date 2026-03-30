# KNOWLEDGE BASE POR AGENTE
## Transpersonal Game Studio — Miguel Martins
## Versão 1.0 · Março 2026

---

## COMO LER ESTE DOCUMENTO

Cada agente tem três tipos de conhecimento a carregar:

**TIPO A — URLs Epic Games (carregar como URL source no Relevance AI)**
Páginas da documentação oficial UE5. Para carregar: Relevance AI → Knowledge → Add URL. Para páginas densas, usa `r.jina.ai/[URL completo]` para converter em texto limpo antes de carregar.

**TIPO B — Documentos internos do estúdio (criar e carregar como ficheiros)**
Documentos que o Miguel e o Hugo criam antes de activar os agentes. Sem estes, os agentes criativos produzem conteúdo genérico. Formato: Markdown ou Word, carregado como ficheiro na KB do agente.

**TIPO C — Documentos gerados por outros agentes (carregar via Make.com)**
Outputs aprovados de agentes que alimentam outros agentes. O Make.com automatiza este fluxo: output aprovado → Google Drive → actualização da KB via API Relevance AI.

**PRIORIDADE DE CARREGAMENTO:**
- 🔴 Crítico — o agente não funciona coerentemente sem isto
- 🟡 Alto — o agente funciona mas com qualidade reduzida
- 🟢 Complementar — aumenta significativamente a qualidade em produção

---

## NOTA SOBRE A BIBLE DO JOGO

A Bible do Jogo é o documento mais importante de toda a arquitectura. É gerada pelo Narrative Agent na primeira sessão de trabalho com base no Conceito do Jogo do Miguel. Após aprovação pelo Miguel, é distribuída automaticamente via Make.com a todos os agentes criativos. Até existir, nenhum agente criativo deve estar em produção.

---
---

# AGENTE #01 — STUDIO DIRECTOR

## TIPO A — URLs Epic Games

| # | Título | URL |
|---|--------|-----|
| 1 | Remote Control API Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/remote-control-api-for-unreal-engine |
| 2 | Remote Control Web Interface | https://dev.epicgames.com/documentation/en-us/unreal-engine/remote-control-web-interface-for-unreal-engine |
| 3 | Remote Control Presets | https://dev.epicgames.com/documentation/en-us/unreal-engine/remote-control-presets-in-unreal-engine |
| 4 | Python Scripting in UE5 | https://dev.epicgames.com/documentation/en-us/unreal-engine/scripting-the-unreal-editor-using-python |
| 5 | Unreal Automation Tool | https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-automation-tool |
| 6 | Editor Scripting Utilities | https://dev.epicgames.com/documentation/en-us/unreal-engine/editor-scripting-utilities-in-unreal-engine |

## TIPO B — Documentos Internos 🔴 CRÍTICO

**B1 — Conceito do Jogo (criar antes de activar qualquer agente)**
Documento de 1-2 páginas onde o Miguel responde:
- Que jogo quero criar? (género, escala, referências)
- Que sensação quero dar ao jogador?
- Quais as 3 coisas únicas do meu jogo que nenhum outro tem?
- O que o jogador nunca deve sentir no meu jogo?
- Qual é o personagem principal e qual é a sua história em 3 frases?

**B2 — Lista de Capacidades dos Agentes**
Documento de referência com o nome, função e tipo de output de cada um dos 18 agentes especializados. O Studio Director usa isto para saber a quem delegar cada tarefa.

## TIPO C — Documentos de Outros Agentes 🟡 Alto

- Bible do Jogo (gerada pelo Agente #12 — Narrative)
- Arquitectura Técnica do Projecto (gerada pelo Agente #02 — Engine Architect)

---
---

# AGENTE #02 — ENGINE ARCHITECT

## TIPO A — URLs Epic Games

| # | Título | URL | Prioridade |
|---|--------|-----|------------|
| 1 | World Partition Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/world-partition-in-unreal-engine | 🔴 |
| 2 | World Partition — Data Layers | https://dev.epicgames.com/documentation/en-us/unreal-engine/world-partition---data-layers-in-unreal-engine | 🔴 |
| 3 | World Partition — Hierarchical Level of Detail | https://dev.epicgames.com/documentation/en-us/unreal-engine/world-partition---hierarchical-level-of-detail-in-unreal-engine | 🟡 |
| 4 | Nanite Virtualized Geometry | https://dev.epicgames.com/documentation/en-us/unreal-engine/nanite-virtualized-geometry-in-unreal-engine | 🔴 |
| 5 | Nanite Assemblies | https://dev.epicgames.com/documentation/en-us/unreal-engine/nanite-assemblies | 🟡 |
| 6 | Nanite Foliage | https://dev.epicgames.com/documentation/en-us/unreal-engine/nanite-foliage | 🟡 |
| 7 | Lumen GI Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-global-illumination-and-reflections-in-unreal-engine | 🔴 |
| 8 | Lumen Technical Details | https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-technical-details-in-unreal-engine | 🔴 |
| 9 | Virtual Shadow Maps | https://dev.epicgames.com/documentation/en-us/unreal-engine/virtual-shadow-maps-in-unreal-engine | 🔴 |
| 10 | C++ Coding Standards | https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine | 🔴 |
| 11 | Programming with C++ | https://dev.epicgames.com/documentation/en-us/unreal-engine/programming-with-cplusplus-in-unreal-engine | 🟡 |
| 12 | Gameplay Framework Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-framework-in-unreal-engine | 🟡 |
| 13 | Asset Management | https://dev.epicgames.com/documentation/en-us/unreal-engine/asset-management-in-unreal-engine | 🟡 |
| 14 | One File Per Actor | https://dev.epicgames.com/documentation/en-us/unreal-engine/one-file-per-actor-in-unreal-engine | 🟡 |

## TIPO B — Documentos Internos 🔴 CRÍTICO

**B1 — Conceito do Jogo** (recebido do Studio Director)

**B2 — Template de Arquitectura Técnica**
O Engine Architect preenche este template na primeira sessão e distribui a todos os agentes:
- Motor e versão UE5 adoptada
- Sistemas obrigatórios activados (Lumen, Nanite, World Partition, Mass AI, PCG)
- Dimensão do mundo (km²) e target de densidade de NPCs
- Targets de performance por plataforma
- Convenções de nomenclatura de assets (obrigatórias para todos os agentes)
- Estrutura de pastas do projecto
- Pipeline de build e integração
- Regras de interoperabilidade entre sistemas

## TIPO C — Documentos de Outros Agentes

- Conceito do Jogo (Agente #01 — Studio Director) 🔴

---
---

# AGENTE #03 — CORE SYSTEMS PROGRAMMER

## TIPO A — URLs Epic Games

| # | Título | URL | Prioridade |
|---|--------|-----|------------|
| 1 | Physics in UE5 Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-in-unreal-engine | 🔴 |
| 2 | Chaos Physics Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-physics-overview | 🔴 |
| 3 | Chaos Destruction Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-destruction-overview | 🟡 |
| 4 | Chaos Vehicles Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-vehicles-overview | 🟡 |
| 5 | Chaos Modular Vehicles | https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-modular-vehicles-overview | 🟢 |
| 6 | Chaos Fields User Guide | https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-fields-user-guide-in-unreal-engine | 🟡 |
| 7 | Chaos Cloth Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-cloth-overview | 🟡 |
| 8 | Collision Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/collision-in-unreal-engine | 🔴 |
| 9 | Character Movement Component | https://dev.epicgames.com/documentation/en-us/unreal-engine/character-movement-component-in-unreal-engine | 🔴 |
| 10 | Physics Settings | https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-settings-in-the-unreal-engine-project-settings | 🟡 |
| 11 | C++ Coding Standards | https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine | 🔴 |
| 12 | Networking Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/networking-and-multiplayer-in-unreal-engine | 🟢 |

## TIPO B — Documentos Internos

**B1 — Arquitectura Técnica do Projecto** (recebida do Agente #02) 🔴
**B2 — Conceito do Jogo** (recebido do Studio Director) 🟡

## TIPO C — Documentos de Outros Agentes

- Arquitectura Técnica completa (Agente #02) 🔴

---
---

# AGENTE #04 — PERFORMANCE & OPTIMIZER

## TIPO A — URLs Epic Games

| # | Título | URL | Prioridade |
|---|--------|-----|------------|
| 1 | Testing and Optimizing — Hub | https://dev.epicgames.com/documentation/en-us/unreal-engine/testing-and-optimizing-your-content | 🔴 |
| 2 | Unreal Insights Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-insights-in-unreal-engine | 🔴 |
| 3 | Stat Commands | https://dev.epicgames.com/documentation/en-us/unreal-engine/stat-commands-in-unreal-engine | 🔴 |
| 4 | GPU Profiling | https://dev.epicgames.com/documentation/en-us/unreal-engine/gpu-profiling-in-unreal-engine | 🔴 |
| 5 | CPU Profiling | https://dev.epicgames.com/documentation/en-us/unreal-engine/cpu-profiling-in-unreal-engine | 🔴 |
| 6 | Level of Detail (LOD) | https://dev.epicgames.com/documentation/en-us/unreal-engine/creating-and-using-lods-in-unreal-engine | 🔴 |
| 7 | Scalability Settings | https://dev.epicgames.com/documentation/en-us/unreal-engine/scalability-settings-in-unreal-engine | 🔴 |
| 8 | Memory Profiling | https://dev.epicgames.com/documentation/en-us/unreal-engine/memory-management-and-profiling-in-unreal-engine | 🟡 |
| 9 | Occlusion Culling | https://dev.epicgames.com/documentation/en-us/unreal-engine/occlusion-culling-in-unreal-engine | 🟡 |
| 10 | Texture Streaming | https://dev.epicgames.com/documentation/en-us/unreal-engine/texture-streaming-in-unreal-engine | 🟡 |
| 11 | Chaos Visual Debugger | https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-visual-debugger-user-guide | 🟢 |
| 12 | Nanite Virtualized Geometry | https://dev.epicgames.com/documentation/en-us/unreal-engine/nanite-virtualized-geometry-in-unreal-engine | 🟡 |
| 13 | Lumen Technical Details | https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-technical-details-in-unreal-engine | 🟡 |

## TIPO B — Documentos Internos

**B1 — Arquitectura Técnica do Projecto com Targets de Performance** 🔴
Documento do Engine Architect com os valores exactos de budget por sistema.

**B2 — Template de Relatório de Performance**
Formato fixo que o Optimizer usa em todos os relatórios — garante parseabilidade pelo Make.com.

## TIPO C — Documentos de Outros Agentes

- Arquitectura Técnica (Agente #02) 🔴
- Relatórios de QA (Agente #18 — ciclo contínuo) 🟡

---
---

# AGENTE #05 — PROCEDURAL WORLD GENERATOR

## TIPO A — URLs Epic Games

| # | Título | URL | Prioridade |
|---|--------|-----|------------|
| 1 | PCG Framework Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/procedural-content-generation-overview | 🔴 |
| 2 | PCG Development Guides | https://dev.epicgames.com/documentation/en-us/unreal-engine/pcg-development-guides | 🔴 |
| 3 | PCG Generation Modes | https://dev.epicgames.com/documentation/en-us/unreal-engine/using-pcg-generation-modes-in-unreal-engine | 🔴 |
| 4 | PCG with World Partition | https://dev.epicgames.com/documentation/en-us/unreal-engine/using-pcg-with-world-partition-in-unreal-engine | 🔴 |
| 5 | PCG with GPU Processing | https://dev.epicgames.com/documentation/en-us/unreal-engine/using-pcg-with-gpu-processing-in-unreal-engine | 🟡 |
| 6 | PCG Biome Core Plugin Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/procedural-content-generation-pcg-biome-core-and-sample-plugins-overview-guide-in-unreal-engine | 🟡 |
| 7 | PCG Biome Core Plugin Reference | https://dev.epicgames.com/documentation/en-us/unreal-engine/procedural-content-generation-pcg-biome-core-and-sample-plugins-reference-guide-in-unreal-engine | 🟢 |
| 8 | Landscape Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/landscape-overview | 🔴 |
| 9 | Landscape Technical Guide | https://dev.epicgames.com/documentation/en-us/unreal-engine/landscape-technical-guide | 🔴 |
| 10 | Landscape Collision Guide | https://dev.epicgames.com/documentation/en-us/unreal-engine/landscape-collision-guide | 🟡 |
| 11 | Water System Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/water-system-overview-in-unreal-engine | 🟡 |
| 12 | World Partition Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/world-partition-in-unreal-engine | 🔴 |
| 13 | Python Scripting in UE5 | https://dev.epicgames.com/documentation/en-us/unreal-engine/scripting-the-unreal-editor-using-python | 🔴 |
| 14 | Procedural Vegetation Editor | https://dev.epicgames.com/documentation/en-us/unreal-engine/procedural-vegetation-editor-pve-in-unreal-engine | 🟡 |

## TIPO B — Documentos Internos 🔴 CRÍTICO

**B1 — Bible do Jogo** (gerada pelo Agente #12)
O World Generator usa a Bible para o período histórico, a cultura e o clima do mundo.

**B2 — Guia Geográfico do Mundo**
Documento criado pelo Miguel e pelo Hugo com as características geográficas intencionais:
- Dimensões do mundo em km²
- Biomas presentes e suas proporções
- Clima predominante e variações regionais
- Nomes de regiões, rios, montanhas
- Localizações de cidades principais
- Intenção narrativa de cada região

## TIPO C — Documentos de Outros Agentes

- Arquitectura Técnica (Agente #02) 🟡
- Bible do Jogo aprovada (Agente #12) 🔴

---
---

# AGENTE #06 — ENVIRONMENT ARTIST

## TIPO A — URLs Epic Games

| # | Título | URL | Prioridade |
|---|--------|-----|------------|
| 1 | Foliage Mode in UE5 | https://dev.epicgames.com/documentation/en-us/unreal-engine/foliage-mode-in-unreal-engine | 🔴 |
| 2 | Procedural Foliage Tool | https://dev.epicgames.com/documentation/en-us/unreal-engine/procedural-foliage-tool-in-unreal-engine | 🔴 |
| 3 | Procedural Vegetation Editor | https://dev.epicgames.com/documentation/en-us/unreal-engine/procedural-vegetation-editor-pve-in-unreal-engine | 🟡 |
| 4 | Landscape Materials | https://dev.epicgames.com/documentation/en-us/unreal-engine/landscape-materials | 🔴 |
| 5 | Materials Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-materials | 🔴 |
| 6 | Material Editor Guide | https://dev.epicgames.com/documentation/en-us/unreal-engine/material-editor-user-guide | 🟡 |
| 7 | Physically Based Materials | https://dev.epicgames.com/documentation/en-us/unreal-engine/physically-based-materials-in-unreal-engine | 🟡 |
| 8 | Substrate Materials | https://dev.epicgames.com/documentation/en-us/unreal-engine/substrate-materials-in-unreal-engine | 🟡 |
| 9 | Static Mesh Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/static-meshes | 🟡 |
| 10 | Decal Materials | https://dev.epicgames.com/documentation/en-us/unreal-engine/decal-materials-in-unreal-engine | 🟡 |
| 11 | Nanite Virtualized Geometry | https://dev.epicgames.com/documentation/en-us/unreal-engine/nanite-virtualized-geometry-in-unreal-engine | 🟡 |
| 12 | Level Design Best Practices | https://dev.epicgames.com/documentation/en-us/unreal-engine/level-design-best-practices | 🟢 |

## TIPO B — Documentos Internos 🔴 CRÍTICO

**B1 — Style Guide Visual**
Documento criado pelo Miguel com aprovação do Hugo — define:
- Paleta de cores por bioma (com valores hexadecimais ou referências de cor)
- Referências visuais (imagens ou descrições de jogos, filmes, fotografias)
- Proibições estéticas (o que nunca deve aparecer no jogo)
- Nível de realismo pretendido (hiperrealista, estilizado, etc.)
- Densidade de detalhe por tipo de zona (cidade densa, floresta, deserto)

**B2 — Bible do Jogo** (para contexto narrativo ambiental) 🟡

## TIPO C — Documentos de Outros Agentes

- Mapa de biomas e narrativa geográfica (Agente #05) 🔴
- Style Guide visual aprovado (documento interno) 🔴
- Ficha histórica de cada zona (Agente #07 — Architecture) 🟡

---
---

# AGENTE #07 — ARCHITECTURE & INTERIOR AGENT

## TIPO A — URLs Epic Games

| # | Título | URL | Prioridade |
|---|--------|-----|------------|
| 1 | PCG Framework Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/procedural-content-generation-overview | 🔴 |
| 2 | PCG Development Guides | https://dev.epicgames.com/documentation/en-us/unreal-engine/pcg-development-guides | 🔴 |
| 3 | PCG Generation Modes | https://dev.epicgames.com/documentation/en-us/unreal-engine/using-pcg-generation-modes-in-unreal-engine | 🟡 |
| 4 | Static Mesh Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/static-meshes | 🔴 |
| 5 | Level Design Best Practices | https://dev.epicgames.com/documentation/en-us/unreal-engine/level-design-best-practices | 🔴 |
| 6 | Placing Actors in UE5 | https://dev.epicgames.com/documentation/en-us/unreal-engine/placing-actors-in-unreal-engine | 🟡 |
| 7 | Collision Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/collision-in-unreal-engine | 🟡 |
| 8 | Blueprint Scripting Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/blueprints-visual-scripting-in-unreal-engine | 🟡 |
| 9 | Materials Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-materials | 🟡 |
| 10 | Nanite Virtualized Geometry | https://dev.epicgames.com/documentation/en-us/unreal-engine/nanite-virtualized-geometry-in-unreal-engine | 🟡 |

## TIPO B — Documentos Internos 🔴 CRÍTICO

**B1 — Bible do Jogo** (período histórico, cultura, economia do mundo)
O Architecture Agent usa a Bible para definir materiais de construção, estilos arquitectónicos e contexto histórico de cada estrutura.

**B2 — Guia de Tipologia de Edifícios**
Documento criado antes de activar este agente:
- Tipos de estruturas que existem no mundo (residencial, comercial, militar, religioso, etc.)
- Materiais disponíveis no período histórico
- Nível de avanço tecnológico da civilização
- Variações regionais de arquitectura
- Exemplos visuais de referência por tipo

## TIPO C — Documentos de Outros Agentes

- Narrativa geográfica e mapa de biomas (Agente #05) 🔴
- Style Guide visual (documento interno) 🟡
- Bible do Jogo (Agente #12) 🔴
- Perfis de NPCs por zona (Agente #09) 🟡 — para poblar interiores coerentemente

---
---

# AGENTE #08 — LIGHTING & ATMOSPHERE AGENT

## TIPO A — URLs Epic Games

| # | Título | URL | Prioridade |
|---|--------|-----|------------|
| 1 | Lumen GI Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-global-illumination-and-reflections-in-unreal-engine | 🔴 |
| 2 | Lumen Technical Details | https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-technical-details-in-unreal-engine | 🔴 |
| 3 | Sky Atmosphere Component | https://dev.epicgames.com/documentation/en-us/unreal-engine/sky-atmosphere-component-in-unreal-engine | 🔴 |
| 4 | Sky Atmosphere Properties | https://dev.epicgames.com/documentation/en-us/unreal-engine/sky-atmosphere-component-properties-in-unreal-engine | 🔴 |
| 5 | Environmental Lighting Hub (Fog, Clouds, Sky) | https://dev.epicgames.com/documentation/en-us/unreal-engine/environmental-light-with-fog-clouds-sky-and-atmosphere-in-unreal-engine | 🔴 |
| 6 | Volumetric Clouds | https://dev.epicgames.com/documentation/en-us/unreal-engine/volumetric-cloud-component-in-unreal-engine | 🔴 |
| 7 | Exponential Height Fog | https://dev.epicgames.com/documentation/en-us/unreal-engine/exponential-height-fog-user-guide | 🔴 |
| 8 | Virtual Shadow Maps | https://dev.epicgames.com/documentation/en-us/unreal-engine/virtual-shadow-maps-in-unreal-engine | 🔴 |
| 9 | Shadowing Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/shadowing-in-unreal-engine | 🟡 |
| 10 | Post Process Effects | https://dev.epicgames.com/documentation/en-us/unreal-engine/post-process-effects | 🟡 |
| 11 | Light Types and Mobility | https://dev.epicgames.com/documentation/en-us/unreal-engine/light-types-and-their-mobility | 🟡 |
| 12 | Lighting the Environment | https://dev.epicgames.com/documentation/en-us/unreal-engine/lighting-the-environment | 🟢 |

## TIPO B — Documentos Internos 🔴 CRÍTICO

**B1 — Mood Board Visual** (criado pelo Miguel)
20-30 referências visuais com anotações sobre o que o Miguel quer replicar em termos de luz, atmosfera e paleta de cor.

**B2 — Paleta Emocional de Estados de Iluminação**
Documento que mapeia estados narrativos a estados de iluminação:
- Estado: exploração calma → temperatura de cor, intensidade, condição meteorológica
- Estado: tensão → alterações específicas
- Estado: perigo → alterações específicas
- Estado: vitória/resolução → alterações específicas
- Etc.

**B3 — Referência da Apresentação RDR2 Sky Rendering (SIGGRAPH 2019)**
URL gratuito: https://advances.realtimerendering.com/s2019/index.htm
Download os slides do talk sobre sky rendering da Rockstar. Carregar como PDF na KB.

## TIPO C — Documentos de Outros Agentes

- Narrativa ambiental por zona (Agente #06) 🔴
- Intenções emocionais por zona (Agente #12 — Narrative) 🟡
- Style Guide visual (documento interno) 🔴

---
---

# AGENTE #09 — NPC BEHAVIOR AGENT

## TIPO A — URLs Epic Games

| # | Título | URL | Prioridade |
|---|--------|-----|------------|
| 1 | AI Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/artificial-intelligence-in-unreal-engine | 🔴 |
| 2 | Behavior Trees Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/behavior-trees-in-unreal-engine | 🔴 |
| 3 | Behavior Tree Quick Start Guide | https://dev.epicgames.com/documentation/en-us/unreal-engine/behavior-tree-in-unreal-engine---quick-start-guide | 🔴 |
| 4 | Behavior Tree User Guide | https://dev.epicgames.com/documentation/en-us/unreal-engine/behavior-tree-in-unreal-engine---user-guide | 🔴 |
| 5 | BT Node Reference: Tasks | https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-behavior-tree-node-reference-tasks | 🔴 |
| 6 | BT Node Reference: Decorators | https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-behavior-tree-node-reference-decorators | 🔴 |
| 7 | BT Node Reference: Services | https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-behavior-tree-node-reference-services | 🟡 |
| 8 | Blackboard in UE5 | https://dev.epicgames.com/documentation/en-us/unreal-engine/blackboard-in-unreal-engine | 🔴 |
| 9 | AI Perception | https://dev.epicgames.com/documentation/en-us/unreal-engine/ai-perception-in-unreal-engine | 🔴 |
| 10 | EQS Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/environment-query-system-overview-in-unreal-engine | 🔴 |
| 11 | EQS Quick Start | https://dev.epicgames.com/documentation/en-us/unreal-engine/environment-query-system-quick-start | 🟡 |
| 12 | EQS User Guide | https://dev.epicgames.com/documentation/en-us/unreal-engine/environment-query-system-user-guide | 🟡 |
| 13 | Mass Entity Framework | https://dev.epicgames.com/documentation/en-us/unreal-engine/mass-entity-in-unreal-engine | 🔴 |
| 14 | State Tree Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/state-tree-overview-in-unreal-engine | 🟡 |
| 15 | Navigation System | https://dev.epicgames.com/documentation/en-us/unreal-engine/navigation-system-in-unreal-engine | 🔴 |
| 16 | AI Debugging | https://dev.epicgames.com/documentation/en-us/unreal-engine/ai-debugging-in-unreal-engine | 🔴 |
| 17 | Visual Logger | https://dev.epicgames.com/documentation/en-us/unreal-engine/visual-logger-in-unreal-engine | 🟡 |

## TIPO B — Documentos Internos 🔴 CRÍTICO

**B1 — Bible do Jogo** (perfis culturais, hierarquias sociais, economia do mundo)
O NPC Behavior Agent precisa de saber como é a sociedade para criar comportamentos plausíveis.

**B2 — Catálogo de Arquétipos Sociais**
Documento criado antes da produção com os tipos de NPC que existem no mundo:
- Nome do arquétipo (ex: pescador, guarda, comerciante, nobre)
- Posição na hierarquia social
- Necessidades básicas e medos
- Rotina tipo por hora do dia
- Reacções tipo a eventos comuns

**B3 — Referência: The Reality of RDR2's AI**
Artigo técnico gratuito sobre o sistema de IA do RDR2:
https://medium.com/the-sound-of-ai/the-reality-of-red-dead-redemption-2s-ai-part-1-c276e9da2763
Carregar as partes 1 e 2 como texto na KB.

## TIPO C — Documentos de Outros Agentes

- Bible do Jogo (Agente #12) 🔴
- Fichas históricas de zonas (Agente #07 — Architecture) 🔴
- Eventos narrativos activos por zona (Agente #12 — Narrative) 🟡
- Perfis de personagens principais (Agente #12) 🔴

---
---

# AGENTE #10 — COMBAT & ENEMY AI AGENT

## TIPO A — URLs Epic Games

| # | Título | URL | Prioridade |
|---|--------|-----|------------|
| 1 | Behavior Trees Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/behavior-trees-in-unreal-engine | 🔴 |
| 2 | Behavior Tree User Guide | https://dev.epicgames.com/documentation/en-us/unreal-engine/behavior-tree-in-unreal-engine---user-guide | 🔴 |
| 3 | BT Node Reference: Tasks | https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-behavior-tree-node-reference-tasks | 🔴 |
| 4 | BT Node Reference: Decorators | https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-behavior-tree-node-reference-decorators | 🔴 |
| 5 | EQS Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/environment-query-system-overview-in-unreal-engine | 🔴 |
| 6 | EQS User Guide | https://dev.epicgames.com/documentation/en-us/unreal-engine/environment-query-system-user-guide | 🔴 |
| 7 | EQS Node Reference | https://dev.epicgames.com/documentation/en-us/unreal-engine/environment-query-system-node-reference | 🔴 |
| 8 | EQS Quick Start | https://dev.epicgames.com/documentation/en-us/unreal-engine/environment-query-system-quick-start | 🟡 |
| 9 | AI Perception | https://dev.epicgames.com/documentation/en-us/unreal-engine/ai-perception-in-unreal-engine | 🔴 |
| 10 | AI Debugging | https://dev.epicgames.com/documentation/en-us/unreal-engine/ai-debugging-in-unreal-engine | 🔴 |
| 11 | Gameplay Ability System | https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-ability-system-for-unreal-engine | 🟡 |
| 12 | Gameplay Tags | https://dev.epicgames.com/documentation/en-us/unreal-engine/using-gameplay-tags-in-unreal-engine-5 | 🟡 |
| 13 | Navigation System | https://dev.epicgames.com/documentation/en-us/unreal-engine/navigation-system-in-unreal-engine | 🔴 |
| 14 | State Tree Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/state-tree-overview-in-unreal-engine | 🟡 |

## TIPO B — Documentos Internos 🔴 CRÍTICO

**B1 — Bible do Jogo** (tom narrativo, facções em conflito, natureza da violência no mundo)
Define o contexto em que o combate existe — é um mundo brutal? Elegante? Táctico?

**B2 — Catálogo de Arquétipos de Inimigos**
Documento criado antes da produção:
- Nome e origem de cada facção inimiga
- Motivação da facção (nunca "são maus" — qual é a sua lógica interna?)
- Arquétipos dentro da facção (grunt, táctico, elite, boss)
- Equipamento e tácticas características
- Comportamentos distintivos que tornam cada facção reconhecível

## TIPO C — Documentos de Outros Agentes

- Bible do Jogo (Agente #12) 🔴
- Arquitectura técnica de sistemas de combate (Agente #02) 🟡
- Contexto de missões com combate (Agente #13 — Quest Designer) 🔴

---
---

# AGENTE #11 — CROWD & TRAFFIC SIMULATION

## TIPO A — URLs Epic Games

| # | Título | URL | Prioridade |
|---|--------|-----|------------|
| 1 | Mass Entity Framework | https://dev.epicgames.com/documentation/en-us/unreal-engine/mass-entity-in-unreal-engine | 🔴 |
| 2 | Mass Avoidance Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/mass-avoidance-overview | 🔴 |
| 3 | Zone Graph Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/zone-graph-overview-in-unreal-engine | 🔴 |
| 4 | Navigation System | https://dev.epicgames.com/documentation/en-us/unreal-engine/navigation-system-in-unreal-engine | 🔴 |
| 5 | Chaos Vehicles Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-vehicles-overview | 🟡 |
| 6 | AI Debugging | https://dev.epicgames.com/documentation/en-us/unreal-engine/ai-debugging-in-unreal-engine | 🟡 |
| 7 | State Tree Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/state-tree-overview-in-unreal-engine | 🟡 |

## TIPO B — Documentos Internos 🔴 CRÍTICO

**B1 — Bible do Jogo** (economia do mundo, vida urbana, classes sociais)
Define como as pessoas se movem numa cidade — os ricos e os pobres não usam os mesmos espaços.

**B2 — Mapa Urbano com Anotações de Fluxo**
Documento do World Generator com anotações sobre:
- Ruas principais vs secundárias
- Zonas de alta e baixa densidade
- Locais de convergência (mercados, fontes, praças)
- Horas de pico por tipo de zona

## TIPO C — Documentos de Outros Agentes

- Mapa urbano e rede de estradas (Agente #05) 🔴
- Perfis de NPCs por zona (Agente #09) 🔴
- Bible do Jogo (Agente #12) 🟡

---
---

# AGENTE #12 — NARRATIVE & DIALOGUE AGENT

## TIPO A — URLs Epic Games

| # | Título | URL | Prioridade |
|---|--------|-----|------------|
| 1 | Dialogue System Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/dialogue-system-in-unreal-engine | 🔴 |
| 2 | Dialogue Voices and Waves | https://dev.epicgames.com/documentation/en-us/unreal-engine/dialogue-voices-and-waves | 🟡 |
| 3 | Level Sequencer Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/cinematics-and-movie-making-in-unreal-engine | 🟡 |
| 4 | Blueprint Scripting Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/blueprints-visual-scripting-in-unreal-engine | 🟡 |
| 5 | Gameplay Tags | https://dev.epicgames.com/documentation/en-us/unreal-engine/using-gameplay-tags-in-unreal-engine-5 | 🟡 |
| 6 | Save Game System | https://dev.epicgames.com/documentation/en-us/unreal-engine/saving-and-loading-your-game-in-unreal-engine | 🟡 |

**Nota:** o Narrative Agent tem menos URLs técnicos porque o seu trabalho é fundamentalmente criativo. O conhecimento crítico para este agente é todo de Tipo B.

## TIPO B — Documentos Internos 🔴 CRÍTICO

**B1 — Conceito do Jogo** (o documento mais importante para este agente)
O Narrative Agent usa o Conceito do Jogo para construir a Bible. Sem ele, a Bible não tem fundação.

**B2 — Premissa Narrativa**
Documento de 1 página criado pelo Miguel antes de activar este agente:
- Quem é o protagonista em uma frase
- Qual é a ferida do protagonista (o que o define antes do jogo começar)
- Qual é o conflito central (interno e externo)
- Como termina a história — mesmo que provisório
- Qual é o tema — o que o jogo diz sobre o ser humano

**B3 — Referências Narrativas**
Lista de 5-10 jogos, filmes ou livros que o Miguel considera referências de narrativa para o seu jogo, com anotações sobre o que admira em cada um.

**B4 — Regras do Mundo Narrativo**
O que existe e o que não existe no mundo do jogo:
- Existe magia? De que tipo?
- Existe tecnologia? Que nível?
- Qual é o nível de violência aceitável?
- Que temas são proibidos?
- Que temas são centrais?

## TIPO C — Documentos de Outros Agentes

- Narrativa geográfica (Agente #05) 🟡 — contexto do mundo físico
- Fichas históricas de estruturas (Agente #07) 🟡 — contexto cultural

---
---

# AGENTE #13 — QUEST & MISSION DESIGNER

## TIPO A — URLs Epic Games

| # | Título | URL | Prioridade |
|---|--------|-----|------------|
| 1 | Blueprint Scripting Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/blueprints-visual-scripting-in-unreal-engine | 🔴 |
| 2 | Gameplay Framework | https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-framework-in-unreal-engine | 🔴 |
| 3 | Gameplay Tags | https://dev.epicgames.com/documentation/en-us/unreal-engine/using-gameplay-tags-in-unreal-engine-5 | 🔴 |
| 4 | Gameplay Ability System | https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-ability-system-for-unreal-engine | 🟡 |
| 5 | Level Sequencer Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/cinematics-and-movie-making-in-unreal-engine | 🟡 |
| 6 | Save Game System | https://dev.epicgames.com/documentation/en-us/unreal-engine/saving-and-loading-your-game-in-unreal-engine | 🟡 |
| 7 | Trigger Volumes | https://dev.epicgames.com/documentation/en-us/unreal-engine/trigger-volumes-in-unreal-engine | 🟡 |
| 8 | AI Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/artificial-intelligence-in-unreal-engine | 🟡 |

## TIPO B — Documentos Internos 🔴 CRÍTICO

**B1 — Bible do Jogo** (o documento central para este agente)
O Quest Designer usa a Bible para todas as decisões de missão — quem pode aparecer, o que pode acontecer, quais as consequências plausíveis.

**B2 — Mapa de Progressão do Jogador**
Documento criado antes da produção:
- O que o jogador sabe e pode fazer no início
- Como as capacidades crescem ao longo do jogo
- Que momentos narrativos desbloqueiam que zonas
- Sequência de missões principais recomendada

**B3 — Tipologia de Missões**
Lista dos tipos de missão que existem no jogo com exemplos:
- Missão principal (formato e duração típica)
- Side quest (limites de complexidade)
- Missão de facção (regras de exclusividade)
- Missão de exploração (sem combat obrigatório?)
- Etc.

## TIPO C — Documentos de Outros Agentes

- Bible do Jogo completa (Agente #12) 🔴
- Catálogo de NPCs com nome (Agente #09) 🔴
- Mapa do mundo com zonas (Agente #05) 🔴
- Catálogo de arquétipos de inimigos (Agente #10) 🟡

---
---

# AGENTE #14 — AUDIO AGENT

## TIPO A — URLs Epic Games

| # | Título | URL | Prioridade |
|---|--------|-----|------------|
| 1 | Working with Audio — Hub | https://dev.epicgames.com/documentation/en-us/unreal-engine/working-with-audio-in-unreal-engine | 🔴 |
| 2 | MetaSounds Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/metasounds-in-unreal-engine | 🔴 |
| 3 | MetaSounds: Next Generation Sound Sources | https://dev.epicgames.com/documentation/en-us/unreal-engine/metasounds-the-next-generation-sound-sources-in-unreal-engine | 🔴 |
| 4 | MetaSounds Quick Start | https://dev.epicgames.com/documentation/en-us/unreal-engine/metasounds-quick-start | 🔴 |
| 5 | MetaSounds Reference Guide | https://dev.epicgames.com/documentation/en-us/unreal-engine/metasounds-reference-guide-in-unreal-engine | 🟡 |
| 6 | MetaSound Function Nodes | https://dev.epicgames.com/documentation/en-us/unreal-engine/metasound-function-nodes-reference-guide-in-unreal-engine | 🟡 |
| 7 | Sound Attenuation | https://dev.epicgames.com/documentation/en-us/unreal-engine/sound-attenuation-in-unreal-engine | 🔴 |
| 8 | Audio Volumes | https://dev.epicgames.com/documentation/en-us/unreal-engine/audio-volumes | 🟡 |
| 9 | Audio Mixer Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/audio-mixer-overview | 🟡 |
| 10 | Dialogue System | https://dev.epicgames.com/documentation/en-us/unreal-engine/dialogue-system-in-unreal-engine | 🟡 |
| 11 | Sound Cue Editor | https://dev.epicgames.com/documentation/en-us/unreal-engine/sound-cue-editor-in-unreal-engine | 🟢 |

## TIPO B — Documentos Internos 🔴 CRÍTICO

**B1 — Referências de Áudio**
Documento criado pelo Miguel:
- 5-10 jogos ou filmes com o áudio que melhor define o mood pretendido
- O que admira especificamente em cada referência (música, sound design, foley, silêncios)
- Estados emocionais que o som deve reforçar
- O que o áudio nunca deve fazer neste jogo

**B2 — Paleta Emocional de Estados Musicais**
Documento criado com o Narrative Agent:
- Lista de estados narrativos/de gameplay
- Para cada estado: instrumentação, intensidade, ritmo, tempo de transição
- Momentos de silêncio obrigatórios

**B3 — Referência GDC: The Sound of GTA V (Rockstar Audio, 2014)**
Disponível no YouTube e GDC Vault. Transcrever os pontos-chave e carregar como texto.

## TIPO C — Documentos de Outros Agentes

- Bible do Jogo — intenções emocionais (Agente #12) 🔴
- Estados de iluminação e condições meteorológicas (Agente #08) 🔴
- Intenções emocionais de missões (Agente #13) 🟡

---
---

# AGENTE #15 — CHARACTER ARTIST AGENT

## TIPO A — URLs Epic Games

| # | Título | URL | Prioridade |
|---|--------|-----|------------|
| 1 | MetaHuman Documentation Hub | https://dev.epicgames.com/documentation/en-us/metahuman | 🔴 |
| 2 | MetaHuman Creator Overview | https://dev.epicgames.com/documentation/en-us/metahuman/metahuman-creator | 🔴 |
| 3 | MetaHuman Export to UE5 | https://dev.epicgames.com/documentation/en-us/metahuman/downloading-and-exporting-metahumans | 🔴 |
| 4 | Game Animation Sample Project | https://dev.epicgames.com/documentation/en-us/unreal-engine/game-animation-sample-project-in-unreal-engine | 🔴 |
| 5 | Adding MetaHuman to Project | https://dev.epicgames.com/documentation/en-us/unreal-engine/adding-a-metahuman-to-the-game-animation-sample-project-in-unreal-engine | 🔴 |
| 6 | Skeletal Mesh Asset Details | https://dev.epicgames.com/documentation/en-us/unreal-engine/skeletal-mesh-asset-details-in-unreal-engine | 🟡 |
| 7 | Level of Detail (LOD) | https://dev.epicgames.com/documentation/en-us/unreal-engine/creating-and-using-lods-in-unreal-engine | 🔴 |
| 8 | Materials Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-materials | 🟡 |
| 9 | Physically Based Materials | https://dev.epicgames.com/documentation/en-us/unreal-engine/physically-based-materials-in-unreal-engine | 🟡 |
| 10 | Nanite Virtualized Geometry | https://dev.epicgames.com/documentation/en-us/unreal-engine/nanite-virtualized-geometry-in-unreal-engine | 🟡 |

## TIPO B — Documentos Internos 🔴 CRÍTICO

**B1 — Style Guide Visual de Personagens**
Documento criado antes de activar este agente:
- Exemplos visuais de referência para cada arquétipo social (pescador, nobre, guerreiro, etc.)
- Paleta de cores de vestuário por classe social e região
- Proibições visuais (o que nunca deve aparecer)
- Nível de detalhe esperado por tipo de personagem

**B2 — Bible do Jogo** (contexto cultural e histórico do período)
Para a precisão cultural de vestuário, adereços e aparência física.

**B3 — Catálogo de Arquétipos de NPC** (do Agente #09)
Lista de todos os tipos de NPC com o seu perfil social — para criar aparências coerentes com cada papel.

## TIPO C — Documentos de Outros Agentes

- Catálogo de arquétipos sociais (Agente #09) 🔴
- Bible do Jogo (Agente #12) 🔴
- Perfis de personagens com nome (Agente #12) 🔴
- Style Guide visual (documento interno) 🔴

---
---

# AGENTE #16 — ANIMATION AGENT

## TIPO A — URLs Epic Games

| # | Título | URL | Prioridade |
|---|--------|-----|------------|
| 1 | Animating Characters — Hub | https://dev.epicgames.com/documentation/en-us/unreal-engine/animating-characters-and-objects-in-unreal-engine | 🔴 |
| 2 | Motion Matching Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/motion-matching-in-unreal-engine | 🔴 |
| 3 | Game Animation Sample Project | https://dev.epicgames.com/documentation/en-us/unreal-engine/game-animation-sample-project-in-unreal-engine | 🔴 |
| 4 | Animation Blueprint Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-blueprints-in-unreal-engine | 🔴 |
| 5 | Animation State Machines | https://dev.epicgames.com/documentation/en-us/unreal-engine/state-machines-in-unreal-engine | 🔴 |
| 6 | Blend Spaces | https://dev.epicgames.com/documentation/en-us/unreal-engine/blend-spaces-in-unreal-engine | 🔴 |
| 7 | Inverse Kinematics (IK) | https://dev.epicgames.com/documentation/en-us/unreal-engine/inverse-kinematics-in-unreal-engine | 🔴 |
| 8 | Control Rig Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/control-rig-in-unreal-engine | 🟡 |
| 9 | Control Rig in Animation Blueprints | https://dev.epicgames.com/documentation/en-us/unreal-engine/control-rig-in-animation-blueprints-in-unreal-engine | 🟡 |
| 10 | Chaos Cloth Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-cloth-overview | 🟡 |
| 11 | Physics Asset Editor | https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-asset-editor-in-unreal-engine | 🟡 |
| 12 | IK Rig Animation Retargeting | https://dev.epicgames.com/documentation/en-us/unreal-engine/ik-rig-animation-retargeting-in-unreal-engine | 🟡 |
| 13 | Adding MetaHuman to Game Animation Sample | https://dev.epicgames.com/documentation/en-us/unreal-engine/adding-a-metahuman-to-the-game-animation-sample-project-in-unreal-engine | 🟡 |

## TIPO B — Documentos Internos 🔴 CRÍTICO

**B1 — Guia de Linguagem Corporal por Arquétipo**
Documento criado antes da produção:
- Para cada arquétipo de personagem: como caminha, como está parado, como reage ao medo
- Referências de filmes ou jogos com a linguagem corporal certa
- O que a animação nunca deve fazer (ex: movimentos demasiado fluidos para um personagem traumatizado)

**B2 — Referência: RDR2 Player Locomotion (GDC 2020)**
Resumo técnico da talk da Rockstar sobre o sistema de locomotion do Arthur Morgan.
Disponível em: https://www.gamedeveloper.com/art/learn-the-secrets-of-i-red-dead-redemption-2-i-s-player-locomotion-at-gdc
Carregar o resumo como texto na KB.

## TIPO C — Documentos de Outros Agentes

- Biografias visuais de personagens (Agente #15) 🔴
- Perfis sociais de NPCs (Agente #09) 🔴
- Tipos de acções de combate requeridas (Agente #10) 🟡

---
---

# AGENTE #17 — VFX AGENT

## TIPO A — URLs Epic Games

| # | Título | URL | Prioridade |
|---|--------|-----|------------|
| 1 | Niagara VFX Hub | https://dev.epicgames.com/documentation/en-us/unreal-engine/creating-visual-effects-in-niagara-for-unreal-engine | 🔴 |
| 2 | Overview of Niagara Effects | https://dev.epicgames.com/documentation/en-us/unreal-engine/overview-of-niagara-effects-for-unreal-engine | 🔴 |
| 3 | Quick Start for Niagara Effects | https://dev.epicgames.com/documentation/en-us/unreal-engine/quick-start-for-niagara-effects-in-unreal-engine | 🔴 |
| 4 | Key Concepts in Niagara | https://dev.epicgames.com/documentation/en-us/unreal-engine/key-concepts-in-niagara-effects-for-unreal-engine | 🔴 |
| 5 | Creating a Niagara System | https://dev.epicgames.com/documentation/en-us/unreal-engine/creating-a-niagara-system | 🔴 |
| 6 | Controlling Niagara Systems | https://dev.epicgames.com/documentation/en-us/unreal-engine/controlling-your-niagara-systems | 🟡 |
| 7 | Niagara Fluids | https://dev.epicgames.com/documentation/en-us/unreal-engine/niagara-fluids-in-unreal-engine | 🟡 |
| 8 | Chaos Destruction Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-destruction-overview | 🟡 |
| 9 | Decal Materials | https://dev.epicgames.com/documentation/en-us/unreal-engine/decal-materials-in-unreal-engine | 🟡 |
| 10 | Post Process Effects | https://dev.epicgames.com/documentation/en-us/unreal-engine/post-process-effects | 🟡 |
| 11 | Materials Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-materials | 🟡 |

## TIPO B — Documentos Internos 🔴 CRÍTICO

**B1 — Style Guide Visual** (paleta de cores e referências estéticas)
O VFX Agent precisa de saber se os efeitos são realistas, estilizados ou expressionistas para calibrar cada sistema Niagara.

**B2 — Catálogo de Intenções por Efeito**
Documento criado antes da produção:
- Lista dos tipos de VFX necessários no jogo
- Para cada tipo: o que deve comunicar ao jogador (dor? magia? destruição? beleza?)
- Nível de intensidade visual aceitável (o jogo é violento? contido?)
- Budget de performance por categoria de efeito

## TIPO C — Documentos de Outros Agentes

- Style Guide visual (documento interno) 🔴
- Intenções emocionais de momentos narrativos (Agente #12) 🔴
- Tipos de encontro de combate (Agente #10) 🟡

---
---

# AGENTE #18 — QA & TESTING AGENT

## TIPO A — URLs Epic Games

| # | Título | URL | Prioridade |
|---|--------|-----|------------|
| 1 | Automation Test Framework | https://dev.epicgames.com/documentation/en-us/unreal-engine/automation-test-framework-in-unreal-engine | 🔴 |
| 2 | Automation System User Guide | https://dev.epicgames.com/documentation/en-us/unreal-engine/automation-system-user-guide-in-unreal-engine | 🔴 |
| 3 | Automation Technical Guide | https://dev.epicgames.com/documentation/en-us/unreal-engine/automation-technical-guide | 🔴 |
| 4 | Functional Testing | https://dev.epicgames.com/documentation/en-us/unreal-engine/functional-testing-in-unreal-engine | 🔴 |
| 5 | Gauntlet Automation Framework | https://dev.epicgames.com/documentation/en-us/unreal-engine/gauntlet-automation-framework-overview-in-unreal-engine | 🟡 |
| 6 | Unreal Insights | https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-insights-in-unreal-engine | 🔴 |
| 7 | GPU Profiling | https://dev.epicgames.com/documentation/en-us/unreal-engine/gpu-profiling-in-unreal-engine | 🟡 |
| 8 | AI Debugging | https://dev.epicgames.com/documentation/en-us/unreal-engine/ai-debugging-in-unreal-engine | 🟡 |
| 9 | Visual Logger | https://dev.epicgames.com/documentation/en-us/unreal-engine/visual-logger-in-unreal-engine | 🟡 |
| 10 | Stat Commands | https://dev.epicgames.com/documentation/en-us/unreal-engine/stat-commands-in-unreal-engine | 🟡 |
| 11 | Setting Up Automation Test Report Server | https://dev.epicgames.com/documentation/en-us/unreal-engine/setting-up-an-automation-test-report-server | 🟢 |

## TIPO B — Documentos Internos 🔴 CRÍTICO

**B1 — Critérios de Aprovação por Feature**
Documento criado pelo Studio Director e actualizado continuamente:
- Para cada sistema ou feature: o que define "aprovado"
- Métricas de performance por zona (valores exactos, não ranges vagos)
- Comportamentos que são intoleráveis (zero tolerância)
- Comportamentos que são aceitáveis em beta mas não em release

**B2 — Arquitectura Técnica do Projecto** (para saber o que testar em cada sistema)

**B3 — Template de Bug Report**
Formato fixo e imutável que o QA Agent usa em todos os relatórios — garante parseabilidade pelo Make.com e consistência para o Studio Director.

## TIPO C — Documentos de Outros Agentes

- Arquitectura Técnica (Agente #02) 🔴
- Relatórios de performance do Optimizer (Agente #04) 🔴
- Outputs de todos os agentes para validação 🔴

---
---

# AGENTE #19 — INTEGRATION & BUILD AGENT

## TIPO A — URLs Epic Games

| # | Título | URL | Prioridade |
|---|--------|-----|------------|
| 1 | Source Control Overview | https://dev.epicgames.com/documentation/en-us/unreal-engine/source-control-in-unreal-engine | 🔴 |
| 2 | Using Git Source Control | https://dev.epicgames.com/documentation/en-us/unreal-engine/using-git-source-control-in-unreal-engine | 🔴 |
| 3 | Using Perforce Source Control | https://dev.epicgames.com/documentation/en-us/unreal-engine/using-perforce-as-source-control-for-unreal-engine | 🟡 |
| 4 | Unreal Build Tool | https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-build-tool | 🔴 |
| 5 | Unreal Automation Tool | https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-automation-tool | 🔴 |
| 6 | Build Operations (Cook, Package, Deploy) | https://dev.epicgames.com/documentation/en-us/unreal-engine/build-operations-cooking-packaging-deploying-and-running-projects-in-unreal-engine | 🔴 |
| 7 | Remote Control API | https://dev.epicgames.com/documentation/en-us/unreal-engine/remote-control-api-for-unreal-engine | 🔴 |
| 8 | Python Scripting in UE5 | https://dev.epicgames.com/documentation/en-us/unreal-engine/scripting-the-unreal-editor-using-python | 🔴 |
| 9 | Packaging Projects | https://dev.epicgames.com/documentation/en-us/unreal-engine/packaging-unreal-engine-projects | 🟡 |
| 10 | One File Per Actor | https://dev.epicgames.com/documentation/en-us/unreal-engine/one-file-per-actor-in-unreal-engine | 🟡 |
| 11 | Editor Scripting Utilities | https://dev.epicgames.com/documentation/en-us/unreal-engine/editor-scripting-utilities-in-unreal-engine | 🟡 |

## TIPO B — Documentos Internos 🔴 CRÍTICO

**B1 — Arquitectura Técnica do Projecto**
O Integration Agent usa a arquitectura para saber a ordem correcta de integração e as regras de interoperabilidade entre sistemas.

**B2 — Ordem de Integração e Mapa de Dependências**
Documento criado pelo Engine Architect especificamente para o Integration Agent:
- Diagrama de dependências entre todos os sistemas
- Ordem de integração obrigatória
- Sistemas que nunca podem ser integrados em paralelo
- Protocolo de resolução de conflitos

**B3 — Template de Changelog**
Formato fixo para os changelogs gerados após cada build — parseável pelo Studio Director e pelo Make.com.

## TIPO C — Documentos de Outros Agentes

- Arquitectura Técnica (Agente #02) 🔴
- Resultados de QA (Agente #18) 🔴
- Outputs de todos os agentes (ciclo contínuo) 🔴

---
---

## RESUMO DE PRIORIDADE DE CRIAÇÃO

### Antes de activar qualquer agente — documentos a criar com o Miguel

| # | Documento | Criado por | Usado por |
|---|-----------|------------|-----------|
| 1 | Conceito do Jogo | Miguel + Hugo | Todos os 19 agentes |
| 2 | Premissa Narrativa | Miguel | Agentes #12, #13, #01 |
| 3 | Style Guide Visual | Miguel | Agentes #06, #07, #08, #15, #17 |
| 4 | Guia Geográfico do Mundo | Miguel + Hugo | Agente #05 |
| 5 | Referências Narrativas | Miguel | Agente #12 |
| 6 | Regras do Mundo Narrativo | Miguel + Hugo | Agentes #12, #09, #10, #13 |

### Primeira semana — documentos gerados pelos agentes

| # | Documento | Gerado por | Distribuído a |
|---|-----------|------------|---------------|
| 1 | Arquitectura Técnica | Agente #02 | Todos os agentes técnicos |
| 2 | Bible do Jogo v1 | Agente #12 | Todos os agentes criativos |
| 3 | Mapa de biomas | Agente #05 | Agentes #06, #07, #08, #11 |

### Regra de ouro

> **Nenhum agente criativo deve estar em produção antes de ter a Bible do Jogo aprovada pelo Miguel. Nenhum agente técnico deve estar em produção antes de ter a Arquitectura Técnica aprovada pelo Miguel.**

---

*Versão 1.0 — Março 2026*
*Knowledge Base Map — Transpersonal Game Studio*
*Construído para o Miguel Martins*
