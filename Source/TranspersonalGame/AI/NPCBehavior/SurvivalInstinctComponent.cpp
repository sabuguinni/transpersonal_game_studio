#include "SurvivalInstinctComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"

UNPC_SurvivalInstinctComponent::UNPC_SurvivalInstinctComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize survival state
    CurrentState = ENPC_SurvivalState::Calm;
    FearLevel = 0.0f;
    HungerLevel = 50.0f;
    ThirstLevel = 50.0f;
    EnergyLevel = 100.0f;
    
    // Threat detection settings
    ThreatDetectionRadius = 1500.0f;
    DinosaurThreatMultiplier = 3.0f;
    PlayerThreatLevel = 1.5f;
    
    // Behavior settings
    FleeSpeed = 600.0f;
    WalkSpeed = 200.0f;
    RestDuration = 10.0f;
    ForageDuration = 15.0f;
    
    // Internal state
    StateChangeTimer = 0.0f;
    LastKnownSafeLocation = FVector::ZeroVector;
    LastThreatCheckTime = 0.0f;
    PrimaryThreat = nullptr;
    bIsInitialized = false;
}

void UNPC_SurvivalInstinctComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize last known safe location to current position
    if (AActor* Owner = GetOwner())
    {
        LastKnownSafeLocation = Owner->GetActorLocation();
        bIsInitialized = true;
    }
}

void UNPC_SurvivalInstinctComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsInitialized)
        return;
    
    // Update survival needs over time
    UpdateNeedsOverTime(DeltaTime);
    
    // Detect threats periodically
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastThreatCheckTime > 0.5f) // Check threats twice per second
    {
        DetectThreats();
        LastThreatCheckTime = CurrentTime;
    }
    
    // Update survival state based on current conditions
    UpdateSurvivalState(DeltaTime);
}

void UNPC_SurvivalInstinctComponent::UpdateSurvivalState(float DeltaTime)
{
    StateChangeTimer += DeltaTime;
    
    ENPC_SurvivalState NewState = CurrentState;
    
    // Check for immediate threats
    if (PrimaryThreat && IsValid(PrimaryThreat))
    {
        float ThreatDistance = FVector::Dist(GetOwner()->GetActorLocation(), PrimaryThreat->GetActorLocation());
        
        if (ThreatDistance < ThreatDetectionRadius * 0.5f) // Close threat
        {
            NewState = ENPC_SurvivalState::Fleeing;
            FearLevel = FMath::Min(100.0f, FearLevel + DeltaTime * 50.0f);
        }
        else if (ThreatDistance < ThreatDetectionRadius) // Distant threat
        {
            NewState = ENPC_SurvivalState::Alert;
            FearLevel = FMath::Min(100.0f, FearLevel + DeltaTime * 20.0f);
        }
    }
    else
    {
        // No immediate threats - check survival needs
        FearLevel = FMath::Max(0.0f, FearLevel - DeltaTime * 10.0f); // Fear decays over time
        
        if (FearLevel > 30.0f)
        {
            NewState = ENPC_SurvivalState::Hiding;
        }
        else if (HungerLevel > 70.0f || ThirstLevel > 70.0f)
        {
            NewState = ENPC_SurvivalState::Foraging;
        }
        else if (EnergyLevel < 30.0f)
        {
            NewState = ENPC_SurvivalState::Resting;
        }
        else
        {
            NewState = ENPC_SurvivalState::Calm;
        }
    }
    
    // State change logic
    if (NewState != CurrentState)
    {
        CurrentState = NewState;
        StateChangeTimer = 0.0f;
        
        // Execute state-specific behavior
        switch (CurrentState)
        {
            case ENPC_SurvivalState::Fleeing:
                StartFleeing(PrimaryThreat);
                break;
            case ENPC_SurvivalState::Foraging:
                StartForaging();
                break;
            case ENPC_SurvivalState::Resting:
                StartResting();
                break;
            default:
                break;
        }
    }
}

void UNPC_SurvivalInstinctComponent::DetectThreats()
{
    if (!GetOwner())
        return;
    
    DetectedThreats.Empty();
    PrimaryThreat = nullptr;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    UWorld* World = GetWorld();
    
    if (!World)
        return;
    
    // Get all actors within threat detection radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), NearbyActors);
    
    float HighestThreatScore = 0.0f;
    
    for (AActor* Actor : NearbyActors)
    {
        if (!Actor || Actor == GetOwner())
            continue;
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        
        if (Distance > ThreatDetectionRadius)
            continue;
        
        if (IsThreateningActor(Actor))
        {
            FNPC_ThreatData ThreatData;
            ThreatData.ThreatActor = Actor;
            ThreatData.Distance = Distance;
            ThreatData.LastSeenTime = World->GetTimeSeconds();
            
            // Calculate threat level based on actor type and distance
            float ThreatScore = 1.0f;
            
            if (Actor->GetName().Contains("Dinosaur") || Actor->GetName().Contains("Rex") || Actor->GetName().Contains("Raptor"))
            {
                ThreatScore = DinosaurThreatMultiplier;
                ThreatData.ThreatLevel = ENPC_ThreatLevel::Extreme;
            }
            else if (Actor->IsA<APawn>()) // Player or other pawns
            {
                ThreatScore = PlayerThreatLevel;
                ThreatData.ThreatLevel = ENPC_ThreatLevel::Medium;
            }
            
            // Closer threats are more dangerous
            ThreatScore *= (ThreatDetectionRadius - Distance) / ThreatDetectionRadius;
            
            DetectedThreats.Add(ThreatData);
            
            if (ThreatScore > HighestThreatScore)
            {
                HighestThreatScore = ThreatScore;
                PrimaryThreat = Actor;
            }
        }
    }
}

