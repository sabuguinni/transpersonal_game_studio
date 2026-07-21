# T-Rex Behavior Audit — Cycle PROD_CYCLE_AUTO_20260721_007
Agent #11 — NPC Behavior Agent

## Regra respeitada
`hugo_no_cpp_h_v2` (importância MAX): **nenhum .cpp/.h foi criado**. A task original pedia
`Source/TranspersonalGame/AI/TRexBehavior.cpp` — isso foi **recusado deliberadamente** porque
C++ é inerte neste editor headless (nunca recompila). Todo o trabalho de comportamento foi
feito diretamente no mundo vivo via `ue5_execute`, e a lógica de patrulha/perseguição/ataque
foi codificada como **actor tags** legíveis por qualquer sistema de AI (Blueprint ou
Behavior Tree) que o #12 Combat & Enemy AI Agent venha a montar.

## Descoberta crítica (verificada em runtime, não assumida)
Fiz uma auditoria completa a todos os actores com "TRex" na label dentro do núcleo jogável.
Resultado real (`ue5_execute`, `get_all_level_actors()` + `Counter` de classes):

```
CLASS_BREAKDOWN {'NiagaraActor': 104, 'Emitter': 2, 'AmbientSound': 2, 'StaticMeshActor': 4}
TOTAL_TREX_LABELED 112
REAL_CREATURE_TREX_COUNT (Character/Pawn) = 0
```

**Não existe nenhum actor Character/Pawn de T-Rex no mundo.** Tudo o que tem "TRex" na label
é:
- 104x `NiagaraActor` (poeira, distorção de rugido)
- 2x `Emitter`
- 2x `AmbientSound` (proximidade de rugido)
- 4x `StaticMeshActor` chamados `TRexPatrolMarker_Hub_001..004` — são **marcadores estáticos
  de patrulha** (posições no hub, z=100 grounded), não uma criatura animada.

Ou seja: existe *encenação sensorial* de um T-Rex (som, poeira, tremor de câmara) mas **a
criatura em si nunca foi de facto colocada como Pawn/Character controlável por IA**. Isto é
um handoff crítico para o #12 (Combat & Enemy AI Agent) e para o #09/#10 (Character/Animation)
se ainda não tiverem gerado o mesh/skeleton do T-Rex.

## Criaturas reais confirmadas no núcleo jogável
Levantamento de todos os actores `Character`/`Pawn` (632 no total, excluindo `PLAYER0`).
Amostra confirmada com espécies reais e vivas no mundo:
- `Carnotaurus_Desert`
- `Pantano_Spinosaurus_Herd1_0`, `PANTANO_Spinosaurus_Herd1_4`
- `Savana_Apatosaurus_Herd1_1/6/8`
- `Deserto_Compsognathus_Herd2_2/14`
- `Deserto_Dilophosaurus_Herd3_1`
- `Montanha_Allosaurus_Herd2_0`
- `PANTANO_Baryonyx_Herd0_11`
- `Pantano_Dracorex_Herd3_3`
- NPCs humanos nomeados: `TribeMember_Brok`, `Hunter_NPC_Krog`, `Gatherer_NPC_Vera`,
  `NPC_WarriorChief`, `PrimitiveHuman_NPC_01`, `PrimitiveNPC_3`

Nenhum destes tem "TRex"/"Tyrannosaurus" no nome — o T-Rex do hub é puramente atmosférico
(som + VFX), não uma criatura simulada.

## Ações realizadas neste ciclo (verificadas via ue5_execute)
1. Bridge validation OK (`MinPlayableMap` carregado).
2. Leitura no GitHub: `Source/TranspersonalGame/AI/Combat/DinosaurCombatAIController.cpp`
   existe mas está **vazio/placeholder** (9 bytes, conteúdo "undefined") — confirma que ainda
   não há lógica de combate de dinossauro implementada em lado nenhum executável.
3. Leitura no GitHub: `Source/TranspersonalGame/Core/Survival/SurvivalComponent.h` —
   **confirmado, existe e está completo** (Health/Hunger/Thirst/Stamina/Fear/Temperature,
   delegates, drenagem, thresholds de dano). Pronto para qualquer NPC/criatura o usar.
4. Auditoria de classes dos 112 actores "TRex"-labeled → descoberta acima.
5. Verificação de grounding dos 4 `TRexPatrolMarker_Hub_00X`: 3 já estavam corretos
   (dentro de 50u do terreno), 1 (`TRexPatrolMarker_Hub_003`) estava a flutuar
   (z=100 vs terreno real z≈318) — **corrigido por line trace real ao Landscape**
   (`bBlockingHit=True` via parsing de `export_text()`, técnica documentada pelo #10).
6. Tags de comportamento aplicadas aos 4 marcadores de patrulha (`Behavior_Patrol_5000`,
   `Behavior_Chase_3000`, `Behavior_Attack_300`, `Species_TRex`) para que o #12 possa
   ler estes valores diretamente da cena ao construir a Behavior Tree/AIController real,
   sem depender de constantes hardcoded em C++ que nunca compilam.
7. Mapa gravado uma única vez, no fim, após todas as correções.

## Parâmetros de comportamento definidos (para o #12 implementar via Blueprint/BT)
- **Patrulha:** raio de 5000 unidades à volta do ponto de origem do T-Rex.
- **Perseguição:** ativa quando jogador entra em 3000 unidades.
- **Ataque:** ativa quando jogador entra em 300 unidades.
- Estes valores foram escritos como tags (`Behavior_Patrol_5000`, `Behavior_Chase_3000`,
  `Behavior_Attack_300`) nos 4 `TRexPatrolMarker_Hub` — ainda não há criatura para os
  consumir, mas a estrutura de dados está pronta em runtime.

## Bloqueadores / dependências para o próximo agente
- **#12 Combat & Enemy AI Agent**: `DinosaurCombatAIController.cpp` está vazio — precisa de
  ser implementado via Blueprint no editor (não em C++, por regra), usando os `AIController`
  + `BehaviorTree` assets do UE5. Não existe ainda nenhum Pawn de T-Rex — se o jogo precisa de
  um T-Rex ativo e perigoso (não só atmosférico), é preciso que #09 (Character Artist) ou #06
  (Environment Artist) tragam/importem um mesh/skeleton de T-Rex primeiro (ex.: via
  `meshy_generate` + import) antes de qualquer AIController lhe poder ser atribuído.
- `SurvivalComponent.h` está pronto e pode ser reutilizado pelo #12 para dar aos dinossauros
  atributos de "fome"/"stamina" se se quiser IA orientada a necessidades (ex.: T-Rex caça
  quando `Hunger` > threshold), mantendo a filosofia de ecologia realista do jogo.

## Ficheiros
- `Docs/NPC_Behavior/TRex_Behavior_Audit_PROD_007.md` (este ficheiro) — único ficheiro escrito
  neste ciclo. Nenhum .cpp/.h criado.
