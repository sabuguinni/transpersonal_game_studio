#include "NPC_TRexBehavior.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/PrimitiveComponent.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"

UNPC_TRexBehavior::UNPC_TRexBehavior()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default stats
    Stats.Health = 1000.0f;
    Stats.Hunger = 50.0f;
    Stats.Aggression = 75.0f;
    Stats.Fear = 10.0f;
    Stats.Energy = 100.0f;
    
    // Initialize behavior parameters
    PatrolRadius = 5000.0f;
    DetectionRange = 3000.0f;
    AttackRange = 300.0f;
    MovementSpeed = 600.0f;
    AttackCooldown = 3.0f;
    
    // Initialize state
    CurrentState = ENPC_TRexState::Idle;
    bIsMoving = false;
    bCanAttack = true;
    bHasTarget = false;
}

void UNPC_TRexBehavior::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTerritoryCenter();
    SetState(ENPC_TRexState::Patrolling);
}

void UNPC_TRexBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateBehavior(DeltaTime);
    UpdateStats(DeltaTime);
}

void UNPC_TRexBehavior::UpdateBehavior(float DeltaTime)
{
    StateTimer += DeltaTime;
    
    // Check for player detection first
    if (CurrentState != ENPC_TRexState::Attacking && CurrentState != ENPC_TRexState::Feeding)
    {
        AActor* DetectedTarget = DetectNearbyTargets();
        if (DetectedTarget)
        {
            TargetActor = DetectedTarget;
            bHasTarget = true;
            SetState(ENPC_TRexState::Hunting);
        }
    }
    
    // Execute current state behavior
    switch (CurrentState)
    {
        case ENPC_TRexState::Idle:
            HandleIdleBehavior(DeltaTime);
            break;
        case ENPC_TRexState::Patrolling:
            HandlePatrolBehavior(DeltaTime);
            break;
        case ENPC_TRexState::Hunting:
            HandleHuntingBehavior(DeltaTime);
            break;
        case ENPC_TRexState::Attacking:
            HandleAttackingBehavior(DeltaTime);
            break;
        case ENPC_TRexState::Feeding:
            HandleFeedingBehavior(DeltaTime);
            break;
        case ENPC_TRexState::Fleeing:
            // Implement fleeing behavior if needed
            break;
    }
}

void UNPC_TRexBehavior::SetState(ENPC_TRexState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;
        
        // State entry actions
        switch (NewState)
        {
            case ENPC_TRexState::Idle:
                bIsMoving = false;
                break;
            case ENPC_TRexState::Patrolling:
                CurrentTarget = GetRandomPatrolPoint();
                bIsMoving = true;
                break;
            case ENPC_TRexState::Hunting:
                PlayRoarSound();
                bIsMoving = true;
                break;
            case ENPC_TRexState::Attacking:
                bIsMoving = false;
                break;
            case ENPC_TRexState::Feeding:
                bIsMoving = false;
                Stats.Hunger = FMath::Clamp(Stats.Hunger + 30.0f, 0.0f, 100.0f);
                break;
        }
    }
}

void UNPC_TRexBehavior::HandleIdleBehavior(float DeltaTime)
{
    IdleTimer += DeltaTime;
    
    // After 5 seconds of idle, start patrolling
    if (IdleTimer >= 5.0f)
    {
        IdleTimer = 0.0f;
        SetState(ENPC_TRexState::Patrolling);
    }
}

void UNPC_TRexBehavior::HandlePatrolBehavior(float DeltaTime)
{
    if (!bIsMoving)
    {
        CurrentTarget = GetRandomPatrolPoint();
        bIsMoving = true;
    }
    
    // Move towards patrol point
    MoveToLocation(CurrentTarget);
    
    // Check if reached patrol point
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    float DistanceToTarget = FVector::Dist(OwnerLocation, CurrentTarget);
    
    if (DistanceToTarget < 200.0f)
    {
        // Reached patrol point, idle for a bit
        SetState(ENPC_TRexState::Idle);
    }
    
    // Patrol for maximum 30 seconds before changing direction
    if (StateTimer > 30.0f)
    {
        CurrentTarget = GetRandomPatrolPoint();
        StateTimer = 0.0f;
    }
}

