#include "NPCBehaviorManager.h"
#include "NPC_BaseNPC.h"
#include "NPC_BehaviorComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"

UNPC_BehaviorManager::UNPC_BehaviorManager()
{
    RegisteredNPCs.Empty();
    NPCBehaviorData.Empty();
}

void UNPC_BehaviorManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("NPC Behavior Manager Initialized"));
    
    // Initialize behavior data storage
    RegisteredNPCs.Empty();
    NPCBehaviorData.Empty();
}

void UNPC_BehaviorManager::Deinitialize()
{
    // Clean up all registered NPCs
    RegisteredNPCs.Empty();
    NPCBehaviorData.Empty();
    
    Super::Deinitialize();
}

void UNPC_BehaviorManager::RegisterNPC(ANPC_BaseNPC* NPC)
{
    if (!NPC)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempted to register null NPC"));
        return;
    }

    if (!RegisteredNPCs.Contains(NPC))
    {
        RegisteredNPCs.Add(NPC);
        
        // Initialize behavior data for this NPC
        FNPC_BehaviorData NewBehaviorData;
        NewBehaviorData.CurrentState = ENPC_BehaviorState::Idle;
        NewBehaviorData.PatrolCenter = NPC->GetActorLocation();
        NewBehaviorData.PatrolRadius = 1500.0f;
        NewBehaviorData.StateTimer = 0.0f;
        NewBehaviorData.AlertLevel = 0.0f;
        NewBehaviorData.TargetActor = nullptr;
        
        NPCBehaviorData.Add(NPC, NewBehaviorData);
        
        UE_LOG(LogTemp, Warning, TEXT("Registered NPC: %s"), *NPC->GetName());
    }
}

void UNPC_BehaviorManager::UnregisterNPC(ANPC_BaseNPC* NPC)
{
    if (NPC && RegisteredNPCs.Contains(NPC))
    {
        RegisteredNPCs.Remove(NPC);
        NPCBehaviorData.Remove(NPC);
        
        UE_LOG(LogTemp, Warning, TEXT("Unregistered NPC: %s"), *NPC->GetName());
    }
}

void UNPC_BehaviorManager::UpdateNPCBehaviors(float DeltaTime)
{
    // Update all registered NPCs
    for (ANPC_BaseNPC* NPC : RegisteredNPCs)
    {
        if (!NPC || !IsValid(NPC))
        {
            continue;
        }

        FNPC_BehaviorData* BehaviorData = NPCBehaviorData.Find(NPC);
        if (!BehaviorData)
        {
            continue;
        }

        // Update state timer
        BehaviorData->StateTimer += DeltaTime;

        // Update behavior based on current state
        switch (BehaviorData->CurrentState)
        {
        case ENPC_BehaviorState::Idle:
            UpdateIdleBehavior(NPC, *BehaviorData, DeltaTime);
            break;
        case ENPC_BehaviorState::Patrol:
            UpdatePatrolBehavior(NPC, *BehaviorData, DeltaTime);
            break;
        case ENPC_BehaviorState::Alert:
            UpdateAlertBehavior(NPC, *BehaviorData, DeltaTime);
            break;
        case ENPC_BehaviorState::Flee:
            UpdateFleeBehavior(NPC, *BehaviorData, DeltaTime);
            break;
        case ENPC_BehaviorState::Hunt:
            UpdateHuntBehavior(NPC, *BehaviorData, DeltaTime);
            break;
        case ENPC_BehaviorState::Social:
            UpdateSocialBehavior(NPC, *BehaviorData, DeltaTime);
            break;
        case ENPC_BehaviorState::Rest:
            UpdateRestBehavior(NPC, *BehaviorData, DeltaTime);
            break;
        }

        // Decay alert level over time
        BehaviorData->AlertLevel = FMath::Max(0.0f, BehaviorData->AlertLevel - (DeltaTime * 0.1f));
    }
}

void UNPC_BehaviorManager::SetNPCBehaviorState(ANPC_BaseNPC* NPC, ENPC_BehaviorState NewState)
{
    if (!NPC)
    {
        return;
    }

    FNPC_BehaviorData* BehaviorData = NPCBehaviorData.Find(NPC);
    if (BehaviorData)
    {
        BehaviorData->CurrentState = NewState;
        BehaviorData->StateTimer = 0.0f;
        
        UE_LOG(LogTemp, Warning, TEXT("NPC %s state changed to %d"), *NPC->GetName(), (int32)NewState);
    }
}

