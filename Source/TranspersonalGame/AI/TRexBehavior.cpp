#include "TRexBehavior.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UTRexBehaviorComponent::UTRexBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentState = ENPC_TRexState::Idle;
    LastRoarTime = -999.0f;
    LastAttackTime = -999.0f;
    StateTimer = 0.0f;
}

void UTRexBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Record home location for patrol radius
    if (GetOwner())
    {
        HomeLocation = GetOwner()->GetActorLocation();
        CurrentPatrolTarget = HomeLocation;
    }

    // Start in patrol state
    SetState(ENPC_TRexState::Patrol);
    PickNextPatrolPoint();
}

void UTRexBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimer += DeltaTime;

    switch (CurrentState)
    {
    case ENPC_TRexState::Idle:
        // After 3 seconds idle, resume patrol
        if (StateTimer > 3.0f)
        {
            SetState(ENPC_TRexState::Patrol);
            PickNextPatrolPoint();
        }
        // Check for player detection
        if (CanSeePlayer() || CanHearPlayer())
        {
            SetState(ENPC_TRexState::Investigate);
        }
        break;

    case ENPC_TRexState::Patrol:
        UpdatePatrol(DeltaTime);
        if (CanSeePlayer())
        {
            if (CanRoar()) TriggerRoar();
            SetState(ENPC_TRexState::Chase);
        }
        else if (CanHearPlayer())
        {
            SetState(ENPC_TRexState::Investigate);
        }
        break;

    case ENPC_TRexState::Investigate:
        UpdateInvestigate(DeltaTime);
        if (CanSeePlayer())
        {
            if (CanRoar()) TriggerRoar();
            SetState(ENPC_TRexState::Chase);
        }
        else if (StateTimer > 8.0f)
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
        else if (!CanSeePlayer() && GetDistanceToPlayer() > Config.ChaseRange * 1.5f)
        {
            SetState(ENPC_TRexState::Patrol);
            PickNextPatrolPoint();
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
        // Roar lasts 2 seconds then transitions to chase
        if (StateTimer > 2.0f)
        {
            SetState(ENPC_TRexState::Chase);
        }
        break;
    }
}

void UTRexBehaviorComponent::SetState(ENPC_TRexState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
    StateTimer = 0.0f;
}

void UTRexBehaviorComponent::PickNextPatrolPoint()
{
    if (!GetOwner()) return;

    // Pick a random point within patrol radius of home
    FVector RandomOffset = FVector(
        FMath::RandRange(-Config.PatrolRadius, Config.PatrolRadius),
        FMath::RandRange(-Config.PatrolRadius, Config.PatrolRadius),
        0.0f
    );
    // Clamp to radius
    if (RandomOffset.Size() > Config.PatrolRadius)
    {
        RandomOffset = RandomOffset.GetSafeNormal() * Config.PatrolRadius;
    }
    CurrentPatrolTarget = HomeLocation + RandomOffset;
}

bool UTRexBehaviorComponent::HasReachedPatrolPoint() const
{
    if (!GetOwner()) return false;
    float Dist = FVector::Dist2D(GetOwner()->GetActorLocation(), CurrentPatrolTarget);
    return Dist < 200.0f;
}

bool UTRexBehaviorComponent::CanSeePlayer() const
{
    AActor* Player = GetPlayerActor();
    if (!Player || !GetOwner()) return false;

    float Dist = GetDistanceToPlayer();
    if (Dist > Config.ChaseRange) return false;

    // Check sight angle
    FVector ToPlayer = (Player->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
    FVector Forward = GetOwner()->GetActorForwardVector();
    float DotProduct = FVector::DotProduct(Forward, ToPlayer);
    float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

    return AngleDeg <= Config.SightAngle;
}

bool UTRexBehaviorComponent::CanHearPlayer() const
{
    float Dist = GetDistanceToPlayer();
    return Dist <= Config.HearingRange;
}

float UTRexBehaviorComponent::GetDistanceToPlayer() const
{
    AActor* Player = GetPlayerActor();
    if (!Player || !GetOwner()) return TNumericLimits<float>::Max();
    return FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
}

void UTRexBehaviorComponent::PerformAttack()
{
    UWorld* World = GetWorld();
    if (!World) return;

    float Now = World->GetTimeSeconds();
    if (Now - LastAttackTime < AttackCooldown) return;

    LastAttackTime = Now;

    AActor* Player = GetPlayerActor();
    if (!Player) return;

    // Apply damage to player
    UGameplayStatics::ApplyDamage(Player, Config.AttackDamage, nullptr, GetOwner(), nullptr);
    UE_LOG(LogTemp, Warning, TEXT("TRex attacked player for %.0f damage"), Config.AttackDamage);
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

    // Apply fear to nearby players/NPCs within roar radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), NearbyActors);
    for (AActor* NearActor : NearbyActors)
    {
        if (NearActor == GetOwner()) continue;
        float Dist = FVector::Dist(GetOwner()->GetActorLocation(), NearActor->GetActorLocation());
        if (Dist <= Config.RoarRadius)
        {
            // Tag nearby pawns as "Frightened" for other systems to react
            NearActor->Tags.AddUnique(FName("Frightened"));
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("TRex ROARED — frightening actors within %.0f units"), Config.RoarRadius);
}

bool UTRexBehaviorComponent::CanRoar() const
{
    UWorld* World = GetWorld();
    if (!World) return false;
    return (World->GetTimeSeconds() - LastRoarTime) >= Config.RoarCooldown;
}

void UTRexBehaviorComponent::UpdatePatrol(float DeltaTime)
{
    if (!GetOwner()) return;

    if (HasReachedPatrolPoint())
    {
        SetState(ENPC_TRexState::Idle);
        return;
    }

    // Move toward patrol target
    FVector Direction = (CurrentPatrolTarget - GetOwner()->GetActorLocation()).GetSafeNormal();
    FVector NewLocation = GetOwner()->GetActorLocation() + Direction * Config.PatrolSpeed * DeltaTime;
    GetOwner()->SetActorLocation(NewLocation, true);

    // Face movement direction
    FRotator NewRot = Direction.Rotation();
    GetOwner()->SetActorRotation(NewRot);
}

void UTRexBehaviorComponent::UpdateChase(float DeltaTime)
{
    AActor* Player = GetPlayerActor();
    if (!Player || !GetOwner()) return;

    FVector Direction = (Player->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
    FVector NewLocation = GetOwner()->GetActorLocation() + Direction * Config.ChaseSpeed * DeltaTime;
    GetOwner()->SetActorLocation(NewLocation, true);

    FRotator NewRot = Direction.Rotation();
    GetOwner()->SetActorRotation(NewRot);
}

void UTRexBehaviorComponent::UpdateAttack(float DeltaTime)
{
    PerformAttack();
}

void UTRexBehaviorComponent::UpdateInvestigate(float DeltaTime)
{
    AActor* Player = GetPlayerActor();
    if (!Player || !GetOwner()) return;

    // Move slowly toward last known player position
    FVector Direction = (Player->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
    FVector NewLocation = GetOwner()->GetActorLocation() + Direction * (Config.PatrolSpeed * 0.5f) * DeltaTime;
    GetOwner()->SetActorLocation(NewLocation, true);
}

AActor* UTRexBehaviorComponent::GetPlayerActor() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return nullptr;
    return PC->GetPawn();
}
