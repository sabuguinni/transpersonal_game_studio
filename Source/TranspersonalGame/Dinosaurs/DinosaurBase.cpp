#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "AIController.h"

// ============================================================
// Constructor
// ============================================================
ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20Hz tick for AI — performance friendly

    // Default movement setup
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->MaxWalkSpeed = Stats.WalkSpeed;
        Move->bOrientRotationToMovement = true;
        Move->RotationRate = FRotator(0.f, 360.f, 0.f);
        Move->bUseControllerDesiredRotation = false;
        Move->GravityScale = 1.f;
        Move->JumpZVelocity = 0.f; // Most dinos can't jump
        Move->NavAgentProps.bCanJump = false;
    }

    // Don't use controller rotation
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// ============================================================
// BeginPlay
// ============================================================
void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Record spawn location as patrol origin
    PatrolOrigin = GetActorLocation();
    CurrentPatrolTarget = PatrolOrigin;

    // Apply stats to movement
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->MaxWalkSpeed = Stats.WalkSpeed;
    }

    // Start in idle
    SetBehaviourState(EDB_BehaviourState::Idle);
}

// ============================================================
// Tick
// ============================================================
void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsAlive()) return;

    // Tick attack cooldown
    if (AttackCooldown > 0.f)
    {
        AttackCooldown -= DeltaTime;
    }

    // Drain hunger slowly
    Stats.Hunger = FMath::Max(0.f, Stats.Hunger - DeltaTime * 0.5f);

    // State machine
    switch (CurrentBehaviour)
    {
    case EDB_BehaviourState::Idle:
        TickIdle(DeltaTime);
        break;
    case EDB_BehaviourState::Patrolling:
        TickPatrol(DeltaTime);
        break;
    case EDB_BehaviourState::Hunting:
    case EDB_BehaviourState::Attacking:
        TickHunting(DeltaTime);
        break;
    default:
        break;
    }
}

// ============================================================
// TakeDamage
// ============================================================
float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                 AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    Stats.CurrentHealth = FMath::Max(0.f, Stats.CurrentHealth - ActualDamage);

    // React to damage — switch to hunting the attacker
    if (DamageCauser && IsAlive())
    {
        CurrentTarget = DamageCauser;
        SetBehaviourState(EDB_BehaviourState::Hunting);
    }

    if (!IsAlive())
    {
        Die();
    }

    return ActualDamage;
}

// ============================================================
// SetBehaviourState
// ============================================================
void ADinosaurBase::SetBehaviourState(EDB_BehaviourState NewState)
{
    if (CurrentBehaviour == NewState) return;

    CurrentBehaviour = NewState;

    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        switch (NewState)
        {
        case EDB_BehaviourState::Hunting:
        case EDB_BehaviourState::Fleeing:
            Move->MaxWalkSpeed = Stats.RunSpeed;
            break;
        default:
            Move->MaxWalkSpeed = Stats.WalkSpeed;
            break;
        }
    }
}

// ============================================================
// IsAlive / GetHealthPercent
// ============================================================
bool ADinosaurBase::IsAlive() const
{
    return Stats.CurrentHealth > 0.f;
}

float ADinosaurBase::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.f) return 0.f;
    return Stats.CurrentHealth / Stats.MaxHealth;
}

// ============================================================
// FindNearestThreat — sphere overlap for player detection
// ============================================================
AActor* ADinosaurBase::FindNearestThreat() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    // Get the player pawn as primary threat
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return nullptr;

    float DistSq = FVector::DistSquared(GetActorLocation(), PlayerPawn->GetActorLocation());
    if (DistSq <= FMath::Square(Stats.DetectionRadius))
    {
        return PlayerPawn;
    }

    return nullptr;
}

// ============================================================
// TickIdle — scan for threats, transition to patrol after wait
// ============================================================
void ADinosaurBase::TickIdle(float DeltaTime)
{
    PatrolWaitTimer += DeltaTime;

    // Scan for threats every tick
    if (Stats.bIsCarnivore)
    {
        AActor* Threat = FindNearestThreat();
        if (Threat)
        {
            CurrentTarget = Threat;
            OnTargetDetected(Threat);
            SetBehaviourState(EDB_BehaviourState::Hunting);
            return;
        }
    }

    // After wait time, start patrolling
    if (PatrolWaitTimer >= PatrolWaitTime)
    {
        PatrolWaitTimer = 0.f;
        MoveToPatrolPoint();
        SetBehaviourState(EDB_BehaviourState::Patrolling);
    }
}

