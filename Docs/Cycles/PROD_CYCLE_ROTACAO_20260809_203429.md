# Ciclo PROD_CYCLE_ROTACAO_20260809_203429 — Studio Director

## Veredicto do portão (1 leitura, `/root/playability_audit.py --resumo`)
- **Global: WARN** — fail=0, warn=5, skip=2, erro=0. 4018 actores no mundo (PIE). Pré-checks OK (MinPlayableMap 3593≥2500, Terrain_Savana 425≥400).
- **CAMADAS_0_2: 9/9 ok** — jogador intocado, pawn=BP_TranspersonalPlayer_C, mesh=MESH_F_WHITE_REGULAR_ASSEMBLED, ps_ground=true.
- **DEFEITOS_DIFERIDOS: 11 checks, 335 ocorrências** (nota: a directiva cita R20/43, R18/27, R37/15, R36/15, R17/13, R31/1 como estado de referência; o resumo deste ciclo mostra R18 já subiu para 30 e R36 para 17 — ambos a piorar, não a melhorar).

### Não-PASS nomeados neste resumo
| Check | Estado | Actual | Base | Nota |
|---|---|---|---|---|
| R18 | WARN | 30 | 27 | Veg_Jungle_028(+52), Veg_Jungle_053(+460), Veg_Jungle_026(+353) — vegetação mal assente pela base, não pela pata |
| R24r | WARN | 73.0 fps | 79.7 | performance a degradar, mediana de 5 amostras limpas |
| R26 | MANUAL | null | "presente" | requer confirmação manual, fora do âmbito dos agentes técnicos |
| R27r | WARN | [MinPlayableMap] | [] | há dirty package não gravado |
| R28r | DELEGADO | null | watchdog activo | fora do âmbito de produção |
| R36 | WARN | 17 | 15 | Rock_Savana_070(-1964), EnvProp_ForestClearing_ImportedMatch_001, Rock_Hub_006(-140) — rochas/props deslocados da base |
| V1-C28 | WARN | hash mismatch | hash mismatch | integridade de ficheiros de sistema (sanity_guard.py) — não é tarefa de conteúdo |

## Atribuição de tarefas (uma por agente, mensurável)

- **#2 Engine Architect** — NADA este ciclo. Não há conflito de arquitectura para resolver.
- **#3 Core Systems Programmer** — NADA este ciclo. R27r (dirty package não gravado) não é um sistema core; será resolvido pelo #19 no fecho.
- **#4 Performance Optimizer** — mover R24r de 73.0 para ≥76.0 fps: perfilar os +54 actores acrescentados desde o último ciclo (4018 vs ~3964) e identificar o custo real antes de qualquer corte; não tocar em Lumen/sol (bloqueados).
- **#5 Procedural World Generator** — NADA este ciclo. Terreno e Terrain_Savana estão protegidos e não fazem parte dos WARN.
- **#6 Environment Artist** — reduzir R18 de 30 para ≤25: re-assentar pela BASE dos bounds (não pela pata, são vegetação estática) os 3 piores casos nomeados: Veg_Jungle_053 (+460), Veg_Jungle_026 (+353), Veg_Jungle_028 (+52), via line trace ao Landscape ignorando não-Landscape.
- **#7 Architecture & Interior Agent** — reduzir R36 de 17 para ≤14: reposicionar contra o terreno as 3 rochas/props nomeados Rock_Savana_070 (-1964, enterrada), EnvProp_ForestClearing_ImportedMatch_001, Rock_Hub_006 (-140).
- **#8 Lighting & Atmosphere Agent** — NADA este ciclo. Sol e SkyLight são BLOCKER (R04/R05r) e estão resolvidos; não tocar.
- **#9 Character Artist Agent** — iniciar a substituição dos 9 CrowdNPC que ainda são esferas /Engine por meshes humanas reais do pack já disponível no projecto (MESH_F_WHITE_REGULAR_ASSEMBLED ou variantes); entregar pelo menos 3 substituídas e verificadas por releitura do skeletal_mesh_asset.
- **#10 Animation Agent** — depender do #9: aplicar ANIMATION_SINGLE_NODE + idle nas 3 CrowdNPC que o #9 converter este ciclo, para não ficarem em bind-pose.
- **#11 NPC Behavior Agent** — NADA este ciclo (sem novos NPCs funcionais para comportamento; esperar #9/#10).
- **#12 Combat & Enemy AI Agent** — iniciar o gap real de IA de dinossauros: escolher 1 espécie já com Pawn/AIController vivo (Triceratops, conforme receita provada em hugo_navegacao_e_criaturas_moveis_v1) e adicionar reacção de fuga simples a proximidade de predador, medível por log de eventos, sem tocar nos 56 Dino_* estáticos protegidos.
- **#13 Crowd & Traffic Simulation** — NADA este ciclo. Sem Mass AI a decidir; aguarda base de IA do #12.
- **#14 Quest & Mission Designer** — NADA este ciclo.
- **#15 Narrative & Dialogue Agent** — NADA este ciclo.
- **#16 Audio Agent** — NADA este ciclo.
- **#17 VFX Agent** — NADA este ciclo.
- **#18 QA & Testing Agent** — verificar após #6 e #7 que R18 e R36 desceram nos números acima antes de qualquer merge; bloquear se algum subir.
- **#19 Integration & Build Agent** — gravar o dirty package de MinPlayableMap (resolve R27r) só depois de #6/#7 confirmarem as suas mutações, com save_dirty_packages(True,True), e reportar de volta ao #1.

## O que NÃO foi pedido este ciclo (deliberado)
Não foi pedido trabalho de terreno (#5), iluminação (#8, bloqueado por R04/R05r), narrativa/quests/áudio/VFX (#14-17) nem crowd simulation (#13) — não há dependências prontas nem checks não-PASS que os justifiquem. Fazer nada nestes é mais barato do que uma escrita errada.
