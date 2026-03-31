# ESTADO DO PROJECTO — SESSÃO 1
## Transpersonal Game Studio
## Data: Março 2026

---

## O QUE FOI FEITO NESTA SESSÃO

### ✅ Plataformas configuradas

| Plataforma | Estado | Notas |
|---|---|---|
| Relevance AI | ✅ Activo | Team Plan, projecto "Transpersonal Game Studio" criado |
| UE5 5.7 | ✅ Activo | Projecto TranspersonalStudio criado em C++ |
| Visual Studio 2022 | ✅ Instalado | Workloads C++ e Game Development activas |
| Remote Control API | ✅ Activo | Porta 30010 confirmada a responder |
| ElevenLabs | ✅ Activo | Plano Creator, API key extraída |
| Freesound | ✅ Activo | Credenciais criadas (sabuginni) |
| GitHub | ⬜ Por configurar | Conta sabuginni existe, repositório UE5 por criar |
| Google Drive | ⬜ Por configurar | Estrutura /Studio/ por criar |
| Make.com | ⬜ Por configurar | Conta por criar |
| Notion | ⬜ Por configurar | Conta por criar |
| Anthropic API | ⬜ Por configurar | Conta por criar para ligar ao Relevance AI |

---

### ✅ Snippets globais criados no Relevance AI

| Nome | Valor |
|---|---|
| `project_name` | `TranspersonalStudio` |
| `dev_phase` | `pre-producao` |
| `ue5_version` | `5.7` |
| `target_fps_pc` | `60` |
| `target_fps_console` | `30` |
| `frame_budget_ms` | `16.6` |
| `world_size_km2` | `200` |
| `max_npcs_simultaneous` | `10000` |
| `max_draw_calls` | `3000` |
| `vram_budget_gb` | `8` |
| `ue5_remote_control_url` | `http://localhost:30010` |
| `webhook_studio_director` | `https://api-d7b62b.stack.tryrelevance.com/latest/agents/hooks/custom-trigger/a6b3c7c7-3afe-4a91-969c-560f70a2546d/1af5723e-c684-4c04-831b-733151b3d2ad` |

---

### ✅ Agente #01 — Studio Director — COMPLETO

**Estado:** Configurado, publicado e operacional

**Goal:** Colado — identidade baseada em Miyamoto e Kojima, tradutor entre visão criativa do Miguel e execução dos 18 agentes

**Rules:** Coladas — protocolo de distribuição de tarefas, estrutura de output, gestão de conflitos, mensagens exactas

**Knowledge Base:**
- `kb_agente_01_studio_director.md` — documentação técnica Epic Games (Remote Control API, Python Scripting)
- `B1_conceito_do_jogo.md` — conceito criativo do jogo criado com o Miguel

**Tools:**
- ✅ Relevance API Call
- ✅ Web Search

**Trigger:**
- Nome: `instrucao_miguel`
- URL: `https://api-d7b62b.stack.tryrelevance.com/latest/agents/hooks/custom-trigger/a6b3c7c7-3afe-4a91-969c-560f70a2546d/1af5723e-c684-4c04-831b-733151b3d2ad`
- Campos: `instrucao`, `contexto_adicional`, `prioridade`, `ciclo_id`

---

### ✅ Documento B1 — Conceito do Jogo — CRIADO

**Género:** Survival de mundo aberto
**Escala:** Regional
**Referências:** Jurassic Park Evolution 3, Jurassic Park Survival
**Sensação central:** Medo constante — o jogador é sempre a presa

**3 mecânicas únicas:**
1. Dinossauros com vida própria independente do jogador
2. Domesticação gradual de herbívoros pequenos
3. Variação física individual por dinossauro (não só cor — estrutura)

**O que nunca sentir:** Aborrecimento, frustração injusta, jogo fácil

**Protagonista:** Paleontologista transportado no tempo ao tocar numa gema. Objetivo: sobreviver, construir base, encontrar segunda gema para regressar ao presente.

---

## O QUE FALTA FAZER — PRÓXIMOS PASSOS

### Agentes por configurar (por ordem)

| # | Agente | Prioridade | Ficheiro KB disponível |
|---|---|---|---|
| #02 | Engine Architect | 🔴 Imediato | `kb_agente_02_engine_architect.md` |
| #12 | Narrative Agent | 🔴 Imediato | `kb_agente_12_narrative_agent.md` |
| #03 | Core Systems Programmer | 🟡 Fase 2 | `kb_agente_03_core_systems.md` |
| #04 | Performance Optimizer | 🟡 Fase 2 | `kb_agente_04_optimizer.md` |
| #05 | World Generator | 🟡 Fase 2 | `kb_agente_05_world_generator.md` |
| #06 | Environment Artist | 🟡 Fase 2 | `kb_agente_06_environment.md` |
| #07 | Architecture Agent | 🟡 Fase 2 | `kb_agente_07_architecture.md` |
| #08 | Lighting Agent | 🟡 Fase 2 | `kb_agente_08_lighting.md` |
| #09 | NPC Behavior Agent | 🟡 Fase 2 | `kb_agente_09_npc.md` |
| #10 | Combat AI Agent | 🟢 Fase 3 | `kb_agente_10_combat.md` |
| #11 | Crowd Simulation | 🟢 Fase 3 | `kb_agente_11_crowd.md` |
| #13 | Quest Designer | 🟢 Fase 3 | `kb_agente_13_quest.md` |
| #14 | Audio Agent | 🟢 Fase 3 | `kb_agente_14_audio.md` |
| #15 | Character Artist | 🟢 Fase 3 | `kb_agente_15_character.md` |
| #16 | Animation Agent | 🟢 Fase 3 | `kb_agente_16_animation.md` |
| #17 | VFX Agent | 🟢 Fase 3 | `kb_agente_17_vfx.md` |
| #18 | QA Agent | 🟢 Fase 3 | `kb_agente_18_qa.md` |
| #19 | Integration Agent | 🟢 Fase 3 | `kb_agente_19_integration.md` |

