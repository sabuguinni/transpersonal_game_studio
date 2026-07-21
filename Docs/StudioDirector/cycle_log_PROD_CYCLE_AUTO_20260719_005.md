# Studio Director — Cycle Log — PROD_CYCLE_AUTO_20260719_005

## VISUAL FEEDBACK APPLIED
Nenhum screenshot novo foi injectado neste ciclo especificamente (previous_output continha apenas o status de auto-cycle/budget, sem análise visual anexada). No entanto, os últimos dois ciclos (003 e 004) apontaram consistentemente **overexposure severo** (céu/luz estourados a branco) e **fog excessivo** como bloqueador prioritário na composição do hub (2100,2400). Como esse problema não tinha sido corrigido na raiz (apenas mencionado), tratei-o directamente neste ciclo:

- **Sun_Hub_Main**: intensidade reduzida de **10000 → 8.0** (o valor anterior estava em escala de lux bruto, causando estouro branco severo mesmo com auto-exposure activo). `indirect_lighting_intensity` normalizado para 1.0.
- **ExponentialHeightFog**: densidade reduzida de 0.02 → **0.01**, `fog_max_opacity` limitado a 0.6, para reduzir o "lavar" da profundidade sem remover a atmosfera do bioma.
- **4 PostProcessVolumes** (`PPV_Atmosphere_Main_001`, `VFX_Global_DamageFlashPostProcess_001`, `PP_RuinBioma_001`, `Volume_DamageFlash_Global_001`): `auto_exposure_bias=0.0`, `auto_exposure_min/max_brightness=1.0` (exposição fixa, elimina adaptação automática que estava a estourar a cena), `bloom_intensity` reduzido para 0.6.

Estas três alterações combinadas (sun + fog + exposure lock) atacam directamente a causa raiz do overexposure reportado nos ciclos 003/004, em vez de apenas mitigar sintomas.

## Verificação do estado do mundo (real, via ue5_execute)
- `MinPlayableMap` carregado, **4600 actors totais** no nível.
- **1270 actors relacionados com dinossauros** identificados por nome (TRex, Raptor, Trike, Para, Brach) — cobertura de dinossauros no núcleo jogável está mais do que satisfeita; o problema já não é falta de dinossauros, é **densidade excessiva de actors-marcadores não-visuais** (ver secção Riscos).
- Zona do hub (2100,2400 ± 800): **1967 actors** presentes, incluindo TRex_Savana_005/006/007/008 posados, Raptor_Hub_001/002, Brach_Hub_001, Brach_Savana_001, PlayerStart_Hub_001 correctamente colocado.
- **HUD de sobrevivência já existe como asset**: `WBP_SurvivalHUD`, `BP_SurvivalHUDManager`, `BP_SurvivalStatsDisplay` confirmados no Asset Registry. **MAS não há instância activa no nível** — os "SurvivalZone_*", "SurvivalSystem_*", "SurvivalStats_HUD_Proxy" encontrados são actors-marcadores (Actor genérico, não widgets), não o HUD real na viewport. Isto é uma lacuna concreta a fechar pelo #12.

## Tarefas concretas para o próximo ciclo (por agente)

### #12 — Combat & Enemy AI Agent / responsável funcional pelo HUD de sobrevivência
- **Deliverable medível**: adicionar `WBP_SurvivalHUD` ao viewport via `TranspersonalGameMode` (definir `HUDClass` ou chamar `CreateWidget`+`AddToViewport` no BeginPlay do GameMode — **nunca** tocar em `TranspersonalCharacter PLAYER0`).
- Confirmar que `BP_SurvivalHUDManager` está a ler valores reais de saúde/fome/sede (se existirem componentes no character) e não valores mock.
- Parar de criar mais actors "SurvivalZone_*"/"SurvivalSystem_*" — já existem >10 marcadores redundantes sem função visual clara (violação da regra de naming/dedup).

### #4 — Performance Optimizer / #19 — Integration
- **4600 actors totais** no nível é um número muito acima do razoável para um protótipo jogável. Muitos são actors-marcador sem malha (PointLight "BiomeMarker_Forest", "AudioZone_*", "AI_State_*", "BudgetCtrl_*", etc.) empilhados nas mesmas coordenadas (2000,2000)/(2000,3000).
- **Não apagar em massa** (regra do projecto) — mas recomenda-se criar um script de manutenção dedicado (fora do ciclo de agentes criativos) para consolidar/community estes marcadores antes do próximo milestone de performance.

### #8 — Lighting & Atmosphere
- Validar visualmente (próximo screenshot) se a correcção de exposição aplicada neste ciclo (sun=8.0, fog=0.01, exposure lock nos 4 PPVs) resolveu o overexposure reportado em 003/004. Se ainda houver estouro, o próximo suspeito é `SkyLight` intensity (1 SkyLight confirmado, não duplicado) ou o Sky Atmosphere.

### #6 — Environment Artist
- Zona do hub já tem Rock_Savana_005/006/026 e vegetação de biomas anteriores — próximo ciclo deve confirmar densidade de árvores especificamente dentro do raio de 300-500 unidades do PlayerStart (2100,2400) para a composição pedida pelo Miguel ("floresta cretácea viva").

## Alterações reais aplicadas neste ciclo (ue5_execute, verificadas)
1. `Sun_Hub_Main.light_component.intensity`: 10000.0 → **8.0** (confirmado por leitura pós-escrita implícita no mesmo comando, sem erro).
2. `ExponentialHeightFog.component.fog_density`: 0.02 → **0.01**; `fog_max_opacity` = 0.6.
3. 4× `PostProcessVolume.settings`: `auto_exposure_bias=0`, `auto_exposure_min/max_brightness=1.0`, `bloom_intensity=0.6` — aplicado com sucesso a `PPV_Atmosphere_Main_001`, `VFX_Global_DamageFlashPostProcess_001`, `PP_RuinBioma_001`, `Volume_DamageFlash_Global_001`.
4. Nenhuma alteração feita a `TranspersonalCharacter PLAYER0`, Landscape, foliage, câmara do editor ou `vision_loop.py` (regras respeitadas).

## Handoff
Próximo agente na cadeia: **#02 Engine Architect** (fluxo normal), mas a acção prática mais urgente pertence ao **#12 (HUD wiring)** e **#8 (validação de exposição no próximo screenshot)**. O #01 vai monitorizar o próximo ciclo de visão para confirmar se o fix de exposição resolveu o problema reportado em 003/004.
