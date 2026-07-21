# Collision Governance Validation Script
# Core Systems Programmer (#03) — PROD_CYCLE_AUTO_20260720_005
#
# PURPOSE
# -------
# The Engine Architect (#02) established a binding rule in
# Docs/Architecture/collision_governance_rule.md:
#   - Auxiliary/editorial actors (Marker_AI_*, Helper_Actor_*, VFX_*, Light_Aux_*)
#     must NEVER have QueryAndPhysics collision on their primitive components
#     (Sprite/Arrow/Billboard) — this creates invisible walls for the player capsule.
#   - Quest/gameplay trigger actors (Trigger_Quest_*, Trigger_ScoutAlert_*, etc.)
#     MUST keep QueryOnly collision on their Box/Sphere components so overlap
#     events (aggro zones, damage triggers, quest logic) keep functioning.
#
# WHY THIS IS A .py FILE, NOT .cpp/.h
# ------------------------------------
# This headless UE5 editor instance runs a pre-built binary and never
# recompiles new C++ (see Brain memory hugo_no_cpp_h_v2, importance MAX).
# Any .cpp/.h written via github_file_write has zero effect on the live game.
# This script is therefore an "Editor Utility" Python script, runnable directly
# inside the UE5 Python console or via ue5_execute(command_type='python'),
# with immediate, verifiable effect on the live world — the only effective
# enforcement channel available in this environment.
#
# WHEN C++ COMPILATION IS RESTORED
# ---------------------------------
# This logic should be ported into a proper editor-only automated test
# (e.g. an FAutomationTestBase subclass, "TranspersonalGame.Architecture.
# CollisionGovernance") that runs on every editor startup / CI build and
# FAILS the build if any auxiliary actor exposes QueryAndPhysics collision,
# or any quest trigger loses QueryOnly collision. Until then, this script
# must be re-run manually every cycle by the Core Systems Programmer.
#
# USAGE (inside UE5 Python / ue5_execute python_code)
# ----------------------------------------------------
#   exec(open(r"<repo>/Tools/EditorValidation/collision_governance_check.py").read())
#   run_collision_governance_check(fix=True)
#
# The function returns a dict report and optionally auto-fixes violations.

import unreal

AUX_PREFIXES = ("Marker_AI_", "Helper_Actor_", "VFX_", "Light_Aux_")
TRIGGER_PREFIX = "Trigger_"

# Component types that are purely editorial/visual and must be ignored
# even if they technically expose a collision property (they shouldn't
# be forced to any particular collision state).
EDITORIAL_ONLY_COMPONENTS = ("BillboardComponent", "ArrowComponent", "TextRenderComponent")


def _get_collision_enabled(component):
    try:
        return component.get_collision_enabled()
    except Exception:
        return None


def run_collision_governance_check(fix: bool = False) -> dict:
    """Scan all level actors and report/fix collision governance violations.

    Args:
        fix: if True, automatically corrects violations found
             (sets NoCollision on aux actor primitives that have
             QueryAndPhysics, and QueryOnly on trigger actors missing it).

    Returns:
        dict with counts and violation details for reporting/logging.
    """
    world = unreal.EditorLevelLibrary.get_editor_world()
    actors = unreal.EditorLevelLibrary.get_all_level_actors()

    report = {
        "world_valid": world is not None,
        "aux_actor_count": 0,
        "aux_violations_found": 0,
        "aux_violations_fixed": 0,
        "trigger_actor_count": 0,
        "trigger_violations_found": 0,
        "trigger_violations_fixed": 0,
        "details": [],
    }

    for actor in actors:
        label = actor.get_actor_label()

        # --- Auxiliary / editorial actors: must have NO_COLLISION ---
        if any(label.startswith(p) for p in AUX_PREFIXES):
            report["aux_actor_count"] += 1
            for comp in actor.get_components_by_class(unreal.PrimitiveComponent):
                ce = _get_collision_enabled(comp)
                if ce is None:
                    continue
                if str(ce) not in ("CollisionEnabled.NO_COLLISION", "NoCollision") and "NO_COLLISION" not in str(ce):
                    report["aux_violations_found"] += 1
                    detail = f"AUX_VIOLATION {label}:{comp.get_class().get_name()}:{ce}"
                    if fix:
                        try:
                            comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
                            report["aux_violations_fixed"] += 1
                            detail += " -> FIXED(NoCollision)"
                        except Exception as e:
                            detail += f" -> FIX_FAILED({e})"
                    report["details"].append(detail)

        # --- Quest/gameplay triggers: must have QUERY_ONLY ---
        if label.startswith(TRIGGER_PREFIX):
            report["trigger_actor_count"] += 1
            for comp in actor.get_components_by_class(unreal.PrimitiveComponent):
                if comp.get_class().get_name() in EDITORIAL_ONLY_COMPONENTS:
                    continue
                ce = _get_collision_enabled(comp)
                if ce is None:
                    continue
                if "QUERY_ONLY" not in str(ce):
                    report["trigger_violations_found"] += 1
                    detail = f"TRIGGER_VIOLATION {label}:{comp.get_class().get_name()}:{ce}"
                    if fix:
                        try:
                            comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_ONLY)
                            report["trigger_violations_fixed"] += 1
                            detail += " -> FIXED(QueryOnly)"
                        except Exception as e:
                            detail += f" -> FIX_FAILED({e})"
                    report["details"].append(detail)

    unreal.log(f"[CollisionGovernance] {report}")
    return report


if __name__ == "__main__":
    run_collision_governance_check(fix=False)
