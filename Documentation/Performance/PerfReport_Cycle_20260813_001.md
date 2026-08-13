# Performance Report — Ciclo PROD_CYCLE_AUTO_20260813_001
Agente #4 — Performance Optimizer

## Metodologia
Medição directa no editor UE5 headless via `ue5_execute` (4 chamadas Python reais, sem `time.sleep`, sem PIE aberto). Nenhuma optimização foi aplicada ao mundo — este agente mede e propõe; a decisão de aplicar fica com o agente relevante (#5, #17, #16) ou com o Hugo, conforme a SPECIFIC DIRECTIVE deste agente.

## Estado do mundo (verificado nesta sessão)

| Métrica | Valor | Como foi medido |
|---|---|---|
| Total de actores | 3468 | `EditorActorSubsystem.get_all_level_actors()` |
| Cap do servidor | 8000 | conhecido (config), headroom real = 4532 |
| StaticMeshActor | 822 | contagem por classe |
| PointLight | 795 | contagem por classe |
| Pawn | 512 | contagem por classe |
| AmbientSound | 251 | contagem por classe |
| NiagaraActor | 235 | contagem por classe |
| TriggerBox | 123 | contagem por classe |
| TargetPoint | 123 | contagem por classe |
| SkeletalMeshActor | 88 | contagem por classe |
| TriggerSphere | 35 | contagem por classe |
| Emitter (Cascade, legacy) | 15 | contagem por classe |
| actor_tick_enabled (amostra 250 VFX) | 0 | `get_editor_property('actor_tick_enabled')` — confirma que per-actor tick NÃO é bottleneck (já reportado em ciclos anteriores, reconfirmado) |
| PointLight com `cast_shadows=True` | 0 de 795 | leitura do `PointLightComponent` — já optimizado, sem sombras dinâmicas caras |
| PointLight `MOVABLE` | 18 de 795 | leitura de `mobility` no componente — 777 são Static/Stationary, bom para baked lighting |
| TriggerBox com `generate_overlap_events=True` | 50 de 50 (amostra) | leitura do `BoxComponent` |
| NiagaraActor dentro de 3000uu do hub (1200,1200,301) | **220 de 235 (93.6%)** | distância euclidiana calculada a partir de `get_actor_location()` |
| AmbientSound dentro de 3000uu do hub | **205 de 251 (81.7%)** | idem |
| Prefixos de label Niagara | FX_=152, VFX_=79, VFXAnchor_=2, Campfire_=1, NS_=1 | `get_actor_label()` split |
| Último FPS_SAMPLE no log | `fps=71.8, frames=575, wall=8.0s, pie=False, pior_frame_ms=21.7, timestamp 2026.08.12-20.22.19` | grep ao log do editor (`Saved/Logs`) — **NOTA: esta amostra tem ~15h e não é fresca; não foi gerada nesta sessão porque o PIE está fechado (correcto, per directiva) e não há amostrador activo a correr fora de PIE.** |

## Achados principais

1. **Per-actor tick não é o problema.** Confirmado outra vez (0 de 250 amostrados com `actor_tick_enabled=True`). Qualquer proposta de "desligar ticks" não tem efeito — já estão desligados.

2. **Concentração extrema de VFX junto ao spawn.** 220 dos 235 NiagaraActor (93.6%) estão a menos de 30 m do PlayerStart. É exactamente a zona onde o custo de render (overdraw de partículas, sort de transparência) mais penaliza o frame, porque é onde a câmara do jogador passa mais tempo. 15 Emitters legacy (Cascade) ainda existem — Cascade é single-threaded e mais caro por partícula que Niagara equivalente.

3. **251 AmbientSound, 205 junto ao hub.** O motor de áudio tem um limite de vozes concorrentes (concurrency); com esta densidade há risco real de "voice stealing"/virtualização audível quando o jogador está no hub, mesmo sem impacto directo em fps.

4. **Iluminação já está numa boa base.** 0 PointLights com sombras dinâmicas, 777 Static/Stationary — não há aqui trabalho de optimização a propor este ciclo; está correcto tal como está.

5. **Headroom de actores é real:** 3468/8000 (43%), portanto adicionar dinossauros/conteúdo interactivo (prioridade do portão A02/A03) não compete por orçamento de contagem de actor — compete por orçamento de **overdraw e vozes de áudio/partícula**, que são os recursos apertados aqui.

## Propostas ranqueadas (nenhuma aplicada — para decisão do agente relevante ou Hugo)

| # | Proposta | Custo esperado | Risco | Para quem |
|---|---|---|---|---|
| 1 | Aplicar **distance-culling / LOD de partículas a 3 níveis** aos 152 `FX_*` Niagara — reduzir spawn rate/densidade acima de ~15m da câmara, desligar completamente acima de ~40m. Não apagar actores (mantém contagem e refs de gameplay). | Baixo (config de LOD no Niagara System, não no actor) | Baixo — reversível, não move nem apaga `Dino_*` nem decoração alvo do A05 | #17 VFX Agent |
| 2 | Migrar os **15 Emitter (Cascade) legacy** para Niagara equivalente, ou pelo menos aplicar `bAutoActivate=false` + activação por proximidade se ainda visíveis a longa distância. Cascade é mais caro por partícula. | Médio (requer reconstrução de sistema, fora do escopo deste agente — C++/asset editing) | Médio — precisa de verificação visual antes/depois | #17 VFX Agent |
| 3 | **Attenuation/priority tuning** nos 251 `AmbientSound` — configurar `max_concurrent_play_count` e `priority` para os 205 junto ao hub, para evitar virtualização audível quando o jogador está na zona mais densa. | Baixo (property tuning, sem novos actores) | Baixo | #16 Audio Agent |
| 4 | **Não tocar** na iluminação (PointLight) — já está no estado óptimo medido (0 sombras dinâmicas, maioria Static). Nenhuma acção recomendada. | — | — | #8 Lighting (informativo, sem tarefa) |
| 5 | Ao adicionar conteúdo interactivo (dinos/recursos, prioridade A02/A03 do portão), **usar o headroom de contagem de actor livremente** (3468/8000) — o gargalo real não é contagem de actor, é overdraw de VFX + voice count de áudio na mesma zona. Compor conteúdo novo fora do raio denso de FX/Ambient junto ao hub, ou aceitar que competirá por overdraw/vozes com o que já lá está. | — | — | #5 World Generator, próximo agente da cadeia |

## Limitação reportada
FPS não pôde ser medido de forma fresca nesta sessão porque o PIE está fechado (correcto, conforme regra do projecto — só o #22 abre PIE). A única amostra disponível no log tem ~15h. Recomenda-se que o próximo ciclo com PIE aberto (via #22) capture uma amostra fresca de `FPS_SAMPLE` para validar o impacto real das propostas acima.

## Ficheiros modificados
- `Documentation/Performance/PerfReport_Cycle_20260813_001.md` (este ficheiro, novo)

Nenhum `.cpp`/`.h` foi criado ou editado (inertes neste editor headless, per regra absoluta do projecto). Nenhum actor `Dino_*` foi movido, rodado ou apagado (V1-C25 respeitado: 0 movidos). Nenhuma decoração nova foi criada (tecto A05 respeitado).
