# T-Rex Behavior Design Spec — NPC Behavior Agent #11
### Ciclo: PROD_CYCLE_AUTO_20260721_002

## ⚠️ Nota de conformidade (regra `hugo_no_cpp_h_v2`, importância MAX)
A directiva deste ciclo pedia a criação de `Source/TranspersonalGame/AI/TRexBehavior.cpp` via `github_file_write`.
Essa acção foi **deliberadamente recusada**: a regra global `hugo_no_cpp_h_v2` (importância MAX, sem excepções) proíbe
escrever ficheiros `.cpp`/`.h` neste editor headless — o binário é pré-compilado e nunca recompila, logo qualquer
`.cpp` novo tem **zero efeito** no jogo vivo e desperdiça o ciclo inteiro. Em substituição, o comportamento foi:
1. **Documentado aqui** como especificação de design (para quando a pipeline C++ voltar a compilar, ou para
   implementação equivalente em Blueprint/Behavior Tree pelo #12 Combat & Enemy AI Agent).
2. **Aplicado na cena viva via `ue5_execute`/Python** como tags de actor (metadata não-destrutiva), para que o
   próximo agente (#12) possa ler estes parâmetros directamente dos actors em vez de depender de um ficheiro morto.

## Verificações realizadas neste ciclo (via `ue5_execute`, mundo vivo)

### 1. Inventário de actors "TRex" em `MinPlayableMap`
- **58 actors** com "TRex" no label, mundo `MinPlayableMap` (3342 actors totais).
- Breakdown por classe: `NiagaraActor` (50), `Emitter` (2), `AmbientSound` (2), `StaticMeshActor` (4).
- **Confirmação repetida de ciclos anteriores**: não existe nenhum `SkeletalMeshActor`/`Character` real de T-Rex —
  apenas markers de VFX/áudio (`VFX_DustBurst_TRex_*`, `Audio_TRexProximity_*`) e 4 `StaticMeshActor` chamados
  `TRexPatrolMarker_Hub_001..004`, todos em `(x, y, z=100)` — coordenadas do hub `(2100, 2400)` conforme
  `hugo_hub_quality_v2_fix`.
- Não há geometria de T-Rex "posável" nesta cena — a lógica de comportamento abaixo aplica-se aos 4 markers de
  patrulha existentes e a qualquer actor T-Rex real que o #9/#6 venha a instanciar.

### 2. Tags de comportamento aplicadas (58 actors, não-destrutivo)
Todos os 58 actors "TRex" (incluindo os 4 `TRexPatrolMarker_Hub_*`) receberam as tags:
- `BehaviorProfile_TRex`
- `PatrolRadius_5000`
- `ChaseRadius_3000`
- `AttackRadius_300`

Estas tags são lidas por `unreal.Actor.tags` e podem ser consultadas por qualquer Behavior Tree / AIController
(Blueprint ou C++, quando recompilável) sem exigir um novo asset.

### 3. Verificação da SurvivalComponent (#3 Core Systems)
`Source/TranspersonalGame/Core/Survival/SurvivalComponent.h` **existe e está completo**: gere Health, Hunger,
Thirst, Stamina, Fear, Temperature, com delegates (`OnSurvivalStatsUpdated`, `OnPlayerDied`) e API Blueprint-callable
(`ApplyHealthDamage`, `ConsumeFood`, `AddFear`, etc.). É a componente correcta para o T-Rex disparar `AddFear` no
jogador quando entra no `ChaseRadius`.

### 4. Verificação do DinosaurCombatAIController (#12 Combat AI)
`Source/TranspersonalGame/AI/Combat/DinosaurCombatAIController.cpp` **existe no repositório mas está vazio/placeholder**
(9 bytes de conteúdo). Não há lógica de patrulha/chase/attack implementada ainda — é trabalho pendente do
**#12 Combat & Enemy AI Agent**, o próximo na cadeia.

## Especificação de comportamento — T-Rex (para implementação por #12 via Behavior Tree)

### Estados (Behavior Tree / State Machine)
1. **Patrol** (raio 5000 unidades em torno do ponto de spawn)
   - Selecciona pontos aleatórios dentro do raio, anda a passo lento (~150 uu/s), pausa a olhar em redor
     (comportamento territorial — Rockstar-style: o T-Rex "vive" a área mesmo sem jogador presente).
   - Transição para **Chase** se `Distance(Player, Self) < 3000`.
2. **Chase** (jogador dentro de 3000 unidades)
   - Corre direccionado ao jogador (~600 uu/s), dispara `AddFear` na `SurvivalComponent` do jogador via
     evento de proximidade (já existe `Audio_TRexProximity_001` e `VFX_Dust_TRexProximity_001` na cena para isto).
   - Transição para **Attack** se `Distance(Player, Self) < 300`.
   - Transição de volta para **Patrol** se `Distance(Player, Self) > 4000` (histerese para evitar oscilação).
3. **Attack** (jogador dentro de 300 unidades)
   - Ataque de mordida com telegraph (anticipation) + dano directo via `ApplyHealthDamage` na `SurvivalComponent`
     do jogador. Cooldown entre ataques (~2s) para permitir janela de fuga/combate ao jogador.
   - Transição para **Chase** se jogador sair do raio de ataque mas continuar dentro do raio de chase.

### Sociologia do comportamento (não é apenas uma BT — é a vida do T-Rex)
- O T-Rex **não existe para o jogador**: a patrulha decorre autonomamente, incluindo fora do alcance de percepção
  do jogador. Se o jogador nunca se aproximar, o T-Rex nunca entra em Chase/Attack — vive a sua área territorial.
- Reage ao inesperado: se for atacado durante Patrol, salta directamente para Attack (sem fase de Chase),
  reflectindo resposta de sobressalto realista de um predador território-defensivo.
- Não há memória de longo prazo do jogador specific (T-Rex não é um "personagem social" com relações — é fauna
  territorial). Sistemas de memória social aplicam-se antes a NPCs humanos, não a dinossauros.

## Ficheiros criados/modificados neste ciclo
- `Docs/NPCBehavior/TRexBehavior_DesignSpec_PROD_CYCLE_AUTO_20260721_002.md` (este ficheiro)

## Estado verificado no UE5 (ue5_execute, resultados reais)
- `MinPlayableMap` carregado, 3342 actors totais.
- 58 actors "TRex" tagueados com `BehaviorProfile_TRex`, `PatrolRadius_5000`, `ChaseRadius_3000`, `AttackRadius_300`.
- 4 `TRexPatrolMarker_Hub_001..004` confirmados em `z=100` (consistente com o hub `(2100,2400)=z100`).
- `SurvivalComponent.h` confirmado completo e funcional.
- `DinosaurCombatAIController.cpp` confirmado como existente mas vazio (placeholder, 9 bytes) — bloqueador para #12.

## Dependências / próximos passos para #12 (Combat & Enemy AI Agent)
1. `DinosaurCombatAIController.cpp` precisa de implementação real — mas **não via github_file_write** (regra
   `hugo_no_cpp_h_v2`); a lógica deve ser montada como Behavior Tree/Blueprint directamente no editor via
   `ue5_execute`, lendo as tags `PatrolRadius_5000`/`ChaseRadius_3000`/`AttackRadius_300` já aplicadas aos actors.
2. Nenhum T-Rex com geometria real (`SkeletalMeshActor`) existe ainda na cena — apenas markers/VFX. #12 deve
   confirmar com #9 (Character Artist) e #6 (Environment Artist) se há um asset T-Rex disponível para instanciar
   antes de tentar ligar IA a um mesh inexistente.
3. `SurvivalComponent` está pronta para receber chamadas de `AddFear`/`ApplyHealthDamage` vindas do comportamento
   de combate do T-Rex.
