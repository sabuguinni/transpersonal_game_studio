# Combat & Enemy AI — Combat State Tags v1
Agent #12 (Combat & Enemy AI) — PROD_CYCLE_AUTO_20260720_003

## Contexto
Recebido do Agent #11 (NPC Behavior): 242 dinossauros no núcleo jogável já tinham
tags `Behavior_Species_{TRex|Raptor|Herbivore}` + parâmetros de território/aggro/attack.
Este ciclo adiciona a camada de **estado de combate tático** sobre essa base,
sem duplicar nenhuma tag existente.

## Auditoria dedup obrigatória (hugo_combat_label_consistency_v1)
Antes de criar qualquer `CombatZone_*` / `BehaviorTag_*`, foi feita busca real no mundo:
- **4 `CombatZone_*` existentes**: `CombatZone_Ambush_Canyon`, `CombatZone_Chokepoint_River`,
  `CombatZone_Generic`, `CombatZone_Triceratops`
- **1 `BehaviorTag_*` existente**: `BehaviorTag_Triceratops`

Nenhuma nova label `CombatZone_*`/`BehaviorTag_*` foi criada este ciclo — reuso confirmado,
zero duplicação. Em vez disso, o trabalho real foi feito via **tags de actor** (`Combat_State_*`),
que não colidem com o padrão de labels vigiado pela regra de consistência.

## Tags de estado de combate aplicadas (242 actores, 726 novas tags)

| Espécie    | Contagem | Estado          | Pack Hunt | Comportamento extra          |
|------------|----------|-----------------|-----------|-------------------------------|
| TRex       | 154      | `Combat_State_Ambush`     | False | `Combat_RoarOnAggro_True` — ruge antes de atacar, dando ao jogador uma janela de reação |
| Raptor     | 86       | `Combat_State_Alert`      | True  | `Combat_FlankBehavior_True` — comportamento de matilha (flanking) sobre a base do Agent #11 |
| Herbivore (Triceratops) | 2 | `Combat_State_Defensive` | False | `Combat_ChargeCounter_True` — carga apenas quando o jogador entra no `AttackRange`, nunca predatório |

Verificação: 154/86/2 confirmados por leitura pós-escrita das tags no mesmo ciclo (sem confiar em contagem de escrita).

## Filosofia de design aplicada
- **T-Rex**: ambush + roar telegrafado — o jogador tem um sinal claro (som) antes do dano,
  coerente com "perder por erro, não por injustiça".
- **Raptor**: pack hunting + flanking — usa a base de território já definida pelo Agent #11
  (`Behavior_TerritoryCenter_x_y`, aggro 2200) para permitir múltiplos raptors a cercar o jogador
  em vez de atacarem em fila (padrão Naughty Dog: inimigos que reagem entre si).
- **Triceratops**: puramente defensivo, carga apenas dentro do `AttackRange_150` já definido —
  não é um predador, é um risco de ambiente que pune agressão do jogador.

## Estado técnico
- Nenhum `.cpp`/`.h` foi criado (regra `hugo_no_cpp_h_v2`, ABSOLUTE, imp:20) — as tags de actor
  são a implementação real e verificável neste editor headless; lógica de BT/EQS que lê estas
  tags deve ser implementada via Blueprint quando o pipeline de Blueprint estiver disponível.
- Nível salvo (`save_current_level() == True`) após verificação pós-escrita.

## Pendente / handoff para #13 Crowd & Traffic Simulation
- As tags `Combat_PackHunt_True` + `Behavior_TerritoryCenter_*` dos 86 raptors são a base pronta
  para simulação de matilha em massa (Mass AI) — #13 pode agrupar raptors por proximidade de
  território para formar grupos de patrulha coordenados.
- Apenas 2 Triceratops taggeados no núcleo — se #13 espalhar mais herbívoros, devem herdar o
  mesmo padrão `Behavior_Species_Herbivore` + `Combat_State_Defensive` antes de entrarem em
  simulação de manada.
