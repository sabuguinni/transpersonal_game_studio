# CONFIGURAÇÃO OPERACIONAL — 19 AGENTES
## Transpersonal Game Studio — Miguel Martins
## Tools · Triggers · Memory · Variables
## Versão 1.0 · Março 2026

---

## COMO LER ESTE DOCUMENTO

Este documento cobre os quatro campos operacionais de cada agente no Relevance AI:

- **TOOLS** — ferramentas a activar para o agente executar tarefas
- **TRIGGER** — o que activa o agente (webhook + payload JSON esperado)
- **MEMORY** — o que o agente deve recordar entre sessões
- **VARIABLES** — variáveis do projecto que o agente usa

No final do documento está a secção **VARIABLES GLOBAIS** — variáveis partilhadas por todos os agentes que só precisam de ser definidas uma vez.

---

## LEGENDA DE TOOLS

As ferramentas disponíveis no Relevance AI relevantes para este projecto:

| Tool | Função |
|------|--------|
| **LLM** | Geração de texto, código, análise — o cérebro do agente |
| **HTTP Request** | Chamadas a APIs externas (Make.com, UE5 Remote Control, GitHub) |
| **Knowledge Retrieval** | Acesso à KB do próprio agente |
| **Code Interpreter** | Execução de Python para análise de dados e geração de ficheiros |
| **File Output** | Geração de ficheiros Markdown, JSON, texto |
| **Web Search** | Pesquisa na web quando o agente precisa de informação externa |
| **Long Term Memory** | Persistência de informação entre sessões (não é a memória de conversa) |

---
---

# AGENTE #01 — STUDIO DIRECTOR

## TOOLS

| Tool | Activar | Configuração |
|------|---------|--------------|
| LLM | ✅ Obrigatório | Modelo: Claude Sonnet ou GPT-4o. Temperature: 0.7 (criativo mas coerente) |
| HTTP Request | ✅ Obrigatório | Sem configuração prévia — o agente define o URL em cada chamada |
| Knowledge Retrieval | ✅ Obrigatório | Ligar à KB deste agente |
| Long Term Memory | ✅ Obrigatório | Ver secção Memory |
| File Output | ✅ Recomendado | Para gerar relatórios de ciclo em Markdown |

## TRIGGER

**Tipo:** Webhook (POST)
**Quem chama:** O Miguel directamente, ou o Make.com em resposta a um evento

**Payload esperado:**
```json
{
  "origem": "miguel",
  "instrucao": "texto da instrução criativa do Miguel",
  "contexto_adicional": "opcional — ficheiros, referências, aprovações anteriores",
  "prioridade": "normal | urgente"
}
```

**Payload de aprovação** (quando o Miguel aprova ou rejeita um ciclo):
```json
{
  "origem": "miguel",
  "tipo": "aprovacao",
  "ciclo_id": "identificador do ciclo",
  "decisao": "aprovado | rejeitado | aprovado_com_alteracao",
  "nota": "texto opcional do Miguel"
}
```

## MEMORY

**Long Term Memory: ACTIVAR**

O Studio Director é o agente com maior necessidade de memória persistente. Deve recordar:

```
ESTADO DO PROJECTO:
- Nome do jogo: [a preencher]
- Versão da Bible do Jogo: [número e data]
- Fase actual do desenvolvimento: [pré-produção / produção / polish]
- Última instrução aprovada pelo Miguel: [texto]
- Ciclos em curso: [lista de ciclos activos com estado]
- Decisões críticas tomadas: [log de decisões irreversíveis]

ESTADO DOS AGENTES:
- Agentes activos: [lista]
- Agentes com tarefas em curso: [lista com descrição da tarefa]
- Agentes bloqueados: [lista com razão do bloqueio]

PREFERÊNCIAS DO MIGUEL:
- Coisas que o Miguel aprovou entusiasticamente: [lista]
- Coisas que o Miguel rejeitou: [lista com razão]
- Tom de comunicação preferido: [notas]
```

## VARIABLES USADAS

`{{project_name}}` `{{bible_version}}` `{{dev_phase}}` `{{miguel_contact}}` `{{make_webhook_base}}`

---
---

# AGENTE #02 — ENGINE ARCHITECT

## TOOLS

| Tool | Activar | Configuração |
|------|---------|--------------|
| LLM | ✅ Obrigatório | Temperature: 0.3 (preciso e consistente) |
| HTTP Request | ✅ Obrigatório | Para distribuir arquitectura aos outros agentes via Make.com |
| Knowledge Retrieval | ✅ Obrigatório | Ligar à KB deste agente |
| File Output | ✅ Obrigatório | Para gerar documentos de arquitectura em Markdown |
| Code Interpreter | ✅ Recomendado | Para validar lógica de configuração UE5 |

## TRIGGER

**Tipo:** Webhook (POST)
**Quem chama:** Studio Director (início de projecto ou nova feature major)

**Payload esperado:**
```json
{
  "origem": "studio_director",
  "tipo": "nova_arquitectura | update_arquitectura",
  "conceito_jogo": "texto do conceito do jogo",
  "requisitos": {
    "dimensao_mundo_km2": 0,
    "npcs_simultaneos": 0,
    "plataformas_alvo": ["PC", "PS5"],
    "features_criticas": []
  },
  "ciclo_id": "identificador do ciclo"
}
```

## MEMORY

**Long Term Memory: ACTIVAR**

