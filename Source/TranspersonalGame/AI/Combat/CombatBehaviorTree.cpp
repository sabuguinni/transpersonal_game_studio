#include "CombatBehaviorTree.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UCombatBehaviorTree::UCombatBehaviorTree()
{
    DinosaurBehaviorTree = nullptr;
    DinosaurBlackboard = nullptr;
    
    // Initialize default behavior data
    DefaultBehaviorData.CurrentState = ECombat_AIState::Idle;
    DefaultBehaviorData.ThreatLevel = ECombat_ThreatLevel::None;
    DefaultBehaviorData.AggressionLevel = 0.5f;
    DefaultBehaviorData.FearLevel = 0.3f;
    DefaultBehaviorData.AlertRadius = 1000.0f;
    DefaultBehaviorData.AttackRadius = 500.0f;
    DefaultBehaviorData.FleeRadius = 200.0f;
    
    CurrentBehaviorData = DefaultBehaviorData;
    InitializeStateTransitions();
}

void UCombatBehaviorTree::InitializeStateTransitions()
{
    ValidStateTransitions.Empty();
    
    // Idle state transitions
    ValidStateTransitions.Add(ECombat_AIState::Idle, {
        ECombat_AIState::Patrol,
        ECombat_AIState::Alert,
        ECombat_AIState::Hunting
    });
    
    // Patrol state transitions
    ValidStateTransitions.Add(ECombat_AIState::Patrol, {
        ECombat_AIState::Idle,
        ECombat_AIState::Alert,
        ECombat_AIState::Hunting,
        ECombat_AIState::Fleeing
    });
    
    // Alert state transitions
    ValidStateTransitions.Add(ECombat_AIState::Alert, {
        ECombat_AIState::Idle,
        ECombat_AIState::Patrol,
        ECombat_AIState::Aggressive,
        ECombat_AIState::Fleeing,
        ECombat_AIState::Hunting
    });
    
    // Aggressive state transitions
    ValidStateTransitions.Add(ECombat_AIState::Aggressive, {
        ECombat_AIState::Alert,
        ECombat_AIState::Fleeing,
        ECombat_AIState::Defending,
        ECombat_AIState::Hunting
    });
    
    // Fleeing state transitions
    ValidStateTransitions.Add(ECombat_AIState::Fleeing, {
        ECombat_AIState::Idle,
        ECombat_AIState::Patrol,
        ECombat_AIState::Alert
    });
    
    // Hunting state transitions
    ValidStateTransitions.Add(ECombat_AIState::Hunting, {
        ECombat_AIState::Aggressive,
        ECombat_AIState::Alert,
        ECombat_AIState::Patrol,
        ECombat_AIState::Fleeing
    });
    
    // Defending state transitions
    ValidStateTransitions.Add(ECombat_AIState::Defending, {
        ECombat_AIState::Aggressive,
        ECombat_AIState::Alert,
        ECombat_AIState::Fleeing
    });
}

void UCombatBehaviorTree::InitializeCombatBehavior(AAIController* AIController)
{
    if (!AIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("UCombatBehaviorTree::InitializeCombatBehavior - AIController is null"));
        return;
    }
    
    if (DinosaurBehaviorTree)
    {
        AIController->RunBehaviorTree(DinosaurBehaviorTree);
        UE_LOG(LogTemp, Log, TEXT("Combat behavior tree started for AI Controller"));
    }
    
    if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
    {
        SetBlackboardValues(BlackboardComp);
        UE_LOG(LogTemp, Log, TEXT("Blackboard values initialized for combat AI"));
    }
}

void UCombatBehaviorTree::UpdateThreatLevel(ECombat_ThreatLevel NewThreatLevel)
{
    if (CurrentBehaviorData.ThreatLevel != NewThreatLevel)
    {
        ECombat_ThreatLevel OldThreatLevel = CurrentBehaviorData.ThreatLevel;
        CurrentBehaviorData.ThreatLevel = NewThreatLevel;
        
        // Adjust behavior based on threat level
        switch (NewThreatLevel)
        {
            case ECombat_ThreatLevel::None:
                CurrentBehaviorData.AggressionLevel = FMath::Max(0.1f, CurrentBehaviorData.AggressionLevel - 0.3f);
                break;
            case ECombat_ThreatLevel::Low:
                CurrentBehaviorData.AggressionLevel = FMath::Clamp(CurrentBehaviorData.AggressionLevel + 0.1f, 0.2f, 1.0f);
                break;
            case ECombat_ThreatLevel::Medium:
                CurrentBehaviorData.AggressionLevel = FMath::Clamp(CurrentBehaviorData.AggressionLevel + 0.2f, 0.4f, 1.0f);
                break;
            case ECombat_ThreatLevel::High:
                CurrentBehaviorData.AggressionLevel = FMath::Clamp(CurrentBehaviorData.AggressionLevel + 0.3f, 0.6f, 1.0f);
                break;
            case ECombat_ThreatLevel::Critical:
                CurrentBehaviorData.AggressionLevel = 1.0f;
                break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Threat level updated from %d to %d, Aggression: %f"), 
               (int32)OldThreatLevel, (int32)NewThreatLevel, CurrentBehaviorData.AggressionLevel);
    }
}

