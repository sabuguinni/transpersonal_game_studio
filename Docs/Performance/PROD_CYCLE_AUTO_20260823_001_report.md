# Performance Optimizer — Relatório de Medição (Ciclo PROD_CYCLE_AUTO_20260823_001)

## Papel deste ciclo
Conforme a AGENT #4 SPECIFIC DIRECTIVE, este ciclo é **MEDIR e PROPOR**, nunca executar optimizações. Nenhum actor foi movido, apagado, reescalado ou tido a mobility alterada. As 4 chamadas `ue5_execute` foram todas de leitura/medição, verificadas por amostragem independente (duas leituras separadas de `frame_count`/`real_time_seconds`).

## Estado do mundo (verificado nesta sessão)
- PIE estava fechado (`is_in_play_in_editor()=False`) — sem interferência de cegueira do instrumento.
- **Total de actores: 3501** (tecto de segurança 8000 → 56% de margem real).
- `actor_tick_enabled` = **0 em todos os actores testados** — confirma memória: tick por-actor NÃO é o gargalo, não proponho desligar ticks já desligados.

### Censo por classe (top relevante)
| Classe | Contagem |
|---|---|
| StaticMeshActor | 847 |
| PointLight | 795 |
| Pawn | 512 |
| AmbientSound | 254 |
| NiagaraActor | 236 |
| TriggerBox | 123 |
| TargetPoint | 123 |
| SkeletalMeshActor | 88 |
| TriggerSphere | 35 |
| SpotLight | 34 |
| Emitter (Cascade) | 15 |
| BP_Creature_Triceratops_C | 5 |

**VFX total: 236 Niagara + 15 Cascade = 251 actores** — confirma-se como a maior população de VFX e o primeiro sítio a olhar, tal como indicado.

## Medição 1 — FPS real (sem invenção de números)
Método: `SystemLibrary.get_frame_count()` + `GameplayStatics.get_real_time_seconds()` amostrados em **duas invocações separadas** do `ue5_execute` (nunca `time.sleep()` no mesmo script, que congelaria a game thread).

- Amostra 1: frame_count=84333396, real_time=735374.155s
- Amostra 2: frame_count=84334641, real_time=735386.945s
- Δframes=1245, Δt=12.790s → **FPS médio medido = 97.3 fps**

Esta medição é em modo EDITOR ocioso (sem PIE, sem jogador a mover-se, sem animação de personagem a renderizar) — é um **piso de referência**, não o número real em jogo.

