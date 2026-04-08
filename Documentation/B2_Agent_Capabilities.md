# B2 — LISTA DE CAPACIDADES DOS 18 AGENTES
## Transpersonal Game Studio — Documento Interno
## Studio Director: Miguel Martins
## Versão 1.0 — Março 2026

---

## AGENTES TÉCNICOS (Core Engine)

### #01 — Studio Director
**Função:** CEO do estúdio. Traduz visão criativa em tarefas técnicas.
**Output:** Documentação de projeto, especificações técnicas, coordenação geral
**Dependências:** Recebe do Miguel, coordena todos os outros

### #02 — Engine Architect  
**Função:** CTO técnico. Define arquitectura completa do jogo.
**Output:** Documento de Arquitectura Técnica, regras de desenvolvimento, estrutura de sistemas
**Dependências:** Recebe do #01, define regras para #03-#18

### #03 — Core Systems Programmer
**Função:** Implementa sistemas base: física, colisão, ragdoll, destruição.
**Output:** Código C++ dos sistemas fundamentais, classes base
**Dependências:** Segue arquitectura do #02

### #04 — Performance Optimizer
**Função:** Garante 60fps PC / 30fps consola.
**Output:** Perfis de performance, LOD chains, otimizações de código
**Dependências:** Analisa sistemas do #03, impõe limites a todos

---

## AGENTES DE MUNDO (World Building)

### #05 — Procedural World Generator
**Função:** Gera terrenos, biomas, rios usando PCG e World Partition.
**Output:** Sistemas de geração procedural, mapas base, configurações de biomas
**Dependências:** Usa sistemas do #03, segue limites do #04

### #06 — Environment Artist
**Função:** Povoa o mundo com vegetação, rochas, props, materiais.
**Output:** Assets ambientais, materiais PBR, sistemas de vegetação
**Dependências:** Trabalha sobre output do #05

### #07 — Architecture & Interior Agent
**Função:** Constrói edifícios e estruturas do período pré-histórico.
**Output:** Blueprints de estruturas, sistemas de construção modular
**Dependências:** Usa base ambiental do #06

### #08 — Lighting & Atmosphere Agent
**Função:** Ciclo dia/noite, clima, iluminação global com Lumen.
**Output:** Sistemas de iluminação dinâmica, configurações atmosféricas
**Dependências:** Completa o mundo visual sobre #07

---

## AGENTES DE PERSONAGENS (Character Systems)

### #09 — Character Artist Agent
**Função:** Cria personagens jogáveis e NPCs usando MetaHuman.
**Output:** Modelos de personagens, sistemas de customização, variações genéticas
**Dependências:** Usa mundo completo do #08

### #10 — Animation Agent
**Função:** Motion Matching para movimentos fluídos, IK de pés.
**Output:** Sistemas de animação, Motion Matching graphs, IK rigs
**Dependências:** Trabalha sobre personagens do #09

### #11 — NPC Behavior Agent
**Função:** Behavior Trees, rotinas diárias, sistema de memória.
**Output:** Árvores comportamentais, sistemas de IA social, rotinas NPCs
**Dependências:** Usa personagens animados do #10

### #12 — Combat & Enemy AI Agent
**Função:** IA de combate tático para dinossauros e inimigos.
**Output:** Sistemas de combate, IA tática, comportamentos de predadores
**Dependências:** Expande comportamentos do #11

### #13 — Crowd & Traffic Simulation
**Função:** Simulação de massas até 50.000 agentes com Mass AI.
**Output:** Sistemas de multidões, simulação de ecossistemas
**Dependências:** Usa toda a IA dos agentes #11-#12

---

## AGENTES DE CONTEÚDO (Content Creation)

### #14 — Quest & Mission Designer
**Função:** Converte narrativa em missões jogáveis.
**Output:** Sistema de quests, objectivos, progressão de campanha
**Dependências:** Precisa da Bible do #15

### #15 — Narrative & Dialogue Agent
**Função:** Bible do Jogo, história principal, diálogos, lore.
**Output:** Bible completa, sistema de diálogos, árvores narrativas
**Dependências:** Trabalha independentemente, mas informa todos

### #16 — Audio Agent
**Função:** Música adaptativa, efeitos sonoros, vozes com MetaSounds.
**Output:** Sistemas de áudio dinâmico, biblioteca de sons, música procedural
**Dependências:** Integra com todos os sistemas anteriores

### #17 — VFX Agent
**Função:** Efeitos visuais com Niagara, LOD chain a 3 níveis.
**Output:** Sistemas de partículas, efeitos ambientais, VFX de combate
**Dependências:** Trabalha sobre mundo e personagens completos

---

## AGENTES DE QUALIDADE (Quality Assurance)

### #18 — QA & Testing Agent
**Função:** Testa tudo. TEM PODER DE BLOQUEAR A BUILD.
**Output:** Relatórios de bugs, certificação de builds, planos de teste
**Dependências:** Testa output de TODOS os agentes

### #19 — Integration & Build Agent
**Função:** Integra outputs de TODOS numa build coerente.
**Output:** Builds funcionais, sistema de versionamento, rollback
**Dependências:** Recebe de TODOS, reporta ao #01

---

## FLUXO DE DEPENDÊNCIAS CRÍTICAS

```
#01 (Studio Director) → #02 (Engine Architect) → #03 (Core Systems)
↓
#04 (Performance) → #05 (World Gen) → #06 (Environment) → #07 (Architecture) → #08 (Lighting)
↓
#09 (Characters) → #10 (Animation) → #11 (NPC Behavior) → #12 (Combat AI) → #13 (Crowds)
↓
#15 (Narrative) → #14 (Quests) → #16 (Audio) → #17 (VFX) → #18 (QA) → #19 (Integration)
```

---

## REGRAS DE BLOQUEIO

- **#02 bloqueia #03-#19** se a Arquitectura Técnica não estiver aprovada
- **#15 bloqueia #14** se a Bible do Jogo não estiver completa  
- **#04 bloqueia qualquer agente** que exceda limites de performance
- **#18 bloqueia #19** se encontrar bugs críticos
- **#19 bloqueia entrega ao Miguel** se a integração falhar

---

*Documento B2 — Lista de Capacidades dos 18 Agentes*
*Transpersonal Game Studio — Março 2026*