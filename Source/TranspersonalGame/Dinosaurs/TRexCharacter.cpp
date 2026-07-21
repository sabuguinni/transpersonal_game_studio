// TRexCharacter.cpp — Core Systems Programmer #03 — PROD_CYCLE_AUTO_20260701_002
// Tyrannosaurus Rex AI character — apex predator with territorial behavior

#include "TRexCharacter.h"
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
#include "DrawDebugHelpers.h"

ATRexCharacter::ATRexCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule — T-Rex is massive: ~4m tall, ~12m long
    GetCapsuleComponent()->InitCapsuleSize(120.0f, 220.0f);

    // Movement — T-Rex: 17-25 km/h walk, up to 45 km/h sprint
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    MoveComp->MaxWalkSpeed = 500.0f;          // ~18 km/h walk
    MoveComp->MaxAcceleration = 800.0f;
    MoveComp->BrakingDecelerationWalking = 600.0f;
    MoveComp->RotationRate = FRotator(0.0f, 120.0f, 0.0f);
    MoveComp->bOrientRotationToMovement = true;
    MoveComp->GravityScale = 1.0f;
    MoveComp->Mass = 8000.0f; // 8 tonnes

    // Survival stats
    MaxHealth = 2500.0f;
    CurrentHealth = 2500.0f;
    MaxHunger = 1000.0f;
    CurrentHunger = 600.0f;
    HungerDrainRate = 0.5f; // slow metabolism

    // Combat stats
    BiteDamage = 350.0f;
    BiteRange = 200.0f;
    AttackCooldown = 3.0f;
    CurrentAttackCooldown = 0.0f;

    // Behavior state
    CurrentBehaviorState = ECore_DinoState::Idle;
    bIsAlerted = false;
    bIsHungry = false;
    TerritoryRadius = 5000.0f; // 50m territory
    DetectionRange = 3000.0f;  // 30m sight
    ChaseRange = 8000.0f;      // 80m chase

    // Roar cooldown
    RoarCooldown = 30.0f;
    CurrentRoarCooldown = 0.0f;

    // AI Perception
    AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = DetectionRange;
    SightConfig->LoseSightRadius = ChaseRange;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    AIPerception->ConfigureSense(*SightConfig);

    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 4000.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    AIPerception->ConfigureSense(*HearingConfig);

    AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
    AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &ATRexCharacter::OnPerceptionUpdated);

    // BehaviorTree
    BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComp"));
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));

    // Spawn default controller
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ATRexCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Set home territory location
    TerritoryCenter = GetActorLocation();

    // Start hunger tick
    GetWorldTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ATRexCharacter::UpdateHunger,
        1.0f,
        true
    );

    // Start patrol behavior
    GetWorldTimerManager().SetTimer(
        PatrolTimerHandle,
        this,
        &ATRexCharacter::UpdatePatrol,
        5.0f,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("TRex spawned at %s — territory radius %.0f"),
        *GetActorLocation().ToString(), TerritoryRadius);
}

void ATRexCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update attack cooldown
    if (CurrentAttackCooldown > 0.0f)
    {
        CurrentAttackCooldown -= DeltaTime;
    }

    // Update roar cooldown
    if (CurrentRoarCooldown > 0.0f)
    {
        CurrentRoarCooldown -= DeltaTime;
    }

    // Chase target if set
    if (TargetActor && CurrentBehaviorState == ECore_DinoState::Chasing)
    {
        float DistToTarget = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());

        if (DistToTarget <= BiteRange)
        {
            // Attack
            PerformBiteAttack();
        }
        else if (DistToTarget > ChaseRange)
        {
            // Lost target
            TargetActor = nullptr;
            CurrentBehaviorState = ECore_DinoState::Returning;
        }
    }

    // Return to territory if too far
    if (CurrentBehaviorState == ECore_DinoState::Returning)
    {
        float DistToHome = FVector::Dist(GetActorLocation(), TerritoryCenter);
        if (DistToHome < 200.0f)
        {
            CurrentBehaviorState = ECore_DinoState::Idle;
        }
    }
}

void ATRexCharacter::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || Actor == this) return;

    // Check if it's the player
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (Actor != PlayerPawn) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Player detected
        TargetActor = Actor;
        bIsAlerted = true;
        CurrentBehaviorState = ECore_DinoState::Chasing;

        // Roar when first detecting player
        if (CurrentRoarCooldown <= 0.0f)
        {
            PerformRoar();
        }

        UE_LOG(LogTemp, Warning, TEXT("TRex detected player — CHASING!"));
    }
    else
    {
        // Lost sight
        if (TargetActor == Actor)
        {
            TargetActor = nullptr;
            CurrentBehaviorState = ECore_DinoState::Returning;
            UE_LOG(LogTemp, Log, TEXT("TRex lost sight of player — returning to territory"));
        }
    }
}

