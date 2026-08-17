# Performance Optimizer — Relatório de Ciclo PROD_CYCLE_AUTO_20260817_001

**Agente #4 — papel deste ciclo: MEDIR e PROPOR, nunca executar optimizações.**
Todas as medições abaixo vêm de chamadas `ue5_execute` reais neste ciclo (4 chamadas, todas com resultado verificável). Nenhum actor foi movido, apagado, criado ou reescalado.

---

## 1. Estado geral do mundo (censo)

| Métrica | Valor | Como foi medido |
|---|---|---|
| Total de actores | 3489 | `EditorActorSubsystem.get_all_level_actors()`, len() |
| PIE aberto no início | False | `LevelEditorSubsystem.is_in_play_in_editor()` |
| StaticMeshActor | 842 | contagem por classe |
| PointLight | 795 | contagem por classe |
| Pawn | 512 | contagem por classe |
| Actor (base) | 375 | contagem por classe |
| AmbientSound | 251 | contagem por classe |
| NiagaraActor | 236 | contagem por classe |
| TriggerBox | 123 | contagem por classe |
| TargetPoint | 123 | contagem por classe |
| SkeletalMeshActor | 88 | contagem por classe |
| TriggerSphere | 35 | contagem por classe |
| SpotLight | 34 | contagem por classe |
| Emitter (Cascade) | 15 | contagem por classe |
| BP_Creature_Triceratops_C | 4 | contagem por classe |

Comparado ao censo de 02/08/2026 (memória): 3746→3489 actores totais, Niagara 232→236, Emitter 15→15, AmbientSound 247→251, TriggerBox 136→123, TriggerSphere 40→35. Números estáveis, sem explosão de população desde o último censo.

---

## 2. FPS — medido, não inventado

**Não consegui amostrar um delta próprio nesta chamada** (frame_count e real_time só dão um snapshot instantâneo: `frame_count=25016604`, `real_time=238351.83s`; medir FPS exigiria uma segunda invocação separada no futuro, sem `time.sleep()`, conforme a regra).

Em vez disso, li as últimas 5 linhas `FPS_SAMPLE` reais do log do editor (produzidas pelo instrumento R24r):

```
fps=120.0 pior_frame_ms=9.1  (frames=960 wall=8.0s, pie=False)
fps=119.9 pior_frame_ms=12.2
fps=120.0 pior_frame_ms=9.9
fps=120.0 pior_frame_ms=9.3
fps=120.0 pior_frame_ms=9.1
```

**Nota importante**: estas amostras são todas com `pie=False` e `n_espectadores=0` — ou seja, medidas no editor sem PIE activo e sem viewport de jogo a renderizar a cena completa (câmara em -99999,-99999,-99999). Isto é FPS do editor ocioso, não do jogo em execução. O floor de 79.7 citado na directiva refere-se a uma amostra em PIE real que não está disponível nesta sessão (PIE fechado, como deve estar para este agente). **Não tenho uma amostra fresca de FPS em jogo real este ciclo — reporto isto como limitação, não invento o número.**

---

## 3. VFX — a maior população, tal como apontado na directiva

### Niagara (236 actores)
Sistemas usados:
| Sistema | Contagem |
|---|---|
| NS_Environment_CampfireSmoke | 171 |
| NS_Dino_Footstep | 32 |
| NS_Environment_AmbientDust | 15 |
| NS_Environmental_Wind | 11 |
| NS_Atmospheric_DustMotes | 3 |
| NS_Dust_Footstep_TRex | 3 |
| NS_Fire_Campfire | 1 |

Distribuição por distância ao PlayerStart (1200,1200,301), 2D:
- ≤50 m: 224 actores Niagara
- 50–200 m: 9
- >200 m: 3 (todos visíveis, não escondidos — `hidden=False`)

**`actor_tick_enabled` = 0 em TODOS os 236 Niagara** (confirmado por varrimento completo, não amostra). Consistente com a directiva: tick por actor já está desligado, não é o gargalo.

### Emitter / Cascade (15 actores)
Templates: `P_Steam_Lit_MOD` (6), `P_AmbientDust` (3), `None` (6 — sem template atribuído, ou seja **6 Emitter actors não renderizam nada e só ocupam slot de actor**).

### AmbientSound (251 actores)
232 estão a ≤50 m do spawn, apenas 2 a >200 m.

---

## 4. Luzes — achado principal deste ciclo

| Tipo | Total | Mobility | cast_shadows=True | ≤50m do spawn | shadow_on ≤50m |
|---|---|---|---|---|---|
| PointLight | 795 | 759 STATIC / 18 MOVABLE / 18 STATIONARY | **0** | 670 | 0 |
| SpotLight | 34 | 31 STATIC / 3 STATIONARY | 2 | 30 | 2 |
| RectLight | 9 | 9 STATIC | 1 | 8 | 0 |

