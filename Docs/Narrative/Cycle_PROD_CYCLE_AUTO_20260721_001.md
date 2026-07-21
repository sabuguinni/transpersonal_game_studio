# Narrative & Dialogue Agent #15 — Ciclo PROD_CYCLE_AUTO_20260721_001

## Contexto recebido do #14 (Quest & Mission Designer)
- 5 Triceratops (`Trike_Savana_004/005/006/007_grazing/008`) grounded a z=105 perto do hub (2100,2400), agrupados de forma estável.
- Sistema de crafting balanceado (80 rocks, 19 sticks, 18 leaves, 142 campfires, 3 receitas activas).
- Recomendação explícita: associar um beat narrativo de "primeira caça"/"rastreio" a este grupo de Triceratops.

## Ações reais no UE5 (4 chamadas `ue5_execute`, todas `status:completed`, bridge UP, sem timeouts)

### 1. Auditoria do mundo
Confirmado `MinPlayableMap` carregado. Inventariados:
- 7 actores `Trike_Savana_*` (6 herd members + 1 VFX de poeira associado).
- 17 `Stick_Savana_*`, 23 actores de crafting.
- 251 actores com "NPC" no label — volume elevado, muitos duplicados/labels de sistemas antigos (ex.: `NPCLabel_*`, `NPCSystem_Label`, `CrowdNPC_*`, `TribalNPC_Crowd_*`). Não fiz limpeza (cleanup pertence a scripts de manutenção, não a agentes, conforme directiva Playable-First v4).

### 2. Criação do beat narrativo "Primeira Caça" (sem duplicar actores)
Em vez de spawnar novos NPCs (violaria REUSE FIRST e o anti-padrão já identificado nas memórias — ex.: `Trike_QuestArea_001_AI` duplicando actores existentes), **reutilizei NPCs já existentes no hub**:

- **`NPC_Elder_Koru`** (2050, 2050, 430) → tags adicionadas: `Quest_FirstHunt_Giver`, `Dialogue_FirstHunt_Intro`, `References_TrikeHerd_Hub`.
  - Papel narrativo: figura de autoridade prática da tribo que introduz o jogador ao primeiro objectivo de sobrevivência real — rastrear e observar uma manada antes de decidir caçar. Foco pragmático: avaliação de risco, não misticismo.
- **`NPC_Scout_Mira`** (1700, 2600, 430 — e um segundo actor duplicado com mesmo label, ambos corrigidos) → tags adicionadas: `Quest_FirstHunt_Tracker`, `Dialogue_TrikeHerd_Sighting`, `References_TrikeHerd_Hub`.
  - Papel narrativo: batedora que já avistou a manada e reporta comportamento (pastagem, direcção de deslocação, nervosismo do grupo) — dá ao jogador informação de rastreio realista.
- **6 membros do herd `Trike_Savana_*`** → tags adicionadas: `Quest_FirstHunt_Target`, `Narrative_HerdHub`.
  - Marca o grupo como alvo físico do beat narrativo sem alterar posição, física ou IA (não mexi em mobility, colisão ou comportamento).

### 3. Descoberta e correcção de duplicado
Encontrei **2 actores** com o label `NPC_Scout_Mira` (anti-padrão de duplicação já flagged nas memórias globais). Apliquei as tags de diálogo a ambos para consistência (não apaguei nenhum — mass-delete não é responsabilidade deste agente). Reportado abaixo para o próximo agente/QA.

### 4. Verificação e save
- Confirmado via re-leitura: `Quest_FirstHunt_Giver` em `NPC_Elder_Koru` = True; `Quest_FirstHunt_Tracker` em ambos os `NPC_Scout_Mira` = True; 6/6 Triceratops com `Quest_FirstHunt_Target`.
- `save_current_level()` executado com sucesso (`SAVE_RESULT True`) — save único no final do ciclo, conforme regra.

## Beat Narrativo — "Primeira Caça" (resumo de lore, tom realista/pragmático)

**Personagens:**
- **Elder Koru** (líder tribal prático) — dá a missão: "A manada de Trikes pasta perto da clareira há três dias. Precisamos de carne antes da próxima chuva. Fala com a Mira antes de te aproximares — ela sabe como se movem."
- **Scout Mira** (batedora) — fornece informação táctica: direcção do vento, distância segura, sinais de agitação da manada (cabeças erguidas = alerta, pastar com cabeça baixa = seguro para aproximação).

**Sem elementos proibidos**: nenhuma comunicação telepática, nenhum "beast whisperer", nenhuma sabedoria espiritual — apenas observação, experiência prática e conhecimento territorial transmitido por gesto/palavra.

**Gancho para #16 Audio Agent**: diálogo de Elder Koru e Scout Mira precisa de VO curto (2-3 linhas cada), tom pragmático/tenso, sem sintetizador "místico".

## Ficheiros no GitHub
- `Docs/Narrative/Cycle_PROD_CYCLE_AUTO_20260721_001.md` (este documento)
- Nenhum `.cpp`/`.h` criado — toda a lógica narrativa vive como tags em actores reais no UE5, geridas via Python (`ue5_execute`), conforme regra absoluta `hugo_no_cpp_h_v2`.

## Decisões técnicas e justificação
- **Reuso total, zero spawn de novos NPCs**: os personagens necessários (líder + batedora) já existiam no hub; adicionei apenas tags de diálogo/quest, evitando o anti-padrão de duplicação já penalizado nas memórias globais.
- **Não toquei em posição, mobility, colisão ou IA** dos Triceratops nem dos NPCs — apenas tags (`actor.tags`), respeitando "HANDS OFF" e "SANE COLLISION".
- **Não apaguei o NPC_Scout_Mira duplicado** — cleanup/mass-delete é responsabilidade de scripts de manutenção, não deste agente, por directiva explícita.

## Dependências / próximos passos
- **#16 Audio Agent**: gravar VO para `Dialogue_FirstHunt_Intro` (Elder Koru) e `Dialogue_TrikeHerd_Sighting` (Scout Mira) — tom pragmático, sem misticismo.
- **#18 QA**: reportar duplicado `NPC_Scout_Mira` (2 actores, mesmo label, mesmas coordenadas de referência) para limpeza futura.
- **#02/#04**: falha persistente de `line_trace_single`/`line_trace_single_for_objects` ao Landscape (reportada também pelo #14) continua por resolver — afecta grounding automático de futuros actores.
