// DinosaurBase.cpp — Engine Architect #02
// Full implementation of the dinosaur base class for the prehistoric survival game.
// All dinosaur species inherit from this class.

#include "DinosaurBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AIController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20Hz tick for performance

    // Capsule sizing — overridden by subclasses
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // Movement defaults
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
        MoveComp->MaxWalkSpeed = 400.0f;
        MoveComp->JumpZVelocity = 600.0f;
        MoveComp->AirControl = 0.2f;
        MoveComp->bUseControllerDesiredRotation = false;
        MoveComp->NavAgentProps.AgentHeight = 192.0f;
        MoveComp->NavAgentProps.AgentRadius = 42.0f;
    }

    // AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
    
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    AIPerceptionComponent->ConfigureSense(*SightConfig);

    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Default stats
    MaxHealth = 500.0f;
    CurrentHealth = 500.0f;
    AttackDamage = 50.0f;
    AttackRange = 200.0f;
    AttackCooldown = 2.0f;
    DetectionRadius = 2000.0f;
    ChaseSpeed = 600.0f;
    PatrolSpeed = 200.0f;
    Species = EDinoSpecies::Raptor;
    CurrentBehaviorState = EDinoBehaviorState::Idle;
    bIsAlive = true;
    bIsAggressive = false;
    bIsHungry = false;
    HungerLevel = 0.5f;
    FearLevel = 0.0f;
    TerritoryRadius = 1500.0f;
    PackSize = 1;

    // Don't use controller rotation yaw
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Auto-possess by AI
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception updated delegate
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurBase::OnPerceptionUpdated);
    }

    // Start hunger timer
    GetWorldTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ADinosaurBase::UpdateHunger,
        10.0f,
        true
    );

    // Start patrol timer
    GetWorldTimerManager().SetTimer(
        PatrolTimerHandle,
        this,
        &ADinosaurBase::UpdatePatrol,
        3.0f,
        true
    );

    // Store home location for territory
    HomeLocation = GetActorLocation();
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsAlive) return;

    // Update behavior state machine
    UpdateBehaviorState(DeltaTime);
}

void ADinosaurBase::UpdateBehaviorState(float DeltaTime)
{
    switch (CurrentBehaviorState)
    {
    case EDinoBehaviorState::Idle:
        // Do nothing — patrol timer handles movement
        break;

    case EDinoBehaviorState::Patrolling:
        // Movement handled by AI controller
        break;

    case EDinoBehaviorState::Chasing:
        if (CurrentTarget)
        {
            float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
            if (DistToTarget <= AttackRange)
            {
                SetBehaviorState(EDinoBehaviorState::Attacking);
            }
            else if (DistToTarget > DetectionRadius * 1.5f)
            {
                // Lost target
                CurrentTarget = nullptr;
                SetBehaviorState(EDinoBehaviorState::Patrolling);
            }
        }
        else
        {
            SetBehaviorState(EDinoBehaviorState::Patrolling);
        }
        break;

    case EDinoBehaviorState::Attacking:
        if (CurrentTarget && bCanAttack)
        {
            float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
            if (DistToTarget <= AttackRange)
            {
                PerformAttack();
            }
            else
            {
                SetBehaviorState(EDinoBehaviorState::Chasing);
            }
        }
        else if (!CurrentTarget)
        {
            SetBehaviorState(EDinoBehaviorState::Patrolling);
        }
        break;

    case EDinoBehaviorState::Fleeing:
        // Move away from threat — handled by AI controller
        FearLevel = FMath::Max(0.0f, FearLevel - DeltaTime * 0.1f);
        if (FearLevel <= 0.0f)
        {
            SetBehaviorState(EDinoBehaviorState::Patrolling);
        }
        break;

    case EDinoBehaviorState::Feeding:
        HungerLevel = FMath::Max(0.0f, HungerLevel - DeltaTime * 0.05f);
        if (HungerLevel <= 0.1f)
        {
            bIsHungry = false;
            SetBehaviorState(EDinoBehaviorState::Patrolling);
        }
        break;

    case EDinoBehaviorState::Dead:
        break;
    }
}

void ADinosaurBase::SetBehaviorState(EDinoBehaviorState NewState)
{
    if (CurrentBehaviorState == NewState) return;

    EDinoBehaviorState OldState = CurrentBehaviorState;
    CurrentBehaviorState = NewState;

    // Update movement speed based on state
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        switch (NewState)
        {
        case EDinoBehaviorState::Chasing:
        case EDinoBehaviorState::Attacking:
            MoveComp->MaxWalkSpeed = ChaseSpeed;
            break;
        case EDinoBehaviorState::Fleeing:
            MoveComp->MaxWalkSpeed = ChaseSpeed * 1.2f;
            break;
        default:
            MoveComp->MaxWalkSpeed = PatrolSpeed;
            break;
        }
    }

    OnBehaviorStateChanged(OldState, NewState);
}

