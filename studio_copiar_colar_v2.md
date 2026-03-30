# GUIA DE CONFIGURAÇÃO — RELEVANCE AI
## Transpersonal Game Studio · 19 Agentes
## Copiar e colar directamente no Relevance AI

---

## MAPEAMENTO DE TOOLS — LER ANTES DE COMEÇAR

Os nomes abaixo são os nomes exactos das tools no Relevance AI. Para cada tool indicamos se deves usar **Add** (tools Verified, prontas a usar) ou **Clone** (templates que precisam de ser clonados primeiro).

| Tool no documento | Nome exacto no Relevance AI | Acção |
|---|---|---|
| LLM | **LLM** | Clone |
| Chamadas a agentes/APIs internas | **Relevance API Call** | Add (Verified) |
| Chamadas HTTP externas (UE5, GitHub, Make.com) | **Google API Call** | Add (Verified) |
| Pesquisa na KB | **Knowledge search** | Add (Verified) |
| Recuperar registos da KB | **Retrieve Record(s) from Knowledge** | Clone |
| Guardar memória persistente | **Add New Record to Knowledge** | Clone |
| Actualizar memória persistente | **Update Record in Knowledge** | Clone |
| Gerar ficheiros para download | **Export data to permanent downloadable file** | Add (Verified) |
| Executar Python | **Python Code** | Clone |
| Pesquisa web | **Web Search** | Add (Verified) |

---

## SOBRE A MEMÓRIA PERSISTENTE

Os agentes que precisam de memória entre sessões usam **Knowledge Tables** como base de dados persistente — não a secção Memory do Relevance AI.

Para cada agente com memória activada:
1. Ir a **Knowledge** → **Create Knowledge Table** → dar o nome indicado (ex: `memoria_studio_director`)
2. Adicionar as 3 tools de memória listadas na secção Tools do agente
3. O conteúdo do bloco MEMORY é o que o agente deve guardar nessa tabela ao longo do tempo

---

## COMO USAR ESTE DOCUMENTO

Cada agente tem quatro secções. Para cada uma há uma instrução clara de **onde colar** no Relevance AI.

```
TOOLS      → Agente → Tools → activar cada tool listada
TRIGGER    → Agente → Triggers → New Trigger → Webhook → copiar payload
MEMORY     → Agente → Memory → Long Term Memory → copiar o bloco de texto
VARIABLES  → Workspace Settings → Variables → criar cada variável listada
```

As **VARIABLES GLOBAIS** no final do documento são configuradas uma única vez para todo o workspace — não por agente.

---
---

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# AGENTE #01 — STUDIO DIRECTOR
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

---

## TOOLS
→ **Onde colar:** Agente #01 → Tools → activar cada tool abaixo

```
✅ LLM
   (Clone — Tool Builder)
   Temperature: 0.7

✅ Relevance API Call
   (Verified — para chamar outros agentes e APIs internas)

✅ Google API Call
   (Verified — para chamadas HTTP a APIs externas: UE5, GitHub, Make.com)

✅ Knowledge search
   (Verified — pesquisa na KB do agente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar registos específicos)

✅ Add New Record to Knowledge
   (Clone — guardar memória persistente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar memória persistente)

✅ Update Record in Knowledge
   (Clone — actualizar memória existente)

✅ Export data to permanent downloadable file
   (Verified — gerar ficheiros Markdown, JSON, texto)
```

---

## TRIGGER
→ **Onde colar:** Agente #01 → Triggers → New Trigger → Webhook → copiar o JSON abaixo como "Expected Payload"

**Trigger 1 — Instrução do Miguel**
```json
{
  "origem": "miguel",
  "instrucao": "texto da instrução criativa do Miguel",
  "contexto_adicional": "",
  "prioridade": "normal"
}
```

**Trigger 2 — Aprovação/Rejeição de ciclo**
```json
{
  "origem": "miguel",
  "tipo": "aprovacao",
  "ciclo_id": "",
  "decisao": "aprovado",
  "nota": ""
}
```

---

## MEMORY
→ **Onde colar:** Agente #01 → Tools → adicionar as 3 tools de memória abaixo + criar uma Knowledge Table dedicada para este agente → colar o texto como instrução inicial nessa tabela

```
Guarda e actualiza sempre estas informações ao longo de cada conversa:

ESTADO DO PROJECTO:
- Nome do jogo
- Versão da Bible do Jogo (número e data)
- Fase actual do desenvolvimento (pré-produção / produção / polish)
- Última instrução aprovada pelo Miguel
- Ciclos em curso (lista com estado de cada um)
- Decisões críticas tomadas (log com data)

ESTADO DOS AGENTES:
- Agentes com tarefas em curso (nome + descrição da tarefa)
- Agentes bloqueados (nome + razão)

PREFERÊNCIAS DO MIGUEL:
- Coisas que o Miguel aprovou entusiasticamente
- Coisas que o Miguel rejeitou (com razão)
- Notas sobre como o Miguel gosta de comunicar
```

