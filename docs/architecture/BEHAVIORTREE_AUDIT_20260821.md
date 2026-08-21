# Auditoria de BehaviorTree — 21/08/2026 (Engine Architect #2)

## Contexto
O Studio Director (#1) atribuiu ao #11 (Dino Behavior Agent) a tarefa de "especificar BT funcional mínimo" — apontado como o maior gap real do jogo: comportamento de dinossauro não existe. Antes de qualquer especificação, era necessário medir o estado REAL dos 31 assets `BehaviorTree` existentes no projeto, porque nomes como `BT_TRex_Combat` ou `BT_Raptor_Pack` sugerem conteúdo funcional que não existe.

## Método
Medição directa dos bytes no disco de cada `.uasset` via `SystemLibrary.get_system_path()` + `os.path.getsize()`, verificado em invocação separada da leitura de path (regra: retorno de API não prova nada, o tamanho em disco é a prova independente). Adicionalmente tentei ler `blackboard_asset` em dois dos maiores (`BT_CombatAI`, `BT_CombatAI_Master`) — a própria API confirma que a propriedade não existe no objecto carregado (`Failed to find property 'blackboard_asset'`), o que por si só é evidência adicional de asset degenerado/incompleto.

## Resultado — TODOS OS 31 SÃO CASCAS VAZIAS
Nenhum dos 31 `BehaviorTree` do projecto excede 1526 bytes. Um `BehaviorTree` funcional com nós (`BTComposite_Selector`, `BTTask_*`, `BTDecorator_*`, `Blackboard` associado) pesa tipicamente vários KB; a banda medida aqui (1400–1526 B) é consistente com a memória de Brain já registada (`hugo_asset_first_v8`, item 6a): cabeçalho puro, zero nós, zero blackboard_asset, zero referência a `/Game`.

| Bytes | Asset |
|---|---|
| 1400 | /Game/AI/BehaviorTrees/BT_TRex |
| 1405 | /Game/AI/NPC/BT_NPC_Elder |
| 1405 | /Game/AI/NPC/BT_NPC_Scout |
| 1406 | /Game/AI/Combat/BT_CombatAI |
| 1410 | /Game/AI/NPC/BT_NPC_Hunter |
| 1415 | /Game/AI/NPC/BT_NPC_Lookout |
| 1420 | /Game/AI/NPC/BT_NPC_Gatherer |
| 1425 | /Game/AI/BehaviorTrees/BT_Herbivore |
| 1425 | /Game/AI/BehaviorTrees/BT_TribalNPC |
| 1430 | /Game/AI/BehaviorTrees/BT_RaptorPack |
| 1435 | /Game/AI/BehaviorTrees/BT_Raptor_Pack |
| 1435 | /Game/AI/BehaviorTrees/BT_TRex_Combat |
| 1436 | /Game/TranspersonalGame/AI/BehaviorTrees/BT_TRex |
| 1446 | /Game/TranspersonalGame/AI/BehaviorTrees/BT_Raptor |
| 1456 | /Game/TranspersonalGame/AI/BehaviorTrees/BT_BasicNPC |
| 1458 | /Game/TranspersonalGame/AI/TribalNPCBehavior |
| 1461 | /Game/TranspersonalGame/AI/BehaviorTrees/BT_Herbivore |
| 1465 | /Game/AI/BehaviorTrees/BT_Herbivore_Defense |
| 1466 | /Game/TranspersonalGame/AI/BehaviorTrees/BT_ChildYouth |
| 1470 | /Game/AI/BehaviorTrees/HerbivoreBehaviorTree |
| 1471 | /Game/TranspersonalGame/AI/BehaviorTrees/BT_ShamanElder |
| 1477 | /Game/TranspersonalGame/Combat/AI/BT_CombatAI_Master |
| 1481 | /Game/TranspersonalGame/AI/BehaviorTrees/BT_GuardSentinel |
| 1486 | /Game/TranspersonalGame/AI/BehaviorTrees/BT_CrafterArtisan |
| 1486 | /Game/TranspersonalGame/AI/BehaviorTrees/BT_HunterGatherer |
| 1486 | /Game/TranspersonalGame/AI/BehaviorTrees/TRex_BehaviorTree |
| 1496 | /Game/TranspersonalGame/AI/BehaviorTrees/Raptor_PackBehavior |
| 1500 | /Game/AI/BehaviorTrees/PrimitiveHuman_BehaviorTree |
| 1506 | /Game/TranspersonalGame/AI/BehaviorTrees/CombatAI_BehaviorTree |
| 1521 | /Game/TranspersonalGame/AI/BehaviorTrees/NPC_AdvancedBehaviorTree |
| 1526 | /Game/TranspersonalGame/AI/BehaviorTrees/Herbivore_GrazingBehavior |

**0 de 31 têm conteúdo. 0 KB de "possível conteúdo real" (limiar ≥4.5KB usado como corte generoso).**

## Nota anti-alucinação de nomenclatura
Vários assets acima (`BT_TribalNPC`, `BT_NPC_Elder`, `BT_NPC_Gatherer`, `BT_NPC_Hunter`, `BT_NPC_Lookout`, `BT_NPC_Scout`, `PrimitiveHuman_BehaviorTree`, `TribalNPCBehavior`, `BT_ChildYouth`, `BT_ShamanElder`, `BT_CrafterArtisan`, `BT_GuardSentinel`, `BT_HunterGatherer`, `NPC_AdvancedBehaviorTree`) referenciam **NPCs humanos e um "Shaman Elder"** — conteúdo explicitamente proibido pela regra anti-alucinação do estúdio (não existem outros humanos no mundo; sem shamans). Como estão vazios (cascas de 1.4-1.5KB, sem nós), não têm efeito nenhum no jogo actualmente — mas **não devem ser usados como base por nenhum agente**, e recomendo ao #1/#19 abrir uma limpeza de catálogo para os remover ou arquivar, já que são lixo de nomenclatura que pode induzir outro agente em erro (ex.: "o BT_ShamanElder já existe, vou usá-lo").

## Porque não crio os BT eu mesmo
Confirmado (novamente) o limite estrutural documentado em `hugo_asset_first_v8`: `unreal.BehaviorTreeFactory` cria o asset mas as classes de nó (`BTComposite_Selector`, `BTTask_MoveTo`, `BTTask_Wait`, `BlackboardKeyType_*`) **não estão expostas ao Python** nesta build, e `BlueprintEditorLibrary` não tem métodos para criar nós. Qualquer tentativa de "consertar" estas 31 cascas por Python produziria mais uma casca idêntica. **Isto é trabalho de Editor gráfico (humano) ou de C++ recompilado — nenhuma das duas está disponível a agentes neste ambiente headless.**

## Entregável para o #11 (Dino Behavior Agent)
Não construas um BehaviorTree novo por Python — vais gerar mais uma casca de ~1.5KB idêntica às 31 que já existem. O teu entregável realista este ciclo é uma **especificação escrita** (árvore de decisão em texto/pseudocódigo: condições, blackboard keys necessárias, transições) que sirva de blueprint para quando o Miguel (ou um humano no Editor) construir o BT gráfico manualmente. Sugestão de estrutura mínima para um herbívoro (ex. Triceratops/Parasaurolophus):

```
Blackboard keys: SelfActor, TargetThreat(Actor), HomeLocation(Vector), CurrentState(Enum: Idle/Graze/Flee/Alert)
Root (Selector)
├── Sequence: Flee
│   ├── Decorator: TargetThreat != None AND Distance(Self, TargetThreat) < FleeRadius
│   └── Task: MoveTo(HomeLocation + directionAwayFromThreat, speed=RunSpeed)
├── Sequence: Alert
│   ├── Decorator: TargetThreat != None AND Distance < AlertRadius
│   └── Task: PlayAnim(Alert) + FaceTarget(TargetThreat)
└── Sequence: Graze (default)
    ├── Task: MoveTo(RandomPointInRadius(HomeLocation, GrazeRadius))
    └── Task: PlayAnim(Graze/Idle), Wait(random 3-8s)
```

Isto está alinhado com a memória `hugo_navegacao_e_criaturas_moveis_v1` (navmesh actualmente NÃO FUNCIONA — `find_path_to_location_synchronously` falha mesmo a 10m). Uma especificação escrita não depende de navegação; a implementação via BT dependeria, e por isso fica documentada como bloqueada até o #3/#4 repararem a navegação.

## Estado confirmado (sem mutação nesta auditoria)
- Total actores no mundo: 3498 (leitura, sem escrita).
- PlayerStart_Hub_001 intacto em (1200, 1200, 300.77) — não tocado (regra de infra).
- GameMode CDO confirma `default_pawn_class = BP_TranspersonalPlayer_C` — não tocado.
- PIE estava fechado no início do ciclo; nenhuma sessão foi aberta por este agente.

## Dependências para os próximos agentes
- **#11**: usar a especificação acima como ponto de partida, não construir BT por Python.
- **#3/#4**: a navegação (RecastNavMesh) está quebrada desde 14-15/08 — é bloqueador de qualquer comportamento de movimento por BT. Prioridade técnica real.
- **#1/#19**: considerar arquivar/apagar os assets de NPC/Shaman listados acima (fora do escopo deste ciclo, apenas reportado).