void ADinosaurBase::OnBehaviorStateChanged(EDinoBehaviorState OldState, EDinoBehaviorState NewState)
{
    // Override in subclasses for species-specific behavior
}

void ADinosaurBase::PerformAttack()
{
    if (!bCanAttack || !CurrentTarget || !bIsAlive) return;

    bCanAttack = false;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(
        CurrentTarget,
        AttackDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    // Reset attack cooldown
    GetWorldTimerManager().SetTimer(
        AttackCooldownHandle,
        [this]() { bCanAttack = true; },
        AttackCooldown,
        false
    );

    OnAttackPerformed(CurrentTarget);
}

void ADinosaurBase::OnAttackPerformed(AActor* Target)
{
    // Override in subclasses for attack animations/sounds
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (!bIsAlive) return 0.0f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

    // React to damage — become aggressive or flee based on species
    if (DamageCauser && bIsAlive)
    {
        FearLevel = FMath::Min(1.0f, FearLevel + 0.3f);
        
        if (bIsAggressive || CurrentHealth > MaxHealth * 0.3f)
        {
            CurrentTarget = DamageCauser;
            SetBehaviorState(EDinoBehaviorState::Chasing);
        }
        else
        {
            // Low health — flee
            SetBehaviorState(EDinoBehaviorState::Fleeing);
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
    CurrentBehaviorState = EDinoBehaviorState::Dead;

    // Clear timers
    GetWorldTimerManager().ClearTimer(HungerTimerHandle);
    GetWorldTimerManager().ClearTimer(PatrolTimerHandle);
    GetWorldTimerManager().ClearTimer(AttackCooldownHandle);

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Ragdoll on death
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (MeshComp)
    {
        MeshComp->SetSimulatePhysics(true);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    // Disable AI
    if (GetController())
    {
        GetController()->UnPossess();
    }

    OnDinosaurDied();

    // Destroy after 30 seconds (cleanup)
    SetLifeSpan(30.0f);
}

void ADinosaurBase::OnDinosaurDied()
{
    // Override in subclasses for death effects/sounds/loot
}

void ADinosaurBase::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!bIsAlive || !Actor) return;

    // Check if the perceived actor is a player
    ACharacter* PlayerChar = Cast<ACharacter>(Actor);
    if (!PlayerChar) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Detected player
        if (bIsAggressive || bIsHungry)
        {
            CurrentTarget = Actor;
            SetBehaviorState(EDinoBehaviorState::Chasing);
        }
        else
        {
            // Curious — watch but don't chase unless provoked
            FearLevel = FMath::Min(1.0f, FearLevel + 0.1f);
        }
    }
    else
    {
        // Lost sight of player
        if (CurrentTarget == Actor)
        {
            CurrentTarget = nullptr;
            if (CurrentBehaviorState == EDinoBehaviorState::Chasing ||
                CurrentBehaviorState == EDinoBehaviorState::Attacking)
            {
                SetBehaviorState(EDinoBehaviorState::Patrolling);
            }
        }
    }
}

void ADinosaurBase::UpdateHunger()
{
    if (!bIsAlive) return;

    HungerLevel = FMath::Min(1.0f, HungerLevel + 0.1f);
    
    if (HungerLevel >= 0.8f)
    {
        bIsHungry = true;
        bIsAggressive = true; // Hungry dinosaurs become aggressive
    }
}

void ADinosaurBase::UpdatePatrol()
{
    if (!bIsAlive) return;
    if (CurrentBehaviorState != EDinoBehaviorState::Idle &&
        CurrentBehaviorState != EDinoBehaviorState::Patrolling) return;

    // Generate a random patrol point within territory
    FVector RandomOffset = FVector(
        FMath::RandRange(-TerritoryRadius, TerritoryRadius),
        FMath::RandRange(-TerritoryRadius, TerritoryRadius),
        0.0f
    );

    PatrolTargetLocation = HomeLocation + RandomOffset;
    SetBehaviorState(EDinoBehaviorState::Patrolling);

    // Move to patrol point via AI controller
    AAIController* AICtrl = Cast<AAIController>(GetController());
    if (AICtrl)
    {
        AICtrl->MoveToLocation(PatrolTargetLocation, 50.0f, true, true, true, true, nullptr, false);
    }
}

float ADinosaurBase::GetHealthPercent() const
{
    if (MaxHealth <= 0.0f) return 0.0f;
    return CurrentHealth / MaxHealth;
}

bool ADinosaurBase::IsInTerritory(FVector Location) const
{
    return FVector::Dist(HomeLocation, Location) <= TerritoryRadius;
}

void ADinosaurBase::SetHomeLocation(FVector NewHome)
{
    HomeLocation = NewHome;
}

FVector ADinosaurBase::GetHomeLocation() const
{
    return HomeLocation;
}
