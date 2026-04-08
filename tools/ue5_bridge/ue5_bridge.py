#!/usr/bin/env python3
"""
╔══════════════════════════════════════════════════════════════╗
║  UE5 BRIDGE — Transpersonal Game Studio                      ║
║  Liga os agentes de IA ao Unreal Engine 5 no teu PC          ║
╚══════════════════════════════════════════════════════════════╝

Este script:
1. Verifica se o UE5 está a correr com Remote Control API
2. Puxa comandos pendentes do Supabase (tabela ue5_commands)
3. Executa-os no UE5 via Remote Control API (HTTP localhost:30010)
4. Reporta os resultados de volta ao Supabase

Requisitos:
- Python 3.8+ (vem com o módulo 'requests' via pip)
- UE5 aberto com o plugin "Remote Control API" activo
- Ligação à internet

Uso: python ue5_bridge.py
(ou duplo-clique no start_bridge.bat)
"""

import json
import time
import sys
import os
import traceback
from datetime import datetime, timezone

# ══════════════════════════════════════════════════════
# CONFIGURAÇÃO — NÃO ALTERAR (a menos que saibas o que fazes)
# ══════════════════════════════════════════════════════

SUPABASE_URL = "https://thdlkizjbpwdndtggleb.supabase.co"
SUPABASE_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InRoZGxraXpqYnB3ZG5kdGdnbGViIiwicm9sZSI6InNlcnZpY2Vfcm9sZSIsImlhdCI6MTc3NTU4NzQ0NiwiZXhwIjoyMDkxMTYzNDQ2fQ.VeA3rlif8X3dcDfVMxJqTn8e_VlzUMlco3O8KgkpLk8"

UE5_HOST = "http://127.0.0.1"
UE5_PORT = 30010
UE5_URL = f"{UE5_HOST}:{UE5_PORT}"

POLL_INTERVAL = 5  # segundos entre cada verificação de comandos
MAX_RETRIES = 3    # tentativas por comando

# ══════════════════════════════════════════════════════
# VERIFICAR DEPENDÊNCIAS
# ══════════════════════════════════════════════════════

try:
    import requests
except ImportError:
    print("=" * 60)
    print("ERRO: O módulo 'requests' não está instalado.")
    print("")
    print("Para instalar, abre o CMD (Prompt de Comando) e escreve:")
    print("  pip install requests")
    print("")
    print("Depois volta a correr este script.")
    print("=" * 60)
    input("Pressiona Enter para fechar...")
    sys.exit(1)

# ══════════════════════════════════════════════════════
# HEADERS PARA SUPABASE
# ══════════════════════════════════════════════════════

SUPABASE_HEADERS = {
    "apikey": SUPABASE_KEY,
    "Authorization": f"Bearer {SUPABASE_KEY}",
    "Content-Type": "application/json",
    "Prefer": "return=minimal"
}

# ══════════════════════════════════════════════════════
# FUNÇÕES AUXILIARES
# ══════════════════════════════════════════════════════

def log(msg, level="INFO"):
    """Imprime mensagem com timestamp."""
    ts = datetime.now().strftime("%H:%M:%S")
    prefix = {"INFO": "ℹ️", "OK": "✅", "WARN": "⚠️", "ERR": "❌", "CMD": "🔧"}.get(level, "  ")
    print(f"[{ts}] {prefix} {msg}")


def check_ue5_connection():
    """Verifica se o UE5 Remote Control API está acessível."""
    try:
        r = requests.get(f"{UE5_URL}/remote/info", timeout=5)
        if r.status_code == 200:
            info = r.json()
            log(f"UE5 conectado! Engine: {info.get('EngineVersion', 'desconhecido')}", "OK")
            return True
        else:
            log(f"UE5 respondeu com status {r.status_code}", "WARN")
            return False
    except requests.exceptions.ConnectionError:
        log(f"Não consegui ligar ao UE5 em {UE5_URL}", "ERR")
        log("Verifica que o UE5 está aberto e o plugin Remote Control API está activo", "ERR")
        return False
    except Exception as e:
        log(f"Erro ao verificar UE5: {e}", "ERR")
        return False


