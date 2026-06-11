#include "CombatStateManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"

UCombatStateManager::UCombatStateManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    CurrentCombatState = ECombat_CombatState::Idle;
    PrimaryThreat = nullptr;
}

void UCombatStateManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize combat state
    SetCombatState(ECombat_CombatState::Patrol);
}

void UCombatStateManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateThreatDetection();
    UpdateCombatLogic();
    ProcessStateTransitions();
}

void UCombatStateManager::SetCombatState(ECombat_CombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        ECombat_CombatState OldState = CurrentCombatState;
        CurrentCombatState = NewState;
        
        // Log state change
        UE_LOG(LogTemp, Log, TEXT("Combat State Changed: %s -> %s"), 
            *UEnum::GetValueAsString(OldState), 
            *UEnum::GetValueAsString(NewState));
    }
}

void UCombatStateManager::RegisterThreat(AActor* ThreatActor, ECombat_ThreatLevel Level)
{
    if (!ThreatActor) return;
    
    // Check if threat already exists
    for (FCombat_ThreatData& Threat : KnownThreats)
    {
        if (Threat.ThreatActor == ThreatActor)
        {
            Threat.ThreatLevel = Level;
            Threat.LastSeenTime = GetWorld()->GetTimeSeconds();
            return;
        }
    }
    
    // Add new threat
    FCombat_ThreatData NewThreat;
    NewThreat.ThreatActor = ThreatActor;
    NewThreat.ThreatLevel = Level;
    NewThreat.Distance = FVector::Dist(GetOwner()->GetActorLocation(), ThreatActor->GetActorLocation());
    NewThreat.LastSeenTime = GetWorld()->GetTimeSeconds();
    
    KnownThreats.Add(NewThreat);
    
    UE_LOG(LogTemp, Warning, TEXT("New threat registered: %s"), *ThreatActor->GetName());
}

void UCombatStateManager::RemoveThreat(AActor* ThreatActor)
{
    KnownThreats.RemoveAll([ThreatActor](const FCombat_ThreatData& Threat)
    {
        return Threat.ThreatActor == ThreatActor;
    });
    
    if (PrimaryThreat == ThreatActor)
    {
        PrimaryThreat = nullptr;
    }
}

AActor* UCombatStateManager::GetPrimaryThreat() const
{
    return PrimaryThreat;
}

float UCombatStateManager::GetThreatDistance(AActor* ThreatActor) const
{
    if (!ThreatActor) return -1.0f;
    
    for (const FCombat_ThreatData& Threat : KnownThreats)
    {
        if (Threat.ThreatActor == ThreatActor)
        {
            return Threat.Distance;
        }
    }
    
    return -1.0f;
}

void UCombatStateManager::UpdateThreatDetection()
{
    if (!GetOwner()) return;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update distances for known threats
    for (FCombat_ThreatData& Threat : KnownThreats)
    {
        if (Threat.ThreatActor)
        {
            Threat.Distance = FVector::Dist(OwnerLocation, Threat.ThreatActor->GetActorLocation());
            
            // Remove threats that are too old or too far
            if (CurrentTime - Threat.LastSeenTime > 10.0f || Threat.Distance > DetectionRadius * 2.0f)
            {
                Threat.ThreatActor = nullptr; // Mark for removal
            }
        }
    }
    
    // Clean up invalid threats
    KnownThreats.RemoveAll([](const FCombat_ThreatData& Threat)
    {
        return Threat.ThreatActor == nullptr;
    });
    
    // Find new threats in detection radius
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner()) continue;
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        if (Distance <= DetectionRadius)
        {
            // Check if this is a player or hostile entity
            if (Actor->IsA<APawn>())
            {
                ECombat_ThreatLevel Level = CalculateThreatLevel(Actor, Distance);
                if (Level != ECombat_ThreatLevel::None)
                {
                    RegisterThreat(Actor, Level);
                }
            }
        }
    }
}

void UCombatStateManager::UpdateCombatLogic()
{
    // Find primary threat (closest high-level threat)
    PrimaryThreat = nullptr;
    float ClosestDistance = FLT_MAX;
    ECombat_ThreatLevel HighestThreat = ECombat_ThreatLevel::None;
    
    for (const FCombat_ThreatData& Threat : KnownThreats)
    {
        if (Threat.ThreatActor && (Threat.ThreatLevel > HighestThreat || 
            (Threat.ThreatLevel == HighestThreat && Threat.Distance < ClosestDistance)))
        {
            PrimaryThreat = Threat.ThreatActor;
            ClosestDistance = Threat.Distance;
            HighestThreat = Threat.ThreatLevel;
        }
    }
}

void UCombatStateManager::ProcessStateTransitions()
{
    ECombat_CombatState NewState = CurrentCombatState;
    
    // Get owner health (if available)
    float HealthRatio = 1.0f; // Default to full health
    
    switch (CurrentCombatState)
    {
        case ECombat_CombatState::Idle:
        case ECombat_CombatState::Patrol:
            if (PrimaryThreat)
            {
                NewState = ECombat_CombatState::Alert;
            }
            break;
            
        case ECombat_CombatState::Alert:
            if (!PrimaryThreat)
            {
                NewState = ECombat_CombatState::Patrol;
            }
            else if (GetThreatDistance(PrimaryThreat) <= AttackRange * 2.0f)
            {
                NewState = ECombat_CombatState::Hunting;
            }
            break;
            
        case ECombat_CombatState::Hunting:
            if (!PrimaryThreat)
            {
                NewState = ECombat_CombatState::Patrol;
            }
            else if (GetThreatDistance(PrimaryThreat) <= AttackRange)
            {
                NewState = ECombat_CombatState::Attacking;
            }
            else if (HealthRatio < FleeHealthThreshold)
            {
                NewState = ECombat_CombatState::Fleeing;
            }
            break;
            
        case ECombat_CombatState::Attacking:
            if (!PrimaryThreat)
            {
                NewState = ECombat_CombatState::Patrol;
            }
            else if (GetThreatDistance(PrimaryThreat) > AttackRange * 1.5f)
            {
                NewState = ECombat_CombatState::Hunting;
            }
            else if (HealthRatio < FleeHealthThreshold)
            {
                NewState = ECombat_CombatState::Fleeing;
            }
            break;
            
        case ECombat_CombatState::Fleeing:
            if (!PrimaryThreat || GetThreatDistance(PrimaryThreat) > DetectionRadius)
            {
                NewState = ECombat_CombatState::Patrol;
            }
            break;
            
        case ECombat_CombatState::Dead:
            // Stay dead
            break;
    }
    
    if (NewState != CurrentCombatState)
    {
        SetCombatState(NewState);
    }
}

ECombat_ThreatLevel UCombatStateManager::CalculateThreatLevel(AActor* ThreatActor, float Distance) const
{
    if (!ThreatActor) return ECombat_ThreatLevel::None;
    
    // Basic threat calculation based on distance and actor type
    if (Distance > DetectionRadius) return ECombat_ThreatLevel::None;
    
    // Players are always high threat
    if (ThreatActor->IsA<APawn>() && Cast<APawn>(ThreatActor)->IsPlayerControlled())
    {
        if (Distance < AttackRange) return ECombat_ThreatLevel::Critical;
        if (Distance < AttackRange * 2.0f) return ECombat_ThreatLevel::High;
        return ECombat_ThreatLevel::Medium;
    }
    
    // Other pawns are medium threat
    if (ThreatActor->IsA<APawn>())
    {
        if (Distance < AttackRange) return ECombat_ThreatLevel::High;
        return ECombat_ThreatLevel::Low;
    }
    
    return ECombat_ThreatLevel::None;
}