---
---

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# AGENTE #02 — ENGINE ARCHITECT
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

---

## TOOLS
→ **Onde colar:** Agente #02 → Tools

```
✅ LLM
   (Clone — Tool Builder)
   Temperature: 0.3

✅ Relevance API Call
   (Verified — para chamar outros agentes e APIs internas)

✅ Google API Call
   (Verified — para chamadas HTTP a APIs externas: UE5, GitHub, Make.com)

✅ Knowledge search
   (Verified — pesquisa na KB do agente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar registos específicos)

✅ Export data to permanent downloadable file
   (Verified — gerar ficheiros Markdown, JSON, texto)

✅ Python Code
   (Clone — executar Python para análise e geração de ficheiros)
```

---

## TRIGGER
→ **Onde colar:** Agente #02 → Triggers → New Trigger → Webhook → Expected Payload

```json
{
  "origem": "studio_director",
  "tipo": "nova_arquitectura",
  "conceito_jogo": "",
  "requisitos": {
    "dimensao_mundo_km2": 0,
    "npcs_simultaneos": 0,
    "plataformas_alvo": ["PC", "PS5"],
    "features_criticas": []
  },
  "ciclo_id": ""
}
```

---

## MEMORY
→ **Onde configurar:** Agente #02 → Tools → adicionar as 3 tools de memória listadas acima
→ Criar uma Knowledge Table chamada `memoria_engine_architect` → colar o texto abaixo como primeiro registo

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

---
---

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# AGENTE #03 — CORE SYSTEMS PROGRAMMER
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

---

## TOOLS
→ **Onde colar:** Agente #03 → Tools

```
✅ LLM
   (Clone — Tool Builder)
   Temperature: 0.2

✅ Python Code
   (Clone — executar Python para análise e geração de ficheiros)

✅ Relevance API Call
   (Verified — para chamar outros agentes e APIs internas)

✅ Google API Call
   (Verified — para chamadas HTTP a APIs externas: UE5, GitHub, Make.com)

✅ Knowledge search
   (Verified — pesquisa na KB do agente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar registos específicos)

✅ Export data to permanent downloadable file
   (Verified — gerar ficheiros Markdown, JSON, texto)
```

---

## TRIGGER
→ **Onde colar:** Agente #03 → Triggers → New Trigger → Webhook → Expected Payload

```json
{
  "origem": "engine_architect",
  "sistema": "",
  "especificacao": "",
  "budget_ms": 0,
  "dependencias": [],
  "prioridade": "normal",
  "ciclo_id": ""
}
```

---

## MEMORY
→ **Nota:** Agente #03 não precisa de memória persistente. Não adicionar as tools de Knowledge para memória. A sessão de conversa é suficiente.

---
---

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# AGENTE #04 — PERFORMANCE & OPTIMIZER
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

---

## TOOLS
→ **Onde colar:** Agente #04 → Tools

```
✅ LLM
   (Clone — Tool Builder)
   Temperature: 0.2

✅ Python Code
   (Clone — executar Python para análise e geração de ficheiros)

✅ Relevance API Call
   (Verified — para chamar outros agentes e APIs internas)

✅ Google API Call
   (Verified — para chamadas HTTP a APIs externas: UE5, GitHub, Make.com)

✅ Knowledge search
   (Verified — pesquisa na KB do agente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar registos específicos)

✅ Export data to permanent downloadable file
   (Verified — gerar ficheiros Markdown, JSON, texto)
```

---

## TRIGGER
→ **Onde colar:** Agente #04 → Triggers → New Trigger → Webhook → Expected Payload

```json
{
  "origem": "qa_agent",
  "tipo": "analise_performance",
  "zona": "",
  "frame_time_actual_ms": 0,
  "plataforma": "PC",
  "relatorio_url": "",
  "ciclo_id": ""
}
```

---

## MEMORY
→ **Onde configurar:** Agente #04 → Tools → adicionar as 3 tools de memória listadas acima
→ Criar uma Knowledge Table chamada `memoria_optimizer` → colar o texto abaixo como primeiro registo

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
- (actualizar sempre que o Engine Architect definir novos valores)
```

---
---

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# AGENTE #05 — PROCEDURAL WORLD GENERATOR
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

---

## TOOLS
→ **Onde colar:** Agente #05 → Tools

```
✅ LLM
   (Clone — Tool Builder)
   Temperature: 0.6

✅ Python Code
   (Clone — executar Python para análise e geração de ficheiros)

✅ Relevance API Call
   (Verified — para chamar outros agentes e APIs internas)

✅ Google API Call
   (Verified — para chamadas HTTP a APIs externas: UE5, GitHub, Make.com)

✅ Knowledge search
   (Verified — pesquisa na KB do agente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar registos específicos)

✅ Export data to permanent downloadable file
   (Verified — gerar ficheiros Markdown, JSON, texto)
