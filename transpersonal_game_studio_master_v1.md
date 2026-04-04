# TRANSPERSONAL GAME STUDIO — DOCUMENTO MASTER COMPLETO
## Guia de Reconstrução Total do Projecto
## Versão 1.0 · Abril 2026
## Hugo Martins + Miguel Martins

---

> **PROPÓSITO DESTE DOCUMENTO**
> Este documento contém tudo o que foi feito, como foi feito, e todos os valores configurados desde o início do projecto. Se perderes o acesso ao projecto Claude, podes carregar este documento num novo projecto e reconstruir tudo sem perder nada. Serve também como referência para configurar outros sistemas de agentes IA com a mesma metodologia.

---

## PARTE 1 — IDENTIDADE DO PROJECTO

### Quem somos

| Campo | Valor |
|-------|-------|
| Nome do estúdio | Transpersonal Game Studio |
| Director Criativo | Miguel Martins (11 anos) |
| Supervisor | Hugo Martins (pai do Miguel) |
| Tipo de jogo | Mundo aberto / aventura AAA |
| Referências | RDR2, GTA |
| Motor | Unreal Engine 5.7 |
| Plataforma de agentes | Relevance AI + Make.com |
| Repositório GitHub | sabuginni/transpersonal_game_studio |
| Organização Relevance AI | Transpersonal International |
| Projecto Relevance AI | Transpersonal Game Studio |

### Visão

O Miguel aprendeu sozinho a criar jogos no Roblox (usa Lua). O Hugo está a construir um estúdio de desenvolvimento de jogos com IA para o Miguel, com a visão de que quando o Miguel tiver 16 anos as ferramentas de IA estarão num ponto que ultrapassa qualquer estúdio AAA humano. O Miguel é o Creative Director — os agentes executam, ele dirige.

---

## PARTE 2 — O JOGO

### Conceito central

Jogo de sobrevivência em mundo aberto Jurássico/Cretáceo. Um paleontologista é transportado no tempo ao tocar numa gema e acorda sozinho num mundo pré-histórico onde os dinossauros têm vidas independentes. O jogador é sempre a presa, nunca o predador dominante. A sensação pretendida: "Estou sozinho num mundo que não foi feito para mim."

### Mecânicas únicas

1. **Dinossauros com vidas independentes** — rotinas diárias, fome, território, sem existirem para servir o jogador
2. **Domesticação gradual de herbívoros pequenos** — baseada em paciência e confiança, não em comandos
3. **Variação física individual por dinossauro** — cada animal é visualmente único (sistema procedural genético)

### Mundo

- **Dimensão:** ~200km² fechado por montanhas intransponíveis
- **5 biomas:** Forest (início), Swamp, Savana, Desert, Snowy Rockside (destino final)
- **Rios:** nascem nas montanhas nevadas e fluem para o interior; mais secos na Savana e Desert; congelados no Snowy Rockside
- **Sem ruínas** — mundo completamente natural
- **Cavernas** existem em vários biomas
- **Gema final** aparece no Snowy Rockside após todas as missões concluídas

### Protagonista

Paleontologista. Começa na Forest — sozinho, sem outros humanos. Ferramenta básica (rústica) e ferramenta avançada. Construção com materiais naturais (madeira, pedra). Missões claras e directas, não puzzles. Fim: escolha entre voltar ao presente ou ficar com os dinossauros para sempre.

### Referências visuais (Style Guide)

- Referências: ARK, Jurassic World Evolution 3
- Dia: azul, colorido, visível — como vida real
- Noite: roxo escuro, estrelas, aurora boreal ocasional
- Dinossauros: realismo científico — não como Jurassic Park
- HUD minimalista

### Referências de áudio

- Música adaptativa: calma na exploração, intensa na fuga/ataque
- Música pode ser ligada/desligada nas definições
- Sons de ambiente sempre activos e independentes
- Dinossauros: sons realistas no mundo aberto; dramáticos (tipo filme) nas missões para carnívoros

---

## PARTE 3 — ARQUITECTURA DO SISTEMA DE 19 AGENTES

### Princípio fundamental

