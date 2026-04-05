# TRANSPERSONAL GAME STUDIO — ESTADO COMPLETO v5
## 5 de Abril de 2026 · Hugo & Miguel Martins

---

## ÍNDICE

1. [Visão Geral do Projecto](#1-visão-geral-do-projecto)
2. [Credenciais Críticas](#2-credenciais-críticas)
3. [Arquitectura do Sistema](#3-arquitectura-do-sistema)
4. [Os 19 Agentes](#4-os-19-agentes)
5. [Webhooks dos Agentes](#5-webhooks-dos-agentes)
6. [Snippets Relevance AI](#6-snippets-relevance-ai)
7. [Sistema de Aprovação Telegram](#7-sistema-de-aprovação-telegram)
8. [Make.com — Orquestração](#8-makecom--orquestração)
9. [n8n — Novo Orquestrador Central](#9-n8n--novo-orquestrador-central)
10. [Google Drive](#10-google-drive)
11. [Notion](#11-notion)
12. [GitHub](#12-github)
13. [UE5](#13-ue5)
14. [Servidor Hetzner](#14-servidor-hetzner)
15. [O que foi feito nesta sessão](#15-o-que-foi-feito-nesta-sessão)
16. [Problemas conhecidos](#16-problemas-conhecidos)
17. [O que falta fazer — por ordem de prioridade](#17-o-que-falta-fazer--por-ordem-de-prioridade)

---

## 1. VISÃO GERAL DO PROJECTO

O Transpersonal Game Studio é um sistema de **19 agentes IA** no Relevance AI que desenvolve colaborativamente um jogo AAA open-world de sobrevivência pré-histórico com dinossauros.

**Director criativo:** Miguel Martins  
**Arquitecto técnico:** Hugo  
**Plataforma de agentes:** Relevance AI (Team Plan) — modelo Claude Sonnet 4.5  
**Orquestrador:** n8n (novo, instalado no servidor Hetzner) a substituir Make.com  

### Conceito do jogo
- Mundo pré-histórico open-world de 200km²
- Protagonista paleontologista
- 3 mecânicas signature: dinossauros com vidas independentes, domesticação gradual de herbívoros pequenos, variação física individual por dinossauro
- 5 biomas: Floresta Densa, Pântano, Savana, Deserto Rochoso, Snowy Rockside
- Endpoint narrativo: gema brilhante escondida em Snowy Rockside

---

## 2. CREDENCIAIS CRÍTICAS

### Relevance AI
| Campo | Valor |
|-------|-------|
| Authorization token | `a6b3c7c7-3afe-4a91-969c-560f70a2546d:sk-ZTBmNjdhZjgtYzhlYy00MTE0LWJmMjEtMGU2M2I5M2Y0MGEz` |
| Region | `d7b62b` |
| Project ID | `a6b3c7c7-3afe-4a91-969c-560f70a2546d` |
| API endpoint | `https://api-d7b62b.stack.tryrelevance.com/latest/agents/trigger` |
| URL base webhooks | `https://api-d7b62b.stack.tryrelevance.com/latest/agents/hooks/custom-trigger/a6b3c7c7-3afe-4a91-969c-560f70a2546d/[TRIGGER_ID]` |

### Telegram Bot
| Campo | Valor |
|-------|-------|
| Bot name | Transpersonal Studio |
| Username | @TranspersonalStudioBot |
| Token | `8470343698:AAHVuWkEFrMI_4VIEJHle867b7htMX_HuPU` |
| Chat ID Hugo | `5308155885` |

### Make.com
| Cenário | Função | Estado |
|---------|--------|--------|
| 00_notificacoes_aprovacao | Webhook → Data Store → Telegram + Gmail | ✅ Activo |
| 00_resposta_telegram_agentes | Telegram → Data Store → Webhook agente | ✅ Activo |
| Webhook notificações | `https://hook.eu2.make.com/d1nfe64bnhik5qtwo22mpb9yqy67k6ny` | ✅ |

### n8n (novo)
| Campo | Valor |
|-------|-------|
| URL | `http://88.99.188.76:5678` |
| Webhook orquestrador | `http://88.99.188.76:5678/webhook/agente-concluido` |
| Webhook iniciar ciclo | `http://88.99.188.76:5678/webhook/iniciar-ciclo` |

### Servidor Hetzner
| Campo | Valor |
|-------|-------|
| Nome | transpersonal-lms |
| IP | `88.99.188.76` |
| Tipo | CPX42 — 8 vCPU, 16GB RAM, 320GB SSD |
| OS | Ubuntu 24.04.4 LTS |
| User | root |

### APIs Externas
| Serviço | Credencial |
|---------|-----------|
| ElevenLabs API key | `f3a95a37bab1aa180341a5ef3a9ca911b6429da1b57cc03fd7293c945d8ec01a` |
| Freesound Client Secret | `YPhQFmclZSMY4sppmhU3krk0Bx8dGvKP9wRSBD2b` |
| Freesound Client ID | `34jouzAlHvOKt6gWNNNR` |

### GitHub
| Campo | Valor |
|-------|-------|
| Conta | sabuginni |
| Repositório | `sabuginni/transpersonal_game_studio` |
| Branch | main |
| Deploy | GitHub Actions com chave SSH (secrets do repositório) |

### Google Drive
| Campo | Valor |
|-------|-------|
| Pasta raiz | `/Studio/` |
| ID raiz | `1HozrwhoyVFQDUtQ4DzCohogyeIvC6dg6` |
| URL | `https://drive.google.com/drive/folders/1HozrwhoyVFQDUtQ4DzCohogyeIvC6dg6` |

---

## 3. ARQUITECTURA DO SISTEMA

### Arquitectura anterior (Make.com) — FRÁGIL
```
Miguel → Studio Director → Make.com → Agente A → Make.com → Agente B → ...
```
**Problema:** cadeia linear sem retry, sem watchdog, duplicate ID bloqueava agentes silenciosamente.

### Nova arquitectura (n8n) — ROBUSTA
```
Miguel → Studio Director (Relevance AI)
              ↓
         n8n Orquestrador (Hetzner 24/7)
         ├── Retry automático (3x com espera crescente)
         ├── ciclo_id único com timestamp (nunca duplica)
         ├── Detecção de falhas
         └── Encadeamento automático
              ↓
         19 Agentes (Relevance AI) — chamados pelo n8n
```

### Cadeia de agentes
```
01_SD → 02_EA → 03_CS → 04_PO → 05_WG → 06_EN → 07_AI → 08_LA
→ 09_NB → 10_CE → 11_CT → 12_ND → 13_QM → 14_AU → 15_CA
→ 16_AN → 17_VF → 18_QA → 19_IB → 01_SD (ciclo completo)
```

### Como o n8n orquestra
1. Agente conclui tarefa → chama `POST http://88.99.188.76:5678/webhook/agente-concluido`
2. n8n recebe → calcula próximo agente → gera novo ciclo_id com timestamp
3. n8n chama webhook do próximo agente no Relevance AI
4. Se falhar → retry automático 3x (espera 30s, 60s, 90s)
5. Ciclo completo quando agente #19 termina

---

## 4. OS 19 AGENTES

| # | Código | Nome | Departamento | Memória | Função |
|---|--------|------|-------------|---------|--------|
| 01 | SD | Studio Director | Orquestração | ✅ | CEO. Único agente que fala com Miguel |
| 02 | EA | Engine Architect | Engine | ✅ | CTO. Arquitectura técnica completa |
| 03 | CS | Core Systems Programmer | Engine | ❌ | Física, colisão, ragdoll, destruição |
| 04 | PO | Performance Optimizer | Engine | ✅ | Garante 60fps PC / 30fps consola |
| 05 | WG | Procedural World Generator | World Building | ✅ | Terrenos, biomas, PCG + World Partition |
| 06 | EN | Environment Artist | World Building | ❌ | Vegetação, rochas, props, materiais |
| 07 | AI | Architecture & Interior | World Building | ✅ | Edifícios e interiores historicamente coerentes |
| 08 | LA | Lighting & Atmosphere | World Building | ✅ | Ciclo dia/noite, clima, Lumen exclusivo |
| 09 | NB | NPC Behavior | IA | ✅ | Behavior Trees, rotinas, memória de NPCs |
| 10 | CE | Combat & Enemy AI | IA | ❌ | IA de combate táctica |
| 11 | CT | Crowd & Traffic | IA | ❌ | Mass AI até 50.000 agentes |
| 12 | ND | Narrative & Dialogue | Narrativa | ✅ | Bible do Jogo, história, diálogos, lore |
| 13 | QM | Quest & Mission Designer | Narrativa | ✅ | Converte beats narrativos em missões |
| 14 | AU | Audio Agent | Narrativa | ❌ | Música adaptativa, MetaSounds |
| 15 | CA | Character Artist | Arte | ✅ | MetaHuman Creator, diversidade visual |
| 16 | AN | Animation Agent | Arte | ❌ | Motion Matching, IK de pés |
| 17 | VF | VFX Agent | Arte | ❌ | Niagara exclusivo, LOD chain 3 níveis |
| 18 | QA | QA & Testing | Qualidade | ✅ | Testes automáticos, bug reports |
| 19 | IB | Integration & Build | Qualidade | ✅ | Integração, builds, rollback 10 versões |

---

## 5. WEBHOOKS DOS AGENTES

Base URL: `https://api-d7b62b.stack.tryrelevance.com/latest/agents/hooks/custom-trigger/a6b3c7c7-3afe-4a91-969c-560f70a2546d/`

| # | Código | Agent ID / Trigger ID | URL completa |
|---|--------|-----------------------|--------------|
| 01 | SD | `1af5723e-c684-4c04-831b-733151b3d2ad` | BASE + `1af5723e-c684-4c04-831b-733151b3d2ad` |
| 02 | EA | `8f61db1d-6732-4dc0-b4a7-693157eabeaf` | BASE + `8f61db1d-6732-4dc0-b4a7-693157eabeaf` |
| 03 | CS | `a1a4912b-2ccb-4897-b18e-31d148643e83` | BASE + `a1a4912b-2ccb-4897-b18e-31d148643e83` |
| 04 | PO | `1746bc75-cc4a-4b37-b9a2-5f2b1a767d18` | BASE + `1746bc75-cc4a-4b37-b9a2-5f2b1a767d18` |
| 05 | WG | `309f3be5-b3c4-4389-8398-e29dae790149` | BASE + `309f3be5-b3c4-4389-8398-e29dae790149` |
| 06 | EN | `92e12d64-6f72-46c6-9d12-23cd5bb10c66` | BASE + `92e12d64-6f72-46c6-9d12-23cd5bb10c66` |
| 07 | AI | `87f6e90b-69e6-424f-9272-2759d7214e87` | BASE + `87f6e90b-69e6-424f-9272-2759d7214e87` |
| 08 | LA | `eb0c53bb-36c2-47d6-a326-9fff5ede24cf` | BASE + `eb0c53bb-36c2-47d6-a326-9fff5ede24cf` |
| 09 | NB | `86b30917-1c35-4138-909c-cdc3ad5a1ddb` | BASE + `86b30917-1c35-4138-909c-cdc3ad5a1ddb` |
| 10 | CE | `d5848575-c133-476a-8ef4-5111043b86e1` | BASE + `d5848575-c133-476a-8ef4-5111043b86e1` |
| 11 | CT | `5e404359-dced-43b8-99c3-b6a91a1a48c2` | BASE + `5e404359-dced-43b8-99c3-b6a91a1a48c2` |
| 12 | ND | `4af35ae8-4026-4bfb-bc78-b67146030467` | BASE + `4af35ae8-4026-4bfb-bc78-b67146030467` |
| 13 | QM | `bc4d15d3-f474-45a8-8e2b-826afd59788a` | BASE + `bc4d15d3-f474-45a8-8e2b-826afd59788a` |
| 14 | AU | `6f7335fe-78af-472a-8bbc-c2505171e540` | BASE + `6f7335fe-78af-472a-8bbc-c2505171e540` |
| 15 | CA | `aae628c4-6c98-43be-b574-41efb137a659` | BASE + `aae628c4-6c98-43be-b574-41efb137a659` |
| 16 | AN | `a27f6caa-068d-4261-865a-e18174bbd69e` | BASE + `a27f6caa-068d-4261-865a-e18174bbd69e` |
| 17 | VF | `5b1834ff-b8c3-48bf-aabc-ffdc5c668a7c` | BASE + `5b1834ff-b8c3-48bf-aabc-ffdc5c668a7c` |
| 18 | QA | `7f74baea-2dbe-480a-8d43-65fc56639a60` | BASE + `7f74baea-2dbe-480a-8d43-65fc56639a60` |
| 19 | IB | `3aba9f0a-ae2c-4b8d-9644-98fc12edaddc` | BASE + `3aba9f0a-ae2c-4b8d-9644-98fc12edaddc` |

---

## 6. SNIPPETS RELEVANCE AI

Total: **52 snippets operacionais**. Sintaxe: `{{snippets.nome_do_snippet}}`

### Configuração do Projecto
| Nome | Valor |
|------|-------|
| project_name | TranspersonalStudio |
| dev_phase | pre-producao |
| ue5_version | 5.7 |
| target_fps_pc | 60 |
| target_fps_console | 30 |
| frame_budget_ms | 16.6 |
| world_size_km2 | 200 |
| max_npcs_simultaneous | 10000 |
| max_draw_calls | 3000 |
| vram_budget_gb | 8 |
| ue5_remote_control_url | http://localhost:30010 |

### Webhooks de Agentes (Relevance AI)
| Nome do Snippet | Agente |
|----------------|--------|
| webhook_studio_director | Studio Director (#01) |
| webhook_engine_architect | Engine Architect (#02) |
| webhook_core_systems | Core Systems Programmer (#03) |
| webhook_performance_optimizer | Performance Optimizer (#04) |
| webhook_world_generator | Procedural World Generator (#05) |
| webhook_environment_artist | Environment Artist (#06) |
| webhook_architecture_interior | Architecture & Interior (#07) |
| webhook_lighting_atmosphere | Lighting & Atmosphere (#08) |
| webhook_npc_behavior | NPC Behavior (#09) |
| webhook_combat_enemy_ai | Combat & Enemy AI (#10) |
| webhook_crowd_traffic | Crowd & Traffic (#11) |
| webhook_narrative_dialogue | Narrative & Dialogue (#12) |
| webhook_quest_mission | Quest & Mission Designer (#13) |
| webhook_audio_agent | Audio Agent (#14) |
| webhook_character_artist | Character Artist (#15) |
| webhook_animation_agent | Animation Agent (#16) |
| webhook_vfx_agent | VFX Agent (#17) |
| webhook_qa_testing | QA & Testing (#18) |
| webhook_integration_build | Integration & Build (#19) |
| webhook_notificacao_aprovacao | Make.com — notificações de aprovação |

### Google Drive
| Nome | Pasta | ID |
|------|-------|----|
| google_drive_root | /Studio/ | 1HozrwhoyVFQDUtQ4DzCohogyeIvC6dg6 |
| google_drive_ai_folder | /Studio/AI/ | 1zqJ5ycp1-lzEs_buSxkztFKRxw6dS8T1 |
| google_drive_animation_folder | /Studio/Animation/ | 1xKOzzLlPJ6PtSyOB9lKqf5slOyg8TTwe |
| google_drive_art_folder | /Studio/Art/ | 1kuif2OpF2oV7Zy0aXMyMNYglR0bf5nFm |
| google_drive_audio_folder | /Studio/Audio/ | 1-nY0sSXziXbO7bkD84ONTCl_oeC6XQ3Z |
| google_drive_builds_folder | /Studio/Builds/ | 1pEaz2vXH-1sNCHD2s-9bNnNz5iy3lENM |
| google_drive_characters_folder | /Studio/Characters/ | 1aMxwZhmvDXQsqHm2LNWFyHAZ-CmSCAhf |
| google_drive_engine_folder | /Studio/Engine/ | 1pdOeK756pMwq0MPSH_-bUZUjL8nZ_IR |
| google_drive_narrative_folder | /Studio/Narrative/ | 1dklK1mOKsxniiGlDirPmH9xxlYYF2_JZ |
| google_drive_qa_folder | /Studio/QA/ | 1Aml7qCGwf_n7cq32yiB3Ln99eCJP4jql |
| google_drive_quests_folder | /Studio/Quests/ | 1xluKp6cVq1dRth9sfA7GmyxYLlnK-R3H |
| google_drive_reports_folder | /Studio/Reports/ | 1LQRJEsEkuF3agYzC8KYMnn-EfDO7knEE |
| google_drive_vfx_folder | /Studio/VFX/ | 17gqmT_5a3yukoYM-dZ0hfgOhjJsy1Qyx |
| google_drive_world_folder | /Studio/World/ | 1wqRoNorqd4rzPDyLFOoetWGbliPEjDdT |

### Notion e APIs Externas
| Nome | Descrição | Valor/ID |
|------|-----------|---------|
| notion_npc_database_id | Base de dados NPCs | 3380f880d0c48059838d000cf04b65b3 |
| notion_quest_database_id | Base de dados Quests | 3380f880d0c4804386e5e48e14b627e9 |
| notion_characters_database_id | Base de dados Characters | 3380f880d0c480f69c04f9f287865983 |
| notion_bugs_database_id | Base de dados Bugs | 3380f880d0c480758be2e8200774755c |
| elevenlabs_api_key | ElevenLabs — vozes | f3a95a37bab1aa180341a5ef3a9ca911b6429da1b57cc03fd7293c945d8ec01a |
| freesound_api_key | Freesound — Client Secret | YPhQFmclZSMY4sppmhU3krk0Bx8dGvKP9wRSBD2b |
| freesound_client_id | Freesound — Client ID | 34jouzAlHvOKt6gWNNNR |

---

## 7. SISTEMA DE APROVAÇÃO TELEGRAM

### Como funciona
1. Agente pede aprovação → chama webhook Make.com (`webhook_notificacao_aprovacao`) com payload JSON
2. Make.com guarda dados no Data Store `agent_approvals` e envia notificação Telegram + Email ao Hugo
3. Hugo responde no Telegram: `EA aprovado` ou `EA rejeitado`
4. Make.com lê o Data Store pelo código do agente, recupera `webhook_url` e `conversation_id`
5. Make.com chama o webhook do agente directamente — o agente recebe e continua

### Payload que os agentes enviam
```json
{
  "agente": "Engine Architect",
  "codigo": "EA",
  "assunto": "...",
  "mensagem": "...",
  "link_agente": "https://app.relevanceai.com/...",
  "webhook_url": "https://api-d7b62b...",
  "conversation_id": "{{CONVERSATION_ID}}"
}
```

### Data Store: agent_approvals
| Campo | Tipo | Descrição |
|-------|------|-----------|
| key (primary) | Text | Código do agente (ex: EA, SD, CS) |
| codigo | Text | Código do agente |
| webhook_url | Text | URL do webhook do agente no Relevance AI |
| conversation_id | Text | ID da conversa activa do agente |

### Cenário 00_notificacoes_aprovacao
- Módulo 1: Webhooks (Custom webhook) — recebe payload do agente
- Módulo 4: Data store (Add/replace a record) — guarda codigo, webhook_url, conversation_id
- Módulo 2: Telegram Bot — envia notificação
- Módulo 3: Gmail — envia email

### Cenário 00_resposta_telegram_agentes
- Módulo 1: Telegram Bot (Watch Updates) — detecta resposta do Hugo
- Módulo 24: Data store (Get a record) — key = substring dos primeiros 2 caracteres
- Router: 19 rotas, uma por agente
- Módulos HTTP: chama webhook_url directamente com a resposta

---

## 8. MAKE.COM — ORQUESTRAÇÃO

**Estado actual:** Make.com está a ser substituído pelo n8n como orquestrador principal. Os 21 cenários continuam activos como backup mas a nova arquitectura usa n8n.

### 19 cenários de pipeline (backup)
```
01_SD → 02_EA → 03_CS → 04_PO → 05_WG → 06_EN → 07_AI → 08_LA
→ 09_NB → 10_CE → 11_CT → 12_ND → 13_QM → 14_AU → 15_CA
→ 16_AN → 17_VF → 18_QA → 19_IB → 01_SD
```

### 2 cenários de aprovação (mantêm-se activos)
- `00_notificacoes_aprovacao` — notifica Hugo quando agente pede aprovação
- `00_resposta_telegram_agentes` — roteia resposta do Hugo para o agente correcto

---

## 9. N8N — NOVO ORQUESTRADOR CENTRAL

### Estado
- ✅ Docker instalado no servidor Hetzner (versão 29.3.1)
- ✅ n8n a correr em Docker com restart always
- ✅ Acessível em `http://88.99.188.76:5678`
- ✅ Workflow "Transpersonal Game Studio — Orquestrador Central" publicado

### Comando Docker (para referência)
```bash
docker run -d --name n8n --restart always -p 5678:5678 \
  -e N8N_SECURE_COOKIE=false \
  -v n8n_data:/home/node/.n8n \
  docker.n8n.io/n8nio/n8n
```

### Workflow — nós e função
| Nó | Função |
|----|--------|
| Agente Concluído (Webhook POST /agente-concluido) | Recebe notificação de agente concluído |
| Calcular Próximo Agente (Code) | Determina próximo agente na cadeia, gera ciclo_id único com timestamp |
| Tem Próximo? (If) | Verifica se há próximo agente ou se ciclo está completo |
| Chamar Próximo Agente (HTTP Request) | Chama webhook do próximo agente com retry 3x |
| Verificar Resultado (Code) | Regista sucesso ou erro |
| Ciclo Completo (Code) | Regista quando agente #19 termina |
| Iniciar Ciclo Manualmente (Webhook POST /iniciar-ciclo) | Permite iniciar ciclo manualmente |
| Preparar Início (Code) | Prepara payload para agente inicial |
| Chamar Agente Inicial (HTTP Request) | Chama agente com retry 3x |

### Payload que os agentes devem enviar ao n8n
```json
{
  "agent_id": "01",
  "ciclo_id": "CICLO_001_ARQUITECTURA_BASE",
  "status": "concluido",
  "outputs": "resumo opcional dos outputs produzidos"
}
```

### Para iniciar um ciclo manualmente
```bash
curl -X POST http://88.99.188.76:5678/webhook/iniciar-ciclo \
  -H "Content-Type: application/json" \
  -d '{
    "ciclo_id": "CICLO_002",
    "agente_id": "01",
    "mensagem": "Iniciar novo ciclo de produção"
  }'
```

---

## 10. GOOGLE DRIVE

### Estrutura de pastas
```
/Studio/
├── AI/          → 1zqJ5ycp1-lzEs_buSxkztFKRxw6dS8T1
├── Animation/   → 1xKOzzLlPJ6PtSyOB9lKqf5slOyg8TTwe
├── Art/         → 1kuif2OpF2oV7Zy0aXMyMNYglR0bf5nFm
├── Audio/       → 1-nY0sSXziXbO7bkD84ONTCl_oeC6XQ3Z
├── Builds/      → 1pEaz2vXH-1sNCHD2s-9bNnNz5iy3lENM
├── Characters/  → 1aMxwZhmvDXQsqHm2LNWFyHAZ-CmSCAhf
├── Engine/      → 1pdOeK756pMwq0MPSH_-bUZUjL8nZ_IR
├── Narrative/   → 1dklK1mOKsxniiGlDirPmH9xxlYYF2_JZ
├── QA/          → 1Aml7qCGwf_n7cq32yiB3Ln99eCJP4jql
├── Quests/      → 1xluKp6cVq1dRth9sfA7GmyxYLlnK-R3H
├── Reports/     → 1LQRJEsEkuF3agYzC8KYMnn-EfDO7knEE
├── VFX/         → 17gqmT_5a3yukoYM-dZ0hfgOhjJsy1Qyx
└── World/       → 1wqRoNorqd4rzPDyLFOoetWGbliPEjDdT
```

---

## 11. NOTION

### Bases de dados
| Base | ID |
|------|----|
| NPCs | 3380f880d0c48059838d000cf04b65b3 |
| Quests | 3380f880d0c4804386e5e48e14b627e9 |
| Characters | 3380f880d0c480f69c04f9f287865983 |
| Bugs | 3380f880d0c480758be2e8200774755c |

---

## 12. GITHUB

| Campo | Valor |
|-------|-------|
| Conta | sabuginni |
| Repositório | transpersonal_game_studio |
| Branch | main |
| Pasta local | `C:\Unreal Projects\transpersonal_game_studio_remote\transpersonal_game_studio` |
| Deploy | GitHub Actions → SSH → Hetzner (credenciais em Secrets do repo) |

---

## 13. UE5

| Campo | Valor |
|-------|-------|
| Projecto | TranspersonalStudio |
| Pasta | `C:\Unreal Projects\TranspersonalStudio` |
| Versão UE5 | 5.7 |
| Modo | C++ |
| Remote Control API | `http://localhost:30010/remote/info` |
| Visual Studio | 2022 com workloads C++ e Game Development |

---

## 14. SERVIDOR HETZNER

| Campo | Valor |
|-------|-------|
| Nome | transpersonal-lms |
| IP | 88.99.188.76 |
| Tipo | CPX42 |
| Specs | 8 vCPU, 16GB RAM, 320GB SSD |
| OS | Ubuntu 24.04.4 LTS |
| Preço | €25.49/mês |
| Serviços a correr | Plataforma LMS (Manus), Docker, n8n |
| Acesso SSH | PuTTY → root@88.99.188.76 port 22 |
| Deploy LMS | GitHub Actions com chave SSH (secrets do repo) |

---

## 15. O QUE FOI FEITO NESTA SESSÃO

### Problemas detectados e corrigidos

**1. Duplicate ID nos triggers dos agentes**
- **Problema:** O campo "Map unique id" de todos os 19 agentes estava configurado com `ciclo_id`. Como o ciclo_id era fixo (ex: `CICLO_001_ARQUITECTURA_BASE`), o Relevance AI bloqueava chamadas repetidas como duplicadas — o agente recebia HTTP 200 mas não processava.
- **Solução parcial hoje:** Corrigido o Performance Optimizer (#04) — campo "Map unique id" deixado em branco.
- **Falta:** Corrigir os restantes 18 agentes (ver secção 17).

**2. Erro "positional operator" no Environment Artist**
- **Problema:** Agente falhou com erro interno do Relevance AI ao tentar usar File Output.
- **Causa provável:** Snippet `google_drive_art_folder` com problema de resolução.
- **Estado:** Por resolver.

**3. Cadeia linear frágil — arquitectura Make.com**
- **Problema:** Sistema sem retry, sem watchdog, sem recuperação automática de falhas.
- **Solução:** Decisão de migrar para n8n como orquestrador central.

### O que foi instalado/configurado hoje

**n8n no servidor Hetzner:**
- Docker instalado (versão 29.3.1)
- n8n a correr em Docker com `--restart always`
- Workflow "Transpersonal Game Studio — Orquestrador Central" criado e publicado
- Workflow tem 9 nós: 2 webhooks de entrada, lógica de encadeamento, retry automático 3x, detecção de ciclo completo

**Python no PC local (Hugo):**
- Python 3.15.0a6 instalado (pre-release — funcional)
- Bibliotecas instaladas: requests, flask, apscheduler
- Ficheiro `orchestrator.py` criado (não está em uso — substituído pelo n8n)

### Outputs produzidos pelos agentes hoje
- **Narrative & Dialogue Agent (#12):** Concluiu narrativa completa
  - Bible do Jogo v0.2: https://userdata-d7b62b.stack.tryrelevance.com/files/perm_public/6c4a174f-ef9e-401e-85c9-99516e9041d8.md
  - 20 Missões Principais: https://userdata-d7b62b.stack.tryrelevance.com/files/perm_public/7c1f0844-a52d-47db-a1bc-2a8a7aa9a97a.md
  - Sistemas Narrativos Complementares: https://userdata-d7b62b.stack.tryrelevance.com/files/perm_public/e8705186-ca29-4e70-8ee4-cb1076672b5a.md
  - Notificação Handoff: https://userdata-d7b62b.stack.tryrelevance.com/files/perm_public/336d2fcb-6e4a-4c8c-8d97-aa6df118b654.md
- **Performance Optimizer (#04):** A trabalhar (tarefa enviada manualmente após correcção do duplicate ID)
- **Studio Director (#01):** Reportou 5/5 agentes activos no CICLO_001

---

## 16. PROBLEMAS CONHECIDOS

| # | Problema | Causa | Impacto | Estado |
|---|----------|-------|---------|--------|
| 1 | 18 agentes ainda com duplicate ID no trigger | Campo "Map unique id" = ciclo_id | Agentes bloqueiam em chamadas repetidas | 🔴 Por corrigir |
| 2 | Agentes não notificam o n8n ao concluir | Goals ainda apontam para Make.com | n8n não orquestra ainda | 🔴 Por corrigir |
| 3 | Environment Artist com erro "positional operator" | File Output com snippet undefined | Agente não consegue guardar outputs | 🟡 Por investigar |
| 4 | Make.com como orquestrador ainda activo | Migração em curso | Dois sistemas a tentar orquestrar | 🟡 Transitório |
| 5 | Python 3.15.0a6 (pre-release) no PC local | Instalação incorrecta | Instável para produção | 🟢 Baixo impacto |

---

## 17. O QUE FALTA FAZER — POR ORDEM DE PRIORIDADE

### 🔴 CRÍTICO — Fazer na próxima sessão

**1. Corrigir duplicate ID nos 18 agentes restantes**

Para cada agente (#02 ao #19 excepto #04 que já foi corrigido):
- Relevance AI → agente → Build → Triggers → Edit trigger
- Campo **Map unique id**: apagar `ciclo_id` → deixar **em branco**
- Continue → Save → Publish

Agentes por corrigir (por ordem):
- [ ] #02 Engine Architect
- [ ] #03 Core Systems Programmer
- [ ] #05 Procedural World Generator
- [ ] #06 Environment Artist
- [ ] #07 Architecture & Interior
- [ ] #08 Lighting & Atmosphere
- [ ] #09 NPC Behavior
- [ ] #10 Combat & Enemy AI
- [ ] #11 Crowd & Traffic
- [ ] #12 Narrative & Dialogue
- [ ] #13 Quest & Mission Designer
- [ ] #14 Audio Agent
- [ ] #15 Character Artist
- [ ] #16 Animation Agent
- [ ] #17 VFX Agent
- [ ] #18 QA & Testing
- [ ] #19 Integration & Build
- [ ] #01 Studio Director

**2. Actualizar Goals dos 19 agentes para notificar o n8n**

Adicionar ao final do Goal de cada agente a seguinte instrução:

```
CONCLUIR TAREFA — NOTIFICAR ORQUESTRADOR:
Quando terminares a tua tarefa, chama imediatamente este webhook via HTTP POST:
URL: http://88.99.188.76:5678/webhook/agente-concluido
Payload:
{
  "agent_id": "[ID DO AGENTE ex: 01]",
  "ciclo_id": "[ciclo_id recebido no início]",
  "status": "concluido",
  "outputs": "resumo do que foi produzido"
}
```

**3. Enviar outputs do Narrative Agent ao Quest & Mission Designer**

O Narrative Agent concluiu mas o Quest & Mission Designer ainda não recebeu os 4 documentos. Enviar payload completo ao webhook do QM:
```
https://api-d7b62b.stack.tryrelevance.com/latest/agents/hooks/custom-trigger/a6b3c7c7-3afe-4a91-969c-560f70a2546d/bc4d15d3-f474-45a8-8e2b-826afd59788a
```

### 🟡 IMPORTANTE — Fazer esta semana

**4. Desactivar cenários Make.com de pipeline (19 cenários)**

Após os agentes estarem a notificar o n8n, desactivar os 19 cenários de pipeline do Make.com (manter apenas os 2 de aprovação Telegram).

**5. Investigar e resolver erro do Environment Artist**

Verificar se snippet `google_drive_art_folder` está correcto. Tentar Retry na task com erro.

**6. Testar ciclo completo end-to-end com n8n**

Enviar tarefa ao Studio Director → verificar se a cadeia propaga automaticamente até ao final via n8n.

### 🟢 A SEGUIR

**7. Ligar ElevenLabs ao Audio Agent**
- API key já configurada no snippet `elevenlabs_api_key`
- Adicionar HTTP Request tool ao Audio Agent para chamar a API

**8. Activar Remote Control UE5 nos agentes**
- Engine Architect, Core Systems, Integration & Build precisam de comunicar com UE5
- UE5 tem de estar a correr com Remote Control activo em `http://localhost:30010`

**9. Configurar Notion com os agentes**
- NPC Behavior, Quest Designer, Character Artist, QA precisam de escrever no Notion
- IDs das bases de dados já estão nos snippets

**10. Integrar Freesound no Audio Agent**
- Client ID e Secret já estão nos snippets
- Adicionar lógica de pesquisa e download de SFX

---

## NOTAS IMPORTANTES PARA FUTURAS SESSÕES

### Regras que nunca devem ser violadas
1. **ciclo_id sempre com timestamp** — nunca usar ciclo_id fixo para evitar duplicate ID
2. **Map unique id sempre em branco** nos triggers dos agentes Relevance AI
3. **Agentes notificam o n8n ao concluir** — não chamam directamente o próximo agente
4. **Make.com mantém-se apenas para aprovações Telegram** — não para pipeline de agentes
5. **Long-Term Memory** — activar apenas nos agentes que precisam (ver tabela na secção 4)
6. **Snippets** — sintaxe `{{snippets.nome}}` nos Goals dos agentes

### Lições aprendidas
- O campo "Map unique id" com valor fixo bloqueia agentes silenciosamente — sempre deixar em branco
- O Relevance AI retorna HTTP 200 mesmo quando bloqueia por duplicate ID — não é sinal de sucesso
- O n8n em Docker com `--restart always` garante que o orquestrador reinicia sozinho após reboot do servidor
- O Manus acede ao servidor Hetzner via GitHub Actions (chave SSH em Secrets) — reset de password root não interfere
- Para colar no PuTTY usar botão direito do rato (não Ctrl+V)

---

*Transpersonal Game Studio · Hugo & Miguel Martins*  
*Versão 5.0 · 5 de Abril de 2026*  
*Documento de estado completo para continuidade entre sessões*
