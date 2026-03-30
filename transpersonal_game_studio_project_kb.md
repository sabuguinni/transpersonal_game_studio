# KNOWLEDGE BASE DO PROJECTO
## Transpersonal Game Studio — Miguel Martins
## Contexto completo para Claude Projects
## Março 2026

---

## IDENTIDADE DO PROJECTO

**Nome do estúdio:** Transpersonal Game Studio
**Director Criativo:** Miguel Martins, 11 anos
**Supervisor e fundador:** Hugo Martins (pai do Miguel)
**Tipo de jogo:** Mundo aberto / aventura — nível AAA (referências: RDR2, GTA)
**Motor:** Unreal Engine 5
**Plataforma de agentes:** Relevance AI + Make.com
**Repositório:** GitHub (conta sabuginni)

---

## CONTEXTO FAMILIAR E VISÃO

O Miguel aprendeu sozinho a criar jogos no Roblox (usa Lua). O Hugo está a construir um estúdio de desenvolvimento de jogos com IA para o Miguel, com a visão de que quando o Miguel tiver 16 anos as ferramentas de IA estarão num ponto que ultrapassa qualquer estúdio AAA humano. O Miguel é o Creative Director — os agentes executam, ele dirige.

---

## ARQUITECTURA DO ESTÚDIO — 19 AGENTES

Os 19 agentes estão organizados em 7 departamentos. Cada agente tem identidade epistémica própria (não apenas rules), protocolo de debate com outros agentes, contrato de output fixo, e mensagens de sinalização exactas e imutáveis.

### DEPARTAMENTO A — ORQUESTRAÇÃO
**#01 Studio Director** — CEO do estúdio. Recebe instruções do Miguel em linguagem simples, distribui tarefas, consolida outputs, reporta ao Miguel. Único agente que comunica directamente com o Miguel. Memória persistente obrigatória.

### DEPARTAMENTO B — ARQUITECTURA & ENGINE
**#02 Engine Architect** — CTO. Define arquitectura técnica completa, sistemas UE5 obrigatórios, regras que todos os agentes seguem. Memória persistente.
**#03 Core Systems Programmer** — Implementa física (Chaos), colisão, ragdoll, destruição, veículos. Output: ficheiros .h e .cpp com Doxygen + Blueprint wrappers.
**#04 Performance & Optimizer** — Garante 60fps PC / 30fps consola. Analisa Unreal Insights, identifica bottlenecks, propõe soluções. Memória persistente.

### DEPARTAMENTO C — WORLD BUILDING
**#05 Procedural World Generator** — Gera terrenos, biomas, estradas, rios usando PCG + World Partition. Output: mapa de biomas JSON + script Python UE5 + narrativa geográfica. Memória persistente.
**#06 Environment Artist** — Adiciona detalhe artístico ao terreno: vegetação, rochas, props, materiais. Cada prop tem razão narrativa.
**#07 Architecture & Interior Agent** — Cria edifícios e interiores historicamente coerentes. Cada interior tem história dos seus habitantes. Memória persistente.
**#08 Lighting & Atmosphere Agent** — Iluminação dinâmica completa: ciclo dia/noite, clima, atmosfera. Usa Lumen exclusivamente — sem lightmaps. Memória persistente.

### DEPARTAMENTO D — INTELIGÊNCIA ARTIFICIAL
**#09 NPC Behavior Agent** — Behavior Trees, rotinas diárias, memória de NPCs, reacções ao jogador. Usa Mass AI para >50 NPCs. Memória persistente — PRIORITÁRIO.
**#10 Combat & Enemy AI Agent** — IA de combate táctica: cover, flanqueamento, arquétipos, dificuldade adaptativa. Win rate alvo 40-70% na primeira tentativa.
**#11 Crowd & Traffic Simulation** — Mass AI para multidões urbanas (até 50.000 agentes). Tráfego, reacções a eventos, variação por hora do dia.

### DEPARTAMENTO E — NARRATIVA & CONTEÚDO
**#12 Narrative & Dialogue Agent** — Cria Bible do Jogo, história, diálogos, lore. Primeiro agente a activar depois do Engine Architect. Memória persistente — PRIORITÁRIO.
**#13 Quest & Mission Designer** — Converte beats narrativos em missões com hook, complicação, dilema, clímax, resolução. Memória persistente.
**#14 Audio Agent** — Sistema de música adaptativa por estados, foley por superfície (mín. 8 tipos), MetaSounds obrigatório.

