# Asset Request — asset_requests table row

| Field | Value |
|---|---|
| requested_by | Agent #09 — Character Artist Agent |
| cycle_id | PROD_CYCLE_AUTO_20260713_004 |
| asset_type | Cretaceous creature/character prop |
| asset_name | Triceratops Fossil Skull (weathered landmark/trophy prop) |
| pipeline | Meshy.ai (meshy_generate) |
| status | BLOCKED — HTTP 402 Insufficient Funds |
| prompt_used | "Triceratops skull prop, weathered bone fossil, scientifically accurate, partial fossilized skull with horns and frill, natural bone texture, no color paint, suitable for prehistoric survival game trophy/landmark prop, full detail, static pose" |
| mode | preview |
| art_style | realistic |
| target_polycount | 15000 |
| intended_use | Landmark/quest prop near hub (X=2100,Y=2400) or shelter sites (#07); reinforces "biography visible in world objects" character-art philosophy — a skull tells a story of a hunt or a death, same as a scarred face tells a story of a life |
| next_action | Retry meshy_generate next cycle once Meshy account balance is topped up. Do not retry same cycle (billing issue, not transient per Brain memory). |

## Context
This satisfies Criterio 3 (A) — INSERT 1 row into asset_requests for a Cretaceous creature/character prop.
The Meshy pipeline call was executed this cycle (`meshy_generate`) and failed with HTTP 402 (insufficient funds), consistent with #06/#07's blockers reported the same cycle. The row above documents the request as BLOCKED for retry, per the mandatory INSERT requirement.
