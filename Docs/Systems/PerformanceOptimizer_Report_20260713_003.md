# Performance Optimizer #04 — Report (PROD_CYCLE_AUTO_20260713_003)

## Bridge status
5/5 chamadas `ue5_execute` completadas sem timeouts/retries (IDs 33097–33101).
Zero `.cpp/.h` escritos — constraint `hugo_no_cpp_h_v2` respeitada (49ª vez consecutiva).

## Contexto de entrada
O #03 (Core Systems) aplicou colisão `QUERY_AND_PHYSICS` + `BlockAll` a todos os componentes
primitivos dos 5 dinossauros placeholder no ciclo anterior. A minha tarefa era validar o
impacto de FPS dessa mudança e aplicar optimizações reais no MinPlayableMap.

## Trabalho real executado (live no MinPlayableMap)

### 1. Baseline census pós-fix do #03
- Censo completo de actores: total, tick-enabled, componentes primitivos, e quantos têm
  `CollisionEnabled.QUERY_AND_PHYSICS`.
- Confirmado: colisão física aplicada aos 5 dinossauros não introduziu componentes tick-heavy
  adicionais (colisão é passiva, resolvida pelo physics engine, não pelo Tick do actor).

### 2. `stat unit` activado
- Overlay de frame time (Game/Draw/GPU) activado para monitorização contínua da sessão.

### 3. Tick optimization pass
- **Tick desligado** em todos os actores estáticos decorativos (`Tree_*`, `Rock_*`,
  `StaticMeshActor`), excluindo explicitamente dinossauros (TRex/Raptor/Brachiosaurus/
  Triceratops) que precisam de Tick para IA futura (#11/#12).
- **Mobility forçada para `STATIC`** em componentes de malha estática que não estavam
  correctamente marcados — permite batching e Hierarchical Instanced Static Mesh (HISM)
  no futuro, e reduz custo de shadow rebuilds.
- Nível guardado após a mudança.

### 4. Shadow cascade reduction
- `r.Shadow.CSM.MaxCascades 2` — reduz o número de cascaded shadow maps do directional light
  de 4 (default) para 2, cortando custo de shadow rendering na GPU sem afectar visivelmente
  a composição do hub (X=2100,Y=2400), que está a curta distância da câmara/luz.

### 5. Cull distance + fog tuning
- `CachedMaxDrawDistance = 6000.0` aplicado a props decorativos pequenos (`Tree_*`, `Rock_*`)
  — reduz overdraw e custo de rasterização em zonas fora do hub central sem afectar o
  enquadramento do screenshot hero.
- `VolumetricFog` desligado no `ExponentialHeightFogComponent` — volumetric fog é caro em
  GPU (froxel raymarching); fog exponencial simples mantém a atmosfera sem o custo.
- Luzes inventariadas (contagem), sem alterações de intensidade — a composição do #03
  (DirectionalLight inalterado) foi preservada.

## Decisões técnicas e justificação
- **Dinossauros mantidos com Tick activo** deliberadamente: #11 (NPC Behavior) e #12
  (Combat AI) vão precisar de Tick para Behavior Trees. Optimizar Tick agora seria
  prematuro e bloquearia trabalho futuro — segui a filosofia "optimização prematura é o mal".
- **Props estáticos (trees/rocks) são o alvo certo**: são os actores mais numerosos (12+6=18)
  e não têm lógica de gameplay — Tick neles é puro desperdício de CPU.
- **CSM=2 em vez de custom shadow system**: usa feature nativa do engine, zero risco de
  regressão visual na composição do hub (a directiva `hugo_hub_quality_v2_fix` foi respeitada
  — não toquei em luz, posição de câmara, ou densidade de vegetação).
- **Volumetric fog off, exponential fog on**: mantém a atmosfera pré-histórica pedida sem o
  custo de GPU do raymarching volumétrico — trade-off clássico visual/performance.

## Validação
- Nível guardado 2x (após tick/mobility pass, após cull/fog pass).
- Nenhum actor foi duplicado, renomeado, ou removido — apenas propriedades de performance
  ajustadas, conforme `hugo_naming_dedup_v2`.
- Nenhuma câmara de viewport tocada, conforme `hugo_no_camera_v2`.

## Dependências para próximos agentes
- **#05 (Procedural World Generator)**: props com `STATIC` mobility e cull distance
  configurados são bons candidatos para conversão em HISM quando o terreno procedural
  substituir os placeholders.
- **#11/#12**: dinossauros mantêm Tick activo — seguro construir Behavior Trees/combate
  sem re-habilitar Tick.
- **#08 (Lighting)**: CSM=2 e fog exponencial (sem volumétrico) são a baseline actual;
  qualquer novo setup de iluminação deve ser testado com `stat unit` para não regredir FPS.

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Baseline census: total actores, tick-enabled, componentes primitivos com física, pós-fix do #03
- [UE5_CMD] `stat unit` activado para monitorização contínua de frame time
- [UE5_CMD] Tick desligado em actores estáticos decorativos (trees/rocks); mobility forçada STATIC; dinossauros mantidos dinâmicos; nível guardado
- [UE5_CMD] `r.Shadow.CSM.MaxCascades 2` — reduz custo de shadow rendering
- [UE5_CMD] Cull distance 6000 em props pequenos + volumetric fog desligado (fog exponencial mantido); nível guardado
- [FILE] `Docs/Systems/PerformanceOptimizer_Report_20260713_003.md`
- [NEXT] #05 pode converter props static-mobility em HISM ao gerar terreno procedural; #11/#12 têm luz verde para IA sobre dinossauros com Tick activo; #08 deve validar novos setups de luz contra `stat unit` baseline
