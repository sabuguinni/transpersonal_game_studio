#include "NPC_TRexBehaviorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "Components/CapsuleComponent.h"

UNPC_TRexBehaviorComponent::UNPC_TRexBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for smooth behavior
}

void UNPC_TRexBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Set home location to current position
    if (GetOwner())
    {
        HomeLocation = GetOwner()->GetActorLocation();
        CurrentTarget = GeneratePatrolPoint();
        SetState(ENPC_TRexState::Patrol);
    }
}

void UNPC_TRexBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetOwner())
        return;

    // Update behavior based on current state
    switch (CurrentState)
    {
        case ENPC_TRexState::Patrol:
            UpdatePatrolBehavior(DeltaTime);
            break;
        case ENPC_TRexState::Chase:
            UpdateChaseBehavior(DeltaTime);
            break;
        case ENPC_TRexState::Attack:
            UpdateAttackBehavior(DeltaTime);
            break;
        case ENPC_TRexState::Return:
            UpdateReturnBehavior(DeltaTime);
            break;
    }
}

void UNPC_TRexBehaviorComponent::SetState(ENPC_TRexState NewState)
{
    if (CurrentState == NewState)
        return;

    CurrentState = NewState;
    
    // Reset timers on state change
    PatrolTimer = 0.0f;
    
    // Set appropriate movement speed
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            switch (NewState)
            {
                case ENPC_TRexState::Patrol:
                case ENPC_TRexState::Return:
                    MovementComp->MaxWalkSpeed = MovementSpeed;
                    break;
                case ENPC_TRexState::Chase:
                    MovementComp->MaxWalkSpeed = ChaseSpeed;
                    break;
                case ENPC_TRexState::Attack:
                    MovementComp->MaxWalkSpeed = MovementSpeed * 0.5f; // Slower during attack
                    break;
            }
        }
    }
}

APawn* UNPC_TRexBehaviorComponent::FindNearestPlayer()
{
    // Cache player reference to avoid expensive searches every tick
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CachedPlayer && (CurrentTime - PlayerCacheTime) < PlayerCacheTimeout)
    {
        return CachedPlayer;
    }

    // Find nearest player
    APawn* NearestPlayer = nullptr;
    float NearestDistance = FLT_MAX;
    
    FVector MyLocation = GetOwner()->GetActorLocation();
    
    for (TActorIterator<APawn> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        APawn* Pawn = *ActorItr;
        if (Pawn && Pawn->IsPlayerControlled())
        {
            float Distance = FVector::Dist(MyLocation, Pawn->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestPlayer = Pawn;
            }
        }
    }
    
    CachedPlayer = NearestPlayer;
    PlayerCacheTime = CurrentTime;
    return NearestPlayer;
}

FVector UNPC_TRexBehaviorComponent::GeneratePatrolPoint()
{
    // Generate random point within patrol radius
    FVector RandomDirection = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0.0f
    ).GetSafeNormal();
    
    float RandomDistance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    FVector TargetLocation = HomeLocation + (RandomDirection * RandomDistance);
    
    // Try to find navigable location
    if (UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld()))
    {
        FNavLocation NavLocation;
        if (NavSystem->ProjectPointToNavigation(TargetLocation, NavLocation, FVector(500.0f, 500.0f, 500.0f)))
        {
            return NavLocation.Location;
        }
    }
    
    return TargetLocation;
}

bool UNPC_TRexBehaviorComponent::CanAttack() const
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastAttackTime) >= AttackCooldown;
}

void UNPC_TRexBehaviorComponent::PerformAttack()
{
    if (!CanAttack())
        return;
        
    LastAttackTime = GetWorld()->GetTimeSeconds();
    
    // Simple attack implementation - in a full game this would trigger animations, damage, etc.
    APawn* Player = FindNearestPlayer();
    if (Player)
    {
        UE_LOG(LogTemp, Warning, TEXT("T-Rex attacks player!"));
        
        // Could add damage dealing, animation triggers, sound effects here
        // For now, just log the attack
    }
}

