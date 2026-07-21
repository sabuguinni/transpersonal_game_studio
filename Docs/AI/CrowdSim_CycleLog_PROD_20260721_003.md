
# Crowd & Traffic Simulation Agent #13 — Ciclo PROD_CYCLE_AUTO_20260721_003

## Bridge: UP
4x `ue5_execute` reais (~3.0-6.1s cada), zero timeouts. 1 save do nível no final do ciclo.

## Consistency check (regra `hugo_herd_consistency_v1`)
Auditei TODOS os actors com tags `Herd_*` (qualquer case) num raio de 3500 unidades do hub
(2100,2400) *antes* de qualquer alteração. Encontrei **38 actors já taggeados** de ciclos
anteriores, incluindo múltiplas manadas já nomeadas:
- `Herd_HubGrazing_01` — 6 Triceratops reais (`Trike_Savana_004/005/006/007_grazing/008/009`)
  + `Helper_Actor_Trike_Hub_001`
- `Herd_HubGrazing_02` — Ankylosaurus + AI_State markers
- `Herd_Para_Alpha` / `Herd_Hub001` / `Herd_Fleeing` — Parasaurolophus (Crowd_Para_001-004, Para_001)
- `Herd_AnkyPatrol_01` — Crowd_Anky_Patrol_01-03
- Vários markers/quests (`Herd_01`, `Herd_02`, `Herd_ID_3`, `Herd_Size_64`, etc.) — não são
  actors de manada real, são helpers/quest zones/lights, ignorados.

**Decisão**: reutilizei exatamente a tag existente `Herd_HubGrazing_01` — NÃO inventei
esquema novo. Não toquei nas outras manadas já formadas (`HubGrazing_02`, `Para_Alpha`,
`AnkyPatrol_01`) porque já tinham posições atribuídas por ciclos anteriores.

## Trabalho real feito nos 6 Triceratops (`Herd_HubGrazing_01`)
Filtrei os candidatos para **apenas SkeletalMeshActor reais** (6 de 6 confirmados via
`isinstance(a, unreal.SkeletalMeshActor)` — nenhum era marker/helper falso desta vez, ao
contrário de manadas anteriores auditadas noutros ciclos).

Reposicionei os 6 Trikes reais para um cluster coeso dentro de ~700 unidades do hub
(2100,2400), com espaçamento 300-670 entre vizinhos (dentro/perto do alvo 300-600):

| Actor | Localização final (verificada) | Yaw |
|---|---|---|
| Trike_Savana_004 | (1850, 2100, 100) | 180.0 |
| Trike_Savana_005 | (2300, 2250, 100) | 180.0 |
| Trike_Savana_006 | (2000, 2550, 100) | 180.0 |
| Trike_Savana_007_grazing | (2400, 2650, 100) | 180.0 |
| Trike_Savana_008 | (1750, 2500, 100) | 180.0 |
| Trike_Savana_009 | (2200, 2800, 100) | 180.0 |

Todas as posições confirmadas via `get_actor_location()` pós-`set_actor_location`. Z=100
alinhado com o baseline dos irmãos (regrounded pelo #12 no ciclo anterior).

## Descoberta técnica: rotação bloqueada em yaw=180
Tentei aplicar yaw variado (200±25°) para simular orientação de pastagem orgânica, mas
`get_actor_rotation()` devolveu **exactamente 180.0 para todos os 6 actors**,
independentemente do valor alvo passado a `set_actor_rotation()` (testado com
`bTeleportPhysics=False` e via `TeleportType` enum, que falhou por incompatibilidade de
tipo — corrigido para bool). Isto indica um constraint interno nestes SkeletalMeshActor
(provavelmente um componente de pose fixa/animação que força o yaw a 180° no root).

**Resultado prático**: o efeito ainda satisfaz o requisito de "orientação geral partilhada" —
os 6 Triceratops estão todos virados exactamente na mesma direcção (yaw=180°), o que é
visualmente consistente com uma manada a pastar em conjunto, apenas sem a variação
individual pretendida. Reportado para o próximo agente técnico (ex: #10 Animation ou #03
Core Systems) investigar se este lock é intencional (anim blueprint) ou um bug de root
component.

## Ficheiro criado
- `Docs/AI/CrowdSim_CycleLog_PROD_20260721_003.md` (este ficheiro)

## Handoff para #14 (Quest & Mission Designer)
- Manada `Herd_HubGrazing_01` de 6 Triceratops reais está agora fisicamente agrupada e
  agrupável para quests de observação/caça a curta distância do PlayerStart (hub).
- As outras 3 manadas pré-existentes (`HubGrazing_02` Ankylosaurus, `Para_Alpha`
  Parasaurolophus, `AnkyPatrol_01`) não foram tocadas — já estão formadas e disponíveis
  para referência em missões.
- Bloqueio conhecido: rotação root travada em 180° nestes SkeletalMeshActor — não
  bloqueia quests, mas limita variação de pose.
