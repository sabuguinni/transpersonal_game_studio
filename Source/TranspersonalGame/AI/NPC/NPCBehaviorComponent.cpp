#include "NPCBehaviorComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeSpeciesBehavior();
    SetBehaviorState(ENPC_BehaviorState::Patrol);
    bIsInitialized = true;
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsInitialized)
    {
        return;
    }

    // Update behavior based on current state
    switch (CurrentBehaviorState)
    {
        case ENPC_BehaviorState::Idle:
            UpdateIdleBehavior(DeltaTime);
            break;
        case ENPC_BehaviorState::Patrol:
            UpdatePatrolBehavior(DeltaTime);
            break;
        case ENPC_BehaviorState::Chase:
            UpdateChaseBehavior(DeltaTime);
            break;
        case ENPC_BehaviorState::Attack:
            UpdateAttackBehavior(DeltaTime);
            break;
        case ENPC_BehaviorState::Flee:
            UpdateFleeBehavior(DeltaTime);
            break;
        default:
            break;
    }

    // Update timers
    StateChangeTimer += DeltaTime;
    if (CurrentTarget)
    {
        TimeSinceLastTargetSeen += DeltaTime;
    }
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        StateChangeTimer = 0.0f;
        
        // Log state change for debugging
        UE_LOG(LogTemp, Log, TEXT("NPC %s changed behavior to %d"), 
               *GetOwner()->GetName(), 
               static_cast<int32>(NewState));
    }
}

void UNPCBehaviorComponent::StartPatrol()
{
    SetBehaviorState(ENPC_BehaviorState::Patrol);
    CurrentTarget = nullptr;
    TimeSinceLastTargetSeen = 0.0f;
}

void UNPCBehaviorComponent::StartChase(AActor* Target)
{
    if (Target)
    {
        CurrentTarget = Target;
        LastKnownTargetLocation = Target->GetActorLocation();
        TimeSinceLastTargetSeen = 0.0f;
        SetBehaviorState(ENPC_BehaviorState::Chase);
    }
}

void UNPCBehaviorComponent::StartAttack(AActor* Target)
{
    if (Target)
    {
        CurrentTarget = Target;
        SetBehaviorState(ENPC_BehaviorState::Attack);
    }
}

void UNPCBehaviorComponent::StartFlee()
{
    SetBehaviorState(ENPC_BehaviorState::Flee);
    CurrentTarget = nullptr;
}

bool UNPCBehaviorComponent::IsPlayerInDetectionRange() const
{
    AActor* Player = FindNearestPlayer();
    if (!Player)
    {
        return false;
    }

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    return Distance <= DetectionRadius;
}

bool UNPCBehaviorComponent::IsPlayerInChaseRange() const
{
    AActor* Player = FindNearestPlayer();
    if (!Player)
    {
        return false;
    }

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    return Distance <= ChaseRadius;
}

bool UNPCBehaviorComponent::IsPlayerInAttackRange() const
{
    AActor* Player = FindNearestPlayer();
    if (!Player)
    {
        return false;
    }

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    return Distance <= AttackRadius;
}

AActor* UNPCBehaviorComponent::FindNearestPlayer() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    // Find player pawn
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    return PlayerPawn;
}

