#include "Combat_BehaviorManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/PrimitiveComponent.h"

UCombat_BehaviorManager::UCombat_BehaviorManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for responsive AI

    // Initialize default values
    CurrentBehaviorState = ECombat_BehaviorState::Idle;
    PreviousBehaviorState = ECombat_BehaviorState::Idle;
    
    ThreatDetectionRadius = 2000.0f;
    ThreatForgetTime = 10.0f;
    AttackRange = 300.0f;
    FleeThreshold = 0.3f; // Flee when health below 30%
    AggressionLevel = 0.5f;
    bCanFormPacks = true;
    bIsTerritorial = false;

    StateChangeTime = 0.0f;
    LastThreatUpdateTime = 0.0f;
    LastPackUpdateTime = 0.0f;
}

void UCombat_BehaviorManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize component
    SetBehaviorState(ECombat_BehaviorState::Idle);
    
    UE_LOG(LogTemp, Log, TEXT("Combat_BehaviorManager initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCombat_BehaviorManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetOwner())
    {
        return;
    }

    // Update threat assessment
    UpdateThreatAssessment(DeltaTime);
    
    // Update pack coordination if in a pack
    if (IsInPack())
    {
        UpdatePackCoordination(DeltaTime);
    }
    
    // Process behavior state transitions
    ProcessBehaviorTransitions(DeltaTime);
}

void UCombat_BehaviorManager::SetBehaviorState(ECombat_BehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        PreviousBehaviorState = CurrentBehaviorState;
        CurrentBehaviorState = NewState;
        StateChangeTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("%s: Behavior state changed from %d to %d"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
               (int32)PreviousBehaviorState, (int32)CurrentBehaviorState);
    }
}

