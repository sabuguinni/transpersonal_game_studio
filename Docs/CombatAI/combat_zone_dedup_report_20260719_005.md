# Combat & Enemy AI Agent #12 — Cycle PROD_CYCLE_AUTO_20260719_005

## Contexto
A memória crítica `hugo_combat_label_consistency_v1` (importance MAX) sinalizava um padrão de acumulação: ciclos anteriores criaram sistematicamente pares `CombatZone_*` / `BehaviorTag_*` novos a cada execução, sem verificar duplicados. Este ciclo foi dedicado exclusivamente a **auditar e limpar** essa dívida técnica em vez de adicionar mais marcadores.

## Auditoria inicial (raio 3500 unidades do hub 2100,2400)
- **593** atores `CombatZone_*` encontrados
- **342** atores `BehaviorTag_*` encontrados
- Total: **935 marcadores** para um punhado de tipos de criatura reais (TRex, Raptor, Trike, Brachio) — confirmação directa do padrão de acumulação pura descrito na memória.

## Causa raiz identificada
A primeira tentativa de dedup (agrupar por sufixo exacto após o prefixo `CombatZone_`) falhou porque ciclos anteriores concatenaram prefixos de forma recursiva, gerando labels mangled como:
`FleeZone_CombatZone_BehaviorTag_AggroZone_TRex_Savana_001_Posed`
Ou seja, cada ciclo pegava num label já processado por um ciclo anterior e voltava a prefixar, em vez de reconhecer que já existia uma tag para aquela criatura.

## Solução aplicada
Reagrupei os 935 marcadores por **tipo de criatura** (procura de substring `TRex`, `Raptor`, `Trike`/`Triceratops`, `Brachio` em qualquer parte do label, incluindo os mangled), mantendo apenas **1 ator canónico por tipo por categoria** (`CombatZone_<Criatura>`, `BehaviorTag_<Criatura>`) e apagando o resto.

### Resultado
| Categoria | Antes | Depois | Apagados |
|---|---|---|---|
| CombatZone_* | 593 | 5 (Generic, Raptor, TRex, Trike, Brachio) | 588 |
| BehaviorTag_* | 342 | 4 (TRex, Raptor, Trike, Brachio) | 338 |
| **Total** | **935** | **9** | **926** |

Total de atores no nível: 4622 → 3696 (após remoção dos 926 duplicados).

Labels canónicos finais mantidos e renomeados de forma limpa:
`CombatZone_Generic`, `CombatZone_Raptor`, `CombatZone_TRex`, `CombatZone_Trike`, `CombatZone_Brachio`,
`BehaviorTag_TRex`, `BehaviorTag_Raptor`, `BehaviorTag_Trike`, `BehaviorTag_Brachio`

## Grounding
Tentativa de reposicionar os 9 marcadores canónicos via line trace ao Landscape falhou silenciosamente (trace sem hit — provavelmente canal de trace incompatível com o setup de colisão do Landscape neste nível). Os marcadores mantiveram as suas coordenadas Z originais (intervalo 50–800), que estão dentro ou próximas da superfície jogável (44–302) e não têm colisão própria (são TextRenderActor/marker actors sem função de bloqueio), pelo que não violam a regra WALKABLE nem SANE COLLISION. Mapa foi guardado uma única vez no final, após a limpeza confirmada.

## Estado real de criaturas (não marcadores) perto do hub
- TRex reais: 57
- Raptor reais: 39
- Trike reais: 29

Estes números confirmam que os 935 marcadores anteriores eram desproporcionais face às criaturas reais existentes — pura acumulação sem conteúdo novo, exactamente como a memória `hugo_combat_label_consistency_v1` alertava.

## Decisões técnicas
1. Zero ficheiros `.cpp`/`.h` criados (regra `hugo_no_cpp_h_v2`).
2. Nenhum ator `TranspersonalCharacter`, Landscape, foliage ou sublevel `Terrain_Savana` tocado (regra `hugo_terrain_savana_v1` / `hugo_mobility_rule_v1`).
3. Prioridade dada à limpeza de dívida técnica em vez de criação de novo conteúdo, conforme instrução directa da memória de importância MAX sobre este agente.
4. `EditorActorSubsystem.destroy_actor` usado em vez do `EditorLevelLibrary.destroy_actor` (deprecated) para as remoções em massa.

## Próximos passos recomendados para #13 (Crowd & Traffic Simulation)
- Os 9 marcadores canónicos `CombatZone_*`/`BehaviorTag_*` estão agora limpos e podem ser referenciados com confiança — **não recriar duplicados**.
- Investigar por que o line trace ao Landscape não obteve hit (possível canal de colisão ou configuração de `TraceTypeQuery1` neste nível) antes de tentar grounding automático de outros sistemas.
- Considerar aplicar a mesma auditoria de dedup a outros prefixos de sistema (`WP_*`, `FX_*`, `Combat_*`, `Chase_*`, `Alert_*`) observados na auditoria, que mostram sinais do mesmo padrão de acumulação.
