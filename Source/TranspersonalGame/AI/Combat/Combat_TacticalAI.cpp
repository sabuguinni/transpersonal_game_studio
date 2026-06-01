#include "Combat_TacticalAI.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCombat_TacticalAI::UCombat_TacticalAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    CurrentState = ECombat_TacticalState::Idle;
    CurrentTarget = nullptr;
    StateChangeTimer = 0.0f;
    bIsInCombat = false;
    LastKnownTargetLocation = FVector::ZeroVector;
    
    TacticalData.AttackRange = 500.0f;
    TacticalData.FlankDistance = 800.0f;
    TacticalData.RetreatThreshold = 0.3f;
    TacticalData.AmbushWaitTime = 5.0f;
    TacticalData.bCanFlank = true;
    TacticalData.bCanAmbush = false;
}

void UCombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();
    
    SetTacticalState(ECombat_TacticalState::Patrol);
    
    if (GetOwner())
    {
        UE_LOG(LogTemp, Warning, TEXT("Combat_TacticalAI initialized for %s"), *GetOwner()->GetName());
    }
}

void UCombat_TacticalAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    StateChangeTimer += DeltaTime;
    
    ExecuteTacticalBehavior();
    
    if (CurrentTarget)
    {
        LastKnownTargetLocation = CurrentTarget->GetActorLocation();
    }
}

void UCombat_TacticalAI::SetTacticalState(ECombat_TacticalState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateChangeTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("%s changed tactical state to %d"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), 
               (int32)NewState);
    }
}

void UCombat_TacticalAI::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (NewTarget)
    {
        bIsInCombat = true;
        LastKnownTargetLocation = NewTarget->GetActorLocation();
        
        if (CurrentState == ECombat_TacticalState::Idle || CurrentState == ECombat_TacticalState::Patrol)
        {
            SetTacticalState(ECombat_TacticalState::Hunt);
        }
    }
    else
    {
        bIsInCombat = false;
        SetTacticalState(ECombat_TacticalState::Patrol);
    }
}

bool UCombat_TacticalAI::CanSeeTarget() const
{
    if (!CurrentTarget || !GetOwner())
    {
        return false;
    }
    
    FVector Start = GetOwner()->GetActorLocation();
    FVector End = CurrentTarget->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(CurrentTarget);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult, Start, End, ECC_Visibility, QueryParams
    );
    
    return !bHit;
}

float UCombat_TacticalAI::GetDistanceToTarget() const
{
    if (!CurrentTarget || !GetOwner())
    {
        return -1.0f;
    }
    
    return FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

FVector UCombat_TacticalAI::GetFlankPosition() const
{
    if (!CurrentTarget || !GetOwner())
    {
        return GetOwner()->GetActorLocation();
    }
    
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
    
    FVector RightVector = FVector::CrossProduct(DirectionToTarget, FVector::UpVector);
    FVector FlankPosition = TargetLocation + (RightVector * TacticalData.FlankDistance);
    
    return FlankPosition;
}

bool UCombat_TacticalAI::ShouldRetreat() const
{
    if (!GetOwner())
    {
        return false;
    }
    
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        float CurrentHealth = 100.0f; // Default health
        float MaxHealth = 100.0f;
        
        float HealthPercentage = CurrentHealth / MaxHealth;
        return HealthPercentage < TacticalData.RetreatThreshold;
    }
    
    return false;
}

void UCombat_TacticalAI::ExecuteTacticalBehavior()
{
    switch (CurrentState)
    {
        case ECombat_TacticalState::Idle:
            UpdateIdleState();
            break;
        case ECombat_TacticalState::Patrol:
            UpdatePatrolState();
            break;
        case ECombat_TacticalState::Hunt:
            UpdateHuntState();
            break;
        case ECombat_TacticalState::Attack:
            UpdateAttackState();
            break;
        case ECombat_TacticalState::Retreat:
            UpdateRetreatState();
            break;
        case ECombat_TacticalState::Flank:
            UpdateFlankState();
            break;
        case ECombat_TacticalState::Ambush:
            UpdateAmbushState();
            break;
        case ECombat_TacticalState::Defend:
            UpdateDefendState();
            break;
    }
}

void UCombat_TacticalAI::AddAlliedUnit(AActor* Ally)
{
    if (Ally && !AlliedUnits.Contains(Ally))
    {
        AlliedUnits.Add(Ally);
    }
}

void UCombat_TacticalAI::RemoveAlliedUnit(AActor* Ally)
{
    AlliedUnits.Remove(Ally);
}

TArray<AActor*> UCombat_TacticalAI::GetNearbyAllies(float Radius) const
{
    TArray<AActor*> NearbyAllies;
    
    if (!GetOwner())
    {
        return NearbyAllies;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Ally : AlliedUnits)
    {
        if (Ally && IsValid(Ally))
        {
            float Distance = FVector::Dist(OwnerLocation, Ally->GetActorLocation());
            if (Distance <= Radius)
            {
                NearbyAllies.Add(Ally);
            }
        }
    }
    
    return NearbyAllies;
}

