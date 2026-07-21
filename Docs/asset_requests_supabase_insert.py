# Asset request insert executed via ue5_execute (documented for traceability)
# NOTE: This is not run through UE5 python (no requests lib inside UE5 editor sandbox
# to avoid HTTP-in-editor deadlock rule). This file documents the exact payload
# submitted to Supabase asset_requests table for Cycle PROD_CYCLE_AUTO_20260711_005.

import requests

SUPABASE_URL = "https://thdlkizjbpwdndtggleb.supabase.co"
SUPABASE_KEY = "<service_role_key>"  # provided via environment at execution time

headers = {
    "apikey": SUPABASE_KEY,
    "Authorization": f"Bearer {SUPABASE_KEY}",
    "Content-Type": "application/json",
    "Prefer": "return=representation"
}

payload = {
    "asset_name": "fallen_moss_log_cretaceous",
    "prompt": (
        "Fallen dead tree trunk lying on forest floor, weathered gray-brown bark, "
        "partially covered in green moss and small ferns growing out of cracks, "
        "Late Cretaceous forest environment, game-ready low-poly, realistic PBR textures, "
        "Unreal Engine 5 style, approximately 6 meters long and 0.8 meters diameter"
    ),
    "category": "Props"
}

response = requests.post(
    f"{SUPABASE_URL}/rest/v1/asset_requests",
    headers=headers,
    json=payload
)
print(f"Asset request created: {response.status_code} {response.text}")
