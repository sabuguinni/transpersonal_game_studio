#include "TRexBehavior.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

UTRexBehaviorComponent::UTRexBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentState = ENPC_TRexState::Idle;
    LastAttackTime = -999.0f;
    LastRoarTime = -999.0f;
    StateTimer = 0.0f;
    HomeLocation = FVector::ZeroVector;
    CurrentPatrolTarget = FVector::ZeroVector;
}

void UTRexBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        HomeLocation = Owner->GetActorLocation();
        CurrentPatrolTarget = HomeLocation;
    }

    SetState(ENPC_TRexState::Patrol);
    PickNextPatrolPoint();
}

void UTRexBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimer += DeltaTime;

    // State machine transitions
    switch (CurrentState)
    {
    case ENPC_TRexState::Idle:
        if (StateTimer > 3.0f)
        {
            SetState(ENPC_TRexState::Patrol);
            PickNextPatrolPoint();
        }
        if (CanSeePlayer() || CanHearPlayer())
        {
            if (CanRoar()) TriggerRoar();
            SetState(ENPC_TRexState::Chase);
        }
        break;

    case ENPC_TRexState::Patrol:
        UpdatePatrol(DeltaTime);
        if (CanSeePlayer() || CanHearPlayer())
        {
            if (CanRoar()) TriggerRoar();
            SetState(ENPC_TRexState::Chase);
        }
        break;

    case ENPC_TRexState::Investigate:
        UpdateInvestigate(DeltaTime);
        if (CanSeePlayer())
        {
            SetState(ENPC_TRexState::Chase);
        }
        if (StateTimer > 8.0f)
        {
            SetState(ENPC_TRexState::Patrol);
            PickNextPatrolPoint();
        }
        break;

    case ENPC_TRexState::Chase:
        UpdateChase(DeltaTime);
        if (IsInAttackRange())
        {
            SetState(ENPC_TRexState::Attack);
        }
        if (!CanSeePlayer() && !CanHearPlayer() && StateTimer > 5.0f)
        {
            SetState(ENPC_TRexState::Investigate);
        }
        break;

    case ENPC_TRexState::Attack:
        UpdateAttack(DeltaTime);
        if (!IsInAttackRange())
        {
            SetState(ENPC_TRexState::Chase);
        }
        break;

    case ENPC_TRexState::Roar:
        if (StateTimer > 2.5f)
        {
            SetState(ENPC_TRexState::Chase);
        }
        break;
    }
}

void UTRexBehaviorComponent::SetState(ENPC_TRexState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;
    }
}

void UTRexBehaviorComponent::PickNextPatrolPoint()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Pick a random point within PatrolRadius of HomeLocation
    const float Angle = FMath::RandRange(0.0f, 360.0f);
    const float Distance = FMath::RandRange(Config.PatrolRadius * 0.3f, Config.PatrolRadius);
    const float RadAngle = FMath::DegreesToRadians(Angle);

    CurrentPatrolTarget = HomeLocation + FVector(
        FMath::Cos(RadAngle) * Distance,
        FMath::Sin(RadAngle) * Distance,
        0.0f
    );
}

bool UTRexBehaviorComponent::HasReachedPatrolPoint() const
{
    AActor* Owner = GetOwner();
    if (!Owner) return false;

    const float DistSq = FVector::DistSquared2D(Owner->GetActorLocation(), CurrentPatrolTarget);
    return DistSq < (400.0f * 400.0f); // 400 unit threshold
}

bool UTRexBehaviorComponent::CanSeePlayer() const
{
    AActor* Player = GetPlayerActor();
    AActor* Owner = GetOwner();
    if (!Player || !Owner) return false;

    const float Dist = GetDistanceToPlayer();
    if (Dist > Config.ChaseRange) return false;

    // Check sight angle
    const FVector ToPlayer = (Player->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
    const FVector Forward = Owner->GetActorForwardVector();
    const float DotProduct = FVector::DotProduct(Forward, ToPlayer);
    const float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

    if (AngleDeg > Config.SightAngle) return false;

    // Line of sight check
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Owner->GetActorLocation() + FVector(0, 0, 150.0f),
        Player->GetActorLocation() + FVector(0, 0, 100.0f),
        ECC_Visibility,
        Params
    );

    return !bBlocked || HitResult.GetActor() == Player;
}

bool UTRexBehaviorComponent::CanHearPlayer() const
{
    const float Dist = GetDistanceToPlayer();
    return Dist < Config.HearingRange;
}

float UTRexBehaviorComponent::GetDistanceToPlayer() const
{
    AActor* Player = GetPlayerActor();
    AActor* Owner = GetOwner();
    if (!Player || !Owner) return TNumericLimits<float>::Max();

    return FVector::Dist(Owner->GetActorLocation(), Player->GetActorLocation());
}

