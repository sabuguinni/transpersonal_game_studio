// DinosaurBase.cpp
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260701_004
// Base class for all dinosaur types in the prehistoric survival game.
// Implements movement, health, aggression, territory, and AI state machine.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule defaults — overridden per species
    GetCapsuleComponent()->InitCapsuleSize(55.0f, 96.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // Mesh setup
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -96.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    // Movement defaults
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
        MoveComp->MaxWalkSpeed = 600.0f;
        MoveComp->JumpZVelocity = 600.0f;
        MoveComp->AirControl = 0.2f;
        MoveComp->NavAgentProps.bCanCrouch = false;
        MoveComp->NavAgentProps.bCanJump = true;
    }

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // AI Perception
    AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    AIPerceptionComp->ConfigureSense(*SightConfig);

    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    AIPerceptionComp->ConfigureSense(*HearingConfig);

    AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

    // Default stats — overridden per species in BeginPlay or Blueprint
    MaxHealth = 500.0f;
    CurrentHealth = 500.0f;
    MaxStamina = 300.0f;
    CurrentStamina = 300.0f;
    AttackDamage = 75.0f;
    AttackRange = 200.0f;
    AttackCooldown = 1.5f;
    TerritoryRadius = 3000.0f;
    AggressionLevel = 0.5f;
    DetectionRange = 2000.0f;
    ChaseRange = 4000.0f;
    WalkSpeed = 300.0f;
    RunSpeed = 700.0f;
    bIsHerbivore = false;
    bIsPack = false;
    PackSize = 1;

    CurrentState = EDinoAIState::Idle;
    bIsAlive = true;
    bIsAttacking = false;
    LastAttackTime = 0.0f;

    HomeLocation = FVector::ZeroVector;
    TargetActor = nullptr;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    HomeLocation = GetActorLocation();

    // Bind perception delegate
    if (AIPerceptionComp)
    {
        AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurBase::OnPerceptionUpdated);
    }

    // Start AI tick loop
    GetWorld()->GetTimerManager().SetTimer(
        AIUpdateTimer,
        this,
        &ADinosaurBase::UpdateAIState,
        0.25f,
        true
    );

    // Stamina regen loop
    GetWorld()->GetTimerManager().SetTimer(
        StaminaRegenTimer,
        this,
        &ADinosaurBase::RegenerateStamina,
        1.0f,
        true
    );
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Face target smoothly when chasing or attacking
    if (TargetActor && (CurrentState == EDinoAIState::Chasing || CurrentState == EDinoAIState::Attacking))
    {
        FVector ToTarget = TargetActor->GetActorLocation() - GetActorLocation();
        ToTarget.Z = 0.0f;
        if (!ToTarget.IsNearlyZero())
        {
            FRotator TargetRot = ToTarget.Rotation();
            FRotator NewRot = FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 5.0f);
            SetActorRotation(NewRot);
        }
    }
}

void ADinosaurBase::UpdateAIState()
{
    if (!bIsAlive) return;

    switch (CurrentState)
    {
        case EDinoAIState::Idle:
            HandleIdleState();
            break;
        case EDinoAIState::Patrolling:
            HandlePatrolState();
            break;
        case EDinoAIState::Chasing:
            HandleChaseState();
            break;
        case EDinoAIState::Attacking:
            HandleAttackState();
            break;
        case EDinoAIState::Fleeing:
            HandleFleeState();
            break;
        case EDinoAIState::Feeding:
            HandleFeedState();
            break;
        case EDinoAIState::Resting:
            HandleRestState();
            break;
    }
}

void ADinosaurBase::HandleIdleState()
{
    // Randomly transition to patrol
    if (FMath::RandRange(0.0f, 1.0f) < 0.1f)
    {
        SetAIState(EDinoAIState::Patrolling);
    }
}

