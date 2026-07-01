// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — Transpersonal Game Studio
// Full implementation of dinosaur AI behavior state machine

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNPC_BehaviorComponent::UNPC_BehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz AI tick for performance

    // Default behavior config
    BehaviorConfig.PatrolRadius = 2000.0f;
    BehaviorConfig.DetectionRange = 1500.0f;
    BehaviorConfig.AttackRange = 300.0f;
    BehaviorConfig.PatrolSpeed = 200.0f;
    BehaviorConfig.ChaseSpeed = 600.0f;
    BehaviorConfig.AttackDamage = 40.0f;
    BehaviorConfig.bIsPackHunter = false;
    BehaviorConfig.MemoryDuration = 15.0f;

    CurrentState = ENPC_BehaviorState::Idle;
    Species = ENPC_DinosaurSpecies::TRex;
    bIsAlerted = false;
    PatrolHomeLocation = FVector::ZeroVector;
    CurrentPatrolTarget = FVector::ZeroVector;
    AttackCooldown = 0.0f;
    StateTimeElapsed = 0.0f;
}

void UNPC_BehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Store home location for patrol radius
    if (AActor* Owner = GetOwner())
    {
        PatrolHomeLocation = Owner->GetActorLocation();
    }

    // Apply species-specific config
    ApplySpeciesConfig();

    // Start initial patrol
    TransitionToState(ENPC_BehaviorState::Patrol);
    PickNewPatrolTarget();
}

void UNPC_BehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimeElapsed += DeltaTime;
    AttackCooldown = FMath::Max(0.0f, AttackCooldown - DeltaTime);

    // Decay stimulus memory
    if (StimulusMemory.bIsValid)
    {
        StimulusMemory.MemoryAge += DeltaTime;
        if (StimulusMemory.MemoryAge >= BehaviorConfig.MemoryDuration)
        {
            StimulusMemory.bIsValid = false;
            bIsAlerted = false;
        }
    }

    // Run state machine
    UpdateStateMachine(DeltaTime);
}

void UNPC_BehaviorComponent::ApplySpeciesConfig()
{
    switch (Species)
    {
    case ENPC_DinosaurSpecies::TRex:
        BehaviorConfig.PatrolRadius = 5000.0f;
        BehaviorConfig.DetectionRange = 3000.0f;
        BehaviorConfig.AttackRange = 350.0f;
        BehaviorConfig.PatrolSpeed = 250.0f;
        BehaviorConfig.ChaseSpeed = 700.0f;
        BehaviorConfig.AttackDamage = 80.0f;
        BehaviorConfig.bIsPackHunter = false;
        BehaviorConfig.MemoryDuration = 20.0f;
        break;

    case ENPC_DinosaurSpecies::Raptor:
        BehaviorConfig.PatrolRadius = 3000.0f;
        BehaviorConfig.DetectionRange = 2000.0f;
        BehaviorConfig.AttackRange = 200.0f;
        BehaviorConfig.PatrolSpeed = 300.0f;
        BehaviorConfig.ChaseSpeed = 900.0f;
        BehaviorConfig.AttackDamage = 30.0f;
        BehaviorConfig.bIsPackHunter = true;
        BehaviorConfig.MemoryDuration = 25.0f;
        break;

    case ENPC_DinosaurSpecies::Brachiosaurus:
        BehaviorConfig.PatrolRadius = 2000.0f;
        BehaviorConfig.DetectionRange = 800.0f;
        BehaviorConfig.AttackRange = 400.0f;
        BehaviorConfig.PatrolSpeed = 150.0f;
        BehaviorConfig.ChaseSpeed = 300.0f;
        BehaviorConfig.AttackDamage = 50.0f;
        BehaviorConfig.bIsPackHunter = false;
        BehaviorConfig.MemoryDuration = 10.0f;
        break;

    case ENPC_DinosaurSpecies::Triceratops:
        BehaviorConfig.PatrolRadius = 2500.0f;
        BehaviorConfig.DetectionRange = 1200.0f;
        BehaviorConfig.AttackRange = 300.0f;
        BehaviorConfig.PatrolSpeed = 200.0f;
        BehaviorConfig.ChaseSpeed = 550.0f;
        BehaviorConfig.AttackDamage = 60.0f;
        BehaviorConfig.bIsPackHunter = false;
        BehaviorConfig.MemoryDuration = 12.0f;
        break;

    case ENPC_DinosaurSpecies::Pterodactyl:
        BehaviorConfig.PatrolRadius = 8000.0f;
        BehaviorConfig.DetectionRange = 4000.0f;
        BehaviorConfig.AttackRange = 150.0f;
        BehaviorConfig.PatrolSpeed = 400.0f;
        BehaviorConfig.ChaseSpeed = 1000.0f;
        BehaviorConfig.AttackDamage = 20.0f;
        BehaviorConfig.bIsPackHunter = false;
        BehaviorConfig.MemoryDuration = 8.0f;
        break;
    }
}

