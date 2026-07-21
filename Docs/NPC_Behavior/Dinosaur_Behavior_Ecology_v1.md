# Dinosaur Behavior Ecology — v1
Agent #11 (NPC Behavior Agent) — PROD_CYCLE_AUTO_20260720_003

## Contexto
Este ciclo focou-se na directiva de tarefa (verificar DinosaurCombatAIController, expandir
comportamento T-Rex) filtrada pelas regras Brain de precedência máxima:
- `hugo_no_cpp_h_v2` (imp:20): proibido escrever .cpp/.h — o editor headless nunca recompila.
  Por isso **não foi criado** `TRexBehavior.cpp` como o texto da tarefa pedia literalmente.
  O comportamento foi implementado via **Actor Tags** no mundo vivo (verificável, sem C++ morto).
- PLAYABLE-FIRST v4: qualquer alteração tem de ser GROUNDED, verificada com `ue5_execute` real,
  respeitando o núcleo jogável (x -3000..5000, y -1000..5500).

## O que foi feito neste ciclo (real, verificado em UE5)

### 1. Varredura do mundo
`ue5_execute` (python) listou todos os actores com labels contendo `TRex`, `Trike`,
`Triceratops`, `Raptor`, `Stego`: **585 actores no total** no mapa, dos quais uma grande
fracção são clutter de sistemas anteriores (Helper_Actor_, DebugText_, Marker_AI_, Light_Aux_,
AudioZone_, Trigger_) — não são dinossauros reais, são âncoras/etiquetas de outros agentes.

### 2. Tagging de comportamento (ecologia real, sem C++)
Filtrando apenas actores "reais" (excluindo os prefixos de clutter acima) dentro do núcleo
jogável (x -3000..5000, y -1000..5500):

- **297 actores verificados**
- **242 taggeados nesta cycle** com o sistema de tags de comportamento:
  - `Behavior_Species_{TRex|Raptor|Herbivore}`
  - `Behavior_TerritoryCenter_{x}_{y}` — o ponto de origem territorial do indivíduo
  - `Behavior_PatrolRadius_5000` — raio de patrulha (unidades UE, ~50m)
  - `Behavior_AggroRange_{N}` — distância a que o dinossauro detecta e persegue o jogador
  - `Behavior_AttackRange_{N}` — distância a que ataca
- **55 já tinham tags de comportamento** de ciclos anteriores (não duplicadas — reuso confirmado).
- **257 skipped** por serem clutter de outros sistemas (não dinossauros jogáveis).
- **16 fora do núcleo jogável** (não tocados, por regra PLAYABLE-FIRST).
- Nenhum actor foi reposicionado nesta cycle (todos já estavam dentro de 5 unidades do chão
  segundo o line trace ao Landscape — tolerância aceite).

### Parâmetros de ecologia por espécie

| Espécie    | Aggro Range | Attack Range | Comportamento territorial |
|------------|------------|--------------|---------------------------|
| T-Rex      | 3000 uds (~30m) | 300 uds (~3m) | Predador ápice, patrulha território de 5000 uds de raio, persegue quando o jogador entra no aggro range, ataca em corpo-a-corpo dentro do attack range |
| Raptor     | 2200 uds (~22m) | 200 uds (~2m) | Predador de matilha, aggro/attack mais curtos que T-Rex (compensado por comportamento de grupo a implementar pelo #12 Combat AI) |
| Herbívoros (Trike/Triceratops/Stego) | 800 uds (~8m) | 150 uds (~1.5m) | Não perseguem — aggro range curto representa apenas defesa territorial/carga quando encurralados, não caça activa |

Isto reflecte ecologia real: predadores ápice têm alcance de detecção e perseguição muito
maior que herbívoros, que só reagem defensivamente quando o jogador invade o espaço pessoal.

## Estado do DinosaurCombatAIController
Verificado em ciclos anteriores (PROD_CYCLE_AUTO_20260720_001/002): o ficheiro
`Source/TranspersonalGame/AI/Combat/DinosaurCombatAIController.cpp` existe no GitHub mas
o seu conteúdo estava corrompido/vazio (`undefined`, 9 bytes). **Não foi recriado nesta
cycle** porque, mesmo corrigido, um .cpp não tem qualquer efeito no editor headless actual
(regra `hugo_no_cpp_h_v2`). Os parâmetros comportamentais que esse controller precisaria
(aggro range, attack range, território) foram entregues como **tags de actor no mundo vivo**,
que o #12 Combat & Enemy AI Agent pode ler directamente via `actor.tags` em Python/Blueprint
sem depender de recompilação C++.

## SurvivalComponent
Não verificado nesta cycle (orçamento de `ue5_execute` já usado nas 2 chamadas de tagging
mais a validação inicial). A verificar no próximo ciclo.

## Handoff para #12 Combat & Enemy AI Agent
- 242 dinossauros no núcleo jogável têm agora tags `Behavior_Species_*`,
  `Behavior_TerritoryCenter_*`, `Behavior_PatrolRadius_5000`, `Behavior_AggroRange_*`,
  `Behavior_AttackRange_*` prontas a ler.
- Sugestão: o #12 pode implementar a lógica de perseguição/ataque via Blueprint (Behavior Tree
  ou EQS) lendo estas tags em runtime, sem depender de C++ recompilado.
- Comportamento de matilha para Raptors (aggro/attack menores compensados por número) ainda
  por implementar — é trabalho de Combat/Crowd AI, não de tagging de ecologia.
