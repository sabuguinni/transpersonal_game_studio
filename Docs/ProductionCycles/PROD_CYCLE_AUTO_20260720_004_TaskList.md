# Studio Director — Task List Real (PROD_CYCLE_AUTO_20260720_004)

## Auditoria real do mundo (via ue5_execute, MinPlayableMap, budget $46.35/$100)

Esta auditoria substitui suposições por dados verificados directamente no editor UE5 ao vivo.

### 1. ALARME CRÍTICO — Actor PLAYER0 não encontrado
- Procura exaustiva (label exacto "PLAYER0", case-insensitive, por classe TranspersonalCharacter/Character/Pawn perto da origem) **não encontrou nenhum actor com o label PLAYER0**.
- Existem **24 actores de classe TranspersonalCharacter** com labels distintos (`MainPlayerCharacter`, `Char_Player_001`, `EnhancedTribalCharacter`, `PrehistoricSurvivor_Player` — duplicado 2x, `PlayerChar_Preview_Hub_001`, etc.), muitos empilhados nas mesmas coordenadas (0,0) ou no hub (2100,2400).
- Isto é uma repetição do padrão já registado em memória (49 clones TranspersonalCharacter com mobility STATIC em Jul/2026). Não toquei em nenhum destes actores — a regra HANDS OFF aplica-se mesmo quando o actor esperado não existe, porque não há evidência de qual (se algum) é o pawn realmente controlado pelo jogador via Pixel Streaming.
- **Acção requerida de #02 Engine Architect**: identificar, com o Hugo, qual destes 24 actores (se algum) é o pawn activo ligado ao PlayerController real, e depreciar/isolar os restantes 23 como NPCs ou remover via script de manutenção dedicado (não por um agente de produção). Isto não é uma decisão que este agente deva tomar sozinho — é um conflito crítico de arquitectura.

### 2. Terreno (Agent #5) — CONFIRMADO REAL
- Sublevel `/Game/Terrain/Terrain_Savana` presente, com `Landscape1` + `InstancedFoliageActor`. Traces de linha nas 3 amostras do hub (2500,2100 / 2100,2800 / 1700,2100) devolveram hits válidos no Landscape — chão real existe sob os dinossauros do hub.
- Não modifiquei o terreno, a foliage nem o sublevel (regra HANDS OFF respeitada).

### 3. Dinossauros no core jogável (Agent #9/#10) — PARCIALMENTE REAL, MUITO RUÍDO
- 639 actores com keyword de dinossauro no nome, dentro do core jogável (x -3000..5000, y -1000..5500).
- Breakdown de classes no core: 407 StaticMeshActor, 56 SkeletalMeshActor, 350 Pawn, 55 Character — ou seja, existem meshes reais, não só markers.
- MAS: grande parte dos "DINO" encontrados são `Helper_Actor_*`, `Marker_AI_*`, `DebugText_*`, `Light_Aux_*`, `Note` — artefactos de debug/desenvolvimento acumulados por agentes anteriores, não gameplay real. Exemplos: `DebugText_Trike_Savana_012..058` (17+ actores só de texto de debug, alguns fora do core em x=-10000).
- **Duplicação confirmada** contra a regra `hugo_naming_dedup_v2`: múltiplos `Trike_Savana_004/005/006/007/008` e `Trike_Hub_001` coexistem com `Helper_Actor_*` e `Marker_AI_*` referenciando presumivelmente os mesmos indivíduos.
- **Acção requerida de #6/#9/#10**: converter/eliminar (via script de manutenção, não delete manual em massa) os `DebugText_*`, `Marker_AI_*` e `Helper_Actor_*` que não têm SkeletalMesh real associado, e consolidar por Type_Bioma_NNN único por indivíduo. Verificar grounding real (z vs trace) dos SkeletalMeshActor Trike/TRex nomeados — tentativa de leitura directa da propriedade `location` do HitResult falhou por restrição de protected property no Python bridge (limitação técnica, não bloqueadora: usar `hit.location` via `unreal.HitResult` struct helper ou `find_look_at_rotation`/`get_world_location` em vez de `get_editor_property` na próxima tentativa).

### 4. HUD de sobrevivência (Agent #12) — ASSET REAL ENCONTRADO
- Confirmado no Asset Registry: `WBP_SurvivalHUD` (WidgetBlueprint real), `BP_SurvivalHUDManager` e `BP_SurvivalStatsDisplay` (Blueprints).
- Isto é diferente dos ~15 actores-placeholder no nível chamados `SurvivalHUD_Health`, `Hunger_Indicator`, `Health_Indicator`, etc. (classe genérica `Actor`, sem lógica) — esses são ruído/leftover, não o HUD funcional.
- **Acção requerida de #12**: confirmar que `WBP_SurvivalHUD` está de facto adicionado ao viewport via `Create Widget + Add to Viewport` no BeginPlay do GameMode/PlayerController real (uma vez resolvido o alarme do item 1), e não apenas existente como asset órfão. Reportar com print/log de confirmação, não apenas existência do ficheiro.

### 5. Volume total de actores — risco de performance/cap
- 3829 actores no nível total; 3216 dentro do core jogável (excluindo Landscape/Foliage).
- 219 AmbientSound + 256 NiagaraActor + 806 PointLight só no core — potencial risco de performance que cabe a #04 avaliar antes da próxima build.

## Task List concreto para o próximo ciclo (por agente)

- **#02 Engine Architect**: Resolver o alarme do Pawn do jogador (item 1) — é um CONFLITO CRÍTICO, escalado para decisão conjunta Miguel/Hugo antes de qualquer outro agente tocar em Characters/Pawns.
- **#05/#06**: Nenhuma acção necessária — terreno confirmado bom, não mexer.
- **#09/#10**: Limpar/consolidar duplicação de dinossauros no hub e Savana (ver item 3), garantir 1 actor por indivíduo com Type_Bioma_NNN, remover DebugText/Marker órfãos via script de manutenção dedicado.
- **#12**: Confirmar activação real do `WBP_SurvivalHUD` no viewport em runtime (não só existência do asset).
- **#04**: Avaliar footprint de performance dos 3216 actores no core (806 PointLights, 256 Niagara) antes da próxima milestone.
- **#18 QA**: Bloquear build até o alarme PLAYER0 (item 1) ser resolvido — não há garantia de qual pawn é controlado pelo jogador humano em Pixel Streaming.

## Verificação (ue5_execute real, 6 chamadas neste ciclo)
1. Bridge check + contagem de actores "dino" (639) e "HUD" (21 labels) no nível.
2. Contagem de actores no core jogável por classe (3216 total, breakdown completo).
3. Busca exaustiva por PLAYER0 exacto — não encontrado.
4. Busca alargada case-insensitive + Character/Pawn perto da origem — 24 TranspersonalCharacter duplicados encontrados.
5. Verificação de assets UMG reais (WBP_SurvivalHUD confirmado) + 3 traces de linha ao Landscape no hub (todos com hit válido).
6. Tentativa de extrair Z do HitResult — falhou por restrição de propriedade protegida no bridge Python (limitação técnica registada, não escondida).

Nenhum actor foi criado, movido ou apagado neste ciclo — este ciclo foi 100% auditoria verificada para desbloquear decisões de arquitectura antes de mais produção.
