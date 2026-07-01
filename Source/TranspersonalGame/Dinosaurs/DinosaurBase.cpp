// DinosaurBase.cpp
// Core Systems Programmer — Agent #03
// Cycle AUTO_20260701_005
// Implements base dinosaur pawn: patrol AI, perception, attack, survival stats

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule defaults — overridden per species in subclass
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);

    // Movement defaults
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
    GetCharacterMovement()->JumpZVelocity = 400.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;

    bUseControllerRotationYaw = false;

    // AI Perception
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRange;
    SightConfig->LoseSightRadius = SightRange * 1.3f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRange;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;

    PerceptionComponent->ConfigureSense(*SightConfig);
    PerceptionComponent->ConfigureSense(*HearingConfig);
    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Defaults
    Health = MaxHealth;
    Hunger = MaxHunger;
    CurrentState = EDinoState::Idle;
    CurrentBehavior = EDinoBehavior::Patrol;
    SpeciesType = EDinoSpecies::TRex;
    bIsAlive = true;
    bIsAggressive = false;
    PatrolRadius = 1500.0f;
    PatrolSpeed = 300.0f;
    ChaseSpeed = 700.0f;
    AttackRange = 200.0f;
    AttackDamage = 40.0f;
    AttackCooldown = 2.0f;
    SightRange = 2000.0f;
    HearingRange = 1000.0f;
    bAttackOnCooldown = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    PatrolOrigin = GetActorLocation();

    // Bind perception delegate
    if (PerceptionComponent)
    {
        PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurBase::OnPerceptionUpdated);
    }

    // Start patrol timer
    GetWorldTimerManager().SetTimer(
        PatrolTimerHandle,
        this,
        &ADinosaurBase::PickNextPatrolPoint,
        FMath::RandRange(3.0f, 6.0f),
        false
    );

    // Hunger drain timer (every 10 seconds)
    GetWorldTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ADinosaurBase::DrainHunger,
        10.0f,
        true
    );
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsAlive) return;

    switch (CurrentBehavior)
    {
        case EDinoBehavior::Patrol:
            TickPatrol(DeltaTime);
            break;
        case EDinoBehavior::Chase:
            TickChase(DeltaTime);
            break;
        case EDinoBehavior::Attack:
            TickAttack(DeltaTime);
            break;
        case EDinoBehavior::Flee:
            TickFlee(DeltaTime);
            break;
        case EDinoBehavior::Idle:
        default:
            // Idle: do nothing, wait for patrol timer
            break;
    }
}

// ============================================================
// PATROL
// ============================================================

void ADinosaurBase::PickNextPatrolPoint()
{
    if (!bIsAlive || CurrentBehavior != EDinoBehavior::Patrol) return;

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys) return;

    FNavLocation NavLoc;
    bool bFound = NavSys->GetRandomReachablePointInRadius(PatrolOrigin, PatrolRadius, NavLoc);

    if (bFound)
    {
        CurrentPatrolTarget = NavLoc.Location;
        CurrentState = EDinoState::Walking;
        GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;

        AAIController* AIC = Cast<AAIController>(GetController());
        if (AIC)
        {
            AIC->MoveToLocation(CurrentPatrolTarget, 50.0f, true, true, false, true);
        }
    }

    // Schedule next patrol point
    float NextDelay = FMath::RandRange(8.0f, 20.0f);
    GetWorldTimerManager().SetTimer(PatrolTimerHandle, this, &ADinosaurBase::PickNextPatrolPoint, NextDelay, false);
}

void ADinosaurBase::TickPatrol(float DeltaTime)
{
    // Patrol is driven by AI MoveToLocation — just ensure speed is correct
    if (GetCharacterMovement()->MaxWalkSpeed != PatrolSpeed)
    {
        GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
    }
}

// ============================================================
// CHASE
// ============================================================

void ADinosaurBase::TickChase(float DeltaTime)
{
    if (!TargetActor || !TargetActor->IsValidLowLevel()) 
    {
        SetBehavior(EDinoBehavior::Patrol);
        return;
    }

    float DistToTarget = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());

    // Close enough to attack
    if (DistToTarget <= AttackRange)
    {
        SetBehavior(EDinoBehavior::Attack);
        return;
    }

    // Lost sight — too far
    if (DistToTarget > SightRange * 1.5f)
    {
        TargetActor = nullptr;
        SetBehavior(EDinoBehavior::Patrol);
        return;
    }

    // Move toward target
    GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
    AAIController* AIC = Cast<AAIController>(GetController());
    if (AIC)
    {
        AIC->MoveToActor(TargetActor, AttackRange * 0.8f, true, true, false, nullptr, true);
    }
}

// ============================================================
// ATTACK
// ============================================================

void ADinosaurBase::TickAttack(float DeltaTime)
{
    if (!TargetActor || !TargetActor->IsValidLowLevel())
    {
        SetBehavior(EDinoBehavior::Patrol);
        return;
    }

    float DistToTarget = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());

    // Target moved out of attack range — chase
    if (DistToTarget > AttackRange * 1.2f)
    {
        SetBehavior(EDinoBehavior::Chase);
        return;
    }

    // Face target
    FVector ToTarget = (TargetActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FRotator LookRot = ToTarget.Rotation();
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), LookRot, DeltaTime, 5.0f));

    // Attack if not on cooldown
    if (!bAttackOnCooldown)
    {
        PerformAttack();
    }
}