void UNPC_TRexBehavior::HandleHuntingBehavior(float DeltaTime)
{
    if (!TargetActor || !bHasTarget)
    {
        SetState(ENPC_TRexState::Patrolling);
        return;
    }
    
    float DistanceToTarget = GetDistanceToPlayer();
    
    // If target is within attack range, attack
    if (DistanceToTarget <= AttackRange && bCanAttack)
    {
        SetState(ENPC_TRexState::Attacking);
        return;
    }
    
    // If target is too far, lose interest
    if (DistanceToTarget > DetectionRange * 1.5f)
    {
        TargetActor = nullptr;
        bHasTarget = false;
        SetState(ENPC_TRexState::Patrolling);
        return;
    }
    
    // Move towards target
    MoveToLocation(TargetActor->GetActorLocation());
    RotateTowardsTarget(TargetActor->GetActorLocation(), DeltaTime);
}

void UNPC_TRexBehavior::HandleAttackingBehavior(float DeltaTime)
{
    if (!TargetActor || !bHasTarget)
    {
        SetState(ENPC_TRexState::Patrolling);
        return;
    }
    
    float DistanceToTarget = GetDistanceToPlayer();
    
    // If target moved out of attack range, chase again
    if (DistanceToTarget > AttackRange)
    {
        SetState(ENPC_TRexState::Hunting);
        return;
    }
    
    // Perform attack
    if (bCanAttack)
    {
        PerformAttack();
        bCanAttack = false;
        LastAttackTime = GetWorld()->GetTimeSeconds();
    }
    
    // Check attack cooldown
    if (GetWorld()->GetTimeSeconds() - LastAttackTime >= AttackCooldown)
    {
        bCanAttack = true;
    }
    
    // After attacking for 10 seconds, go back to hunting
    if (StateTimer > 10.0f)
    {
        SetState(ENPC_TRexState::Hunting);
    }
}

void UNPC_TRexBehavior::HandleFeedingBehavior(float DeltaTime)
{
    // Feed for 15 seconds then go back to patrolling
    if (StateTimer > 15.0f)
    {
        SetState(ENPC_TRexState::Patrolling);
    }
}

AActor* UNPC_TRexBehavior::DetectNearbyTargets()
{
    if (!GetWorld())
        return nullptr;
    
    // Find player character
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
        return nullptr;
    
    APawn* PlayerPawn = PlayerController->GetPawn();
    if (!PlayerPawn)
        return nullptr;
    
    // Check if player is in detection range
    if (IsPlayerInRange(DetectionRange))
    {
        return PlayerPawn;
    }
    
    return nullptr;
}

bool UNPC_TRexBehavior::IsPlayerInRange(float Range) const
{
    float Distance = GetDistanceToPlayer();
    return Distance <= Range && Distance > 0.0f;
}

float UNPC_TRexBehavior::GetDistanceToPlayer() const
{
    if (!GetWorld())
        return -1.0f;
    
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
        return -1.0f;
    
    APawn* PlayerPawn = PlayerController->GetPawn();
    if (!PlayerPawn)
        return -1.0f;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    return FVector::Dist(OwnerLocation, PlayerLocation);
}

void UNPC_TRexBehavior::MoveToLocation(const FVector& TargetLocation)
{
    if (!GetOwner())
        return;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector Direction = (TargetLocation - OwnerLocation).GetSafeNormal();
    
    // Simple movement - in a real implementation, use AI Controller and pathfinding
    FVector NewLocation = OwnerLocation + (Direction * MovementSpeed * GetWorld()->GetDeltaSeconds());
    GetOwner()->SetActorLocation(NewLocation);
    
    // Rotate towards movement direction
    RotateTowardsTarget(TargetLocation, GetWorld()->GetDeltaSeconds());
}

void UNPC_TRexBehavior::PatrolAroundTerritory()
{
    CurrentTarget = GetRandomPatrolPoint();
    MoveToLocation(CurrentTarget);
}