void UNPC_SurvivalInstinctComponent::ProcessThreat(AActor* ThreatActor)
{
    if (!ThreatActor || !GetOwner())
        return;
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), ThreatActor->GetActorLocation());
    
    if (Distance < ThreatDetectionRadius * 0.3f) // Very close threat
    {
        StartFleeing(ThreatActor);
    }
    else if (Distance < ThreatDetectionRadius * 0.7f) // Medium distance threat
    {
        CurrentState = ENPC_SurvivalState::Alert;
        FearLevel += 20.0f;
    }
}

void UNPC_SurvivalInstinctComponent::StartFleeing(AActor* ThreatSource)
{
    if (!GetOwner() || !ThreatSource)
        return;
    
    CurrentState = ENPC_SurvivalState::Fleeing;
    FearLevel = FMath::Min(100.0f, FearLevel + 30.0f);
    
    // Calculate flee direction
    FVector FleeDirection = GetFleeDirection(ThreatSource);
    
    // If the owner is a pawn, try to move it
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        if (OwnerPawn->GetMovementComponent())
        {
            FVector FleeLocation = GetOwner()->GetActorLocation() + (FleeDirection * 1000.0f);
            
            // Simple movement - in a real implementation, this would use AI movement
            FVector CurrentLocation = GetOwner()->GetActorLocation();
            FVector NewLocation = FMath::VInterpTo(CurrentLocation, FleeLocation, GetWorld()->GetDeltaSeconds(), 2.0f);
            GetOwner()->SetActorLocation(NewLocation);
        }
    }
}

void UNPC_SurvivalInstinctComponent::StartForaging()
{
    CurrentState = ENPC_SurvivalState::Foraging;
    StateChangeTimer = 0.0f;
    
    // Reduce hunger and thirst over time while foraging
    // This is a simple implementation - real foraging would involve finding food sources
}

void UNPC_SurvivalInstinctComponent::StartResting()
{
    CurrentState = ENPC_SurvivalState::Resting;
    StateChangeTimer = 0.0f;
    
    // Restore energy while resting
    // In a real implementation, NPCs would find safe spots to rest
}

bool UNPC_SurvivalInstinctComponent::IsThreateningActor(AActor* Actor)
{
    if (!Actor)
        return false;
    
    FString ActorName = Actor->GetName();
    
    // Dinosaurs are always threatening
    if (ActorName.Contains("Dinosaur") || ActorName.Contains("Rex") || ActorName.Contains("Raptor") || ActorName.Contains("TRex"))
    {
        return true;
    }
    
    // Players can be threatening
    if (Actor->IsA<APawn>() && ActorName.Contains("Character"))
    {
        return true;
    }
    
    return false;
}

FVector UNPC_SurvivalInstinctComponent::GetFleeDirection(AActor* ThreatSource)
{
    if (!GetOwner() || !ThreatSource)
        return FVector::ForwardVector;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector ThreatLocation = ThreatSource->GetActorLocation();
    
    // Calculate direction away from threat
    FVector FleeDirection = (OwnerLocation - ThreatLocation).GetSafeNormal();
    
    // Add some randomness to avoid predictable movement
    FVector RandomOffset = FVector(
        FMath::RandRange(-0.3f, 0.3f),
        FMath::RandRange(-0.3f, 0.3f),
        0.0f
    );
    
    FleeDirection += RandomOffset;
    FleeDirection.Normalize();
    
    return FleeDirection;
}

void UNPC_SurvivalInstinctComponent::UpdateNeedsOverTime(float DeltaTime)
{
    // Survival needs increase over time
    HungerLevel += DeltaTime * 2.0f;  // Gets hungry over time
    ThirstLevel += DeltaTime * 3.0f;  // Gets thirsty faster than hungry
    
    // Energy decreases when active, increases when resting
    if (CurrentState == ENPC_SurvivalState::Resting)
    {
        EnergyLevel = FMath::Min(100.0f, EnergyLevel + DeltaTime * 20.0f);
    }
    else if (CurrentState == ENPC_SurvivalState::Fleeing)
    {
        EnergyLevel = FMath::Max(0.0f, EnergyLevel - DeltaTime * 30.0f); // Fleeing is exhausting
    }
    else
    {
        EnergyLevel = FMath::Max(0.0f, EnergyLevel - DeltaTime * 5.0f); // Normal energy drain
    }
    
    // When foraging, reduce hunger and thirst
    if (CurrentState == ENPC_SurvivalState::Foraging)
    {
        HungerLevel = FMath::Max(0.0f, HungerLevel - DeltaTime * 10.0f);
        ThirstLevel = FMath::Max(0.0f, ThirstLevel - DeltaTime * 8.0f);
    }
    
    // Clamp values
    HungerLevel = FMath::Clamp(HungerLevel, 0.0f, 100.0f);
    ThirstLevel = FMath::Clamp(ThirstLevel, 0.0f, 100.0f);
    EnergyLevel = FMath::Clamp(EnergyLevel, 0.0f, 100.0f);
    FearLevel = FMath::Clamp(FearLevel, 0.0f, 100.0f);
}