void UNPC_TRexBehaviorComponent::UpdatePatrolBehavior(float DeltaTime)
{
    // Check for nearby players
    APawn* Player = FindNearestPlayer();
    if (Player)
    {
        float DistanceToPlayer = GetDistanceToPlayer();
        if (DistanceToPlayer <= ChaseDistance)
        {
            SetState(ENPC_TRexState::Chase);
            return;
        }
    }
    
    // Continue patrolling
    if (IsAtLocation(CurrentTarget))
    {
        PatrolTimer += DeltaTime;
        if (PatrolTimer >= PatrolWaitTime)
        {
            CurrentTarget = GeneratePatrolPoint();
            PatrolTimer = 0.0f;
        }
    }
    else
    {
        MoveToLocation(CurrentTarget, MovementSpeed);
    }
}

void UNPC_TRexBehaviorComponent::UpdateChaseBehavior(float DeltaTime)
{
    APawn* Player = FindNearestPlayer();
    if (!Player)
    {
        SetState(ENPC_TRexState::Return);
        return;
    }
    
    float DistanceToPlayer = GetDistanceToPlayer();
    
    // Check if player is too far away
    if (DistanceToPlayer > ChaseDistance * 1.5f) // Add some hysteresis
    {
        SetState(ENPC_TRexState::Return);
        return;
    }
    
    // Check if close enough to attack
    if (DistanceToPlayer <= AttackDistance)
    {
        SetState(ENPC_TRexState::Attack);
        return;
    }
    
    // Continue chasing
    MoveToLocation(Player->GetActorLocation(), ChaseSpeed);
}

void UNPC_TRexBehaviorComponent::UpdateAttackBehavior(float DeltaTime)
{
    APawn* Player = FindNearestPlayer();
    if (!Player)
    {
        SetState(ENPC_TRexState::Return);
        return;
    }
    
    float DistanceToPlayer = GetDistanceToPlayer();
    
    // If player moved away, go back to chasing
    if (DistanceToPlayer > AttackDistance * 1.2f) // Add hysteresis
    {
        SetState(ENPC_TRexState::Chase);
        return;
    }
    
    // Perform attack if cooldown is ready
    if (CanAttack())
    {
        PerformAttack();
    }
    
    // Move slightly towards player to maintain attack range
    MoveToLocation(Player->GetActorLocation(), MovementSpeed * 0.5f);
}

void UNPC_TRexBehaviorComponent::UpdateReturnBehavior(float DeltaTime)
{
    // Check if player comes back into range
    APawn* Player = FindNearestPlayer();
    if (Player)
    {
        float DistanceToPlayer = GetDistanceToPlayer();
        if (DistanceToPlayer <= ChaseDistance)
        {
            SetState(ENPC_TRexState::Chase);
            return;
        }
    }
    
    // Return to home location
    if (IsAtLocation(HomeLocation))
    {
        SetState(ENPC_TRexState::Patrol);
        CurrentTarget = GeneratePatrolPoint();
    }
    else
    {
        MoveToLocation(HomeLocation, MovementSpeed);
    }
}

void UNPC_TRexBehaviorComponent::MoveToLocation(const FVector& TargetLocation, float Speed)
{
    if (!GetOwner())
        return;
        
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
        return;
        
    FVector Direction = (TargetLocation - GetOwner()->GetActorLocation()).GetSafeNormal();
    Character->AddMovementInput(Direction, 1.0f);
    
    // Face movement direction
    if (!Direction.IsNearlyZero())
    {
        FRotator TargetRotation = Direction.Rotation();
        FRotator CurrentRotation = GetOwner()->GetActorRotation();
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), 2.0f);
        GetOwner()->SetActorRotation(NewRotation);
    }
}

bool UNPC_TRexBehaviorComponent::IsAtLocation(const FVector& Location, float Tolerance) const
{
    if (!GetOwner())
        return false;
        
    return FVector::Dist(GetOwner()->GetActorLocation(), Location) <= Tolerance;
}

float UNPC_TRexBehaviorComponent::GetDistanceToPlayer() const
{
    APawn* Player = const_cast<UNPC_TRexBehaviorComponent*>(this)->FindNearestPlayer();
    if (!Player || !GetOwner())
        return FLT_MAX;
        
    return FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
}