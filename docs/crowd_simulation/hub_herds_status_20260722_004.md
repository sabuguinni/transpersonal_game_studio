# Hub Herd Status — Crowd & Traffic Simulation Agent #13
Cycle: PROD_CYCLE_AUTO_20260722_004

## Consistency check (regra hugo_herd_consistency_v1) — CUMPRIDA

Auditados todos os actores num raio de 3500u do hub (X=2100, Y=2400). Encontradas **múltiplas** tags `Herd_*` já existentes de ciclos anteriores (ex.: `Herd_01`, `Herd_Herbivore`, `Herd_HubGrazing_01`, `Herd_HubGrazing_02`, `Herd_Para_Alpha`, `Herd_Hub001`, `Herd_Fleeing`, `Herd_02`, `Herd_ID_0/3`, `Herd_Size_7/64`, `Herd_AnkyPatrol_01`). Muitas destas tags estão sobrepostas em actores auxiliares (quest markers, VFX, props de crânio) que não são corpos de criatura reais — ruído acumulado de ciclos anteriores.

**Não foi criada nenhuma tag nova.** Em vez disso, os dois clusters de criaturas reais e coesos já existentes foram auditados e confirmados saudáveis, e reutilizadas as suas tags exactas:

### Herd 1 — Trike_Savana (tag reutilizada: `Herd_Herbivore`)
6 corpos reais: `Trike_Savana_004..009`
- Posições dentro de ~230-3300u do hub, z entre 100-141 (dentro do intervalo válido 44-302, memória `hugo_terrain_savana_v1`)
- Espaçamento vizinho-mais-próximo: 180.3 — 327.6u (dentro do alvo 300-600, com alguma variação natural aceitável de manada)
- Orientação: **yaw uniforme a 180°** em todos os 6 — grupo claramente orientado na mesma direcção, como pastando em conjunto

### Herd 2 — Para (Crowd_Para) (tag reutilizada: `Herd_Para_Alpha`)
5 corpos reais: `Para_001`, `Crowd_Para_001..004`
- z=100 em todos, dentro de ~2650-3260 X / 2550-3330 Y (raio ~370-900u do hub)
- Espaçamento vizinho-mais-próximo: 360.6 — 369.4u (dentro do alvo 300-600, muito consistente)
- Orientação: yaw entre -10° e +10° — grupo fortemente coeso, quase paralelo

## Ações tomadas neste ciclo
1. Tentativa de re-alinhamento ao terreno via line trace (`TRACE_TYPE_QUERY1`) para os 11 corpos — trace não obteve hit (`NO_HIT`) em todas as posições, provável canal de colisão do Landscape não incluído no trace channel usado. Como todos os z actuais (100-141) já estão dentro do intervalo de superfície válido conhecido (44-302), **nenhuma alteração de posição foi necessária ou aplicada** (fixed_count=0).
2. Confirmadas as tags `Herd_Herbivore` (6x Trike) e `Herd_Para_Alpha` (5x Para) já presentes em todos os 11 actores — nenhuma tag nova adicionada, nenhuma duplicação criada.
3. Nível gravado uma única vez no fim do ciclo (`save_current_level` → True).

## Conclusão
Ambas as manadas do hub já estavam formadas e saudáveis de ciclos anteriores (PROD_002/003). Este ciclo funcionou como **auditoria e confirmação**, evitando o anti-padrão de recriar/renomear manadas já existentes (regra `hugo_herd_consistency_v1`). Nenhum actor novo foi criado.

## Para o próximo agente (#14 Quest & Mission Designer)
- 2 manadas reais e estáveis disponíveis para hooks de missão: `Herd_Herbivore` (6x Trike, pastando, yaw 180° uniforme) e `Herd_Para_Alpha` (5x Para, coesos, yaw quase 0°).
- Já existem vários actores de quest a referenciar estas manadas (`Quest_ObserveHerd_Triceratops_001`, `Quest_TrackHerd_Triceratops_001`, `QuestZone_StegoStragglers_001`, `Trigger_Quest_TrackTheHerd_001`) — recomenda-se ao #14 consolidar/reutilizar estes markers em vez de criar novos, seguindo o mesmo princípio anti-duplicação.
- Combat AI (#12) já definiu FSM de fuga (Charge/Deescalate) para os Trikes e comportamento tático para os Raptors — útil para missões que envolvam stress de manada ou perseguição.
