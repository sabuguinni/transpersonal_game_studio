// TRexBehaviorComponent.cpp
// Agent #11 — NPC Behavior Agent
// CYCLE: PROD_CYCLE_AUTO_20260629_006
//
// T-Rex behavioral brain:
//   - Patrols 5000-unit radius territory
//   - Chases player when within 3000 units
//   - Attacks when within 300 units
//   - Persistent threat memory (30s decay)
//   - Calls UDinosaurAnimInstance::SetDinoAIState() for animation coupling
//   - Calls UDinosaurAnimInstance::TriggerAttackMontage(0) for bite attacks

#include "TRexBehaviorComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"

UNPC_TRexBehaviorComponent::UNPC_TRexBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for large predator

    PatrolRadius = 5000.0f;
    ChaseRadius = 3000.0f;
    AttackRadius = 300.0f;
    PatrolSpeed = 200.0f;
    ChaseSpeed = 650.0f;
    AttackCooldown = 2.5f;

    CurrentState = ENPC_TRexState::Idle;
    bHasPatrolOrigin = false;
    LastAttackTime = -999.0f;
    ThreatMemoryDecayTime = 30.0f;
    LastThreatTime = -999.0f;
    bPlayerInMemory = false;
}

void UNPC_TRexBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache owner pawn
    OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("TRexBehaviorComponent: Owner is not a Pawn!"));
        return;
    }

    // Store patrol origin at spawn location
    PatrolOrigin = OwnerPawn->GetActorLocation();
    bHasPatrolOrigin = true;

    // Cache AI controller
    OwnerAIController = Cast<AAIController>(OwnerPawn->GetController());

    // Cache movement component
    OwnerMovement = OwnerPawn->FindComponentByClass<UCharacterMovementComponent>();

    // Cache anim instance reference (set externally by DinosaurAnimInstance)
    // Agent #10 sets this via SetDinoAIState coupling

    UE_LOG(LogTemp, Log, TEXT("TRexBehaviorComponent: Initialized at origin %s"), *PatrolOrigin.ToString());

    // Start in patrol state
    TransitionToState(ENPC_TRexState::Patrol);
}

void UNPC_TRexBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerPawn) return;

    // Decay threat memory
    float Now = GetWorld()->GetTimeSeconds();
    if (bPlayerInMemory && (Now - LastThreatTime) > ThreatMemoryDecayTime)
    {
        bPlayerInMemory = false;
        UE_LOG(LogTemp, Log, TEXT("TRexBehaviorComponent: Player memory decayed — returning to patrol"));
    }

    // Sense player
    SensePlayer();

    // Execute current state
    switch (CurrentState)
    {
        case ENPC_TRexState::Idle:   TickIdle(DeltaTime);   break;
        case ENPC_TRexState::Patrol: TickPatrol(DeltaTime); break;
        case ENPC_TRexState::Alert:  TickAlert(DeltaTime);  break;
        case ENPC_TRexState::Chase:  TickChase(DeltaTime);  break;
        case ENPC_TRexState::Attack: TickAttack(DeltaTime); break;
        case ENPC_TRexState::Return: TickReturn(DeltaTime); break;
        default: break;
    }
}

void UNPC_TRexBehaviorComponent::SensePlayer()
{
    if (!OwnerPawn) return;

    ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!Player) return;

    float DistToPlayer = FVector::Dist(OwnerPawn->GetActorLocation(), Player->GetActorLocation());

    if (DistToPlayer <= ChaseRadius)
    {
        // Player detected — update threat memory
        LastThreatTime = GetWorld()->GetTimeSeconds();
        bPlayerInMemory = true;
        LastKnownPlayerLocation = Player->GetActorLocation();

        if (DistToPlayer <= AttackRadius)
        {
            TransitionToState(ENPC_TRexState::Attack);
        }
        else
        {
            TransitionToState(ENPC_TRexState::Chase);
        }
    }
    else if (bPlayerInMemory)
    {
        // Player out of sense range but in memory — investigate last known location
        if (CurrentState != ENPC_TRexState::Chase && CurrentState != ENPC_TRexState::Attack)
        {
            TransitionToState(ENPC_TRexState::Alert);
        }
    }
}

