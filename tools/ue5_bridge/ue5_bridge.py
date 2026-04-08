#!/usr/bin/env python3
"""
UE5 Bridge / Relay — Transpersonal Game Studio
================================================
Polls the Supabase `ue5_commands` table for pending commands
and executes them on UE5 via the Remote Control API (port 30010).

Usage:
  python3 ue5_bridge.py                          # Default: localhost:30010
  python3 ue5_bridge.py --ue5-host 192.168.1.50  # Remote UE5 machine
  python3 ue5_bridge.py --ue5-port 30010          # Custom port
  python3 ue5_bridge.py --poll-interval 3         # Poll every 3 seconds

Requirements:
  pip install requests supabase
  (or just: pip install requests — supabase REST API is called via requests)

The script runs continuously and:
  1. Queries ue5_commands WHERE status = 'pending' ORDER BY created_at ASC
  2. For each command, sends the appropriate HTTP request to UE5 Remote Control API
  3. Updates the command status to 'completed' or 'error' with the result
  4. Sends a Telegram notification on errors
"""

import argparse
import json
import logging
import sys
import time
import traceback
from datetime import datetime, timezone

import requests

# ─── Configuration ───────────────────────────────────────────────────────────

SUPABASE_URL = "https://thdlkizjbpwdndtggleb.supabase.co"
SUPABASE_ANON_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InRoZGxraXpqYnB3ZG5kdGdnbGViIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NDM3MDIyMDksImV4cCI6MjA1OTI3ODIwOX0.r5zBFDm7qVFHMOBxMFBPvnqIqHSIgfkwMOJwGhz-n3c"
SUPABASE_SERVICE_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InRoZGxraXpqYnB3ZG5kdGdnbGViIiwicm9sZSI6InNlcnZpY2Vfcm9sZSIsImlhdCI6MTc0MzcwMjIwOSwiZXhwIjoyMDU5Mjc4MjA5fQ.LMCbJEbpBkZ3pFTxBaXqMOy3SjBLSNS-0ez-dlLAelk"

TELEGRAM_BOT_TOKEN = "8470343698:AAHVuWkEFrMI_4VIEJHle867b7htMX_HuPU"
TELEGRAM_CHAT_ID = "5308155885"

# ─── Logging ─────────────────────────────────────────────────────────────────

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    handlers=[
        logging.StreamHandler(sys.stdout),
        logging.FileHandler("ue5_bridge.log", mode="a"),
    ],
)
log = logging.getLogger("ue5_bridge")

# ─── Supabase helpers ────────────────────────────────────────────────────────

HEADERS = {
    "apikey": SUPABASE_SERVICE_KEY,
    "Authorization": f"Bearer {SUPABASE_SERVICE_KEY}",
    "Content-Type": "application/json",
    "Prefer": "return=representation",
}


def supabase_get(table: str, params: dict) -> list:
    """GET rows from a Supabase table."""
    r = requests.get(f"{SUPABASE_URL}/rest/v1/{table}", headers=HEADERS, params=params)
    r.raise_for_status()
    return r.json()


def supabase_patch(table: str, match: dict, data: dict) -> list:
    """PATCH (update) rows in a Supabase table."""
    params = {f"{k}": f"eq.{v}" for k, v in match.items()}
    r = requests.patch(
        f"{SUPABASE_URL}/rest/v1/{table}",
        headers=HEADERS,
        params=params,
        json=data,
    )
    r.raise_for_status()
    return r.json()


# ─── Telegram notification ───────────────────────────────────────────────────

def notify_telegram(message: str):
    """Send a notification to Telegram."""
    try:
        requests.post(
            f"https://api.telegram.org/bot{TELEGRAM_BOT_TOKEN}/sendMessage",
            json={"chat_id": TELEGRAM_CHAT_ID, "text": message, "parse_mode": "HTML"},
            timeout=10,
        )
    except Exception as e:
        log.warning(f"Telegram notification failed: {e}")


# ─── UE5 Remote Control API ─────────────────────────────────────────────────