// ============================================================
// TickPatrol — move to patrol point, idle on arrival
// ============================================================
void ADinosaurBase::TickPatrol(float DeltaTime)
{
    // Carnivores scan for prey while patrolling
    if (Stats.bIsCarnivore)
    {
        AActor* Threat = FindNearestThreat();
        if (Threat)
        {
            CurrentTarget = Threat;
            OnTargetDetected(Threat);
            SetBehaviourState(EDB_BehaviourState::Hunting);
            return;
        }
    }

    // Check if we've reached the patrol target
    float DistToTarget = FVector::Dist2D(GetActorLocation(), CurrentPatrolTarget);
    if (DistToTarget < 150.f)
    {
        // Reached patrol point — go idle and wait
        SetBehaviourState(EDB_BehaviourState::Idle);
        PatrolWaitTimer = 0.f;
    }
}

// ============================================================
// TickHunting — chase and attack target
// ============================================================
void ADinosaurBase::TickHunting(float DeltaTime)
{
    if (!CurrentTarget.IsValid())
    {
        SetBehaviourState(EDB_BehaviourState::Idle);
        return;
    }

    AActor* Target = CurrentTarget.Get();
    float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());

    // Lost target — return to patrol
    if (DistToTarget > Stats.DetectionRadius * 1.5f)
    {
        CurrentTarget = nullptr;
        SetBehaviourState(EDB_BehaviourState::Patrolling);
        MoveToPatrolPoint();
        return;
    }

    // In attack range — attack
    if (DistToTarget <= Stats.AttackRadius)
    {
        PerformAttack(Target);
    }
    else
    {
        // Move toward target via AI controller
        if (AAIController* AIC = Cast<AAIController>(GetController()))
        {
            AIC->MoveToActor(Target, Stats.AttackRadius * 0.8f);
        }
    }
}

// ============================================================
// MoveToPatrolPoint — pick a random point within patrol radius
// ============================================================
void ADinosaurBase::MoveToPatrolPoint()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Pick random point within patrol radius using NavMesh
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (NavSys)
    {
        FNavLocation NavLoc;
        if (NavSys->GetRandomReachablePointInRadius(PatrolOrigin, PatrolRadius, NavLoc))
        {
            CurrentPatrolTarget = NavLoc.Location;

            if (AAIController* AIC = Cast<AAIController>(GetController()))
            {
                AIC->MoveToLocation(CurrentPatrolTarget, 100.f);
            }
            return;
        }
    }

    // Fallback: random point without NavMesh
    FVector RandomOffset = FMath::VRand() * PatrolRadius;
    RandomOffset.Z = 0.f;
    CurrentPatrolTarget = PatrolOrigin + RandomOffset;

    if (AAIController* AIC = Cast<AAIController>(GetController()))
    {
        AIC->MoveToLocation(CurrentPatrolTarget, 100.f);
    }
}

// ============================================================
// PerformAttack
// ============================================================
void ADinosaurBase::PerformAttack(AActor* Target)
{
    if (!Target || AttackCooldown > 0.f) return;

    AttackCooldown = AttackCooldownDuration;
    SetBehaviourState(EDB_BehaviourState::Attacking);

    // Apply damage
    UGameplayStatics::ApplyDamage(
        Target,
        Stats.AttackDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    OnAttackLanded(Target, Stats.AttackDamage);

    // Return to hunting after attack
    SetBehaviourState(EDB_BehaviourState::Hunting);
}

// ============================================================
// Die
// ============================================================
void ADinosaurBase::Die()
{
    SetBehaviourState(EDB_BehaviourState::Idle);

    // Stop movement
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->StopMovementImmediately();
        Move->DisableMovement();
    }

    // Disable collision
    SetActorEnableCollision(false);

    // Notify Blueprint
    OnDinosaurDeath();

    // Destroy after 10 seconds (corpse linger time)
    SetLifeSpan(10.f);
}
