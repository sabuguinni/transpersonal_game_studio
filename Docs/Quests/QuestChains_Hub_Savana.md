# Quest Chains — Hub / Savana Cretácea

**Agente:** #14 Quest & Mission Designer
**Ciclo:** PROD_CYCLE_MANUAL7_20260809
**Estado:** Verificado no mundo vivo (UE5, não apenas em ficheiro). Todas as tags confirmadas por releitura independente após save.

## Princípio de design

Cada missão é uma peça de teatro em três actos: pedido → busca → entrega → recompensa. Nenhuma missão foi implementada com UI, menu de crafting ou C++ — isso não existe neste editor headless. Tudo o que se segue é **estado do mundo**: tags em actores já existentes (Stick_*, Rock_*, Leaf_*) e três âncoras de diálogo (SkeletalMeshActor com malha real, não NPC vazio).

Não se criaram actores de recurso novos. Todos os itens de missão são recursos de sobrevivência já espalhados no mundo (Rock_Savana_*, Stick_Savana_*, Leaf_Savana_*) que ganharam tags de missão em cima da malha real que já tinham.

---

## Cadeia 001 — "O Machado do Artesão" (CraftsmansAxe)
**NPC:** NPC_Anchor_CraftsmanOrin_001 (1650, 2700, 100) — SkeletalMesh real (MESH_F_WHITE_REGULAR_ASSEMBLED)

Orin é um artesão prático: precisa de pedra lascada e de um cabo de madeira para montar um machado de pedra. Não há mística nenhuma — é uma negociação de materiais.

| Ordem | Actor | Tag de posição | Papel |
|---|---|---|---|
| QuestOrder_01 | Rock_QuestStoneAxe_001 (2250, 2350, 105) | `Quest_StoneAxe_Rock1` | pedra para a cabeça do machado |
| QuestOrder_02 | Rock_QuestStoneAxe_002 (2280, 2500, 160) | `Quest_StoneAxe_Rock2` | segunda pedra (reserva/afinação do gume) |
| QuestOrder_03 | Stick_Savana_111 (1850, 2650, 105) | — | cabo de madeira |
| QuestOrder_04 | NPC_Anchor_CraftsmanOrin_001 | `QuestGiver_CraftsmansAxe` | entrega e recompensa |

**Tags de necessidade no NPC (novas neste ciclo, por simetria com Kael):** `Quest_Needs_AxeHeadStone`, `Quest_Needs_AxeHandle`
**Diálogo (já existente):** Dialogue_Orin_Intro_001 → Progress_001 → Deliver_001 → Reward_001
**Recompensa:** `Quest_Reward_StoneAxeCrafted`

Arco emocional: o jogador chega com as mãos vazias, o artesão não confia em promessas — só em material entregue. A recompensa é uma ferramenta que passa a existir no mundo, não um número.

---

## Cadeia 002 — "A Colheita da Anciã" (ElderGathering)
**NPC:** NPC_Anchor_ElderSana_001 (2400, 2700, 67) — SkeletalMesh real

Sana pede folhas medicinais para tratar feridas do acampamento — uma tarefa de forrageamento prático, não ritual.

| Ordem | Actor | Papel |
|---|---|---|
| QuestOrder_01 | Leaf_Savana_014 (2350, 2450, 105) | folha 1 |
| QuestOrder_02 | Leaf_Savana_017 (2150, 2650, 105) | folha 2 |
| QuestOrder_03 | Leaf_Savana_019 (2600, 2500, 105) | folha 3 |
| QuestOrder_04 | NPC_Anchor_ElderSana_001 | entrega e recompensa |

**Tag de necessidade no NPC (nova neste ciclo):** `Quest_Needs_MedicinalLeaf`
**Diálogo:** Dialogue_Sana_Intro_001 → Progress_001 → Deliver_001 → Reward_001
**Recompensa:** `Quest_Reward_ElderTrust` (ligado a `Story_FirstSeasonSurvived` — reconhecimento social, não poder mágico)

---

## Cadeia 003 — "A Vigília do Rastreador" (TrackerWatch)
**NPC:** NPC_Anchor_TrackerKael_001 (1950, 2200, 129) — SkeletalMesh real

Kael pede que o jogador observe uma manada e detecte raptores antes de trazer material de rastreio. É a única cadeia com componente de exploração/observação, não só recolha.