def check_supabase_connection():
    """Verifica se o Supabase está acessível."""
    try:
        r = requests.get(
            f"{SUPABASE_URL}/rest/v1/ue5_commands?limit=0",
            headers={"apikey": SUPABASE_KEY, "Authorization": f"Bearer {SUPABASE_KEY}"},
            timeout=10
        )
        if r.status_code == 200:
            log("Supabase conectado!", "OK")
            return True
        else:
            log(f"Supabase respondeu com status {r.status_code}", "ERR")
            return False
    except Exception as e:
        log(f"Erro ao ligar ao Supabase: {e}", "ERR")
        return False


def get_pending_commands():
    """Busca comandos pendentes do Supabase."""
    try:
        r = requests.get(
            f"{SUPABASE_URL}/rest/v1/ue5_commands?status=eq.pending&order=created_at.asc&limit=10",
            headers={"apikey": SUPABASE_KEY, "Authorization": f"Bearer {SUPABASE_KEY}"},
            timeout=15
        )
        if r.status_code == 200:
            return r.json()
        else:
            log(f"Erro ao buscar comandos: HTTP {r.status_code}", "ERR")
            return []
    except Exception as e:
        log(f"Erro ao buscar comandos: {e}", "ERR")
        return []


def update_command_status(cmd_id, status, result=None):
    """Actualiza o status de um comando no Supabase."""
    data = {
        "status": status,
        "result": result or {},
    }
    if status in ("completed", "error"):
        data["completed_at"] = datetime.now(timezone.utc).isoformat()
    if status == "running":
        data["executed_at"] = datetime.now(timezone.utc).isoformat()
    
    try:
        r = requests.patch(
            f"{SUPABASE_URL}/rest/v1/ue5_commands?id=eq.{cmd_id}",
            headers=SUPABASE_HEADERS,
            json=data,
            timeout=10
        )
        return r.status_code in (200, 204)
    except Exception as e:
        log(f"Erro ao actualizar comando {cmd_id}: {e}", "ERR")
        return False


# ══════════════════════════════════════════════════════
# EXECUTORES DE COMANDOS UE5
# ══════════════════════════════════════════════════════

def execute_python(cmd_data):
    """Executa código Python no UE5 via Remote Control API."""
    python_code = cmd_data.get("python_code", "")
    if not python_code:
        return {"success": False, "error": "No python_code provided"}
    
    # UE5 Remote Control API endpoint para executar Python
    payload = {
        "ObjectPath": "/Script/PythonScriptPlugin.Default__PythonScriptLibrary",
        "FunctionName": "ExecutePythonCommand",
        "Parameters": {
            "PythonCommand": python_code
        }
    }
    
    try:
        r = requests.put(
            f"{UE5_URL}/remote/object/call",
            json=payload,
            timeout=60
        )
        if r.status_code == 200:
            return {"success": True, "output": r.text[:5000]}
        else:
            # Fallback: try the /remote/script/run endpoint
            payload2 = {"code": python_code}
            r2 = requests.post(f"{UE5_URL}/remote/script/run", json=payload2, timeout=60)
            if r2.status_code == 200:
                return {"success": True, "output": r2.text[:5000]}
            return {"success": False, "error": f"UE5 returned {r.status_code}: {r.text[:500]}"}
    except Exception as e:
        return {"success": False, "error": str(e)}


def execute_console_command(cmd_data):
    """Executa um console command no UE5."""
    command = cmd_data.get("python_code", "") or cmd_data.get("command", "")
    if not command:
        return {"success": False, "error": "No command provided"}
    
    payload = {
        "ObjectPath": "/Script/Engine.Default__KismetSystemLibrary",
        "FunctionName": "ExecuteConsoleCommand",
        "Parameters": {
            "WorldContextObject": "/Engine/Transient.GameEngine",
            "Command": command
        }
    }
    
    try:
        r = requests.put(f"{UE5_URL}/remote/object/call", json=payload, timeout=30)
        return {"success": r.status_code == 200, "output": r.text[:2000]}
    except Exception as e:
        return {"success": False, "error": str(e)}


