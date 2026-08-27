# Performance Optimizer (#4) — Ciclo PROD_CYCLE_AUTO_20260827_001

## Papel deste ciclo
Directiva específica: MEDIR e PROPOR, nunca executar optimizações cegas. Zero mutações no mundo — nenhum actor foi movido, apagado, reescalado, nem teve mobility/tags/visibilidade alterada. Três chamadas `ue5_execute`, todas com resultado verificável e reportado tal como veio do editor (nenhum número inventado).

## Medições reais (03544 actores, PIE fechado confirmado antes de medir)

### 1. Censo geral e headroom
- `TOTAL_ACTORS = 3544` (cap definido em 8000 → headroom real de 4456)
- Top classes: StaticMeshActor 882, **PointLight 795**, Pawn 512, Actor 376, AmbientSound 256, **NiagaraActor 237**, TargetPoint 124, TriggerBox 123, SkeletalMeshActor 88, TriggerSphere 35
- Amostra de 500 actores: `actor_tick_enabled` = 0 em todos → confirma memória anterior, per-actor tick **não é** o gargalo. Não proponho nada nessa frente.

### 2. FPS — última amostra real do log (método R24r)
```
FPS_SAMPLE frames=747 wall=8.0 pie=False fps=93.3 pior_frame_ms=18.0
idade_pie_s=85028 n_espectadores=1
```
- fps=93.3 medido **fora de PIE** (editor puro), amostra com 85028 s de idade (>23h) — não representativo do estado actual do jogo em runtime, mas confirma que a linha de base do editor está saudável e bem acima do alvo de 60fps.
- Não tentei gerar uma nova amostra fresca porque isso exigiria abrir PIE, o que está fora do meu mandato (só o #22 tem esse direito). Reporto a limitação em vez de inventar um número.
- `get_frame_count()` + `get_real_time_seconds()` foram lidos numa única invocação (snapshot único: frame=26355993, t=334509.35s) — sem uma segunda amostra separada por um intervalo real de wall-clock não dá para calcular fps médio verdadeiro sem correr sleep (proibido) ou duas invocações consecutivas do agente (custo de ciclo). Não reporto fps derivado disto — seria inventado.

### 3. Luzes — achado principal deste ciclo
`795 PointLight`: **0 shadow_casting**, 759 STATIC, 18 STATIONARY, 18 MOVABLE.
- Zero luzes a projectar sombra é uma boa notícia de custo (sombras dinâmicas de ponto são a operação mais cara por luz), mas 795 PointLights estáticas ainda pagam custo de:
  - Lightmap/lighting build (armazenamento e memória, não runtime se forem totalmente bakeadas)
  - Com Lumen activo (GI dinâmica), luzes STATIC ainda contribuem para o cálculo de iluminação por pixel — Lumen não é "grátis" só porque a luz é estática.
- **Proposta (ranking #1, baixo risco)**: agrupar PointLights STATIC que estejam a menos de ~150uu umas das outras (padrão comum em VFX de tocha/campfire com luz duplicada) em uma única luz por cluster, com raio de atenuação ajustado. Não posso medir clusters de luz neste ciclo (não fiz essa query), mas o padrão de 3791 pares Niagara sobrepostos (ver abaixo) sugere fortemente que o mesmo padrão de duplicação existe nas luzes anexadas aos mesmos efeitos. **Acção sugerida para o #17 (VFX) ou #8 (Lighting)**: antes de adicionar qualquer luz nova, verificar se já existe uma luz a <150uu no mesmo efeito.

### 4. Niagara — achado principal deste ciclo
- `NIAGARA_TOTAL = 237`, todos com asset válido carregado (amostra de 50/50)
- **`NIAGARA_CLOSE_PAIRS_UNDER_300UU = 3791`** — isto é o número mais importante deste ciclo. Com 237 actores, o máximo teórico de pares é C(237,2)=27966; 3791 pares a menos de 3 metros uns dos outros é uma taxa de sobreposição de ~13.6%, muito acima do que se espera de efeitos distintos e intencionais. Isto indica fortemente que há **stacking de VFX duplicados** (mesmo efeito colocado várias vezes na mesma posição/aproximada, provavelmente por agentes diferentes em ciclos diferentes a decorar o mesmo ponto — ex: fogueira com fumo + faísca + brilho todos como actores separados sobrepostos, ou o mesmo efeito repetido por engano).
- Não consegui confirmar `hidden_in_game`/`is_hidden_ed` nesta API para NiagaraActor (a chamada falhou silenciosamente nas 237 tentativas — `HIDDEN=0 VISIBLE=0` é um resultado **inconclusivo**, não uma afirmação de que nenhum está escondido; reporto a falha de medição em vez de inventar um dado).
- **Proposta (ranking #2, risco médio, custo esperado de investigação baixo)**: o próximo agente com mandato de tocar em VFX (#17) deveria, antes de adicionar qualquer NiagaraActor novo, correr uma query de proximidade (<300uu) contra os 237 existentes e fundir/remover duplicados manifestos. Isto reduziria simultaneamente a carga de partículas (GPU) e o tecto de decoração (A05: 951 decorativos, tecto 986 — margem de apenas 35).

### 5. TriggerBox/TriggerSphere
- `TriggerBox=123` (todos com colisão activa), `TriggerSphere=35` (todos com colisão activa) → 158 volumes com overlap events activos. Isto é baixo custo por si (eventos de overlap só disparam em transição, não por tick contínuo já que actor_tick_enabled=0), não requer acção.

## Ranking de propostas (impacto estimado vs risco vs custo)
| # | Proposta | Impacto esperado | Risco | Quem executa |
|---|---|---|---|---|
| 1 | Investigar e fundir NiagaraActor duplicados/sobrepostos (3791 pares <300uu) | Médio-alto (GPU partículas + margem A05) | Baixo (é remoção de duplicado óbvio, não de conteúdo funcional) | #17 VFX |
| 2 | Auditar PointLights STATIC por cluster (<150uu) para fusão | Baixo-médio (custo Lumen por pixel) | Baixo | #8 Lighting |
| 3 | Nova amostra FPS fresca em PIE real | Informativo (sem custo de fps) | N/A — requer mandato do #22 | #22 |
| 4 | Confirmar leitura de `hidden_in_game` em NiagaraActor (API falhou) | Nenhum custo directo, mas destrava medição futura | Nenhum | qualquer agente com Python UE5 |

## Nenhuma acção destrutiva tomada
Não movi, apaguei, reescalei, nem mudei mobility/tags/visibilidade de nenhum actor. Não toquei em `Dino_*`, Landscape, Terrain_Savana, câmara do editor, ou pawn do jogador. Não criei `.cpp`/`.h`.

## Dependências para o próximo agente (#5 Procedural World Generator)
- Nenhum bloqueio da minha parte. O headroom de actores é confortável (4456 até ao cap de 8000), pelo que o #5 pode continuar a povoar dentro do seu mandato sem preocupação de tecto de performance geral.
- Nota: se o #5 spawnar decoração nova, o tecto A05 (986) está a apenas 35 de distância dos 951 actuais — recomendo que qualquer decoração nova seja coordenada com o #6/#17 dado o achado de duplicação Niagara acima.
