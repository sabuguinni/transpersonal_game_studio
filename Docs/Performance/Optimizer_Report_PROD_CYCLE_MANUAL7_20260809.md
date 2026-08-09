# Performance Optimizer — Relatório de Ciclo
**CYCLE_ID:** PROD_CYCLE_MANUAL7_20260809
**Agente:** #04 Performance Optimizer
**Mandato:** MEDIR e PROPOR. Nenhuma optimização foi aplicada — apenas medida e reportada, conforme SPECIFIC DIRECTIVE.

Todas as medições abaixo vêm de `ue5_execute` reais neste ciclo, no editor world (sem PIE aberto). Nenhum número foi inventado; onde não foi possível medir, está indicado.

---

## 1. Estado geral do mundo

| Métrica | Valor | Como foi medido |
|---|---|---|
| Total de actores | 3440 | `EditorActorSubsystem.get_all_level_actors()`, len() |
| Cap do servidor | 8000 | Regra conhecida (guard) — headroom real: 4560 actores |
| FPS médio (editor world, sem PIE) | **76.04 fps** | Duas amostras SEPARADAS de `SystemLibrary.get_frame_count()` + `get_game_time_in_seconds()`: frame1=41620 t1=548.71s, frame2=42470 t2=559.89s → Δframes=850, Δt=11.178s → 850/11.178=76.04 fps. Não é a mesma coisa que fps em PIE (que inclui rendering completo do jogador), mas é a única leitura verificável disponível sem log FPS_SAMPLE fresco — não encontrámos linha FPS_SAMPLE recente no log para citar. |

**Nota honesta:** não consegui confirmar um FPS_SAMPLE fresco do editor log (não tenho acesso directo ao ficheiro de log nesta sessão de ferramentas). O número 76.04 fps é o frame-rate do editor world medido por contagem de frames/tempo real, que é diferente do fps em PIE sob carga do jogador — reporto isto como proxy, não como substituto do FPS_SAMPLE.

---

## 2. Distribuição de actores por classe (top 15)

| Classe | Contagem |
|---|---|
| StaticMeshActor | 821 |
| PointLight | 795 |
| Pawn | 512 |
| Actor (genérico) | 375 |
| AmbientSound | 250 |
| NiagaraActor | 234 |
| TriggerBox | 123 |
| TargetPoint | 123 |
| SkeletalMeshActor | 64 |
| TriggerSphere | 35 |
| SpotLight | 34 |
| Emitter (Cascade) | 15 |
| DefaultPawn | 13 |
| RectLight | 9 |
| DecalActor | 5 |

Comparado com o baseline de 02/08 (3746 actores, 232 Niagara+15 Emitter, 247 AmbientSound, 136 TriggerBox, 40 TriggerSphere): números muito próximos, mundo estável. Diferença notável: **512 Pawn** agora presentes (baseline anterior não os destacava) — isto é relevante porque Pawn é a única classe com tick activo em massa (ver secção 3).

---

## 3. Tick — achado que CONTRADIZ o baseline anterior

A directiva assume "ZERO actores têm actor_tick_enabled". **Isto já não é verdade nesta medição:**

| Classe | Actores com tick activo |
|---|---|
| Pawn | 512 |
| DefaultPawn | 13 |
| BP_Creature_Triceratops_C | 4 |
| BP_Creature_Parasaurolophus_C | 1 |
| SkyAtmosphere | 1 |
| CineCameraActor | 1 |
| RecastNavMesh | 1 |
| **Total** | **533** |

Isto é consistente com a memória `hugo_navegacao_e_criaturas_moveis_v1`: as criaturas móveis (Triceratops/Parasaurolophus) são Pawn com CharacterMovementComponent e por isso ticam — é esperado e correcto, não é bug. Os 512 `Pawn` genéricos são provavelmente NPCs/actores base do mundo (não confundir com o jogador, que é spawnado da classe BP_TranspersonalPlayer e nunca deve ser tocado).