FVector UNPC_TRexBehavior::GetRandomPatrolPoint() const
{
    // Generate random point within patrol radius
    float RandomAngle = FMath::RandRange(0.0f, 360.0f);
    float RandomDistance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    
    FVector RandomOffset;
    RandomOffset.X = FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * RandomDistance;
    RandomOffset.Y = FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * RandomDistance;
    RandomOffset.Z = 0.0f;
    
    return TerritoryCenter + RandomOffset;
}

void UNPC_TRexBehavior::PerformAttack()
{
    // Random attack type
    float AttackRoll = FMath::RandRange(0.0f, 1.0f);
    
    if (AttackRoll < 0.7f)
    {
        PerformBiteAttack();
    }
    else
    {
        PerformTailSwipe();
    }
    
    PlayRoarSound();
}

void UNPC_TRexBehavior::PerformBiteAttack()
{
    // Implement bite attack logic
    // This would typically involve:
    // - Playing bite animation
    // - Dealing damage to target
    // - Playing sound effects
    
    if (TargetActor)
    {
        // Simple damage application - in real game, use damage system
        UE_LOG(LogTemp, Warning, TEXT("T-Rex performs bite attack on %s"), *TargetActor->GetName());
    }
}

void UNPC_TRexBehavior::PerformTailSwipe()
{
    // Implement tail swipe attack logic
    // This would typically involve:
    // - Playing tail swipe animation
    // - Area of effect damage
    // - Knockback effect
    
    UE_LOG(LogTemp, Warning, TEXT("T-Rex performs tail swipe attack"));
}

void UNPC_TRexBehavior::PlayRoarSound()
{
    // Play T-Rex roar sound
    // In a real implementation, this would use audio components
    UE_LOG(LogTemp, Warning, TEXT("T-Rex ROARS!"));
}

void UNPC_TRexBehavior::PlayFootstepSound()
{
    // Play footstep sound
    UE_LOG(LogTemp, Warning, TEXT("T-Rex footstep"));
}

void UNPC_TRexBehavior::InitializeTerritoryCenter()
{
    if (GetOwner())
    {
        TerritoryCenter = GetOwner()->GetActorLocation();
    }
}

void UNPC_TRexBehavior::UpdateStats(float DeltaTime)
{
    // Decrease hunger over time
    Stats.Hunger = FMath::Clamp(Stats.Hunger - (DeltaTime * 0.1f), 0.0f, 100.0f);
    
    // Decrease energy when moving
    if (bIsMoving)
    {
        Stats.Energy = FMath::Clamp(Stats.Energy - (DeltaTime * 0.5f), 0.0f, 100.0f);
    }
    else
    {
        // Recover energy when not moving
        Stats.Energy = FMath::Clamp(Stats.Energy + (DeltaTime * 0.2f), 0.0f, 100.0f);
    }
    
    // Adjust aggression based on hunger
    if (Stats.Hunger < 20.0f)
    {
        Stats.Aggression = FMath::Clamp(Stats.Aggression + (DeltaTime * 2.0f), 0.0f, 100.0f);
    }
    else
    {
        Stats.Aggression = FMath::Clamp(Stats.Aggression - (DeltaTime * 0.5f), 0.0f, 100.0f);
    }
}

bool UNPC_TRexBehavior::IsInTerritory(const FVector& Location) const
{
    float DistanceFromCenter = FVector::Dist(Location, TerritoryCenter);
    return DistanceFromCenter <= PatrolRadius;
}

void UNPC_TRexBehavior::RotateTowardsTarget(const FVector& TargetLocation, float DeltaTime)
{
    if (!GetOwner())
        return;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector Direction = (TargetLocation - OwnerLocation).GetSafeNormal();
    
    if (!Direction.IsNearlyZero())
    {
        FRotator TargetRotation = Direction.Rotation();
        FRotator CurrentRotation = GetOwner()->GetActorRotation();
        
        // Smooth rotation
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 2.0f);
        GetOwner()->SetActorRotation(NewRotation);
    }
}