# TRANSPERSONAL GAME STUDIO — ESTADO COMPLETO DA CONFIGURAÇÃO
## Documento de continuidade para novo chat
## Versão 1.0 · Abril 2026

---

## IDENTIDADE DO PROJECTO

**Nome do estúdio:** Transpersonal Game Studio
**Director Criativo:** Miguel Martins, 11 anos
**Supervisor:** Hugo Martins (pai do Miguel)
**Tipo de jogo:** Mundo aberto / aventura AAA (referências: RDR2, GTA)
**Motor:** Unreal Engine 5.7
**Plataforma de agentes:** Relevance AI + Make.com (futuro)
**Repositório:** GitHub (conta sabuginni)
**Organização Relevance AI:** Transpersonal International / Transpersonal Game St...

---

## ESTADO ACTUAL — TODOS OS 19 AGENTES

| # | Nome | Estado | Memória |
|---|------|--------|---------|
| 01 | Studio Director | ✅ Published | ✅ Long-Term Memory activada |
| 02 | Engine Architect | ✅ Published | ✅ Long-Term Memory activada |
| 03 | Core Systems Programmer | ✅ Published | ❌ Não necessário |
| 04 | Performance Optimizer | ✅ Published | ✅ Long-Term Memory activada |
| 05 | Procedural World Generator | ✅ Published | ✅ Long-Term Memory activada |
| 06 | Environment Artist | ✅ Published | ❌ Não necessário |
| 07 | Architecture & Interior Agent | ✅ Published | ✅ Long-Term Memory activada |
| 08 | Lighting & Atmosphere Agent | ✅ Published | ✅ Long-Term Memory activada |
| 09 | NPC Behavior Agent | ✅ Published | ✅ Long-Term Memory activada (PRIORITÁRIO) |
| 10 | Combat & Enemy AI Agent | ✅ Published | ❌ Não necessário |
| 11 | Crowd & Traffic Simulation | ✅ Published | ❌ Não necessário |
| 12 | Narrative & Dialogue Agent | ✅ Published | ✅ Long-Term Memory activada (PRIORITÁRIO) |
| 13 | Quest & Mission Designer | ✅ Published | ✅ Long-Term Memory activada |
| 14 | Audio Agent | ✅ Published | ❌ Não necessário |
| 15 | Character Artist Agent | ✅ Published | ✅ Long-Term Memory activada |
| 16 | Animation Agent | ✅ Published | ❌ Não necessário |
| 17 | VFX Agent | ✅ Published | ❌ Não necessário |
| 18 | QA & Testing Agent | ✅ Published | ✅ Long-Term Memory activada |
| 19 | Integration & Build Agent | ✅ Published | ✅ Long-Term Memory activada (PRIORITÁRIO) |

**TODOS OS 19 AGENTES ESTÃO PUBLICADOS E OPERACIONAIS.**

---

## PROCESSO DE CONFIGURAÇÃO — COMO FOI FEITO

Cada agente foi configurado seguindo sempre esta sequência:

1. **Relevance AI → Agents → + New Agent**
2. Nome + modelo **Claude Sonnet 4.5 (latest)**
3. **Prompt** → colar **Goal** no campo Goal
4. **Prompt** → colar **Rules** no campo Rules
5. **Tools** → Add tool → adicionar tools da biblioteca pessoal
6. **Knowledge** → Add existing knowledge → seleccionar ficheiros .md
7. **Triggers** → Add Trigger → Webhook → nome + descrição → Continue → `{{$}}` → Continue → `ciclo_id` / `ciclo_id` → Continue
8. Copiar webhook URL → guardar em **Variables** do agente
9. **Memory** → activar Long-Term Memory (se aplicável) → Advanced Settings → colar instruções
10. **Publish**

---

## TOOLS POR AGENTE — REFERÊNCIA RÁPIDA

### Agentes COM memória (Add New Record to Knowledge tentado mas não funcional — usar Long-Term Memory nativa)

**Tools standard para agentes com memória:**
- LLM (temperature varia por agente)
- Python Code (maioria)
- Retrieve Record(s) from Knowledge
- Export data to permanent downloadable file
- Knowledge search
- Relevance API Call
- Google API Call
- Add New Record to Knowledge (quando disponível)

