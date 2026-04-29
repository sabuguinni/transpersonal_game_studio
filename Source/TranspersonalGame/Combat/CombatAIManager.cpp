#include "CombatAIManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"

UCombatAIManager::UCombatAIManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    // Initialize default values
    ThreatDetectionRadius = 1500.0f;
    CombatUpdateInterval = 0.1f;
    bEnablePackCoordination = true;
    MaxPackSize = 5;
    
    CurrentTarget = nullptr;
    LastUpdateTime = 0.0f;
    StateChangeTime = 0.0f;
    
    // Initialize combat data with safe defaults
    CombatData.CurrentState = ECombat_TacticalState::Idle;
    CombatData.AggressionLevel = 0.5f;
    CombatData.FearThreshold = 0.7f;
    CombatData.TerritoryRadius = 1000.0f;
    CombatData.PreferredRange = ECombat_EngagementRange::Medium;
    CombatData.bIsPackHunter = false;
    CombatData.AttackCooldown = 2.0f;
    CombatData.LastAttackTime = 0.0f;
}

void UCombatAIManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (AActor* Owner = GetOwner())
    {
        InitializeCombatAI(Owner);
        UE_LOG(LogTemp, Log, TEXT("CombatAIManager initialized for %s"), *Owner->GetName());
    }
}

void UCombatAIManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (GetWorld())
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastUpdateTime >= CombatUpdateInterval)
        {
            UpdateTacticalState(DeltaTime);
            LastUpdateTime = CurrentTime;
        }
    }
}

void UCombatAIManager::InitializeCombatAI(AActor* OwnerActor)
{
    if (!OwnerActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Cannot initialize with null owner"));
        return;
    }
    
    // Reset state
    CurrentTarget = nullptr;
    KnownThreats.Empty();
    PackMembers.Empty();
    
    // Set initial state based on actor type
    FString ActorName = OwnerActor->GetName();
    if (ActorName.Contains(TEXT("Raptor")))
    {
        CombatData.bIsPackHunter = true;
        CombatData.AggressionLevel = 0.8f;
        CombatData.PreferredRange = ECombat_EngagementRange::Close;
    }
    else if (ActorName.Contains(TEXT("TRex")))
    {
        CombatData.bIsPackHunter = false;
        CombatData.AggressionLevel = 0.9f;
        CombatData.TerritoryRadius = 2000.0f;
        CombatData.PreferredRange = ECombat_EngagementRange::Close;
    }
    else if (ActorName.Contains(TEXT("Brachio")))
    {
        CombatData.bIsPackHunter = true;
        CombatData.AggressionLevel = 0.2f;
        CombatData.FearThreshold = 0.3f;
        CombatData.PreferredRange = ECombat_EngagementRange::Long;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Combat AI initialized for %s - Aggression: %f, Pack Hunter: %s"), 
           *ActorName, CombatData.AggressionLevel, CombatData.bIsPackHunter ? TEXT("Yes") : TEXT("No"));
}

void UCombatAIManager::UpdateTacticalState(float DeltaTime)
{
    if (!GetOwner() || !GetWorld())
        return;
    
    // Update threat assessment
    UpdateThreatAssessment();
    
    // Process pack coordination if enabled
    if (bEnablePackCoordination && CombatData.bIsPackHunter)
    {
        ProcessPackCoordination();
    }
    
    // Update current state based on threats and conditions
    ECombat_TacticalState NewState = CombatData.CurrentState;
    
    if (KnownThreats.Num() > 0)
    {
        AActor* PrimaryThreat = SelectPrimaryTarget(KnownThreats);
        if (PrimaryThreat)
        {
            float ThreatScore = CalculateThreatScore(PrimaryThreat);
            
            if (ThreatScore > CombatData.FearThreshold)
            {
                NewState = ECombat_TacticalState::Flee;
            }
            else if (ShouldEngageTarget(PrimaryThreat))
            {
                NewState = ECombat_TacticalState::Attack;
                CurrentTarget = PrimaryThreat;
            }
            else
            {
                NewState = ECombat_TacticalState::Alert;
            }
        }
    }
    else
    {
        // No immediate threats - return to patrol or idle
        NewState = ECombat_TacticalState::Patrol;
    }
    
    // Update state if changed
    if (NewState != CombatData.CurrentState)
    {
        CombatData.CurrentState = NewState;
        StateChangeTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Log, TEXT("%s changed combat state to %d"), 
               *GetOwner()->GetName(), (int32)NewState);
    }
}