void UCombatBehaviorTree::TransitionToState(ECombat_AIState NewState)
{
    if (CanTransitionToState(NewState))
    {
        ECombat_AIState OldState = CurrentBehaviorData.CurrentState;
        CurrentBehaviorData.CurrentState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("AI State transition: %d -> %d"), (int32)OldState, (int32)NewState);
        
        // Perform state-specific initialization
        switch (NewState)
        {
            case ECombat_AIState::Aggressive:
                CurrentBehaviorData.AggressionLevel = FMath::Min(1.0f, CurrentBehaviorData.AggressionLevel + 0.2f);
                break;
            case ECombat_AIState::Fleeing:
                CurrentBehaviorData.FearLevel = FMath::Min(1.0f, CurrentBehaviorData.FearLevel + 0.3f);
                break;
            case ECombat_AIState::Idle:
                CurrentBehaviorData.AggressionLevel = FMath::Max(0.1f, CurrentBehaviorData.AggressionLevel - 0.1f);
                CurrentBehaviorData.FearLevel = FMath::Max(0.1f, CurrentBehaviorData.FearLevel - 0.1f);
                break;
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid state transition attempted: %d -> %d"), 
               (int32)CurrentBehaviorData.CurrentState, (int32)NewState);
    }
}

bool UCombatBehaviorTree::CanTransitionToState(ECombat_AIState TargetState) const
{
    return IsValidTransition(CurrentBehaviorData.CurrentState, TargetState);
}

bool UCombatBehaviorTree::IsValidTransition(ECombat_AIState FromState, ECombat_AIState ToState) const
{
    if (const TArray<ECombat_AIState>* ValidTransitions = ValidStateTransitions.Find(FromState))
    {
        return ValidTransitions->Contains(ToState);
    }
    return false;
}

float UCombatBehaviorTree::CalculateAggressionModifier(float DistanceToTarget) const
{
    if (DistanceToTarget <= CurrentBehaviorData.FleeRadius)
    {
        // Very close - high aggression or high fear depending on creature type
        return CurrentBehaviorData.AggressionLevel > 0.6f ? 1.5f : 0.3f;
    }
    else if (DistanceToTarget <= CurrentBehaviorData.AttackRadius)
    {
        // Attack range - normal aggression
        return 1.0f;
    }
    else if (DistanceToTarget <= CurrentBehaviorData.AlertRadius)
    {
        // Alert range - reduced aggression
        return 0.7f;
    }
    else
    {
        // Far away - minimal aggression
        return 0.2f;
    }
}

void UCombatBehaviorTree::SetBlackboardValues(UBlackboardComponent* BlackboardComp)
{
    if (!BlackboardComp)
    {
        return;
    }
    
    // Set combat state values in blackboard
    BlackboardComp->SetValueAsEnum(TEXT("AIState"), (uint8)CurrentBehaviorData.CurrentState);
    BlackboardComp->SetValueAsEnum(TEXT("ThreatLevel"), (uint8)CurrentBehaviorData.ThreatLevel);
    BlackboardComp->SetValueAsFloat(TEXT("AggressionLevel"), CurrentBehaviorData.AggressionLevel);
    BlackboardComp->SetValueAsFloat(TEXT("FearLevel"), CurrentBehaviorData.FearLevel);
    BlackboardComp->SetValueAsFloat(TEXT("AlertRadius"), CurrentBehaviorData.AlertRadius);
    BlackboardComp->SetValueAsFloat(TEXT("AttackRadius"), CurrentBehaviorData.AttackRadius);
    BlackboardComp->SetValueAsFloat(TEXT("FleeRadius"), CurrentBehaviorData.FleeRadius);
    
    UE_LOG(LogTemp, Log, TEXT("Blackboard values set for combat AI"));
}