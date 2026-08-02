# Performance Optimizer #04 — Audit real, PROD_CYCLE_MANUAL2_20260802

## Papel neste ciclo
Directiva específica do agente: **MEDIR e PROPOR, nunca executar optimizações**. Este relatório contém apenas números medidos via `ue5_execute` (3 chamadas reais) e uma lista de propostas ranqueadas. Nenhuma optimização foi aplicada ao mundo.

---

## 1. Métricas medidas (com método)

| Métrica | Valor | Como foi medido |
|---|---|---|
| Total actors no editor world | **3195** | `EditorLevelLibrary.get_all_level_actors()` len() — nota: a baseline do bloco de contexto media 3746 há alguns dias; o mundo mudou entretanto (esperado, outros agentes trabalharam). Reportado o valor fresco, não o antigo. |
| NiagaraActor | **232** | Contagem por classe sobre todos os actors |
| Emitter (Cascade legado) | **15** | Idem |
| AmbientSound | **247** | Idem |
| TriggerBox | **136** | Idem |
| TriggerSphere | **40** | Idem |
| Actors com `actor_tick_enabled=True` | **0** | Confirma a memória: tick por-actor NÃO é o gargalo. Não proponho desligar ticks — já estão desligados. |
| NiagaraComponents totais | **232** (1 por NiagaraActor) | `get_components_by_class(NiagaraComponent)` |
| NiagaraComponents ACTIVOS agora | **0 / 232** | `.is_active()` — o editor está fora de PIE, por isso todos os sistemas Niagara estão dormentes neste instante. Isto é uma medição do estado actual, **não** prova que ficam a 0 em PIE (o BeginPlay tipicamente activa-os). |
| ParticleSystemComponents (Cascade) ACTIVOS agora | **9 / 15** | Idem, mas Cascade tende a auto-activar em preview do editor — os Emitters legados já estão a simular fora de PIE, ao contrário do Niagara. |
| FPS real (amostras do log, sessões PIE anteriores) | **79.9 / 80.4 / 80.7 fps**, pior frame 19.3–21.2 ms | Grep de linhas `FPS_SAMPLE` no `TranspersonalGame.log` (61420 linhas escaneadas, 28 matches). As 3 últimas amostras reais são de 28-29/07; **não existe amostra fresca de hoje (02/08)** porque não há sessão PIE activa agora — reportado como tal, não inventado. |

### Nota crítica sobre a FPS medida
A média (79.9–80.7 fps) está confortavelmente acima do alvo de 60fps PC. Mas o **pior frame** em cada amostra (19.3–21.2 ms) corresponde a ~47–52 fps instantâneos — um spike, não a média. Isto é exactamente a distinção que Fabian faria: a média esconde o stutter. **O alvo de optimização real não é a média, é o 1% low / pior frame.**

---

## 2. Propostas ranqueadas (nenhuma aplicada — decisão de outro agente/Hugo)

### #1 — Investigar a origem do pior-frame de ~20ms (spike), não a média
**Custo esperado:** 0 (é só instrumentação: `stat unit`, `stat game`, `stat niagara` durante uma sessão PIE de 30s+, correlacionar spikes com eventos como spawn de partículas ou trigger overlaps).
**Risco:** nenhum — é medição pura.
**Porquê primeiro:** a média já cumpre o alvo; o spike é o único sintoma real de "stutter" reportado nos dados, e não sabemos ainda a causa. Aplicar qualquer optimização antes de saber a causa é cortar código às cegas.

### #2 — Distance culling / max instance count nos 232 NiagaraActor
**Custo esperado:** praticamente nulo em CPU/GPU quando culled (a feature de scalability do Niagara já suporta isto nativamente via System Overview > Scalability > Distance Culling e Effect Type).
**Risco:** médio — pode remover efeitos ambientais que o jogador devia ver de perto; exige QA visual do #17/#18 depois de aplicado.
**Porquê:** é a maior população de actors "especiais" no mundo (232), a directiva já a assinala como o primeiro sítio a olhar. Mas a medição mostra 0 activos fora de PIE — o impacto real só se confirma com uma sessão PIE instrumentada (ver proposta #1).

### #3 — Consolidar/gerir os 15 Emitter (Cascade) legados
**Custo esperado:** Cascade é simulado em CPU (mais caro por-partícula que Niagara GPU-sim); 9/15 já estão activos mesmo fora de PIE, o que sugere que em PIE os 15 estarão todos a simular continuamente.
**Risco:** baixo-médio — migração para Niagara é trabalho do #17 VFX Agent, não meu; aqui só sinalizo o custo e o candidato.
**Porquê:** é a única categoria onde a "actividade fora de PIE" já é mensurável e positiva (60% activos), ao contrário do Niagara (0%).

### #4 — Auditoria de concorrência de som (247 AmbientSound)
**Custo esperado:** desconhecido sem profiling de áudio (`stat sounds`); tipicamente barato por-voz mas 247 fontes simultâneas sem Sound Concurrency/attenuation por distância pode saturar o mixer.
**Risco:** baixo — é trabalho de configuração, não de remoção de conteúdo. Pertence ao #16 Audio Agent; aqui só forneço o número e a bandeira.

### #5 — TriggerBox/TriggerSphere (176 no total)
**Custo esperado:** baixo, salvo se muitos overlaparem simultaneamente perto do hub (custo de broadphase de colisão). Sem uma sessão PIE com `stat collision` não há número real para decidir.
**Risco:** nenhum na fase de medição.

---

## 3. O que NÃO foi feito (e porquê)
- **Nenhuma optimização foi aplicada** — é a directiva específica deste agente neste ciclo (medir e propor, não executar).
- **Nenhum `.cpp`/`.h` foi escrito** — regra absoluta MAX, C++ é inerte neste editor headless.
- **Nenhum actor `Dino_*` foi tocado**, Landscape/sublevel/pawn intocados, câmara do viewport intocada — todas as proibições da directiva respeitadas.
- Não inventei nenhuma amostra de FPS de hoje: o valor mais recente real (79.9–80.7fps) é de 28-29/07; reportei essa data explicitamente em vez de fingir frescura.

## 4. Handoff
Para o **#5 Procedural World Generator**: a população de actors (3195, headroom até ao tecto de 8000 ainda real mas finito) e as 5 propostas acima ficam disponíveis para qualquer agente que decida gerar mais conteúdo — gerar terreno/biomas adicionais deve ter em conta que o maior custo latente identificado é VFX (232 Niagara + 15 Cascade), não geometria.
Para o **#17 VFX Agent** (quando chegar a vez dele na cadeia): propostas #2 e #3 são directamente accionáveis por ele.
Para o **#16 Audio Agent**: proposta #4.
