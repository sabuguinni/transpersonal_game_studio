#include "DinosaurBehaviorTree.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

// ============================================================
// Constructor
// ============================================================
ADinosaurBehaviorTree::ADinosaurBehaviorTree()
{
    PrimaryActorTick.bCanEverTick = true;

    // Perception component
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

    // Default movement
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = CombatStats.PatrolSpeed;
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    }

    bUseControllerRotationYaw = false;
}

// ============================================================
// BeginPlay
// ============================================================
void ADinosaurBehaviorTree::BeginPlay()
{
    Super::BeginPlay();

    // Set initial state based on behavior mode
    if (BehaviorMode == ENarr_DinoBehaviorMode::Predator)
    {
        SetState(ENarr_DinoState::Patrol);
    }
    else
    {
        SetState(ENarr_DinoState::Idle);
    }

    // Find pack members if pack hunter
    if (bIsPackHunter)
    {
        FindNearbyPackMembers();
    }
}

// ============================================================
// Tick — state machine dispatcher
// ============================================================
void ADinosaurBehaviorTree::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastAttack += DeltaTime;

    switch (CurrentState)
    {
        case ENarr_DinoState::Idle:
            HandleIdleState(DeltaTime);
            break;
        case ENarr_DinoState::Patrol:
            HandlePatrolState(DeltaTime);
            break;
        case ENarr_DinoState::Investigate:
            HandleInvestigateState(DeltaTime);
            break;
        case ENarr_DinoState::Chase:
            HandleChaseState(DeltaTime);
            break;
        case ENarr_DinoState::Attack:
            HandleAttackState(DeltaTime);
            break;
        case ENarr_DinoState::Flee:
            HandleFleeState(DeltaTime);
            break;
        default:
            break;
    }
}

// ============================================================
// SetState
// ============================================================
void ADinosaurBehaviorTree::SetState(ENarr_DinoState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;

    // Adjust movement speed based on state
    if (GetCharacterMovement())
    {
        switch (NewState)
        {
            case ENarr_DinoState::Chase:
            case ENarr_DinoState::Attack:
                GetCharacterMovement()->MaxWalkSpeed = CombatStats.ChaseSpeed;
                break;
            case ENarr_DinoState::Flee:
                GetCharacterMovement()->MaxWalkSpeed = CombatStats.ChaseSpeed * 1.2f;
                break;
            case ENarr_DinoState::Patrol:
            case ENarr_DinoState::Investigate:
                GetCharacterMovement()->MaxWalkSpeed = CombatStats.PatrolSpeed;
                break;
            default:
                GetCharacterMovement()->MaxWalkSpeed = CombatStats.PatrolSpeed * 0.5f;
                break;
        }
    }
}

// ============================================================
// HandleIdleState
// ============================================================
void ADinosaurBehaviorTree::HandleIdleState(float DeltaTime)
{
    PatrolWaitTimer += DeltaTime;
    if (PatrolWaitTimer >= PatrolWaitTime)
    {
        PatrolWaitTimer = 0.0f;
        if (PatrolPoints.Num() > 0)
        {
            SetState(ENarr_DinoState::Patrol);
        }
    }
}

// ============================================================
// HandlePatrolState
// ============================================================
void ADinosaurBehaviorTree::HandlePatrolState(float DeltaTime)
{
    if (PatrolPoints.Num() == 0) return;

    AActor* TargetPoint = PatrolPoints[CurrentPatrolIndex];
    if (!TargetPoint) return;

    float DistToPoint = FVector::Dist(GetActorLocation(), TargetPoint->GetActorLocation());

    if (DistToPoint < 150.0f)
    {
        // Reached patrol point — wait then advance
        if (!bWaiting)
        {
            bWaiting = true;
            PatrolWaitTimer = 0.0f;
            SetState(ENarr_DinoState::Idle);
        }
        AdvancePatrolPoint();
    }
    else
    {
        MoveToLocation(TargetPoint->GetActorLocation(), CombatStats.PatrolSpeed);
    }

    // Scan for player while patrolling
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn && CanSeeTarget(PlayerPawn))
    {
        OnTargetDetected(PlayerPawn);
    }
}