void ADinosaurBase::PerformAttack()
{
    if (!TargetActor || bAttackOnCooldown) return;

    // Apply damage
    UGameplayStatics::ApplyDamage(TargetActor, AttackDamage, GetController(), this, UDamageType::StaticClass());

    // Start cooldown
    bAttackOnCooldown = true;
    GetWorldTimerManager().SetTimer(
        AttackCooldownHandle,
        [this]() { bAttackOnCooldown = false; },
        AttackCooldown,
        false
    );

    // Satisfy hunger slightly
    Hunger = FMath::Clamp(Hunger + 10.0f, 0.0f, MaxHunger);

    OnAttackPerformed();
}

void ADinosaurBase::OnAttackPerformed_Implementation()
{
    // Blueprint override point — play attack animation, sound, VFX
}

// ============================================================
// FLEE
// ============================================================

void ADinosaurBase::TickFlee(float DeltaTime)
{
    if (!TargetActor) 
    {
        SetBehavior(EDinoBehavior::Patrol);
        return;
    }

    float DistToThreat = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());

    // Safe distance reached — stop fleeing
    if (DistToThreat > SightRange * 2.0f)
    {
        TargetActor = nullptr;
        SetBehavior(EDinoBehavior::Patrol);
        return;
    }

    // Move away from threat
    FVector AwayDir = (GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal();
    FVector FleeTarget = GetActorLocation() + AwayDir * 2000.0f;

    GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
    AAIController* AIC = Cast<AAIController>(GetController());
    if (AIC)
    {
        AIC->MoveToLocation(FleeTarget, 50.0f, true, true, false, true);
    }
}

// ============================================================
// PERCEPTION
// ============================================================

void ADinosaurBase::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!bIsAlive || !Actor) return;

    // Ignore other dinosaurs of same species
    ADinosaurBase* OtherDino = Cast<ADinosaurBase>(Actor);
    if (OtherDino && OtherDino->SpeciesType == SpeciesType) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Detected something
        if (bIsAggressive)
        {
            TargetActor = Actor;
            SetBehavior(EDinoBehavior::Chase);
        }
        else
        {
            // Herbivores flee from predators
            if (OtherDino && OtherDino->bIsAggressive)
            {
                TargetActor = Actor;
                SetBehavior(EDinoBehavior::Flee);
            }
        }
    }
    else
    {
        // Lost perception
        if (TargetActor == Actor)
        {
            TargetActor = nullptr;
            if (CurrentBehavior == EDinoBehavior::Chase || CurrentBehavior == EDinoBehavior::Attack)
            {
                SetBehavior(EDinoBehavior::Patrol);
            }
        }
    }
}

// ============================================================
// DAMAGE / DEATH
// ============================================================

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (!bIsAlive) return 0.0f;

    Health = FMath::Clamp(Health - ActualDamage, 0.0f, MaxHealth);

    if (Health <= 0.0f)
    {
        Die();
    }
    else if (DamageCauser && !bIsAggressive)
    {
        // Herbivore flees when attacked
        TargetActor = DamageCauser;
        SetBehavior(EDinoBehavior::Flee);
    }
    else if (DamageCauser && bIsAggressive)
    {
        // Predator retaliates
        TargetActor = DamageCauser;
        SetBehavior(EDinoBehavior::Chase);
    }

    return ActualDamage;
}

void ADinosaurBase::Die()
{
    bIsAlive = false;
    CurrentState = EDinoState::Dead;

    // Clear timers
    GetWorldTimerManager().ClearTimer(PatrolTimerHandle);
    GetWorldTimerManager().ClearTimer(HungerTimerHandle);
    GetWorldTimerManager().ClearTimer(AttackCooldownHandle);

    // Ragdoll
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Disable AI
    AAIController* AIC = Cast<AAIController>(GetController());
    if (AIC) AIC->StopMovement();

    OnDeath();

    // Destroy after 30 seconds (corpse decay)
    SetLifeSpan(30.0f);
}

void ADinosaurBase::OnDeath_Implementation()
{
    // Blueprint override point — play death animation, spawn loot, etc.
}

// ============================================================
// HUNGER
// ============================================================

void ADinosaurBase::DrainHunger()
{
    if (!bIsAlive) return;

    Hunger = FMath::Clamp(Hunger - 2.0f, 0.0f, MaxHunger);

    // Starving predators become more aggressive
    if (Hunger < MaxHunger * 0.2f && bIsAggressive)
    {
        SightRange *= 1.5f;
        ChaseSpeed *= 1.1f;
    }
}

// ============================================================
// HELPERS
// ============================================================

void ADinosaurBase::SetBehavior(EDinoBehavior NewBehavior)
{
    if (CurrentBehavior == NewBehavior) return;

    CurrentBehavior = NewBehavior;

    switch (NewBehavior)
    {
        case EDinoBehavior::Patrol:
            CurrentState = EDinoState::Walking;
            GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
            PickNextPatrolPoint();
            break;
        case EDinoBehavior::Chase:
            CurrentState = EDinoState::Running;
            GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
            break;
        case EDinoBehavior::Attack:
            CurrentState = EDinoState::Attacking;
            GetCharacterMovement()->StopMovementImmediately();
            break;
        case EDinoBehavior::Flee:
            CurrentState = EDinoState::Running;
            GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
            break;
        case EDinoBehavior::Idle:
            CurrentState = EDinoState::Idle;
            GetCharacterMovement()->StopMovementImmediately();
            break;
    }
}

float ADinosaurBase::GetHealthPercent() const
{
    return MaxHealth > 0.0f ? Health / MaxHealth : 0.0f;
}

float ADinosaurBase::GetHungerPercent() const
{
    return MaxHunger > 0.0f ? Hunger / MaxHunger : 0.0f;
}