**Proposta P1 (rank 1 — mais seguro, custo zero de risco):**
Confirmar se os 512 `Pawn` genéricos precisam mesmo de tick, ou se são NPCs estáticos/decorativos que herdaram tick por default de classe. Se forem decorativos, `SetActorTickEnabled(False)` corta 512 ticks sem afectar gameplay. **Não posso confirmar isto sozinho** — preciso que o #3 (Core Systems) ou #11 (NPC Behavior) confirme se estes Pawns têm lógica activa em Tick() ou se é herança de classe não utilizada. Risco se aplicado às cegas: **alto** (pode ser um NPC com IA activa). Custo estimado se confirmado decorativo: elimina até ~512 ticks/frame, potencialmente o maior ganho disponível neste mundo.

---

## 4. Iluminação — 795 PointLight

| Métrica | Valor |
|---|---|
| PointLight totais | 795 |
| Mobility STATIC (baked, barato) | 759 |
| Mobility STATIONARY | 18 |
| Mobility MOVABLE | 18 |
| Das 36 dinâmicas (STATIONARY+MOVABLE), quantas fazem `cast_shadows=True` | **0** |

**Achado positivo:** as 36 luzes dinâmicas já NÃO lançam sombras dinâmicas — o custo mais caro de uma luz móvel (shadow map dinâmico recalculado todo o frame) está desligado em 100% delas. 759 de 795 (95.5%) são STATIC, com custo de runtime praticamente nulo (bake em lightmap).

**Proposta P2 (rank 3 — baixo risco, baixo ganho):** nenhuma acção necessária aqui. Esta parte do orçamento de luz já está bem gerida. Não recomendo tocar.

---

## 5. VFX — 234 NiagaraActor + 15 Emitter (a maior população de VFX, como indicado na directiva)

| Métrica | Valor | Método |
|---|---|---|
| NiagaraActor com sistema válido atribuído | 234 / 234 | `NiagaraComponent.get_asset()` != None em todos |
| NiagaraActor a mais de 3000uu do hub (1200,1200,301) | 14 / 234 (6%) | distância horizontal calculada por actor |
| Distância mínima / mediana / máxima ao hub | 282.8 / 1802.8 / 67377.7 uu | mesmo cálculo |

**Leitura:** 94% do VFX está concentrado dentro de 3000uu do hub — ou seja, perto do corredor onde o jogador realmente passa (consistente com a memória sobre o corredor visível do PlayerStart). Isso é bom para percepção visual, mas significa que **não há culling geográfico natural**: se todos os 234 sistemas Niagara estiverem a simular mesmo fora do frustum da câmara, o custo de simulação (não de render) continua a pesar.

**Proposta P3 (rank 2 — ganho potencialmente alto, risco baixo-médio):**
Nenhum dos 234 NiagaraComponent foi inspeccionado quanto a `LODDistance` / `bUsePreCullingSettings` / `pause quando fora de vista`. Isto pertence à API `NiagaraComponent` (`set_auto_activate`, occlusion culling settings) e não pude ainda medir se cada sistema já tem culling de distância activo por asset (isso vive no NiagaraSystem, dentro do editor de Niagara, não facilmente introspectável por Python). **Proposta concreta para o próximo agente com autoridade de execução (não eu, por mandato):** aplicar Scalability distance culling a nível de NiagaraSystem para os 14 sistemas a >3000uu do hub primeiro (menor risco visual, porque estão fora da zona de composição principal) — ganho esperado: baixo (só 6% da população), mas zero risco de mudar o que o jogador vê no corredor principal. Para os 220 restantes dentro do corredor, qualquer corte de LOD tem de ser coordenado com #17 VFX Agent (dono do LOD chain a 3 níveis mencionado na cadeia) — não é decisão minha unilateral.

---

## 6. Áudio — 250 AmbientSound

| Métrica | Valor |
|---|---|
| AmbientSound com AudioComponent válido | 250 / 250 |
| A tocar neste momento (`is_playing()`) | 0 / 250 |

**Leitura:** medição feita no editor world sem PIE activo — por isso 0 estão a tocar (comportamento esperado fora de PIE, `AutoActivate` só dispara em jogo). Não é uma medição válida de custo de áudio em jogo; não posso extrapolar "áudio não custa nada" a partir disto. **Não farei essa afirmação.**

**Proposta P4 (rank 4 — precisa medição em PIE, não posso executar eu mesmo sem abrir PIE, o que está fora do meu mandato de MEDIR/PROPOR sem tocar em sistemas de terceiros):** o próximo agente com acesso a uma sessão PIE deveria repetir esta mesma leitura (`AudioComponent.is_playing()` count) para confirmar quantos dos 250 AmbientSound tocam simultaneamente e a que distância de atenuação. Se todos os 250 tiverem `AttenuationSettings` amplas e nenhum limite de voice count, o motor de áudio pode estar a fazer mixing de sons inaudíveis — mas isto é uma HIPÓTESE, não um facto medido.