class UE5RemoteControl:
    """Client for UE5 Remote Control API (HTTP, port 30010 by default)."""

    def __init__(self, host: str = "localhost", port: int = 30010):
        self.base_url = f"http://{host}:{port}"
        self.session = requests.Session()
        self.session.headers.update({"Content-Type": "application/json"})

    def health_check(self) -> bool:
        """Check if UE5 Remote Control API is reachable."""
        try:
            r = self.session.get(f"{self.base_url}/remote/info", timeout=5)
            return r.status_code == 200
        except Exception:
            return False

    def execute_python(self, script: str) -> dict:
        """Execute a Python script in the UE5 Editor."""
        r = self.session.put(
            f"{self.base_url}/remote/script/execute",
            json={"Command": script},
            timeout=120,
        )
        return {"status_code": r.status_code, "body": r.text}

    def call_function(self, object_path: str, function_name: str, parameters: dict = None) -> dict:
        """Call a function on a UObject."""
        payload = {
            "objectPath": object_path,
            "functionName": function_name,
        }
        if parameters:
            payload["parameters"] = parameters
        r = self.session.put(
            f"{self.base_url}/remote/object/call",
            json=payload,
            timeout=120,
        )
        return {"status_code": r.status_code, "body": r.text}

    def set_property(self, object_path: str, property_name: str, value) -> dict:
        """Set a property on a UObject."""
        r = self.session.put(
            f"{self.base_url}/remote/object/property",
            json={
                "objectPath": object_path,
                "propertyName": property_name,
                "propertyValue": {"value": value},
                "access": "WRITE_ACCESS",
            },
            timeout=30,
        )
        return {"status_code": r.status_code, "body": r.text}

    def get_property(self, object_path: str, property_name: str) -> dict:
        """Get a property from a UObject."""
        r = self.session.put(
            f"{self.base_url}/remote/object/property",
            json={
                "objectPath": object_path,
                "propertyName": property_name,
                "access": "READ_ACCESS",
            },
            timeout=30,
        )
        return {"status_code": r.status_code, "body": r.text}

    def spawn_actor(self, class_path: str, location: dict = None, rotation: dict = None, label: str = None) -> dict:
        """Spawn an actor in the current level."""
        # Use Python scripting to spawn — more flexible than object/call
        loc = location or {"X": 0, "Y": 0, "Z": 0}
        rot = rotation or {"Pitch": 0, "Yaw": 0, "Roll": 0}
        label_str = f'.set_actor_label("{label}")' if label else ""

        script = f"""
import unreal
editor = unreal.EditorLevelLibrary()
location = unreal.Vector({loc['X']}, {loc['Y']}, {loc['Z']})
rotation = unreal.Rotator({rot.get('Pitch', 0)}, {rot.get('Yaw', 0)}, {rot.get('Roll', 0)})
actor_class = unreal.EditorAssetLibrary.load_asset('{class_path}')
if actor_class:
    actor = editor.spawn_actor_from_object(actor_class, location, rotation)
    if actor:
        {f'actor.set_actor_label("{label}")' if label else 'pass'}
        print(f"Spawned: {{actor.get_name()}} at {{location}}")
    else:
        print("ERROR: Failed to spawn actor")
else:
    print("ERROR: Could not load class {class_path}")
"""
        return self.execute_python(script)

    def run_console_command(self, command: str) -> dict:
        """Execute a console command in UE5."""
        script = f"""
import unreal
unreal.SystemLibrary.execute_console_command(None, '{command}')
print("Console command executed: {command}")
"""
        return self.execute_python(script)

    def import_asset(self, source_path: str, destination_path: str) -> dict:
        """Import an asset into the UE5 project."""
        script = f"""
import unreal
task = unreal.AssetImportTask()
task.filename = r'{source_path}'
task.destination_path = '{destination_path}'
task.automated = True
task.replace_existing = True
task.save = True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
print(f"Imported: {source_path} -> {destination_path}")
"""
        return self.execute_python(script)


# ─── Command Executor ────────────────────────────────────────────────────────

def execute_command(ue5: UE5RemoteControl, cmd: dict) -> dict:
    """
    Execute a single ue5_command on the UE5 instance.
    Returns {"success": bool, "result": str}
    """
    cmd_type = cmd.get("command_type", "").lower()
    # DB column is 'command_data', but we also check 'parameters' for backwards compat
    params = cmd.get("command_data", cmd.get("parameters", {}))

    # Parse parameters if it's a string
    if isinstance(params, str):
        try:
            params = json.loads(params)
        except json.JSONDecodeError:
            params = {"raw": params}

    log.info(f"Executing command: {cmd_type} | Agent #{cmd.get('agent_number')} | Cycle: {cmd.get('cycle_id')}")

    try:
        if cmd_type == "execute_python":
            script = params.get("script", params.get("code", params.get("raw", "")))
            if not script:
                return {"success": False, "result": "No script provided in parameters"}
            result = ue5.execute_python(script)

        elif cmd_type == "set_property":
            result = ue5.set_property(
                params.get("object_path", ""),
                params.get("property_name", ""),
                params.get("value"),
            )

        elif cmd_type == "get_property":
            result = ue5.get_property(
                params.get("object_path", ""),
                params.get("property_name", ""),
            )

        elif cmd_type == "spawn_actor":
            result = ue5.spawn_actor(
                params.get("class_path", ""),
                params.get("location"),
                params.get("rotation"),
                params.get("label"),
            )

        elif cmd_type == "import_asset":
            result = ue5.import_asset(
                params.get("source_path", ""),
                params.get("destination_path", ""),
            )

        elif cmd_type == "call_function":
            result = ue5.call_function(
                params.get("object_path", ""),
                params.get("function_name", ""),
                params.get("parameters"),
            )

        elif cmd_type == "run_console_command":
            command = params.get("command", params.get("raw", ""))
            result = ue5.run_console_command(command)

        else:
            return {"success": False, "result": f"Unknown command type: {cmd_type}"}

        # Check if UE5 returned an error
        status_code = result.get("status_code", 0)
        body = result.get("body", "")

        if status_code >= 200 and status_code < 300:
            return {"success": True, "result": body[:4000]}  # Truncate large responses
        else:
            return {"success": False, "result": f"UE5 returned {status_code}: {body[:2000]}"}

    except requests.exceptions.Timeout:
        return {"success": False, "result": "UE5 command timed out (120s)"}
    except requests.exceptions.ConnectionError:
        return {"success": False, "result": "Cannot connect to UE5 Remote Control API"}
    except Exception as e:
        return {"success": False, "result": f"Exception: {str(e)}"}