ECombat_TacticalState UCombatAIManager::EvaluateThreatLevel(AActor* Target)
{
    if (!Target)
        return ECombat_TacticalState::Idle;
    
    float ThreatScore = CalculateThreatScore(Target);
    
    if (ThreatScore > CombatData.FearThreshold)
        return ECombat_TacticalState::Flee;
    else if (ThreatScore > 0.5f)
        return ECombat_TacticalState::Alert;
    else
        return ECombat_TacticalState::Patrol;
}

bool UCombatAIManager::ShouldEngageTarget(AActor* Target)
{
    if (!Target || !GetOwner())
        return false;
    
    // Check if we're on cooldown
    if (GetWorld())
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - CombatData.LastAttackTime < CombatData.AttackCooldown)
            return false;
    }
    
    // Calculate distance
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    
    // Check if target is within engagement range
    float MaxEngagementRange = 0.0f;
    switch (CombatData.PreferredRange)
    {
        case ECombat_EngagementRange::Close:
            MaxEngagementRange = 500.0f;
            break;
        case ECombat_EngagementRange::Medium:
            MaxEngagementRange = 1500.0f;
            break;
        case ECombat_EngagementRange::Long:
            MaxEngagementRange = 3000.0f;
            break;
    }
    
    if (Distance > MaxEngagementRange)
        return false;
    
    // Check aggression vs threat level
    float ThreatScore = CalculateThreatScore(Target);
    return CombatData.AggressionLevel > ThreatScore;
}

FVector UCombatAIManager::CalculateOptimalPosition(AActor* Target)
{
    if (!Target || !GetOwner())
        return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector TargetLocation = Target->GetActorLocation();
    FVector Direction = (TargetLocation - OwnerLocation).GetSafeNormal();
    
    // Calculate optimal distance based on preferred range
    float OptimalDistance = 0.0f;
    switch (CombatData.PreferredRange)
    {
        case ECombat_EngagementRange::Close:
            OptimalDistance = 300.0f;
            break;
        case ECombat_EngagementRange::Medium:
            OptimalDistance = 1000.0f;
            break;
        case ECombat_EngagementRange::Long:
            OptimalDistance = 2000.0f;
            break;
    }
    
    // Add some randomness for more natural behavior
    OptimalDistance += FMath::RandRange(-100.0f, 100.0f);
    
    return TargetLocation - (Direction * OptimalDistance);
}

void UCombatAIManager::ExecuteAttackPattern(AActor* Target)
{
    if (!Target || !GetOwner() || !GetWorld())
        return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check cooldown
    if (CurrentTime - CombatData.LastAttackTime < CombatData.AttackCooldown)
        return;
    
    // Update last attack time
    CombatData.LastAttackTime = CurrentTime;
    
    // Log attack execution
    UE_LOG(LogTemp, Log, TEXT("%s executing attack pattern against %s"), 
           *GetOwner()->GetName(), *Target->GetName());
    
    // For now, just log the attack - actual damage/animation would be handled by other systems
    // This is the tactical decision-making layer
}

void UCombatAIManager::CoordinatePackBehavior(TArray<AActor*> PackMembers)
{
    if (!CombatData.bIsPackHunter || PackMembers.Num() <= 1)
        return;
    
    // Simple pack coordination - assign roles based on position
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        if (AActor* Member = PackMembers[i])
        {
            // Pack leader (first member) engages directly
            // Others flank or support based on index
            UE_LOG(LogTemp, Log, TEXT("Pack member %s assigned role %d"), 
                   *Member->GetName(), i);
        }
    }
}

float UCombatAIManager::CalculateThreatScore(AActor* Target)
{
    if (!Target || !GetOwner())
        return 0.0f;
    
    float ThreatScore = 0.0f;
    
    // Distance factor (closer = more threatening)
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    float DistanceFactor = FMath::Clamp(1.0f - (Distance / ThreatDetectionRadius), 0.0f, 1.0f);
    ThreatScore += DistanceFactor * 0.4f;
    
    // Player factor (players are always high threat)
    if (Target->IsA<APawn>())
    {
        if (Cast<APawn>(Target)->GetController() && Cast<APawn>(Target)->GetController()->IsA<APlayerController>())
        {
            ThreatScore += 0.6f;
        }
    }
    
    // Size/type factor based on name
    FString TargetName = Target->GetName();
    if (TargetName.Contains(TEXT("TRex")))
        ThreatScore += 0.8f;
    else if (TargetName.Contains(TEXT("Raptor")))
        ThreatScore += 0.5f;
    else if (TargetName.Contains(TEXT("Player")) || TargetName.Contains(TEXT("Character")))
        ThreatScore += 0.7f;
    
    return FMath::Clamp(ThreatScore, 0.0f, 1.0f);
}