**Tools standard para agentes SEM memória:**
- LLM (temperature varia por agente)
- Retrieve Record(s) from Knowledge
- Export data to permanent downloadable file
- Knowledge search
- Relevance API Call
- Google API Call

### Temperatures por agente
| Agente | Temperature |
|--------|-------------|
| #01 Studio Director | 0.7 |
| #02 Engine Architect | 0.3 |
| #03 Core Systems | 0.2 |
| #04 Performance Optimizer | 0.2 |
| #05 World Generator | 0.6 |
| #06 Environment Artist | 0.6 |
| #07 Architecture & Interior | 0.5 |
| #08 Lighting & Atmosphere | 0.5 |
| #09 NPC Behavior | 0.4 |
| #10 Combat AI | 0.3 |
| #11 Crowd Simulation | 0.3 |
| #12 Narrative & Dialogue | 0.8 |
| #13 Quest Designer | 0.6 |
| #14 Audio | 0.5 |
| #15 Character Artist | 0.5 |
| #16 Animation | 0.4 |
| #17 VFX | 0.5 |
| #18 QA Testing | 0.1 |
| #19 Integration & Build | 0.1 |

---

## KNOWLEDGE BASE POR AGENTE

Cada agente tem dois ficheiros .md carregados:
1. `kb_agente_[XX]_[nome].md` — KB específica do agente com URLs Epic Games + documentação
2. `b1_conceito_do_jogo.md` — Conceito do jogo (partilhado por todos)

**Nota:** Os ficheiros KB foram gerados previamente e carregados directamente. Não foram usados URLs Jina — os ficheiros .md já contêm o conteúdo processado.

---

## TRIGGERS POR AGENTE

| Agente | Nome do trigger | Variable guardada |
|--------|----------------|-------------------|
| #01 Studio Director | `instrucao_miguel` + `aprovacao_ciclo` | `webhook_studio_director` |
| #02 Engine Architect | `nova_arquitectura` | `webhook_engine_architect` |
| #03 Core Systems | `novo_sistema` | `webhook_core_systems` |
| #04 Performance Optimizer | `analise_performance` | `webhook_performance_optimizer` |
| #05 World Generator | `gerar_regiao` | `webhook_world_generator` |
| #06 Environment Artist | `detalhar_zona` | `webhook_environment_artist` |
| #07 Architecture & Interior | `criar_estrutura` | `webhook_architecture_agent` |
| #08 Lighting & Atmosphere | `definir_iluminacao` | `webhook_lighting_agent` |
| #09 NPC Behavior | `popular_zona` + `evento_narrativo` | `webhook_npc_behavior_popular_zona` + `webhook_npc_behavior_evento_narrativo` |
| #10 Combat AI | `definir_combate` | `webhook_combat_agent` |
| #11 Crowd Simulation | `simular_zona_urbana` | `webhook_crowd_simulation` |
| #12 Narrative & Dialogue | `criar_bible` + `dialogos_npc` | `webhook_narrative_criar_bible` + `webhook_narrative_dialogos_npc` |
| #13 Quest Designer | `criar_missao` | `webhook_quest_designer` |
| #14 Audio | `definir_audio` | `webhook_audio_agent` |
| #15 Character Artist | `criar_personagem` | `webhook_character_artist` |
| #16 Animation | `animar_personagem` | `webhook_animation_agent` |
| #17 VFX | `criar_vfx` | `webhook_vfx_agent` |
| #18 QA Testing | `testar_build` | `webhook_qa_agent` |
| #19 Integration & Build | `integrar_asset` + `build_automatica` | `webhook_integration_integrar_asset` + `webhook_integration_build_automatica` |

---

## MEMÓRIA PERSISTENTE — COMO FOI IMPLEMENTADA

**Método usado:** Long-Term Memory nativa do Relevance AI (não Knowledge Tables)

**Processo para activar:**
1. Agente → **Memory** no menu esquerdo
2. Activar toggle **Enable Long-Term Memory**
3. **Advanced Settings** → campo **"Override the system prompt for saving to memory"** → colar instruções específicas
4. **Save** → **Publish**

**Instruções de memória por agente:**

