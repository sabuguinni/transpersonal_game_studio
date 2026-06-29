// DinosaurBase.cpp
// Engine Architect Agent #02 — Transpersonal Game Studio
// Full implementation of the foundational dinosaur class

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
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule defaults — overridden per species
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // Movement defaults
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = 400.0f;
        MoveComp->MaxWalkSpeedCrouched = 200.0f;
        MoveComp->JumpZVelocity = 500.0f;
        MoveComp->GravityScale = 1.2f;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        MoveComp->NavAgentProps.bCanCrouch = false;
        MoveComp->NavAgentProps.bCanJump = true;
    }

    // AI Perception
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    PerceptionComponent->ConfigureSense(*SightConfig);

    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    PerceptionComponent->ConfigureSense(*HearingConfig);

    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurBase::OnPerceptionUpdated);

    // Default species stats
    DinosaurStats.MaxHealth = 300.0f;
    DinosaurStats.CurrentHealth = 300.0f;
    DinosaurStats.MaxStamina = 100.0f;
    DinosaurStats.CurrentStamina = 100.0f;
    DinosaurStats.AttackDamage = 50.0f;
    DinosaurStats.AttackRange = 200.0f;
    DinosaurStats.AttackCooldown = 1.5f;
    DinosaurStats.MoveSpeed = 400.0f;
    DinosaurStats.SprintSpeed = 800.0f;
    DinosaurStats.TurnRate = 360.0f;
    DinosaurStats.DetectionRadius = 2000.0f;
    DinosaurStats.AggressionLevel = 0.5f;
    DinosaurStats.FleeHealthThreshold = 0.2f;

    // Default behaviour
    CurrentBehaviorState = EDinosaurBehaviorState::Idle;
    DinosaurSpecies = EDinosaurSpecies::Raptor;
    bIsAlpha = false;
    bIsInPack = false;
    PackSize = 1;
    LastAttackTime = -999.0f;
    CurrentTarget = nullptr;
    HomeLocation = FVector::ZeroVector;
    PatrolRadius = 1500.0f;
    bIsRoaming = false;
    RoamInterval = 8.0f;

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    HomeLocation = GetActorLocation();

    // Apply species-specific stats
    ApplySpeciesStats();

    // Start roaming timer
    GetWorldTimerManager().SetTimer(
        RoamTimerHandle,
        this,
        &ADinosaurBase::OnRoamTimerFired,
        RoamInterval,
        true,
        FMath::RandRange(0.5f, RoamInterval)
    );

    // Start stamina regen timer
    GetWorldTimerManager().SetTimer(
        StaminaRegenTimerHandle,
        this,
        &ADinosaurBase::RegenerateStamina,
        0.5f,
        true,
        1.0f
    );

    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s spawned — Species: %d, Health: %.0f, State: Idle"),
        *GetName(), (int32)DinosaurSpecies, DinosaurStats.CurrentHealth);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateBehaviorState(DeltaTime);
}