```
ARQUITECTURA ACTUAL:
- Versão do documento: [número]
- Sistemas activados: [lista: Lumen, Nanite, World Partition, Mass AI, PCG]
- Targets de performance definidos: [valores exactos por plataforma]
- Convenções de nomenclatura: [regras activas]
- Decisões irreversíveis tomadas: [log com data e razão]

CONFLITOS RESOLVIDOS:
- [data] Conflito entre [Agente A] e [Agente B] sobre [sistema]: resolvido com [solução]
```

## VARIABLES USADAS

`{{project_name}}` `{{ue5_version}}` `{{target_fps_pc}}` `{{target_fps_console}}` `{{world_size_km2}}` `{{make_webhook_base}}`

---
---

# AGENTE #03 — CORE SYSTEMS PROGRAMMER

## TOOLS

| Tool | Activar | Configuração |
|------|---------|--------------|
| LLM | ✅ Obrigatório | Temperature: 0.2 (código preciso) |
| Code Interpreter | ✅ Obrigatório | Para gerar e validar código C++ e Python |
| HTTP Request | ✅ Obrigatório | Para notificar Integration Agent de novo código |
| Knowledge Retrieval | ✅ Obrigatório | Ligar à KB deste agente |
| File Output | ✅ Obrigatório | Para gerar ficheiros .h, .cpp, .md de documentação |

## TRIGGER

**Tipo:** Webhook (POST)
**Quem chama:** Engine Architect (novo sistema a implementar) ou Studio Director

**Payload esperado:**
```json
{
  "origem": "engine_architect | studio_director",
  "sistema": "nome do sistema a implementar",
  "especificacao": "descrição técnica do comportamento esperado",
  "budget_ms": 0,
  "dependencias": [],
  "prioridade": "normal | critico",
  "ciclo_id": "identificador do ciclo"
}
```

## MEMORY

**Long Term Memory: NÃO OBRIGATÓRIO**
A arquitectura técnica está na KB. Não é necessário memória persistente — cada tarefa é auto-contida.

**Sessão Memory: ACTIVAR** (padrão do Relevance AI — já activo por defeito)

## VARIABLES USADAS

`{{project_name}}` `{{ue5_version}}` `{{github_repo}}` `{{coding_standards_url}}` `{{make_webhook_base}}`

---
---

# AGENTE #04 — PERFORMANCE & OPTIMIZER

## TOOLS

| Tool | Activar | Configuração |
|------|---------|--------------|
| LLM | ✅ Obrigatório | Temperature: 0.2 |
| Code Interpreter | ✅ Obrigatório | Para análise de relatórios de performance (CSV, JSON do Unreal Insights) |
| HTTP Request | ✅ Obrigatório | Para buscar relatórios e notificar agentes afectados |
| Knowledge Retrieval | ✅ Obrigatório | Ligar à KB deste agente |
| File Output | ✅ Obrigatório | Para gerar relatórios de optimização |

## TRIGGER

**Tipo:** Webhook (POST)
**Quem chama:** QA Agent (resultado de teste de performance), Make.com (schedule diário), ou Studio Director

**Payload esperado:**
```json
{
  "origem": "qa_agent | schedule | studio_director",
  "tipo": "analise_performance | alerta_critico",
  "zona": "nome da zona ou sistema com problema",
  "frame_time_actual_ms": 0,
  "plataforma": "PC | PS5 | Xbox",
  "relatorio_url": "URL do relatório Unreal Insights no Google Drive",
  "ciclo_id": "identificador do ciclo"
}
```

## MEMORY

**Long Term Memory: ACTIVAR**

```
HISTÓRICO DE PERFORMANCE:
- [data] Zona [nome]: [X]ms → [Y]ms após optimização [descrição]
- Sistemas com histórico de problemas recorrentes: [lista]
- Optimizações aplicadas e resultado: [log]

BUDGET ACTUAL POR SISTEMA:
- Rendering: [X]ms
- Physics: [X]ms
- AI: [X]ms
- Audio: [X]ms
```

## VARIABLES USADAS

`{{target_fps_pc}}` `{{target_fps_console}}` `{{frame_budget_ms}}` `{{google_drive_reports_folder}}` `{{make_webhook_base}}`

---
---

# AGENTE #05 — PROCEDURAL WORLD GENERATOR

## TOOLS

| Tool | Activar | Configuração |
|------|---------|--------------|
| LLM | ✅ Obrigatório | Temperature: 0.6 (criativo dentro de regras) |
| Code Interpreter | ✅ Obrigatório | Para gerar scripts Python UE5 de geração de landscape |
| HTTP Request | ✅ Obrigatório | Para chamar UE5 Remote Control API e notificar agentes dependentes |
| Knowledge Retrieval | ✅ Obrigatório | Ligar à KB deste agente |
| File Output | ✅ Obrigatório | Para gerar mapas de biomas JSON e scripts Python |

## TRIGGER

**Tipo:** Webhook (POST)
**Quem chama:** Studio Director (nova região a gerar)

**Payload esperado:**
```json
{
  "origem": "studio_director",
  "regiao": "nome da região",
  "descricao": "descrição criativa da região pelo Miguel",
  "area_km2": 0,
  "biomas_pretendidos": [],
  "clima": "descritivo",
  "ligacoes_a": ["regiões adjacentes se existirem"],
  "intencao_narrativa": "o que esta região significa na história",
  "ciclo_id": "identificador do ciclo"
}
```