### DEPARTAMENTO F — ARTE & PERSONAGENS
**#15 Character Artist Agent** — MetaHuman Creator, diversidade visual, LOD chain obrigatória, morph targets de expressão. Memória persistente.
**#16 Animation Agent** — Motion Matching para locomotion, IK de pés obrigatório, assinatura de movimento única por personagem.
**#17 VFX Agent** — Niagara exclusivamente (nunca Cascade), LOD chain de 3 níveis obrigatória, legibilidade gameplay > espectacularidade.

### DEPARTAMENTO G — QUALIDADE & INTEGRAÇÃO
**#18 QA & Testing Agent** — Testes automáticos contínuos. Bug report com formato fixo imutável. Nunca apresenta build com bugs críticos ao Miguel. Memória persistente.
**#19 Integration & Build Agent** — Maestro técnico. Ordem de integração obrigatória. Mantém 10 builds para rollback. Memória persistente — PRIORITÁRIO.

---

## ARQUITECTURA DE CONHECIMENTO — 3 CAMADAS

Cada agente tem três tipos de conhecimento separados:

**GOALS** — Identidade epistémica do agente. Quem é, como pensa, qual a sua convicção central, o que não é. Copiado para o campo Goals do agente no Relevance AI.

**RULES** — Comportamento, protocolos de debate, contratos de output, mensagens de sinalização exactas. Copiado para o campo Rules.

**KNOWLEDGE BASE (KB)** — Documentação técnica UE5 carregada via URLs com prefixo `https://r.jina.ai/` + documentos internos criados pelo Hugo e Miguel + outputs de outros agentes.

---

## REGRA DO JINA

Todos os URLs da documentação Epic Games usam o prefixo `https://r.jina.ai/` antes do URL completo. O site da Epic usa JavaScript para renderizar conteúdo — sem o Jina, o Relevance AI apanha apenas o título da página. O Jina é gratuito sem conta nem cartão de crédito.

Exemplo:
`https://r.jina.ai/https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-technical-details-in-unreal-engine`

---

## TOOLS NO RELEVANCE AI — NOMES EXACTOS

| Função | Tool exacta | Tipo |
|--------|-------------|------|
| Geração de texto/código | LLM | Clone |
| Chamadas a agentes/APIs internas | Relevance API Call | Verified (Add) |
| Chamadas HTTP externas (UE5, GitHub, Make.com) | Google API Call | Verified (Add) |
| Pesquisa na KB | Knowledge search | Verified (Add) |
| Recuperar registos | Retrieve Record(s) from Knowledge | Clone |
| Guardar memória persistente | Add New Record to Knowledge | Clone |
| Actualizar memória | Update Record in Knowledge | Clone |
| Gerar ficheiros | Export data to permanent downloadable file | Verified (Add) |
| Executar Python | Python Code | Clone |
| Pesquisa web | Web Search | Verified (Add) |

---

## MEMÓRIA PERSISTENTE

A memória persistente no Relevance AI é implementada via **Knowledge Tables** — não existe um campo "Long Term Memory Instructions". Para cada agente com memória activada:
1. Criar uma Knowledge Table com o nome `memoria_[nome_agente]`
2. Adicionar as 3 tools: Add New Record to Knowledge + Retrieve Record(s) from Knowledge + Update Record in Knowledge
3. O conteúdo de memória definido nos documentos é o que o agente guarda nessa tabela ao longo do tempo

**Agentes COM memória persistente:** #01, #02, #04, #05, #07, #08, #09 (prioritário), #12 (prioritário), #13, #15, #18, #19 (prioritário)
**Agentes SEM memória persistente:** #03, #06, #10, #11, #14, #16, #17

---

## TRIGGERS — ESTRUTURA

Cada agente é activado por **Webhook (POST)**. O Make.com chama o agente com um payload JSON específico. A estrutura base de cada payload:

```json
{
  "origem": "nome_do_agente_que_chamou",
  "tipo": "tipo_de_tarefa",
  "ciclo_id": "identificador_único_do_ciclo",
  "[campos específicos do agente]": "..."
}
```