ENPC_BehaviorState UNPC_BehaviorManager::GetNPCBehaviorState(ANPC_BaseNPC* NPC) const
{
    if (!NPC)
    {
        return ENPC_BehaviorState::Idle;
    }

    const FNPC_BehaviorData* BehaviorData = NPCBehaviorData.Find(NPC);
    return BehaviorData ? BehaviorData->CurrentState : ENPC_BehaviorState::Idle;
}

void UNPC_BehaviorManager::TriggerAlert(FVector AlertLocation, float AlertRadius, float AlertIntensity)
{
    for (ANPC_BaseNPC* NPC : RegisteredNPCs)
    {
        if (!NPC || !IsValid(NPC))
        {
            continue;
        }

        float DistanceToAlert = FVector::Dist(NPC->GetActorLocation(), AlertLocation);
        if (DistanceToAlert <= AlertRadius)
        {
            ProcessNPCAlert(NPC, AlertLocation, AlertIntensity);
        }
    }
}

void UNPC_BehaviorManager::ProcessNPCAlert(ANPC_BaseNPC* NPC, FVector AlertLocation, float AlertIntensity)
{
    if (!NPC)
    {
        return;
    }

    FNPC_BehaviorData* BehaviorData = NPCBehaviorData.Find(NPC);
    if (!BehaviorData)
    {
        return;
    }

    // Increase alert level
    BehaviorData->AlertLevel = FMath::Min(1.0f, BehaviorData->AlertLevel + AlertIntensity);

    // Change behavior based on alert level
    if (BehaviorData->AlertLevel > 0.7f)
    {
        SetNPCBehaviorState(NPC, ENPC_BehaviorState::Flee);
    }
    else if (BehaviorData->AlertLevel > 0.3f)
    {
        SetNPCBehaviorState(NPC, ENPC_BehaviorState::Alert);
    }
}

TArray<ANPC_BaseNPC*> UNPC_BehaviorManager::GetNearbyNPCs(ANPC_BaseNPC* SourceNPC, float SearchRadius) const
{
    TArray<ANPC_BaseNPC*> NearbyNPCs;
    
    if (!SourceNPC)
    {
        return NearbyNPCs;
    }

    FVector SourceLocation = SourceNPC->GetActorLocation();

    for (ANPC_BaseNPC* NPC : RegisteredNPCs)
    {
        if (NPC && NPC != SourceNPC && IsValid(NPC))
        {
            float Distance = FVector::Dist(SourceLocation, NPC->GetActorLocation());
            if (Distance <= SearchRadius)
            {
                NearbyNPCs.Add(NPC);
            }
        }
    }

    return NearbyNPCs;
}

void UNPC_BehaviorManager::InitiateSocialInteraction(ANPC_BaseNPC* NPC1, ANPC_BaseNPC* NPC2)
{
    if (!NPC1 || !NPC2)
    {
        return;
    }

    // Set both NPCs to social behavior
    SetNPCBehaviorState(NPC1, ENPC_BehaviorState::Social);
    SetNPCBehaviorState(NPC2, ENPC_BehaviorState::Social);

    // Set each other as targets
    FNPC_BehaviorData* BehaviorData1 = NPCBehaviorData.Find(NPC1);
    FNPC_BehaviorData* BehaviorData2 = NPCBehaviorData.Find(NPC2);

    if (BehaviorData1)
    {
        BehaviorData1->TargetActor = NPC2;
    }
    if (BehaviorData2)
    {
        BehaviorData2->TargetActor = NPC1;
    }
}

void UNPC_BehaviorManager::UpdateIdleBehavior(ANPC_BaseNPC* NPC, FNPC_BehaviorData& BehaviorData, float DeltaTime)
{
    // After 3-5 seconds of idle, start patrolling
    if (BehaviorData.StateTimer > FMath::RandRange(3.0f, 5.0f))
    {
        SetNPCBehaviorState(NPC, ENPC_BehaviorState::Patrol);
    }
}

void UNPC_BehaviorManager::UpdatePatrolBehavior(ANPC_BaseNPC* NPC, FNPC_BehaviorData& BehaviorData, float DeltaTime)
{
    // Get a random patrol point and move towards it
    FVector CurrentLocation = NPC->GetActorLocation();
    FVector PatrolTarget = GetRandomPatrolPoint(BehaviorData);
    
    float DistanceToTarget = FVector::Dist(CurrentLocation, PatrolTarget);
    
    // If close to target or patrolling for too long, switch to idle
    if (DistanceToTarget < 200.0f || BehaviorData.StateTimer > 10.0f)
    {
        SetNPCBehaviorState(NPC, ENPC_BehaviorState::Idle);
    }
}