void ADinosaurBase::ApplySpeciesStats()
{
    switch (DinosaurSpecies)
    {
        case EDinosaurSpecies::TRex:
            DinosaurStats.MaxHealth = 1500.0f;
            DinosaurStats.CurrentHealth = 1500.0f;
            DinosaurStats.AttackDamage = 200.0f;
            DinosaurStats.AttackRange = 350.0f;
            DinosaurStats.MoveSpeed = 350.0f;
            DinosaurStats.SprintSpeed = 700.0f;
            DinosaurStats.AggressionLevel = 0.9f;
            DinosaurStats.DetectionRadius = 3000.0f;
            GetCapsuleComponent()->InitCapsuleSize(100.0f, 200.0f);
            GetCharacterMovement()->MaxWalkSpeed = DinosaurStats.MoveSpeed;
            break;

        case EDinosaurSpecies::Raptor:
            DinosaurStats.MaxHealth = 200.0f;
            DinosaurStats.CurrentHealth = 200.0f;
            DinosaurStats.AttackDamage = 60.0f;
            DinosaurStats.AttackRange = 150.0f;
            DinosaurStats.MoveSpeed = 600.0f;
            DinosaurStats.SprintSpeed = 1000.0f;
            DinosaurStats.AggressionLevel = 0.8f;
            DinosaurStats.DetectionRadius = 2000.0f;
            GetCapsuleComponent()->InitCapsuleSize(40.0f, 80.0f);
            GetCharacterMovement()->MaxWalkSpeed = DinosaurStats.MoveSpeed;
            break;

        case EDinosaurSpecies::Brachiosaurus:
            DinosaurStats.MaxHealth = 3000.0f;
            DinosaurStats.CurrentHealth = 3000.0f;
            DinosaurStats.AttackDamage = 100.0f;
            DinosaurStats.AttackRange = 400.0f;
            DinosaurStats.MoveSpeed = 250.0f;
            DinosaurStats.SprintSpeed = 450.0f;
            DinosaurStats.AggressionLevel = 0.1f;
            DinosaurStats.FleeHealthThreshold = 0.4f;
            DinosaurStats.DetectionRadius = 1500.0f;
            GetCapsuleComponent()->InitCapsuleSize(150.0f, 300.0f);
            GetCharacterMovement()->MaxWalkSpeed = DinosaurStats.MoveSpeed;
            break;

        case EDinosaurSpecies::Triceratops:
            DinosaurStats.MaxHealth = 800.0f;
            DinosaurStats.CurrentHealth = 800.0f;
            DinosaurStats.AttackDamage = 120.0f;
            DinosaurStats.AttackRange = 250.0f;
            DinosaurStats.MoveSpeed = 350.0f;
            DinosaurStats.SprintSpeed = 650.0f;
            DinosaurStats.AggressionLevel = 0.5f;
            DinosaurStats.DetectionRadius = 1800.0f;
            GetCapsuleComponent()->InitCapsuleSize(90.0f, 130.0f);
            GetCharacterMovement()->MaxWalkSpeed = DinosaurStats.MoveSpeed;
            break;

        case EDinosaurSpecies::Pterodactyl:
            DinosaurStats.MaxHealth = 150.0f;
            DinosaurStats.CurrentHealth = 150.0f;
            DinosaurStats.AttackDamage = 40.0f;
            DinosaurStats.AttackRange = 100.0f;
            DinosaurStats.MoveSpeed = 800.0f;
            DinosaurStats.SprintSpeed = 1400.0f;
            DinosaurStats.AggressionLevel = 0.6f;
            DinosaurStats.DetectionRadius = 4000.0f;
            GetCapsuleComponent()->InitCapsuleSize(30.0f, 50.0f);
            GetCharacterMovement()->MaxWalkSpeed = DinosaurStats.MoveSpeed;
            GetCharacterMovement()->SetMovementMode(MOVE_Flying);
            break;

        case EDinosaurSpecies::Stegosaurus:
            DinosaurStats.MaxHealth = 600.0f;
            DinosaurStats.CurrentHealth = 600.0f;
            DinosaurStats.AttackDamage = 80.0f;
            DinosaurStats.AttackRange = 200.0f;
            DinosaurStats.MoveSpeed = 280.0f;
            DinosaurStats.SprintSpeed = 500.0f;
            DinosaurStats.AggressionLevel = 0.3f;
            DinosaurStats.DetectionRadius = 1200.0f;
            GetCapsuleComponent()->InitCapsuleSize(80.0f, 120.0f);
            GetCharacterMovement()->MaxWalkSpeed = DinosaurStats.MoveSpeed;
            break;

        default:
            break;
    }
}

void ADinosaurBase::UpdateBehaviorState(float DeltaTime)
{
    // Health-based flee check
    float HealthRatio = DinosaurStats.CurrentHealth / DinosaurStats.MaxHealth;
    if (HealthRatio <= DinosaurStats.FleeHealthThreshold && CurrentBehaviorState != EDinosaurBehaviorState::Fleeing)
    {
        SetBehaviorState(EDinosaurBehaviorState::Fleeing);
        return;
    }

    // State machine tick
    switch (CurrentBehaviorState)
    {
        case EDinosaurBehaviorState::Idle:
            // Handled by roam timer
            break;

        case EDinosaurBehaviorState::Roaming:
            // Movement handled by AI controller / nav mesh
            break;

        case EDinosaurBehaviorState::Hunting:
            if (CurrentTarget)
            {
                float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
                if (DistToTarget <= DinosaurStats.AttackRange)
                {
                    SetBehaviorState(EDinosaurBehaviorState::Attacking);
                }
            }
            else
            {
                SetBehaviorState(EDinosaurBehaviorState::Idle);
            }
            break;

        case EDinosaurBehaviorState::Attacking:
            if (CurrentTarget)
            {
                float Now = GetWorld()->GetTimeSeconds();
                if (Now - LastAttackTime >= DinosaurStats.AttackCooldown)
                {
                    PerformAttack(CurrentTarget);
                }
            }
            else
            {
                SetBehaviorState(EDinosaurBehaviorState::Idle);
            }
            break;

        case EDinosaurBehaviorState::Fleeing:
            // Move away from threat — handled by AI controller
            break;

        case EDinosaurBehaviorState::Resting:
            RegenerateStamina();
            break;

        default:
            break;
    }
}

void ADinosaurBase::SetBehaviorState(EDinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState == NewState) return;

    EDinosaurBehaviorState OldState = CurrentBehaviorState;
    CurrentBehaviorState = NewState;

    OnBehaviorStateChanged(OldState, NewState);

    UE_LOG(LogTemp, Verbose, TEXT("[DinosaurBase] %s state: %d → %d"),
        *GetName(), (int32)OldState, (int32)NewState);
}

