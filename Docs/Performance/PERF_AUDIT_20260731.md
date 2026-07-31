# Performance Audit — Optimizer #04 (PROD_CYCLE_MANUAL_20260731)

## Conflito de directiva (12ª ocorrência consecutiva)
A directiva de sistema deste ciclo pedia `QuestManager.h/.cpp`, spawn de água e sistema de diálogo de quest —
trabalho do **#14 Quest & Mission Designer**, e uma violação directa da regra absoluta MAX `hugo_absolute_prohibitions_v1`
(nunca escrever `.cpp`/`.h`: o binário deste editor headless é pré-compilado e nunca recompila).
Não executado. Mantive-me estritamente na função de Performance Optimizer.

## Estado medido do mundo live (via `ue5_execute`, dados reais, não fabricados)

### Censo de actores
- **Total de actores no mapa:** 3272
- **Dentro do núcleo jogável** (x -3000..5000, y -1000..5500): **2691**
- Distribuição por tipo (top): PointLight 822, StaticMeshActor 575, Pawn 512, Actor 391,
  NiagaraActor 268, AmbientSound 247, TriggerBox 136, TargetPoint 120, SkeletalMeshActor 56,
  TriggerSphere 40, SpotLight 36, Emitter 17, DefaultPawn 13, RectLight 10, DecalActor 5.

### Luzes (o maior risco aparente de frame budget)
822 PointLights pareceria alarmante à primeira vista, mas a mobilidade real desfaz o alarme:
- **STATIC: 781** (bakeadas, custo de runtime ~zero — pagam-se só no lightmap build)
- **STATIONARY: 22** (custo moderado, shadow map só para a luz principal)
- **MOVABLE: 19** (as únicas realmente caras por frame)
- **Shadow-casting real:** apenas **1** de 822 — excelente, o maior custo de uma luz dinâmica
  (sombra recalculada por frame) está praticamente ausente.
- **Raio de atenuação > 2000uu:** 34 luzes, mas nenhuma delas MOVABLE (confirmado por re-leitura).

**Acção tomada:** tentativa de capar raio das 19 luzes MOVABLE a 1500uu e desligar sombras dinâmicas
desnecessárias. **Resultado verificado por releitura da mesma propriedade:** 0 alterações necessárias —
as 19 já cumpriam o orçamento (raio ≤1500, sem sombra). Ou seja, o sistema de luzes já está dentro do
orçamento de performance; não há dívida técnica aqui neste momento.

### Skeletal Meshes
- 56 `SkeletalMeshActor`, todos os 56 com animação a tocar (`saved_playing=True`).
  Nenhum "morto" a consumir slot de skinning sem propósito visual.

### Niagara (VFX)
- 268 `NiagaraActor` no total; **255 dentro do núcleo jogável**, **13 fora**.
  Os 13 fora do núcleo (ex.: zonas que o jogador raramente visita) representam ticking desperdiçado
  se não tiverem culling por distância configurado — **recomendação para #17 (VFX Agent)**: confirmar
  `use_fixed_bounds` / distance culling nesses 13, não removi nada (não é a minha área de conteúdo).

### Console stat
- Executado `stat unit` no editor live (overlay de Game/Render/GPU thread) como baseline de instrumentação
  para o próximo ciclo de QA/perf.

## Conclusão
Não foi necessário nenhum corte de qualidade visual: o sistema de luzes dinâmicas já respeita o orçamento
(19 movable, raio ≤1500uu, 1 shadow-caster). O risco real e não resolvido é o comportamento de 13 efeitos
Niagara fora do núcleo jogável, que fica para o #17 confirmar/corrigir.

## Dependências / próximo agente (#05 Procedural World Generator)
- Nenhum bloqueio de performance impede a geração de mais mundo: orçamento de luzes e skeletal meshes
  está saudável no estado actual (2691 actores no núcleo).
- Ao gerar novas zonas, manter a mesma proporção STATIC:MOVABLE em luzes (>95% static) para não
  degradar o orçamento.
- Issue aberta para #17: rever culling de distância nos 13 NiagaraActor fora do núcleo jogável.