// ============================================================
// HandleInvestigateState
// ============================================================
void ADinosaurBehaviorTree::HandleInvestigateState(float DeltaTime)
{
    InvestigateTimer += DeltaTime;

    MoveToLocation(LastKnownTargetLocation, CombatStats.PatrolSpeed);

    float DistToInvestigate = FVector::Dist(GetActorLocation(), LastKnownTargetLocation);

    if (DistToInvestigate < 200.0f || InvestigateTimer > 15.0f)
    {
        // Nothing found — return to patrol
        InvestigateTimer = 0.0f;
        SetState(ENarr_DinoState::Patrol);
        CurrentTarget = nullptr;
    }

    // Check if we spot the player during investigation
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn && CanSeeTarget(PlayerPawn))
    {
        OnTargetDetected(PlayerPawn);
    }
}

// ============================================================
// HandleChaseState
// ============================================================
void ADinosaurBehaviorTree::HandleChaseState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        OnTargetLost();
        return;
    }

    // Update last known location
    LastKnownTargetLocation = CurrentTarget->GetActorLocation();

    float DistToTarget = GetDistanceToTarget();

    // In attack range?
    if (DistToTarget <= CombatStats.AttackRange)
    {
        SetState(ENarr_DinoState::Attack);
        return;
    }

    // Lost sight?
    if (!CanSeeTarget(CurrentTarget))
    {
        // Give up after a while — investigate last known position
        SetState(ENarr_DinoState::Investigate);
        return;
    }

    MoveToLocation(CurrentTarget->GetActorLocation(), CombatStats.ChaseSpeed);

    // Pack hunters alert nearby members
    if (bIsPackHunter)
    {
        AlertPackMembers(CurrentTarget);
    }
}

// ============================================================
// HandleAttackState
// ============================================================
void ADinosaurBehaviorTree::HandleAttackState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetState(ENarr_DinoState::Patrol);
        return;
    }

    float DistToTarget = GetDistanceToTarget();

    // Target moved out of range — chase again
    if (DistToTarget > CombatStats.AttackRange * 1.5f)
    {
        SetState(ENarr_DinoState::Chase);
        return;
    }

    // Face target
    FVector ToTarget = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FRotator LookAt = ToTarget.Rotation();
    SetActorRotation(FRotator(0.0f, LookAt.Yaw, 0.0f));

    // Attack cooldown
    if (TimeSinceLastAttack >= CombatStats.AttackCooldown)
    {
        TimeSinceLastAttack = 0.0f;
        // Damage is applied via overlap/collision — log for now
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s attacks target %s for %.1f damage"),
            *GetName(), *CurrentTarget->GetName(), CombatStats.AttackDamage);
    }

    // Check flee threshold
    if (CombatStats.CurrentHealth / CombatStats.MaxHealth <= CombatStats.FleeHealthThreshold)
    {
        SetState(ENarr_DinoState::Flee);
    }
}

// ============================================================
// HandleFleeState
// ============================================================
void ADinosaurBehaviorTree::HandleFleeState(float DeltaTime)
{
    if (!CurrentTarget) return;

    // Move directly away from threat
    FVector AwayFromThreat = GetActorLocation() - CurrentTarget->GetActorLocation();
    AwayFromThreat.Normalize();
    FVector FleeTarget = GetActorLocation() + AwayFromThreat * 3000.0f;

    MoveToLocation(FleeTarget, CombatStats.ChaseSpeed * 1.2f);

    // If far enough away, calm down
    if (GetDistanceToTarget() > Senses.SightRadius * 1.5f)
    {
        CurrentTarget = nullptr;
        SetState(ENarr_DinoState::Idle);
    }
}

// ============================================================
// OnTargetDetected
// ============================================================
void ADinosaurBehaviorTree::OnTargetDetected(AActor* Target)
{
    if (!Target) return;

    CurrentTarget = Target;
    LastKnownTargetLocation = Target->GetActorLocation();

    if (BehaviorMode == ENarr_DinoBehaviorMode::Predator)
    {
        SetState(ENarr_DinoState::Chase);
    }
    else
    {
        // Herbivores flee from player
        SetState(ENarr_DinoState::Flee);
    }
}

// ============================================================
// OnTargetLost
// ============================================================
void ADinosaurBehaviorTree::OnTargetLost()
{
    if (LastKnownTargetLocation != FVector::ZeroVector)
    {
        SetState(ENarr_DinoState::Investigate);
    }
    else
    {
        CurrentTarget = nullptr;
        SetState(ENarr_DinoState::Patrol);
    }
}

