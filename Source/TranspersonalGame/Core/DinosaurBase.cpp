// DinosaurBase.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Full implementation of dinosaur base class: patrol/chase/attack state machine
// Cycle: PROD_CYCLE_AUTO_20260624_006

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);

    // Movement defaults — overridden per species in subclasses
    GetCharacterMovement()->MaxWalkSpeed = 300.0f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 400.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    bUseControllerRotationYaw = false;

    // Species defaults
    Species = EDinoSpecies::TRex;
    CurrentState = EDinoState::Idle;
    MaxHealth = 1000.0f;
    CurrentHealth = MaxHealth;
    AttackDamage = 150.0f;
    DetectionRadius = 2000.0f;
    AttackRadius = 300.0f;
    PatrolRadius = 1500.0f;
    ChaseSpeed = 600.0f;
    PatrolSpeed = 200.0f;
    bIsAggressive = true;
    bIsPackHunter = false;
    PatrolWaitTime = 3.0f;
    AttackCooldown = 2.0f;
    TimeSinceLastAttack = 0.0f;
    PatrolWaitTimer = 0.0f;
    HomeLocation = FVector::ZeroVector;
    TargetActor = nullptr;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();
    HomeLocation = GetActorLocation();
    CurrentState = EDinoState::Patrol;
    PickNewPatrolPoint();
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastAttack += DeltaTime;

    switch (CurrentState)
    {
        case EDinoState::Idle:
            TickIdle(DeltaTime);
            break;
        case EDinoState::Patrol:
            TickPatrol(DeltaTime);
            break;
        case EDinoState::Chase:
            TickChase(DeltaTime);
            break;
        case EDinoState::Attack:
            TickAttack(DeltaTime);
            break;
        case EDinoState::Flee:
            TickFlee(DeltaTime);
            break;
        case EDinoState::Dead:
            // No tick when dead
            break;
    }

    // Always scan for threats/prey
    if (CurrentState != EDinoState::Dead && CurrentState != EDinoState::Attack)
    {
        ScanForTargets();
    }
}

// ============================================================
// STATE MACHINE TICKS
// ============================================================

void ADinosaurBase::TickIdle(float DeltaTime)
{
    PatrolWaitTimer += DeltaTime;
    if (PatrolWaitTimer >= PatrolWaitTime)
    {
        PatrolWaitTimer = 0.0f;
        SetState(EDinoState::Patrol);
        PickNewPatrolPoint();
    }
}

void ADinosaurBase::TickPatrol(float DeltaTime)
{
    float DistToPatrol = FVector::Dist(GetActorLocation(), CurrentPatrolTarget);
    if (DistToPatrol < 150.0f)
    {
        // Reached patrol point — wait briefly then pick new one
        SetState(EDinoState::Idle);
        PatrolWaitTimer = 0.0f;
    }
    else
    {
        // Move toward patrol target
        FVector Dir = (CurrentPatrolTarget - GetActorLocation()).GetSafeNormal();
        GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
        AddMovementInput(Dir, 1.0f);
    }
}

void ADinosaurBase::TickChase(float DeltaTime)
{
    if (!TargetActor || !IsValid(TargetActor))
    {
        TargetActor = nullptr;
        SetState(EDinoState::Patrol);
        PickNewPatrolPoint();
        return;
    }

    float DistToTarget = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());

    // Lost target — too far
    if (DistToTarget > DetectionRadius * 1.5f)
    {
        TargetActor = nullptr;
        SetState(EDinoState::Patrol);
        PickNewPatrolPoint();
        return;
    }

    // Close enough to attack
    if (DistToTarget <= AttackRadius)
    {
        SetState(EDinoState::Attack);
        return;
    }

    // Chase
    FVector Dir = (TargetActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
    AddMovementInput(Dir, 1.0f);
}

void ADinosaurBase::TickAttack(float DeltaTime)
{
    if (!TargetActor || !IsValid(TargetActor))
    {
        TargetActor = nullptr;
        SetState(EDinoState::Patrol);
        return;
    }

    float DistToTarget = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());

    // Target escaped
    if (DistToTarget > AttackRadius * 1.5f)
    {
        SetState(EDinoState::Chase);
        return;
    }

    // Perform attack on cooldown
    if (TimeSinceLastAttack >= AttackCooldown)
    {
        PerformAttack();
        TimeSinceLastAttack = 0.0f;
    }
}

