#include "AI/NPCBehavior/TRexBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UTRexBehaviorComponent::UTRexBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — sufficient for AI

    // Territory defaults
    PatrolRadius        = 5000.0f;
    TerritoryCenter     = FVector::ZeroVector;

    // Detection defaults
    DetectionRange      = 3000.0f;
    AttackRange         = 300.0f;
    HearingRange        = 1500.0f;
    FieldOfViewDegrees  = 120.0f;

    // Movement defaults
    PatrolSpeed         = 300.0f;
    ChaseSpeed          = 700.0f;

    // Combat defaults
    AttackDamage        = 80.0f;
    AttackCooldown      = 2.5f;

    // Memory defaults
    MemoryDuration      = 30.0f;

    // State init
    CurrentState            = ENPC_TRexState::Patrolling;
    LastKnownPlayerLocation = FVector::ZeroVector;
    TimeSincePlayerSeen     = 0.0f;
    bPlayerInMemory         = false;
    TimeSinceLastAttack     = 0.0f;
    CurrentPatrolIndex      = 0;
    PatrolWaitTimer         = 0.0f;
    bWaitingAtPatrolPoint   = false;
    TrackedPlayer           = nullptr;
}

void UTRexBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Set territory center to actor's starting location
    if (AActor* Owner = GetOwner())
    {
        TerritoryCenter = Owner->GetActorLocation();
    }

    // Auto-generate patrol points if none defined
    if (PatrolPoints.Num() == 0)
    {
        GeneratePatrolPoints(6);
    }
}

void UTRexBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update attack cooldown
    TimeSinceLastAttack += DeltaTime;

    // Update memory decay
    UpdateMemory(DeltaTime);

    // Scan for player every tick (10Hz)
    ScanForPlayer();

    // State machine update
    switch (CurrentState)
    {
        case ENPC_TRexState::Patrolling:
            UpdatePatrolState(DeltaTime);
            break;
        case ENPC_TRexState::Investigating:
            UpdateInvestigateState(DeltaTime);
            break;
        case ENPC_TRexState::Chasing:
            UpdateChaseState(DeltaTime);
            break;
        case ENPC_TRexState::Attacking:
            UpdateAttackState(DeltaTime);
            break;
        case ENPC_TRexState::Resting:
            UpdateRestingState(DeltaTime);
            break;
        default:
            break;
    }
}

// --- State Updates ---

void UTRexBehaviorComponent::UpdatePatrolState(float DeltaTime)
{
    if (PatrolPoints.Num() == 0) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    const FVector Target = GetNextPatrolPoint();
    const float DistToTarget = FVector::Dist(Owner->GetActorLocation(), Target);

    if (bWaitingAtPatrolPoint)
    {
        PatrolWaitTimer -= DeltaTime;
        if (PatrolWaitTimer <= 0.0f)
        {
            bWaitingAtPatrolPoint = false;
            CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
        }
        return;
    }

    if (DistToTarget < 150.0f)
    {
        // Arrived at patrol point — wait
        bWaitingAtPatrolPoint = true;
        PatrolWaitTimer = PatrolPoints[CurrentPatrolIndex].WaitDuration;
        return;
    }

    MoveToLocation(Target, PatrolSpeed);
}

void UTRexBehaviorComponent::UpdateInvestigateState(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    const float DistToLastKnown = FVector::Dist(Owner->GetActorLocation(), LastKnownPlayerLocation);

    if (DistToLastKnown < 200.0f)
    {
        // Reached last known position — player gone, return to patrol
        if (!bPlayerInMemory)
        {
            TransitionToState(ENPC_TRexState::Patrolling);
        }
        return;
    }

    MoveToLocation(LastKnownPlayerLocation, PatrolSpeed * 1.2f);
}

void UTRexBehaviorComponent::UpdateChaseState(float DeltaTime)
{
    if (!TrackedPlayer) 
    {
        TransitionToState(ENPC_TRexState::Investigating);
        return;
    }

    const float DistToPlayer = GetDistanceToPlayer();

    if (DistToPlayer <= AttackRange)
    {
        TransitionToState(ENPC_TRexState::Attacking);
        return;
    }

    if (DistToPlayer > DetectionRange * 1.5f && !bPlayerInMemory)
    {
        // Player escaped detection range + memory expired
        TransitionToState(ENPC_TRexState::Patrolling);
        return;
    }

    // Chase the player
    MoveToLocation(TrackedPlayer->GetActorLocation(), ChaseSpeed);
    LastKnownPlayerLocation = TrackedPlayer->GetActorLocation();
}