void UNPC_TRexBehaviorComponent::TickIdle(float DeltaTime)
{
    // After 3 seconds idle, begin patrol
    IdleTimer += DeltaTime;
    if (IdleTimer > 3.0f)
    {
        IdleTimer = 0.0f;
        TransitionToState(ENPC_TRexState::Patrol);
    }
}

void UNPC_TRexBehaviorComponent::TickPatrol(float DeltaTime)
{
    if (!OwnerAIController) return;

    // If we've reached the patrol target (or don't have one), pick a new one
    float DistToTarget = FVector::Dist(OwnerPawn->GetActorLocation(), CurrentPatrolTarget);
    if (DistToTarget < 200.0f || !bHasPatrolTarget)
    {
        PickNewPatrolTarget();
    }

    // Move toward patrol target
    OwnerAIController->MoveToLocation(CurrentPatrolTarget, 150.0f);

    // Set movement speed
    if (OwnerMovement)
    {
        OwnerMovement->MaxWalkSpeed = PatrolSpeed;
    }
}

void UNPC_TRexBehaviorComponent::TickAlert(float DeltaTime)
{
    // Move toward last known player location
    if (!OwnerAIController) return;

    float DistToLastKnown = FVector::Dist(OwnerPawn->GetActorLocation(), LastKnownPlayerLocation);
    if (DistToLastKnown < 300.0f)
    {
        // Reached last known location — player gone, return to patrol
        if (!bPlayerInMemory)
        {
            TransitionToState(ENPC_TRexState::Return);
        }
    }
    else
    {
        OwnerAIController->MoveToLocation(LastKnownPlayerLocation, 200.0f);
        if (OwnerMovement)
        {
            OwnerMovement->MaxWalkSpeed = PatrolSpeed * 1.5f;
        }
    }
}

void UNPC_TRexBehaviorComponent::TickChase(float DeltaTime)
{
    if (!OwnerAIController) return;

    ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!Player)
    {
        TransitionToState(ENPC_TRexState::Alert);
        return;
    }

    // Chase player directly
    OwnerAIController->MoveToActor(Player, 250.0f);

    if (OwnerMovement)
    {
        OwnerMovement->MaxWalkSpeed = ChaseSpeed;
    }

    // Roar when entering chase (one-time)
    if (bJustEnteredChase)
    {
        bJustEnteredChase = false;
        BroadcastRoar();
    }
}

void UNPC_TRexBehaviorComponent::TickAttack(float DeltaTime)
{
    float Now = GetWorld()->GetTimeSeconds();
    if ((Now - LastAttackTime) < AttackCooldown) return;

    ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!Player) return;

    float DistToPlayer = FVector::Dist(OwnerPawn->GetActorLocation(), Player->GetActorLocation());
    if (DistToPlayer > AttackRadius * 1.5f)
    {
        // Player escaped attack range — resume chase
        TransitionToState(ENPC_TRexState::Chase);
        return;
    }

    // Execute bite attack
    LastAttackTime = Now;
    ExecuteBiteAttack(Player);
}

void UNPC_TRexBehaviorComponent::TickReturn(float DeltaTime)
{
    if (!OwnerAIController) return;

    float DistToOrigin = FVector::Dist(OwnerPawn->GetActorLocation(), PatrolOrigin);
    if (DistToOrigin < 300.0f)
    {
        TransitionToState(ENPC_TRexState::Idle);
        return;
    }

    OwnerAIController->MoveToLocation(PatrolOrigin, 200.0f);
    if (OwnerMovement)
    {
        OwnerMovement->MaxWalkSpeed = PatrolSpeed;
    }
}

void UNPC_TRexBehaviorComponent::TransitionToState(ENPC_TRexState NewState)
{
    if (CurrentState == NewState) return;

    ENPC_TRexState OldState = CurrentState;
    CurrentState = NewState;

    // Reset per-state flags
    if (NewState == ENPC_TRexState::Chase)
    {
        bJustEnteredChase = true;
    }

    // Drive animation via DinosaurAnimInstance coupling (Agent #10 API)
    UpdateAnimationState();

    UE_LOG(LogTemp, Log, TEXT("TRexBehaviorComponent: %s -> %s"),
        *UEnum::GetValueAsString(OldState),
        *UEnum::GetValueAsString(NewState));
}

