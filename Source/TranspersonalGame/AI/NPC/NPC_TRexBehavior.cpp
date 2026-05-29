#include "NPC_TRexBehavior.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/PrimitiveComponent.h"

UNPC_TRexBehavior::UNPC_TRexBehavior()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    CurrentState = ENPC_TRexState::Patrol;
    ChaseDistance = 3000.0f;
    AttackDistance = 300.0f;
    ChaseSpeed = 600.0f;
    AttackCooldown = 3.0f;
    LastAttackTime = 0.0f;
    TargetPlayer = nullptr;
    CurrentPatrolTarget = FVector::ZeroVector;
    
    PatrolData.PatrolCenter = FVector::ZeroVector;
    PatrolData.PatrolRadius = 5000.0f;
    PatrolData.PatrolSpeed = 300.0f;
}

void UNPC_TRexBehavior::BeginPlay()
{
    Super::BeginPlay();
    
    // Set initial patrol center to T-Rex spawn location
    if (GetOwner())
    {
        PatrolData.PatrolCenter = GetOwner()->GetActorLocation();
        CurrentPatrolTarget = GetRandomPatrolPoint();
    }
}

void UNPC_TRexBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner())
    {
        return;
    }

    // Update target player reference
    TargetPlayer = FindNearestPlayer();
    
    // State machine update
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

void UNPC_TRexBehavior::UpdatePatrolBehavior(float DeltaTime)
{
    // Check if player is in chase range
    if (TargetPlayer && IsPlayerInRange(ChaseDistance))
    {
        CurrentState = ENPC_TRexState::Chase;
        return;
    }
    
    // Move towards current patrol target
    if (CurrentPatrolTarget != FVector::ZeroVector)
    {
        MoveToLocation(CurrentPatrolTarget, PatrolData.PatrolSpeed);
        
        // Check if reached patrol target
        FVector OwnerLocation = GetOwner()->GetActorLocation();
        float DistanceToTarget = FVector::Dist(OwnerLocation, CurrentPatrolTarget);
        
        if (DistanceToTarget < 500.0f)
        {
            CurrentPatrolTarget = GetRandomPatrolPoint();
        }
    }
}

void UNPC_TRexBehavior::UpdateChaseBehavior(float DeltaTime)
{
    if (!TargetPlayer)
    {
        CurrentState = ENPC_TRexState::Return;
        return;
    }
    
    // Check if player is in attack range
    if (IsPlayerInRange(AttackDistance))
    {
        CurrentState = ENPC_TRexState::Attack;
        return;
    }
    
    // Check if player is too far away
    if (!IsPlayerInRange(ChaseDistance * 1.5f))
    {
        CurrentState = ENPC_TRexState::Return;
        return;
    }
    
    // Chase the player
    FVector PlayerLocation = TargetPlayer->GetActorLocation();
    MoveToLocation(PlayerLocation, ChaseSpeed);
}

void UNPC_TRexBehavior::UpdateAttackBehavior(float DeltaTime)
{
    if (!TargetPlayer)
    {
        CurrentState = ENPC_TRexState::Return;
        return;
    }
    
    // Check if player moved out of attack range
    if (!IsPlayerInRange(AttackDistance))
    {
        CurrentState = ENPC_TRexState::Chase;
        return;
    }
    
    // Execute attack if cooldown is ready
    if (CanAttack())
    {
        ExecuteAttack();
    }
}

void UNPC_TRexBehavior::UpdateReturnBehavior(float DeltaTime)
{
    // Check if player is back in chase range
    if (TargetPlayer && IsPlayerInRange(ChaseDistance))
    {
        CurrentState = ENPC_TRexState::Chase;
        return;
    }
    
    // Move back to patrol center
    MoveToLocation(PatrolData.PatrolCenter, PatrolData.PatrolSpeed);
    
    // Check if reached patrol center
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    float DistanceToCenter = FVector::Dist(OwnerLocation, PatrolData.PatrolCenter);
    
    if (DistanceToCenter < 1000.0f)
    {
        CurrentState = ENPC_TRexState::Patrol;
        CurrentPatrolTarget = GetRandomPatrolPoint();
    }
}

APawn* UNPC_TRexBehavior::FindNearestPlayer()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (PlayerController && PlayerController->GetPawn())
    {
        return PlayerController->GetPawn();
    }
    
    return nullptr;
}

FVector UNPC_TRexBehavior::GetRandomPatrolPoint()
{
    FVector RandomDirection = UKismetMathLibrary::RandomUnitVector();
    float RandomDistance = FMath::RandRange(PatrolData.PatrolRadius * 0.3f, PatrolData.PatrolRadius);
    
    FVector PatrolPoint = PatrolData.PatrolCenter + (RandomDirection * RandomDistance);
    PatrolPoint.Z = PatrolData.PatrolCenter.Z; // Keep same height
    
    return PatrolPoint;
}

bool UNPC_TRexBehavior::IsPlayerInRange(float Range)
{
    if (!TargetPlayer || !GetOwner())
    {
        return false;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector PlayerLocation = TargetPlayer->GetActorLocation();
    float Distance = FVector::Dist(OwnerLocation, PlayerLocation);
    
    return Distance <= Range;
}

void UNPC_TRexBehavior::MoveToLocation(FVector TargetLocation, float Speed)
{
    if (!GetOwner())
    {
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector Direction = (TargetLocation - OwnerLocation).GetSafeNormal();
    
    // Rotate towards target
    FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(OwnerLocation, TargetLocation);
    GetOwner()->SetActorRotation(FMath::RInterpTo(GetOwner()->GetActorRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), 2.0f));
    
    // Move towards target
    FVector NewLocation = OwnerLocation + (Direction * Speed * GetWorld()->GetDeltaSeconds());
    GetOwner()->SetActorLocation(NewLocation);
}

void UNPC_TRexBehavior::SetPatrolCenter(FVector NewCenter)
{
    PatrolData.PatrolCenter = NewCenter;
    CurrentPatrolTarget = GetRandomPatrolPoint();
}

void UNPC_TRexBehavior::SetPatrolRadius(float NewRadius)
{
    PatrolData.PatrolRadius = FMath::Clamp(NewRadius, 1000.0f, 20000.0f);
}

bool UNPC_TRexBehavior::CanAttack() const
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    return (CurrentTime - LastAttackTime) >= AttackCooldown;
}

void UNPC_TRexBehavior::ExecuteAttack()
{
    if (!TargetPlayer || !GetOwner())
    {
        return;
    }
    
    LastAttackTime = GetWorld()->GetTimeSeconds();
    
    // Log attack for debugging
    UE_LOG(LogTemp, Warning, TEXT("T-Rex %s attacking player at distance %f"), 
           *GetOwner()->GetName(), 
           FVector::Dist(GetOwner()->GetActorLocation(), TargetPlayer->GetActorLocation()));
    
    // TODO: Implement actual damage dealing when combat system is ready
    // For now, just log the attack
}