void ADinosaurBase::HandlePatrolState()
{
    // Return home if too far
    float DistFromHome = FVector::Dist(GetActorLocation(), HomeLocation);
    if (DistFromHome > TerritoryRadius)
    {
        MoveToLocation(HomeLocation);
        SetAIState(EDinoAIState::Idle);
    }
}

void ADinosaurBase::HandleChaseState()
{
    if (!TargetActor || !bIsAlive) 
    {
        SetAIState(EDinoAIState::Patrolling);
        return;
    }

    float DistToTarget = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());

    // Lost target
    if (DistToTarget > ChaseRange)
    {
        TargetActor = nullptr;
        SetAIState(EDinoAIState::Patrolling);
        return;
    }

    // Close enough to attack
    if (DistToTarget <= AttackRange)
    {
        SetAIState(EDinoAIState::Attacking);
        return;
    }

    // Keep chasing
    GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
    MoveToActor(TargetActor);
}

void ADinosaurBase::HandleAttackState()
{
    if (!TargetActor || !bIsAlive)
    {
        SetAIState(EDinoAIState::Patrolling);
        return;
    }

    float DistToTarget = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());

    // Target moved away
    if (DistToTarget > AttackRange * 1.5f)
    {
        SetAIState(EDinoAIState::Chasing);
        return;
    }

    // Attack if cooldown elapsed
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime >= AttackCooldown)
    {
        PerformAttack();
    }
}

void ADinosaurBase::HandleFleeState()
{
    if (!TargetActor) 
    {
        SetAIState(EDinoAIState::Idle);
        return;
    }

    // Flee away from threat
    FVector FleeDir = GetActorLocation() - TargetActor->GetActorLocation();
    FleeDir.Normalize();
    FVector FleeTarget = GetActorLocation() + FleeDir * 3000.0f;
    GetCharacterMovement()->MaxWalkSpeed = RunSpeed * 1.2f;
    MoveToLocation(FleeTarget);

    float DistToThreat = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
    if (DistToThreat > ChaseRange * 1.5f)
    {
        TargetActor = nullptr;
        SetAIState(EDinoAIState::Idle);
    }
}

void ADinosaurBase::HandleFeedState()
{
    // Restore health slowly while feeding
    CurrentHealth = FMath::Min(CurrentHealth + 10.0f, MaxHealth);
    
    // Random chance to finish feeding
    if (FMath::RandRange(0.0f, 1.0f) < 0.05f)
    {
        SetAIState(EDinoAIState::Idle);
    }
}

void ADinosaurBase::HandleRestState()
{
    // Restore stamina faster while resting
    CurrentStamina = FMath::Min(CurrentStamina + 20.0f, MaxStamina);
    
    if (CurrentStamina >= MaxStamina * 0.8f)
    {
        SetAIState(EDinoAIState::Idle);
    }
}

void ADinosaurBase::PerformAttack()
{
    if (!TargetActor || !bIsAlive) return;

    LastAttackTime = GetWorld()->GetTimeSeconds();
    bIsAttacking = true;

    // Apply damage
    UGameplayStatics::ApplyDamage(
        TargetActor,
        AttackDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    // Blueprint event for animation
    OnAttack(TargetActor);

    // Reset attack flag after animation window
    FTimerHandle AttackResetTimer;
    GetWorld()->GetTimerManager().SetTimer(
        AttackResetTimer,
        [this]() { bIsAttacking = false; },
        0.5f,
        false
    );
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (!bIsAlive) return 0.0f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHealth -= ActualDamage;

    OnDamageReceived(ActualDamage, DamageCauser);

    // React to damage — set attacker as target
    if (DamageCauser && CurrentState != EDinoAIState::Fleeing)
    {
        TargetActor = DamageCauser;
        
        // Herbivores flee, carnivores retaliate
        if (bIsHerbivore)
        {
            SetAIState(EDinoAIState::Fleeing);
        }
        else
        {
            SetAIState(EDinoAIState::Chasing);
        }
    }

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }

    return ActualDamage;
}

