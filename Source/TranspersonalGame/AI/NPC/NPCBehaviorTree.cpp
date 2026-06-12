#include "NPCBehaviorTree.h"
#include "Engine/Engine.h"

UNPC_BehaviorTreeAsset::UNPC_BehaviorTreeAsset()
{
    PatrolRadius = 1000.0f;
    ChaseDistance = 1500.0f;
    FleeDistance = 500.0f;
    MemoryDuration = 30.0f;
}