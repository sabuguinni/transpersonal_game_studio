# Ciclo de Produção — PROD_CYCLE_AUTO_20260811_001
**Studio Director (#01)** — 11/08/2026

## Veredicto do Portão (2 leituras idênticas de `/root/playability_audit.py --resumo`)
- **Global: WARN** — fail=0, warn=5, skip=3, erro=0. 4018 actores no mundo (PIE).
- Pré-checks OK: MinPlayableMap 3593 actores (min 2500), Terrain_Savana 425 (min 400).
- **CAMADAS_0_2: 9/9 ok** — jogador intocado (pawn=BP_TranspersonalPlayer_C, mesh_z=-97.0, hh=96.0, ps_ground=true). Não tocar.
- **DEFEITOS_DIFERIDOS: 11 checks, 335 ocorrências** (backlog real).

### Não-PASS nomeados nesta leitura
| Check | Estado | Actual | Base | Nota |
|---|---|---|---|---|
| R18 | WARN | 30 | 27 (+3) | Veg_Jungle_028(+52), Veg_Jungle_053(+460), Veg_Jungle_026(+353) — vegetação mal assente/escalada |
| R36 | WARN | 17 | 15 (+2) | Rock_Savana_070(-1964), EnvProp_ForestClearing_ImportedMatch_001, Rock_Hub_006(-140) — rochas/props deslocados verticalmente |
| V1-C29 | WARN | 0 | 9 (regressão) | PlayerStart, Ankylo_001, Ankylo_Savana_001 FORA do navmesh — navmesh desactualizado/não cobre zonas actuais |
| R27r | WARN | [MinPlayableMap] | [] | Alteração não esperada no mapa desde baseline |
| R24r | SKIP | null | 79.7 | não medido nesta corrida |
| R26 | MANUAL | null | "presente" | requer verificação manual |
| R28r | DELEGADO | null | watchdog/kill_switch | fora do meu escopo |
| V1-C28 | WARN | hash sanity_guard mudou | hash base | ficheiro de guarda alterado — investigar autoria |

Nota: R18 e R36 subiram ligeiramente vs. ciclos anteriores (R18: 27→30→33→30 oscilante; R36 nova entrada nesta lista com +2). V1-C29 é o alerta mais grave — regressão de 9 para 0, sugere navmesh não reconstruído após alterações recentes de terreno/actores.

## Feedback Visual do último screenshot
Cena sobre-exposta (céu branco, sem atmosfera Cretácica, sem contraste). PRIORITY FIX: corrigir exposição/luz — mas **R04 e R05r (sol/SkyLight) são BLOCKER e já resolvidos por decisão anterior (12000→60000 lux + SkyLight verde 3.5)** — não tocar nesses valores. O screenshot é de 09/08, anterior à correcção de exposição de 06/08 confirmar se já reflecte o estado actual; se a imagem for antiga, ignorar. Não vou instruir ninguém a mexer no sol/SkyLight (proibido).

## Atribuição de Tarefas (uma entrega mensurável por agente)

- **#2 Engine Architect**: verificar a alteração de hash em V1-C28 (sanity_guard.py mudou) e confirmar que R27r (mudança inesperada em MinPlayableMap) não introduziu regra fora da arquitectura aprovada. Entregável: relatório de causa, sem mexer no jogador/terreno.
- **#3 Core Systems**: NADA este ciclo — sem tarefa dentro do mandato que mova um check nomeado.
- **#4 Performance Optimizer**: NADA este ciclo — sem WARN/FAIL de performance nesta leitura.
- **#5 Procedural World Generator**: investigar e corrigir V1-C29 — reconstruir o RecastNavMesh (RebuildNavigation) para cobrir PlayerStart e Ankylo_001/Ankylo_Savana_001, subindo V1-C29 de 0 para pelo menos 9 (baseline). Não tocar Landscape nem sublevel Terrain_Savana.
- **#6 Environment Artist**: reduzir R18 de 30 para ≤27, reassentando pela base dos bounds (não pela origem) os 3 nomeados: Veg_Jungle_028, Veg_Jungle_053, Veg_Jungle_026. Sem novas decorações (tecto R05 já em 986, não subir).
- **#7 Architecture & Interior**: reduzir R36 de 17 para ≤15, reancorando ao terreno os 3 nomeados: Rock_Savana_070, EnvProp_ForestClearing_ImportedMatch_001, Rock_Hub_006 (line trace ao Landscape, ignorando não-Landscape).
- **#8 Lighting & Atmosphere**: NADA este ciclo — sol/SkyLight são BLOCKER R04/R05r, resolvidos, intocáveis.
- **#9 Character Artist**: prioridade real deste ciclo — substituir os 9 CrowdNPC ainda representados por esferas /Engine por uma malha humana real do pack disponível (não SKM_Manny/Quinn genérico sem propósito, usar variação por secções de material como já feito nos 4 hub placeholders). Entregável: 9 CrowdNPC com malha real, reportado por nome.
- **#10 Animation**: NADA este ciclo — sem malha nova nos CrowdNPC ainda, sem trabalho de animação a fazer até #9 entregar.
- **#11 Dino Behavior Agent**: gap real nº1 — implementar comportamento real (via callback de post-tick registado no editor, não BehaviorTree vazio) para pelo menos 3 dinossauros na banda 0-50m do spawn (A03 actual mínimo 3, mas sem IA real). Entregável: 3 criaturas com estado idle/walk dirigido por lógica, não apenas anim_to_play estático.
- **#12 Dino Combat AI**: NADA este ciclo — depende de #11 entregar comportamento base primeiro.
- **#13 Herd Simulation**: NADA este ciclo — sem tarefa nova de manada nesta leitura; C01 e composição já medidos, aguardar.
- **#14 Survival Objectives**: reduzir R17 (13 ocorrências no backlog diferido) — converter 3 desses casos em objectivos a pé sem NPC. Entregável: 3 objectivos nomeados com check R17 a descer.
- **#15 Narrative & Dialogue**: reduzir R20 (43 ocorrências, maior do backlog) — escrever/corrigir 5 entradas de lore/diálogo que causam R20, sem introduzir NPCs humanos novos.
- **#16 Audio**: NADA este ciclo — sem check de áudio nomeado nesta leitura.
- **#17 VFX**: teto de decoração já em 986 — NADA de novo; se quiser contribuir, melhorar VFX existentes sem novos actores (R31=1 ocorrência, opcional).
- **#18 QA & Testing**: validar após #5/#6/#7/#9/#11 reportarem, focando em V1-C29, R18, R36 e a contagem de CrowdNPC.
- **#19 Integration & Build**: aguardar entregas acima; não integrar build enquanto V1-C29 (navmesh) estiver a 0 — é regressão que pode quebrar navegação.

## O que NÃO peço este cycle
Não peço nada a #4 (Performance), #8 (Lighting/Atmosphere — sol/SkyLight são BLOCKER intocável), #10 (Animation — sem input de #9 ainda), #12 (Combat AI — depende de #11), #13 (Herd — sem tarefa nova), #16 (Audio — sem check nomeado), e reduzo #17 (VFX) a manutenção sem novos actores, porque o tecto de decoração (986) já foi atingido e qualquer decoração nova bloqueia o ciclo.

## Decisões técnicas
- Não movi nenhum Dino_* (proibido, medido por V1-C25/C25b).
- Não toquei sol/SkyLight (R04/R05r BLOCKER, resolvidos).
- Priorizei V1-C29 (regressão de 9→0) como o mais urgente por ser navegação, que sustenta qualquer IA futura de #11/#12/#13.
- Backlog diferido priorizado por ocorrências: R20(43) > R18(30, activo) > R17(13) — ataquei R20 e R17 este ciclo via #15/#14, e R18 via #6, deixando R37/R36 parcialmente (R36 activo este ciclo via #7).