## MEMORY

**Long Term Memory: ACTIVAR**

```
MAPA GLOBAL DO MUNDO:
- Regiões geradas: [lista com nome, área, biomas, estado: rascunho/aprovado]
- Rios e ligações hidrográficas: [mapa de dependências]
- Rede de estradas gerada: [nós e ligações principais]
- Convenções de nomenclatura geográfica: [regras]
```

## VARIABLES USADAS

`{{project_name}}` `{{world_size_km2}}` `{{ue5_remote_control_url}}` `{{google_drive_world_folder}}` `{{make_webhook_base}}`

---
---

# AGENTE #06 — ENVIRONMENT ARTIST

## TOOLS

| Tool | Activar | Configuração |
|------|---------|--------------|
| LLM | ✅ Obrigatório | Temperature: 0.6 |
| HTTP Request | ✅ Obrigatório | Para chamar UE5 Remote Control API |
| Knowledge Retrieval | ✅ Obrigatório | Ligar à KB deste agente |
| File Output | ✅ Obrigatório | Para gerar configs de Foliage Tool e Landscape Paint |

## TRIGGER

**Tipo:** Webhook (POST)
**Quem chama:** World Generator (terreno pronto) ou Studio Director

**Payload esperado:**
```json
{
  "origem": "world_generator | studio_director",
  "zona": "nome da zona",
  "bioma": "tipo de bioma",
  "narrativa_geografica": "texto da narrativa geográfica do World Generator",
  "intencao_narrativa": "o que aconteceu nesta zona",
  "referencia_visual": "zona do style guide a aplicar",
  "ciclo_id": "identificador do ciclo"
}
```

## MEMORY

**Long Term Memory: NÃO OBRIGATÓRIO**
O style guide está na KB. Cada zona é tratada de forma independente.

## VARIABLES USADAS

`{{ue5_remote_control_url}}` `{{fab_api_key}}` `{{google_drive_art_folder}}` `{{make_webhook_base}}`

---
---

# AGENTE #07 — ARCHITECTURE & INTERIOR AGENT

## TOOLS

| Tool | Activar | Configuração |
|------|---------|--------------|
| LLM | ✅ Obrigatório | Temperature: 0.5 |
| HTTP Request | ✅ Obrigatório | Para chamar UE5 Remote Control API |
| Knowledge Retrieval | ✅ Obrigatório | Ligar à KB deste agente |
| File Output | ✅ Obrigatório | Para gerar fichas históricas e listas de props em JSON |

## TRIGGER

**Tipo:** Webhook (POST)
**Quem chama:** World Generator (nova zona com estruturas), Studio Director, ou Quest Designer (estrutura específica para missão)

**Payload esperado:**
```json
{
  "origem": "world_generator | studio_director | quest_designer",
  "tipo_estrutura": "residencial | comercial | militar | religioso | ruina",
  "localizacao": "zona e coordenadas aproximadas",
  "nivel_economico": "pobre | medio | rico",
  "estado_conservacao": "intacto | degradado | ruina",
  "historia_pretendida": "o que aconteceu aqui",
  "uso_em_missao": "opcional — se esta estrutura suporta uma missão específica",
  "ciclo_id": "identificador do ciclo"
}
```

## MEMORY

**Long Term Memory: ACTIVAR**

```
CATÁLOGO DE ESTRUTURAS CRIADAS:
- [nome/id]: tipo, localização, estado, elementos interactivos, missões associadas
- Props em falta no catálogo (solicitados mas não disponíveis): [lista]
```

## VARIABLES USADAS

`{{ue5_remote_control_url}}` `{{fab_api_key}}` `{{google_drive_world_folder}}` `{{make_webhook_base}}`

---
---

# AGENTE #08 — LIGHTING & ATMOSPHERE AGENT

## TOOLS

| Tool | Activar | Configuração |
|------|---------|--------------|
| LLM | ✅ Obrigatório | Temperature: 0.4 |
| HTTP Request | ✅ Obrigatório | Para chamar UE5 Remote Control API |
| Knowledge Retrieval | ✅ Obrigatório | Ligar à KB deste agente |
| File Output | ✅ Obrigatório | Para gerar configs de iluminação em JSON |

## TRIGGER

**Tipo:** Webhook (POST)
**Quem chama:** World Generator ou Environment Artist (zona pronta), ou Make.com (schedule de ciclo dia/noite)

**Payload esperado:**
```json
{
  "origem": "world_generator | environment_artist | schedule",
  "zona": "nome da zona ou 'global'",
  "hora_do_dia": "dawn | morning | midday | afternoon | golden_hour | dusk | night | midnight",
  "condicao_meteorologica": "clear | overcast | rain | storm | fog | snow",
  "intencao_emocional": "descrição do mood pretendido",
  "evento_narrativo_activo": "opcional — missão ou evento que altera o mood",
  "ciclo_id": "identificador do ciclo"
}
```

## MEMORY

**Long Term Memory: ACTIVAR**

```
ESTADOS DE ILUMINAÇÃO APROVADOS:
- [zona] [hora] [condição]: aprovado em [data], parâmetros em [localização no Drive]
- Estados em revisão: [lista]

PREFERÊNCIAS VISUAIS DO MIGUEL:
- Estados que o Miguel aprovou entusiasticamente: [lista]
- Temperaturas de cor preferidas: [notas]
```

## VARIABLES USADAS

