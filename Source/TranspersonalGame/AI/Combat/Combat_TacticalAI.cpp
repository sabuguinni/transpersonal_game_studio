#include "Combat_TacticalAI.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

UCombat_TacticalAI::UCombat_TacticalAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
}

void UCombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();
    
    TacticalData.CurrentState = ECombat_TacticalState::Patrol;
    LastStateChangeTime = GetWorld()->GetTimeSeconds();
    
    FindPlayerTarget();
}

void UCombat_TacticalAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!PlayerTarget)
    {
        FindPlayerTarget();
        return;
    }
    
    // Update tactical data
    TacticalData.DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), PlayerTarget->GetActorLocation());
    TacticalData.bHasLineOfSight = HasLineOfSightToPlayer();
    TacticalData.ThreatLevel = CalculateThreatLevel();
    
    if (TacticalData.bHasLineOfSight)
    {
        TacticalData.LastKnownPlayerLocation = PlayerTarget->GetActorLocation();
        TacticalData.TimesSincePlayerSeen = 0.0f;
    }
    else
    {
        TacticalData.TimesSincePlayerSeen += DeltaTime;
    }
    
    // Update tactical state based on conditions
    UpdateTacticalState();
}

void UCombat_TacticalAI::UpdateTacticalState()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastStateChangeTime < StateChangeDelay)
    {
        return; // Prevent rapid state changes
    }
    
    ECombat_TacticalState NewState = TacticalData.CurrentState;
    
    // State machine logic
    switch (TacticalData.CurrentState)
    {
        case ECombat_TacticalState::Patrol:
            if (TacticalData.DistanceToPlayer <= AlertRadius && TacticalData.bHasLineOfSight)
            {
                NewState = ECombat_TacticalState::Alert;
            }
            break;
            
        case ECombat_TacticalState::Alert:
            if (TacticalData.DistanceToPlayer <= AttackRadius)
            {
                NewState = ECombat_TacticalState::Attacking;
            }
            else if (TacticalData.bHasLineOfSight && bCanFlank && TacticalData.DistanceToPlayer > AttackRadius)
            {
                NewState = ECombat_TacticalState::Flanking;
            }
            else if (!TacticalData.bHasLineOfSight && TacticalData.TimesSincePlayerSeen < 5.0f)
            {
                NewState = ECombat_TacticalState::Hunting;
            }
            else if (TacticalData.TimesSincePlayerSeen > 10.0f)
            {
                NewState = ECombat_TacticalState::Patrol;
            }
            break;
            
        case ECombat_TacticalState::Hunting:
            if (TacticalData.bHasLineOfSight)
            {
                NewState = ECombat_TacticalState::Alert;
            }
            else if (TacticalData.TimesSincePlayerSeen > 15.0f)
            {
                NewState = ECombat_TacticalState::Patrol;
            }
            break;
            
        case ECombat_TacticalState::Attacking:
            if (ShouldRetreat())
            {
                NewState = ECombat_TacticalState::Retreating;
            }
            else if (TacticalData.DistanceToPlayer > AttackRadius * 1.5f)
            {
                NewState = ECombat_TacticalState::Alert;
            }
            break;
            
        case ECombat_TacticalState::Flanking:
            if (TacticalData.DistanceToPlayer <= AttackRadius)
            {
                NewState = ECombat_TacticalState::Attacking;
            }
            else if (!TacticalData.bHasLineOfSight)
            {
                NewState = ECombat_TacticalState::Hunting;
            }
            break;
            
        case ECombat_TacticalState::Retreating:
            if (TacticalData.DistanceToPlayer > AlertRadius * 2.0f)
            {
                NewState = ECombat_TacticalState::Patrol;
            }
            break;
            
        case ECombat_TacticalState::Ambush:
            if (TacticalData.DistanceToPlayer <= AttackRadius)
            {
                NewState = ECombat_TacticalState::Attacking;
            }
            break;
    }
    
    if (NewState != TacticalData.CurrentState)
    {
        SetTacticalState(NewState);
    }
}