void UNPC_TRexBehaviorComponent::UpdateAnimationState()
{
    // These flags map to UDinosaurAnimInstance::SetDinoAIState(bAttacking, bEating, bRoaring, bAlert, bSleeping)
    bool bAttacking = (CurrentState == ENPC_TRexState::Attack);
    bool bEating    = false; // Extended in future cycle
    bool bRoaring   = bJustEnteredChase;
    bool bAlert     = (CurrentState == ENPC_TRexState::Alert || CurrentState == ENPC_TRexState::Chase);
    bool bSleeping  = (CurrentState == ENPC_TRexState::Idle && IdleTimer > 10.0f);

    // Broadcast to Blueprint/AnimInstance via delegate (set by owning Blueprint)
    OnAnimStateChanged.Broadcast(bAttacking, bEating, bRoaring, bAlert, bSleeping);
}

void UNPC_TRexBehaviorComponent::PickNewPatrolTarget()
{
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys)
    {
        // Fallback: random point in radius without NavMesh
        FVector RandomOffset = FVector(
            FMath::RandRange(-PatrolRadius, PatrolRadius),
            FMath::RandRange(-PatrolRadius, PatrolRadius),
            0.0f
        );
        CurrentPatrolTarget = PatrolOrigin + RandomOffset;
        bHasPatrolTarget = true;
        return;
    }

    FNavLocation NavLocation;
    bool bFound = NavSys->GetRandomReachablePointInRadius(PatrolOrigin, PatrolRadius, NavLocation);
    if (bFound)
    {
        CurrentPatrolTarget = NavLocation.Location;
        bHasPatrolTarget = true;
    }
}

void UNPC_TRexBehaviorComponent::ExecuteBiteAttack(ACharacter* Target)
{
    if (!Target) return;

    // Apply damage via UE5 damage system
    float BiteDamage = 85.0f; // T-Rex bite — near-lethal
    UGameplayStatics::ApplyDamage(Target, BiteDamage, OwnerAIController, OwnerPawn, nullptr);

    // Trigger attack montage via DinosaurAnimInstance (Agent #10 API)
    // AttackTypeIndex 0 = bite
    OnAttackMontageRequested.Broadcast(0);

    UE_LOG(LogTemp, Log, TEXT("TRexBehaviorComponent: Bite attack executed — %.0f damage to %s"),
        BiteDamage, *Target->GetName());
}

void UNPC_TRexBehaviorComponent::BroadcastRoar()
{
    // Roar alert — notify nearby raptors and other dinosaurs
    // This feeds into the crowd simulation (Agent #13)
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == OwnerPawn) continue;
        float Dist = FVector::Dist(OwnerPawn->GetActorLocation(), Actor->GetActorLocation());
        if (Dist < 4000.0f)
        {
            // Broadcast roar event — other dinosaurs can respond
            OnRoarBroadcast.Broadcast(OwnerPawn->GetActorLocation(), 4000.0f);
            break;
        }
    }

    // Trigger roar montage (AttackTypeIndex 2 = tail/roar in DinosaurAnimInstance)
    OnAttackMontageRequested.Broadcast(2);

    UE_LOG(LogTemp, Log, TEXT("TRexBehaviorComponent: Roar broadcast at %s"), *OwnerPawn->GetActorLocation().ToString());
}

// ============================================================
// External API — called by other agents
// ============================================================

void UNPC_TRexBehaviorComponent::NotifyPlayerDetectedByOtherDino(FVector PlayerLocation)
{
    // Agent #12 (Combat AI) can call this when another dino spots the player
    LastKnownPlayerLocation = PlayerLocation;
    LastThreatTime = GetWorld()->GetTimeSeconds();
    bPlayerInMemory = true;

    if (CurrentState == ENPC_TRexState::Idle || CurrentState == ENPC_TRexState::Patrol)
    {
        TransitionToState(ENPC_TRexState::Alert);
    }
}

void UNPC_TRexBehaviorComponent::ForceReturnToTerritory()
{
    // Called by Agent #12 when combat zone is cleared
    bPlayerInMemory = false;
    TransitionToState(ENPC_TRexState::Return);
}