**Achado**: praticamente nenhuma luz projecta sombra dinâmica (3 em 838 luzes no total). 759 dos 795 PointLight são STATIC — em Lumen isso significa lightmap bakeado, custo de runtime baixo por luz individual, mas **670 PointLight concentradas em 50 m do spawn** é uma densidade de luzes muito alta para uma única zona (a "corridor" visível do PlayerStart). Cada luz STATIC ainda participa no culling e nas estruturas de aceleração de Lumen/GI mesmo sem sombra dinâmica.

---

## 5. Propostas ranqueadas (para decisão de Hugo / próximo agente que execute)

### P1 — Fundir os 171 NS_Environment_CampfireSmoke em menos emissores com maior raio de spawn (ALTO impacto, BAIXO risco)
- **Problema**: 171 instâncias do mesmo sistema de fumo de fogueira, concentradas junto ao spawn (224 dos 236 Niagara totais estão a ≤50 m).
- **Proposta**: para grupos de campfires próximos (ex.: <500 uu entre si), substituir N sistemas Niagara individuais por 1 sistema com múltiplos emissores/spawn points (GPU particles instanciadas), reduzindo o número de `NiagaraComponent` activos sem reduzir densidade visual de fumo.
- **Custo esperado de implementação**: médio (requer editar o Niagara System, não script Python simples — provavelmente tarefa do #17 VFX Agent).
- **Risco**: baixo — fumo é ambiente, não gameplay; fundir não muda leitura visual se bem posicionado.
- **Quem deve executar**: #17 (VFX Agent), com este agente a validar o custo antes/depois.

### P2 — Remover ou atribuir template aos 6 `Emitter` (Cascade) sem template (`template=None`) (BAIXO impacto, BAIXO risco, TRIVIAL)
- 6 actores Emitter ocupam slot de actor e componente sem renderizar nada. Não violam o tecto A05 de decorativos (contam à parte), mas são desperdício puro — zero benefício visual por custo de actor não-zero (culling, transform, etc).
- **Proposta**: o próximo agente com mandato de mover/editar (não eu, per REGRA ANTI-ALUCINAÇÃO e limites deste papel) deve ou atribuir-lhes um `ParticleSystem` template válido ou reportar como lixo técnico ao QA (#18).
- **Risco**: nulo — não têm efeito visual actualmente.

### P3 — Não tocar nas luzes STATIC (759 PointLight), mas auditar as 670 concentradas em ≤50 m antes de qualquer agente de Lighting adicionar mais (MÉDIO impacto preventivo, BAIXO risco)
- 795 PointLight já é uma população grande; 670 delas na zona de showcase são luzes bakeadas (STATIC), com custo de runtime relativamente baixo, mas cada uma consome memória de lightmap e tempo de build de iluminação.
- **Proposta**: se o #8 (Lighting Agent) planeia adicionar mais luzes dinâmicas nesta zona (como o exemplo dado na directiva — "500 luzes dinâmicas"), a abordagem correcta para não rebentar o frame budget é: manter luzes ambiente como STATIC/bakeadas (já é o padrão actual, 759/795), e reservar MOVABLE/STATIONARY apenas para pontos focais (já há 18+18=36 nessa categoria) — ou seja, o padrão actual já é a arquitectura recomendada. **Não fazer downgrade do que já existe.**
- Este item é confirmação de boa prática existente, não uma acção nova.

### P4 — AmbientSound: 232 de 251 concentrados em ≤50 m (INFO, sem acção recomendada)
- Áudio 3D tem custo de CPU baixo por fonte comparado a VFX/luzes, mas 232 fontes activas simultâneas na mesma zona pode saturar o mixer de áudio (polifonia). Recomendo ao #16 (Audio Agent) verificar se há culling por distância/prioridade configurado nas MetaSounds, não uma redução de contagem.

---

## 6. O que NÃO fiz (e porquê, conforme os limites do papel)
- Não movi, apaguei, criei ou reescalei nenhum actor.
- Não toquei em nenhum `Dino_*`.
- Não abri PIE (estava fechado, confirmado, e mantive-o fechado — não é meu mandato abri-lo).
- Não criei/editei .cpp/.h (proibido; C++ é inerte neste editor headless).
- Não inventei um número de FPS em jogo — reportei a limitação da amostra disponível (editor ocioso, pie=False) em vez de fabricar um valor plausível.

---

## Ficheiros produzidos
- `Docs/Performance/PROD_CYCLE_AUTO_20260817_001_optimizer_report.md` (este ficheiro)

## Dependências / inputs necessários de outros agentes
- **#17 (VFX)**: decidir e executar a fusão dos 171 NS_Environment_CampfireSmoke (P1) e resolver os 6 Emitter sem template (P2).
- **#8 (Lighting)**: manter o padrão STATIC-first ao adicionar luzes novas na zona de showcase (P3); não converter em massa PointLight existentes para MOVABLE.
- **#16 (Audio)**: validar polifonia/culling dos 232 AmbientSound próximos do spawn (P4).
- **Próxima medição de FPS real**: precisa de uma sessão com PIE aberto e espectador activo (mandato do #22) para produzir uma amostra `FPS_SAMPLE` válida com `pie=True` e câmara real, que este agente possa depois ler e comparar ao floor de 79.7.
