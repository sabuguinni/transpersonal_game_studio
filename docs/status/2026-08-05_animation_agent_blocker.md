# Animation Agent #10 — Blocker Report
CYCLE_ID: PROD_CYCLE_MANUAL6_20260805

## Contexto
Recebi o handoff do Character Artist Agent #09, que reportou bloqueio de infraestrutura pela 6ª vez consecutiva na cadeia (#04→#05→#06→#07→#08→#09). A minha directiva específica deste ciclo era: configurar estados idle/walk/run para o player, garantir AnimInstance funcional, e trigger de salto. Antes de tocar em qualquer Animation Blueprint ou skeletal mesh component, repeti — de forma independente, com invocações `ue5_execute` separadas — o mesmo protocolo de verificação de estado do editor usado pelos agentes anteriores.

## Verificação real efectuada (2 chamadas `ue5_execute`, ambas executadas contra o UE5 vivo)

**Chamada 1 — estado do editor world:**
- `EditorActorSubsystem.get_all_level_actors()` = **0**
- `EditorLevelLibrary.get_all_level_actors()` = **0** (concordante com o leitor anterior)
- `UnrealEditorSubsystem.get_editor_world()` = **None**
- `EditorAssetLibrary.does_asset_exist('/Game/Maps/MinPlayableMap')` = **False**
- `SystemLibrary.get_frame_count()` amostra 1 = **2323361**

**Chamada 2 — prova de vida do processo + teste decisivo de mutação (invocação separada, não no mesmo tick):**
- `SystemLibrary.get_frame_count()` amostra 2 = **2324515** (delta de +1154 frames em relação à amostra 1 — o binário do UE5 está vivo e a correr, não em crash nem congelado)
- `EditorLevelLibrary.spawn_actor_from_class(PointLight, ...)` = **None** (falha silenciosa de mutação — confirma ausência de world mutável, apesar do processo estar vivo)
- Terceiro leitor independente via `AssetRegistryHelpers.get_asset_registry().get_asset_by_object_path(...)`: o **AssetData do MinPlayableMap EXISTE no AssetRegistry** (package_name, package_path e asset_class_path=World todos presentes), o que **contradiz directamente** o resultado `does_asset_exist=False` do `EditorAssetLibrary` na mesma chamada.

## Achado novo relevante para o próximo diagnóstico
Este ciclo acrescenta um dado que os ciclos #07/#08/#09 não tinham: **dois leitores de existência do mapa discordam entre si**. `EditorAssetLibrary.does_asset_exist` diz False, mas `AssetRegistry.get_asset_by_object_path` devolve um `AssetData` válido e preenchido para o mesmo package path. Isto sugere que o package está **registado no AssetRegistry mas não está carregado/montado como o world activo do editor** — coerente com `get_editor_world()=None` e `get_all_level_actors()=0`, mas incoerente com um "mapa simplesmente inexistente". O processo está vivo (frame counter avança), mas o editor não tem nenhum nível aberto/mutável neste momento.

## Conclusão
Bloqueio de infraestrutura confirmado pela **7ª vez consecutiva na cadeia** (#04→#05→#06→#07→#08→#09→#10). O meu entregável específico (idle/walk/run states, AnimBlueprint/AnimInstance switching, jump trigger) exige:
1. Um world mutável para ler/escrever propriedades do ABP_Quinn_C e do CDO do BP_TranspersonalPlayer;
2. Actores acessíveis para verificar em vivo (`anim_class`, `state_machine nodes`, `blend_spaces`) qualquer alteração feita.

Com `get_all_level_actors()=0`, `editor_world=None` e `spawn_actor_from_class` a devolver `None`, qualquer configuração de AnimBlueprint que eu reportasse como "feita" seria fabricada — violando directamente a regra "VERIFIED IN WORLD" (Definition of Done #5) e a regra anti-alucinação. **Não simulei** estados de animação, nós de state machine, blend spaces nem triggers de salto. Não toquei em nenhum actor (jogador, câmara, Landscape, sublevel Terrain_Savana, `Dino_*`, NPCs, ABP_Quinn_C) porque não havia nenhum world mutável para tocar.

## Plano pronto para execução imediata assim que o editor recuperar
Com base no que já está verificado e documentado nas memórias do Brain (ABP_Quinn_C já tem AnimGraph com StateMachine + BlendSpacePlayer + SequencePlayer, referências a `BS_MF_Unarmed_WalkRun` e `MF_Idle`, 39884 bytes — não é uma casca vazia como os ABPs de dinossauro):

1. **Reverificar** `anim_class` do CDO de `BP_TranspersonalPlayer` == `ABP_Quinn_C` (já confirmado em ciclos anteriores — reconfirmar apenas).
2. **Ler** os nós da state machine de `ABP_Quinn_C` via `AnimBlueprintEditorLibrary` / inspecção de `AnimGraphNode_StateMachine` para confirmar transições existentes entre Idle/Walk/Run (BlendSpace `BS_MF_Unarmed_WalkRun` já referenciado sugere que isto já pode estar parcialmente funcional a partir do trabalho de outro agente/ciclo).
3. **Não recriar** o ABP do zero via Python — confirmado nas memórias que `AnimBlueprintFactory` cria cascas vazias sem acesso a nós via Python (BTTask_/AnimGraphNode_ não expostos). Qualquer AnimBlueprint novo criado por script Python seria uma casca sem conteúdo.
4. **Jump trigger**: verificar se `CharacterMovementComponent` do CDO tem `jump_z_velocity` configurado e se existe uma transição de state machine ligada à variável booleana `bIsFalling`/`bIsJumping` já exposta pelo ABP existente — configurar via Blueprint/RC apenas se necessário, nunca via C++.
5. Medir antes/depois por releitura independente do `anim_class` e da presença de nós de blend, nunca confiar no valor de retorno de uma única chamada.

## Ficheiro criado
- `docs/status/2026-08-05_animation_agent_blocker.md` — este relatório.

## Decisões técnicas
- Prioritizei a Definition of Done regra #5 (VERIFIED IN WORLD) sobre inventar configuração de AnimBlueprint para simular cumprimento da directiva.
- Não usei `EditorLoadingAndSavingUtils.load_map()` (proibido pelo Brain — crasha via `FPyReferenceCollector`).
- Não escrevi `.cpp`/`.h` (regra absoluta).
- Não confiei em memórias de ciclos anteriores (que reportavam ABP_Quinn_C com conteúdo real) sem tentar reverificação ao vivo neste ciclo — a reverificação foi tentada e bloqueada pela ausência de world mutável, não pela falta de tentativa.
- Reportei a discordância entre `EditorAssetLibrary.does_asset_exist` e `AssetRegistry.get_asset_by_object_path` como achado novo, por ser potencialmente o fio a puxar para o diagnóstico raiz que os ciclos #07-#09 não tinham isolado.

## Dependências / próximo agente (#11 NPC Behavior Agent)
- Recomendo ao #01/operador: investigar por que o AssetRegistry regista o `MinPlayableMap` mas o `EditorAssetLibrary` não o reconhece e `get_editor_world()` devolve `None` — sugere que o nível está registado mas não está **aberto/carregado** como o mundo activo do editor. Isto é uma pista mais específica do que os ciclos anteriores tinham (que só reportavam "ausência" pura).
- Pausar ciclos de conteúdo/animação até `get_editor_world() != None` **e** `get_all_level_actors() > 0` **e** `spawn_actor_from_class` produzir um actor real (não `None`), confirmados em simultâneo por leitor independente.
- #11 deve repetir esta verificação de 2 chamadas antes de assumir que há Characters/Pawns acessíveis para configurar Behavior Trees.
