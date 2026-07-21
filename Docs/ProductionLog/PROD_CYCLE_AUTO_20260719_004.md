# Studio Director — Ciclo PROD_CYCLE_AUTO_20260719_004

## VISUAL FEEDBACK APPLIED
O screenshot injetado (`vision_capture_20260719_220433.png`) reportou:
- **Atmosfera 🟡**: overexposure severo (branco/verde pálido estourado), sem mood Cretáceo (deveria ser quente/âmbar), fog mal configurado, tudo ilegível por exposição.
- **Terreno 🟢**: visível, com variação de elevação subtil, base passável.
- **Actores 🟢**: ~2 dinossauros grandes (sauropod/hadrosaur), 4-6 árvores, 3-4 rochas — densidade decente.
- **Priority Fix do assessment**: corrigir Exponential Height Fog + Post Process Volume imediatamente — reduzir Fog Density, cap Auto Exposure Max a ~2.0, sun intensity apropriada.

### Ação concreta tomada este ciclo (resposta directa ao fix prioritário):
1. Localizados todos os `PostProcessVolume`, `ExponentialHeightFog` e `DirectionalLight` na MinPlayableMap via `ue5_execute` (diagnóstico).
2. Aplicado fix de exposição em cada PostProcessVolume encontrado:
   - `override_auto_exposure_min_brightness = True`, `auto_exposure_min_brightness = 0.5`
   - `override_auto_exposure_max_brightness = True`, `auto_exposure_max_brightness = 2.0` (conforme pedido no assessment)
   - `override_auto_exposure_bias = True`, `auto_exposure_bias = 0.0`
3. Reduzida `fog_density` para `0.02` e `fog_height_falloff` para `0.2` em todos os actores `ExponentialHeightFog` encontrados — resolve o "fog lavando a profundidade" reportado.
4. Ajustada intensidade do `DirectionalLight` (sol) para `5.5` lux com tint quente/âmbar (LinearColor 1.0, 0.92, 0.75) — SEM tocar em `pitch`/rotação do sol, respeitando a regra `hugo_no_camera_v2`/sun guard (apenas intensidade e cor alterados, nunca ângulo).
5. Nível gravado (`save_current_level`) após todas as alterações confirmadas, uma única gravação no fim do turno (regra "save once at end").

### Nota de transparência (limite da ferramenta)
O bridge Remote Control devolve apenas `{"ReturnValue": true}` para chamadas Python — não devolve o `print()`/stdout com os valores numéricos antes/depois. As 4 chamadas `ue5_execute` reportaram `success:true` e `status:completed`, o que confirma execução sem erro no editor, mas os valores exactos (contagem de PPVs/fogs/lights encontrados) não puderam ser lidos de volta nesta sessão. Não invento esses números — reporto apenas que os comandos correram e o nível foi gravado com sucesso.

## Tarefas para a cadeia (ciclo seguinte)

| Agente | Tarefa concreta | Deliverable mensurável |
|---|---|---|
| #08 Lighting & Atmosphere | Validar visualmente (próximo screenshot) se exposure cap (0.5–2.0) e fog density 0.02 resolveram o overexposure. Se ainda estourado, revisar Sky Atmosphere / Sky Light intensity separadamente do Directional Light. | Screenshot com céu não-estourado, sombras legíveis, mood âmbar Cretáceo |
| #05/#06 World/Environment | Aumentar densidade de vegetação no hub (2100,2400) — assessment reporta apenas 4-6 árvores, meta é "floresta densa" (regra hugo_hub_quality_v2_fix) | Contagem de árvores/foliage no core >15, verificada por lista de actors |
| #09/#12 Character/Combat AI | Confirmar que os 2 dinossauros visíveis (sauropod/hadrosaur) têm pose natural e colisão sã; adicionar mais 1-2 espécies (ex. Triceratops, T-Rex) assentes no terreno via trace, sem sobrepor | Lista de actor labels Type_Bioma_NNN com z verificado por trace |
| #18 QA | Validar que nenhuma alteração de exposição/fog quebrou o path walkable ou colisão de actores existentes | Relatório QA pass/fail |

## Decisões técnicas e justificação
- Fix aplicado exclusivamente via `ue5_execute` (Python no editor live), nenhum ficheiro `.cpp/.h` criado — em conformidade com regra `hugo_no_cpp_h_v2`.
- Sol: apenas `intensity` e `light_color` alterados, `pitch`/rotação intocados — em conformidade com o sun pitch guard.
- Nenhuma alteração a `TranspersonalCharacter PLAYER0`, Landscape, foliage ou sublevel `Terrain_Savana` — em conformidade com `hugo_mobility_rule_v1` e `hugo_terrain_savana_v1`.
- Gravação única do nível no fim do ciclo, depois de todas as alterações confirmadas.

## Dependências para o próximo ciclo
- Necessário screenshot novo do `vision_loop.py` para confirmar visualmente se o fix de exposição/fog resolveu o problema reportado (não posso confirmar visualmente sem HTTP externo, por regra `hugo_ue5_no_http_deadlock_v1`).
- #08 deve assumir a validação fina de Sky Atmosphere/Sky Light se o Directional Light sozinho não for suficiente.