void UNPC_BehaviorManager::UpdateAlertBehavior(ANPC_BaseNPC* NPC, FNPC_BehaviorData& BehaviorData, float DeltaTime)
{
    // Look around, check for threats
    float PlayerDistance = CalculateDistanceToPlayer(NPC);
    
    if (PlayerDistance < 500.0f)
    {
        // Player is close, decide to flee or hunt based on NPC type
        SetNPCBehaviorState(NPC, ENPC_BehaviorState::Flee);
    }
    else if (BehaviorData.StateTimer > 5.0f)
    {
        // Alert timeout, return to patrol
        SetNPCBehaviorState(NPC, ENPC_BehaviorState::Patrol);
    }
}

void UNPC_BehaviorManager::UpdateFleeBehavior(ANPC_BaseNPC* NPC, FNPC_BehaviorData& BehaviorData, float DeltaTime)
{
    // Move away from threats
    float PlayerDistance = CalculateDistanceToPlayer(NPC);
    
    if (PlayerDistance > 2000.0f || BehaviorData.StateTimer > 8.0f)
    {
        // Safe distance reached or flee timeout
        SetNPCBehaviorState(NPC, ENPC_BehaviorState::Idle);
    }
}

void UNPC_BehaviorManager::UpdateHuntBehavior(ANPC_BaseNPC* NPC, FNPC_BehaviorData& BehaviorData, float DeltaTime)
{
    // Aggressive behavior - move towards target
    if (BehaviorData.TargetActor)
    {
        float DistanceToTarget = FVector::Dist(NPC->GetActorLocation(), BehaviorData.TargetActor->GetActorLocation());
        
        if (DistanceToTarget > 3000.0f || BehaviorData.StateTimer > 15.0f)
        {
            // Target too far or hunt timeout
            BehaviorData.TargetActor = nullptr;
            SetNPCBehaviorState(NPC, ENPC_BehaviorState::Patrol);
        }
    }
    else
    {
        // No target, return to patrol
        SetNPCBehaviorState(NPC, ENPC_BehaviorState::Patrol);
    }
}

void UNPC_BehaviorManager::UpdateSocialBehavior(ANPC_BaseNPC* NPC, FNPC_BehaviorData& BehaviorData, float DeltaTime)
{
    // Social interaction with other NPCs
    if (BehaviorData.TargetActor)
    {
        float DistanceToTarget = FVector::Dist(NPC->GetActorLocation(), BehaviorData.TargetActor->GetActorLocation());
        
        if (DistanceToTarget > 1000.0f || BehaviorData.StateTimer > 10.0f)
        {
            // Social interaction ended
            BehaviorData.TargetActor = nullptr;
            SetNPCBehaviorState(NPC, ENPC_BehaviorState::Idle);
        }
    }
    else
    {
        SetNPCBehaviorState(NPC, ENPC_BehaviorState::Idle);
    }
}

void UNPC_BehaviorManager::UpdateRestBehavior(ANPC_BaseNPC* NPC, FNPC_BehaviorData& BehaviorData, float DeltaTime)
{
    // Rest for 5-10 seconds then return to normal behavior
    if (BehaviorData.StateTimer > FMath::RandRange(5.0f, 10.0f))
    {
        SetNPCBehaviorState(NPC, ENPC_BehaviorState::Idle);
    }
}

FVector UNPC_BehaviorManager::GetRandomPatrolPoint(const FNPC_BehaviorData& BehaviorData) const
{
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(BehaviorData.PatrolRadius * 0.3f, BehaviorData.PatrolRadius);
    
    FVector RandomOffset;
    RandomOffset.X = FMath::Cos(RandomAngle) * RandomDistance;
    RandomOffset.Y = FMath::Sin(RandomAngle) * RandomDistance;
    RandomOffset.Z = 0.0f;
    
    return BehaviorData.PatrolCenter + RandomOffset;
}

bool UNPC_BehaviorManager::IsLocationSafe(FVector Location) const
{
    // Simple safety check - avoid water, cliffs, etc.
    // This would need more sophisticated implementation with navigation mesh
    return true;
}

float UNPC_BehaviorManager::CalculateDistanceToPlayer(ANPC_BaseNPC* NPC) const
{
    if (!NPC)
    {
        return 10000.0f;
    }

    UWorld* World = NPC->GetWorld();
    if (!World)
    {
        return 10000.0f;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        return 10000.0f;
    }

    return FVector::Dist(NPC->GetActorLocation(), PlayerPawn->GetActorLocation());
}