`{{ue5_remote_control_url}}` `{{google_drive_art_folder}}` `{{make_webhook_base}}`

---
---

# AGENTE #09 — NPC BEHAVIOR AGENT

## TOOLS

| Tool | Activar | Configuração |
|------|---------|--------------|
| LLM | ✅ Obrigatório | Temperature: 0.5 |
| HTTP Request | ✅ Obrigatório | Para chamar UE5 Remote Control API e notificar Crowd Agent |
| Knowledge Retrieval | ✅ Obrigatório | Ligar à KB deste agente |
| File Output | ✅ Obrigatório | Para gerar Behavior Trees em XML e perfis de NPC em JSON |
| Code Interpreter | ✅ Recomendado | Para gerar estruturas de Behavior Tree complexas |

## TRIGGER

**Tipo:** Webhook (POST)
**Quem chama:** World Generator (nova zona populada), Narrative Agent (evento narrativo), ou Studio Director

**Payload esperado:**
```json
{
  "origem": "world_generator | narrative_agent | studio_director",
  "zona": "nome da zona",
  "arquetipos_necessarios": ["pescador", "guarda", "comerciante"],
  "densidade_populacao": "baixa | media | alta",
  "eventos_narrativos_activos": [],
  "reputacao_jogador_na_zona": "neutro | positivo | negativo",
  "ciclo_id": "identificador do ciclo"
}
```

**Payload de evento narrativo** (quando Narrative Agent altera o mundo):
```json
{
  "origem": "narrative_agent",
  "tipo": "evento_narrativo",
  "evento": "descrição do evento",
  "zonas_afectadas": [],
  "npcs_afectados": [],
  "alteracao_comportamento": "descrição da mudança esperada",
  "ciclo_id": "identificador do ciclo"
}
```

## MEMORY

**Long Term Memory: ACTIVAR — PRIORITÁRIO**

```
CATÁLOGO DE NPCs:
- [id_npc]: nome, arquétipo, zona, rotina, estado actual, reputação com jogador
- NPCs com memória de eventos específicos: [log de interacções]

EVENTOS MUNDIAIS QUE ALTERARAM COMPORTAMENTOS:
- [data_jogo] [evento]: NPCs afectados e como o comportamento mudou

ESTADO DA REPUTAÇÃO POR ZONA:
- [zona]: reputação actual do jogador, eventos que a determinaram
```

## VARIABLES USADAS

`{{ue5_remote_control_url}}` `{{google_drive_ai_folder}}` `{{notion_npc_database_id}}` `{{make_webhook_base}}`

---
---

# AGENTE #10 — COMBAT & ENEMY AI AGENT

## TOOLS

| Tool | Activar | Configuração |
|------|---------|--------------|
| LLM | ✅ Obrigatório | Temperature: 0.3 |
| HTTP Request | ✅ Obrigatório | Para chamar UE5 Remote Control API |
| Knowledge Retrieval | ✅ Obrigatório | Ligar à KB deste agente |
| File Output | ✅ Obrigatório | Para gerar Behavior Trees de combate em XML |
| Code Interpreter | ✅ Recomendado | Para gerar EQS queries complexas |

## TRIGGER

**Tipo:** Webhook (POST)
**Quem chama:** Quest Designer (novo encontro de combate), Studio Director

**Payload esperado:**
```json
{
  "origem": "quest_designer | studio_director",
  "encontro_id": "identificador único do encontro",
  "localizacao": "zona e tipo de terreno",
  "arquetipos_inimigos": [
    {"tipo": "grunt | tactico | elite | boss", "quantidade": 0}
  ],
  "nivel_progressao_jogador": "inicio | medio | avancado",
  "intencao_dramatica": "o que este combate deve fazer sentir",
  "restricoes_narrativas": "o que não pode acontecer neste encontro",
  "ciclo_id": "identificador do ciclo"
}
```

## MEMORY

**Long Term Memory: NÃO OBRIGATÓRIO**
Cada encontro é independente. O catálogo de arquétipos está na KB.

## VARIABLES USADAS

`{{ue5_remote_control_url}}` `{{google_drive_ai_folder}}` `{{make_webhook_base}}`

---
---

# AGENTE #11 — CROWD & TRAFFIC SIMULATION

## TOOLS

| Tool | Activar | Configuração |
|------|---------|--------------|
| LLM | ✅ Obrigatório | Temperature: 0.3 |
| HTTP Request | ✅ Obrigatório | Para chamar UE5 Remote Control API e receber updates do Lighting Agent |
| Knowledge Retrieval | ✅ Obrigatório | Ligar à KB deste agente |
| File Output | ✅ Obrigatório | Para gerar configs Mass AI em JSON |

## TRIGGER

**Tipo:** Webhook (POST)
**Quem chama:** World Generator (nova zona urbana), Lighting Agent (mudança de hora/tempo), NPC Behavior Agent (evento que afecta multidão)

**Payload esperado:**
```json
{
  "origem": "world_generator | lighting_agent | npc_behavior_agent",
  "zona": "nome da zona urbana",
  "hora_do_dia": "dawn | morning | midday | afternoon | golden_hour | dusk | night",
  "condicao_meteorologica": "clear | rain | storm | snow",
  "eventos_activos": [],
  "densidade_alvo": "baixa | media | alta",
  "ciclo_id": "identificador do ciclo"
}
```

## MEMORY

**Long Term Memory: NÃO OBRIGATÓRIO**
Configurações de cada zona ficam na KB e no Google Drive.