```

---

## TRIGGER
→ **Onde colar:** Agente #05 → Triggers → New Trigger → Webhook → Expected Payload

```json
{
  "origem": "studio_director",
  "regiao": "",
  "descricao": "",
  "area_km2": 0,
  "biomas_pretendidos": [],
  "clima": "",
  "ligacoes_a": [],
  "intencao_narrativa": "",
  "ciclo_id": ""
}
```

---

## MEMORY
→ **Onde configurar:** Agente #05 → Tools → adicionar as 3 tools de memória listadas acima
→ Criar uma Knowledge Table chamada `memoria_world_generator` → colar o texto abaixo como primeiro registo

```
Guarda e actualiza sempre estas informações:

MAPA GLOBAL DO MUNDO:
- Regiões geradas: nome, área em km², biomas, estado (rascunho/aprovado)
- Rios e sistema hidrográfico: pontos de nascente, direcção de fluxo, ligações
- Rede de estradas: nós principais e ligações geradas
- Convenções de nomenclatura geográfica adoptadas
```

---
---

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# AGENTE #06 — ENVIRONMENT ARTIST
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

---

## TOOLS
→ **Onde colar:** Agente #06 → Tools

```
✅ LLM
   (Clone — Tool Builder)
   Temperature: 0.6

✅ Relevance API Call
   (Verified — para chamar outros agentes e APIs internas)

✅ Google API Call
   (Verified — para chamadas HTTP a APIs externas: UE5, GitHub, Make.com)

✅ Knowledge search
   (Verified — pesquisa na KB do agente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar registos específicos)

✅ Export data to permanent downloadable file
   (Verified — gerar ficheiros Markdown, JSON, texto)
```

---

## TRIGGER
→ **Onde colar:** Agente #06 → Triggers → New Trigger → Webhook → Expected Payload

```json
{
  "origem": "world_generator",
  "zona": "",
  "bioma": "",
  "narrativa_geografica": "",
  "intencao_narrativa": "",
  "referencia_visual": "",
  "ciclo_id": ""
}
```

---

## MEMORY
→ **Nota:** Agente #06 não precisa de memória persistente. Não adicionar as tools de Knowledge para memória. A sessão de conversa é suficiente.

---
---

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# AGENTE #07 — ARCHITECTURE & INTERIOR AGENT
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

---

## TOOLS
→ **Onde colar:** Agente #07 → Tools

```
✅ LLM
   (Clone — Tool Builder)
   Temperature: 0.5

✅ Relevance API Call
   (Verified — para chamar outros agentes e APIs internas)

✅ Google API Call
   (Verified — para chamadas HTTP a APIs externas: UE5, GitHub, Make.com)

✅ Knowledge search
   (Verified — pesquisa na KB do agente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar registos específicos)

✅ Export data to permanent downloadable file
   (Verified — gerar ficheiros Markdown, JSON, texto)
```

---

## TRIGGER
→ **Onde colar:** Agente #07 → Triggers → New Trigger → Webhook → Expected Payload

```json
{
  "origem": "world_generator",
  "tipo_estrutura": "residencial",
  "localizacao": "",
  "nivel_economico": "medio",
  "estado_conservacao": "intacto",
  "historia_pretendida": "",
  "uso_em_missao": "",
  "ciclo_id": ""
}
```

---

## MEMORY
→ **Onde configurar:** Agente #07 → Tools → adicionar as 3 tools de memória listadas acima
→ Criar uma Knowledge Table chamada `memoria_architecture_agent` → colar o texto abaixo como primeiro registo

```
Guarda e actualiza sempre estas informações:

CATÁLOGO DE ESTRUTURAS CRIADAS:
- ID/nome, tipo, localização, estado de conservação, elementos interactivos, missões associadas

PROPS EM FALTA:
- Props solicitados que não existem no catálogo disponível (para escalar ao Studio Director)
```

---
---

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# AGENTE #08 — LIGHTING & ATMOSPHERE AGENT
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

---

## TOOLS
→ **Onde colar:** Agente #08 → Tools

```
✅ LLM
   (Clone — Tool Builder)
   Temperature: 0.4

✅ Relevance API Call
   (Verified — para chamar outros agentes e APIs internas)

✅ Google API Call
   (Verified — para chamadas HTTP a APIs externas: UE5, GitHub, Make.com)

✅ Knowledge search
   (Verified — pesquisa na KB do agente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar registos específicos)

✅ Export data to permanent downloadable file
   (Verified — gerar ficheiros Markdown, JSON, texto)
