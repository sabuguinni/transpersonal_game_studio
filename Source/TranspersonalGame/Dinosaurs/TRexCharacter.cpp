// TRexCharacter.cpp — Tyrannosaurus Rex implementation for prehistoric survival game
// Agent #3 — Core Systems Programmer | Transpersonal Game Studio

#include "Dinosaurs/TRexCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ATRexCharacter::ATRexCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // T-Rex is massive — adjust capsule
    GetCapsuleComponent()->SetCapsuleHalfHeight(220.0f);
    GetCapsuleComponent()->SetCapsuleRadius(80.0f);

    // T-Rex movement — powerful but not super fast
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement)
    {
        Movement->MaxWalkSpeed = 600.0f;         // ~22 km/h — realistic T-Rex sprint
        Movement->MaxAcceleration = 400.0f;
        Movement->BrakingDecelerationWalking = 300.0f;
        Movement->RotationRate = FRotator(0.0f, 120.0f, 0.0f);
        Movement->bOrientRotationToMovement = true;
        Movement->GravityScale = 1.2f;           // Heavier feel
        Movement->JumpZVelocity = 0.0f;          // T-Rex cannot jump
        Movement->bCanWalkOffLedges = true;
        Movement->NavAgentProps.AgentRadius = 80.0f;
        Movement->NavAgentProps.AgentHeight = 440.0f;
    }

    // T-Rex stats
    MaxHealth = 2500.0f;
    CurrentHealth = 2500.0f;
    BaseDamage = 180.0f;
    AttackRange = 250.0f;
    DetectionRange = 4000.0f;
    HearingRange = 6000.0f;
    PatrolRadius = 2000.0f;
    bIsAggressive = true;
    bIsPredator = true;
    DinosaurSpecies = ECore_DinosaurSpecies::TRex;
    CurrentBehaviorState = ECore_DinosaurBehavior::Patrolling;
    bIsAlpha = true;
    PackSize = 1; // T-Rex is solitary
    CurrentStamina = 100.0f;
    MaxStamina = 100.0f;
    StaminaRegenRate = 5.0f;
    AttackCooldown = 2.5f;
    TimeSinceLastAttack = 0.0f;
    bIsAttacking = false;
    bIsRoaring = false;
    RoarCooldown = 30.0f;
    TimeSinceLastRoar = 0.0f;
    TerritoryRadius = 5000.0f;
    bHasTerritory = true;
    CurrentTarget = nullptr;
    bIsChasing = false;
    ChaseSpeed = 650.0f;
    PatrolSpeed = 200.0f;

    // AI Perception
    AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));

    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = DetectionRange;
    SightConfig->LoseSightRadius = DetectionRange * 1.5f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    AIPerceptionComp->ConfigureSense(*SightConfig);

    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRange;
    HearingConfig->SetMaxAge(3.0f);
    AIPerceptionComp->ConfigureSense(*HearingConfig);

    AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
    AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ATRexCharacter::OnPerceptionUpdated);
}

void ATRexCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Store home territory center
    TerritoryCenter = GetActorLocation();

    // Set patrol speed initially
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement)
    {
        Movement->MaxWalkSpeed = PatrolSpeed;
    }

    UE_LOG(LogTemp, Log, TEXT("TRexCharacter: Spawned at %s, Territory center set"), *GetActorLocation().ToString());
}

void ATRexCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update timers
    TimeSinceLastAttack += DeltaTime;
    TimeSinceLastRoar += DeltaTime;

    // Stamina regen when not chasing
    if (!bIsChasing && CurrentStamina < MaxStamina)
    {
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + StaminaRegenRate * DeltaTime);
    }

    // Drain stamina when chasing
    if (bIsChasing && CurrentTarget)
    {
        CurrentStamina = FMath::Max(0.0f, CurrentStamina - 10.0f * DeltaTime);

        // Give up chase if exhausted
        if (CurrentStamina <= 0.0f)
        {
            StopChasing();
            UE_LOG(LogTemp, Log, TEXT("TRexCharacter: Exhausted — gave up chase"));
        }
    }

    // Territory enforcement — return home if too far
    if (!bIsChasing && bHasTerritory)
    {
        float DistFromTerritory = FVector::Dist(GetActorLocation(), TerritoryCenter);
        if (DistFromTerritory > TerritoryRadius * 1.5f)
        {
            ReturnToTerritory();
        }
    }

    // Periodic roar
    if (TimeSinceLastRoar >= RoarCooldown && !bIsAttacking)
    {
        PerformRoar();
    }
}

float ATRexCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

    UE_LOG(LogTemp, Log, TEXT("TRexCharacter: Took %.1f damage. Health: %.1f/%.1f"),
        ActualDamage, CurrentHealth, MaxHealth);

    // Become enraged when below 30% health
    if (CurrentHealth < MaxHealth * 0.3f && !bIsChasing)
    {
        bIsAggressive = true;
        BaseDamage *= 1.5f; // Enrage bonus
        UE_LOG(LogTemp, Warning, TEXT("TRexCharacter: ENRAGED — damage boosted!"));
    }

    // Aggro attacker
    if (DamageCauser && !bIsChasing)
    {
        CurrentTarget = DamageCauser;
        StartChasing(DamageCauser);
    }

    if (CurrentHealth <= 0.0f)
    {
        OnDeath();
    }

    return ActualDamage;
}

void ATRexCharacter::PerformAttack()
{
    if (TimeSinceLastAttack < AttackCooldown) return;
    if (!CurrentTarget) return;

    float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
    if (DistToTarget > AttackRange) return;

    bIsAttacking = true;
    TimeSinceLastAttack = 0.0f;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(
        CurrentTarget,
        BaseDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    UE_LOG(LogTemp, Log, TEXT("TRexCharacter: ATTACK! Dealt %.1f damage to %s"),
        BaseDamage, *CurrentTarget->GetName());

    // Reset attack flag after animation would play
    FTimerHandle AttackResetTimer;
    GetWorldTimerManager().SetTimer(AttackResetTimer, [this]()
    {
        bIsAttacking = false;
    }, 1.5f, false);
}

void ATRexCharacter::PerformRoar()
{
    if (bIsAttacking) return;

    bIsRoaring = true;
    TimeSinceLastRoar = 0.0f;

    // Scare nearby prey — reduce their stamina/courage
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;

        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist < 3000.0f) // Roar radius
        {
            // Broadcast roar event — other systems (SurvivalComponent) can respond
            UE_LOG(LogTemp, Log, TEXT("TRexCharacter: ROAR heard by %s at distance %.0f"),
                *Actor->GetName(), Dist);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("TRexCharacter: ROAR!"));

    FTimerHandle RoarResetTimer;
    GetWorldTimerManager().SetTimer(RoarResetTimer, [this]()
    {
        bIsRoaring = false;
    }, 3.0f, false);
}

void ATRexCharacter::StartChasing(AActor* Target)
{
    if (!Target) return;

    CurrentTarget = Target;
    bIsChasing = true;
    CurrentBehaviorState = ECore_DinosaurBehavior::Chasing;

    // Increase speed for chase
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement)
    {
        Movement->MaxWalkSpeed = ChaseSpeed;
    }

    UE_LOG(LogTemp, Log, TEXT("TRexCharacter: Chasing %s"), *Target->GetName());
}

void ATRexCharacter::StopChasing()
{
    bIsChasing = false;
    CurrentTarget = nullptr;
    CurrentBehaviorState = ECore_DinosaurBehavior::Patrolling;

    // Return to patrol speed
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement)
    {
        Movement->MaxWalkSpeed = PatrolSpeed;
    }
}

void ATRexCharacter::ReturnToTerritory()
{
    CurrentBehaviorState = ECore_DinosaurBehavior::Patrolling;

    // Move toward territory center
    FVector DirectionHome = (TerritoryCenter - GetActorLocation()).GetSafeNormal();
    AddMovementInput(DirectionHome, 1.0f);
}

void ATRexCharacter::OnDeath()
{
    CurrentBehaviorState = ECore_DinosaurBehavior::Dead;
    bIsChasing = false;

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Disable tick
    SetActorTickEnabled(false);

    UE_LOG(LogTemp, Warning, TEXT("TRexCharacter: DEAD at %s"), *GetActorLocation().ToString());

    // Destroy after delay (let death animation play)
    FTimerHandle DeathTimer;
    GetWorldTimerManager().SetTimer(DeathTimer, [this]()
    {
        Destroy();
    }, 30.0f, false);
}

void ATRexCharacter::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || Actor == this) return;

    // Check if it's a player or prey
    APawn* PerceivedPawn = Cast<APawn>(Actor);
    if (!PerceivedPawn) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Detected prey/player
        if (bIsAggressive && !bIsChasing)
        {
            StartChasing(Actor);
        }
    }
    else
    {
        // Lost sight of target
        if (CurrentTarget == Actor)
        {
            // Keep last known position for a bit, then give up
            FTimerHandle LoseTargetTimer;
            GetWorldTimerManager().SetTimer(LoseTargetTimer, [this]()
            {
                if (!AIPerceptionComp->HasAnyCurrentStimulus(*CurrentTarget))
                {
                    StopChasing();
                    UE_LOG(LogTemp, Log, TEXT("TRexCharacter: Lost target — returning to patrol"));
                }
            }, 5.0f, false);
        }
    }
}

bool ATRexCharacter::IsTargetInAttackRange() const
{
    if (!CurrentTarget) return false;
    return FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation()) <= AttackRange;
}

float ATRexCharacter::GetHealthPercent() const
{
    return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}