void UNPC_BehaviorComponent::UpdateStateMachine(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Sense player every tick
    APawn* Player = SensePlayer();

    switch (CurrentState)
    {
    case ENPC_BehaviorState::Idle:
        UpdateIdle(DeltaTime, Player);
        break;

    case ENPC_BehaviorState::Patrol:
        UpdatePatrol(DeltaTime, Player);
        break;

    case ENPC_BehaviorState::Investigate:
        UpdateInvestigate(DeltaTime, Player);
        break;

    case ENPC_BehaviorState::Chase:
        UpdateChase(DeltaTime, Player);
        break;

    case ENPC_BehaviorState::Attack:
        UpdateAttack(DeltaTime, Player);
        break;

    case ENPC_BehaviorState::Flee:
        UpdateFlee(DeltaTime, Player);
        break;

    case ENPC_BehaviorState::Graze:
        UpdateGraze(DeltaTime, Player);
        break;

    case ENPC_BehaviorState::Sleep:
        UpdateSleep(DeltaTime, Player);
        break;
    }
}

APawn* UNPC_BehaviorComponent::SensePlayer()
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC) return nullptr;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return nullptr;

    AActor* Owner = GetOwner();
    if (!Owner) return nullptr;

    float DistToPlayer = FVector::Dist(Owner->GetActorLocation(), PlayerPawn->GetActorLocation());

    if (DistToPlayer <= BehaviorConfig.DetectionRange)
    {
        // Update stimulus memory
        StimulusMemory.LastKnownPlayerLocation = PlayerPawn->GetActorLocation();
        StimulusMemory.LastSeenTime = World->GetTimeSeconds();
        StimulusMemory.MemoryAge = 0.0f;
        StimulusMemory.ThreatLevel = FMath::Clamp(1.0f - (DistToPlayer / BehaviorConfig.DetectionRange), 0.0f, 1.0f);
        StimulusMemory.bIsValid = true;
        bIsAlerted = true;
        return PlayerPawn;
    }

    return nullptr;
}

void UNPC_BehaviorComponent::UpdateIdle(float DeltaTime, APawn* Player)
{
    if (Player)
    {
        TransitionToState(ENPC_BehaviorState::Investigate);
        return;
    }

    // After idling for 3 seconds, start patrol
    if (StateTimeElapsed >= 3.0f)
    {
        TransitionToState(ENPC_BehaviorState::Patrol);
        PickNewPatrolTarget();
    }
}