### #01 Studio Director
```
Guarda e actualiza sempre estas informações:
ESTADO DO PROJECTO:
- Nome do jogo actual e versão
- Agentes activos e estado de cada um
- Ciclos completados e em curso
DECISÕES APROVADAS PELO MIGUEL:
- Data, decisão, contexto
ITENS PENDENTES DE APROVAÇÃO:
- O que está à espera de decisão do Miguel
```

### #02 Engine Architect
```
Guarda e actualiza sempre estas informações:
ARQUITECTURA ACTUAL:
- Versão do documento de arquitectura
- Sistemas UE5 activados (Lumen, Nanite, World Partition, Mass AI, PCG)
- Targets de performance por plataforma (valores exactos)
- Convenções de nomenclatura de assets em vigor
- Decisões irreversíveis (log com data e razão)
CONFLITOS RESOLVIDOS:
- Data, agentes em conflito, sistema, solução adoptada
```

### #04 Performance Optimizer
```
Guarda e actualiza sempre estas informações:
HISTÓRICO DE PERFORMANCE:
- Data, zona, frame time antes e depois da optimização, descrição da solução
- Sistemas com problemas recorrentes (padrões identificados)
BUDGET ACTUAL POR SISTEMA:
- Rendering: [ms]
- Physics: [ms]
- AI: [ms]
- Audio: [ms]
```

### #05 World Generator
```
Guarda e actualiza sempre estas informações:
MAPA GLOBAL DO MUNDO:
- Regiões geradas: nome, área em km², biomas, estado (rascunho/aprovado)
- Rios e sistema hidrográfico: pontos de nascente, direcção de fluxo, ligações
- Rede de estradas: nós principais e ligações geradas
- Convenções de nomenclatura geográfica adoptadas
```

### #07 Architecture & Interior
```
Guarda e actualiza sempre estas informações:
CATÁLOGO DE ESTRUTURAS CRIADAS:
- ID/nome, tipo, localização, estado de conservação, elementos interactivos, missões associadas
PROPS EM FALTA:
- Props solicitados que não existem no catálogo disponível
```

### #08 Lighting & Atmosphere
```
Guarda e actualiza sempre estas informações:
ESTADOS DE ILUMINAÇÃO APROVADOS:
- Zona, hora do dia, condição meteorológica, data de aprovação, localização dos parâmetros
PREFERÊNCIAS VISUAIS DO MIGUEL:
- Estados que aprovou com entusiasmo
- Temperaturas de cor e intensidades preferidas
- O que rejeitou e porquê
```

### #09 NPC Behavior (PRIORITÁRIO)
```
Guarda e actualiza sempre estas informações:
CATÁLOGO DE NPCs:
- ID, nome, arquétipo, zona, estado actual da rotina, reputação com o jogador
- NPCs que recordam interacções específicas com o jogador (log de eventos relevantes)
EVENTOS QUE ALTERARAM COMPORTAMENTOS:
- Data no jogo, evento, NPCs afectados, como o comportamento mudou
REPUTAÇÃO DO JOGADOR POR ZONA:
- Zona, reputação actual, eventos que a determinaram
```

### #12 Narrative & Dialogue (PRIORITÁRIO)
```
Guarda e actualiza sempre estas informações:
BIBLE DO JOGO — ESTADO ACTUAL:
- Versão e data
- Premissa em uma frase
- Ferida do protagonista em uma frase
- Facções e estado actual de cada uma
- Eventos que já aconteceram no mundo (timeline)
- Personagens com nome e estado actual
CONSISTÊNCIA NARRATIVA:
- Factos estabelecidos que nunca podem ser contraditos
- Contradições detectadas e como foram resolvidas (log)
```

### #13 Quest Designer
```
Guarda e actualiza sempre estas informações:
CATÁLOGO DE MISSÕES:
- ID, tipo, estado (rascunho/implementado/aprovado/em QA), zona, NPCs, número de ramificações
CONSEQUÊNCIAS PERMANENTES JÁ IMPLEMENTADAS:
- Missão, consequência no mundo, facções afectadas
MISSÕES COM PROBLEMAS DE PACING:
- ID, descrição do problema, nota de revisão
```

### #15 Character Artist
```
Guarda e actualiza sempre estas informações:
CATÁLOGO DE PERSONAGENS CRIADOS:
- ID, nome ou tipo, arquétipo, zona, parâmetros MetaHuman principais, estado (criado/aprovado)
- Personagens principais com morph targets de expressão completos
VARIAÇÕES POR ARQUÉTIPO:
- Arquétipo, número de variações únicas criadas, zonas onde foram colocadas
```