```

---

## TRIGGER
→ **Onde colar:** Agente #08 → Triggers → New Trigger → Webhook → Expected Payload

```json
{
  "origem": "environment_artist",
  "zona": "",
  "hora_do_dia": "golden_hour",
  "condicao_meteorologica": "clear",
  "intencao_emocional": "",
  "evento_narrativo_activo": "",
  "ciclo_id": ""
}
```

---

## MEMORY
→ **Onde configurar:** Agente #08 → Tools → adicionar as 3 tools de memória listadas acima
→ Criar uma Knowledge Table chamada `memoria_lighting_agent` → colar o texto abaixo como primeiro registo

```
Guarda e actualiza sempre estas informações:

ESTADOS DE ILUMINAÇÃO APROVADOS:
- Zona, hora do dia, condição meteorológica, data de aprovação, localização dos parâmetros no Drive

PREFERÊNCIAS VISUAIS DO MIGUEL:
- Estados que aprovou com entusiasmo
- Temperaturas de cor e intensidades preferidas
- O que rejeitou e porquê
```

---
---

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# AGENTE #09 — NPC BEHAVIOR AGENT
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

---

## TOOLS
→ **Onde colar:** Agente #09 → Tools

```
✅ LLM
   (Clone — Tool Builder)
   Temperature: 0.5

✅ Relevance API Call
   (Verified — para chamar outros agentes e APIs internas)

✅ Google API Call
   (Verified — para chamadas HTTP a APIs externas: UE5, GitHub, Make.com)

✅ Knowledge search
   (Verified — pesquisa na KB do agente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar registos específicos)

✅ Export data to permanent downloadable file
   (Verified — gerar ficheiros Markdown, JSON, texto)

✅ Python Code
   (Clone — executar Python para análise e geração de ficheiros)

✅ Add New Record to Knowledge
   (Clone — guardar memória persistente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar memória persistente)

✅ Update Record in Knowledge
   (Clone — actualizar memória existente)
```

---

## TRIGGER
→ **Onde colar:** Agente #09 → Triggers → New Trigger → Webhook → Expected Payload

**Trigger 1 — Nova zona a popular**
```json
{
  "origem": "world_generator",
  "zona": "",
  "arquetipos_necessarios": [],
  "densidade_populacao": "media",
  "eventos_narrativos_activos": [],
  "reputacao_jogador_na_zona": "neutro",
  "ciclo_id": ""
}
```

**Trigger 2 — Evento narrativo que altera comportamentos**
```json
{
  "origem": "narrative_agent",
  "tipo": "evento_narrativo",
  "evento": "",
  "zonas_afectadas": [],
  "npcs_afectados": [],
  "alteracao_comportamento": "",
  "ciclo_id": ""
}
```

---

## MEMORY
→ **Onde configurar:** Agente #09 → Tools → adicionar as 3 tools de memória listadas acima
→ Criar uma Knowledge Table chamada `memoria_npc_behavior` → colar o texto abaixo como primeiro registo

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

---
---

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# AGENTE #10 — COMBAT & ENEMY AI AGENT
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

---

## TOOLS
→ **Onde colar:** Agente #10 → Tools

```
✅ LLM
   (Clone — Tool Builder)
   Temperature: 0.3

✅ Relevance API Call
   (Verified — para chamar outros agentes e APIs internas)

✅ Google API Call
   (Verified — para chamadas HTTP a APIs externas: UE5, GitHub, Make.com)

✅ Knowledge search
   (Verified — pesquisa na KB do agente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar registos específicos)

✅ Export data to permanent downloadable file
   (Verified — gerar ficheiros Markdown, JSON, texto)

✅ Python Code
   (Clone — executar Python para análise e geração de ficheiros)
```

---

## TRIGGER
→ **Onde colar:** Agente #10 → Triggers → New Trigger → Webhook → Expected Payload

```json
{
  "origem": "quest_designer",
  "encontro_id": "",
  "localizacao": "",
  "arquetipos_inimigos": [
    {"tipo": "grunt", "quantidade": 0}
  ],
  "nivel_progressao_jogador": "inicio",
  "intencao_dramatica": "",
  "restricoes_narrativas": "",
  "ciclo_id": ""
}
```

---

## MEMORY
→ **Nota:** Agente #10 não precisa de memória persistente. Não adicionar as tools de Knowledge para memória. A sessão de conversa é suficiente.

---
---

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# AGENTE #11 — CROWD & TRAFFIC SIMULATION
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

---

## TOOLS
→ **Onde colar:** Agente #11 → Tools

```
✅ LLM
   (Clone — Tool Builder)
   Temperature: 0.3

✅ Relevance API Call
   (Verified — para chamar outros agentes e APIs internas)

✅ Google API Call
   (Verified — para chamadas HTTP a APIs externas: UE5, GitHub, Make.com)

✅ Knowledge search
   (Verified — pesquisa na KB do agente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar registos específicos)

✅ Export data to permanent downloadable file
   (Verified — gerar ficheiros Markdown, JSON, texto)
