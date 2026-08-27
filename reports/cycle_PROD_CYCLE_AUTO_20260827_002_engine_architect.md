# Engine Architect (#2) — Ciclo PROD_CYCLE_AUTO_20260827_001 (2ª passagem)

## Contexto
O ciclo anterior (#1 Studio Director) reportou WARN global (R01, R18) e delegou trabalho a #6/#8/#11 sobre flora anacrónica, fog e comportamento. Como Engine Architect, a minha responsabilidade não é essa produção de conteúdo — é auditar e reparar a **arquitectura de Blueprint** de que #3, #5 e #11 dependem para spawnar criaturas móveis novas.

## Auditoria realizada (3 chamadas `ue5_execute`, todas com efeito/prova verificável)

### 1. Estado do PIE
`LevelEditorSubsystem.is_in_play_in_editor()` → `False`. Mundo acessível, nenhuma cegueira de instrumento para os próximos agentes.

### 2. Inventário real dos Blueprints de criatura móvel
Localizados em `/Game/TranspersonalGame/Blueprints/Creatures/` (não em `/Game/Blueprints/` como a memória antiga indicava — caminho corrigido):

| Blueprint | Bytes (disco) | Mesh atribuída | AI Controller |
|---|---|---|---|
| BP_Creature_Ankylosaurus | 39 618 B | SKM_Ankylo_Mesh | BP_Creature_AnkyloAIController |
| BP_Creature_Pachycephalosaurus | 40 936 B | SKM_Pachycephalo | BP_Creature_AIController |
| BP_Creature_Parasaurolophus | 40 477 B | SKM_Parasaurolophus_Mesh | BP_Creature_AIController |
| BP_Creature_Triceratops | 34 497 B | SKM_Triceratops | BP_Creature_AIController |
| BP_Creature_Velociraptor | 38 294 B | SKM_Velociraptor_Skin | BP_Creature_AIController |

**Nenhum destes é casca vazia** (todos >34 KB, todos com mesh real e AI Controller real atribuídos, animation_mode=ANIMATION_SINGLE_NODE). Isto contradiz a suposição de que só o Triceratops existia — há 5 espécies com Blueprint Character funcional pronto a usar por #5/#11.

### 3. DEFEITO ARQUITECTURAL ENCONTRADO E CORRIGIDO
Ao verificar o contrato "cápsula = medida da própria malha" (regra de memória `hugo_navegacao_e_criaturas_moveis_v1`), encontrei:

- **BP_Creature_Pachycephalosaurus**: cápsula (r=152.6, h=256.9) — **estes são exactamente os valores do Triceratops**, não os da própria malha (extent real = 100.6 x 328.8 x 175.8). Cápsula sobredimensionada em ~46% no raio e ~46% na altura → colisão errada, possível bloqueio de movimento ou penetração incorrecta no terreno.
- **BP_Creature_Parasaurolophus**: cápsula raio=194.3 vs extent.x real=100.3 (quase o dobro).

**Causa provável**: um agente anterior derivou estes Blueprints por duplicação do Triceratops (prática correcta, per `hugo_navegacao_v1`: "derivar as outras especies DESTE") mas não recalculou a cápsula para a malha própria antes de gravar.

**Correcção aplicada**:
```
capsule_radius = extent.x da própria malha
capsule_half_height = extent.z da própria malha
mesh.relative_location.z = -capsule_half_height
```
- Pachycephalosaurus: r 152.6→100.6, h 256.9→175.8
- Parasaurolophus: r 194.3→100.3, h 215.9→215.9 (altura já estava correcta)

Ambos recompilados (`BlueprintEditorLibrary.compile_blueprint`, obrigatório) e gravados (`EditorLoadingAndSavingUtils.save_packages`, lista explícita de 2 packages).

**Prova por releitura independente (invocação separada)**: capsule e mesh_z relidos do CDO batem com o extent da malha em ambos (`radius_ok=True height_ok=True meshz_ok=True`); ficheiros `.uasset` no disco confirmados com mtime actualizado nesta sessão.

## Estado dos restantes 3 (Ankylosaurus, Triceratops, Velociraptor)
Já correctos — cápsula bate com a própria malha, yaw da mesh = -90° (regra keyword obrigatória), mesh_z = -half_height. Não tocados.

## O que NÃO fiz
- Não toquei em C++ (.cpp/.h) — proibição absoluta, inerte neste editor.
- Não spawnei criaturas novas — essa é função exclusiva do #5 (`spawn_criatura.py`).
- Não mexi no PlayerStart, Landscape, sol ou pawn do jogador.
- Não usei `time.sleep()` nem chamadas em série a depender de tempo real.

## Impacto para a cadeia
- **#5 (World Generator)**: pode agora usar `BP_Creature_Pachycephalosaurus` e `BP_Creature_Parasaurolophus` com confiança — a cápsula já não vai causar penetração/flutuação por colisão errada quando o `spawn_criatura.py` assentar pela pata.
- **#11 (Dino Behavior)**: os 5 Blueprints estão prontos para o "pastor" (post-tick callback) — todos com `ai_controller_class`, `character_movement.max_walk_speed` (140–300 uu/s conforme espécie) e animação single-node configurados.
- **#3 (Core Systems)**: nenhuma acção C++ necessária; a arquitectura de Blueprint está consistente nas 5 espécies.

## Ficheiros
- `reports/cycle_PROD_CYCLE_AUTO_20260827_002_engine_architect.md` (este ficheiro)

## Ferramentas usadas
4x `ue5_execute` (leitura PIE, inventário de assets, auditoria de cápsulas + recompile, verificação independente por releitura). Todas com efeito ou prova verificável em disco/mundo. 0 escritas de C++.