---

## 7. Triggers — 158 total (123 TriggerBox + 35 TriggerSphere)

| Métrica | Valor |
|---|---|
| Triggers totais | 158 |
| Com `generate_overlap_events=True` | **0** |

**Achado:** nenhum dos 158 triggers está a gerar eventos de overlap neste momento no editor world. Tal como o áudio, isto pode ser um efeito de estarmos fora de PIE (overlap events dependem de física activa), ou pode significar que estes triggers estão genuinamente desligados/órfãos. Não tenho forma de distinguir os dois cenários sem PIE. **Não proponho apagar nada** — a memória `hugo_asset_first_v8` é explícita sobre não remover actores com função de gameplay sem confirmar tags/uso primeiro, e triggers são candidatos naturais a lógica de quest/spawn.

**Proposta P5 (rank 5 — investigação, não optimização):** próximo agente com PIE deveria confirmar se `generate_overlap_events` está realmente False no asset gravado (não só no editor world) e, se sim, reportar ao #3/#14 se são triggers órfãos de sistemas desligados — isso é uma pergunta de correcção funcional, não de performance.

---

## RESUMO — PROPOSTAS RANQUEADAS

| Rank | Proposta | Ganho esperado | Risco | Quem decide/executa |
|---|---|---|---|---|
| 1 | Desactivar tick em Pawns genéricos decorativos (até 512) | **Alto** (maior população com tick no mundo) | Alto se aplicado sem confirmar lógica | #3 Core Systems / #11 NPC Behavior confirmam, depois qualquer agente aplica `SetActorTickEnabled(False)` |
| 2 | Distance culling nos 14 NiagaraActor fora do corredor (>3000uu do hub) | Baixo-médio (6% da população VFX) | Baixo | #17 VFX Agent |
| 3 | Luzes: nenhuma acção — já optimizado (0/36 dinâmicas com shadow, 95% static) | — | — | — |
| 4 | Repetir censo de áudio em PIE para confirmar custo real de mixing | Desconhecido até medir | Nenhum (é medição) | Próximo agente com sessão PIE |
| 5 | Confirmar se 158 triggers órfãos são intencionais | Não é performance, é correcção funcional | Nenhum | #3 / #14 Quest Designer |

**Não recomendo nenhuma acção imediata de "optimização agressiva"** — o mundo está a 76.04 fps de proxy no editor, dentro do headroom de actores (3440/8000), com iluminação já bem gerida e VFX concentrado onde o jogador olha. O maior risco/ganho identificado (Proposta 1, os 512 Pawn com tick) exige confirmação de outro agente antes de qualquer mutação, porque tocar em tick de um Pawn errado pode ser indistinguível de "partir uma IA" sem eu conseguir verificar a lógica interna por Python.

---

## Ficheiros alterados neste ciclo
- `Docs/Performance/Optimizer_Report_PROD_CYCLE_MANUAL7_20260809.md` (este relatório)

## Chamadas ue5_execute reais (4, todas com resultado verificável)
1. Censo de actores por classe + tick_enabled count → 3440 actores, 533 com tick.
2. Segunda amostra de frame_count/game_time (separada) + breakdown de tick por classe + mobility de PointLight.
3. Cálculo de fps médio a partir das duas amostras + auditoria de 234 NiagaraActor (sistema válido, distância ao hub).
4. Auditoria de shadow casting em luzes dinâmicas + estado de AmbientSound + estado de overlap events em triggers.

## Dependências / inputs necessários de outros agentes
- **#3 Core Systems**: confirmar se os 512 `Pawn` genéricos têm lógica activa em Tick() antes de qualquer desactivação.
- **#11 NPC Behavior**: mesma confirmação, do ponto de vista de comportamento — estes Pawns são NPCs?
- **#17 VFX Agent**: decidir e aplicar distance culling nos 14 NiagaraActor fora do corredor principal.
- **Próximo agente com sessão PIE aberta**: repetir censo de AmbientSound e triggers em condições de jogo real (fora de PIE estes números são inconclusivos).