```

---

## TRIGGER
→ **Onde colar:** Agente #11 → Triggers → New Trigger → Webhook → Expected Payload

```json
{
  "origem": "world_generator",
  "zona": "",
  "hora_do_dia": "midday",
  "condicao_meteorologica": "clear",
  "eventos_activos": [],
  "densidade_alvo": "media",
  "ciclo_id": ""
}
```

---

## MEMORY
→ **Nota:** Agente #11 não precisa de memória persistente. Não adicionar as tools de Knowledge para memória. A sessão de conversa é suficiente.

---
---

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# AGENTE #12 — NARRATIVE & DIALOGUE AGENT
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

---

## TOOLS
→ **Onde colar:** Agente #12 → Tools

```
✅ LLM
   (Clone — Tool Builder)
   Temperature: 0.8

✅ Knowledge search
   (Verified — pesquisa na KB do agente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar registos específicos)

✅ Export data to permanent downloadable file
   (Verified — gerar ficheiros Markdown, JSON, texto)

✅ Relevance API Call
   (Verified — para chamar outros agentes e APIs internas)

✅ Google API Call
   (Verified — para chamadas HTTP a APIs externas: UE5, GitHub, Make.com)

✅ Web Search
   (Verified)
```

---

## TRIGGER
→ **Onde colar:** Agente #12 → Triggers → New Trigger → Webhook → Expected Payload

**Trigger 1 — Criar ou actualizar Bible do Jogo**
```json
{
  "origem": "studio_director",
  "tipo": "criar_bible",
  "conceito_jogo": "",
  "premissa_narrativa": "",
  "referencias_narrativas": [],
  "regras_do_mundo": "",
  "ciclo_id": ""
}
```

**Trigger 2 — Diálogos de NPC**
```json
{
  "origem": "npc_behavior_agent",
  "tipo": "dialogos_npc",
  "arquetipos": [],
  "zona": "",
  "eventos_activos": [],
  "ciclo_id": ""
}
```

---

## MEMORY
→ **Onde configurar:** Agente #12 → Tools → adicionar as 3 tools de memória listadas acima
→ Criar uma Knowledge Table chamada `memoria_narrative_agent` → colar o texto abaixo como primeiro registo

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

---
---

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# AGENTE #13 — QUEST & MISSION DESIGNER
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

---

## TOOLS
→ **Onde colar:** Agente #13 → Tools

```
✅ LLM
   (Clone — Tool Builder)
   Temperature: 0.6

✅ Relevance API Call
   (Verified — para chamar outros agentes e APIs internas)

✅ Google API Call
   (Verified — para chamadas HTTP a APIs externas: UE5, GitHub, Make.com)

✅ Knowledge search
   (Verified — pesquisa na KB do agente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar registos específicos)

✅ Export data to permanent downloadable file
   (Verified — gerar ficheiros Markdown, JSON, texto)
```

---

## TRIGGER
→ **Onde colar:** Agente #13 → Triggers → New Trigger → Webhook → Expected Payload

```json
{
  "origem": "narrative_agent",
  "missao_id": "",
  "tipo": "missao_principal",
  "beat_narrativo": "",
  "intencao_emocional": "",
  "zona": "",
  "npcs_envolvidos": [],
  "restricoes": "",
  "ciclo_id": ""
}
```

---

## MEMORY
→ **Onde configurar:** Agente #13 → Tools → adicionar as 3 tools de memória listadas acima
→ Criar uma Knowledge Table chamada `memoria_quest_designer` → colar o texto abaixo como primeiro registo

```
Guarda e actualiza sempre estas informações:

CATÁLOGO DE MISSÕES:
- ID, tipo, estado (rascunho/implementado/aprovado/em QA), zona, NPCs, número de ramificações

CONSEQUÊNCIAS PERMANENTES JÁ IMPLEMENTADAS:
- Missão, consequência no mundo, facções afectadas

MISSÕES COM PROBLEMAS DE PACING:
- ID, descrição do problema, nota de revisão
```

---
---

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# AGENTE #14 — AUDIO AGENT
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

---

## TOOLS
→ **Onde colar:** Agente #14 → Tools

```
✅ LLM
   (Clone — Tool Builder)
   Temperature: 0.5

✅ Relevance API Call
   (Verified — para chamar outros agentes e APIs internas)

✅ Google API Call
   (Verified — para chamadas HTTP a APIs externas: UE5, GitHub, Make.com)

✅ Knowledge search
   (Verified — pesquisa na KB do agente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar registos específicos)

✅ Export data to permanent downloadable file
   (Verified — gerar ficheiros Markdown, JSON, texto)
```

---

## TRIGGER
→ **Onde colar:** Agente #14 → Triggers → New Trigger → Webhook → Expected Payload

```json
{
  "origem": "lighting_agent",
  "tipo": "estado_ambiente",
  "zona": "",
  "estado_jogo": "exploracao",
  "condicao_meteorologica": "clear",
  "hora_do_dia": "morning",
  "intencao_emocional": "",
  "momento_narrativo_especial": "",
  "ciclo_id": ""
}
```

---

## MEMORY
→ **Nota:** Agente #14 não precisa de memória persistente. Não adicionar as tools de Knowledge para memória. A sessão de conversa é suficiente.

---
---

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# AGENTE #15 — CHARACTER ARTIST AGENT
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

---

## TOOLS
→ **Onde colar:** Agente #15 → Tools

```
✅ LLM
   (Clone — Tool Builder)
   Temperature: 0.5

