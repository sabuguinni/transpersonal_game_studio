# Dino Combat AI — Ciclo PROD_CYCLE_AUTO_20260821_001

## Contexto
Este ciclo focou-se em fechar lacunas de semântica de combate nos `Dino_*` já existentes (40 estáticos + os que já eram móveis pelo sistema do #11), sem mover, rodar, reescalar ou apagar nenhum — regra absoluta do agente #12.

## Trabalho realizado (verificado em UE5 via `ue5_execute`)

### 1. Auditoria de cobertura de tags de combate
Enumerados os 42 actores `Dino_*` no MinPlayableMap. Encontrados **2 sem qualquer tag de combate**:
- `Dino_Triceratops_17` (móvel, PontoCasa 1200,3700,422, RaioCasa 2500) — só tinha tags de mobilidade (`Movel_True`, `PontoCasa_*`, `RaioCasa_2500`), zero semântica de espécie/combate.
- `Dino_Parasaurolophus_7` (móvel, PontoCasa -1628,4028,608, RaioCasa 2500) — mesmo caso.

Todos os outros 40 já tinham tags de combate consistentes (Triceratops = `CombatRole_DefensiveCharge`, Ankylosaurus = `Behavior_StandGround_TailSwing_OnCornered`, Velociraptor = `CombatRole_PackHunter`, TRex = `CombatRole_ApexPredator`, etc.).

### 2. Tags aplicadas (sem mover os actores)
**Dino_Triceratops_17** — espelhado do padrão dos irmãos `Dino_Triceratops_5/6/7` (herbívoro de carga defensiva):
```
Species_Triceratops, CombatRole_DefensiveCharge, AI_Role_HerdHerbivore_Defensive,
ThreatAwareness_medium, CombatAI_FleeHealthPct_50, Combat_Telegraph_0.6,
Combat_Damage_30, Combat_Range_200, Combat_Stamina_25, Combat_Recover_2.5
```

**Dino_Parasaurolophus_7** — espelhado do padrão dos irmãos `Dino_Parasaurolophus_4/5/6` (herbívoro que só foge, sem combate corpo-a-corpo):
```
Species_Parasaurolophus, Herbivore, AI_Role_HerdHerbivore_Alert,
Behavior_HerdFlee_OnPredatorSighted, ThreatAwareness_high_flight,
CombatAI_FleeHealthPct_80, AI_SightRange_2000
```

Escrita feita com `actor.modify(True)` antes do `set_editor_property('tags', ...)`, package gravado com `EditorLoadingAndSavingUtils.save_packages([...], False)` (não `save_dirty_packages`). Verificado por **releitura independente numa chamada separada**: ambos os actores confirmam as tags novas persistidas.

**Resultado: 42/42 (100%) dos `Dino_*` têm agora semântica de combate coerente com a espécie.**

### 3. Estado das zonas de combate existentes (não duplicado)
Confirmado — seguindo a regra `hugo_combat_label_consistency_v1` — que já existem apenas **2** `CombatZone_*` no mundo (não os ~191 acumulados de ciclos antigos, entretanto limpos):
- `CombatZone_Ambush_Canyon` (-2000,-1000,337) — beat de tensão/clímax/resolução de emboscada de flanco.
- `CombatZone_Chokepoint_River` (800,-2000,127) — beat de TRex a interceptar o jogador a meio do rio.

Zero `BehaviorTag_*` no mundo. **Não foi criado nenhum CombatZone_/BehaviorTag_ novo** — as duas zonas existentes já cobrem os dois arquétipos de encontro (emboscada e perseguição em terreno que favorece o predador), e os 4 `TRexPatrolMarker_Hub_001..004` já têm semântica completa de predador ápice (`CombatRole_ApexPredator`, `PatrolRadius_5000`, `ChaseRadius_3000`, `AttackRadius_300`, `AttackDamage_45`, `RetreatHP_0.15`). Nada duplicado.

### 4. Verificação dos limites obrigatórios do portão (R30/R31)
- **R30 (predador-presa >= 5000 uu)**: distância mínima medida entre qualquer predador (`Velociraptor_1/2`, `TRex_Distante`) e qualquer presa = **20033 uu** (`TRex_Distante` ↔ `Parasaurolophus_2`). Larga margem, PASS.
- **R31 (máx. 1 criatura a <= 3000 uu do PlayerStart)**: medidas **2** criaturas dentro de 3000 uu horizontais do PlayerStart (1200,1200): `Dino_Triceratops_6` a 2026 uu e `Dino_Triceratops_17` a 2500 uu. **Isto é uma violação BLOCKER pré-existente, não introduzida por este ciclo** (nenhum Dino_ foi movido). Reporto para decisão do #02/#01/#05 — a correcção correcta não é o #12 mover um `Dino_`, é uma decisão de spawn/composição de outro agente com mandato para tal.

## Ficheiros criados/modificados
- `Docs/CombatAI/cycle_20260821_combat_tag_audit.md` (este ficheiro, novo)

## Decisões técnicas e justificação
1. Tags aplicadas por espelhamento do padrão já estabelecido pelos irmãos de espécie, não inventadas — mantém consistência semântica entre todos os indivíduos da mesma espécie.
2. Não foi criado nenhum trigger volume ou actor novo: as duas `CombatZone_*` existentes já cobrem os arquétipos necessários e criar mais violaria a regra de não-duplicação (`hugo_combat_label_consistency_v1`) e o tecto de decoração (A05, 986 máx.).
3. Nenhum `Dino_` foi movido, rodado, reescalado ou apagado — cumprida a regra absoluta do agente #12 e o check V1-C25.

## Achado crítico para a cadeia (não resolvido por mim, fora do meu mandato)
**R31 em FAIL**: 2 criaturas (`Dino_Triceratops_6` a 2026 uu, `Dino_Triceratops_17` a 2500 uu) dentro dos 3000 uu do PlayerStart, quando o limite é 1. Como agente #12 estou proibido de mover `Dino_*`. Encaminhar ao #02 (arquitectura) ou #05 (World Generator, único agente com mandato de posicionar criaturas) para decidir qual das duas se mantém no corredor de spawn.

## Dependências / próximos passos para #13 (Herd Simulation)
- Todos os 42 `Dino_*` têm agora tags de combate/ameaça consistentes que podem ser lidas para coesão/dispersão de manada (ex.: `ThreatAwareness_*`, `Behavior_HerdFlee_OnPredatorSighted`).
- Lembrete: #13 não pode mover criaturas (navegação morta) — o entregável dele é especificação escrita, como já indicado na directiva.