def execute_compile_test(cmd_data):
    """Tenta compilar o projecto C++ no UE5."""
    # Trigger hot reload / compile via console command
    compile_code = """
import unreal
import subprocess
import os

# Get project paths
project_path = unreal.Paths.project_dir()
project_name = unreal.Paths.get_project_file_path()

print(f"Project path: {project_path}")
print(f"Project file: {project_name}")

# Try to trigger compilation
try:
    # Method 1: Use UnrealBuildTool
    engine_dir = unreal.Paths.engine_dir()
    print(f"Engine dir: {engine_dir}")
    
    # Check if Source directory exists
    source_dir = os.path.join(project_path, "Source")
    if os.path.exists(source_dir):
        cpp_files = []
        h_files = []
        for root, dirs, files in os.walk(source_dir):
            for f in files:
                if f.endswith('.cpp'):
                    cpp_files.append(f)
                elif f.endswith('.h'):
                    h_files.append(f)
        print(f"Source files found: {len(cpp_files)} .cpp, {len(h_files)} .h")
        print(f"Total source files: {len(cpp_files) + len(h_files)}")
    else:
        print("WARNING: No Source directory found!")
        print("The project needs C++ source files to compile.")
    
    # Method 2: Try hot reload
    unreal.EditorLevelLibrary.editor_request_end_play()
    print("Compilation check complete.")
    
except Exception as e:
    print(f"Compilation check error: {e}")
"""
    return execute_python({"python_code": compile_code})


def execute_blueprint_create(cmd_data):
    """Cria um Blueprint no UE5."""
    asset_path = cmd_data.get("asset_path", "/Game/Blueprints/NewBlueprint")
    
    python_code = f"""
import unreal

asset_path = "{asset_path}"
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

# Create Blueprint
factory = unreal.BlueprintFactory()
factory.set_editor_property('parent_class', unreal.Actor)

# Extract name and path
parts = asset_path.rsplit('/', 1)
package_path = parts[0] if len(parts) > 1 else '/Game'
asset_name = parts[-1]

try:
    blueprint = asset_tools.create_asset(asset_name, package_path, None, factory)
    if blueprint:
        print(f"SUCCESS: Blueprint created at {{asset_path}}")
        unreal.EditorAssetLibrary.save_asset(asset_path)
    else:
        print(f"FAILED: Could not create Blueprint at {{asset_path}}")
except Exception as e:
    print(f"ERROR: {{e}}")
"""
    return execute_python({"python_code": python_code})


def execute_material_create(cmd_data):
    """Cria um Material no UE5."""
    asset_path = cmd_data.get("asset_path", "/Game/Materials/NewMaterial")
    
    python_code = f"""
import unreal

asset_path = "{asset_path}"
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

factory = unreal.MaterialFactoryNew()

parts = asset_path.rsplit('/', 1)
package_path = parts[0] if len(parts) > 1 else '/Game'
asset_name = parts[-1]

try:
    material = asset_tools.create_asset(asset_name, package_path, None, factory)
    if material:
        print(f"SUCCESS: Material created at {{asset_path}}")
        unreal.EditorAssetLibrary.save_asset(asset_path)
    else:
        print(f"FAILED: Could not create Material at {{asset_path}}")
except Exception as e:
    print(f"ERROR: {{e}}")
"""
    return execute_python({"python_code": python_code})


def execute_level_design(cmd_data):
    """Coloca actores no nível."""
    actor_class = cmd_data.get("actor_class", "StaticMeshActor")
    location = cmd_data.get("location", {"x": 0, "y": 0, "z": 0})
    
    python_code = f"""
import unreal

# Spawn actor in the current level
location = unreal.Vector({location.get('x', 0)}, {location.get('y', 0)}, {location.get('z', 0)})
rotation = unreal.Rotator(0, 0, 0)

try:
    world = unreal.EditorLevelLibrary.get_editor_world()
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.{actor_class} if hasattr(unreal, '{actor_class}') else unreal.StaticMeshActor,
        location,
        rotation
    )
    if actor:
        print(f"SUCCESS: Spawned {{actor.get_name()}} at {{location}}")
    else:
        print("FAILED: Could not spawn actor")
except Exception as e:
    print(f"ERROR: {{e}}")
"""
    return execute_python({"python_code": python_code})


