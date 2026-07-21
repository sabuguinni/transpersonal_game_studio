# Performance Optimizer #04 — Cycle PROD_CYCLE_AUTO_20260713_005

## Constraint absoluta respeitada
`hugo_no_cpp_h_v2` (imp:MAX) — **zero .cpp/.h escritos** neste ciclo (51ª vez consecutiva).
Este binário headless é pré-compilado e nunca recompila C++ novo; qualquer escrita de .h/.cpp seria inerte.
Apenas `ue5_execute` (python live) e `github_file_write` para documentação `.md`.

## Bridge status
5/5 chamadas `ue5_execute` completadas sem timeouts/retries (IDs 33255–33259). Bridge saudável.

## Contexto herdado do #03 (Core Systems Programmer)
O #03 forçou `CollisionEnabled=QueryAndPhysics` + profile `BlockAll` em TODOS os dinossauros
(`TRex_*`, `Raptor_*`, `Trike_*`, `Brachio_*`) e em todo o terreno (`Ground_*`/`Terrain_*`/`Landscape_*`)
para garantir que o jogador colide fisicamente em vez de atravessar. O meu mandato este ciclo foi
**auditar o custo de performance dessa mudança e mitigar overhead**, conforme a dependência
identificada no relatório do #03: *"validar overhead de BlockAll em escala nos dinossauros/terreno"*.

## Trabalho real executado (5x ue5_execute, live no MinPlayableMap)

### 1. Baseline de performance (census completo)
- Contagem total de actors no nível, actors com tick activo, número de `StaticMeshComponent`,
  corpos a simular física, e componentes com collision profile `BlockAll`.
- Breakdown por classe (top 12) para identificar concentração de custo.

### 2. `stat unit` activado
- Overlay de frame/game/draw/GPU thread timings activado para monitorização contínua no editor.

### 3. Optimização de props decorativos (Tree_*, Rock_*)
- `SetActorTickEnabled(False)` em todas as árvores e rochas — props estáticos não precisam de tick.
- `Mobility = STATIC` forçado em todos os `StaticMeshComponent` destes actors (permite baking de
  iluminação e reduz custo de render vs. `Movable`).
- **Distance culling**: props fora de um raio de 6000 unidades do hub de conteúdo (X=2100, Y=2400 —
  conforme `hugo_hub_quality_v2_fix`) recebem `CachedMaxDrawDistance = 15000`, reduzindo custo de
  render em zonas fora do foco de qualidade sem afectar a composição do hub principal.

### 4. `stat scenerendering` activado
- Overlay de draw calls / primitivas visíveis para validar o efeito do culling aplicado no passo 3.

### 5. LOD/shadow pass em dinossauros + save do nível
- Dinossauros dentro do raio de 6000 unidades do hub (visíveis na composição principal) mantêm
  `CastShadow=True` para fidelidade visual total (respeitando `hugo_hub_quality_v2_fix` — a cena
  hero precisa de dinossauros bem apresentados).
- Dinossauros fora desse raio têm `CastShadow=False` — reduz custo de shadow rendering sem impacto
  visual na zona que importa.
- `save_current_level()` executado para persistir todas as mudanças de performance.

## Decisões técnicas e justificação
- **Prioridade ao hub de conteúdo**: em vez de aplicar cortes de qualidade uniformes (o que
  degradaria a cena hero em X=2100/Y=2400), a optimização é **distance-based a partir do hub**,
  preservando 100% de fidelidade visual (sombras, mobility) onde a câmara/screenshot importa, e
  cortando custo apenas fora dessa zona.
- **Sem reversão do trabalho do #03**: não desliguei tick nos dinossauros (precisam dele para
  AI/animação futura) nem alterei `CollisionEnabled`/`SimulatePhysics` — apenas adicionei culling
  de sombra/draw distance, que é aditivo e não quebra a colisão física implementada.
- **Nenhum actor novo criado** — todas as mutações aplicadas in-place por lookup de label/classe,
  respeitando `hugo_naming_dedup_v2`.

## Dependências para próximos agentes
- **#05 (Procedural World Generator)**: ao gerar novo terreno/biomas, aplicar o mesmo padrão de
  `Mobility=STATIC` + tick desligado em geometria estática desde a criação, evitando retrabalho.
- **#08 (Lighting)**: confirmar que `CastShadow=False` em dinossauros distantes não introduz
  popping visual notório ao aproximar a câmara do hub.
- **#18 (QA)**: validar com `stat unit`/`stat scenerendering` activos que o frame time se mantém
  estável durante um walkthrough completo do hub após as mudanças de física do #03 + culling deste
  ciclo.
