# TRANSPERSONAL GAME STUDIO — SNIPPETS KB
## Base de Conhecimento: Todos os Snippets do Relevance AI
## Versão 1.0 · 4 de Abril de 2026

---

## COMO USAR SNIPPETS NOS AGENTES

Sintaxe nos prompts dos agentes: `{{snippets.nome_do_snippet}}`

Exemplo: `{{snippets.webhook_studio_director}}` → substitui pelo URL do webhook do Studio Director

Localização no Relevance AI: Projecto Transpersonal Game Studio → More → Snippets

---

## SNIPPETS DE CONFIGURAÇÃO DO PROJECTO

| Nome | Valor | new_snippet_# |
|------|-------|---------------|
| `project_name` | TranspersonalStudio | new_snippet |
| `dev_phase` | pre-producao | new_snippet_1 |
| `ue5_version` | 5.7 | new_snippet_2 |
| `target_fps_pc` | 60 | new_snippet_3 |
| `target_fps_console` | 30 | new_snippet_4 |
| `frame_budget_ms` | 16.6 | new_snippet_5 |
| `world_size_km2` | 200 | new_snippet_6 |
| `max_npcs_simultaneous` | 10000 | new_snippet_7 |
| `max_draw_calls` | 3000 | new_snippet_8 |
| `vram_budget_gb` | 8 | new_snippet_9 |
| `ue5_remote_control_url` | http://localhost:30010 | new_snippet_10 |

---

## SNIPPETS DE WEBHOOKS DE AGENTES

### Webhooks para o Studio Director chamar outros agentes

