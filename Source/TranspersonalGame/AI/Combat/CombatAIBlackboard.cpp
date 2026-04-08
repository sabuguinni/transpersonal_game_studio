#include "CombatAIBlackboard.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"

// Define static key names
const FName UCombatAIBlackboard::PlayerActorKey = TEXT("PlayerActor");
const FName UCombatAIBlackboard::PlayerLocationKey = TEXT("PlayerLocation");
const FName UCombatAIBlackboard::LastKnownPlayerLocationKey = TEXT("LastKnownPlayerLocation");
const FName UCombatAIBlackboard::CombatStateKey = TEXT("CombatState");
const FName UCombatAIBlackboard::ThreatLevelKey = TEXT("ThreatLevel");
const FName UCombatAIBlackboard::CurrentTargetKey = TEXT("CurrentTarget");
const FName UCombatAIBlackboard::PatrolPointKey = TEXT("PatrolPoint");
const FName UCombatAIBlackboard::HomeLocationKey = TEXT("HomeLocation");
const FName UCombatAIBlackboard::PackLeaderKey = TEXT("PackLeader");
const FName UCombatAIBlackboard::PackMembersKey = TEXT("PackMembers");
const FName UCombatAIBlackboard::StaminaKey = TEXT("Stamina");
const FName UCombatAIBlackboard::HealthPercentageKey = TEXT("HealthPercentage");
const FName UCombatAIBlackboard::CanSeePlayerKey = TEXT("CanSeePlayer");
const FName UCombatAIBlackboard::TimeSinceLastSightingKey = TEXT("TimeSinceLastSighting");
const FName UCombatAIBlackboard::IsInTerritoryKey = TEXT("IsInTerritory");
const FName UCombatAIBlackboard::FleeLocationKey = TEXT("FleeLocation");
const FName UCombatAIBlackboard::AttackTargetKey = TEXT("AttackTarget");
const FName UCombatAIBlackboard::IsPackHuntingKey = TEXT("IsPackHunting");
const FName UCombatAIBlackboard::PlayerHasWeaponKey = TEXT("PlayerHasWeapon");
const FName UCombatAIBlackboard::PlayerNearFireKey = TEXT("PlayerNearFire");
const FName UCombatAIBlackboard::HungerLevelKey = TEXT("HungerLevel");
const FName UCombatAIBlackboard::FearLevelKey = TEXT("FearLevel");

UCombatAIBlackboard::UCombatAIBlackboard()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCombatAIBlackboard::BeginPlay()
{
    Super::BeginPlay();
    InitializeKeys();
}

void UCombatAIBlackboard::InitializeKeys()
{
    // Initialize default values for combat AI
    SetValueAsEnum(CombatStateKey, static_cast<uint8>(ECombatAIState::Idle));
    SetValueAsEnum(ThreatLevelKey, static_cast<uint8>(ECombatThreatLevel::None));
    SetValueAsFloat(StaminaKey, 100.0f);
    SetValueAsFloat(HealthPercentageKey, 1.0f);
    SetValueAsBool(CanSeePlayerKey, false);
    SetValueAsFloat(TimeSinceLastSightingKey, 0.0f);
    SetValueAsBool(IsInTerritoryKey, true);
    SetValueAsBool(IsPackHuntingKey, false);
    SetValueAsBool(PlayerHasWeaponKey, false);
    SetValueAsBool(PlayerNearFireKey, false);
    SetValueAsFloat(HungerLevelKey, 0.5f);
    SetValueAsFloat(FearLevelKey, 0.0f);
}

void UCombatAIBlackboard::SetCombatState(ECombatAIState NewState)
{
    SetValueAsEnum(CombatStateKey, static_cast<uint8>(NewState));
}

ECombatAIState UCombatAIBlackboard::GetCombatState() const
{
    return static_cast<ECombatAIState>(GetValueAsEnum(CombatStateKey));
}

void UCombatAIBlackboard::SetThreatLevel(ECombatThreatLevel NewThreatLevel)
{
    SetValueAsEnum(ThreatLevelKey, static_cast<uint8>(NewThreatLevel));
}

ECombatThreatLevel UCombatAIBlackboard::GetThreatLevel() const
{
    return static_cast<ECombatThreatLevel>(GetValueAsEnum(ThreatLevelKey));
}

void UCombatAIBlackboard::UpdatePlayerMemory(AActor* PlayerActor, const FVector& PlayerLocation, bool bCanSeePlayer)
{
    SetValueAsObject(PlayerActorKey, PlayerActor);
    SetValueAsVector(PlayerLocationKey, PlayerLocation);
    SetValueAsBool(CanSeePlayerKey, bCanSeePlayer);
    
    if (bCanSeePlayer)
    {
        SetValueAsVector(LastKnownPlayerLocationKey, PlayerLocation);
        SetValueAsFloat(TimeSinceLastSightingKey, 0.0f);
    }
    else
    {
        float CurrentTime = GetValueAsFloat(TimeSinceLastSightingKey);
        SetValueAsFloat(TimeSinceLastSightingKey, CurrentTime + GetWorld()->GetDeltaSeconds());
    }
}

void UCombatAIBlackboard::SetPackLeader(AActor* LeaderActor)
{
    SetValueAsObject(PackLeaderKey, LeaderActor);
}

void UCombatAIBlackboard::AddPackMember(AActor* MemberActor)
{
    // Note: This would need custom implementation for array handling
    // For now, we'll use a simplified approach
    SetValueAsBool(IsPackHuntingKey, true);
}

void UCombatAIBlackboard::RemovePackMember(AActor* MemberActor)
{
    // Simplified implementation
    // In a full implementation, this would manage an array of pack members
}

bool UCombatAIBlackboard::IsPackHunting() const
{
    return GetValueAsBool(IsPackHuntingKey);
}

float UCombatAIBlackboard::GetDistanceToPlayer() const
{
    AActor* PlayerActor = Cast<AActor>(GetValueAsObject(PlayerActorKey));
    if (PlayerActor && GetOwner())
    {
        return FVector::Dist(GetOwner()->GetActorLocation(), PlayerActor->GetActorLocation());
    }
    return -1.0f;
}

bool UCombatAIBlackboard::ShouldFlee() const
{
    float HealthPercentage = GetValueAsFloat(HealthPercentageKey);
    float FearLevel = GetValueAsFloat(FearLevelKey);
    bool PlayerHasWeapon = GetValueAsBool(PlayerHasWeaponKey);
    bool PlayerNearFire = GetValueAsBool(PlayerNearFireKey);
    
    // Flee if health is low, fear is high, or player has significant advantages
    return (HealthPercentage < 0.3f) || 
           (FearLevel > 0.7f) || 
           (PlayerHasWeapon && PlayerNearFire);
}