// ============================================================
// TakeDamageFromPlayer
// ============================================================
void ADinosaurBehaviorTree::TakeDamageFromPlayer(float Damage)
{
    CombatStats.CurrentHealth = FMath::Max(0.0f, CombatStats.CurrentHealth - Damage);

    if (!IsAlive())
    {
        SetState(ENarr_DinoState::Idle);
        SetActorEnableCollision(false);
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s has been killed"), *GetName());
        return;
    }

    // Predators become more aggressive when hurt
    if (BehaviorMode == ENarr_DinoBehaviorMode::Predator)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            CurrentTarget = PlayerPawn;
            SetState(ENarr_DinoState::Chase);
        }
    }
    else
    {
        // Herbivores flee when hurt
        SetState(ENarr_DinoState::Flee);
    }

    // Alert pack
    if (bIsPackHunter)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            AlertPackMembers(PlayerPawn);
        }
    }
}

// ============================================================
// IsAlive
// ============================================================
bool ADinosaurBehaviorTree::IsAlive() const
{
    return CombatStats.CurrentHealth > 0.0f;
}

// ============================================================
// AlertPackMembers
// ============================================================
void ADinosaurBehaviorTree::AlertPackMembers(AActor* Threat)
{
    FindNearbyPackMembers();
    for (ADinosaurBehaviorTree* Member : NearbyPackMembers)
    {
        if (Member && Member != this && Member->IsAlive())
        {
            Member->OnTargetDetected(Threat);
        }
    }
}

// ============================================================
// FindNearbyPackMembers
// ============================================================
void ADinosaurBehaviorTree::FindNearbyPackMembers()
{
    NearbyPackMembers.Empty();

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADinosaurBehaviorTree::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        ADinosaurBehaviorTree* Member = Cast<ADinosaurBehaviorTree>(Actor);
        if (Member && Member != this && Member->Species == Species)
        {
            float Dist = FVector::Dist(GetActorLocation(), Member->GetActorLocation());
            if (Dist <= HerdAlertRadius)
            {
                NearbyPackMembers.Add(Member);
            }
        }
    }
}

// ============================================================
// CanSeeTarget
// ============================================================
bool ADinosaurBehaviorTree::CanSeeTarget(AActor* Target) const
{
    if (!Target || !GetWorld()) return false;

    float Dist = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (Dist > Senses.SightRadius) return false;

    // Angle check
    FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector ForwardVec = GetActorForwardVector();
    float DotProduct = FVector::DotProduct(ForwardVec, ToTarget);
    float HalfAngleCos = FMath::Cos(FMath::DegreesToRadians(Senses.SightAngleDegrees * 0.5f));

    if (DotProduct < HalfAngleCos) return false;

    // Line of sight trace
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        GetActorLocation() + FVector(0, 0, 80.0f),
        Target->GetActorLocation() + FVector(0, 0, 80.0f),
        ECC_Visibility,
        QueryParams
    );

    // If nothing blocked, we can see the target
    return !bHit || HitResult.GetActor() == Target;
}

// ============================================================
// GetDistanceToTarget
// ============================================================
float ADinosaurBehaviorTree::GetDistanceToTarget() const
{
    if (!CurrentTarget) return TNumericLimits<float>::Max();
    return FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
}

// ============================================================
// AdvancePatrolPoint
// ============================================================
void ADinosaurBehaviorTree::AdvancePatrolPoint()
{
    if (PatrolPoints.Num() == 0) return;
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    bWaiting = false;
}

// ============================================================
// MoveToLocation
// ============================================================
void ADinosaurBehaviorTree::MoveToLocation(const FVector& Location, float Speed)
{
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = Speed;
    }

    // Use AIController if available
    AAIController* AICtrl = Cast<AAIController>(GetController());
    if (AICtrl)
    {
        AICtrl->MoveToLocation(Location, 50.0f, true, true, false, true);
    }
    else
    {
        // Fallback: direct movement input
        FVector Direction = (Location - GetActorLocation()).GetSafeNormal();
        AddMovementInput(Direction, 1.0f);
    }
}