void UNPC_BehaviorComponent::UpdatePatrol(float DeltaTime, APawn* Player)
{
    if (Player)
    {
        TransitionToState(ENPC_BehaviorState::Chase);
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Move toward patrol target
    FVector ToTarget = CurrentPatrolTarget - Owner->GetActorLocation();
    float DistToTarget = ToTarget.Size();

    if (DistToTarget < 200.0f)
    {
        // Reached patrol point — idle briefly then pick new target
        TransitionToState(ENPC_BehaviorState::Idle);
    }
    else
    {
        // Move toward target
        FVector Direction = ToTarget.GetSafeNormal();
        Owner->SetActorLocation(
            Owner->GetActorLocation() + Direction * BehaviorConfig.PatrolSpeed * DeltaTime,
            true
        );
        // Face movement direction
        FRotator NewRot = Direction.Rotation();
        Owner->SetActorRotation(NewRot);
    }
}

void UNPC_BehaviorComponent::UpdateInvestigate(float DeltaTime, APawn* Player)
{
    if (Player)
    {
        AActor* Owner = GetOwner();
        if (!Owner) return;
        float Dist = FVector::Dist(Owner->GetActorLocation(), Player->GetActorLocation());
        if (Dist <= BehaviorConfig.AttackRange)
        {
            TransitionToState(ENPC_BehaviorState::Attack);
        }
        else
        {
            TransitionToState(ENPC_BehaviorState::Chase);
        }
        return;
    }

    // No player visible — investigate last known location
    if (StimulusMemory.bIsValid)
    {
        AActor* Owner = GetOwner();
        if (!Owner) return;

        FVector ToMemory = StimulusMemory.LastKnownPlayerLocation - Owner->GetActorLocation();
        float Dist = ToMemory.Size();

        if (Dist < 300.0f)
        {
            // Reached investigation point — return to patrol
            TransitionToState(ENPC_BehaviorState::Patrol);
            PickNewPatrolTarget();
        }
        else
        {
            FVector Dir = ToMemory.GetSafeNormal();
            Owner->SetActorLocation(
                Owner->GetActorLocation() + Dir * (BehaviorConfig.PatrolSpeed * 1.5f) * DeltaTime,
                true
            );
            Owner->SetActorRotation(Dir.Rotation());
        }
    }
    else
    {
        TransitionToState(ENPC_BehaviorState::Patrol);
        PickNewPatrolTarget();
    }
}

void UNPC_BehaviorComponent::UpdateChase(float DeltaTime, APawn* Player)
{
    if (!Player)
    {
        if (StimulusMemory.bIsValid)
        {
            TransitionToState(ENPC_BehaviorState::Investigate);
        }
        else
        {
            TransitionToState(ENPC_BehaviorState::Patrol);
            PickNewPatrolTarget();
        }
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector ToPlayer = Player->GetActorLocation() - Owner->GetActorLocation();
    float Dist = ToPlayer.Size();

    if (Dist <= BehaviorConfig.AttackRange)
    {
        TransitionToState(ENPC_BehaviorState::Attack);
        return;
    }

    // Chase at full speed
    FVector Dir = ToPlayer.GetSafeNormal();
    Owner->SetActorLocation(
        Owner->GetActorLocation() + Dir * BehaviorConfig.ChaseSpeed * DeltaTime,
        true
    );
    Owner->SetActorRotation(Dir.Rotation());
}

void UNPC_BehaviorComponent::UpdateAttack(float DeltaTime, APawn* Player)
{
    if (!Player)
    {
        TransitionToState(ENPC_BehaviorState::Investigate);
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner) return;

    float Dist = FVector::Dist(Owner->GetActorLocation(), Player->GetActorLocation());

    if (Dist > BehaviorConfig.AttackRange * 1.5f)
    {
        // Player escaped attack range — chase again
        TransitionToState(ENPC_BehaviorState::Chase);
        return;
    }

    // Execute attack if cooldown expired
    if (AttackCooldown <= 0.0f)
    {
        ExecuteAttack(Player);
        AttackCooldown = 1.5f; // 1.5s between attacks
    }

    // Face the player
    FVector ToPlayer = (Player->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
    Owner->SetActorRotation(ToPlayer.Rotation());
}

void UNPC_BehaviorComponent::UpdateFlee(float DeltaTime, APawn* Player)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Flee away from player
    FVector FleeDir = FVector::ZeroVector;
    if (Player)
    {
        FleeDir = (Owner->GetActorLocation() - Player->GetActorLocation()).GetSafeNormal();
    }
    else
    {
        // Flee toward home
        FleeDir = (PatrolHomeLocation - Owner->GetActorLocation()).GetSafeNormal();
    }

    Owner->SetActorLocation(
        Owner->GetActorLocation() + FleeDir * BehaviorConfig.ChaseSpeed * DeltaTime,
        true
    );
    Owner->SetActorRotation(FleeDir.Rotation());

    // Stop fleeing after 8 seconds
    if (StateTimeElapsed >= 8.0f)
    {
        TransitionToState(ENPC_BehaviorState::Patrol);
        PickNewPatrolTarget();
    }
}

void UNPC_BehaviorComponent::UpdateGraze(float DeltaTime, APawn* Player)
{
    if (Player)
    {
        AActor* Owner = GetOwner();
        if (!Owner) return;
        float Dist = FVector::Dist(Owner->GetActorLocation(), Player->GetActorLocation());
        if (Dist < BehaviorConfig.DetectionRange * 0.5f)
        {
            // Player too close — flee or investigate
            if (Species == ENPC_DinosaurSpecies::Brachiosaurus || Species == ENPC_DinosaurSpecies::Triceratops)
            {
                TransitionToState(ENPC_BehaviorState::Flee);
            }
            else
            {
                TransitionToState(ENPC_BehaviorState::Chase);
            }
        }
    }

    // Graze for 10-20 seconds then patrol
    if (StateTimeElapsed >= 15.0f)
    {
        TransitionToState(ENPC_BehaviorState::Patrol);
        PickNewPatrolTarget();
    }
}

void UNPC_BehaviorComponent::UpdateSleep(float DeltaTime, APawn* Player)
{
    if (Player)
    {
        AActor* Owner = GetOwner();
        if (!Owner) return;
        float Dist = FVector::Dist(Owner->GetActorLocation(), Player->GetActorLocation());
        // Only wake if player is very close
        if (Dist < BehaviorConfig.DetectionRange * 0.3f)
        {
            TransitionToState(ENPC_BehaviorState::Investigate);
        }
    }

    // Sleep for 30 seconds
    if (StateTimeElapsed >= 30.0f)
    {
        TransitionToState(ENPC_BehaviorState::Graze);
    }
}

void UNPC_BehaviorComponent::TransitionToState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;

    ENPC_BehaviorState OldState = CurrentState;
    CurrentState = NewState;
    StateTimeElapsed = 0.0f;

    OnBehaviorStateChanged.Broadcast(OldState, NewState);

    UE_LOG(LogTemp, Log, TEXT("[NPC_Behavior] %s: %d -> %d"),
        GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
        (int32)OldState,
        (int32)NewState);
}

void UNPC_BehaviorComponent::PickNewPatrolTarget()
{
    // Pick random point within patrol radius of home
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Dist = FMath::RandRange(BehaviorConfig.PatrolRadius * 0.3f, BehaviorConfig.PatrolRadius);
    float RadAngle = FMath::DegreesToRadians(Angle);

    CurrentPatrolTarget = PatrolHomeLocation + FVector(
        FMath::Cos(RadAngle) * Dist,
        FMath::Sin(RadAngle) * Dist,
        0.0f
    );
}

void UNPC_BehaviorComponent::ExecuteAttack(APawn* Target)
{
    if (!Target) return;

    // Apply damage via UE5 damage system
    UGameplayStatics::ApplyDamage(
        Target,
        BehaviorConfig.AttackDamage,
        nullptr,
        GetOwner(),
        nullptr
    );

    UE_LOG(LogTemp, Log, TEXT("[NPC_Behavior] %s attacks player for %.1f damage"),
        GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
        BehaviorConfig.AttackDamage);
}

void UNPC_BehaviorComponent::SetSpecies(ENPC_DinosaurSpecies NewSpecies)
{
    Species = NewSpecies;
    ApplySpeciesConfig();
}

void UNPC_BehaviorComponent::ForceState(ENPC_BehaviorState NewState)
{
    TransitionToState(NewState);
}

FString UNPC_BehaviorComponent::GetCurrentStateName() const
{
    switch (CurrentState)
    {
    case ENPC_BehaviorState::Idle:        return TEXT("Idle");
    case ENPC_BehaviorState::Patrol:      return TEXT("Patrol");
    case ENPC_BehaviorState::Investigate: return TEXT("Investigate");
    case ENPC_BehaviorState::Chase:       return TEXT("Chase");
    case ENPC_BehaviorState::Attack:      return TEXT("Attack");
    case ENPC_BehaviorState::Flee:        return TEXT("Flee");
    case ENPC_BehaviorState::Graze:       return TEXT("Graze");
    case ENPC_BehaviorState::Sleep:       return TEXT("Sleep");
    default:                              return TEXT("Unknown");
    }
}
