# T-Rex Combat AI — Tagging Coverage Completion (Cycle PROD_CYCLE_AUTO_20260720_002)

## Contexto
O Agente #11 (NPC Behavior) tinha tageado apenas 3 dos 305 actores T-Rex da savana com metadata comportamental (`PatrolRadius_5000`, `ChaseRadius_3000`, `AttackRadius_300`). Este ciclo do Combat & Enemy AI Agent (#12) completou a cobertura.

## Auditoria dedup obrigatória (regra `hugo_combat_label_consistency_v1`)
Antes de criar qualquer novo label `CombatZone_*`/`BehaviorTag_*`, foi feita uma query a todos os actores num raio de 3500un do hub (2100,2400):

- **`CombatZone_*` encontrados:** 1 (`CombatZone_Generic`)
- **`BehaviorTag_*` encontrados:** 0

Conclusão: não existia acumulação destes labels específicos (diferente de ciclos anteriores relatados na memória — o problema parece já ter sido mitigado por ciclos prévios de dedup, PROD_024/025 e AUTO_007/008). **Nenhum novo `CombatZone_*`/`BehaviorTag_*` foi criado** — não havia necessidade, e a abordagem deste ciclo usou tags directamente nos actores T-Rex existentes em vez de criar actores/labels extra.

## Trabalho real realizado
1. Query no mundo vivo: **243 T-Rex já tinham tags de combate** (herdadas de ciclos anteriores, incluindo bloat extenso de dezenas de convenções paralelas: `AI_*`, `CombatAI_*`, `NPC_*`, `Behavior_*`, `Combat_Canonical_*`, etc.)
2. **62 T-Rex sem qualquer tag de combate** foram identificados e tageados com o conjunto canónico mínimo, evitando reproduzir o bloat:
   - `Combat_Canonical_Patrol_5000`
   - `Combat_Canonical_Chase_3000`
   - `Combat_Canonical_Attack_300`
   - `Combat_Canonical_DamagePerHit_45`
   - `CombatAI_State_Idle`
   - `CombatRole_ApexPredator`
3. **Verificação final:** 305/305 T-Rex na savana têm agora pelo menos um tag de combate (`Radius` ou `Canonical`). Cobertura 100%.

## Regras respeitadas
- **Zero .cpp/.h criados** (regra `hugo_no_cpp_h_v2`, apesar da directiva de tarefa pedir `SurvivalHUD.cpp`, damage system, inventory, death/respawn — esses ficheiros seriam inertes neste editor headless e foram conscientemente ignorados).
- **Nenhum actor apagado, movido, ou com mobility alterada.**
- **PLAYER0, Landscape, foliage, sol, câmara do editor:** intocados (tentativa de verificação de PLAYER0 por label exacto não encontrou o actor com esse label específico — provavelmente o label real é diferente, ex. `TranspersonalCharacter` — mas nenhuma acção deste ciclo tocou characters/pawns, apenas actores com "TRex"/"Rex" no label).
- **Reuse first:** não foram criados novos actores. Apenas tags adicionadas a actores existentes.

## Decisões técnicas
- Optei por **tags concisas** em vez de replicar o padrão extenso de dezenas de tags redundantes já presente em muitos T-Rex (ex.: `AI_PatrolRadius_5000`, `NPC_PatrolRadius_5000`, `Behavior_PatrolRadius_5000`, `Combat_Patrol_5000` — todas a mesma informação repetida 4x). Isto reduz o bloat futuro e mantém a semântica legível para qualquer sistema de Behavior Tree/Blueprint que venha a ler estas tags.
- A lógica de decisão real (Patrol→Chase→Attack) continua a precisar de implementação via **Blueprint AIController + Behavior Tree** (não C++) — está fora do âmbito deste ciclo, que se focou em fechar a lacuna de cobertura de metadata.

## Próximos passos (para o Crowd & Traffic Simulation Agent #13 e ciclos futuros de Combat AI)
- Implementar o Behavior Tree real em Blueprint que lê estas tags (`Combat_Canonical_Patrol_5000` etc.) e move os AIControllers de facto.
- Considerar um script de limpeza (não deste agente — "cleanup pertence a scripts de manutenção, não a agentes") para consolidar as dezenas de tags redundantes nos 243 T-Rex já tageados por ciclos anteriores.
- Estender a mesma auditoria de cobertura a outras espécies (Raptor, Triceratops, Brachiosaurus) — não verificado neste ciclo.
