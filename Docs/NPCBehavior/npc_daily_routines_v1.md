# NPC Behavior — Daily Routines & Sociology (v1)
Agente #11 — NPC Behavior Agent
Ciclo: PROD_CYCLE_AUTO_20260719_005

## Contexto
Este documento regista o estado real, verificado no mundo vivo (MinPlayableMap), dos NPCs tribais
existentes na área do hub (2100, 2400) e a primeira camada de comportamento social aplicada:
**estados de rotina diária** (Actor Tags), a base sobre a qual Behavior Trees futuras vão ler contexto.

Princípio orientador (Rockstar AI + Tynan Sylvester): nenhum NPC existe para servir o jogador.
Cada tag `Routine_X` representa o que esse NPC está a fazer agora, na sua própria vida — o jogador
é uma interrupção possível, não o centro da simulação.

## Descoberta crítica herdada do Animation Agent #10 (confirmada nesta auditoria)
Existem **49 actores da classe `TranspersonalCharacter`** no MinPlayableMap (não 10 como o #10
reportou — a contagem exacta via `EditorActorSubsystem.get_all_level_actors()` filtrando por classe
devolveu 49). Isto é um sintoma do anti-padrão `hugo_naming_dedup_v2`: múltiplos agentes duplicaram
personagens em vez de reutilizar actores existentes por label.

Tentei identificar o pawn autoritativo via `GameplayStatics.get_player_controller(world, 0)` —
em editor headless (sem PIE a correr), isto tipicamente devolve `None` ou nenhum pawn possuído,
porque o Pixel Streaming liga-se a uma sessão PIE separada que este bridge Python não vê
directamente. **Não foi possível confirmar autoritativamente qual dos 49 é o pawn humano real
a partir do editor parado.** Recomendação: esta confirmação só é fiável a partir de dentro de uma
sessão PIE activa (ex: `unreal.GameplayStatics.get_player_pawn` chamado enquanto o jogo corre) ou
por inspecção directa do `DefaultPawnClass` no GameMode.

**Não toquei em nenhum dos 49 `TranspersonalCharacter`** — só toquei nos NPCs tribais claramente
distintos (labels `Tribal_*`, `NPC_*`, `QuestNPC_*`, `Gatherer_*`, etc.), que são NPCs de história/
ambiente e não candidatos a pawn do jogador.

## NPCs com estado de rotina aplicado (verificado em execução real, tags persistidas nos actores)

| Label | Distância ao hub (2100,2400) | Routine Tag |
|---|---|---|
| Tribal_Hunter | 1403.6 | Routine_Patrolling |
| NPC_TribalHunter | 1236.9 | Routine_Patrolling |
| NPC_Tribal_1 | 412.3 | Routine_Patrolling |
| Character_Gatherer_Female | 412.3 | Routine_Socializing |
| Tribal_Gatherer | 2473.9 | Routine_Socializing |
| TribalScout | 721.1 | Routine_Toolmaking |
| TribalGatherer_03 | 984.9 | Routine_Socializing |
| QuestNPC_Elder_Thok | 1403.6 | Routine_Foraging |

Método: tag determinística por hash do label (`hash(label) % 5`), mapeada a 5 estados de rotina:
`Foraging`, `Toolmaking`, `Resting`, `Patrolling`, `Socializing`. Determinismo garante que o mesmo
NPC mantém sempre o mesmo estado base entre ciclos, evitando "personalidade aleatória" a cada save.

## Sociologia por estado (para consumo futuro por Behavior Trees — #11/#12)
- **Foraging**: NPC prioriza deslocação para vegetação/água próxima; ignora o jogador salvo ameaça directa.
- **Toolmaking**: NPC fica estacionário perto de recursos (pedra/madeira); reage a aproximação com
  desconfiança (olha, não foge de imediato — é um trabalho que não larga por qualquer motivo).
- **Resting**: NPC parado, animação idle; vulnerável, primeira prioridade se atacado é fugir, não lutar.
- **Patrolling**: NPC move-se em rota entre pontos conhecidos; é o estado com maior probabilidade de
  detectar o jogador primeiro (papel de "sentinela" tribal).
