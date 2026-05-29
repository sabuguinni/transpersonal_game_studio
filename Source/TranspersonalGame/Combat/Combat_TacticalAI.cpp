#include "Combat_TacticalAI.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

UCombat_TacticalAI::UCombat_TacticalAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    CurrentState = ECombat_TacticalState::Idle;
    DinosaurType = ECombat_DinosaurType::Predator;
    CurrentTarget = nullptr;
    DistanceToTarget = 0.0f;
    CurrentHealth = 100.0f;
    MaxHealth = 100.0f;
    LastStateChangeTime = 0.0f;
    StateChangeDelay = 2.0f;
    bHasLineOfSight = false;

    TacticalData.DetectionRange = 2000.0f;
    TacticalData.AttackRange = 500.0f;
    TacticalData.RetreatHealthThreshold = 0.3f;
    TacticalData.AggressionLevel = 0.7f;
    TacticalData.bCanPackHunt = false;
}

void UCombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();
    
    LastStateChangeTime = GetWorld()->GetTimeSeconds();
    CurrentHealth = MaxHealth;
    
    UE_LOG(LogTemp, Warning, TEXT("Combat_TacticalAI: BeginPlay - Component initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCombat_TacticalAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetOwner())
        return;

    // Update target and distance
    if (!CurrentTarget)
    {
        CurrentTarget = FindNearestPlayer();
    }

    if (CurrentTarget)
    {
        UpdateTargetDistance();
    }

    // Update health status
    EvaluateHealthStatus();

    // Execute tactical behavior based on current state
    UpdateTacticalBehavior();
}

void UCombat_TacticalAI::SetTacticalState(ECombat_TacticalState NewState)
{
    if (CurrentState != NewState)
    {
        ECombat_TacticalState OldState = CurrentState;
        CurrentState = NewState;
        LastStateChangeTime = GetWorld()->GetTimeSeconds();

        UE_LOG(LogTemp, Warning, TEXT("Combat_TacticalAI: State changed from %d to %d for %s"), 
               (int32)OldState, (int32)NewState, *GetOwner()->GetName());
    }
}

AActor* UCombat_TacticalAI::FindNearestPlayer()
{
    if (!GetWorld())
        return nullptr;

    AActor* NearestPlayer = nullptr;
    float NearestDistance = TacticalData.DetectionRange;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor != GetOwner())
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestPlayer = Actor;
            }
        }
    }

    return NearestPlayer;
}

bool UCombat_TacticalAI::IsPlayerInRange(float Range)
{
    if (!CurrentTarget)
        return false;

    return DistanceToTarget <= Range;
}

void UCombat_TacticalAI::UpdateTacticalBehavior()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Prevent rapid state changes
    if (CurrentTime - LastStateChangeTime < StateChangeDelay)
        return;

    switch (CurrentState)
    {
        case ECombat_TacticalState::Idle:
            HandleIdleState();
            break;
        case ECombat_TacticalState::Hunting:
            HandleHuntingState();
            break;
        case ECombat_TacticalState::Stalking:
            HandleStalkingState();
            break;
        case ECombat_TacticalState::Attacking:
            HandleAttackingState();
            break;
        case ECombat_TacticalState::Retreating:
            HandleRetreatingState();
            break;
        case ECombat_TacticalState::Circling:
            HandleCirclingState();
            break;
    }
}

void UCombat_TacticalAI::ExecuteAttackPattern()
{
    if (!CurrentTarget || !GetOwner())
        return;

    AAIController* AIController = Cast<AAIController>(GetOwner()->GetInstigatorController());
    if (AIController)
    {
        AIController->MoveToActor(CurrentTarget, TacticalData.AttackRange * 0.8f);
        UE_LOG(LogTemp, Warning, TEXT("Combat_TacticalAI: Executing attack pattern on %s"), *CurrentTarget->GetName());
    }
}

void UCombat_TacticalAI::ExecuteRetreatPattern()
{
    if (!CurrentTarget || !GetOwner())
        return;

    FVector RetreatDirection = GetOwner()->GetActorLocation() - CurrentTarget->GetActorLocation();
    RetreatDirection.Normalize();
    FVector RetreatLocation = GetOwner()->GetActorLocation() + (RetreatDirection * 1000.0f);

    AAIController* AIController = Cast<AAIController>(GetOwner()->GetInstigatorController());
    if (AIController)
    {
        AIController->MoveToLocation(RetreatLocation);
        UE_LOG(LogTemp, Warning, TEXT("Combat_TacticalAI: Executing retreat pattern"));
    }
}