## VARIABLES USADAS

`{{ue5_remote_control_url}}` `{{google_drive_ai_folder}}` `{{make_webhook_base}}`

---
---

# AGENTE #12 — NARRATIVE & DIALOGUE AGENT

## TOOLS

| Tool | Activar | Configuração |
|------|---------|--------------|
| LLM | ✅ Obrigatório | Temperature: 0.8 (criativo) |
| Knowledge Retrieval | ✅ Obrigatório | Ligar à KB deste agente |
| File Output | ✅ Obrigatório | Para gerar Bible do Jogo, diálogos em JSON, documentos narrativos |
| HTTP Request | ✅ Obrigatório | Para distribuir Bible e perfis de personagens aos agentes dependentes |
| Web Search | ✅ Recomendado | Para pesquisa de contexto histórico e cultural quando necessário |

## TRIGGER

**Tipo:** Webhook (POST)
**Quem chama:** Studio Director (criar/actualizar narrativa), Quest Designer (beat narrativo para missão)

**Payload esperado — criação de Bible:**
```json
{
  "origem": "studio_director",
  "tipo": "criar_bible | actualizar_bible",
  "conceito_jogo": "texto completo do conceito do jogo",
  "premissa_narrativa": "texto da premissa",
  "referencias_narrativas": [],
  "regras_do_mundo": "texto das regras",
  "versao_anterior": "opcional — versão actual da Bible se for actualização",
  "ciclo_id": "identificador do ciclo"
}
```

**Payload esperado — diálogos de NPC:**
```json
{
  "origem": "npc_behavior_agent | studio_director",
  "tipo": "dialogos_npc",
  "arquetipos": ["lista de arquétipos que precisam de diálogos"],
  "zona": "contexto geográfico",
  "eventos_activos": [],
  "ciclo_id": "identificador do ciclo"
}
```

## MEMORY

**Long Term Memory: ACTIVAR — PRIORITÁRIO**

```
BIBLE DO JOGO:
- Versão actual: [número e data]
- Premissa: [uma frase]
- Ferida do protagonista: [uma frase]
- Facções e o seu estado actual: [lista]
- Eventos que já aconteceram no mundo: [timeline]
- Personagens com nome e estado actual: [lista]

CONSISTÊNCIA NARRATIVA:
- Factos estabelecidos que nunca podem ser contraditos: [lista]
- Contradições detectadas e como foram resolvidas: [log]
```

## VARIABLES USADAS

`{{project_name}}` `{{google_drive_narrative_folder}}` `{{notion_characters_database_id}}` `{{make_webhook_base}}`

---
---

# AGENTE #13 — QUEST & MISSION DESIGNER

## TOOLS

| Tool | Activar | Configuração |
|------|---------|--------------|
| LLM | ✅ Obrigatório | Temperature: 0.6 |
| HTTP Request | ✅ Obrigatório | Para chamar UE5 Remote Control API (colocar triggers no nível) e notificar QA Agent |
| Knowledge Retrieval | ✅ Obrigatório | Ligar à KB deste agente |
| File Output | ✅ Obrigatório | Para gerar dados de missão em JSON e storyboards de cinematics |

## TRIGGER

**Tipo:** Webhook (POST)
**Quem chama:** Narrative Agent (novo beat narrativo), Studio Director

**Payload esperado:**
```json
{
  "origem": "narrative_agent | studio_director",
  "missao_id": "identificador único",
  "tipo": "missao_principal | side_quest | encontro | descoberta",
  "beat_narrativo": "texto do beat narrativo desta missão",
  "intencao_emocional": "o que o jogador deve sentir no final",
  "zona": "onde acontece",
  "npcs_envolvidos": [],
  "restricoes": "o que não pode acontecer narrativamente",
  "ciclo_id": "identificador do ciclo"
}
```

## MEMORY

**Long Term Memory: ACTIVAR**

```
CATÁLOGO DE MISSÕES:
- [missao_id]: tipo, estado (rascunho/implementado/aprovado/QA), zona, NPCs, ramificações
- Missões com problemas de pacing: [lista com nota]

ESTADO DO MUNDO PÓS-MISSÃO:
- Consequências permanentes já implementadas: [lista]
- Facções afectadas por missões completadas: [log]
```

## VARIABLES USADAS

`{{ue5_remote_control_url}}` `{{google_drive_quests_folder}}` `{{notion_quest_database_id}}` `{{make_webhook_base}}`

---
---

# AGENTE #14 — AUDIO AGENT

## TOOLS

| Tool | Activar | Configuração |
|------|---------|--------------|
| LLM | ✅ Obrigatório | Temperature: 0.5 |
| HTTP Request | ✅ Obrigatório | Para chamar UE5 Remote Control API e ElevenLabs API (vozes) |
| Knowledge Retrieval | ✅ Obrigatório | Ligar à KB deste agente |
| File Output | ✅ Obrigatório | Para gerar configs MetaSounds em JSON |

## TRIGGER

**Tipo:** Webhook (POST)
**Quem chama:** Lighting Agent (mudança de estado), Quest Designer (nova missão com requisitos de áudio), Studio Director

