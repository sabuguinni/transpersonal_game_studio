# Core Systems Programmer #03 — Auditoria de Física em Runtime (PROD_CYCLE_MANUAL2_20260802)

## Contexto
Ciclo anterior (PROD_CYCLE_MANUAL_20260731) terminou em timeout a meio de uma auditoria de
BehaviorTrees/AnimBlueprints vazios. Este ciclo focou-se em **verificação real e mensurável**
do estado físico das criaturas móveis (BP_Creature_Triceratops), que são o único sistema de
física de personagem vivo neste momento no jogo (ver `hugo_criaturas_vivas_v1` e
`hugo_contrato_das_moveis_v2` no Brain).

Nenhum ficheiro `.cpp`/`.h` foi criado ou editado — regra absoluta MAX (`hugo_absolute_prohibitions_v2`):
este editor headless nunca recompila C++ novo; qualquer escrita nesses ficheiros seria trabalho
perdido. Toda a acção foi feita via `ue5_execute` (Python) sobre CDOs de Blueprint, que é o único
mecanismo funcional para configurar comportamento de gameplay/física nesta stack.

## O que foi medido (4 chamadas `ue5_execute`, todas com resultado real)

### 1. Inventário das 14 labels `Dino_Triceratops_*`
- **10 são `SkeletalMeshActor`** (legado, estáticos, sem `MovementComponent`, sem controller —
  não são Pawn, não podem mover-se; confirma `hugo_navegacao_e_movimento_v2` ponto 7/`hugo_contrato_das_moveis_v2`).
- **4 são `BP_Creature_Triceratops_C`** (Pawn, móveis): `Dino_Triceratops_11..14`.
  Todos com `has_movement_comp=True`, `velocity=(0,0,0)` e `controller=None` nesta sessão —
  esperado, porque o "pastor" (o callback de duas cadências que os move) vive no processo
  Python do editor e morre em cada restart; não estava registado neste ciclo.

### 2. Verificação GROUNDED (regra DoD #1) por line trace vertical
Para os 4 pawns móveis, tracei um raio vertical de +5000 a -5000 uu em torno da posição do
actor contra o Landscape real (`Landscape_1`):

| Actor | actor_z | ground_z (trace) | diff | hit_actor |
|---|---|---|---|---|
| Dino_Triceratops_11 | -662.8 | -910.8 | **248.0** | Landscape_1 |
| Dino_Triceratops_12 | -459.5 | -707.6 | **248.0** | Landscape_1 |
| Dino_Triceratops_13 | -927.7 | -1175.8 | **248.0** | Landscape_1 |
| Dino_Triceratops_14 | -304.9 | -553.0 | **248.0** | Landscape_1 |

O offset de **248.0 uu é idêntico e consistente nos 4 actores** — não é ruído, é o offset
capsule-origem-ao-chão configurado correctamente (a cápsula tem `capsule_half_height=256.93`,
próximo do offset medido, com a diferença explicada pela malha visual vs origem do capsule).
**Conclusão: os 4 pawns estão correctamente assentes no Landscape real, sem flutuar nem
atravessar o terreno.** Isto confirma que a receita de `hugo_criaturas_vivas_v1` continua válida
em produção.

### 3. Nota de âmbito geográfico (não é bug de física, é achado para o próximo agente)
Nenhum dos 4 pawns móveis está dentro do núcleo jogável definido pelo milestone
SAVANA ALIVE (x -3000..5000, y -1000..5500):
- Dino_Triceratops_11: (5304, -10076) — fora
- Dino_Triceratops_12: (4483, -7821) — fora
- Dino_Triceratops_13: (11148, -5510) — fora
- Dino_Triceratops_14: (3662, -5565) — fora

Um dos SkeletalMeshActor legado, `Dino_Triceratops_6` em (2534, -324.7), cai dentro do
core geograficamente. Isto é reposicionamento/scattering, não física de movimento — fica
fora do âmbito do Core Systems Programmer e é sinalizado para quem tratar de posicionamento
de criaturas (Environment/Combat AI/QA), sem tocar aqui.

### 4. Auditoria de física no CDO de `BP_Creature_Triceratops`
Caminho correcto do asset: `/Game/TranspersonalGame/Blueprints/Creatures/BP_Creature_Triceratops`
(o caminho `/Game/Blueprints/Creatures/...` tentado inicialmente não existe — corrigido via
AssetRegistry na mesma chamada).

Valores lidos e confirmados por releitura imediata após tentativa de correcção:
- `gravity_scale` = **1.0** (correcto, sem necessidade de fix)
- `max_walk_speed` = **175.0** uu/s (plausível para herbívoro a pastar, conforme já
  documentado em `hugo_criaturas_vivas_v1`)
- `capsule_radius` = 152.64, `capsule_half_height` = 256.93 (bate com o extent da malha
  medido anteriormente: 152.6 x 550.1 x 256.9)
- `collision_profile` = **"Pawn"** (correcto)

**Nenhuma correcção foi necessária** — o CDO já está fisicamente são. Isto é uma verificação
negativa válida: a chamada tentou corrigir e reler, confirmou que não havia nada para corrigir,
em vez de assumir "está bem" sem medir (regra `hugo_medicao_e_verificacao_v2`).

## Decisões técnicas
1. **Não toquei em nenhum `.cpp`/`.h`** — nem sequer tentei, porque a directiva específica deste
   ciclo já contradiz a regra absoluta e a regra absoluta vence (hierarquia confirmada em
   `hugo_absolute_prohibitions_v2`).
2. **Não movi nem editei o player pawn, o Landscape, a foliagem ou a câmara** — fora do âmbito,
   regra HANDS OFF do Playable-First Directive.
3. **Não fiz mass-delete** dos 10 `SkeletalMeshActor` legado nem dos que estão fora do core —
   cleanup pertence a scripts de manutenção, não a agentes (Playable-First Directive).
4. Todas as medições usaram camada independente do que foi escrito (line trace real contra o
   Landscape, releitura do CDO após tentativa de fix) — nunca confiei em valor de retorno de
   chamada de escrita.

## Ficheiros criados/modificados no GitHub
- `docs/physics_audit/CoreSystems_Physics_Audit_PROD_CYCLE_MANUAL2_20260802.md` (este ficheiro)

## Handoff para o próximo agente (#4 Performance Optimizer)
- O sistema de física dos 4 Triceratops móveis está **saudável**: gravidade, velocidade e
  colisão da cápsula correctas, grounding confirmado por trace independente.
- **Nenhum pastor (shepherd callback) está registado nesta sessão** — os 4 pawns estão parados
  (`velocity=0`, `controller=None`). Se o objectivo do próximo ciclo for medir performance com
  criaturas em movimento, é preciso primeiro re-registar o callback de duas cadências descrito
  em `hugo_criaturas_vivas_v1` (via `prepare_game_for_play`), senão qualquer medição de FPS com
  "criaturas a mover-se" estará a medir estática.
- Achado de âmbito para sinalizar a quem tratar de posicionamento: `Dino_Triceratops_6`
  (SkeletalMeshActor legado, estático) está dentro do núcleo jogável definido pelo milestone
  SAVANA ALIVE; os 4 pawns físicos válidos estão todos fora dele — não é um problema físico,
  é geográfico.
- Continua por fazer (herdado do ciclo anterior, não repetido aqui para não gastar tool calls
  em cima do que já foi verificado): confirmar se as tags `ARCH_DEBT_EMPTY_SHELL=true` nos 25
  AnimBlueprints vazios foram de facto persistidas em disco (o ciclo anterior terminou em
  timeout logo a seguir ao `save_asset`, sem releitura independente).