void UNPCBehaviorComponent::UpdateBehaviorBasedOnSpecies()
{
    switch (DinosaurSpecies)
    {
        case ENPC_DinosaurSpecies::TRex:
            BehaviorStats.Aggression = 90.0f;
            BehaviorStats.Territorial = 95.0f;
            BehaviorStats.PackInstinct = 10.0f;
            DetectionRadius = 2000.0f;
            ChaseRadius = 4000.0f;
            AttackRadius = 400.0f;
            PatrolData.PatrolRadius = 3000.0f;
            PatrolData.PatrolSpeed = 400.0f;
            break;
            
        case ENPC_DinosaurSpecies::Velociraptor:
            BehaviorStats.Aggression = 80.0f;
            BehaviorStats.PackInstinct = 90.0f;
            BehaviorStats.HuntingSkill = 95.0f;
            DetectionRadius = 1500.0f;
            ChaseRadius = 3000.0f;
            AttackRadius = 250.0f;
            PatrolData.PatrolRadius = 2000.0f;
            PatrolData.PatrolSpeed = 600.0f;
            break;
            
        case ENPC_DinosaurSpecies::Triceratops:
            BehaviorStats.Aggression = 40.0f;
            BehaviorStats.Territorial = 70.0f;
            BehaviorStats.FearThreshold = 60.0f;
            DetectionRadius = 1000.0f;
            ChaseRadius = 1500.0f;
            AttackRadius = 300.0f;
            PatrolData.PatrolRadius = 1500.0f;
            PatrolData.PatrolSpeed = 250.0f;
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
            BehaviorStats.Aggression = 10.0f;
            BehaviorStats.Curiosity = 80.0f;
            BehaviorStats.FearThreshold = 30.0f;
            DetectionRadius = 800.0f;
            ChaseRadius = 0.0f; // Doesn't chase
            AttackRadius = 0.0f; // Doesn't attack
            PatrolData.PatrolRadius = 4000.0f;
            PatrolData.PatrolSpeed = 150.0f;
            break;
            
        default:
            break;
    }
}

void UNPCBehaviorComponent::UpdateIdleBehavior(float DeltaTime)
{
    // Check for nearby players
    if (IsPlayerInDetectionRange())
    {
        AActor* Player = FindNearestPlayer();
        if (Player && BehaviorStats.Aggression > 50.0f)
        {
            StartChase(Player);
        }
    }

    // Return to patrol after idle time
    if (StateChangeTimer > 3.0f)
    {
        StartPatrol();
    }
}

void UNPCBehaviorComponent::UpdatePatrolBehavior(float DeltaTime)
{
    // Check for player detection
    if (IsPlayerInDetectionRange())
    {
        AActor* Player = FindNearestPlayer();
        if (Player)
        {
            if (BehaviorStats.Aggression > BehaviorStats.FearThreshold)
            {
                StartChase(Player);
                return;
            }
            else if (BehaviorStats.FearThreshold > BehaviorStats.Aggression)
            {
                StartFlee();
                return;
            }
        }
    }

    // Continue patrol movement
    if (PatrolData.PatrolPoints.Num() > 0)
    {
        FVector TargetPoint = PatrolData.PatrolPoints[CurrentPatrolIndex];
        MoveToLocation(TargetPoint);

        if (HasReachedLocation(TargetPoint))
        {
            CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolData.PatrolPoints.Num();
        }
    }
    else
    {
        // Generate random patrol points if none exist
        FVector CurrentLocation = GetOwner()->GetActorLocation();
        for (int32 i = 0; i < 4; i++)
        {
            FVector RandomOffset = FVector(
                FMath::RandRange(-PatrolData.PatrolRadius, PatrolData.PatrolRadius),
                FMath::RandRange(-PatrolData.PatrolRadius, PatrolData.PatrolRadius),
                0.0f
            );
            PatrolData.PatrolPoints.Add(CurrentLocation + RandomOffset);
        }
    }
}

void UNPCBehaviorComponent::UpdateChaseBehavior(float DeltaTime)
{
    if (!CurrentTarget)
    {
        StartPatrol();
        return;
    }

    // Check if target is still in chase range
    if (!IsPlayerInChaseRange())
    {
        TimeSinceLastTargetSeen += DeltaTime;
        if (TimeSinceLastTargetSeen > LoseTargetTime)
        {
            StartPatrol();
            return;
        }
    }
    else
    {
        TimeSinceLastTargetSeen = 0.0f;
        LastKnownTargetLocation = CurrentTarget->GetActorLocation();
    }

    // Check if close enough to attack
    if (IsPlayerInAttackRange())
    {
        StartAttack(CurrentTarget);
        return;
    }

    // Move towards target
    MoveToLocation(LastKnownTargetLocation);
}