void UCombat_BehaviorManager::RegisterThreat(AActor* ThreatActor, ECombat_ThreatLevel ThreatLevel)
{
    if (!ThreatActor || !IsValidThreat(ThreatActor))
    {
        return;
    }

    // Check if threat already exists
    for (FCombat_ThreatData& ThreatData : KnownThreats)
    {
        if (ThreatData.ThreatActor == ThreatActor)
        {
            // Update existing threat
            ThreatData.ThreatLevel = ThreatLevel;
            ThreatData.LastSeenTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
            ThreatData.LastKnownPosition = ThreatActor->GetActorLocation();
            ThreatData.Distance = FVector::Dist(GetOwner()->GetActorLocation(), ThreatActor->GetActorLocation());
            return;
        }
    }

    // Add new threat
    FCombat_ThreatData NewThreat;
    NewThreat.ThreatActor = ThreatActor;
    NewThreat.ThreatLevel = ThreatLevel;
    NewThreat.LastSeenTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    NewThreat.LastKnownPosition = ThreatActor->GetActorLocation();
    NewThreat.Distance = FVector::Dist(GetOwner()->GetActorLocation(), ThreatActor->GetActorLocation());
    
    KnownThreats.Add(NewThreat);
    
    UE_LOG(LogTemp, Log, TEXT("%s: Registered new threat %s with level %d"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
           *ThreatActor->GetName(), (int32)ThreatLevel);
}

void UCombat_BehaviorManager::UpdateThreatAssessment(float DeltaTime)
{
    if (!GetOwner() || !GetWorld())
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update existing threats
    for (int32 i = KnownThreats.Num() - 1; i >= 0; i--)
    {
        FCombat_ThreatData& ThreatData = KnownThreats[i];
        
        // Remove invalid or forgotten threats
        if (!IsValid(ThreatData.ThreatActor) || 
            (CurrentTime - ThreatData.LastSeenTime) > ThreatForgetTime)
        {
            KnownThreats.RemoveAt(i);
            continue;
        }
        
        // Update distance
        ThreatData.Distance = FVector::Dist(GetOwner()->GetActorLocation(), 
                                          ThreatData.ThreatActor->GetActorLocation());
        
        // Update last known position if we can still see the threat
        if (ThreatData.Distance <= ThreatDetectionRadius)
        {
            ThreatData.LastKnownPosition = ThreatData.ThreatActor->GetActorLocation();
            ThreatData.LastSeenTime = CurrentTime;
        }
    }
    
    LastThreatUpdateTime = CurrentTime;
}

FCombat_ThreatData UCombat_BehaviorManager::GetHighestThreat() const
{
    FCombat_ThreatData HighestThreat;
    
    for (const FCombat_ThreatData& ThreatData : KnownThreats)
    {
        if ((int32)ThreatData.ThreatLevel > (int32)HighestThreat.ThreatLevel ||
            (ThreatData.ThreatLevel == HighestThreat.ThreatLevel && ThreatData.Distance < HighestThreat.Distance))
        {
            HighestThreat = ThreatData;
        }
    }
    
    return HighestThreat;
}

void UCombat_BehaviorManager::ClearThreats()
{
    KnownThreats.Empty();
    UE_LOG(LogTemp, Log, TEXT("%s: All threats cleared"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCombat_BehaviorManager::JoinPack(AActor* PackLeader)
{
    if (!PackLeader || PackLeader == GetOwner())
    {
        return;
    }

    PackData.PackLeader = PackLeader;
    PackData.PackMembers.AddUnique(GetOwner());
    
    UE_LOG(LogTemp, Log, TEXT("%s: Joined pack led by %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
           *PackLeader->GetName());
}

void UCombat_BehaviorManager::LeavePack()
{
    if (PackData.PackLeader)
    {
        UE_LOG(LogTemp, Log, TEXT("%s: Left pack led by %s"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
               *PackData.PackLeader->GetName());
    }
    
    PackData.PackLeader = nullptr;
    PackData.PackMembers.Empty();
    PackData.bIsHunting = false;
}

void UCombat_BehaviorManager::UpdatePackCoordination(float DeltaTime)
{
    if (!IsInPack() || !GetWorld())
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update pack center based on member positions
    FVector CenterSum = FVector::ZeroVector;
    int32 ValidMembers = 0;
    
    for (AActor* Member : PackData.PackMembers)
    {
        if (IsValid(Member))
        {
            CenterSum += Member->GetActorLocation();
            ValidMembers++;
        }
    }
    
    if (ValidMembers > 0)
    {
        PackData.PackCenter = CenterSum / ValidMembers;
    }
    
    LastPackUpdateTime = CurrentTime;
}

FVector UCombat_BehaviorManager::CalculateOptimalAttackPosition(AActor* Target)
{
    if (!Target || !GetOwner())
    {
        return FVector::ZeroVector;
    }

    FVector TargetLocation = Target->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Calculate direction to target
    FVector DirectionToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
    
    // Position slightly closer than attack range for optimal engagement
    float OptimalDistance = AttackRange * 0.8f;
    FVector OptimalPosition = TargetLocation - (DirectionToTarget * OptimalDistance);
    
    return OptimalPosition;
}

bool UCombat_BehaviorManager::ShouldEngageTarget(AActor* Target)
{
    if (!Target || !GetOwner())
    {
        return false;
    }

    float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    
    // Don't engage if target is too far
    if (DistanceToTarget > ThreatDetectionRadius)
    {
        return false;
    }
    
    // Consider aggression level and pack support
    float EngagementThreshold = AggressionLevel;
    if (IsInPack())
    {
        EngagementThreshold += 0.3f; // More likely to engage with pack support
    }
    
    return EngagementThreshold > 0.5f;
}

bool UCombat_BehaviorManager::ShouldFleeFromThreat(const FCombat_ThreatData& ThreatData)
{
    if (!ThreatData.ThreatActor)
    {
        return false;
    }

    // Flee if threat level is critical or we're heavily damaged
    if (ThreatData.ThreatLevel == ECombat_ThreatLevel::Critical)
    {
        return true;
    }
    
    // Consider health status (would need health component integration)
    // For now, use aggression level as proxy for willingness to fight
    return AggressionLevel < FleeThreshold;
}

FVector UCombat_BehaviorManager::GetFlankingPosition(AActor* Target, bool bLeftFlank)
{
    if (!Target || !GetOwner())
    {
        return FVector::ZeroVector;
    }

    FVector TargetLocation = Target->GetActorLocation();
    FVector TargetForward = Target->GetActorForwardVector();
    
    // Calculate perpendicular direction for flanking
    FVector FlankDirection = FVector::CrossProduct(TargetForward, FVector::UpVector);
    if (!bLeftFlank)
    {
        FlankDirection *= -1.0f;
    }
    
    // Position at flanking distance
    float FlankDistance = AttackRange * 1.2f;
    FVector FlankPosition = TargetLocation + (FlankDirection * FlankDistance);
    
    return FlankPosition;
}

FVector UCombat_BehaviorManager::GetAmbushPosition(AActor* Target)
{
    if (!Target || !GetOwner())
    {
        return FVector::ZeroVector;
    }

    FVector TargetLocation = Target->GetActorLocation();
    FVector TargetForward = Target->GetActorForwardVector();
    
    // Position behind target for ambush
    FVector AmbushPosition = TargetLocation - (TargetForward * AttackRange * 0.5f);
    
    return AmbushPosition;
}

void UCombat_BehaviorManager::ExecuteTacticalManeuver(const FString& ManeuverType)
{
    UE_LOG(LogTemp, Log, TEXT("%s: Executing tactical maneuver: %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
           *ManeuverType);
    
    if (ManeuverType == "Flank")
    {
        SetBehaviorState(ECombat_BehaviorState::Hunting);
    }
    else if (ManeuverType == "Ambush")
    {
        SetBehaviorState(ECombat_BehaviorState::Alert);
    }
    else if (ManeuverType == "Retreat")
    {
        SetBehaviorState(ECombat_BehaviorState::Fleeing);
    }
    else if (ManeuverType == "PackHunt")
    {
        if (IsInPack())
        {
            PackData.bIsHunting = true;
            SetBehaviorState(ECombat_BehaviorState::Hunting);
        }
    }
}

void UCombat_BehaviorManager::ProcessBehaviorTransitions(float DeltaTime)
{
    if (!GetOwner() || !GetWorld())
    {
        return;
    }

    FCombat_ThreatData HighestThreat = GetHighestThreat();
    
    switch (CurrentBehaviorState)
    {
        case ECombat_BehaviorState::Idle:
            if (HighestThreat.ThreatActor)
            {
                if (HighestThreat.ThreatLevel >= ECombat_ThreatLevel::Medium)
                {
                    SetBehaviorState(ECombat_BehaviorState::Alert);
                }
                else
                {
                    SetBehaviorState(ECombat_BehaviorState::Patrol);
                }
            }
            break;
            
        case ECombat_BehaviorState::Patrol:
            if (HighestThreat.ThreatActor)
            {
                if (HighestThreat.Distance <= AttackRange && ShouldEngageTarget(HighestThreat.ThreatActor))
                {
                    SetBehaviorState(ECombat_BehaviorState::Attacking);
                }
                else if (HighestThreat.ThreatLevel >= ECombat_ThreatLevel::Medium)
                {
                    SetBehaviorState(ECombat_BehaviorState::Hunting);
                }
            }
            break;
            
        case ECombat_BehaviorState::Alert:
            if (HighestThreat.ThreatActor)
            {
                if (ShouldFleeFromThreat(HighestThreat))
                {
                    SetBehaviorState(ECombat_BehaviorState::Fleeing);
                }
                else if (ShouldEngageTarget(HighestThreat.ThreatActor))
                {
                    SetBehaviorState(ECombat_BehaviorState::Hunting);
                }
            }
            else
            {
                SetBehaviorState(ECombat_BehaviorState::Patrol);
            }
            break;
            
        case ECombat_BehaviorState::Hunting:
            if (HighestThreat.ThreatActor)
            {
                if (HighestThreat.Distance <= AttackRange)
                {
                    SetBehaviorState(ECombat_BehaviorState::Attacking);
                }
                else if (ShouldFleeFromThreat(HighestThreat))
                {
                    SetBehaviorState(ECombat_BehaviorState::Fleeing);
                }
            }
            else
            {
                SetBehaviorState(ECombat_BehaviorState::Patrol);
            }
            break;
            
        case ECombat_BehaviorState::Attacking:
            if (!HighestThreat.ThreatActor || HighestThreat.Distance > AttackRange * 1.5f)
            {
                SetBehaviorState(ECombat_BehaviorState::Hunting);
            }
            else if (ShouldFleeFromThreat(HighestThreat))
            {
                SetBehaviorState(ECombat_BehaviorState::Fleeing);
            }
            break;
            
        case ECombat_BehaviorState::Fleeing:
            if (!HighestThreat.ThreatActor || HighestThreat.Distance > ThreatDetectionRadius * 2.0f)
            {
                SetBehaviorState(ECombat_BehaviorState::Idle);
            }
            break;
            
        default:
            break;
    }
}

void UCombat_BehaviorManager::UpdateThreatPositions()
{
    for (FCombat_ThreatData& ThreatData : KnownThreats)
    {
        if (IsValid(ThreatData.ThreatActor))
        {
            ThreatData.LastKnownPosition = ThreatData.ThreatActor->GetActorLocation();
            ThreatData.Distance = FVector::Dist(GetOwner()->GetActorLocation(), 
                                              ThreatData.ThreatActor->GetActorLocation());
        }
    }
}

void UCombat_BehaviorManager::CoordinatePackMovement()
{
    if (!IsInPack() || !PackData.PackLeader)
    {
        return;
    }

    // Pack coordination logic would be implemented here
    // For now, just update pack center
    UpdatePackCoordination(0.0f);
}

ECombat_ThreatLevel UCombat_BehaviorManager::AssessThreatLevel(AActor* ThreatActor)
{
    if (!ThreatActor)
    {
        return ECombat_ThreatLevel::None;
    }

    // Basic threat assessment based on distance and actor type
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), ThreatActor->GetActorLocation());
    
    if (Distance > ThreatDetectionRadius)
    {
        return ECombat_ThreatLevel::None;
    }
    else if (Distance > ThreatDetectionRadius * 0.7f)
    {
        return ECombat_ThreatLevel::Low;
    }
    else if (Distance > AttackRange * 2.0f)
    {
        return ECombat_ThreatLevel::Medium;
    }
    else if (Distance > AttackRange)
    {
        return ECombat_ThreatLevel::High;
    }
    else
    {
        return ECombat_ThreatLevel::Critical;
    }
}

bool UCombat_BehaviorManager::IsValidThreat(AActor* Actor)
{
    if (!Actor || Actor == GetOwner())
    {
        return false;
    }

    // Check if actor is a valid threat (pawn, character, etc.)
    return Actor->IsA<APawn>();
}