**Payload esperado:**
```json
{
  "origem": "lighting_agent | quest_designer | studio_director",
  "tipo": "estado_ambiente | momento_narrativo | sistema_musica",
  "zona": "nome da zona ou 'global'",
  "estado_jogo": "exploracao | tensao | combate | vitoria | cutscene",
  "condicao_meteorologica": "clear | rain | storm | fog | snow",
  "hora_do_dia": "dawn | morning | midday | afternoon | golden_hour | dusk | night",
  "intencao_emocional": "o que o áudio deve reforçar",
  "momento_narrativo_especial": "opcional — evento específico que precisa de tratamento sonoro",
  "ciclo_id": "identificador do ciclo"
}
```

## MEMORY

**Long Term Memory: NÃO OBRIGATÓRIO**
O sistema de estados musicais está na KB. Cada configuração é independente.

## VARIABLES USADAS

`{{ue5_remote_control_url}}` `{{elevenlabs_api_key}}` `{{freesound_api_key}}` `{{google_drive_audio_folder}}` `{{make_webhook_base}}`

---
---

# AGENTE #15 — CHARACTER ARTIST AGENT

## TOOLS

| Tool | Activar | Configuração |
|------|---------|--------------|
| LLM | ✅ Obrigatório | Temperature: 0.5 |
| HTTP Request | ✅ Obrigatório | Para chamar MetaHuman API e notificar Animation Agent |
| Knowledge Retrieval | ✅ Obrigatório | Ligar à KB deste agente |
| File Output | ✅ Obrigatório | Para gerar especificações de personagem em JSON e biografias visuais em Markdown |

## TRIGGER

**Tipo:** Webhook (POST)
**Quem chama:** NPC Behavior Agent (novo tipo de NPC definido), Narrative Agent (novo personagem com nome), Studio Director

**Payload esperado:**
```json
{
  "origem": "npc_behavior_agent | narrative_agent | studio_director",
  "tipo": "personagem_principal | npc_nomeado | npc_generico",
  "arquetipos_necessarios": [],
  "quantidade_variacoes": 0,
  "zona_cultural": "contexto cultural e geográfico",
  "nivel_economico": "pobre | medio | rico",
  "perfil_social": "texto do perfil social do NPC Behavior Agent",
  "tracos_personalidade": "opcional — para personagens principais",
  "ciclo_id": "identificador do ciclo"
}
```

## MEMORY

**Long Term Memory: ACTIVAR**

```
CATÁLOGO DE PERSONAGENS CRIADOS:
- [id]: nome/tipo, arquétipo, zona, MetaHuman params, LOD chain, estado (criado/aprovado)
- Personagens principais com morph targets completos: [lista]
- Variações por tipo de NPC: [contagem por arquétipo]
```

## VARIABLES USADAS

`{{metahuman_api_url}}` `{{google_drive_characters_folder}}` `{{make_webhook_base}}`

---
---

# AGENTE #16 — ANIMATION AGENT

## TOOLS

| Tool | Activar | Configuração |
|------|---------|--------------|
| LLM | ✅ Obrigatório | Temperature: 0.4 |
| HTTP Request | ✅ Obrigatório | Para chamar UE5 Remote Control API |
| Knowledge Retrieval | ✅ Obrigatório | Ligar à KB deste agente |
| File Output | ✅ Obrigatório | Para gerar Animation Blueprint configs em JSON |
| Code Interpreter | ✅ Recomendado | Para gerar lógica de Motion Matching complexa |

## TRIGGER

**Tipo:** Webhook (POST)
**Quem chama:** Character Artist (novo personagem pronto para rig), Combat AI (novo arquétipo com acções de combate), Studio Director

**Payload esperado:**
```json
{
  "origem": "character_artist | combat_ai | studio_director",
  "personagem_id": "identificador do personagem",
  "tipo": "locomotion | combate | interaccao | cutscene",
  "arquetipos_movimento": "como esta personagem se move (biografia física)",
  "accoes_necessarias": [],
  "skeleton_asset": "caminho do asset no projecto UE5",
  "ciclo_id": "identificador do ciclo"
}
```

## MEMORY

**Long Term Memory: NÃO OBRIGATÓRIO**
As especificações de animação por arquétipo ficam na KB.

## VARIABLES USADAS

`{{ue5_remote_control_url}}` `{{google_drive_animation_folder}}` `{{make_webhook_base}}`

---
---

# AGENTE #17 — VFX AGENT

## TOOLS

| Tool | Activar | Configuração |
|------|---------|--------------|
| LLM | ✅ Obrigatório | Temperature: 0.5 |
| HTTP Request | ✅ Obrigatório | Para chamar UE5 Remote Control API e notificar Optimizer |
| Knowledge Retrieval | ✅ Obrigatório | Ligar à KB deste agente |
| File Output | ✅ Obrigatório | Para gerar Niagara system configs em JSON |

## TRIGGER

**Tipo:** Webhook (POST)
**Quem chama:** Quest Designer (momento narrativo com VFX especial), Combat AI (novo tipo de combate), Studio Director

**Payload esperado:**
```json
{
  "origem": "quest_designer | combat_ai | studio_director",
  "efeito_id": "identificador único",
  "tipo": "combate | ambiente | narrativo | destruicao | magico",
  "intencao": "o que este efeito deve comunicar ao jogador",
  "superficies_relevantes": ["madeira", "pedra", "metal"],
  "budget_ms": 0,
  "budget_particulas": 0,
  "ciclo_id": "identificador do ciclo"
}
```

## MEMORY

**Long Term Memory: NÃO OBRIGATÓRIO**
O catálogo de efeitos e os seus parâmetros ficam na KB e no Google Drive.