void UCombat_TacticalAI::SetTacticalState(ECombat_TacticalState NewState)
{
    ECombat_TacticalState OldState = TacticalData.CurrentState;
    TacticalData.CurrentState = NewState;
    LastStateChangeTime = GetWorld()->GetTimeSeconds();
    
    // Log state changes for debugging
    UE_LOG(LogTemp, Log, TEXT("TacticalAI %s: State changed from %d to %d"), 
           *GetOwner()->GetName(), (int32)OldState, (int32)NewState);
    
    // Trigger state-specific behaviors
    switch (NewState)
    {
        case ECombat_TacticalState::Alert:
            OnPlayerDetected(TacticalData.LastKnownPlayerLocation);
            break;
        case ECombat_TacticalState::Patrol:
            OnPlayerLost();
            break;
    }
}

FVector UCombat_TacticalAI::GetFlankingPosition()
{
    if (!PlayerTarget)
    {
        return GetOwner()->GetActorLocation();
    }
    
    FVector PlayerLocation = PlayerTarget->GetActorLocation();
    FVector MyLocation = GetOwner()->GetActorLocation();
    FVector ToPlayer = (PlayerLocation - MyLocation).GetSafeNormal();
    
    // Calculate flanking position 90 degrees to the side
    FVector FlankDirection = FVector::CrossProduct(ToPlayer, FVector::UpVector).GetSafeNormal();
    FVector FlankPosition = PlayerLocation + (FlankDirection * FlankingDistance);
    
    return FlankPosition;
}

bool UCombat_TacticalAI::ShouldRetreat()
{
    // Check if owner has health component and is below threshold
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        // Simple retreat logic based on distance and threat level
        if (TacticalData.ThreatLevel > 8.0f && TacticalData.DistanceToPlayer < AttackRadius * 0.5f)
        {
            return true;
        }
    }
    
    return false;
}

void UCombat_TacticalAI::OnPlayerDetected(FVector PlayerLocation)
{
    TacticalData.LastKnownPlayerLocation = PlayerLocation;
    TacticalData.TimesSincePlayerSeen = 0.0f;
    
    // Broadcast to other AI units nearby
    UE_LOG(LogTemp, Warning, TEXT("TacticalAI %s: Player detected at %s"), 
           *GetOwner()->GetName(), *PlayerLocation.ToString());
}

void UCombat_TacticalAI::OnPlayerLost()
{
    UE_LOG(LogTemp, Log, TEXT("TacticalAI %s: Player lost, returning to patrol"), 
           *GetOwner()->GetName());
}

void UCombat_TacticalAI::FindPlayerTarget()
{
    if (UWorld* World = GetWorld())
    {
        PlayerTarget = UGameplayStatics::GetPlayerPawn(World, 0);
    }
}

bool UCombat_TacticalAI::HasLineOfSightToPlayer()
{
    if (!PlayerTarget)
    {
        return false;
    }
    
    FVector StartLocation = GetOwner()->GetActorLocation();
    FVector EndLocation = PlayerTarget->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(PlayerTarget);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    // Draw debug line in development builds
    #if WITH_EDITOR
    DrawDebugLine(GetWorld(), StartLocation, EndLocation, 
                  bHit ? FColor::Red : FColor::Green, false, 0.1f);
    #endif
    
    return !bHit; // No hit means clear line of sight
}

float UCombat_TacticalAI::CalculateThreatLevel()
{
    float Threat = 0.0f;
    
    // Distance-based threat (closer = more threatening)
    if (TacticalData.DistanceToPlayer < AttackRadius)
    {
        Threat += 5.0f * (1.0f - (TacticalData.DistanceToPlayer / AttackRadius));
    }
    
    // Line of sight increases threat
    if (TacticalData.bHasLineOfSight)
    {
        Threat += 3.0f;
    }
    
    // Recent player sighting increases threat
    if (TacticalData.TimesSincePlayerSeen < 5.0f)
    {
        Threat += 2.0f * (1.0f - (TacticalData.TimesSincePlayerSeen / 5.0f));
    }
    
    return FMath::Clamp(Threat, 0.0f, 10.0f);
}