void UTRexBehaviorComponent::PerformAttack()
{
    UWorld* World = GetWorld();
    if (!World) return;

    const float CurrentTime = World->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < AttackCooldown) return;

    LastAttackTime = CurrentTime;

    AActor* Player = GetPlayerActor();
    if (!Player) return;

    // Apply damage to player
    UGameplayStatics::ApplyDamage(
        Player,
        Config.AttackDamage,
        nullptr,
        GetOwner(),
        nullptr
    );

    UE_LOG(LogTemp, Warning, TEXT("T-Rex attacked player for %.1f damage!"), Config.AttackDamage);
}

bool UTRexBehaviorComponent::IsInAttackRange() const
{
    return GetDistanceToPlayer() <= Config.AttackRange;
}

void UTRexBehaviorComponent::TriggerRoar()
{
    UWorld* World = GetWorld();
    if (!World) return;

    LastRoarTime = World->GetTimeSeconds();
    SetState(ENPC_TRexState::Roar);

    // Apply fear to all players/pawns within roar radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), NearbyActors);

    AActor* Owner = GetOwner();
    if (!Owner) return;

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == Owner) continue;
        const float Dist = FVector::Dist(Owner->GetActorLocation(), Actor->GetActorLocation());
        if (Dist <= Config.RoarRadius)
        {
            // Broadcast roar event — other systems (SurvivalComponent fear) can respond
            UE_LOG(LogTemp, Warning, TEXT("T-Rex ROAR reached actor: %s at distance %.0f"), *Actor->GetName(), Dist);
        }
    }
}

bool UTRexBehaviorComponent::CanRoar() const
{
    UWorld* World = GetWorld();
    if (!World) return false;

    return (World->GetTimeSeconds() - LastRoarTime) >= Config.RoarCooldown;
}

void UTRexBehaviorComponent::UpdatePatrol(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    if (HasReachedPatrolPoint())
    {
        // Reached patrol point — idle briefly then pick new point
        SetState(ENPC_TRexState::Idle);
        return;
    }

    // Move toward patrol target
    const FVector CurrentLoc = Owner->GetActorLocation();
    const FVector Direction = (CurrentPatrolTarget - CurrentLoc).GetSafeNormal2D();
    const FVector NewLoc = CurrentLoc + Direction * Config.PatrolSpeed * DeltaTime;

    Owner->SetActorLocation(NewLoc, true);

    // Face movement direction
    if (!Direction.IsNearlyZero())
    {
        const FRotator NewRot = Direction.Rotation();
        Owner->SetActorRotation(FRotator(0.0f, NewRot.Yaw, 0.0f));
    }
}

void UTRexBehaviorComponent::UpdateChase(float DeltaTime)
{
    AActor* Owner = GetOwner();
    AActor* Player = GetPlayerActor();
    if (!Owner || !Player) return;

    const FVector CurrentLoc = Owner->GetActorLocation();
    const FVector Direction = (Player->GetActorLocation() - CurrentLoc).GetSafeNormal2D();
    const FVector NewLoc = CurrentLoc + Direction * Config.ChaseSpeed * DeltaTime;

    Owner->SetActorLocation(NewLoc, true);

    // Face player
    if (!Direction.IsNearlyZero())
    {
        const FRotator NewRot = Direction.Rotation();
        Owner->SetActorRotation(FRotator(0.0f, NewRot.Yaw, 0.0f));
    }
}

void UTRexBehaviorComponent::UpdateAttack(float DeltaTime)
{
    // Face player and attempt attack
    AActor* Owner = GetOwner();
    AActor* Player = GetPlayerActor();
    if (!Owner || !Player) return;

    const FVector Direction = (Player->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal2D();
    if (!Direction.IsNearlyZero())
    {
        Owner->SetActorRotation(FRotator(0.0f, Direction.Rotation().Yaw, 0.0f));
    }

    PerformAttack();
}

void UTRexBehaviorComponent::UpdateInvestigate(float DeltaTime)
{
    // Move toward last known player position (CurrentPatrolTarget used as investigation point)
    AActor* Owner = GetOwner();
    if (!Owner) return;

    const FVector CurrentLoc = Owner->GetActorLocation();
    const FVector Direction = (CurrentPatrolTarget - CurrentLoc).GetSafeNormal2D();

    if (FVector::DistSquared2D(CurrentLoc, CurrentPatrolTarget) > (200.0f * 200.0f))
    {
        const FVector NewLoc = CurrentLoc + Direction * (Config.PatrolSpeed * 0.6f) * DeltaTime;
        Owner->SetActorLocation(NewLoc, true);
    }
}

AActor* UTRexBehaviorComponent::GetPlayerActor() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC) return nullptr;

    return PC->GetPawn();
}