void UNPCBehaviorComponent::UpdateAttackBehavior(float DeltaTime)
{
    if (!CurrentTarget)
    {
        StartPatrol();
        return;
    }

    // Check if target moved out of attack range
    if (!IsPlayerInAttackRange())
    {
        StartChase(CurrentTarget);
        return;
    }

    // Perform attack animation/logic here
    // For now, just log the attack
    if (StateChangeTimer > 2.0f) // Attack duration
    {
        UE_LOG(LogTemp, Warning, TEXT("NPC %s attacks target!"), *GetOwner()->GetName());
        
        // After attack, decide next action
        if (BehaviorStats.Aggression > 70.0f)
        {
            StartChase(CurrentTarget); // Continue aggressive behavior
        }
        else
        {
            StartPatrol(); // Return to normal behavior
        }
    }
}

void UNPCBehaviorComponent::UpdateFleeBehavior(float DeltaTime)
{
    // Move away from player
    AActor* Player = FindNearestPlayer();
    if (Player)
    {
        FVector FleeDirection = GetOwner()->GetActorLocation() - Player->GetActorLocation();
        FleeDirection.Normalize();
        FVector FleeTarget = GetOwner()->GetActorLocation() + (FleeDirection * 2000.0f);
        MoveToLocation(FleeTarget);
    }

    // Return to patrol after fleeing for a while
    if (StateChangeTimer > 8.0f)
    {
        StartPatrol();
    }
}

void UNPCBehaviorComponent::InitializeSpeciesBehavior()
{
    UpdateBehaviorBasedOnSpecies();
    
    // Initialize patrol points around current location
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    PatrolData.PatrolPoints.Empty();
    
    for (int32 i = 0; i < 4; i++)
    {
        float Angle = (i * 90.0f) * PI / 180.0f;
        FVector PatrolPoint = CurrentLocation + FVector(
            FMath::Cos(Angle) * PatrolData.PatrolRadius * 0.7f,
            FMath::Sin(Angle) * PatrolData.PatrolRadius * 0.7f,
            0.0f
        );
        PatrolData.PatrolPoints.Add(PatrolPoint);
    }
}

FVector UNPCBehaviorComponent::GetNextPatrolPoint()
{
    if (PatrolData.PatrolPoints.Num() == 0)
    {
        return GetOwner()->GetActorLocation();
    }

    if (PatrolData.bRandomPatrol)
    {
        int32 RandomIndex = FMath::RandRange(0, PatrolData.PatrolPoints.Num() - 1);
        return PatrolData.PatrolPoints[RandomIndex];
    }
    else
    {
        return PatrolData.PatrolPoints[CurrentPatrolIndex];
    }
}

void UNPCBehaviorComponent::MoveToLocation(const FVector& TargetLocation)
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        return;
    }

    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (!MovementComp)
    {
        return;
    }

    // Calculate direction to target
    FVector Direction = TargetLocation - GetOwner()->GetActorLocation();
    Direction.Z = 0.0f; // Keep movement on ground plane
    Direction.Normalize();

    // Set movement speed based on current behavior
    float DesiredSpeed = PatrolData.PatrolSpeed;
    switch (CurrentBehaviorState)
    {
        case ENPC_BehaviorState::Chase:
            DesiredSpeed *= 1.5f; // Faster when chasing
            break;
        case ENPC_BehaviorState::Flee:
            DesiredSpeed *= 2.0f; // Fastest when fleeing
            break;
        case ENPC_BehaviorState::Attack:
            DesiredSpeed *= 0.5f; // Slower when attacking
            break;
        default:
            break;
    }

    MovementComp->MaxWalkSpeed = DesiredSpeed;

    // Add movement input
    OwnerCharacter->AddMovementInput(Direction, 1.0f);

    // Rotate to face movement direction
    if (!Direction.IsZero())
    {
        FRotator TargetRotation = Direction.Rotation();
        FRotator CurrentRotation = GetOwner()->GetActorRotation();
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), 3.0f);
        GetOwner()->SetActorRotation(NewRotation);
    }
}

bool UNPCBehaviorComponent::HasReachedLocation(const FVector& TargetLocation, float Tolerance) const
{
    float Distance = FVector::Dist2D(GetOwner()->GetActorLocation(), TargetLocation);
    return Distance <= Tolerance;
}