✅ Relevance API Call
   (Verified — para chamar outros agentes e APIs internas)

✅ Google API Call
   (Verified — para chamadas HTTP a APIs externas: UE5, GitHub, Make.com)

✅ Knowledge search
   (Verified — pesquisa na KB do agente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar registos específicos)

✅ Export data to permanent downloadable file
   (Verified — gerar ficheiros Markdown, JSON, texto)
```

---

## TRIGGER
→ **Onde colar:** Agente #15 → Triggers → New Trigger → Webhook → Expected Payload

```json
{
  "origem": "npc_behavior_agent",
  "tipo": "npc_generico",
  "arquetipos_necessarios": [],
  "quantidade_variacoes": 1,
  "zona_cultural": "",
  "nivel_economico": "medio",
  "perfil_social": "",
  "tracos_personalidade": "",
  "ciclo_id": ""
}
```

---

## MEMORY
→ **Onde configurar:** Agente #15 → Tools → adicionar as 3 tools de memória listadas acima
→ Criar uma Knowledge Table chamada `memoria_character_artist` → colar o texto abaixo como primeiro registo

```
Guarda e actualiza sempre estas informações:

CATÁLOGO DE PERSONAGENS CRIADOS:
- ID, nome ou tipo, arquétipo, zona, parâmetros MetaHuman principais, estado (criado/aprovado)
- Personagens principais com morph targets de expressão completos

VARIAÇÕES POR ARQUÉTIPO:
- Arquétipo, número de variações únicas criadas, zonas onde foram colocadas
```

---
---

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# AGENTE #16 — ANIMATION AGENT
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

---

## TOOLS
→ **Onde colar:** Agente #16 → Tools

```
✅ LLM
   (Clone — Tool Builder)
   Temperature: 0.4

✅ Relevance API Call
   (Verified — para chamar outros agentes e APIs internas)

✅ Google API Call
   (Verified — para chamadas HTTP a APIs externas: UE5, GitHub, Make.com)

✅ Knowledge search
   (Verified — pesquisa na KB do agente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar registos específicos)

✅ Export data to permanent downloadable file
   (Verified — gerar ficheiros Markdown, JSON, texto)

✅ Python Code
   (Clone — executar Python para análise e geração de ficheiros)
```

---

## TRIGGER
→ **Onde colar:** Agente #16 → Triggers → New Trigger → Webhook → Expected Payload

```json
{
  "origem": "character_artist",
  "personagem_id": "",
  "tipo": "locomotion",
  "arquetipos_movimento": "",
  "accoes_necessarias": [],
  "skeleton_asset": "",
  "ciclo_id": ""
}
```

---

## MEMORY
→ **Nota:** Agente #16 não precisa de memória persistente. Não adicionar as tools de Knowledge para memória. A sessão de conversa é suficiente.

---
---

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# AGENTE #17 — VFX AGENT
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

---

## TOOLS
→ **Onde colar:** Agente #17 → Tools

```
✅ LLM
   (Clone — Tool Builder)
   Temperature: 0.5

✅ Relevance API Call
   (Verified — para chamar outros agentes e APIs internas)

✅ Google API Call
   (Verified — para chamadas HTTP a APIs externas: UE5, GitHub, Make.com)

✅ Knowledge search
   (Verified — pesquisa na KB do agente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar registos específicos)

✅ Export data to permanent downloadable file
   (Verified — gerar ficheiros Markdown, JSON, texto)
```

---

## TRIGGER
→ **Onde colar:** Agente #17 → Triggers → New Trigger → Webhook → Expected Payload

```json
{
  "origem": "quest_designer",
  "efeito_id": "",
  "tipo": "combate",
  "intencao": "",
  "superficies_relevantes": [],
  "budget_ms": 0,
  "budget_particulas": 0,
  "ciclo_id": ""
}
```

---

## MEMORY
→ **Nota:** Agente #17 não precisa de memória persistente. Não adicionar as tools de Knowledge para memória. A sessão de conversa é suficiente.

---
---

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# AGENTE #18 — QA & TESTING AGENT
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

---

## TOOLS
→ **Onde colar:** Agente #18 → Tools

```
✅ LLM
   (Clone — Tool Builder)
   Temperature: 0.1

✅ Relevance API Call
   (Verified — para chamar outros agentes e APIs internas)

✅ Google API Call
   (Verified — para chamadas HTTP a APIs externas: UE5, GitHub, Make.com)

✅ Python Code
   (Clone — executar Python para análise e geração de ficheiros)