void ADinosaurBase::Die()
{
    if (!bIsAlive) return;

    bIsAlive = false;
    CurrentState = EDinoAIState::Dead;

    // Stop movement
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Stop AI timers
    GetWorld()->GetTimerManager().ClearTimer(AIUpdateTimer);
    GetWorld()->GetTimerManager().ClearTimer(StaminaRegenTimer);

    // Blueprint event
    OnDeath();

    // Destroy after delay (allow death animation)
    SetLifeSpan(10.0f);
}

void ADinosaurBase::SetAIState(EDinoAIState NewState)
{
    if (CurrentState == NewState) return;

    EDinoAIState OldState = CurrentState;
    CurrentState = NewState;

    // Speed adjustment
    switch (NewState)
    {
        case EDinoAIState::Chasing:
        case EDinoAIState::Fleeing:
            GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
            break;
        case EDinoAIState::Patrolling:
        case EDinoAIState::Feeding:
            GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
            break;
        case EDinoAIState::Idle:
        case EDinoAIState::Resting:
            GetCharacterMovement()->MaxWalkSpeed = 0.0f;
            break;
        default:
            break;
    }

    OnStateChanged(OldState, NewState);
}

void ADinosaurBase::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!bIsAlive || !Actor) return;

    // Ignore other dinosaurs of same type
    if (Actor->IsA(GetClass())) return;

    bool bIsPlayer = Actor->ActorHasTag(TEXT("Player"));

    if (Stimulus.WasSuccessfullySensed())
    {
        float DistToActor = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());

        if (bIsHerbivore)
        {
            // Herbivores flee from players and carnivores
            if (bIsPlayer || Actor->IsA(ADinosaurBase::StaticClass()))
            {
                TargetActor = Actor;
                SetAIState(EDinoAIState::Fleeing);
            }
        }
        else
        {
            // Carnivores attack based on aggression
            if (bIsPlayer && FMath::RandRange(0.0f, 1.0f) < AggressionLevel)
            {
                TargetActor = Actor;
                SetAIState(EDinoAIState::Chasing);
            }
        }
    }
    else
    {
        // Lost perception — return to patrol if this was our target
        if (Actor == TargetActor && CurrentState == EDinoAIState::Chasing)
        {
            TargetActor = nullptr;
            SetAIState(EDinoAIState::Patrolling);
        }
    }
}

void ADinosaurBase::RegenerateStamina()
{
    if (!bIsAlive) return;

    if (CurrentState == EDinoAIState::Idle || CurrentState == EDinoAIState::Resting)
    {
        CurrentStamina = FMath::Min(CurrentStamina + 15.0f, MaxStamina);
    }
    else if (CurrentState == EDinoAIState::Chasing || CurrentState == EDinoAIState::Fleeing)
    {
        CurrentStamina = FMath::Max(CurrentStamina - 10.0f, 0.0f);
        if (CurrentStamina <= 0.0f)
        {
            SetAIState(EDinoAIState::Resting);
        }
    }
}

void ADinosaurBase::MoveToLocation(const FVector& Location)
{
    AAIController* AICtrl = Cast<AAIController>(GetController());
    if (AICtrl)
    {
        AICtrl->MoveToLocation(Location, 50.0f);
    }
}

void ADinosaurBase::MoveToActor(AActor* Actor)
{
    AAIController* AICtrl = Cast<AAIController>(GetController());
    if (AICtrl && Actor)
    {
        AICtrl->MoveToActor(Actor, AttackRange * 0.8f);
    }
}

// Blueprint-implementable events — default empty implementations
void ADinosaurBase::OnAttack_Implementation(AActor* Target) {}
void ADinosaurBase::OnDamageReceived_Implementation(float Damage, AActor* DamageCauser) {}
void ADinosaurBase::OnDeath_Implementation() {}
void ADinosaurBase::OnStateChanged_Implementation(EDinoAIState OldState, EDinoAIState NewState) {}
