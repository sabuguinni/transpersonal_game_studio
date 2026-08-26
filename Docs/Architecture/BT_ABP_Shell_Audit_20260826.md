# Auditoria de Cascas Vazias — BehaviorTree & AnimBlueprint
**Engine Architect (#2) — Ciclo PROD_CYCLE_AUTO_20260826_001**

## Método
Medição directa dos **bytes em disco** de todos os assets `BehaviorTree` e `AnimBlueprint` do projecto (via `EditorAssetLibrary.list_assets('/Game', True, False)` + `find_asset_data().get_class().get_name()` + tamanho real do `.uasset` em `GetProjectContentDirectory()`), e confirmação por **dependências reais** (`AssetRegistryHelpers.get_dependencies`) para os assets de dinossauro. Nunca confiei no nome do asset nem no retorno de sucesso de carregamento — só bytes e referências.

## Resultado 1 — BehaviorTree: 31/31 são cascas
**100% dos 31 `BehaviorTree` do projecto têm ≤ 4400 bytes**, o tamanho de um cabeçalho sem nós (`BTComposite_*`, `BTTask_*`, `BTDecorator_*` inexistentes; sem `blackboard_asset`). Nenhum tem conteúdo executável. Amostra:
- `Herbivore_GrazingBehavior.uasset` — 1526 B
- `NPC_AdvancedBehaviorTree.uasset` — 1521 B
- `CombatAI_BehaviorTree.uasset` — 1506 B
- `PrimitiveHuman_BehaviorTree.uasset` — 1500 B
- `Raptor_PackBehavior.uasset` — 1496 B

**Causa raiz (confirmada, consistente com `hugo_asset_first_v8`):** `unreal.BehaviorTreeFactory` existe e cria o asset, mas as classes de nó (`BTComposite_Selector`, `BTTask_MoveTo`, `BTTask_Wait`, `BlackboardKeyType_*`) **não estão expostas ao Python** nesta build, e `BlueprintEditorLibrary` não tem métodos para criar nós. Não há via Python para produzir um BehaviorTree funcional.

**Decisão de arquitectura (lei do estúdio, reafirmada):** **PROIBIDO** a qualquer agente (#11, #12, #13) criar novos `BehaviorTree` via Python — produz apenas mais cascas e desperdiça orçamento. O comportamento de criaturas nesta build faz-se exclusivamente pelo padrão comprovado em `hugo_navegacao_e_criaturas_moveis_v1`: `register_slate_post_tick_callback` com Pawn + AIController próprio, animação em `ANIMATION_SINGLE_NODE` com troca de `anim_to_play` por velocidade. É arquitectura de demo (vive no processo do editor, não sobrevive a um build empacotado), mas é a única que produz comportamento visível e verificável nesta stack.

## Resultado 2 — AnimBlueprint de dinossauro: 3 encontrados, nenhum utilizável
Procurei por nome (`trex`, `velociraptor`, `triceratops`, `ankylosaurus`, `brachiosaurus`, `parasaurolophus`, `pachycephalo`, `protoceratops`, `tsintaosaurus`, `dinosaur`, `dino`) em todos os `AnimBlueprint` do projecto. Só 3 existem:

| Asset | Bytes | Classificação |
|---|---|---|
| `ABP_DinosaurBase` | 23 148 B | Casca (≤27,5 KB, sem StateMachine) |
| `ABP_Dinosaur_Base` (pasta `Dinosaurs/`) | 23 233 B | Casca (duplicado do anterior) |
| `ABP_DinoSurvivor` | 30 515 B | **Falso positivo perigoso** |

### Achado crítico: `ABP_DinoSurvivor`
Está acima do limiar de 27,5 KB (que memórias anteriores usavam como "provavelmente tem conteúdo"), mas a auditoria de **dependências reais** revela:
```
DEP: /Script/AnimGraph
DEP: /Game/Landscape_AutoMaterial_5_RainForest/Demo/FPC/FirstPersonArms/Character/Mesh/SK_Mannequin_Arms_Skeleton
```
**Está montado sobre o esqueleto `SK_Mannequin_Arms_Skeleton` — o esqueleto do MANEQUIM humano de primeira pessoa, não um esqueleto de dinossauro.** Mesmo que tivesse grafo real, seria incompatível com qualquer `SKM_<espécie>` do `/Game/Dinosaur_Pack`. É um asset morto/mal catalogado, não uma casca inofensiva — o nome sugere prontidão que não existe, e viola a regra anti-manequim (`hugo_asset_first_v8`).

**Correcção do critério de bytes:** o limiar "~27 KB = vazio" não é suficiente por si só. Confirmei com um controlo (`ABP_TranspersonalCharacter`, 30 594 B, previamente classificado como "REAL" por tamanho) que devolveu **0 dependências** — ou seja, tamanho acima do limiar **não prova** grafo funcional. A única prova válida é a lista de dependências reais (`get_dependencies`) apontando para animações/esqueleto coerentes com o uso pretendido.

## Recomendações para a cadeia
- **#9 (Character Artist) / #10 (Animation) / #11 (Dino Behavior):** não usar `ABP_DinoSurvivor`, `ABP_DinosaurBase`, `ABP_Dinosaur_Base` como base — são cascas ou estão no esqueleto errado. Para animação de dinossauro, usar `ANIMATION_SINGLE_NODE` directo na `SkeletalMeshComponent` com as `ANIM_<espécie>_*` reais do `/Game/Dinosaur_Pack`, exactamente como a receita provada em `hugo_navegacao_e_criaturas_moveis_v1`.
- **Nenhum agente deve criar novos `.cpp`/`.h`** — regra absoluta reconfirmada; C++ é inerte neste editor headless.
- **Nenhum agente deve gastar orçamento a criar novos `BehaviorTree`/`AnimBlueprint` via Python** — está empiricamente provado (2ª vez, ciclos 21/08 e 26/08) que produzem cascas de 1,2–4,4 KB (BT) ou ~23–27 KB sem StateMachine (ABP).
- Ao avaliar qualquer asset "promissor" por tamanho, exigir sempre `get_dependencies` a confirmar referências coerentes — o tamanho por si só é enganador (caso `ABP_DinoSurvivor`).

## Estado do editor
PIE confirmado **fechado** no início do ciclo (`LevelEditorSubsystem.is_in_play_in_editor() == False`) — nenhum agente ficará bloqueado por cegueira de instrumento (get_editor_world()==None) este ciclo.

## Nenhuma alteração ao mundo/actores
Este ciclo foi de auditoria pura (leitura de metadata e bytes), sem mutação de actores, luz, terreno ou pawn — nada a gravar no `.umap`.