| Nome | Agente | Agent ID | new_snippet_# |
|------|--------|----------|---------------|
| `webhook_studio_director` | Studio Director (#01) | 1af5723e-c684-4c04-831b-733151b3d2ad | new_snippet_11 |
| `webhook_engine_architect` | Engine Architect (#02) | 8f61db1d-6732-4dc0-b4a7-693157eabeaf | new_snippet_34 |
| `webhook_core_systems` | Core Systems Programmer (#03) | a1a4912b-2ccb-4897-b18e-31d148643e83 | new_snippet_13 |
| `webhook_performance_optimizer` | Performance Optimizer (#04) | 1746bc75-cc4a-4b37-b9a2-5f2b1a767d18 | — |
| `webhook_world_generator` | Procedural World Generator (#05) | 309f3be5-b3c4-4389-8398-e29dae790149 | — |
| `webhook_environment_artist` | Environment Artist (#06) | 92e12d64-6f72-46c6-9d12-23cd5bb10c66 | — |
| `webhook_architecture_interior` | Architecture & Interior (#07) | 87f6e90b-69e6-424f-9272-2759d7214e87 | — |
| `webhook_lighting_atmosphere` | Lighting & Atmosphere (#08) | eb0c53bb-36c2-47d6-a326-9fff5ede24cf | — |
| `webhook_npc_behavior` | NPC Behavior (#09) | 86b30917-1c35-4138-909c-cdc3ad5a1ddb | — |
| `webhook_combat_enemy_ai` | Combat & Enemy AI (#10) | d5848575-c133-476a-8ef4-5111043b86e1 | — |
| `webhook_crowd_traffic` | Crowd & Traffic (#11) | 5e404359-dced-43b8-99c3-b6a91a1a48c2 | — |
| `webhook_narrative_dialogue` | Narrative & Dialogue (#12) | 4af35ae8-4026-4bfb-bc78-b67146030467 | — |
| `webhook_quest_mission` | Quest & Mission Designer (#13) | bc4d15d3-f474-45a8-8e2b-826afd59788a | — |
| `webhook_audio_agent` | Audio Agent (#14) | 6f7335fe-78af-472a-8bbc-c2505171e540 | — |
| `webhook_character_artist` | Character Artist (#15) | aae628c4-6c98-43be-b574-41efb137a659 | — |
| `webhook_animation_agent` | Animation Agent (#16) | a27f6caa-068d-4261-865a-e18174bbd69e | — |
| `webhook_vfx_agent` | VFX Agent (#17) | 5b1834ff-b8c3-48bf-aabc-ffdc5c668a7c | — |
| `webhook_qa_testing` | QA & Testing (#18) | 7f74baea-2dbe-480a-8d43-65fc56639a60 | — |
| `webhook_integration_build` | Integration & Build (#19) | 3aba9f0a-ae2c-4b8d-9644-98fc12edaddc | — |

### URL base de todos os webhooks de agentes
```
https://api-d7b62b.stack.tryrelevance.com/latest/agents/hooks/custom-trigger/a6b3c7c7-3afe-4a91-969c-560f70a2546d/[AGENT_ID]
```

### Webhook de notificações (Make.com)

| Nome | Valor | new_snippet_# |
|------|-------|---------------|
| `webhook_notificacao_aprovacao` | https://hook.eu2.make.com/d1nfe64bnhik5qtwo22mpb9yqy67k6ny | new_snippet_35 |

---

## SNIPPETS DO GOOGLE DRIVE

### Pasta raiz

| Nome | Descrição | ID da Pasta | new_snippet_# |
|------|-----------|-------------|---------------|
| `google_drive_root` | Pasta raiz /Studio/ | 1HozrwhoyVFQDUtQ4DzCohogyeIvC6dg6 | new_snippet_12 |

### Subpastas

| Nome | Pasta | ID | new_snippet_# |
|------|-------|----|---------------|
| `google_drive_ai_folder` | /Studio/AI/ | 1zqJ5ycp1-lzEs_buSxkztFKRxw6dS8T1 | new_snippet_14 |
| `google_drive_animation_folder` | /Studio/Animation/ | 1xKOzzLlPJ6PtSyOB9lKqf5slOyg8TTwe | new_snippet_15 |
| `google_drive_art_folder` | /Studio/Art/ | 1kuif2OpF2oV7Zy0aXMyMNYglR0bf5nFm | new_snippet_16 |
| `google_drive_audio_folder` | /Studio/Audio/ | 1-nY0sSXziXbO7bkD84ONTCl_oeC6XQ3Z | new_snippet_17 |
| `google_drive_builds_folder` | /Studio/Builds/ | 1pEaz2vXH-1sNCHD2s-9bNnNz5iy3lENM | new_snippet_18 |
| `google_drive_characters_folder` | /Studio/Characters/ | 1aMxwZhmvDXQsqHm2LNWFyHAZ-CmSCAhf | new_snippet_19 |
| `google_drive_engine_folder` | /Studio/Engine/ | 1pdOeK756pMwq0MPSH_-bUZUjL8nZ_IR | new_snippet_20 |
| `google_drive_narrative_folder` | /Studio/Narrative/ | 1dklK1mOKsxniiGlDirPmH9xxlYYF2_JZ | new_snippet_21 |
| `google_drive_qa_folder` | /Studio/QA/ | 1Aml7qCGwf_n7cq32yiB3Ln99eCJP4jql | new_snippet_22 |
| `google_drive_quests_folder` | /Studio/Quests/ | 1xluKp6cVq1dRth9sfA7GmyxYLlnK-R3H | new_snippet_23 |
| `google_drive_reports_folder` | /Studio/Reports/ | 1LQRJEsEkuF3agYzC8KYMnn-EfDO7knEE | new_snippet_24 |
| `google_drive_vfx_folder` | /Studio/VFX/ | 17gqmT_5a3yukoYM-dZ0hfgOhjJsy1Qyx | new_snippet_25 |
| `google_drive_world_folder` | /Studio/World/ | 1wqRoNorqd4rzPDyLFOoetWGbliPEjDdT | new_snippet_26 |

---

## SNIPPETS DO NOTION

| Nome | Descrição | ID da Base de Dados | new_snippet_# |
|------|-----------|---------------------|---------------|
| `notion_npc_database_id` | Base de dados NPCs | 3380f880d0c48059838d000cf04b65b3 | new_snippet_27 |
| `notion_quest_database_id` | Base de dados Quests | 3380f880d0c4804386e5e48e14b627e9 | new_snippet_28 |
| `notion_characters_database_id` | Base de dados Characters | 3380f880d0c480f69c04f9f287865983 | new_snippet_29 |
| `notion_bugs_database_id` | Base de dados Bugs | 3380f880d0c480758be2e8200774755c | new_snippet_30 |

---

## SNIPPETS DE APIs EXTERNAS

| Nome | Serviço | new_snippet_# |
|------|---------|---------------|
| `elevenlabs_api_key` | ElevenLabs — geração de vozes | new_snippet_31 |
| `freesound_api_key` | Freesound — Client Secret/API key | new_snippet_32 |
| `freesound_client_id` | Freesound — Client ID | new_snippet_33 |

---

## CADEIA DE COMUNICAÇÃO ENTRE AGENTES

A cadeia segue esta ordem (confirmada nos cenários Make.com):

```
01_SD → 02_EA → 03_CS → 04_PO → 05_WG → 06_EN → 07_AI → 08_LA
→ 09_NB → 10_CE → 11_CT → 12_ND → 13_QM → 14_AU → 15_CA
→ 16_AN → 17_VF → 18_QA → 19_IB → 01_SD (ciclo completo)
```

Cada agente, ao terminar uma tarefa, chama o webhook do **agente seguinte** na cadeia usando `{{snippets.webhook_[próximo_agente]}}`.

---

## TABELA COMPLETA DE AGENTES — CÓDIGOS E IDs

| Código | Agente | Agent ID |
|--------|--------|----------|
| SD | Studio Director (#01) | 1af5723e-c684-4c04-831b-733151b3d2ad |
| EA | Engine Architect (#02) | 1593baa3-c209-4342-96c4-ad538b6859e3 |
| CS | Core Systems Programmer (#03) | a1a4912b-2ccb-4897-b18e-31d148643e83 |
| PO | Performance Optimizer (#04) | 1746bc75-cc4a-4b37-b9a2-5f2b1a767d18 |
| WG | Procedural World Generator (#05) | 309f3be5-b3c4-4389-8398-e29dae790149 |
| EN | Environment Artist (#06) | 92e12d64-6f72-46c6-9d12-23cd5bb10c66 |
| AI | Architecture & Interior (#07) | 87f6e90b-69e6-424f-9272-2759d7214e87 |
| LA | Lighting & Atmosphere (#08) | eb0c53bb-36c2-47d6-a326-9fff5ede24cf |
| NB | NPC Behavior (#09) | 86b30917-1c35-4138-909c-cdc3ad5a1ddb |
| CE | Combat & Enemy AI (#10) | d5848575-c133-476a-8ef4-5111043b86e1 |
| CT | Crowd & Traffic (#11) | 5e404359-dced-43b8-99c3-b6a91a1a48c2 |
| ND | Narrative & Dialogue (#12) | 4af35ae8-4026-4bfb-bc78-b67146030467 |
| QM | Quest & Mission Designer (#13) | bc4d15d3-f474-45a8-8e2b-826afd59788a |
| AU | Audio Agent (#14) | 6f7335fe-78af-472a-8bbc-c2505171e540 |
| CA | Character Artist (#15) | aae628c4-6c98-43be-b574-41efb137a659 |
| AN | Animation Agent (#16) | a27f6caa-068d-4261-865a-e18174bbd69e |
| VF | VFX Agent (#17) | 5b1834ff-b8c3-48bf-aabc-ffdc5c668a7c |
| QA | QA & Testing (#18) | 7f74baea-2dbe-480a-8d43-65fc56639a60 |
| IB | Integration & Build (#19) | 3aba9f0a-ae2c-4b8d-9644-98fc12edaddc |

---

## ESTADO DOS SNIPPETS

| Categoria | Total | Estado |
|-----------|-------|--------|
| Configuração do projecto | 11 | ✅ Todos criados e com valores |
| Webhooks de agentes | 19 + 1 notificação | ✅ Todos criados e com valores |
| Google Drive | 14 (raiz + 13 subpastas) | ✅ Todos criados e com valores |
| Notion | 4 | ✅ Todos criados e com valores |
| APIs externas | 3 | ✅ Todos criados e com valores |
| **TOTAL** | **52** | ✅ **100% operacionais** |

---

*Transpersonal Game Studio · Hugo & Miguel Martins · 4 de Abril de 2026*
*KB gerada após sessão de configuração completa de snippets*