void UTRexBehaviorComponent::UpdateAttackState(float DeltaTime)
{
    if (!TrackedPlayer)
    {
        TransitionToState(ENPC_TRexState::Chasing);
        return;
    }

    const float DistToPlayer = GetDistanceToPlayer();

    if (DistToPlayer > AttackRange * 1.5f)
    {
        TransitionToState(ENPC_TRexState::Chasing);
        return;
    }

    // Execute attack if cooldown ready
    if (TimeSinceLastAttack >= AttackCooldown)
    {
        TimeSinceLastAttack = 0.0f;

        // Apply damage to player
        UGameplayStatics::ApplyDamage(
            TrackedPlayer,
            AttackDamage,
            nullptr,
            GetOwner(),
            nullptr
        );

        UE_LOG(LogTemp, Warning, TEXT("TRex ATTACK: %.0f damage to %s"),
            AttackDamage, *TrackedPlayer->GetName());
    }
}

void UTRexBehaviorComponent::UpdateRestingState(float DeltaTime)
{
    // Resting T-Rex is less alert — reduced detection range
    // Will still react if player gets very close
    if (TrackedPlayer)
    {
        const float Dist = GetDistanceToPlayer();
        if (Dist < DetectionRange * 0.5f)
        {
            TransitionToState(ENPC_TRexState::Chasing);
        }
    }
}

// --- Memory System ---

void UTRexBehaviorComponent::UpdateMemory(float DeltaTime)
{
    if (bPlayerInMemory)
    {
        TimeSincePlayerSeen += DeltaTime;
        if (TimeSincePlayerSeen >= MemoryDuration)
        {
            bPlayerInMemory = false;
            TimeSincePlayerSeen = 0.0f;
            TrackedPlayer = nullptr;
            UE_LOG(LogTemp, Log, TEXT("TRex: Player memory expired — returning to patrol"));
        }
    }
}

// --- Detection ---

void UTRexBehaviorComponent::ScanForPlayer()
{
    UWorld* World = GetWorld();
    if (!World) return;

    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!PlayerChar) return;

    const float Dist = FVector::Dist(GetOwner()->GetActorLocation(), PlayerChar->GetActorLocation());

    bool bCanDetect = false;

    // Visual detection (within FOV and range)
    if (Dist <= DetectionRange && CanSeePlayer())
    {
        bCanDetect = true;
    }
    // Hearing detection (closer range, no FOV requirement)
    else if (Dist <= HearingRange && CanHearPlayer())
    {
        bCanDetect = true;
    }

    if (bCanDetect)
    {
        TrackedPlayer = PlayerChar;
        OnPlayerDetected(PlayerChar);
    }
    else if (TrackedPlayer && !bCanDetect)
    {
        // Only lose direct sight — memory persists
        if (CurrentState == ENPC_TRexState::Chasing || CurrentState == ENPC_TRexState::Attacking)
        {
            // Keep chasing to last known position
            LastKnownPlayerLocation = PlayerChar->GetActorLocation();
        }
    }
}

bool UTRexBehaviorComponent::CanSeePlayer() const
{
    UWorld* World = GetWorld();
    if (!World) return false;

    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!PlayerChar) return false;

    AActor* Owner = GetOwner();
    if (!Owner) return false;

    // FOV check
    const FVector ToPlayer = (PlayerChar->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
    const FVector Forward = Owner->GetActorForwardVector();
    const float DotProduct = FVector::DotProduct(Forward, ToPlayer);
    const float HalfFOVCos = FMath::Cos(FMath::DegreesToRadians(FieldOfViewDegrees * 0.5f));

    if (DotProduct < HalfFOVCos) return false;

    // Line of sight check
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    const bool bBlocked = World->LineTraceSingleByChannel(
        HitResult,
        Owner->GetActorLocation() + FVector(0, 0, 100),
        PlayerChar->GetActorLocation() + FVector(0, 0, 100),
        ECC_Visibility,
        Params
    );

    return !bBlocked || HitResult.GetActor() == PlayerChar;
}

bool UTRexBehaviorComponent::CanHearPlayer() const
{
    // Simplified: player movement generates sound within hearing range
    // Full implementation would check player velocity/noise level
    UWorld* World = GetWorld();
    if (!World) return false;

    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!PlayerChar) return false;

    const float Dist = FVector::Dist(GetOwner()->GetActorLocation(), PlayerChar->GetActorLocation());
    if (Dist > HearingRange) return false;

    // Check if player is moving (generating noise)
    UCharacterMovementComponent* MovComp = PlayerChar->GetCharacterMovement();
    if (!MovComp) return false;

    return MovComp->Velocity.SizeSquared() > 100.0f; // Moving faster than ~10 cm/s
}