TArray<AActor*> UCombatAIManager::FindNearbyThreats(float SearchRadius)
{
    TArray<AActor*> Threats;
    
    if (!GetOwner() || !GetWorld())
        return Threats;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Get all actors in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor || Actor == GetOwner())
            continue;
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        if (Distance <= SearchRadius)
        {
            // Check if this actor is a potential threat
            FString ActorName = Actor->GetName();
            if (ActorName.Contains(TEXT("Player")) || 
                ActorName.Contains(TEXT("Character")) ||
                ActorName.Contains(TEXT("Dinosaur")) ||
                ActorName.Contains(TEXT("TRex")) ||
                ActorName.Contains(TEXT("Raptor")))
            {
                Threats.Add(Actor);
            }
        }
    }
    
    return Threats;
}

AActor* UCombatAIManager::SelectPrimaryTarget(TArray<AActor*> PotentialTargets)
{
    if (PotentialTargets.Num() == 0)
        return nullptr;
    
    AActor* BestTarget = nullptr;
    float HighestThreatScore = 0.0f;
    
    for (AActor* Target : PotentialTargets)
    {
        if (!Target)
            continue;
        
        float ThreatScore = CalculateThreatScore(Target);
        if (ThreatScore > HighestThreatScore)
        {
            HighestThreatScore = ThreatScore;
            BestTarget = Target;
        }
    }
    
    return BestTarget;
}

void UCombatAIManager::UpdateThreatAssessment()
{
    if (!GetOwner())
        return;
    
    // Find all nearby threats
    KnownThreats = FindNearbyThreats(ThreatDetectionRadius);
    
    // Remove any null or invalid threats
    KnownThreats.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });
}

void UCombatAIManager::ProcessPackCoordination()
{
    if (!CombatData.bIsPackHunter)
        return;
    
    // Find nearby pack members (same type actors)
    PackMembers.Empty();
    
    if (GetOwner())
    {
        FString OwnerName = GetOwner()->GetName();
        TArray<AActor*> NearbyActors = FindNearbyThreats(CombatData.TerritoryRadius);
        
        for (AActor* Actor : NearbyActors)
        {
            if (Actor && Actor != GetOwner())
            {
                FString ActorName = Actor->GetName();
                // Simple pack detection - same type of dinosaur
                if ((OwnerName.Contains(TEXT("Raptor")) && ActorName.Contains(TEXT("Raptor"))) ||
                    (OwnerName.Contains(TEXT("Brachio")) && ActorName.Contains(TEXT("Brachio"))))
                {
                    PackMembers.Add(Actor);
                    if (PackMembers.Num() >= MaxPackSize)
                        break;
                }
            }
        }
    }
    
    // Coordinate with pack if we have members
    if (PackMembers.Num() > 0)
    {
        CoordinatePackBehavior(PackMembers);
    }
}

bool UCombatAIManager::IsWithinTerritory(FVector Position)
{
    if (!GetOwner())
        return false;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    float Distance = FVector::Dist(OwnerLocation, Position);
    return Distance <= CombatData.TerritoryRadius;
}

FVector UCombatAIManager::GetFleeDirection(AActor* Threat)
{
    if (!Threat || !GetOwner())
        return FVector::ZeroVector;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector ThreatLocation = Threat->GetActorLocation();
    
    // Flee in opposite direction from threat
    FVector FleeDirection = (OwnerLocation - ThreatLocation).GetSafeNormal();
    
    // Add some randomness to avoid predictable patterns
    FVector RandomOffset = FVector(
        FMath::RandRange(-0.3f, 0.3f),
        FMath::RandRange(-0.3f, 0.3f),
        0.0f
    );
    
    return (FleeDirection + RandomOffset).GetSafeNormal();
}