### Plataformas por configurar
- ⬜ GitHub — criar repositório `transpersonal-studio` na conta sabuginni
- ⬜ Google Drive — criar estrutura `/Studio/` com 13 subpastas
- ⬜ Make.com — criar conta e configurar cenários de orquestração
- ⬜ Notion — criar conta e bases de dados de NPCs, missões e bugs
- ⬜ Anthropic API — criar conta em console.anthropic.com e ligar ao Relevance AI

### Documentos internos por criar
- ⬜ B2 — Lista de capacidades dos 18 agentes (para carregar na KB do Studio Director)
- ⬜ Premissa Narrativa detalhada
- ⬜ Style Guide Visual
- ⬜ Guia Geográfico do Mundo
- ⬜ Regras do Mundo Narrativo

---

## COMO CONFIGURAR O AGENTE #02 — ENGINE ARCHITECT

### Goal a colar
```
És o Engine Architect do Transpersonal Game Studio — o CTO técnico do estúdio. O teu trabalho é definir a arquitectura técnica completa do jogo, os sistemas UE5 obrigatórios, e as regras que todos os outros agentes técnicos devem seguir.

O teu pensamento é moldado por John Carmack com a convicção de que a elegância técnica não é opcional — é o que separa um jogo que funciona de um jogo que dura. A tua convicção central é que uma arquitectura mal definida no início custa dez vezes mais para corrigir no final. Não és um agente que implementa — és o agente que define as regras que todos os outros seguem.

Quando recebes um conceito de jogo, a tua primeira pergunta não é "como implemento isto" — é "que arquitectura garante que isto pode crescer durante anos sem colapsar".
```

### Rules a colar
```
Antes de definir qualquer sistema, identificas obrigatoriamente: quais os sistemas críticos para o loop de jogo central, quais as dependências entre sistemas, e qual o budget de performance de cada sistema em millisegundos.

O documento de Arquitectura Técnica que produces tem sempre esta estrutura obrigatória: sistemas obrigatórios por departamento, regras de interoperabilidade entre sistemas, order de implementação com dependências, e budgets de performance por sistema.

Quando um agente técnico te pede validação de uma decisão de implementação, respondes sempre com: aprovado, aprovado com condições, ou rejeitado — nunca com "depende" sem especificação concreta.

Quando dois sistemas técnicos entram em conflito de recursos, o protocolo é: calculas o impacto de cada abordagem em frame budget, propões a solução que maximiza qualidade dentro do budget, e documentas a decisão com a razão técnica.

A mensagem exacta que envias ao Studio Director quando a Arquitectura Técnica está pronta é: "Arquitectura Técnica v[número] concluída. Sistemas definidos: [N]. Budget total alocado: [X]ms de [Y]ms disponíveis. Pronta para distribuição aos agentes técnicos."

As regras absolutas que nunca violas: nunca aprovoas código que excede o frame budget sem justificação documentada. Nunca defines dois sistemas que dependem circularmente um do outro. Nunca deixas um agente técnico trabalhar sem arquitectura aprovada.
```

### Tools a adicionar
- Relevance API Call
- Web Search
- Python Code (Clone)

### Trigger
- Nome: `nova_arquitectura`
- Campos: `origem`, `tipo`, `conceito_jogo`, `requisitos`, `ciclo_id`

### Knowledge Base
- Carregar `kb_agente_02_engine_architect.md`
- Carregar `B1_conceito_do_jogo.md` (o mesmo ficheiro do Studio Director)

---

## ESTRUTURA DO WORKSPACE RELEVANCE AI

```
Organização: Transpersonal International
└── Projecto: Transpersonal Game Studio
    ├── Agentes/
    │   ├── ✅ Studio Director (#01)
    │   └── [18 agentes por criar]
    ├── Snippets/ (12 snippets criados)
    └── Knowledge/ (KBs por agente)
```

---

## NOTAS TÉCNICAS IMPORTANTES

- Os snippets no Relevance AI usam a sintaxe `{{snippets.nome_do_snippet}}` nos prompts dos agentes
- O Remote Control API do UE5 responde em `http://localhost:30010/remote/info`
- O projecto UE5 está em modo C++ — o Visual Studio 2022 é obrigatório para compilar
- Os webhooks dos agentes são URLs secretos — guardar sempre nos Snippets antes de fechar
- A memória persistente dos agentes usa Knowledge Tables, não a secção Memory nativa do Relevance AI

---

*Documento gerado no final da Sessão 1*
*Transpersonal Game Studio — Março 2026*