void UCombat_TacticalAI::UpdateIdleState()
{
    if (StateChangeTimer > 2.0f)
    {
        SetTacticalState(ECombat_TacticalState::Patrol);
    }
}

void UCombat_TacticalAI::UpdatePatrolState()
{
    // Look for targets
    if (CurrentTarget)
    {
        SetTacticalState(ECombat_TacticalState::Hunt);
        return;
    }
    
    // Basic patrol behavior
    if (StateChangeTimer > 5.0f)
    {
        StateChangeTimer = 0.0f;
        // Move to random location
    }
}

void UCombat_TacticalAI::UpdateHuntState()
{
    if (!CurrentTarget)
    {
        SetTacticalState(ECombat_TacticalState::Patrol);
        return;
    }
    
    float DistanceToTarget = GetDistanceToTarget();
    
    if (DistanceToTarget <= TacticalData.AttackRange)
    {
        if (TacticalData.bCanFlank && GetNearbyAllies(1500.0f).Num() > 0)
        {
            SetTacticalState(ECombat_TacticalState::Flank);
        }
        else
        {
            SetTacticalState(ECombat_TacticalState::Attack);
        }
    }
    
    if (ShouldRetreat())
    {
        SetTacticalState(ECombat_TacticalState::Retreat);
    }
}

void UCombat_TacticalAI::UpdateAttackState()
{
    if (!CurrentTarget)
    {
        SetTacticalState(ECombat_TacticalState::Patrol);
        return;
    }
    
    if (ShouldRetreat())
    {
        SetTacticalState(ECombat_TacticalState::Retreat);
        return;
    }
    
    float DistanceToTarget = GetDistanceToTarget();
    if (DistanceToTarget > TacticalData.AttackRange * 1.5f)
    {
        SetTacticalState(ECombat_TacticalState::Hunt);
    }
}

void UCombat_TacticalAI::UpdateRetreatState()
{
    if (!ShouldRetreat())
    {
        SetTacticalState(ECombat_TacticalState::Hunt);
    }
    
    // Move away from target
    if (CurrentTarget && GetOwner())
    {
        FVector RetreatDirection = (GetOwner()->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
        // Apply retreat movement
    }
}

void UCombat_TacticalAI::UpdateFlankState()
{
    if (!CurrentTarget)
    {
        SetTacticalState(ECombat_TacticalState::Patrol);
        return;
    }
    
    FVector FlankPos = GetFlankPosition();
    FVector CurrentPos = GetOwner()->GetActorLocation();
    
    if (FVector::Dist(CurrentPos, FlankPos) < 200.0f)
    {
        SetTacticalState(ECombat_TacticalState::Attack);
    }
}

void UCombat_TacticalAI::UpdateAmbushState()
{
    if (StateChangeTimer > TacticalData.AmbushWaitTime)
    {
        SetTacticalState(ECombat_TacticalState::Attack);
    }
}

void UCombat_TacticalAI::UpdateDefendState()
{
    if (!CurrentTarget)
    {
        SetTacticalState(ECombat_TacticalState::Patrol);
        return;
    }
    
    // Defensive positioning logic
    float DistanceToTarget = GetDistanceToTarget();
    if (DistanceToTarget < TacticalData.AttackRange * 0.5f)
    {
        SetTacticalState(ECombat_TacticalState::Attack);
    }
}

bool UCombat_TacticalAI::IsTargetInRange() const
{
    float Distance = GetDistanceToTarget();
    return Distance > 0 && Distance <= TacticalData.AttackRange;
}

bool UCombat_TacticalAI::HasLineOfSight() const
{
    return CanSeeTarget();
}

FVector UCombat_TacticalAI::CalculateOptimalPosition() const
{
    if (!CurrentTarget || !GetOwner())
    {
        return GetOwner()->GetActorLocation();
    }
    
    // Calculate position based on current state and tactical data
    switch (CurrentState)
    {
        case ECombat_TacticalState::Flank:
            return GetFlankPosition();
        case ECombat_TacticalState::Retreat:
            {
                FVector RetreatDirection = (GetOwner()->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
                return GetOwner()->GetActorLocation() + (RetreatDirection * 1000.0f);
            }
        default:
            return CurrentTarget->GetActorLocation();
    }
}

void UCombat_TacticalAI::CoordinateWithAllies()
{
    TArray<AActor*> NearbyAllies = GetNearbyAllies(1500.0f);
    
    for (AActor* Ally : NearbyAllies)
    {
        if (UCombat_TacticalAI* AllyAI = Ally->FindComponentByClass<UCombat_TacticalAI>())
        {
            // Coordinate tactical decisions
            if (AllyAI->CurrentState == ECombat_TacticalState::Attack && CurrentState == ECombat_TacticalState::Hunt)
            {
                if (TacticalData.bCanFlank)
                {
                    SetTacticalState(ECombat_TacticalState::Flank);
                }
            }
        }
    }
}