✅ Knowledge search
   (Verified — pesquisa na KB do agente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar registos específicos)

✅ Export data to permanent downloadable file
   (Verified — gerar ficheiros Markdown, JSON, texto)

✅ Add New Record to Knowledge
   (Clone — guardar memória persistente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar memória persistente)

✅ Update Record in Knowledge
   (Clone — actualizar memória existente)
```

---

## TRIGGER
→ **Onde colar:** Agente #18 → Triggers → New Trigger → Webhook → Expected Payload

```json
{
  "origem": "integration_agent",
  "build_numero": "",
  "build_url": "",
  "changelog": "",
  "sistemas_alterados": [],
  "tipo_teste": "full",
  "ciclo_id": ""
}
```

---

## MEMORY
→ **Onde configurar:** Agente #18 → Tools → adicionar as 3 tools de memória listadas acima
→ Criar uma Knowledge Table chamada `memoria_qa_agent` → colar o texto abaixo como primeiro registo

```
Guarda e actualiza sempre estas informações:

HISTÓRICO DE BUILDS:
- Número, estado (aprovada/bloqueada), bugs críticos encontrados, data, sistemas testados

BUGS CONHECIDOS:
- ID, sistema, severidade (Crítico/Alto/Médio/Baixo), estado (aberto/resolvido), agente responsável

PADRÕES DE FALHA RECORRENTES:
- Sistema, tipo de falha, frequência, data da última ocorrência
```

---
---

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# AGENTE #19 — INTEGRATION & BUILD AGENT
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

---

## TOOLS
→ **Onde colar:** Agente #19 → Tools

```
✅ LLM
   (Clone — Tool Builder)
   Temperature: 0.1

✅ Relevance API Call
   (Verified — para chamar outros agentes e APIs internas)

✅ Google API Call
   (Verified — para chamadas HTTP a APIs externas: UE5, GitHub, Make.com)

✅ Python Code
   (Clone — executar Python para análise e geração de ficheiros)

✅ Knowledge search
   (Verified — pesquisa na KB do agente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar registos específicos)

✅ Export data to permanent downloadable file
   (Verified — gerar ficheiros Markdown, JSON, texto)

✅ Add New Record to Knowledge
   (Clone — guardar memória persistente)

✅ Retrieve Record(s) from Knowledge
   (Clone — recuperar memória persistente)

✅ Update Record in Knowledge
   (Clone — actualizar memória existente)
```

---

## TRIGGER
→ **Onde colar:** Agente #19 → Triggers → New Trigger → Webhook → Expected Payload

**Trigger 1 — Novo asset para integrar**
```json
{
  "origem": "nome_do_agente",
  "tipo_asset": "codigo",
  "asset_id": "",
  "asset_url": "",
  "sistemas_afectados": [],
  "dependencias_necessarias": [],
  "aprovado_por": "studio_director",
  "ciclo_id": ""
}
```

**Trigger 2 — Build automática (schedule)**
```json
{
  "origem": "schedule",
  "tipo": "build_automatica",
  "incluir_assets_desde": ""
}
```

---

## MEMORY
→ **Onde configurar:** Agente #19 → Tools → adicionar as 3 tools de memória listadas acima
→ Criar uma Knowledge Table chamada `memoria_integration_agent` → colar o texto abaixo como primeiro registo

```
Guarda e actualiza sempre estas informações:

ESTADO DO REPOSITÓRIO:
- Estado actual da branch principal (limpo / conflitos pendentes)
- Assets aguardando integração (agente, asset, data de entrega)

HISTÓRICO DE BUILDS:
- Número, componentes incluídos, resultado do QA, data, rollback disponível (S/N)

MAPA DE DEPENDÊNCIAS:
- Sistema A depende de Sistema B (data da última verificação OK)
- Conflitos resolvidos (log)