### Última FPS_SAMPLE do log (R24r)
```
frames=960 wall=8.0 pie=False fps=120.0 pior_frame_ms=9.3
frames=960 wall=8.0 pie=False fps=120.0 pior_frame_ms=9.5
frames=947 wall=8.0 pie=False fps=118.3 pior_frame_ms=125.0   <-- outlier
```
**Achado**: um dos três samples mais recentes do log mostra `pior_frame_ms=125.0`, um pico de ~13x sobre o baseline de ~8-9ms. Isto é um hitch, não degradação sustentada (fps médio da amostra manteve-se em 118.3). Não consigo atribuir a causa com os dados actuais (candidatos plausíveis: GC do editor, streaming de assets, ou o callback do "pastor" com as suas duas cadências de 0.15s/2.5s). **Recomendo ao próximo agente com acesso a profiling em runtime (QA/#18 ou Integration/#19) capturar `stat unit` / `stat gpu` durante um pico semelhante para isolar a causa.**

## Medição 2 — PointLight (795 actores, 2ª maior contagem)
- Mobility: **759 STATIC, 18 STATIONARY, 18 MOVABLE**
- `cast_shadows = 0 em todos os 795` (nenhum lança sombra dinâmica) — **já optimizado**, não há acção a propor aqui.
- Intensidade média: 832.8 | Raio médio de atenuação: 659.2 uu (~6.6 m)

**Avaliação**: com 96% das luzes STATIC e zero a lançar sombras, o custo destas 795 luzes é predominantemente de bake/lightmap (pago em tempo de build, não em fps) e overlap na acumulação de luz por pixel no Lumen. Risco de regressão ao mexer é desnecessário face ao ganho — **não proponho alteração aqui este ciclo**; é já uma configuração eficiente.

## Medição 3 — Niagara: concentração anómala perto do spawn
- 236 NiagaraActor no total. Distribuição por distância ao spawn (1200,1200,301):
  - **0-50m: 224 (95% de todos os Niagara do mundo)**
  - 50-150m: 9
  - 400m+: 3
- Dos 224 próximos do spawn, breakdown por prefixo de label: `FX_` 142, `VFX_` 78, `VFXAnchor_` 2, `Campfire_` 1, `LoreMarker_` 1.
- **Uso de asset de sistema Niagara entre os 224 próximos do spawn:**
  - `NS_Environment_CampfireSmoke`: **166 instâncias** (74% de todo o VFX próximo do jogador)
  - `NS_Dino_Footstep`: 27
  - `NS_Environment_AmbientDust`: 14
  - `NS_Environmental_Wind`: 11
  - `NS_Dust_Footstep_TRex`: 3
  - `NS_Atmospheric_DustMotes`: 2
  - `NS_Fire_Campfire`: 1

### Achado principal e proposta #1 (prioridade alta)
**166 instâncias do MESMO sistema `NS_Environment_CampfireSmoke` dentro de 50m do spawn** — exactamente onde a câmara do jogador passa a maior parte do tempo. Fumo é tipicamente um sistema alpha-blended com sub-UV/flipbook e por vezes interacção com luz — é dos tipos de VFX mais caros em overdraw/fill-rate, e overdraw escala com o número de partículas **visíveis em ecrã simultaneamente**, não com o número total no mundo.

**Proposta (para o VFX Agent #17 executar, não eu — respeito o limite de decoração A05 = 986, já em 950)**:
1. Configurar **Niagara Scalability** (LOD/distância) no próprio System Editor: reduzir contagem de partículas e taxa de emissão por distância à câmara, e usar `Scalability Cull Distance` para desactivar sistemas fora do frustum ou a média/longa distância. Isto é uma alteração ao ASSET Niagara (não ao número de actores no mundo) — custo de implementação baixo, risco baixo, não mexe em contagem de actores nem quebra o tecto A05.
2. Investigar se muitas destas 166 instâncias estão **sobrepostas ou redundantes** na mesma fogueira/agrupamento (o nome sugere fogueiras de campismo — mas o contexto anti-alucinação proíbe fogueiras tribais comunitárias; se estas são remanescentes de conteúdo antigo, cabe ao #17/#1 decidir remoção, não a mim mover/apagar actores).
3. Custo esperado da correcção: redução de fill-rate GPU proporcional ao número de partículas activas simultaneamente visíveis — mais impactante em PC gama média e em consola (30fps) do que em PC alto (que já tem margem). Risco: BAIXO se feito via scalability settings do asset; MÉDIO se envolver fusão/remoção de actores (decisão de outro agente).

## Medição 4 — Triggers e Audio
- TriggerBox 123 + TriggerSphere 35 = 158 volumes de colisão. `TriggerBox` não expõe `actor_tick_enabled` como propriedade legível nesta API (erro de atributo confirmado) — mas por serem volumes de overlap event-driven (não polling), o custo é desprezável. **Sem acção proposta.**
- AmbientSound: 254 actores, apenas **2 estão a mais de 400m do spawn** (candidatos a orfãos/redundantes se o attenuation radius for pequeno — verificação de baixo custo, sem risco, recomendo ao próximo agente de áudio confirmar se têm falloff configurado correctamente para não processar quando o jogador está longe).

## Tabela-resumo de propostas (ranked)
| # | Métrica | Valor medido | Proposta | Custo esperado | Risco |
|---|---|---|---|---|---|
| 1 | NS_Environment_CampfireSmoke instâncias em 0-50m | 166 | Scalability LOD/distância no asset Niagara (via #17) | Redução relevante de fill-rate GPU, maior ganho em consola | Baixo (scalability) / Médio (se fundir actores) |
| 2 | pior_frame_ms num sample de log | 125.0ms (pico) | Capturar `stat unit`/`stat gpu` no próximo pico (via #18/#19) | Diagnóstico, custo zero | Nenhum |
| 3 | AmbientSound >400m do spawn | 2 | Verificar attenuation/falloff (via áudio) | Marginal | Nenhum |
| 4 | PointLight cast_shadows | 0/795 | Nenhuma acção — já optimizado | — | — |
| 5 | actor_tick_enabled | 0/3501 | Nenhuma acção — já optimizado | — | — |

## Números que NÃO inventei
- Não reportei fps "de jogo real" porque não há sessão PIE activa nesta medição — reportei o fps medido em modo editor ocioso (97.3 fps por amostragem directa) e as três últimas FPS_SAMPLE reais do log (120.0, 120.0, 118.3), identificando o outlier em vez de o esconder.

## Dependências / próximos passos
- **#17 (VFX Agent)**: aplicar scalability ao `NS_Environment_CampfireSmoke` (166 instâncias concentradas perto do spawn) — maior alavanca de performance visual identificada este ciclo. Respeitar tecto A05 (986 decorativos, já em 950) — não acrescentar actores novos, apenas ajustar o asset.
- **#18/#19 (QA/Integration)**: quando houver sessão PIE real com jogador a jogar, capturar `stat unit`/`stat gpu` num pico de frame-time para diagnosticar a causa do hitch de 125ms observado no log.
- **#5 (World Generator)**: nenhuma acção de física pendente — R14 confirmado limpo pelo #3 este ciclo (0 anomalias em 43 criaturas `Dino_*`).