## VARIABLES USADAS

`{{ue5_remote_control_url}}` `{{google_drive_vfx_folder}}` `{{make_webhook_base}}`

---
---

# AGENTE #18 — QA & TESTING AGENT

## TOOLS

| Tool | Activar | Configuração |
|------|---------|--------------|
| LLM | ✅ Obrigatório | Temperature: 0.1 (máxima precisão) |
| HTTP Request | ✅ Obrigatório | Para chamar UE5 Automation Framework, buscar builds, e notificar agentes com bugs |
| Code Interpreter | ✅ Obrigatório | Para análise de logs de teste e relatórios de performance |
| Knowledge Retrieval | ✅ Obrigatório | Ligar à KB deste agente |
| File Output | ✅ Obrigatório | Para gerar bug reports em JSON e relatórios de build em Markdown |

## TRIGGER

**Tipo:** Webhook (POST) + Schedule
**Quem chama:** Integration Agent (nova build pronta), Make.com (schedule — cada build automática), Studio Director (teste on-demand)

**Payload esperado:**
```json
{
  "origem": "integration_agent | schedule | studio_director",
  "build_numero": "número da build",
  "build_url": "URL da build no servidor",
  "changelog": "o que mudou nesta build",
  "sistemas_alterados": [],
  "tipo_teste": "full | regression | performance | missoes",
  "ciclo_id": "identificador do ciclo"
}
```

## MEMORY

**Long Term Memory: ACTIVAR**

```
HISTÓRICO DE BUILDS:
- Build [número]: estado (aprovada/bloqueada), bugs críticos, data, sistemas testados

BUGS CONHECIDOS:
- [bug_id]: sistema, severidade, estado (aberto/resolvido/won't fix), agente responsável

PADRÕES DE FALHA RECORRENTES:
- [sistema]: tipo de falha recorrente, frequência, última ocorrência
```

## VARIABLES USADAS

`{{ue5_automation_url}}` `{{google_drive_qa_folder}}` `{{notion_bugs_database_id}}` `{{target_fps_pc}}` `{{target_fps_console}}` `{{make_webhook_base}}`

---
---

# AGENTE #19 — INTEGRATION & BUILD AGENT

## TOOLS

| Tool | Activar | Configuração |
|------|---------|--------------|
| LLM | ✅ Obrigatório | Temperature: 0.1 (máxima precisão) |
| HTTP Request | ✅ Obrigatório | Para chamar GitHub API, UE5 Build System, e notificar QA Agent |
| Code Interpreter | ✅ Obrigatório | Para análise de conflitos de merge e validação de dependências |
| Knowledge Retrieval | ✅ Obrigatório | Ligar à KB deste agente |
| File Output | ✅ Obrigatório | Para gerar changelogs e relatórios de integração em Markdown |

## TRIGGER

**Tipo:** Webhook (POST) + Schedule
**Quem chama:** Qualquer agente quando entrega um asset aprovado, ou Make.com (schedule a cada 6 horas)

**Payload esperado — novo asset para integrar:**
```json
{
  "origem": "nome_do_agente",
  "tipo_asset": "codigo | blueprint | config | narrative | audio | vfx",
  "asset_id": "identificador",
  "asset_url": "localização no Google Drive ou GitHub branch",
  "sistemas_afectados": [],
  "dependencias_necessarias": [],
  "aprovado_por": "studio_director | miguel",
  "ciclo_id": "identificador do ciclo"
}
```

**Payload de schedule:**
```json
{
  "origem": "schedule",
  "tipo": "build_automatica",
  "incluir_assets_desde": "timestamp da última build"
}
```

## MEMORY

**Long Term Memory: ACTIVAR — PRIORITÁRIO**

```
ESTADO DO REPOSITÓRIO:
- Branch principal: estado actual (limpo/com conflitos pendentes)
- Última build bem-sucedida: número, data, sistemas incluídos
- Assets aguardando integração: [lista com agente e data de entrega]

HISTÓRICO DE BUILDS:
- Build [número]: componentes, resultado QA, data, rollback disponível (S/N)

MAPA DE DEPENDÊNCIAS ACTUAL:
- [sistema A] depende de [sistema B]: última verificação OK em [data]
- Conflitos resolvidos: [log]

ROLLBACK DISPONÍVEL:
- Últimas 10 builds com localização e estado
```

## VARIABLES USADAS

`{{github_repo}}` `{{github_token}}` `{{ue5_build_server_url}}` `{{google_drive_builds_folder}}` `{{make_webhook_base}}`

---
---

# VARIABLES GLOBAIS DO PROJECTO

Estas variáveis são definidas uma vez e partilhadas por todos os agentes. No Relevance AI, configurar em **Settings → Variables** do projecto ou workspace.

## Variáveis de Projecto

| Nome da Variável | Tipo | Descrição | Valor de Exemplo |
|-----------------|------|-----------|-----------------|
| `{{project_name}}` | String | Nome do jogo | `"Project Nova"` |
| `{{dev_phase}}` | String | Fase actual | `"pre-producao"` |
| `{{bible_version}}` | String | Versão da Bible do Jogo | `"v1.2"` |
| `{{miguel_contact}}` | String | ID ou email do Miguel para notificações | `"miguel@..."` |

## Variáveis de Infra-estrutura UE5