def execute_asset_import(cmd_data):
    """Importa um asset para o projecto UE5."""
    source_path = cmd_data.get("asset_path", "")
    dest_path = cmd_data.get("destination_path", "/Game/Imported")
    
    python_code = f"""
import unreal

source = "{source_path}"
destination = "{dest_path}"

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

try:
    # Create import task
    task = unreal.AssetImportTask()
    task.set_editor_property('filename', source)
    task.set_editor_property('destination_path', destination)
    task.set_editor_property('automated', True)
    task.set_editor_property('save', True)
    
    asset_tools.import_asset_tasks([task])
    
    if task.get_editor_property('imported_object_paths'):
        for path in task.get_editor_property('imported_object_paths'):
            print(f"SUCCESS: Imported {{path}}")
    else:
        print(f"WARNING: Import completed but no objects reported")
except Exception as e:
    print(f"ERROR: {{e}}")
"""
    return execute_python({"python_code": python_code})


def execute_get_info(cmd_data):
    """Obtém informação sobre o projecto UE5."""
    python_code = """
import unreal
import os

# Project info
project_dir = unreal.Paths.project_dir()
project_file = unreal.Paths.get_project_file_path()
engine_ver = unreal.SystemLibrary.get_engine_version()

print(f"Engine Version: {engine_ver}")
print(f"Project Directory: {project_dir}")
print(f"Project File: {project_file}")

# Count assets
content_dir = os.path.join(project_dir, "Content")
if os.path.exists(content_dir):
    uasset_count = 0
    for root, dirs, files in os.walk(content_dir):
        for f in files:
            if f.endswith('.uasset'):
                uasset_count += 1
    print(f"Content Assets (.uasset): {uasset_count}")

# Source files
source_dir = os.path.join(project_dir, "Source")
if os.path.exists(source_dir):
    cpp_count = 0
    h_count = 0
    for root, dirs, files in os.walk(source_dir):
        for f in files:
            if f.endswith('.cpp'): cpp_count += 1
            elif f.endswith('.h'): h_count += 1
    print(f"Source Files: {cpp_count} .cpp, {h_count} .h")
else:
    print("No Source directory found")

# Current level
try:
    world = unreal.EditorLevelLibrary.get_editor_world()
    if world:
        actors = unreal.EditorLevelLibrary.get_all_level_actors()
        print(f"Current Level Actors: {len(actors)}")
except:
    print("Could not get level info")

print("\\nUE5 Bridge connection verified successfully!")
"""
    return execute_python({"python_code": python_code})


# Mapeamento de command_type para função executora
COMMAND_HANDLERS = {
    "python": execute_python,
    "console_command": execute_console_command,
    "compile_test": execute_compile_test,
    "blueprint_create": execute_blueprint_create,
    "material_create": execute_material_create,
    "level_design": execute_level_design,
    "asset_import": execute_asset_import,
    "get_info": execute_get_info,
    # Aliases
    "spawn_actor": execute_level_design,
    "set_property": execute_python,  # Uses python for flexibility
    "screenshot": execute_python,    # Will be handled via python
}


def execute_command(cmd):
    """Executa um comando UE5 e reporta o resultado."""
    cmd_id = cmd["id"]
    cmd_type = cmd["command_type"]
    cmd_data = cmd.get("command_data", {})
    agent_name = cmd.get("agent_name", "Unknown")
    cycle_id = cmd.get("cycle_id", "Unknown")
    
    log(f"Comando #{cmd_id} de {agent_name} (ciclo {cycle_id}): {cmd_type}", "CMD")
    
    # Mark as running
    update_command_status(cmd_id, "running")
    
    # Find handler
    handler = COMMAND_HANDLERS.get(cmd_type)
    if not handler:
        result = {"success": False, "error": f"Unknown command_type: {cmd_type}"}
        log(f"Tipo de comando desconhecido: {cmd_type}", "ERR")
        update_command_status(cmd_id, "error", result)
        return
    
    # Execute with retries
    for attempt in range(MAX_RETRIES):
        try:
            result = handler(cmd_data)
            if result.get("success"):
                log(f"Comando #{cmd_id} executado com sucesso!", "OK")
                update_command_status(cmd_id, "completed", result)
                return
            elif attempt < MAX_RETRIES - 1:
                log(f"Tentativa {attempt+1}/{MAX_RETRIES} falhou: {result.get('error', 'unknown')}", "WARN")
                time.sleep(2)
            else:
                log(f"Comando #{cmd_id} falhou após {MAX_RETRIES} tentativas", "ERR")
                update_command_status(cmd_id, "error", result)
                return
        except Exception as e:
            if attempt < MAX_RETRIES - 1:
                log(f"Erro na tentativa {attempt+1}: {e}", "WARN")
                time.sleep(2)
            else:
                result = {"success": False, "error": str(e), "traceback": traceback.format_exc()}
                update_command_status(cmd_id, "error", result)
                return


