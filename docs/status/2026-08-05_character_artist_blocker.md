# Character Artist Agent #09 — Blocker Report
CYCLE_ID: PROD_CYCLE_MANUAL6_20260805

## Contexto

Este ciclo tinha um entregável de criação claro e mensurável, herdado do CREATION DELIVERABLE
(05/08/2026, Hugo): converter as 9 esferas NPC (StaticMeshActor + Sphere + M_PrimitiveSkin_Tan)
em SkeletalMeshActor com `MESH_F_WHITE_REGULAR_ASSEMBLED` + `ABP_Quinn_C`, medindo R37/R20
antes e depois.

A minha própria memória de ciclo anterior (PROD_CYCLE_MANUAL5_20260805) indica que **8 dos 9
NPCs já foram convertidos** nesse ciclo ("Converter os 8 NPCs esfera em SkeletalMeshA[ctor]...").
Este ciclo devia continuar esse trabalho (o 9º NPC em falta, verificação R37/R20, e reporte
before/after), mas o editor está inacessível — ver abaixo.

## Verificação real efectuada (2 chamadas `ue5_execute`, ambas executadas contra o UE5 vivo)

Repeti de forma **independente** a bateria de testes de estado do editor, seguindo o mesmo
protocolo dos 4 agentes anteriores na cadeia (#05 → #06 → #07 → #08), sem assumir os relatos
deles nem os meus próprios de ciclos anteriores.

**Chamada 1:**
- `EditorActorSubsystem.get_all_level_actors()` → **0**
- `UnrealEditorSubsystem.get_editor_world()` → **None**
- `EditorAssetLibrary.does_asset_exist('/Game/Maps/MinPlayableMap')` → **False**
- `SystemLibrary.get_frame_count()` (amostra 1) → **2314597**
- `EditorLevelLibrary.get_all_level_actors()` (leitor antigo, independente) → **0**

**Chamada 2 (invocação separada, ~1s depois):**
- `SystemLibrary.get_frame_count()` (amostra 2) → **2315799** — processo está a tickar (+1202 frames), não há crash.
- `EditorLevelLibrary.spawn_actor_from_class(PointLight, (0,0,0))` → **None** — falha silenciosa de spawn, confirma world não mutável.
- `EditorAssetLibrary.does_asset_exist('/Game/Maps/MinPlayableMap')` (recheck) → **False** — consistente com a chamada 1.
- `EditorActorSubsystem.get_all_level_actors()` (recheck) → **0** — consistente.

## Conclusão

Bloqueio de infraestrutura confirmado pela **6ª vez consecutiva na cadeia** (#04→#05→#06→#07→#08→#09),
com dois leitores independentes (EditorActorSubsystem / EditorLevelLibrary) concordantes em
ambas as chamadas, um teste de spawn a falhar silenciosamente (retorna `None` em vez de lançar
excepção — sintoma de world nulo, não de erro transitório), e `does_asset_exist` a continuar
`False` no MinPlayableMap.

**Não simulei** nem inventei medições R37/R20 (contagem de esferas NPC restantes, verificação de
skeletal mesh, secções de material visíveis). Com `get_all_level_actors()=0` e `editor_world=None`,
qualquer valor reportado sobre os NPCs seria fabricado — isso viola directamente a regra
anti-alucinação e a regra "VERIFIED IN WORLD" (Definition of Done #5).

Não toquei em: NPCs esfera, jogador, câmara do editor, Landscape, sublevel Terrain_Savana,
qualquer `Dino_*`, sol, ou qualquer outro actor — porque não há actores acessíveis para tocar.

## Trabalho preparado para retomar imediatamente

Assim que `get_editor_world() != None` e `get_all_level_actors() > 0` forem confirmados em
simultâneo, o próximo passo é:

1. Reler o estado dos 9 NPCs esfera pelo label (buscar `NPC_*` com `Sphere` + `M_PrimitiveSkin_Tan`)
   para confirmar quantos ainda restam por converter (segundo a minha memória do ciclo MANUAL5,
   deveria restar 1).
2. Para cada esfera restante: aplicar a receita provada —
   - `spawn_actor_from_class(unreal.SkeletalMeshActor, loc, rot)` na posição da esfera (line trace
     ao Landscape para confirmar z, ignorando actores não-Landscape).
   - `skeletal_mesh_component.set_editor_property('skeletal_mesh_asset', MESH_F_WHITE_REGULAR_ASSEMBLED)`
     e também `set_editor_property('skinned_asset', ...)` (as duas vistas, escrever em ambas).
   - Promover o componente para MOVABLE antes de mutar.
   - `set_editor_property('anim_instance_class', ABP_Quinn_C)`, `animation_mode=ANIMATION_BLUEPRINT`.
   - Esconder uma combinação diferente de secções de material (`show_material_section`) por NPC,
     para não ficarem clones.
   - Assentar pelo **osso mais baixo** (get_num_bones + get_bone_name + get_socket_location),
     nunca pela base do AABB — a folga medida nesta malha é ~27 uu.
   - Esconder a esfera original (`set_actor_hidden_in_game(True)`), nunca apagá-la (exceção
     nomeada autoriza troca de malha via spawn, não exige apagar o StaticMeshActor original).
3. Medir R37 (contagem de esferas NPC visíveis com M_PrimitiveSkin_Tan) e R20 antes/depois e
   reportar os dois números, como exigido pelo CREATION DELIVERABLE.
4. `actor.modify(True)` antes de qualquer save; gravar com `save_dirty_packages(True, True)` no
   fim, uma única vez.

## Ficheiros criados neste ciclo
- `docs/status/2026-08-05_character_artist_blocker.md` (este ficheiro)

## Decisões técnicas
- Priorizei a Definition of Done regra #5 (VERIFIED IN WORLD) sobre inventar números de
  R37/R20 para simular cumprimento do entregável.
- Não usei `EditorLoadingAndSavingUtils.load_map()` para tentar recarregar o mapa — proibido
  pelo Brain (crasha via `FPyReferenceCollector`, referências retidas impedem GC do package antigo).
- Não escrevi `.cpp`/`.h` — regra absoluta, C++ inerte neste editor headless.
- Não movi, rodei, reescalei nem apaguei qualquer actor — não havia actores acessíveis para o fazer.

## Dependências necessárias
- Recuperação real do processo UE5 fora do bridge (restart do editor / recarregamento do nível
  persistente) — 6º ciclo consecutivo consecutivo a reportar este mesmo bloqueio, agravando-se
  desde o ciclo #07 (`does_asset_exist` passou de `True` para `False` nesse ciclo e mantém-se
  `False` desde então).
- Recomendo ao #01/operador: verificar se o nível persistente MinPlayableMap foi descarregado ou
  se o package foi movido/renomeado entre ciclos — o sintoma é consistente com perda de referência
  ao package do mapa no AssetRegistry, não apenas com "nenhum world carregado em PIE".
- Pausar ciclos de conteúdo (incluindo este) até `get_editor_world() != None` **e**
  `get_all_level_actors() > 0` **e** `does_asset_exist(MinPlayableMap) == True` serem confirmados
  em simultâneo, por leitura independente.

## Próximo agente (#10 Animation Agent)
Igualmente bloqueado a montante — o entregável de animação depende dos 9 NPCs terem
`SkeletalMeshComponent` válido, o que não pode ser confirmado nem avançado enquanto o world
estiver inacessível. Deve repetir esta verificação de 2 passos (actor count + frame count em
duas amostras separadas + teste de spawn) antes de assumir que o bloqueio foi resolvido, e
NÃO assumir que a conversão dos 9 NPCs (relatada no ciclo MANUAL5) está completa sem reler o
estado ao vivo — a minha própria memória de ciclo anterior não é substituto de verificação.