void ATRexCharacter::PerformBiteAttack()
{
    if (CurrentAttackCooldown > 0.0f) return;
    if (!TargetActor) return;

    float Dist = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
    if (Dist > BiteRange) return;

    // Apply damage
    UGameplayStatics::ApplyDamage(
        TargetActor,
        BiteDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    CurrentAttackCooldown = AttackCooldown;
    CurrentBehaviorState = ECore_DinoState::Attacking;

    UE_LOG(LogTemp, Warning, TEXT("TRex BITE ATTACK — %.0f damage to %s"),
        BiteDamage, *TargetActor->GetName());

    // Reset to chasing after attack
    FTimerHandle AttackResetTimer;
    GetWorldTimerManager().SetTimer(AttackResetTimer, [this]()
    {
        if (TargetActor)
            CurrentBehaviorState = ECore_DinoState::Chasing;
    }, 1.5f, false);
}

void ATRexCharacter::PerformRoar()
{
    if (CurrentRoarCooldown > 0.0f) return;

    CurrentRoarCooldown = RoarCooldown;
    CurrentBehaviorState = ECore_DinoState::Roaring;

    // Scare nearby prey (increase their fear)
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

    for (AActor* NearActor : NearbyActors)
    {
        if (!NearActor || NearActor == this) continue;
        float Dist = FVector::Dist(GetActorLocation(), NearActor->GetActorLocation());
        if (Dist < 2000.0f) // 20m roar radius
        {
            // Broadcast roar event — other systems can listen
            UE_LOG(LogTemp, Log, TEXT("TRex ROAR heard by %s at dist %.0f"),
                *NearActor->GetName(), Dist);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("TRex ROARS at %s"), *GetActorLocation().ToString());

    // Resume chasing after roar
    FTimerHandle RoarEndTimer;
    GetWorldTimerManager().SetTimer(RoarEndTimer, [this]()
    {
        if (TargetActor)
            CurrentBehaviorState = ECore_DinoState::Chasing;
        else
            CurrentBehaviorState = ECore_DinoState::Idle;
    }, 2.0f, false);
}

void ATRexCharacter::UpdateHunger()
{
    CurrentHunger -= HungerDrainRate;
    if (CurrentHunger < 0.0f) CurrentHunger = 0.0f;

    bIsHungry = (CurrentHunger < MaxHunger * 0.3f);

    if (bIsHungry && CurrentBehaviorState == ECore_DinoState::Idle)
    {
        // Hungry T-Rex actively hunts
        CurrentBehaviorState = ECore_DinoState::Hunting;
    }
}

void ATRexCharacter::UpdatePatrol()
{
    if (CurrentBehaviorState != ECore_DinoState::Idle &&
        CurrentBehaviorState != ECore_DinoState::Patrolling) return;

    // Pick random patrol point within territory
    FVector RandomOffset = FVector(
        FMath::RandRange(-TerritoryRadius * 0.5f, TerritoryRadius * 0.5f),
        FMath::RandRange(-TerritoryRadius * 0.5f, TerritoryRadius * 0.5f),
        0.0f
    );

    FVector PatrolTarget = TerritoryCenter + RandomOffset;

    // Move toward patrol point
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLoc;
        if (NavSys->GetRandomPointInNavigableRadius(PatrolTarget, 500.0f, NavLoc))
        {
            CurrentBehaviorState = ECore_DinoState::Patrolling;
            // Movement handled by AI controller
        }
    }
}

float ATRexCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth -= ActualDamage;
    CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        OnDeath();
    }
    else if (DamageCauser && CurrentBehaviorState != ECore_DinoState::Attacking)
    {
        // Retaliate against attacker
        TargetActor = DamageCauser;
        CurrentBehaviorState = ECore_DinoState::Chasing;
        PerformRoar();
    }

    return ActualDamage;
}

void ATRexCharacter::OnDeath()
{
    CurrentBehaviorState = ECore_DinoState::Dead;

    // Stop all timers
    GetWorldTimerManager().ClearTimer(HungerTimerHandle);
    GetWorldTimerManager().ClearTimer(PatrolTimerHandle);

    // Disable collision and movement
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    UE_LOG(LogTemp, Warning, TEXT("TRex DIED at %s"), *GetActorLocation().ToString());

    // Destroy after 30 seconds (carcass decay)
    SetLifeSpan(30.0f);
}