### #18 QA Testing
```
Guarda e actualiza sempre estas informações:
HISTÓRICO DE BUILDS:
- Número, estado (aprovada/bloqueada), bugs críticos encontrados, data, sistemas testados
BUGS CONHECIDOS:
- ID, sistema, severidade (Crítico/Alto/Médio/Baixo), estado (aberto/resolvido), agente responsável
PADRÕES DE FALHA RECORRENTES:
- Sistema, tipo de falha, frequência, data da última ocorrência
```

### #19 Integration & Build (PRIORITÁRIO)
```
Guarda e actualiza sempre estas informações:
HISTÓRICO DE BUILDS:
- Número, data, agentes incluídos, conflitos resolvidos, estado (aprovada pelo QA / bloqueada)
- As últimas 10 builds aprovadas (para rollback)
CONFLITOS RECORRENTES:
- Agentes em conflito, tipo de conflito, solução adoptada, frequência
ORDEM DE INTEGRAÇÃO ACTUAL:
- Qualquer desvio aprovado pelo Engine Architect à ordem padrão
```

---

## PROBLEMAS CONHECIDOS E SOLUÇÕES

### Add New Record to Knowledge não funciona
- **Problema:** A tool "Add New Record to Knowledge" não fica adicionada ao agente mesmo depois de seleccionada
- **Solução adoptada:** Usar Long-Term Memory nativa do Relevance AI (Memory → Enable Long-Term Memory)
- **Estado:** Resolvido para todos os 12 agentes com memória

### Google API Call pede conta Google
- **Problema:** Ao adicionar Google API Call aparece popup a pedir conta Google conectada
- **Solução:** Clicar **Skip** — a tool funciona para HTTP genérico sem autenticação Google

### Tools duplicadas
- **Problema:** Alguns agentes ficaram com "Export data to permanent downloadable file" duplicado
- **Solução:** Remover o duplicado via **...** → **Remove** na tool

---

## O QUE FALTA FAZER

### Imediato
- [ ] Verificar que todos os 19 agentes estão de facto Published (não Unpublished)
- [ ] Confirmar webhooks URLs guardados para todos os agentes

### Próximos passos (Make.com)
- [ ] Criar conta Make.com
- [ ] Configurar cenários de orquestração entre agentes
- [ ] Ligar webhooks do Relevance AI ao Make.com

### Documentos internos a criar com o Miguel
- [ ] B1 — Conceito do Jogo (já existe como `b1_conceito_do_jogo.md`)
- [ ] B2 — Premissa Narrativa
- [ ] B3 — Style Guide Visual
- [ ] B4 — Guia Geográfico do Mundo
- [ ] B5 — Referências de Áudio

### UE5
- [ ] Instalar Visual Studio 2022 com workloads C++
- [ ] Criar projecto C++ UE5 fora do OneDrive
- [ ] Activar Remote Control API no editor

### Integrações futuras
- [ ] GitHub — repositório `transpersonal-studio`
- [ ] Google Drive — estrutura `/Studio/` com subpastas
- [ ] ElevenLabs — API key já extraída
- [ ] Freesound — credenciais configuradas
- [ ] Notion — bases de dados de NPCs, missões, bugs

---

## DOCUMENTOS DE REFERÊNCIA NO PROJECTO

Ficheiros que devem estar carregados no Claude Project para continuidade:

1. `studio_agentes_identity_v3.md` — Goals e Rules completos dos 19 agentes (documento autoritativo)
2. `studio_KB_por_agente_v1.md` — KB por agente com URLs Epic Games
3. `studio_copiar_colar_v2.md` — Guia copiar-colar com nomes exactos das tools
4. `studio_config_operacional_v1.md` — Tools, Triggers, Memory e Variables
5. `transpersonal_studio_continuacao_agentes_v1.md` — Guia de continuação sessões anteriores
6. `transpersonal_game_studio_project_kb.md` — Knowledge base do projecto

---

*Transpersonal Game Studio · Hugo & Miguel Martins · Abril 2026*
*Todos os 19 agentes configurados e publicados no Relevance AI*