float UTRexBehaviorComponent::GetDistanceToPlayer() const
{
    UWorld* World = GetWorld();
    if (!World || !TrackedPlayer) return TNumericLimits<float>::Max();

    return FVector::Dist(GetOwner()->GetActorLocation(), TrackedPlayer->GetActorLocation());
}

// --- Events ---

void UTRexBehaviorComponent::OnPlayerDetected(AActor* Player)
{
    if (!Player) return;

    TrackedPlayer = Player;
    LastKnownPlayerLocation = Player->GetActorLocation();
    bPlayerInMemory = true;
    TimeSincePlayerSeen = 0.0f;

    if (CurrentState == ENPC_TRexState::Patrolling || CurrentState == ENPC_TRexState::Resting || CurrentState == ENPC_TRexState::Investigating)
    {
        const float Dist = GetDistanceToPlayer();
        if (Dist <= AttackRange)
        {
            TransitionToState(ENPC_TRexState::Attacking);
        }
        else
        {
            TransitionToState(ENPC_TRexState::Chasing);
        }
    }
}

void UTRexBehaviorComponent::OnPlayerLost()
{
    if (CurrentState == ENPC_TRexState::Chasing)
    {
        TransitionToState(ENPC_TRexState::Investigating);
    }
}

// --- Utility ---

void UTRexBehaviorComponent::TransitionToState(ENPC_TRexState NewState)
{
    if (CurrentState == NewState) return;

    UE_LOG(LogTemp, Log, TEXT("TRex state: %s -> %s"),
        *UEnum::GetValueAsString(CurrentState),
        *UEnum::GetValueAsString(NewState));

    CurrentState = NewState;

    // Reset patrol wait when re-entering patrol
    if (NewState == ENPC_TRexState::Patrolling)
    {
        bWaitingAtPatrolPoint = false;
        PatrolWaitTimer = 0.0f;
    }
}

void UTRexBehaviorComponent::SetTerritoryCenter(FVector NewCenter)
{
    TerritoryCenter = NewCenter;
    GeneratePatrolPoints(6);
}

void UTRexBehaviorComponent::GeneratePatrolPoints(int32 NumPoints)
{
    PatrolPoints.Empty();

    for (int32 i = 0; i < NumPoints; ++i)
    {
        const float Angle = (360.0f / NumPoints) * i;
        const float RadiusFraction = FMath::RandRange(0.4f, 1.0f);
        const float Rad = FMath::DegreesToRadians(Angle);

        FNPC_TRexPatrolPoint Point;
        Point.Location = TerritoryCenter + FVector(
            FMath::Cos(Rad) * PatrolRadius * RadiusFraction,
            FMath::Sin(Rad) * PatrolRadius * RadiusFraction,
            0.0f
        );
        Point.WaitDuration = FMath::RandRange(3.0f, 10.0f);
        PatrolPoints.Add(Point);
    }

    UE_LOG(LogTemp, Log, TEXT("TRex: Generated %d patrol points in %.0f unit radius"), NumPoints, PatrolRadius);
}

FVector UTRexBehaviorComponent::GetNextPatrolPoint() const
{
    if (PatrolPoints.Num() == 0) return TerritoryCenter;
    return PatrolPoints[CurrentPatrolIndex].Location;
}

void UTRexBehaviorComponent::MoveToLocation(const FVector& Target, float Speed)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    const FVector CurrentLoc = Owner->GetActorLocation();
    const FVector Direction = (Target - CurrentLoc).GetSafeNormal();

    // Rotate owner toward target
    if (!Direction.IsNearlyZero())
    {
        const FRotator TargetRot = Direction.Rotation();
        const FRotator CurrentRot = Owner->GetActorRotation();
        const FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, 0.1f, 5.0f);
        Owner->SetActorRotation(NewRot);
    }

    // Move via character movement if available
    ACharacter* OwnerChar = Cast<ACharacter>(Owner);
    if (OwnerChar)
    {
        OwnerChar->AddMovementInput(Direction, 1.0f);
    }
    else
    {
        // Direct translation fallback
        const FVector NewLoc = CurrentLoc + Direction * Speed * 0.1f;
        Owner->SetActorLocation(NewLoc, true);
    }
}