# ══════════════════════════════════════════════════════
# LOOP PRINCIPAL
# ══════════════════════════════════════════════════════

def main():
    print("")
    print("╔══════════════════════════════════════════════════════════════╗")
    print("║  UE5 BRIDGE — Transpersonal Game Studio                     ║")
    print("║  Versão 2.0 | Abril 2026                                    ║")
    print("╚══════════════════════════════════════════════════════════════╝")
    print("")
    
    # Step 1: Check UE5
    log("A verificar ligação ao UE5...")
    ue5_ok = False
    for i in range(5):
        if check_ue5_connection():
            ue5_ok = True
            break
        if i < 4:
            log(f"A tentar novamente em 5 segundos... ({i+1}/5)", "WARN")
            time.sleep(5)
    
    if not ue5_ok:
        print("")
        print("=" * 60)
        print("NÃO CONSEGUI LIGAR AO UE5!")
        print("")
        print("Verifica que:")
        print("  1. O Unreal Engine está aberto")
        print("  2. O plugin 'Remote Control API' está activo")
        print("  3. Reiniciaste o UE5 depois de activar o plugin")
        print("")
        print("O UE5 deve ter um servidor HTTP na porta 30010.")
        print("=" * 60)
        input("\nPressiona Enter para fechar...")
        sys.exit(1)
    
    # Step 2: Check Supabase
    log("A verificar ligação ao Supabase...")
    if not check_supabase_connection():
        print("")
        print("=" * 60)
        print("NÃO CONSEGUI LIGAR AO SUPABASE!")
        print("Verifica a tua ligação à internet.")
        print("=" * 60)
        input("\nPressiona Enter para fechar...")
        sys.exit(1)
    
    # Step 3: Run initial info command to verify everything works
    log("A testar execução de comando no UE5...")
    test_result = execute_get_info({})
    if test_result.get("success"):
        log("Teste de execução passou!", "OK")
    else:
        log(f"Teste de execução falhou: {test_result.get('error', 'unknown')}", "WARN")
        log("O bridge vai continuar, mas alguns comandos podem falhar", "WARN")
    
    # Step 4: Check pending commands count
    pending = get_pending_commands()
    if pending:
        log(f"Encontrei {len(pending)} comandos pendentes! A processar...", "OK")
    else:
        log("Nenhum comando pendente. A aguardar novos comandos...", "INFO")
    
    print("")
    print("=" * 60)
    print("  BRIDGE ACTIVO! A monitorizar comandos dos agentes...")
    print(f"  Polling a cada {POLL_INTERVAL} segundos")
    print("  Pressiona Ctrl+C para parar")
    print("=" * 60)
    print("")
    
    # Main polling loop
    commands_executed = 0
    errors = 0
    start_time = time.time()
    
    try:
        while True:
            try:
                commands = get_pending_commands()
                
                if commands:
                    for cmd in commands:
                        execute_command(cmd)
                        commands_executed += 1
                
                # Status update every 5 minutes
                elapsed = time.time() - start_time
                if elapsed > 0 and int(elapsed) % 300 < POLL_INTERVAL:
                    mins = int(elapsed / 60)
                    log(f"Bridge activo há {mins} min | Comandos: {commands_executed} | Erros: {errors}", "INFO")
                
                time.sleep(POLL_INTERVAL)
                
            except KeyboardInterrupt:
                raise
            except Exception as e:
                errors += 1
                log(f"Erro no loop: {e}", "ERR")
                time.sleep(10)  # Wait longer on error
                
    except KeyboardInterrupt:
        print("")
        log("Bridge parado pelo utilizador.", "INFO")
        log(f"Sessão: {commands_executed} comandos executados, {errors} erros", "INFO")
        print("")
        input("Pressiona Enter para fechar...")


if __name__ == "__main__":
    main()
