# Performance Optimizer #04 — Cycle PROD_CYCLE_AUTO_20260710_007

## Constraint absoluta respeitada
`hugo_no_cpp_h_v2` (imp:MAX): zero ficheiros `.cpp`/`.h` escritos. Todo o trabalho de optimização foi feito ao vivo no `MinPlayableMap` via Remote Control Python, sobre os actores já corrigidos pelo #03 (colisão `QueryAndPhysics` + canais `ECC_WorldStatic`/`ECC_Pawn`).

## Contexto recebido do #03
O #03 aplicou colisão física correta (`QueryAndPhysics`) a todos os dinossauros e props (Tree/Rock), garantindo canais corretos (`ECC_WorldStatic` para props, `ECC_Pawn` para dinossauros) e capsule profile `Pawn` no jogador. Isto introduz custo adicional de física/colisão que precisa de ser controlado para não degradar o frame budget — é exatamente o trabalho deste ciclo.

## Trabalho executado (5x `ue5_execute`)
1. **Bridge validation + auditoria** — Confirmei mundo carregado, contei todos os actores por classe, e medi baseline: quantos actores têm tick activo, quantos componentes simulam física, quantos componentes têm colisão `QueryAndPhysics`.
2. **`stat unit`** — Activei overlay de timing (Frame/Game/Draw/GPU thread) para baseline de performance em runtime.
3. **`stat fps`** — Activei contador de FPS em tempo real para monitorização contínua.
4. **Optimização aplicada**:
   - Props estáticos (Tree_*, Rock_*): `SetActorTickEnabled(False)` — decoração nunca precisa de tick, poupança directa de custo de CPU por frame.
   - Props estáticos: `SimulatePhysics = False` explicitamente forçado (mesmo tendo colisão `QueryAndPhysics` para o jogador colidir, não devem simular física — evita custo de solver físico desnecessário em geometria que nunca se move).
   - Cull distance (`CachedMaxDrawDistance`) definida em props estáticos (8000uu) para reduzir draw calls a longa distância.
   - Cull distance definida em dinossauros (12000uu, maior por serem meshes de maior escala/importância visual) para proteger o budget de GPU sem esconder demasiado cedo os elementos-chave da cena (hero shot em X=2100, Y=2400).
5. **Validação readback** — Reconfirmed que nenhum prop estático ainda tem tick activo, nenhum tem física a simular, e que os dinossauros têm cull distance aplicada. Zero regressões detectadas.

## Decisões técnicas e justificação
- **Tick desligado em decoração estática**: Tree/Rock nunca precisam de `Tick()` — é overhead puro. Em cenas com dezenas de props, isto soma-se rapidamente no game thread.
- **Física NÃO simulada em props estáticos apesar de colisão `QueryAndPhysics`**: a colisão do #03 serve para o jogador bater fisicamente contra árvores/rochas (bloqueio de movimento), mas os próprios props não precisam de ser corpos físicos activos no solver — isso seria desperdício e risco de instabilidade (tombar sozinhos).
- **Cull distances diferenciadas**: dinossauros (elementos hero do content hub, memória `hugo_hub_quality_v2_fix`) têm cull distance maior que props genéricos, para garantir que continuam visíveis no hero screenshot em X=2100/Y=2400 mesmo a alguma distância da câmara.
- **Sem novos actores spawnados**: respeita `hugo_naming_dedup_v2` — trabalho de optimização puro sobre actores existentes, sem duplicação.
- **Sem alteração da câmara do editor**: respeita `hugo_no_camera_v2`.

## Ficheiros modificados
- `Docs/Performance/Cycle007_Perf_Optimization_Pass.md` (novo)

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Auditoria de performance: contagem de actores por classe, actores com tick activo, componentes a simular física, componentes com colisão `QueryAndPhysics`.
- [UE5_CMD] `stat unit` + `stat fps` activados para monitorização contínua de frame budget.
- [UE5_CMD] Optimização aplicada: tick desligado em props estáticos, física não simulada em props, cull distances configuradas (8000uu props / 12000uu dinossauros).
- [UE5_CMD] Validação readback confirmando zero regressões (nenhum prop a tickar, nenhum a simular física, cull distances persistidas).
- [FILE] `Docs/Performance/Cycle007_Perf_Optimization_Pass.md`
- [NEXT] #05 (Procedural World Generator): usar os canais de colisão consistentes (`ECC_WorldStatic`) como base para geração procedural de terreno/biomas sem conflitos de colisão. #12 (Combat AI): construir traces de combate sobre `ECC_Pawn` já validado nos dinossauros. #19: monitorizar `stat fps`/`stat unit` overlays já activos durante testes de build para detectar regressões de performance introduzidas por outros sistemas.