void ADinosaurBase::OnBehaviorStateChanged(EDinosaurBehaviorState OldState, EDinosaurBehaviorState NewState)
{
    // Override in subclasses for species-specific reactions
    switch (NewState)
    {
        case EDinosaurBehaviorState::Hunting:
            GetCharacterMovement()->MaxWalkSpeed = DinosaurStats.SprintSpeed;
            break;
        case EDinosaurBehaviorState::Fleeing:
            GetCharacterMovement()->MaxWalkSpeed = DinosaurStats.SprintSpeed;
            break;
        case EDinosaurBehaviorState::Idle:
        case EDinosaurBehaviorState::Roaming:
            GetCharacterMovement()->MaxWalkSpeed = DinosaurStats.MoveSpeed;
            break;
        case EDinosaurBehaviorState::Resting:
            GetCharacterMovement()->MaxWalkSpeed = 0.0f;
            break;
        default:
            break;
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    DinosaurStats.CurrentHealth = FMath::Clamp(
        DinosaurStats.CurrentHealth - ActualDamage,
        0.0f,
        DinosaurStats.MaxHealth
    );

    OnDamageReceived(ActualDamage, DamageCauser);

    if (DinosaurStats.CurrentHealth <= 0.0f)
    {
        Die(DamageCauser);
    }

    return ActualDamage;
}

void ADinosaurBase::OnDamageReceived(float DamageAmount, AActor* DamageCauser)
{
    // Aggression response — attack back if not already hunting
    if (DamageCauser && DinosaurStats.AggressionLevel > 0.3f)
    {
        if (CurrentBehaviorState == EDinosaurBehaviorState::Idle ||
            CurrentBehaviorState == EDinosaurBehaviorState::Roaming)
        {
            CurrentTarget = DamageCauser;
            SetBehaviorState(EDinosaurBehaviorState::Hunting);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s took %.0f damage — HP: %.0f/%.0f"),
        *GetName(), DamageAmount, DinosaurStats.CurrentHealth, DinosaurStats.MaxHealth);
}

void ADinosaurBase::PerformAttack(AActor* Target)
{
    if (!Target || !Target->IsValidLowLevel()) return;

    float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistToTarget > DinosaurStats.AttackRange * 1.2f)
    {
        SetBehaviorState(EDinosaurBehaviorState::Hunting);
        return;
    }

    LastAttackTime = GetWorld()->GetTimeSeconds();

    // Apply damage
    UGameplayStatics::ApplyDamage(
        Target,
        DinosaurStats.AttackDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s attacks %s for %.0f damage"),
        *GetName(), *Target->GetName(), DinosaurStats.AttackDamage);
}

void ADinosaurBase::Die(AActor* Killer)
{
    SetBehaviorState(EDinosaurBehaviorState::Dead);
    CurrentTarget = nullptr;

    GetWorldTimerManager().ClearTimer(RoamTimerHandle);
    GetWorldTimerManager().ClearTimer(StaminaRegenTimerHandle);

    // Disable collision and movement
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s has died. Killer: %s"),
        *GetName(), Killer ? *Killer->GetName() : TEXT("Unknown"));

    // Destroy after 10 seconds (allow ragdoll/loot)
    SetLifeSpan(10.0f);
}

bool ADinosaurBase::IsAlive() const
{
    return DinosaurStats.CurrentHealth > 0.0f &&
           CurrentBehaviorState != EDinosaurBehaviorState::Dead;
}

float ADinosaurBase::GetHealthPercent() const
{
    if (DinosaurStats.MaxHealth <= 0.0f) return 0.0f;
    return DinosaurStats.CurrentHealth / DinosaurStats.MaxHealth;
}

void ADinosaurBase::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || Actor == this) return;

    // Check if it's the player
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (Actor == PlayerPawn && Stimulus.WasSuccessfullySensed())
    {
        if (DinosaurStats.AggressionLevel >= 0.5f &&
            CurrentBehaviorState != EDinosaurBehaviorState::Attacking &&
            CurrentBehaviorState != EDinosaurBehaviorState::Dead)
        {
            CurrentTarget = Actor;
            SetBehaviorState(EDinosaurBehaviorState::Hunting);
            UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s detected player — switching to Hunt"), *GetName());
        }
    }
}

void ADinosaurBase::OnRoamTimerFired()
{
    if (CurrentBehaviorState != EDinosaurBehaviorState::Idle &&
        CurrentBehaviorState != EDinosaurBehaviorState::Roaming)
    {
        return;
    }

    // Pick a random point within patrol radius
    FVector RandomOffset = FVector(
        FMath::RandRange(-PatrolRadius, PatrolRadius),
        FMath::RandRange(-PatrolRadius, PatrolRadius),
        0.0f
    );
    FVector TargetLocation = HomeLocation + RandomOffset;

    // Use nav mesh to move
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLoc;
        if (NavSys->GetRandomReachablePointInRadius(TargetLocation, PatrolRadius * 0.5f, NavLoc))
        {
            AAIController* AICtrl = Cast<AAIController>(GetController());
            if (AICtrl)
            {
                AICtrl->MoveToLocation(NavLoc.Location, 50.0f);
                SetBehaviorState(EDinosaurBehaviorState::Roaming);
            }
        }
    }
}

void ADinosaurBase::RegenerateStamina()
{
    if (CurrentBehaviorState == EDinosaurBehaviorState::Resting ||
        CurrentBehaviorState == EDinosaurBehaviorState::Idle)
    {
        DinosaurStats.CurrentStamina = FMath::Min(
            DinosaurStats.CurrentStamina + 5.0f,
            DinosaurStats.MaxStamina
        );
    }
}