- **Socializing**: NPC próximo de outro NPC (par ou grupo); interrompido por qualquer evento de
  perigo, dispersa o grupo.

## Tentativa de grounding via line trace (resultado parcial)
Tentei verificar/corrigir o `z` de cada NPC tagueado contra o Landscape via
`unreal.SystemLibrary.line_trace_single` com `TraceTypeQuery.TRACE_TYPE_QUERY1`. O acesso a
`bBlockingHit`/`Location` via `get_editor_property()` falhou (propriedades marcadas como
"protected" nesta build do Python binding). Usando `hit.to_tuple()` confirmei que o trace correu
(block_hit=True) mas o canal usado não intersectou de forma fiável o Landscape à distância
esperada — os valores devolvidos coincidiam com o ponto de partida em alguns casos, sugerindo
canal de trace incorrecto para Landscape (`TRACE_TYPE_QUERY1` pode não incluir a landscape collision
neste projecto). **Não apliquei nenhum snap de posição** porque não tinha um `ground_z` fiável —
preferível não mover NPCs com dados suspeitos a correr o risco de os enterrar ou flutuar.
Os NPCs listados já estavam a z=80-300, dentro da faixa válida documentada (`hugo_terrain_savana_v1`:
44-302), pelo que a ausência de snap não é um risco imediato conhecido.

**Próximo passo técnico recomendado para #12/#04**: usar `TraceTypeQuery.TRACE_TYPE_QUERY3`
(Visibility) ou `ECollisionChannel.ECC_WorldStatic` explicitamente, e ler os campos do `HitResult`
via `to_tuple()` (índices confirmados: 0=block_hit, 1=blocking_hit, 2=distance, 3=time,
4=location, 5=impact_point, 6=normal, 7=impact_normal, 8=physical_material, 9=actor...) em vez de
`get_editor_property()`, que está bloqueado nesta build.

## Achado a escalar para #01/#02 (herdado do #10, confirmado independentemente)
- 49 actores `TranspersonalCharacter` no mapa — duplicação sistemática, viola `hugo_naming_dedup_v2`.
  Recomenda-se uma passagem de auditoria/consolidação dedicada (não feita aqui — fora do escopo do
  ciclo e do orçamento de tools; requer decisão de qual actor é o pawn real antes de apagar duplicados).
- Confirmar o pawn real do jogador exige inspecção em sessão PIE activa ou do GameMode, não do
  editor parado.

## O que NÃO foi feito (e porquê)
- Não foram escritos ficheiros `.cpp`/`.h` (regra `hugo_no_cpp_h_v2`). O `DinosaurCombatAIController`
  e `SurvivalComponent` mencionados na directiva do ciclo são C++ inerte neste editor headless —
  não têm efeito no jogo vivo mesmo que existam no repositório. Comportamento de T-Rex real
  (patrulha 5000u, chase a 3000u, ataque a 300u) foi implementado em ciclos anteriores directamente
  via Python/AIController em runtime (ver memórias `PROD_CYCLE_AUTO_20260719_003/004`), não via
  ficheiro C++ novo.
- Não apaguei nenhum dos 49 duplicados `TranspersonalCharacter` — decisão irreversível fora do
  meu mandato sem confirmação de qual é o pawn real.

## Ficheiros
- `Docs/NPCBehavior/npc_daily_routines_v1.md` (este ficheiro)

## Handoff para #12 — Combat & Enemy AI Agent
- As tags `Routine_X` nos NPCs tribais do hub estão disponíveis para leitura em Behavior Trees:
  um NPC em `Routine_Resting` deve ter menor probabilidade de reacção de combate imediata do que
  um em `Routine_Patrolling`.
- Usar o canal de trace correcto (`ECC_WorldStatic` ou índice `to_tuple()`) ao implementar detecção
  de terreno para IA de combate de dinossauros — o canal usado aqui falhou silenciosamente.
- Resolver a ambiguidade dos 49 `TranspersonalCharacter` antes de qualquer sistema de combate que
  dependa de identificar "o jogador" por classe/label.