O Studio Director (#01) é o único agente chamado directamente pelo Miguel. Todos os outros são chamados via Make.com a partir do Studio Director ou de outros agentes.

---

## VARIABLES GLOBAIS — CONFIGURAR POR FASES

**Fase 1 — Hoje:**
`project_name`, `dev_phase` (pre-producao), `miguel_contact`, `ue5_version` (5.4), `target_fps_pc` (60), `target_fps_console` (30), `frame_budget_ms` (16.6), `world_size_km2`, `max_npcs_simultaneous` (10000), `max_draw_calls` (3000), `vram_budget_gb` (8)

**Fase 2 — Quando Google Drive estiver organizado:**
Pastas: `/Studio/World/`, `/Studio/Art/`, `/Studio/Narrative/`, `/Studio/AI/`, `/Studio/Audio/`, `/Studio/Characters/`, `/Studio/Animation/`, `/Studio/VFX/`, `/Studio/Quests/`, `/Studio/QA/`, `/Studio/Builds/`, `/Studio/Reports/`

**Fase 3 — Quando Make.com estiver activo:** `make_webhook_base`
**Fase 4 — Quando GitHub estiver criado:** `github_repo`, `github_token`
**Fase 5 — Quando UE5 Remote Control estiver activo:** `ue5_remote_control_url` (http://localhost:30010)
**Fase 6 — APIs externas:** `elevenlabs_api_key`, `freesound_api_key`, `fab_api_key`, `metahuman_api_url`
**Fase 7 — Bases de dados:** `notion_npc_database_id`, `notion_quest_database_id`, `notion_characters_database_id`, `notion_bugs_database_id`

---

## DOCUMENTOS INTERNOS — CRIAR ANTES DE ACTIVAR AGENTES

Estes documentos são criados pelo Hugo e pelo Miguel e carregados como ficheiros na KB dos agentes relevantes:

**PRIORITÁRIO — antes de qualquer agente:**
- **Conceito do Jogo** — que jogo, que sensação, 3 coisas únicas, o que o jogador nunca deve sentir, protagonista em 3 frases
- **Premissa Narrativa** — protagonista, ferida, conflito, fim possível, tema
- **Style Guide Visual** — paleta de cores, referências visuais, proibições estéticas
- **Guia Geográfico do Mundo** — dimensões, biomas, clima, nomes de lugares
- **Regras do Mundo Narrativo** — o que existe e o que não existe (magia? tecnologia? violência?)
- **Referências Narrativas** — 5-10 jogos/filmes de referência com anotações

**Documentos gerados pelos agentes (distribuídos automaticamente via Make.com):**
- **Arquitectura Técnica** (gerada por #02) → distribuída a todos os agentes técnicos
- **Bible do Jogo** (gerada por #12) → distribuída a todos os agentes criativos

---

## ORDEM DE ACTIVAÇÃO DOS AGENTES

1. Criar todos os documentos internos com o Miguel
2. Configurar Variables Globais (Fase 1)
3. Activar #01 Studio Director
4. Activar #02 Engine Architect — gera Arquitectura Técnica
5. Activar #12 Narrative Agent — gera Bible do Jogo v1
6. Distribuir Arquitectura Técnica e Bible a todos os agentes via KB
7. Activar departamentos B, C, D, E, F, G por ordem

**Regra de ouro:** Nenhum agente criativo em produção sem Bible do Jogo aprovada. Nenhum agente técnico em produção sem Arquitectura Técnica aprovada.

---

## FILOSFIA DO SISTEMA DE AGENTES

Aprendida por comparação com o sistema de agentes da Transpersonal International (empresa do Hugo):

**O que os agentes do estúdio têm que agentes genéricos não têm:**
1. **Identidade epistémica** — cada agente sabe como pensa, não só o que faz. O Engine Architect pensa como John Carmack. O Narrative Agent pensa como Robert McKee.
2. **Declaração do que não são** — "Não és um gerador de heightmaps. És o arquitecto geográfico de uma civilização."
3. **Protocolos de debate fixos** — cada agente sabe exactamente como resolver conflitos com outros agentes específicos, com condições de cedência definidas.
4. **Contratos de output** — não "produz análise" mas exactamente quais os campos, em que ordem, com que critérios.
5. **Mensagens de sinalização exactas e imutáveis** — o Make.com consegue parsear outputs determinísticos.

---

## DOCUMENTOS PRODUZIDOS NESTE CHAT

Todos os ficheiros foram gerados e estão disponíveis para download:

1. **`game_empire_miguel.docx`** — Guia de 10 dias Unreal Engine 5 para o Miguel, em inglês, personalizado com o nome dele
2. **`studio_agentes_IA.docx`** — Arquitectura completa dos 19 agentes com system prompts (versão 1 — antes da melhoria de qualidade)
3. **`studio_KB_completa_v2.docx`** — Knowledge Base por agente com URLs, documentos internos e documentos de outros agentes (versão com Tipo A/B/C)
4. **`studio_agentes_identity_v3.md`** — Goals e Rules completos dos 19 agentes com identidade epistémica, protocolos de debate e contratos de output (versão final de qualidade)
5. **`studio_KB_por_agente_v1.md`** — KB por agente com URLs por subpágina (versão sem Jina)
6. **`studio_KB_jina_v3.md`** — KB por agente com 211 URLs no formato correcto `https://r.jina.ai/https://...`
7. **`studio_config_operacional_v1.md`** — Tools, Triggers, Memory e Variables para todos os agentes (com nomes genéricos)
8. **`studio_copiar_colar_v2.md`** — Guia copiar-colar com nomes exactos das tools do Relevance AI, payloads JSON dos triggers, instruções de memória, e variables globais por fases

---

## PRÓXIMOS PASSOS — O QUE FALTA FAZER

**Já feito:**
- ✅ Arquitectura de 19 agentes definida
- ✅ Goals e Rules com identidade epistémica para todos os 19 agentes
- ✅ KB com 211 URLs no formato Jina por agente
- ✅ Documentos internos identificados e descritos
- ✅ Tools, Triggers, Memory e Variables configurados
- ✅ Guia copiar-colar pronto para Relevance AI

**Em curso (Manus a executar):**
- 🔄 Transcrição do conteúdo dos 211 URLs via Jina para ficheiros .md por agente (19 ficheiros `kb_agente_01_studio_director.md` etc.)

**A fazer:**
- ⬜ Criar documentos internos com o Miguel (Conceito do Jogo, Premissa Narrativa, Style Guide Visual, etc.)
- ⬜ Configurar Variables Globais Fase 1 no Relevance AI
- ⬜ Criar os 19 agentes no Relevance AI e colar Goals + Rules
- ⬜ Carregar KBs em cada agente (ficheiros do Manus + documentos internos)
- ⬜ Configurar Tools em cada agente (guia copiar-colar disponível)
- ⬜ Criar Knowledge Tables de memória para os 11 agentes que precisam
- ⬜ Configurar Triggers quando Make.com estiver ligado
- ⬜ Organizar Google Drive com estrutura `/Studio/`
- ⬜ Criar repositório GitHub para o projecto UE5
- ⬜ Activar UE5 Remote Control API no editor
- ⬜ Guia de 10 dias UE5 para o Miguel começar a aprender

---

## REGRAS DE COMUNICAÇÃO DESTE PROJECTO

**Língua:** Português de Portugal (novo acordo ortográfico). Sem gerúndios. Sem linguagem brasileira.

**Tom:** Directo, técnico quando necessário, mas sempre acessível para o Miguel (11 anos) quando o contexto o exige.

**Formato de respostas:** Prosa quando possível. Tabelas e listas apenas quando a informação é genuinamente tabular. Sem bullet points decorativos.

**Ficheiros:** Sempre .md para documentos de configuração e KB. .docx para guias para o Miguel ou documentos formatados.

**Prioridade absoluta:** A visão criativa do Miguel tem sempre prioridade sobre qualquer consideração técnica.

---

## REFERÊNCIAS TÉCNICAS PRINCIPAIS

**UE5 sistemas críticos para este projecto:**
- Lumen (GI dinâmico) — obrigatório, sem lightmaps
- Nanite (geometria virtualizada) — obrigatório para static meshes >100k polígonos
- World Partition — obrigatório para mundos >4km²
- Mass AI / Mass Entity — obrigatório para >50 NPCs simultâneos
- PCG Framework (Procedural Content Generation) — geração de mundo
- Chaos Physics — física, destruição, veículos, pano
- MetaHuman Creator — personagens foto-realistas
- Motion Matching — sistema de animação por pose database
- MetaSounds — sistema de áudio procedural
- UE5 Remote Control API — automação via HTTP (porta 30010)

**Targets de performance:**
- PC alta gama: 60fps / 16.6ms frame budget
- PS5 / Xbox Series X: 30fps / 33.3ms frame budget
- Draw calls máximos: 3000 por frame
- NPCs simultâneos: 10.000 com Mass AI LOD
- VRAM budget: 8GB

---

*Versão 1.0 · Março 2026*
*Transpersonal Game Studio · Hugo e Miguel Martins*
*KB de contexto completo para Claude Projects*
