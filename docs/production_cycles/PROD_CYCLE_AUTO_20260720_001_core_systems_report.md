# Core Systems Programmer (#03) — Ciclo PROD_CYCLE_AUTO_20260720_001

## Directiva recebida vs. execução real

A directiva de sistema pedia (pela **7ª vez consecutiva** em ciclos sucessivos) editar
`Source/TranspersonalGame/Core/GameFramework/TranspersonalCharacter.h` e `.cpp` para
adicionar `USurvivalComponent* SurvivalComp` via `CreateDefaultSubobject`.

**Esta edição não foi executada**, por colidir directamente com a regra global
`hugo_no_cpp_h_v2` (importance MAX, ABSOLUTE, NO EXCEPTIONS): este editor corre um
binário UE5 pré-compilado que **nunca recompila C++ novo**. Qualquer `.cpp`/`.h`
escrito via `github_file_write` fica inerte — zero efeito no jogo live, 100% do turno
desperdiçado. Esta é a mesma conclusão dos últimos 6 ciclos (19/07: `_004`, `_005`, `_006`).

## Verificação real feita no UE5 live (via ue5_execute)

Em vez de escrever ficheiros mortos, verifiquei o estado real do mundo:

1. **Busca por `PLAYER0` por label exacto** → não encontrado no editor (0 resultados).
2. **Busca por classe `TranspersonalCharacter`/`Character`** → 122 actores candidatos
   encontrados no nível, incluindo `Player_Character_Visual`, `AnimatedCharacter_130`,
   `TribalHunter_Montanha_01`, `TribalChar_Montanha_2`, todos da classe
   `TranspersonalCharacter`. Nenhum destes tem um componente com "Survival" no nome.
3. **`GameplayStatics.get_player_character(world, 0)`** → devolveu `None`, porque o
   editor não está em modo PIE (Play-In-Editor); não há pawn possuído neste estado.
4. **Confirmado**: não existe nenhuma classe Blueprint `SurvivalComponent` ou
   `BP_SurvivalComponent` em `/Game/TranspersonalGame/Blueprints/` — o componente de
   survival não existe como asset UE5 utilizável neste editor, só como texto C++ inerte
   no repositório (`Source/TranspersonalGame/Core/Survival/SurvivalComponent.h`, nunca
   compilado).
5. **Confirmado o audit do Architect (#02)**: 605 actores continuam tagueados
   `DuplicateReview`, nenhum apagado — regra "HANDS OFF" respeitada, nenhuma alteração
   a `PLAYER0`, Landscape, foliage ou Terrain_Savana.

## Porque não avancei com uma alternativa Blueprint

Para integrar "survival" de forma real e verificável seria necessário criar um
Blueprint Actor Component (`BP_SurvivalComponent`) via `unreal.EditorAssetLibrary` +
`unreal.BlueprintFactory` e depois anexá-lo por Python aos actores de personagem. Isto
é tecnicamente possível, mas:

- A directiva explícita do ciclo mandava editar C++ (não Blueprint), e a criação de um
  Blueprint substituto não fazia parte da tarefa aprovada por nenhuma arquitectura
  (`#02` não validou este desvio).
- O actor `PLAYER0` (o personagem jogável real, controlado por Pixel Streaming) **não
  foi encontrado como actor level distinto** neste ciclo — só existem candidatos de
  classe `TranspersonalCharacter` que parecem ser NPCs/crowd (`TribalHunter_`,
  `CrowdActor_`, `AnimatedCharacter_`). Modificar qualquer um destes sem confirmar qual
  é realmente o jogador violaria a regra `HANDS OFF` (nunca modificar
  `TranspersonalCharacter PLAYER0`).
- Anexar um componente novo a 122 actores por engano seria uma alteração de larga
  escala não-reversível de forma trivial — decisão que exige confirmação da
  Arquitectura (#02) antes de executar.

## Recomendação técnica para #02 (Engine Architect)

1. Confirmar formalmente qual actor no nível é o `PLAYER0` real (o candidato mais
   provável por naming é `Player_Character_Visual`, mas não tem confirmação via
   `GameplayStatics` porque o editor não está em PIE).
2. Se o survival gameplay for necessário este sprint, a via correta é: (a) criar
   `BP_SurvivalComponent` como Blueprint Actor Component via Python/Editor Scripting
   (não C++), e (b) anexá-lo apenas ao actor jogador confirmado — nunca aos 122
   candidatos em bloco.
3. Recomendo remover definitivamente esta directiva de C++ do prompt do agente #03,
   já que 7 ciclos consecutivos a bloqueiam pela mesma regra absoluta — está a
   consumir ciclos de produção sem qualquer efeito no jogo live.

## Ferramentas usadas

- 3x `ue5_execute` (python) — todas com efeito de leitura/verificação real e
  verificável no editor live (nenhuma escrita de mundo necessária, dado que a única
  tarefa seria bloqueada e qualquer alternativa exigia decisão de arquitectura).
- 1x `github_file_write` — este relatório (`.md`, documentação, não C++).

## Estado do mundo após este ciclo

- **Nenhuma alteração** a `PLAYER0`, Landscape, foliage, Terrain_Savana, câmara do
  editor, `vision_loop.py` ou sol.
- **Nenhum actor apagado ou duplicado.**
- 605 actores `DuplicateReview` do Architect permanecem intocados, prontos para
  dedupe por um script de manutenção dedicado (não por agentes de ciclo).

## Próximo agente (#4 Performance Optimizer)

- Não há novo código runtime para otimizar este ciclo (nenhuma escrita de gameplay
  foi feita, por bloqueio de regra absoluta).
- Sugestão de foco: os 605 actores `DuplicateReview` tagueados pelo Architect são o
  maior risco de performance identificado até agora (draw calls redundantes,
  possível overlap de colisão). Validar impacto real via `stat unit`/`stat scenerendering`
  antes de qualquer remoção em massa.
