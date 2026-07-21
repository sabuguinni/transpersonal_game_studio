#include "Combat_TRexBehaviorTree.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "Engine/Engine.h"

UCombat_TRexBehaviorTree::UCombat_TRexBehaviorTree()
{
    // Set default combat parameters for T-Rex
    DetectionRadius = 5000.0f;
    AttackRange = 800.0f;
    ChaseSpeed = 600.0f;
    PatrolSpeed = 200.0f;
    AttackCooldown = 3.0f;
    RoarCooldown = 8.0f;
    
    TerritoryRadius = 10000.0f;
    TerritoryCenter = FVector::ZeroVector;
    HuntingPersistence = 15.0f;
    LostTargetSearchTime = 5.0f;
    
    AggroThreshold = 0.7f;
    FleeThreshold = 0.2f;
    EnrageThreshold = 0.3f;

    // Initialize behavior tree and blackboard
    InitializeBehaviorTree();
}

void UCombat_TRexBehaviorTree::InitializeBehaviorTree()
{
    // Create blackboard data for T-Rex AI state management
    TRexBlackboard = NewObject<UBlackboardData>(this, TEXT("TRexBlackboard"));
    
    if (TRexBlackboard)
    {
        // Add blackboard keys for T-Rex combat AI
        
        // Target tracking
        TRexBlackboard->UpdatePersistentKey<UBlackboardKeyType_Object>(TEXT("TargetActor"));
        TRexBlackboard->UpdatePersistentKey<UBlackboardKeyType_Vector>(TEXT("TargetLocation"));
        TRexBlackboard->UpdatePersistentKey<UBlackboardKeyType_Vector>(TEXT("LastKnownLocation"));
        
        // Combat states
        TRexBlackboard->UpdatePersistentKey<UBlackboardKeyType_Bool>(TEXT("IsInCombat"));
        TRexBlackboard->UpdatePersistentKey<UBlackboardKeyType_Bool>(TEXT("IsChasing"));
        TRexBlackboard->UpdatePersistentKey<UBlackboardKeyType_Bool>(TEXT("IsAttacking"));
        TRexBlackboard->UpdatePersistentKey<UBlackboardKeyType_Bool>(TEXT("IsEnraged"));
        TRexBlackboard->UpdatePersistentKey<UBlackboardKeyType_Bool>(TEXT("ShouldFlee"));
        
        // Territory and patrol
        TRexBlackboard->UpdatePersistentKey<UBlackboardKeyType_Vector>(TEXT("PatrolPoint"));
        TRexBlackboard->UpdatePersistentKey<UBlackboardKeyType_Vector>(TEXT("TerritoryCenter"));
        TRexBlackboard->UpdatePersistentKey<UBlackboardKeyType_Bool>(TEXT("IsPatrolling"));
        
        // Timing and cooldowns
        TRexBlackboard->UpdatePersistentKey<UBlackboardKeyType_Float>(TEXT("LastAttackTime"));
        TRexBlackboard->UpdatePersistentKey<UBlackboardKeyType_Float>(TEXT("LastRoarTime"));
        TRexBlackboard->UpdatePersistentKey<UBlackboardKeyType_Float>(TEXT("CombatStartTime"));
        TRexBlackboard->UpdatePersistentKey<UBlackboardKeyType_Float>(TEXT("HealthPercentage"));
        
        // Distance calculations
        TRexBlackboard->UpdatePersistentKey<UBlackboardKeyType_Float>(TEXT("DistanceToTarget"));
        TRexBlackboard->UpdatePersistentKey<UBlackboardKeyType_Float>(TEXT("DistanceToTerritory"));
        
        UE_LOG(LogTemp, Log, TEXT("T-Rex Blackboard initialized with combat keys"));
    }

    // Create behavior tree asset (will be configured in Blueprint)
    TRexCombatBehaviorTree = NewObject<UBehaviorTree>(this, TEXT("TRexCombatBehaviorTree"));
    
    if (TRexCombatBehaviorTree && TRexBlackboard)
    {
        // Set the blackboard asset for the behavior tree
        TRexCombatBehaviorTree->BlackboardAsset = TRexBlackboard;
        
        UE_LOG(LogTemp, Log, TEXT("T-Rex Combat Behavior Tree initialized successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to initialize T-Rex Behavior Tree or Blackboard"));
    }
}