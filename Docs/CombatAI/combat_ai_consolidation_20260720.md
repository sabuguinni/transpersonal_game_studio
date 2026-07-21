# Combat & Enemy AI Agent #12 — Ciclo PROD_CYCLE_AUTO_20260720_001

## Bridge: UP
4x `ue5_execute` (`python`), todos `completed`, sem timeouts (~3.0-6.1s cada). Zero `.cpp`/`.h` escritos (regra `hugo_no_cpp_h_v2`, imp:20).

## Contexto
Handoff do NPC Behavior Agent #11: tags canónicas de T-Rex já consolidadas (16 tags limpas). Raptor ainda tinha 58 tags acumuladas e redundantes (várias métricas de combate duplicadas com prefixos diferentes: `AttackRange_200` vs `CombatAI_AttackRange_180` vs `CombatAI_AttackRange_300`, três valores de chase range em conflito, etc.). Triceratops (`Trike_Hub_001`) não tinha perfil de combate — apenas tags genéricas de outros sistemas.

## Auditoria dedup obrigatória (regra `hugo_combat_label_consistency_v1`)
Query a atores com labels `CombatZone_*`/`BehaviorTag_*` num raio de 3500 unidades do hub (2100,2400):
- **CombatZone_* encontrados: 5** (suffixes: Trike, Generic, Brachio, TRex, Raptor)
- **BehaviorTag_* encontrados: 4** (suffixes: Trike, Brachio, TRex, Raptor)
- **Novos labels CombatZone_/BehaviorTag_ criados este ciclo: 0** — trabalho feito por atualização de tags em atores existentes, não por criação de novos marcadores, conforme recomendação da memória crítica.

## Mudanças reais no mundo vivo

### 1. Raptor_Hub_001_Posed (x=2200, y=2200, z=111.7)
- Tags reduzidas de **58 → 16**, removendo duplicação de métricas conflituosas (ex.: 3 valores diferentes de chase range, 3 de attack range, tags de VFX/Audio fora de escopo deste sistema).
- Conjunto canónico final: `AISpecies:Raptor`, `AIRole:Flanker`, `AIDiet:Predator`, `Behavior_PackHunter`, `AI_Role_PackHunter_Flanker`, `PackID_RaptorSquad_A`, `SightRadius_3500`, `AggroRadius_1200`, `ChaseRange_2000`, `AttackRange_200`, `AttackDamage_25`, `AtkCooldown_1.2`, `RetreatHP_0.20`, `CombatTactic_FlankAndSurround`, `AI_State_Patrolling`, `QA_Grounded_Verified`.
- Perfil tático: caçador de matilha que flanqueia (`FlankAndSurround`), foge com a matilha reduzida a <2 elementos e recua abaixo de 20% HP — coerente com o comportamento passado pelo Agent #11.

### 2. Trike_Hub_001 / Actor_484 (x=2050, y=2650, z=80.1)
- Adicionado perfil de combate defensivo de herbívoro (60 tags totais após merge, mantendo as 53 pré-existentes de outros sistemas + 11 novas, sem duplicar as já presentes):
  `AISpecies:Triceratops`, `AIRole:Defensive`, `AIDiet:Herbivore`, `Behavior_GrazePassive`, `CombatTactic_ChargeWhenCornered`, `AggroRadius_600`, `ChargeRange_800`, `AttackDamage_60`, `AtkCooldown_3.0`, `RetreatHP_0.0`, `FleeThreshold_None_StandGround`.
- Design: Triceratops não foge (não é presa fácil) — pasta passivamente até ser encurralado, depois carrega com dano alto (60) mas cooldown lento (3s), sem threshold de fuga. Contraste ecológico deliberado com os predadores (T-Rex ambush, Raptor flanking).

## ⚠️ Achado crítico para QA/dedup (não corrigido este ciclo — fora do meu escopo de "não apagar")
Existem **2 atores diferentes com o label `Trike_Hub_001`**:
- `Actor_484` — loc (2050, 2650, 80.1) — recebeu o perfil de combate acima (60 tags).
- `StaticMeshActor_411` — loc (-425, 2620, 205.7) — actor pré-existente não relacionado, com 71 tags próprias, **não tocado**.
Isto é uma colisão de nomes (dois actores com o mesmo label, provavelmente de ciclos anteriores de outro agente), não uma duplicação criada por mim. Reportado para o QA Agent (#18) ou para uma futura passagem de limpeza — não apaguei nenhum actor per regra "não mass-delete", e confirmei explicitamente por query directa (`get_name()`) qual dos dois recebeu a edição real.

## Verificação
- Tags confirmadas persistidas via query directa pós-escrita (Raptor: 16 tags lidos de volta; Trike/Actor_484: identificado inequivocamente por `get_name()`).
- Z de ambos os atores dentro do intervalo válido do terreno (44-302): Raptor z=111.7, Trike/Actor_484 z=80.1.
- Nenhum actor `TranspersonalCharacter`, `Landscape`, foliage ou sublevel `Terrain_Savana` tocado.
- **1 save único no fim do ciclo** (`save_current_level() -> True`), após todas as verificações.

## Handoff — Crowd & Traffic Simulation Agent #13
- Perfis de combate canónicos agora limpos para T-Rex (Agent #11), Raptor e Triceratops (este ciclo). Usar estas tags como fonte única de verdade para simulação de massa (evitar recriar métricas de agressão/fuga — reutilizar `AggroRadius_*`, `RetreatHP_*`, `ChaseRange_*` já definidos).
- Reportar ao QA (#18) a colisão de label `Trike_Hub_001` (2 actores distintos, coords (2050,2650,80.1) e (-425,2620,205.7)) para resolução em passagem de limpeza dedicada.
- Ainda restam ~590+ dinossauros no núcleo jogável sem perfil de combate consolidado (apenas T-Rex, Raptor e 1 Triceratops tratados até agora); recomenda-se continuar o processo por espécie em ciclos futuros, sempre com dedup-first.