# ─── Main Loop ───────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(description="UE5 Bridge — Transpersonal Game Studio")
    parser.add_argument("--ue5-host", default="localhost", help="UE5 Remote Control host (default: localhost)")
    parser.add_argument("--ue5-port", type=int, default=30010, help="UE5 Remote Control port (default: 30010)")
    parser.add_argument("--poll-interval", type=int, default=5, help="Seconds between polls (default: 5)")
    parser.add_argument("--max-batch", type=int, default=5, help="Max commands per poll (default: 5)")
    args = parser.parse_args()

    ue5 = UE5RemoteControl(host=args.ue5_host, port=args.ue5_port)

    # Startup health check
    log.info(f"UE5 Bridge starting — connecting to {args.ue5_host}:{args.ue5_port}")
    if ue5.health_check():
        log.info("UE5 Remote Control API is reachable")
        notify_telegram(
            f"🟢 <b>UE5 Bridge Online</b>\n"
            f"Connected to {args.ue5_host}:{args.ue5_port}\n"
            f"Polling interval: {args.poll_interval}s"
        )
    else:
        log.warning("UE5 Remote Control API is NOT reachable — will retry on each poll")
        notify_telegram(
            f"🟡 <b>UE5 Bridge Started (UE5 not yet reachable)</b>\n"
            f"Target: {args.ue5_host}:{args.ue5_port}\n"
            f"Will retry on each poll cycle"
        )

    commands_executed = 0
    commands_failed = 0
    last_health_check = time.time()

    while True:
        try:
            # Periodic health check every 60 seconds
            now = time.time()
            if now - last_health_check > 60:
                if not ue5.health_check():
                    log.warning("UE5 health check failed — editor may be closed or unreachable")
                last_health_check = now

            # Poll for pending commands
            pending = supabase_get("ue5_commands", {
                "status": "eq.pending",
                "order": "created_at.asc",
                "limit": str(args.max_batch),
            })

            if not pending:
                time.sleep(args.poll_interval)
                continue

            log.info(f"Found {len(pending)} pending command(s)")

            for cmd in pending:
                cmd_id = cmd.get("id")

                # Mark as "executing"
                supabase_patch("ue5_commands", {"id": cmd_id}, {
                    "status": "executing",
                    "started_at": datetime.now(timezone.utc).isoformat(),
                })

                # Execute
                result = execute_command(ue5, cmd)

                # Update status
                if result["success"]:
                    supabase_patch("ue5_commands", {"id": cmd_id}, {
                        "status": "completed",
                        "result": result["result"],
                        "completed_at": datetime.now(timezone.utc).isoformat(),
                    })
                    commands_executed += 1
                    log.info(f"Command {cmd_id} completed | Agent #{cmd.get('agent_number')} | Total: {commands_executed}")
                else:
                    supabase_patch("ue5_commands", {"id": cmd_id}, {
                        "status": "error",
                        "result": result["result"],
                        "completed_at": datetime.now(timezone.utc).isoformat(),
                    })
                    commands_failed += 1
                    log.error(f"Command {cmd_id} FAILED: {result['result'][:200]}")

                    # Notify on error
                    agent_num = cmd.get("agent_number", "?")
                    cycle_id = cmd.get("cycle_id", "?")
                    notify_telegram(
                        f"🔴 <b>UE5 Command Failed</b>\n"
                        f"Agent: #{agent_num}\n"
                        f"Cycle: {cycle_id}\n"
                        f"Type: {cmd.get('command_type', '?')}\n"
                        f"Error: {result['result'][:300]}"
                    )

        except requests.exceptions.ConnectionError:
            log.error("Lost connection to Supabase — retrying in 30s")
            time.sleep(30)
        except Exception as e:
            log.error(f"Unexpected error in main loop: {e}")
            log.error(traceback.format_exc())
            time.sleep(10)

        time.sleep(args.poll_interval)


if __name__ == "__main__":
    main()