ROLLBACK:
- Localização das últimas 10 builds aprovadas
```

---
---

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# VARIABLES GLOBAIS DO WORKSPACE
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

→ **Onde configurar:** Relevance AI → Settings → Variables → New Variable
→ **Criar uma vez — partilhadas por todos os agentes**

---

## BLOCO 1 — Configurar já hoje

→ Criar estas variáveis imediatamente, antes de activar qualquer agente

| Nome | Tipo | Valor a preencher |
|------|------|-------------------|
| `project_name` | Text | Nome do jogo |
| `dev_phase` | Text | `pre-producao` |
| `bible_version` | Text | `v0` (actualizar quando existir) |
| `miguel_contact` | Text | Email ou ID do Miguel |
| `ue5_version` | Text | `5.4` (ou versão instalada) |
| `target_fps_pc` | Number | `60` |
| `target_fps_console` | Number | `30` |
| `frame_budget_ms` | Number | `16.6` |
| `world_size_km2` | Number | (dimensão pretendida) |
| `max_npcs_simultaneous` | Number | `10000` |
| `max_draw_calls` | Number | `3000` |
| `vram_budget_gb` | Number | `8` |

---

## BLOCO 2 — Configurar quando o Google Drive estiver organizado

→ Criar a estrutura de pastas `/Studio/` no Drive primeiro, depois preencher os IDs

| Nome | Tipo | Valor a preencher |
|------|------|-------------------|
| `google_drive_root` | Text | ID da pasta `/Studio/` |
| `google_drive_world_folder` | Text | ID da pasta `/Studio/World/` |
| `google_drive_art_folder` | Text | ID da pasta `/Studio/Art/` |
| `google_drive_narrative_folder` | Text | ID da pasta `/Studio/Narrative/` |
| `google_drive_ai_folder` | Text | ID da pasta `/Studio/AI/` |
| `google_drive_audio_folder` | Text | ID da pasta `/Studio/Audio/` |
| `google_drive_characters_folder` | Text | ID da pasta `/Studio/Characters/` |
| `google_drive_animation_folder` | Text | ID da pasta `/Studio/Animation/` |
| `google_drive_vfx_folder` | Text | ID da pasta `/Studio/VFX/` |
| `google_drive_quests_folder` | Text | ID da pasta `/Studio/Quests/` |
| `google_drive_qa_folder` | Text | ID da pasta `/Studio/QA/` |
| `google_drive_builds_folder` | Text | ID da pasta `/Studio/Builds/` |
| `google_drive_reports_folder` | Text | ID da pasta `/Studio/Reports/` |

---

## BLOCO 3 — Configurar quando o Make.com estiver activo

| Nome | Tipo | Valor a preencher |
|------|------|-------------------|
| `make_webhook_base` | Text | URL base dos webhooks Make.com |

---

## BLOCO 4 — Configurar quando o GitHub estiver criado

| Nome | Tipo | Valor a preencher |
|------|------|-------------------|
| `github_repo` | Text | URL do repositório |
| `github_token` | Secret | Token de acesso pessoal |

---

## BLOCO 5 — Configurar quando o UE5 Remote Control estiver activo

| Nome | Tipo | Valor a preencher |
|------|------|-------------------|
| `ue5_remote_control_url` | Text | `http://localhost:30010` |
| `ue5_automation_url` | Text | `http://localhost:30020` |
| `ue5_build_server_url` | Text | URL do servidor de build |

---

## BLOCO 6 — APIs externas (por ordem de necessidade)

| Nome | Tipo | Quando criar | Onde obter |
|------|------|-------------|------------|
| `elevenlabs_api_key` | Secret | Quando Audio Agent precisar de vozes | elevenlabs.io → Profile → API Keys |
| `freesound_api_key` | Secret | Quando Audio Agent precisar de SFX | freesound.org → API → Apply |
| `fab_api_key` | Secret | Quando agentes de arte precisarem de assets | fab.com → Account → API |
| `metahuman_api_url` | Text | Quando Character Artist estiver em produção | Epic Games Developer Portal |

---

## BLOCO 7 — Bases de dados (Notion ou Airtable)

→ Criar as bases de dados primeiro, depois copiar os IDs aqui

| Nome | Tipo | Base de dados |
|------|------|---------------|
| `notion_npc_database_id` | Text | Base de dados de todos os NPCs |
| `notion_quest_database_id` | Text | Base de dados de todas as missões |
| `notion_characters_database_id` | Text | Base de dados de personagens com nome |
| `notion_bugs_database_id` | Text | Base de dados de bugs do QA |

---
---

# SUMÁRIO — QUAIS OS AGENTES COM LONG TERM MEMORY ACTIVADA

→ Referência rápida para confirmar a configuração de Memory

| Agente | Long Term Memory |
|--------|-----------------|
| #01 Studio Director | ✅ ACTIVAR |
| #02 Engine Architect | ✅ ACTIVAR |
| #03 Core Systems | ❌ não necessário |
| #04 Optimizer | ✅ ACTIVAR |
| #05 World Generator | ✅ ACTIVAR |
| #06 Environment Artist | ❌ não necessário |
| #07 Architecture | ✅ ACTIVAR |
| #08 Lighting | ✅ ACTIVAR |
| #09 NPC Behavior | ✅ ACTIVAR — PRIORITÁRIO |
| #10 Combat AI | ❌ não necessário |
| #11 Crowd Simulation | ❌ não necessário |
| #12 Narrative | ✅ ACTIVAR — PRIORITÁRIO |
| #13 Quest Designer | ✅ ACTIVAR |
| #14 Audio | ❌ não necessário |
| #15 Character Artist | ✅ ACTIVAR |
| #16 Animation | ❌ não necessário |
| #17 VFX | ❌ não necessário |
| #18 QA Testing | ✅ ACTIVAR |
| #19 Integration | ✅ ACTIVAR — PRIORITÁRIO |

---

*Versão 1.0 · Março 2026 · Transpersonal Game Studio*