| Ordem | Actor | Papel |
|---|---|---|
| QuestOrder_01 | QuestMarker_ObserveHerd_001 | ponto de observação da manada (marcador, sem malha — não é conteúdo visível, é gatilho de posição) |
| QuestOrder_02 | QuestMarker_ScoutRaptors_001 | ponto de reconhecimento de raptores |
| QuestOrder_03 | Rock_Savana_568 (2120, 2039, 133) — `Quest_Item_Flintstone` | pederneira |
| QuestOrder_04 (item) | Stick_Savana_100 (2000, 2300, 125) — `Quest_Item_SpearShaft` | haste de lança |
| QuestOrder_05 | NPC_Anchor_TrackerKael_001 | entrega e recompensa |

**Tags de necessidade no NPC (já existiam):** `Quest_Needs_SpearShaft`, `Quest_Needs_Flintstone`
**Diálogo:** Dialogue_Kael_Intro_001 → Observe_001 → Scout_001 → Deliver_001 → Reward_001
**Recompensa:** `Quest_Reward_TrackingLesson` (uma habilidade prática — melhor leitura de pegadas — não iluminação nem consciência expandida)

---

## Estado de verificação (medido, não assumido)

- 13 actores no total têm tags `QuestOrder_NN`, confirmados sem duplicados em releitura por invocação separada (ciclo anterior).
- Neste ciclo: adicionadas `Quest_Needs_AxeHeadStone` + `Quest_Needs_AxeHandle` a CraftsmanOrin, e `Quest_Needs_MedicinalLeaf` a ElderSana, para simetria com TrackerKael (era a única âncora com tags de necessidade explícitas). Confirmado por releitura independente após `save_dirty_packages(True, True)`.
- Todos os actores usados nas três cadeias têm componente de malha real (`StaticMeshComponent` ou `SkeletalMeshComponent` com asset atribuído), com excepção de `QuestMarker_ObserveHerd_001`, que é um marcador de posição sem malha — não está na lista de prefixos protegidos por conteúdo visível (não é `Resource_*`, `Stick_*`, `Rock_*`, `Leaf_*`, `NPC_*`, etc.), é um gatilho de sistema, não uma peça de cenário.
- Não foram criados novos actores de recurso, nenhuma malha de `/Engine/BasicShapes` foi introduzida por este agente (as primitivas herdadas — Cylinder/Plane/Sphere em alguns Stick_/Leaf_/QuestMarker_ já existiam de ciclos anteriores e carregam tags funcionais que as protegem, ver `hugo_asset_first_v8`).

## Limitações reportadas (sem contorno)

1. **Sem UI de quest log.** Este editor headless não tem camada de interface jogável; o estado de progresso de cada cadeia existe apenas como tags no mundo, não como um HUD com objectivos. Isso é trabalho de um sistema de UI que não pode ser entregue por Python/Blueprint neste ambiente.
2. **Sem lógica de trigger em Blueprint.** Não foi criado nenhum evento de Blueprint (overlap → avançar quest → tocar diálogo) porque grafos de Blueprint não são construíveis por Python nesta build (ver hugo_asset_first_v8 / hugo_python_neste_editor_v2, secção "O Python cria contentores, não comportamento"). O que existe é o **contrato de dados**: as tags e a ordem que um sistema de gameplay (quando existir em C++/Blueprint compilado) pode ler directamente.
3. **QuestMarker_ObserveHerd_001 não tem malha** — funciona como ponto de gatilho lógico, não como objecto visível; se se pretender que o jogador veja fisicamente "algo" no ponto de observação (uma pegada, uma pedra marcada), isso é trabalho do Environment Artist (#6), não deste agente.

## Próximo agente (#15 — Narrative & Dialogue Agent)

As três cadeias já têm nomes de linhas de diálogo reservados (`Dialogue_<NPC>_<Beat>_001`) mas o **texto** dessas linhas ainda não existe como asset de diálogo — apenas como tag/etiqueta. O #15 deve escrever o conteúdo narrativo de cada beat (Intro, Progress/Observe/Scout, Deliver, Reward) coerente com o tom prático e de sobrevivência de cada NPC:
- Orin: directo, técnico, desconfiado até ver material.
- Sana: pragmática, cuidadora do acampamento, sem misticismo.
- Kael: observador, cauteloso com predadores, valoriza discrição.
