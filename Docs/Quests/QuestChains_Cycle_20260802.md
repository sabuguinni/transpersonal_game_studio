# Quest Chains — Ciclo PROD_CYCLE_MANUAL2_20260802
**Agente #14 — Quest & Mission Designer**

## Contexto
Três NPCs de diálogo já existem no mundo (colocados por ciclos anteriores, ancorados perto do hub):
- `NPC_Anchor_CraftsmanOrin_001` (1650, 2700, 105)
- `NPC_Anchor_ElderSana_001` (2400, 2700, 72)
- `NPC_Anchor_TrackerKael_001` (1950, 2200, 134)

Estes três NPCs são humanos primitivos práticos (artesão de ferramentas, anciã da tribo, batedor/rastreador), sem qualquer conotação espiritual. Este ciclo desenha três cadeias de missão **find → carry → deliver** sobre actores e tags que **já existem** no mundo — nada foi criado do zero, só ligado com tags novas de cadeia.

## QC1 — "O Machado do Artesão" (QuestChain_001_CraftsmansAxe)
**Quest giver:** NPC_Anchor_CraftsmanOrin_001 (tag `QuestGiver_CraftsmansAxe`)

**Arco emocional:** Orin herdou o conhecimento de talhar pedra de um artesão anterior que morreu (lore já presente na tag `Narrative_LoreLink_ToolmakerOrin_Origin` / `Lore_Text_FormerToolmakerDied_KnowledgePassedToOrin` em `Rock_Savana_005`). O jogador não está só a "buscar 3 itens" — está a ajudar a continuar um legado de sobrevivência que quase se perdeu.

**Passos jogáveis:**
1. FIND: `Rock_QuestStoneAxe_001` (2280, 2500, 105) — tag `QuestStep_Find`
2. FIND: `Rock_QuestStoneAxe_002` (2250, 2350, 105) — tag `QuestStep_Find`
3. FIND: `Stick_Savana_111` (1850, 2650, 105) — tag `QuestStep_Find` (o mais próximo de Orin, 206 uu)
4. DELIVER: voltar a NPC_Anchor_CraftsmanOrin_001 — tag `QuestStep_Deliver`

**Recompensa implícita:** desbloqueia a receita StoneAxe (2 Rocha + 1 Pau), já validada como receita de crafting existente (tags `Used_In_Recipe_StoneAxe` / `Quest_CraftStoneAxe`).

## QC2 — "A Colheita da Anciã" (QuestChain_002_ElderGathering)
**Quest giver:** NPC_Anchor_ElderSana_001 (tag `QuestGiver_ElderGathering`)

**Arco emocional:** Sana é a anciã que sobreviveu à primeira estação (tag `Story_FirstSeasonSurvived` já presente). Pedir folhas não é um fetch quest vazio — é a anciã a preparar a tribo para o inverno, transmitindo experiência prática de sobrevivência ao jogador.

**Passos jogáveis:**
1. FIND: `Leaf_Savana_014` (2350, 2450, 105) — 255 uu de Sana
2. FIND: `Leaf_Savana_017` (2150, 2650, 105) — 255 uu de Sana
3. FIND: `Leaf_Savana_019` (2600, 2500, 105) — 283 uu de Sana
4. DELIVER: voltar a NPC_Anchor_ElderSana_001

**Recompensa implícita:** material para abrigo/isolamento (Leaf = CraftIngredient_Leaf, já tagueado para receitas existentes).

## QC3 — "A Vigília do Rastreador" (QuestChain_003_TrackerWatch)
**Quest giver:** NPC_Anchor_TrackerKael_001 (tag `QuestGiver_TrackerWatch`)

**Arco emocional:** este NÃO é find-carry-deliver de itens — é observação e risco. Kael pede ao jogador que confirme se uma manada está segura para caçar e se uma matilha de raptores está por perto. Tensão: o jogador tem de se aproximar de predadores reais (Pack_Size_443, CombatAI ativo) sem ser detectado.

**Passos jogáveis:**
1. OBSERVE: `QuestMarker_ObserveHerd_001` (1800, 2200, 200) — tag `QuestStep_Observe`, ~150 uu de Kael
2. SCOUT: `QuestMarker_ScoutRaptors_001` (2300, 1900, 100) — tag `QuestStep_Scout`, zona de matilha de raptores já configurada com IA de combate completa (flanking, pack hunting, aggro radius 1200)
3. DELIVER (relatar): voltar a NPC_Anchor_TrackerKael_001

**Recompensa implícita:** informação de perigo (o jogador aprende a localização da matilha antes de a encontrar por acidente) — reforça o tema de sobrevivência por conhecimento, não por sorte.

## Tags de cadeia aplicadas neste ciclo (11 actores, verificado por releitura independente)
| Actor | Tags adicionadas |
|---|---|
| Rock_QuestStoneAxe_001 | QuestChain_001_CraftsmansAxe, QuestStep_Find |
| Rock_QuestStoneAxe_002 | QuestChain_001_CraftsmansAxe, QuestStep_Find |
| Stick_Savana_111 | QuestChain_001_CraftsmansAxe, QuestStep_Find |
| NPC_Anchor_CraftsmanOrin_001 | QuestChain_001_CraftsmansAxe, QuestStep_Deliver, QuestGiver_CraftsmansAxe |
| Leaf_Savana_014 | QuestChain_002_ElderGathering, QuestStep_Find |
| Leaf_Savana_017 | QuestChain_002_ElderGathering, QuestStep_Find |
| Leaf_Savana_019 | QuestChain_002_ElderGathering, QuestStep_Find |
| NPC_Anchor_ElderSana_001 | QuestChain_002_ElderGathering, QuestStep_Deliver, QuestGiver_ElderGathering |
| QuestMarker_ObserveHerd_001 | QuestChain_003_TrackerWatch, QuestStep_Observe |
| QuestMarker_ScoutRaptors_001 | QuestChain_003_TrackerWatch, QuestStep_Scout |
| NPC_Anchor_TrackerKael_001 | QuestChain_003_TrackerWatch, QuestStep_Deliver, QuestGiver_TrackerWatch |

Persistência confirmada via `save_dirty_packages(True, True)` + releitura independente de `get_editor_property('tags')` em todos os 11 actores (não confiado apenas no valor de retorno do save).

## Limitações reportadas (não implementável neste editor headless)
- **Nenhuma UI de quest log / journal**: não existe camada de UI neste ambiente headless. As tags são a única forma de expressar estado de missão.
- **Nenhum tracking de progresso runtime**: não há sistema de quest C++ (proibido criar .cpp/.h — inerte no editor). O avanço de passo (Find → Deliver) tem de ser lido externamente via tags, não há disparo automático de eventos in-game.
- **Diálogo real dos NPCs**: fica a cargo do Agente #15 (Narrative & Dialogue), que já preparou `Narrative_VoiceLine_Ready` nos anchors de Orin e Sana.
- Não foram criados novos actores de recurso, nem movidos/rodados/apagados quaisquer `Dino_*`, respeitando a directiva do ciclo.

## Próximo agente (#15 — Narrative & Dialogue Agent)
- QC1 e QC2 já têm `Narrative_VoiceLine_Ready` nos NPCs — falta escrever o texto de diálogo real de entrega/recompensa.
- QC3 (Kael) ainda não tem `Narrative_VoiceLine_Ready` — recomenda-se adicionar diálogo de aviso sobre a matilha de raptores.
- Considerar dar nome/voz aos três NPCs em falas curtas de entrega de missão (ex.: reação de Orin ao receber os materiais, reação de Sana às folhas, aviso de Kael sobre os raptores).