void UCombat_TacticalAI::ExecuteCirclingPattern()
{
    if (!CurrentTarget || !GetOwner())
        return;

    FVector ToTarget = CurrentTarget->GetActorLocation() - GetOwner()->GetActorLocation();
    ToTarget.Normalize();
    
    // Create perpendicular vector for circling
    FVector CircleDirection = FVector::CrossProduct(ToTarget, FVector::UpVector);
    FVector CircleLocation = GetOwner()->GetActorLocation() + (CircleDirection * 800.0f);

    AAIController* AIController = Cast<AAIController>(GetOwner()->GetInstigatorController());
    if (AIController)
    {
        AIController->MoveToLocation(CircleLocation);
        UE_LOG(LogTemp, Warning, TEXT("Combat_TacticalAI: Executing circling pattern"));
    }
}

void UCombat_TacticalAI::UpdateTargetDistance()
{
    if (CurrentTarget && GetOwner())
    {
        DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    }
    else
    {
        DistanceToTarget = 0.0f;
    }
}

void UCombat_TacticalAI::EvaluateHealthStatus()
{
    // In a real implementation, this would get health from a health component
    // For now, we'll simulate health degradation in combat
    if (CurrentState == ECombat_TacticalState::Attacking && CurrentTarget)
    {
        CurrentHealth = FMath::Max(0.0f, CurrentHealth - (GetWorld()->GetDeltaSeconds() * 2.0f));
    }
}

bool UCombat_TacticalAI::ShouldRetreat()
{
    float HealthRatio = CurrentHealth / MaxHealth;
    return HealthRatio <= TacticalData.RetreatHealthThreshold;
}

bool UCombat_TacticalAI::ShouldAttack()
{
    return CurrentTarget && IsPlayerInRange(TacticalData.AttackRange) && !ShouldRetreat();
}

void UCombat_TacticalAI::HandleIdleState()
{
    if (CurrentTarget && IsPlayerInRange(TacticalData.DetectionRange))
    {
        SetTacticalState(ECombat_TacticalState::Hunting);
    }
}

void UCombat_TacticalAI::HandleHuntingState()
{
    if (!CurrentTarget)
    {
        SetTacticalState(ECombat_TacticalState::Idle);
        return;
    }

    if (ShouldRetreat())
    {
        SetTacticalState(ECombat_TacticalState::Retreating);
        return;
    }

    if (IsPlayerInRange(TacticalData.AttackRange * 1.5f))
    {
        SetTacticalState(ECombat_TacticalState::Stalking);
    }
    else if (IsPlayerInRange(TacticalData.DetectionRange))
    {
        ExecuteAttackPattern();
    }
    else
    {
        SetTacticalState(ECombat_TacticalState::Idle);
    }
}

void UCombat_TacticalAI::HandleStalkingState()
{
    if (!CurrentTarget)
    {
        SetTacticalState(ECombat_TacticalState::Idle);
        return;
    }

    if (ShouldRetreat())
    {
        SetTacticalState(ECombat_TacticalState::Retreating);
        return;
    }

    if (ShouldAttack())
    {
        SetTacticalState(ECombat_TacticalState::Attacking);
    }
    else if (TacticalData.AggressionLevel > 0.5f)
    {
        SetTacticalState(ECombat_TacticalState::Circling);
    }
}

void UCombat_TacticalAI::HandleAttackingState()
{
    if (!CurrentTarget)
    {
        SetTacticalState(ECombat_TacticalState::Idle);
        return;
    }

    if (ShouldRetreat())
    {
        SetTacticalState(ECombat_TacticalState::Retreating);
        return;
    }

    ExecuteAttackPattern();

    if (!IsPlayerInRange(TacticalData.AttackRange * 2.0f))
    {
        SetTacticalState(ECombat_TacticalState::Hunting);
    }
}

void UCombat_TacticalAI::HandleRetreatingState()
{
    ExecuteRetreatPattern();

    if (!IsPlayerInRange(TacticalData.DetectionRange * 1.5f))
    {
        CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + 10.0f); // Simulate healing
        SetTacticalState(ECombat_TacticalState::Idle);
    }
}

void UCombat_TacticalAI::HandleCirclingState()
{
    if (!CurrentTarget)
    {
        SetTacticalState(ECombat_TacticalState::Idle);
        return;
    }

    if (ShouldRetreat())
    {
        SetTacticalState(ECombat_TacticalState::Retreating);
        return;
    }

    if (ShouldAttack())
    {
        SetTacticalState(ECombat_TacticalState::Attacking);
    }
    else
    {
        ExecuteCirclingPattern();
    }
}