O sistema funciona como um estúdio real: o Miguel fala apenas com o Studio Director (#01). O Studio Director distribui tarefas e coordena os outros 18 agentes. Os agentes comunicam entre si via webhooks. O ciclo fecha quando o Integration & Build Agent (#19) reporta de volta ao Studio Director.

### Os 19 agentes

| # | Nome | Departamento | Memória | Função |
|---|------|-------------|---------|--------|
| 01 | Studio Director | Orquestração | ✅ PRIORITÁRIO | CEO do estúdio. Único agente que fala com o Miguel |
| 02 | Engine Architect | Arquitectura & Engine | ✅ | CTO. Define arquitectura técnica completa |
| 03 | Core Systems Programmer | Arquitectura & Engine | ❌ | Física, colisão, ragdoll, destruição |
| 04 | Performance Optimizer | Arquitectura & Engine | ✅ | Garante 60fps PC / 30fps consola |
| 05 | Procedural World Generator | World Building | ✅ | Terrenos, biomas, rios, PCG + World Partition |
| 06 | Environment Artist | World Building | ❌ | Vegetação, rochas, props, materiais |
| 07 | Architecture & Interior Agent | World Building | ✅ | Edifícios e interiores historicamente coerentes |
| 08 | Lighting & Atmosphere Agent | World Building | ✅ | Ciclo dia/noite, clima, Lumen exclusivo |
| 09 | NPC Behavior Agent | IA | ✅ PRIORITÁRIO | Behavior Trees, rotinas, memória de NPCs |
| 10 | Combat & Enemy AI Agent | IA | ❌ | IA de combate táctica |
| 11 | Crowd & Traffic Simulation | IA | ❌ | Mass AI para multidões (até 50.000 agentes) |
| 12 | Narrative & Dialogue Agent | Narrativa | ✅ PRIORITÁRIO | Bible do Jogo, história, diálogos, lore |
| 13 | Quest & Mission Designer | Narrativa | ✅ | Converte beats narrativos em missões |
| 14 | Audio Agent | Narrativa | ❌ | Música adaptativa, MetaSounds |
| 15 | Character Artist Agent | Arte | ✅ | MetaHuman Creator, diversidade visual |
| 16 | Animation Agent | Arte | ❌ | Motion Matching, IK de pés |
| 17 | VFX Agent | Arte | ❌ | Niagara exclusivo, LOD chain 3 níveis |
| 18 | QA & Testing Agent | Qualidade | ✅ | Testes automáticos, bug reports |
| 19 | Integration & Build Agent | Qualidade | ✅ PRIORITÁRIO | Integração, builds, rollback 10 versões |

### Ordem de integração (lei — nunca violar sem aprovação do Engine Architect)

Engine Architect → Core Systems → World Generator → Environment Artist → Architecture Agent → Lighting Agent → Character Artist → Animation Agent → NPC Behavior Agent → Combat AI Agent → Crowd Simulation Agent → Narrative Agent → Quest Designer Agent → Audio Agent → VFX Agent → QA Agent

---

## PARTE 4 — PLATAFORMAS E CREDENCIAIS

### Relevance AI

- **Plano:** Team Plan
- **Organização:** Transpersonal International
- **Projecto:** Transpersonal Game Studio
- **Modelo de todos os agentes:** Claude Sonnet 4.5 (latest)
- **URL base da API:** `https://api-d7b62b.stack.tryrelevance.com`
- **Webhooks:** guardar sempre em Snippets antes de fechar o popup

### Make.com

- **Plano:** Core
- **19 cenários activos** — um por ligação entre agentes
- **Configuração padrão de cada cenário:** Authentication None, Method POST, Body application/json, Parse response Yes

### Unreal Engine 5

- **Versão:** 5.7
- **Projecto:** TranspersonalStudio (modo C++)
- **Localização:** `C:\Unreal Projects\TranspersonalStudio`
- **Remote Control API:** activa na porta 30010
- **URL de verificação:** `http://localhost:30010/remote/info`

### GitHub

- **Conta:** sabuginni
- **Repositório:** `sabuginni/transpersonal_game_studio`
- **Branch principal:** main
- **Cliente:** GitHub Desktop (autenticação Google OAuth)
- **Pasta local:** `C:\Unreal Projects\transpersonal_game_studio_remote\transpersonal_game_studio`

### Visual Studio 2022

- **Workloads instalados:** Desktop development with C++ + Game development with C++

### Google Drive

- **Estrutura:** `/Studio/` com 13 subpastas

### Notion

- **4 bases de dados** criadas em conta pessoal

### ElevenLabs

- **Plano:** Creator (300k créditos)
- **Email:** hugomartins@transpersonalinternational.com
- **API Key name:** transpersonal-studio

### Freesound

- **Conta:** sabuginni
- **App name:** Transpersonal Studio

---

## PARTE 5 — TODOS OS SNIPPETS CONFIGURADOS NO RELEVANCE AI

> **Como aceder:** Relevance AI → More → Snippets
> **Como usar nos agentes:** `{{snippets.nome_do_snippet}}`

### Snippets de projecto e UE5

| Nome do Snippet | Valor |
|----------------|-------|
| `project_name` | TranspersonalStudio |
| `dev_phase` | pre-producao |
| `ue5_version` | 5.7 |
| `target_fps_pc` | 60 |
| `target_fps_console` | 30 |
| `frame_budget_ms` | 16.6 |
| `world_size_km2` | 200 |
| `max_npcs_simultaneous` | 10000 |
| `max_draw_calls` | 3000 |
| `vram_budget_gb` | 8 |
| `ue5_remote_control_url` | http://localhost:30010 |

### Snippets de webhooks dos agentes

| Nome do Snippet | Valor |
|----------------|-------|
| `webhook_studio_director` | https://api-d7b62b.stack.tryrelevance.com/latest/agents/hooks/custom-trigger/a6b3c7c7-3afe-4a91-969c-560f70a2546d/1af5723e-c684-4c04-831b-733151b3d2ad |
| `webhook_engine_architect` | https://api-d7b62b.stack.tryrelevance.com/latest/agents/hooks/custom-trigger/a6b3c7c7-3afe-4a91-969c-560f70a2546d/8f61db1d-6732-4dc0-b4a7-693157eabeaf |

### Snippets do Google Drive

| Nome do Snippet | ID da pasta |
|----------------|-------------|
| `google_drive_root` | 1HozrwhoyVFQDUtQ4DzCohogyeIvC6dg6 |
| `google_drive_ai_folder` | 1zqJ5ycp1-lzEs_buSxkztFKRxw6dS8T1 |
| `google_drive_animation_folder` | 1xKOzzLIPJ6PtSyOB9IKqf5slOyg8TTwe |
| `google_drive_art_folder` | 1kuif2OpF2oV7Zy0aXMyMNYgIR0bf5nFm |
| `google_drive_audio_folder` | 1-nY0sSXziXbO7bkD84ONTCl_oeC6XQ3Z |
| `google_drive_builds_folder` | 1pEaz2vXH-1sNCHD2s-9bNnNz5iy3IENM |
| `google_drive_characters_folder` | 1aMxwZhmvDXQsqHm2LNWFyHAZ-CmSCAhf |
| `google_drive_engine_folder` | 1pdOeK756pMwq0MPSH_-bUZUjL8nZ_IR |
| `google_drive_narrative_folder` | 1dklK1mOKsxniiGlDirPmH9xxlYYF2_JZ |
| `google_drive_qa_folder` | 1AmI7qCGwf_n7cq32yiB3Ln99eCJP4jqI |
| `google_drive_quests_folder` | 1xluKp6cVq1dRth9sfA7GmyxYLlnK-R3H |
| `google_drive_reports_folder` | 1LQRJEsEkuF3agYzC8KYMnn-EfDO7knEE |
| `google_drive_vfx_folder` | 17gqmT_5a3yukoYM-dZ0hfgOhjJsy1Qyx |
| `google_drive_world_folder` | 1wqRoNorqd4rzPDyLFOoetWGbliPEjDdT |

### Snippets do Notion

| Nome do Snippet | ID da base de dados |
|----------------|---------------------|
| `notion_npc_database_id` | 3380f880d0c48059838d000cf04b65b3 |
| `notion_quest_database_id` | 3380f880d0c4804386e5e48e14b627e9 |
| `notion_characters_database_id` | 3380f880d0c480f69c04f9f287865983 |
| `notion_bugs_database_id` | 3380f880d0c480758be2e8200774755c |

### Snippets de APIs externas

| Nome do Snippet | Valor |
|----------------|-------|
| `elevenlabs_api_key` | f3a95a37bab1aa180341a5ef3a9ca911b6429da1b57cc03fd7293c945d8ec01a |
| `freesound_api_key` | YPhQFmcIZSMY4sppmhU3krk0Bx8dGvKP9wRSBD2b |
| `freesound_client_id` | 34jouzAlHvOKt6gWNNNR |

---

## PARTE 6 — GOOGLE DRIVE: ESTRUTURA DE PASTAS

### Pasta raiz: `/Studio/` (ID: 1HozrwhoyVFQDUtQ4DzCohogyeIvC6dg6)

| Pasta | ID |
|-------|-----|
| `/Studio/AI/` | 1zqJ5ycp1-lzEs_buSxkztFKRxw6dS8T1 |
| `/Studio/Animation/` | 1xKOzzLIPJ6PtSyOB9IKqf5slOyg8TTwe |
| `/Studio/Art/` | 1kuif2OpF2oV7Zy0aXMyMNYgIR0bf5nFm |
| `/Studio/Audio/` | 1-nY0sSXziXbO7bkD84ONTCl_oeC6XQ3Z |
| `/Studio/Builds/` | 1pEaz2vXH-1sNCHD2s-9bNnNz5iy3IENM |
| `/Studio/Characters/` | 1aMxwZhmvDXQsqHm2LNWFyHAZ-CmSCAhf |
| `/Studio/Engine/` | 1pdOeK756pMwq0MPSH_-bUZUjL8nZ_IR |
| `/Studio/Narrative/` | 1dklK1mOKsxniiGlDirPmH9xxlYYF2_JZ |
| `/Studio/QA/` | 1AmI7qCGwf_n7cq32yiB3Ln99eCJP4jqI |
| `/Studio/Quests/` | 1xluKp6cVq1dRth9sfA7GmyxYLlnK-R3H |
| `/Studio/Reports/` | 1LQRJEsEkuF3agYzC8KYMnn-EfDO7knEE |
| `/Studio/VFX/` | 17gqmT_5a3yukoYM-dZ0hfgOhjJsy1Qyx |
| `/Studio/World/` | 1wqRoNorqd4rzPDyLFOoetWGbliPEjDdT |

### Mapa agente → pasta Google Drive

| Agente | Pasta atribuída |
|--------|----------------|
| Engine Architect (#02) | `{{snippets.google_drive_engine_folder}}` |
| Procedural World Generator (#05) | `{{snippets.google_drive_world_folder}}` |
| Environment Artist (#06) | `{{snippets.google_drive_world_folder}}` |
| Architecture & Interior Agent (#07) | `{{snippets.google_drive_world_folder}}` |
| Lighting & Atmosphere Agent (#08) | `{{snippets.google_drive_world_folder}}` |
| NPC Behavior Agent (#09) | `{{snippets.google_drive_ai_folder}}` |
| Combat & Enemy AI Agent (#10) | `{{snippets.google_drive_ai_folder}}` |
| Crowd & Traffic Simulation (#11) | `{{snippets.google_drive_ai_folder}}` |
| Narrative & Dialogue Agent (#12) | `{{snippets.google_drive_narrative_folder}}` |
| Quest & Mission Designer (#13) | `{{snippets.google_drive_quests_folder}}` |
| Audio Agent (#14) | `{{snippets.google_drive_audio_folder}}` |
| Character Artist Agent (#15) | `{{snippets.google_drive_characters_folder}}` |
| Animation Agent (#16) | `{{snippets.google_drive_animation_folder}}` |
| VFX Agent (#17) | `{{snippets.google_drive_vfx_folder}}` |
| QA & Testing Agent (#18) | `{{snippets.google_drive_qa_folder}}` |
| Integration & Build Agent (#19) | `{{snippets.google_drive_builds_folder}}` |

---

## PARTE 7 — NOTION: BASES DE DADOS

### Base de dados NPCs (ID: 3380f880d0c48059838d000cf04b65b3)

Propriedades: Nome (texto), Arquétipo (texto), Zona (texto), Estado (selecção: rascunho/implementado/aprovado), Reputação com jogador (número), Agente responsável (texto), Notas (texto)

### Base de dados Quests (ID: 3380f880d0c4804386e5e48e14b627e9)

Propriedades: Nome (texto), Tipo (selecção: principal/secundária/opcional), Zona (texto), Estado (selecção: rascunho/implementado/aprovado), NPCs envolvidos (texto), Agente responsável (texto), Notas (texto)

### Base de dados Characters (ID: 3380f880d0c480f69c04f9f287865983)

Propriedades: Nome (texto), Arquétipo (texto), Zona (texto), Estado (selecção: rascunho/implementado/aprovado), Morph targets (texto), Agente responsável (texto), Notas (texto)

### Base de dados Bugs (ID: 3380f880d0c480758be2e8200774755c)

Propriedades: Nome (texto), Sistema (texto), Severidade (selecção: crítico/major/minor), Estado (selecção: aberto/resolvido/won't fix), Agente responsável (texto), Passos para reproduzir (texto), Notas (texto)

### Mapa agente → base de dados Notion

| Agente | Base(s) de dados |
|--------|-----------------|
| NPC Behavior Agent (#09) | `{{snippets.notion_npc_database_id}}` |
| Narrative & Dialogue Agent (#12) | `{{snippets.notion_npc_database_id}}` + `{{snippets.notion_quest_database_id}}` |
| Quest & Mission Designer (#13) | `{{snippets.notion_quest_database_id}}` |
| Character Artist Agent (#15) | `{{snippets.notion_characters_database_id}}` |
| QA & Testing Agent (#18) | `{{snippets.notion_bugs_database_id}}` |

---

## PARTE 8 — MAKE.COM: ORQUESTRAÇÃO

### 19 cenários activos

| # | Cenário | Estado |
|---|---------|--------|
| 1 | `01_studio_director → 02_engine_architect` | ✅ Activo |
| 2 | `02_engine_architect → 03_core_systems_programmer` | ✅ Activo |
| 3 | `03_core_systems → 04_performance_optimizer` | ✅ Activo |
| 4 | `04_performance_optimizer → 05_world_generator` | ✅ Activo |
| 5 | `05_procedural_world_generator → 06_environment_artist` | ✅ Activo |
| 6 | `06_environment_artist → 07_architecture_interior_agent` | ✅ Activo |
| 7 | `07_architecture_interior → 08_lighting_atmosphere_agent` | ✅ Activo |
| 8 | `08_lighting_atmosphere → 09_npc_behavior_agent` | ✅ Activo |
| 9 | `09_npc_behavior → 10_combat_enemy_ai_agent` | ✅ Activo |
| 10 | `10_combat_enemy_ai → 11_crowd_traffic_simulation` | ✅ Activo |
| 11 | `11_crowd_simulation → 12_narrative_dialogue_agent` | ✅ Activo |
| 12 | `12_narrative_dialogue → 13_quest_mission_designer` | ✅ Activo |
| 13 | `13_quest_designer → 14_audio_agent` | ✅ Activo |
| 14 | `14_audio_agent → 15_character_artist_agent` | ✅ Activo |
| 15 | `15_character_artist → 16_animation_agent` | ✅ Activo |
| 16 | `16_animation_agent → 17_vfx_agent` | ✅ Activo |
| 17 | `17_vfx_agent → 18_qa_testing_agent` | ✅ Activo |
| 18 | `18_qa_testing → 19_integration_build_agent` | ✅ Activo |
| 19 | `19_integration_build → 01_studio_director` | ✅ Activo |

### Configuração padrão de cada cenário Make.com

- Authentication: None
- Method: POST
- Body: application/json
- Body input method: JSON string
- Parse response: Yes
- Toggle: "Immediately as data arrives" — ACTIVADO

### Variables make_webhook configuradas em cada agente

Cada agente tem uma variable `make_webhook_[próximo_agente]` no Relevance AI com o URL do Make.com correspondente. Esta variable é usada pela Relevance API Call tool para disparar o próximo agente na cadeia.

---

## PARTE 9 — PROCESSO DE CONFIGURAÇÃO DE CADA AGENTE

### Sequência padrão (seguida para todos os 19 agentes)

1. Relevance AI → Agents → **+ New Agent**
2. Nome + modelo: **Claude Sonnet 4.5 (latest)**
3. **Prompt → Goal** — colar o texto de Goal do agente
4. **Prompt → Rules** — colar o texto de Rules do agente
5. **Tools** → Add tool → adicionar tools (ver lista abaixo)
6. **Knowledge** → Add existing knowledge → seleccionar ficheiros .md (KB específica do agente + B1 obrigatório + documentos B2-B5 conforme relevância)
7. **Triggers** → Add Trigger → Webhook → nome + descrição → Continue → `{{$}}` como message input → Continue → `ciclo_id` como thread ID → Continue
8. Copiar webhook URL → guardar em **Variables/Snippets** antes de fechar
9. **Memory** → activar Long-Term Memory (se aplicável) → Advanced Settings → colar instruções
10. **Publish**
11. Adicionar linha no **Goal**: `A tua pasta de trabalho no Google Drive é: {{snippets.google_drive_[pasta]_folder}}`
12. Adicionar linha no **Goal** (se aplicável): `A tua base de dados no Notion é: {{snippets.notion_[nome]_database_id}}`
13. **Publish** novamente

### Tools disponíveis no Relevance AI

| Tool no Relevance AI | Como adicionar | Uso |
|---------------------|----------------|-----|
| LLM | Clone (Tool Builder) | Raciocínio e geração de conteúdo |
| Relevance API Call | Add (Verified) | Chamar outros agentes |
| Google API Call | Add (Verified) | HTTP POST para webhooks externos |
| Knowledge search | Add (Verified) | Pesquisar na KB |
| Retrieve Record(s) from Knowledge | Clone | Recuperar registos específicos da KB |
| Export data to permanent downloadable file | Add (Verified) | Gerar ficheiros .md, .json |
| Python Code | Clone | Executar Python |
| Web Search | Add (Verified) | Pesquisa web |
| Save to Memory | Built-in | Guardar em Long-Term Memory |
| Delete from Memory | Built-in | Apagar da Long-Term Memory |

### Nota crítica sobre Google API Call

Ao adicionar a Google API Call tool, aparece um popup a pedir conta Google. **Clicar Skip** — a tool funciona para HTTP genérico sem autenticação Google. Se não clicar Skip, fica bloqueado.

### Nota crítica sobre Long-Term Memory

Long-Term Memory activa-se em: **Agente → Memory → Enable Long-Term Memory → Advanced Settings → colar instruções**. A memória persistente usa este toggle nativo, não Knowledge Tables manuais.

### Nota crítica sobre tools duplicadas

Alguns agentes ficam com "Export data to permanent downloadable file" duplicado. Remover o duplicado via **...** → **Remove** na tool.

---

## PARTE 10 — DOCUMENTOS INTERNOS DO JOGO (KB)

### Documentos criados com o Miguel e carregados nas KBs dos agentes

| Documento | Versão | Agentes com KB |
|-----------|--------|----------------|
| B1 — Conceito do Jogo | v1.0 | TODOS os agentes |
| B2 — Premissa Narrativa | v1.1 | Studio Director, Narrative & Dialogue, Quest Designer, NPC Behavior |
| B3 — Style Guide Visual | v1.0 | Environment Artist, Architecture & Interior, Lighting & Atmosphere, Character Artist, VFX Agent |
| B4 — Guia Geográfico do Mundo | v1.0 | Studio Director, Procedural World Generator, Environment Artist |
| B5 — Referências de Áudio | v1.0 | Audio Agent |

### Pontos-chave do B2 — Premissa Narrativa

- Protagonista: paleontologista transportado ao tocar numa gema
- Começa na Forest — sozinho, sem outros humanos
- Exploração livre — sem guias ou obrigações de percurso
- Construção com materiais naturais (madeira, pedra)
- Ferramenta básica (rústica) e ferramenta avançada (mais elaborada)
- Missões claras e directas — não são puzzles
- Modo criativo: recursos infinitos, configurações ajustáveis
- Fim: escolha entre voltar ao presente ou ficar com os dinossauros
- Gema só aparece no Snowy Rockside após todas as missões concluídas

### Pontos-chave do B4 — Guia Geográfico

- Mundo: ~200 km², fechado por montanhas intransponíveis
- 5 biomas: Forest, Swamp, Savana, Desert, Snowy Rockside
- Início do jogador: sempre na Forest
- Rios nascem nas montanhas e fluem para o interior
- Rios mais secos na Savana e Desert, congelados no Snowy Rockside
- Sem ruínas — mundo completamente natural
- Cavernas existem em vários biomas

---

## PARTE 11 — METODOLOGIA: O QUE APRENDEMOS

### Lições críticas que aplicar em qualquer projecto de agentes IA

**1. Long-Term Memory: usar o toggle nativo, não Knowledge Tables manuais**
O "Add New Record to Knowledge" tool é inconsistente e falha. O toggle Long-Term Memory nativo funciona de forma fiável para todos os agentes.

**2. Google API Call: clicar Skip no popup de autenticação**
A tool funciona para HTTP genérico sem Google auth. Clica Skip sempre que o popup aparecer.

**3. Tools duplicadas: verificar e remover**
Especialmente "Export data to permanent downloadable file" — remover via três pontos → Remove.

**4. Webhooks: guardar IMEDIATAMENTE em Snippets**
O URL do webhook só aparece uma vez quando o trigger é criado. Se não for guardado nesse momento, tens de recriar o trigger.

**5. Goal e Rules: copiar dos documentos originais, não de resumos**
Claude pode truncar conteúdo ao resumir. Usar sempre os documentos originais como fonte.

**6. Snippets: sintaxe correcta é `{{snippets.nome_do_snippet}}`**
Esta sintaxe é usada nos campos Goal e Rules dos agentes para referenciar valores configurados centralmente.

**7. Relevance API Call vs Google API Call para chamar agentes**
Para chamar outros agentes via webhook, usar a **Google API Call** (HTTP POST directo ao URL do webhook). A Relevance API Call usa um sistema de jobs assíncrono que pode não disparar o webhook imediatamente.

**8. Studio Director precisa de instrução explícita para chamar agentes**
No Rules do Studio Director, incluir o protocolo de chamada com o URL exacto e instrução "nunca descreves que vais chamar — chamas de facto usando a tool".

**9. agent_id dos agentes está no URL do Relevance AI**
Formato: `https://app.relevanceai.com/agents/[workspace_id]/[agent_id]/...`

**10. Testar cada agente individualmente**
Após configurar, enviar uma mensagem de teste directamente a cada agente para verificar que lê os documentos da KB correctamente e produz output coerente.

---

## PARTE 12 — ESTADO ACTUAL DO PROJECTO (Abril 2026)

### O que está completo e operacional

| Sistema | Estado |
|---------|--------|
| 19 agentes Relevance AI | ✅ Todos publicados e operacionais |
| Long-Term Memory (12 agentes) | ✅ Activada |
| KBs carregadas | ✅ KB específica + B1 + B2-B5 conforme relevância |
| Make.com (19 cenários) | ✅ Todos activos |
| Variables make_webhook (por agente) | ✅ Configuradas |
| Google Drive (/Studio/ + 13 subpastas) | ✅ Criado |
| Snippets Google Drive (14) | ✅ Configurados |
| Google Drive atribuído a cada agente no Goal | ✅ Feito |
| Notion (4 bases de dados) | ✅ Criadas |
| Snippets Notion (4) | ✅ Configurados |
| Notion atribuído a agentes relevantes | ✅ Feito |
| ElevenLabs (snippet + Audio Agent) | ✅ Ligado |
| Freesound (snippets + Audio Agent) | ✅ Ligado |
| GitHub (repositório com ficheiros UE5) | ✅ Operacional |
| Visual Studio 2022 | ✅ Instalado com workloads C++ |
| UE5 Remote Control API | ✅ Activa na porta 30010 |
| Fluxo Miguel → Studio Director → Engine Architect | ✅ Testado e confirmado |
| Procedural World Generator | ✅ Testado — operacional |
| NPC Behavior Agent | ✅ Testado — operacional (46s) |
| Narrative & Dialogue Agent | ✅ Testado — operacional (24s) |

### O que falta fazer

| Item | Prioridade |
|------|-----------|
| Testar os restantes 16 agentes individualmente | Alta |
| Adicionar snippet `webhook_engine_architect` a todos os agentes que precisam de o chamar | Alta |
| Configurar snippets de webhooks dos restantes agentes | Alta |
| Notion — ligar ao sistema de agentes via API calls | Média |
| ElevenLabs — configurar vozes específicas para personagens | Baixa |
| Fab.com — API key para assets | Baixa |
| MetaHuman — pipeline para Character Artist | Baixa |

---

## PARTE 13 — GUIA DE RECONSTRUÇÃO (SE PERDERES O PROJECTO)

### Passo 1 — Criar workspace no Relevance AI

1. Criar conta em relevanceai.com
2. Criar organização: "Transpersonal International"
3. Criar projecto: "Transpersonal Game Studio"
4. Ir a Settings → Variables e criar todos os snippets da Parte 5

### Passo 2 — Carregar documentos na Knowledge Base

1. Ir a Knowledge → Create Knowledge Table
2. Carregar os ficheiros B1, B2, B3, B4, B5 e os ficheiros KB específicos de cada agente
3. Os ficheiros KB de cada agente estão nos documentos do projecto Claude original

### Passo 3 — Criar os 19 agentes

Para cada agente, seguir a sequência da Parte 9. Os textos de Goal e Rules de todos os agentes estão no ficheiro `studio_agentes_identity_v3.md` do projecto Claude original.

### Passo 4 — Configurar Make.com

1. Criar conta em make.com
2. Criar 19 cenários (um por ligação entre agentes)
3. Cada cenário: recebe webhook → faz HTTP POST para o webhook do próximo agente
4. Configuração: Authentication None, POST, application/json, Parse Yes
5. Guardar os URLs dos webhooks Make.com nos snippets dos agentes correspondentes

### Passo 5 — Recriar Google Drive

1. Criar pasta `/Studio/` no Google Drive
2. Criar 13 subpastas (AI, Animation, Art, Audio, Builds, Characters, Engine, Narrative, QA, Quests, Reports, VFX, World)
3. Copiar os IDs de cada pasta do URL → actualizar snippets na Parte 5

### Passo 6 — Recriar Notion

1. Criar 4 bases de dados (NPCs, Quests, Characters, Bugs) com as propriedades da Parte 7
2. Copiar os IDs → actualizar snippets na Parte 5

### Passo 7 — Testar o fluxo

1. Abrir Studio Director → Run → New task
2. Escrever: "O plano está definido nos documentos B1, B2 e B4. Inicia o CICLO_001_ARQUITECTURA_BASE e chama o Engine Architect agora."
3. Verificar se o Engine Architect recebe a chamada na sua Timeline

---

## PARTE 14 — APLICAÇÃO DESTA METODOLOGIA A OUTROS PROJECTOS DE AGENTES

### O que torna este sistema eficaz

Este sistema de agentes funciona bem porque segue princípios que se aplicam a qualquer projecto de agentes IA:

**Identidade epistémica** — cada agente tem uma personalidade e convicção central, não apenas rules técnicas. Isto produz outputs mais coerentes e menos genéricos.

**Contratos de output fixos** — cada agente tem uma "mensagem exacta" que envia quando termina o trabalho. Isto torna a orquestração previsível.

**Memória selectiva** — nem todos os agentes precisam de memória. Activar apenas onde é necessário reduz complexidade e custos.

**Documentos base partilhados** — todos os agentes têm acesso ao documento B1 (conceito base). Documentos específicos são carregados apenas nos agentes relevantes.

**Snippets centralizados** — valores que mudam (IDs, URLs, API keys) ficam num local central. Mudar um snippet actualiza todos os agentes que o referenciam.

**Webhooks como protocolo de comunicação** — cada agente tem um endpoint de entrada (trigger) e sabe o endpoint de saída do próximo agente. Simples e robusto.

### Como adaptar a um projecto de marketing ou empresa

Para um sistema de agentes de gestão de empresa (marketing, vendas, operações), a metodologia é idêntica:

1. **Definir o "Studio Director" equivalente** — o agente que recebe instruções do humano e distribui tarefas
2. **Mapear os departamentos** — cada departamento tem agentes especializados (equivalente aos departamentos A-G do Game Studio)
3. **Definir os documentos base** — equivalente ao B1, são os documentos da empresa: brand guidelines, target audience, products, tone of voice
4. **Criar os snippets de infraestrutura** — URLs das ferramentas, IDs das bases de dados, API keys
5. **Configurar Make.com** — mesmo processo, mesma lógica
6. **Testar o fluxo** — enviar uma instrução ao agente director e verificar se os outros agentes a recebem

### Estrutura sugerida para agentes de marketing

| # | Agente | Função |
|---|--------|--------|
| 01 | Marketing Director | Recebe briefings, distribui tarefas, reporta resultados |
| 02 | Brand Strategist | Garante consistência de marca em todos os outputs |
| 03 | Content Strategist | Define temas, calendário editorial, pilares de conteúdo |
| 04 | Copywriter | Escreve textos para todos os canais |
| 05 | Social Media Manager | Adapta conteúdo para cada rede social |
| 06 | SEO Specialist | Optimiza conteúdo para motores de busca |
| 07 | Email Marketing Agent | Sequências de email, newsletters |
| 08 | Analytics Agent | Analisa performance, identifica padrões |
| 09 | Ad Copy Agent | Cria copy para anúncios pagos |
| 10 | QA Agent | Verifica consistência, erros, alinhamento com brand |

### Documentos base para agentes de marketing

- **M1 — Brand Bible** — quem somos, o que vendemos, tom de voz, o que nunca dizemos
- **M2 — Target Audience** — personas detalhadas, dores, desejos, linguagem
- **M3 — Product Guide** — todos os produtos/serviços com benefícios e diferenciadores
- **M4 — Competitor Analysis** — o que fazem os concorrentes, onde diferenciamos
- **M5 — Content Guidelines** — formatos, extensões, estruturas por canal

---

*Transpersonal Game Studio · Hugo & Miguel Martins*
*Documento Master v1.0 · Abril 2026*
*Gerado no final da Sessão 4 — Projecto completamente operacional*