| Nome da Variável | Tipo | Descrição | Valor de Exemplo |
|-----------------|------|-----------|-----------------|
| `{{ue5_version}}` | String | Versão do Unreal Engine | `"5.4"` |
| `{{ue5_remote_control_url}}` | String | URL do Remote Control API | `"http://localhost:30010"` |
| `{{ue5_automation_url}}` | String | URL do servidor de automação | `"http://localhost:30020"` |
| `{{ue5_build_server_url}}` | String | URL do servidor de build | `"http://buildserver:8080"` |
| `{{world_size_km2}}` | Number | Dimensão do mundo em km² | `200` |
| `{{target_fps_pc}}` | Number | Target de FPS em PC | `60` |
| `{{target_fps_console}}` | Number | Target de FPS em consola | `30` |
| `{{frame_budget_ms}}` | Number | Budget de frame em ms (PC) | `16.6` |

## Variáveis de Performance

| Nome da Variável | Tipo | Descrição | Valor de Exemplo |
|-----------------|------|-----------|-----------------|
| `{{max_draw_calls}}` | Number | Draw calls máximos por frame | `3000` |
| `{{max_npcs_simultaneous}}` | Number | NPCs simultâneos máximos | `10000` |
| `{{vram_budget_gb}}` | Number | Budget de VRAM em GB | `8` |

## Variáveis de APIs Externas

| Nome da Variável | Tipo | Descrição | Quando configurar |
|-----------------|------|-----------|------------------|
| `{{make_webhook_base}}` | String | URL base dos webhooks Make.com | Quando Make.com estiver configurado |
| `{{github_repo}}` | String | URL do repositório GitHub | Quando repo estiver criado |
| `{{github_token}}` | Secret | Token de acesso GitHub | Quando repo estiver criado |
| `{{google_drive_root}}` | String | ID da pasta raiz no Google Drive | Quando Drive estiver configurado |
| `{{elevenlabs_api_key}}` | Secret | API key ElevenLabs para vozes | Quando conta ElevenLabs estiver criada |
| `{{freesound_api_key}}` | Secret | API key Freesound para SFX | Quando conta Freesound estiver criada |
| `{{fab_api_key}}` | Secret | API key Fab.com (marketplace Epic) | Quando conta Fab estiver criada |
| `{{metahuman_api_url}}` | String | URL da API MetaHuman | Quando pipeline MetaHuman estiver configurado |

## Variáveis de Google Drive (pastas por departamento)

| Nome da Variável | Descrição |
|-----------------|-----------|
| `{{google_drive_world_folder}}` | Pasta: /Studio/World/ |
| `{{google_drive_art_folder}}` | Pasta: /Studio/Art/ |
| `{{google_drive_narrative_folder}}` | Pasta: /Studio/Narrative/ |
| `{{google_drive_ai_folder}}` | Pasta: /Studio/AI/ |
| `{{google_drive_audio_folder}}` | Pasta: /Studio/Audio/ |
| `{{google_drive_characters_folder}}` | Pasta: /Studio/Characters/ |
| `{{google_drive_animation_folder}}` | Pasta: /Studio/Animation/ |
| `{{google_drive_vfx_folder}}` | Pasta: /Studio/VFX/ |
| `{{google_drive_quests_folder}}` | Pasta: /Studio/Quests/ |
| `{{google_drive_qa_folder}}` | Pasta: /Studio/QA/ |
| `{{google_drive_builds_folder}}` | Pasta: /Studio/Builds/ |
| `{{google_drive_reports_folder}}` | Pasta: /Studio/Reports/ |

## Variáveis de Bases de Dados (Notion ou Airtable)

| Nome da Variável | Descrição |
|-----------------|-----------|
| `{{notion_npc_database_id}}` | Base de dados de todos os NPCs |
| `{{notion_quest_database_id}}` | Base de dados de todas as missões |
| `{{notion_characters_database_id}}` | Base de dados de personagens com nome |
| `{{notion_bugs_database_id}}` | Base de dados de bugs do QA |

---

## ORDEM DE CONFIGURAÇÃO RECOMENDADA

### Fase 1 — Antes de activar qualquer agente
1. Definir `{{project_name}}`, `{{dev_phase}}`, `{{miguel_contact}}`
2. Definir todas as variáveis UE5 (mesmo que o UE5 não esteja ligado ainda — os valores ficam guardados)
3. Criar a estrutura de pastas no Google Drive e preencher as variáveis `{{google_drive_*}}`

### Fase 2 — Quando o Make.com estiver configurado
4. Preencher `{{make_webhook_base}}`
5. Activar os Triggers de cada agente
6. Testar o fluxo Studio Director → Engine Architect → Studio Director

### Fase 3 — Quando o repositório estiver criado
7. Preencher `{{github_repo}}` e `{{github_token}}`
8. Activar o Integration Agent completamente

### Fase 4 — Quando o UE5 Remote Control estiver activo
9. Preencher `{{ue5_remote_control_url}}`
10. Testar chamadas de agentes ao UE5 editor

### Fase 5 — APIs externas (por ordem de necessidade)
11. ElevenLabs quando o Audio Agent precisar de gerar vozes
12. Freesound quando o Audio Agent precisar de SFX
13. Fab quando os agentes de arte precisarem de assets
14. MetaHuman API quando o Character Artist estiver em produção

---

*Versão 1.0 — Março 2026*
*Configuração Operacional — Transpersonal Game Studio*
*Tools · Triggers · Memory · Variables — 19 Agentes*