void ADinosaurBase::TickFlee(float DeltaTime)
{
    if (!TargetActor || !IsValid(TargetActor))
    {
        SetState(EDinoState::Patrol);
        return;
    }

    // Flee away from threat
    FVector FleeDir = (GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal();
    GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed * 1.2f;
    AddMovementInput(FleeDir, 1.0f);

    float DistToThreat = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
    if (DistToThreat > DetectionRadius * 2.0f)
    {
        TargetActor = nullptr;
        SetState(EDinoState::Patrol);
        PickNewPatrolPoint();
    }
}

// ============================================================
// CORE LOGIC
// ============================================================

void ADinosaurBase::ScanForTargets()
{
    if (!bIsAggressive) return;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);

    AActor* NearestThreat = nullptr;
    float NearestDist = DetectionRadius;

    for (AActor* Actor : FoundActors)
    {
        if (Actor == this) continue;

        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist < NearestDist)
        {
            NearestDist = Dist;
            NearestThreat = Actor;
        }
    }

    if (NearestThreat && CurrentState == EDinoState::Patrol)
    {
        TargetActor = NearestThreat;
        SetState(EDinoState::Chase);
    }
}

void ADinosaurBase::PerformAttack()
{
    if (!TargetActor) return;

    // Apply damage via UE5 damage system
    UGameplayStatics::ApplyDamage(
        TargetActor,
        AttackDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    OnAttackPerformed();
}

void ADinosaurBase::PickNewPatrolPoint()
{
    // Random point within PatrolRadius of home
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Dist = FMath::RandRange(200.0f, PatrolRadius);
    FVector Offset(
        FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
        FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist,
        0.0f
    );
    CurrentPatrolTarget = HomeLocation + Offset;
}

void ADinosaurBase::SetState(EDinoState NewState)
{
    if (CurrentState == NewState) return;
    EDinoState OldState = CurrentState;
    CurrentState = NewState;
    OnStateChanged(OldState, NewState);
}

// ============================================================
// DAMAGE / DEATH
// ============================================================

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    if (CurrentHealth <= 0.0f && CurrentState != EDinoState::Dead)
    {
        Die();
    }
    else if (DamageCauser && CurrentState == EDinoState::Patrol)
    {
        // Aggro on attacker
        TargetActor = DamageCauser;
        SetState(EDinoState::Chase);
    }

    return ActualDamage;
}

void ADinosaurBase::Die()
{
    SetState(EDinoState::Dead);
    GetCharacterMovement()->DisableMovement();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    OnDinosaurDied();

    // Destroy after 10 seconds (ragdoll time)
    SetLifeSpan(10.0f);
}

// ============================================================
// BLUEPRINT EVENTS (empty base implementations)
// ============================================================

void ADinosaurBase::OnStateChanged_Implementation(EDinoState OldState, EDinoState NewState)
{
    // Override in Blueprint or subclass
}

void ADinosaurBase::OnAttackPerformed_Implementation()
{
    // Override in Blueprint or subclass
}

void ADinosaurBase::OnDinosaurDied_Implementation()
{
    // Override in Blueprint or subclass
}

// ============================================================
// UTILITY
// ============================================================

float ADinosaurBase::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

bool ADinosaurBase::IsAlive() const
{
    return CurrentState != EDinoState::Dead && CurrentHealth > 0.0f;
}

FString ADinosaurBase::GetSpeciesName() const
{
    switch (Species)
    {
        case EDinoSpecies::TRex:           return TEXT("Tyrannosaurus Rex");
        case EDinoSpecies::Velociraptor:   return TEXT("Velociraptor");
        case EDinoSpecies::Triceratops:    return TEXT("Triceratops");
        case EDinoSpecies::Brachiosaurus:  return TEXT("Brachiosaurus");
        case EDinoSpecies::Ankylosaurus:   return TEXT("Ankylosaurus");
        case EDinoSpecies::Parasaurolophus:return TEXT("Parasaurolophus");
        default:                           return TEXT("Unknown Dinosaur");
    }
}
