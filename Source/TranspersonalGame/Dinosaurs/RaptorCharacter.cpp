// RaptorCharacter.cpp — Core Systems Programmer #03 — PROD_CYCLE_AUTO_20260701_002
// Velociraptor AI character — pack hunter with flanking and coordinated attack behavior

#include "RaptorCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

ARaptorCharacter::ARaptorCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule — Raptor: ~1.8m tall, ~2.5m long
    GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);

    // Movement — Raptors are fast: ~60 km/h sprint
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    MoveComp->MaxWalkSpeed = 700.0f;           // ~25 km/h patrol
    MoveComp->MaxAcceleration = 2000.0f;       // very agile
    MoveComp->BrakingDecelerationWalking = 1200.0f;
    MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f); // very agile turning
    MoveComp->bOrientRotationToMovement = true;
    MoveComp->GravityScale = 1.0f;
    MoveComp->Mass = 80.0f; // ~80kg

    // Survival stats
    MaxHealth = 250.0f;
    CurrentHealth = 250.0f;
    MaxHunger = 300.0f;
    CurrentHunger = 180.0f;
    HungerDrainRate = 1.2f; // faster metabolism than T-Rex

    // Combat stats — raptors use claw slashes
    ClawDamage = 65.0f;
    ClawRange = 120.0f;
    AttackCooldown = 1.2f;
    CurrentAttackCooldown = 0.0f;
    JumpAttackDamage = 120.0f;
    bCanJumpAttack = true;
    JumpAttackCooldown = 8.0f;
    CurrentJumpAttackCooldown = 0.0f;

    // Pack behavior
    PackLeader = nullptr;
    PackIndex = 0;
    bIsPackLeader = false;
    PackCoordinationRadius = 1500.0f; // 15m pack cohesion
    FlankAngle = 45.0f;

    // Behavior state
    CurrentBehaviorState = ECore_DinoState::Idle;
    bIsAlerted = false;
    TerritoryRadius = 3000.0f;
    DetectionRange = 2500.0f;
    ChaseRange = 6000.0f;

    // Vocalization
    ChirpCooldown = 5.0f;
    CurrentChirpCooldown = 0.0f;

    // AI Perception
    AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = DetectionRange;
    SightConfig->LoseSightRadius = ChaseRange;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f; // wider FOV than T-Rex
    SightConfig->SetMaxAge(4.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    AIPerception->ConfigureSense(*SightConfig);

    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 3000.0f;
    HearingConfig->SetMaxAge(2.5f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    AIPerception->ConfigureSense(*HearingConfig);

    AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
    AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &ARaptorCharacter::OnPerceptionUpdated);

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ARaptorCharacter::BeginPlay()
{
    Super::BeginPlay();

    TerritoryCenter = GetActorLocation();

    // Hunger tick
    GetWorldTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ARaptorCharacter::UpdateHunger,
        1.0f,
        true
    );

    // Pack coordination tick
    GetWorldTimerManager().SetTimer(
        PackCoordTimerHandle,
        this,
        &ARaptorCharacter::UpdatePackCoordination,
        2.0f,
        true
    );

    // Patrol tick
    GetWorldTimerManager().SetTimer(
        PatrolTimerHandle,
        this,
        &ARaptorCharacter::UpdatePatrol,
        4.0f,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("Raptor[%d] spawned — PackLeader: %s"),
        PackIndex, bIsPackLeader ? TEXT("YES") : TEXT("NO"));
}

void ARaptorCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update cooldowns
    if (CurrentAttackCooldown > 0.0f) CurrentAttackCooldown -= DeltaTime;
    if (CurrentJumpAttackCooldown > 0.0f) CurrentJumpAttackCooldown -= DeltaTime;
    if (CurrentChirpCooldown > 0.0f) CurrentChirpCooldown -= DeltaTime;

    // Sprint boost when chasing
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (CurrentBehaviorState == ECore_DinoState::Chasing)
    {
        MoveComp->MaxWalkSpeed = 1650.0f; // ~60 km/h sprint
    }
    else
    {
        MoveComp->MaxWalkSpeed = 700.0f;
    }

    // Chase/attack logic
    if (TargetActor && (CurrentBehaviorState == ECore_DinoState::Chasing ||
                        CurrentBehaviorState == ECore_DinoState::Attacking))
    {
        float DistToTarget = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());

        if (DistToTarget <= ClawRange)
        {
            // Try jump attack first if available
            if (bCanJumpAttack && CurrentJumpAttackCooldown <= 0.0f && DistToTarget < ClawRange * 2.0f)
            {
                PerformJumpAttack();
            }
            else
            {
                PerformClawAttack();
            }
        }
        else if (DistToTarget > ChaseRange)
        {
            TargetActor = nullptr;
            CurrentBehaviorState = ECore_DinoState::Returning;
        }
    }

    // Return to territory
    if (CurrentBehaviorState == ECore_DinoState::Returning)
    {
        float DistToHome = FVector::Dist(GetActorLocation(), TerritoryCenter);
        if (DistToHome < 150.0f)
        {
            CurrentBehaviorState = ECore_DinoState::Idle;
        }
    }
}

void ARaptorCharacter::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || Actor == this) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (Actor != PlayerPawn) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        TargetActor = Actor;
        bIsAlerted = true;
        CurrentBehaviorState = ECore_DinoState::Chasing;

        // Alert pack members
        if (bIsPackLeader)
        {
            AlertPackMembers(Actor);
        }

        // Chirp alert
        if (CurrentChirpCooldown <= 0.0f)
        {
            PerformAlertChirp();
        }

        UE_LOG(LogTemp, Warning, TEXT("Raptor[%d] detected player — CHASING! Pack alerted."), PackIndex);
    }
    else
    {
        if (TargetActor == Actor)
        {
            TargetActor = nullptr;
            CurrentBehaviorState = ECore_DinoState::Returning;
        }
    }
}

void ARaptorCharacter::AlertPackMembers(AActor* Target)
{
    // Find other raptors nearby and alert them
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARaptorCharacter::StaticClass(), NearbyActors);

    int32 AlertedCount = 0;
    for (AActor* NearActor : NearbyActors)
    {
        if (!NearActor || NearActor == this) continue;

        ARaptorCharacter* PackMember = Cast<ARaptorCharacter>(NearActor);
        if (!PackMember) continue;

        float Dist = FVector::Dist(GetActorLocation(), PackMember->GetActorLocation());
        if (Dist < PackCoordinationRadius)
        {
            PackMember->ReceivePackAlert(Target, this);
            AlertedCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Raptor[%d] (leader) alerted %d pack members"), PackIndex, AlertedCount);
}

void ARaptorCharacter::ReceivePackAlert(AActor* Target, ARaptorCharacter* AlertingRaptor)
{
    if (!Target) return;

    TargetActor = Target;
    bIsAlerted = true;
    CurrentBehaviorState = ECore_DinoState::Chasing;

    // Calculate flanking position based on pack index
    // Each raptor approaches from a different angle to surround the target
    float FlankOffset = PackIndex * 90.0f; // 0°, 90°, 180°, 270° flanking positions
    FVector TargetLoc = Target->GetActorLocation();
    FVector FlankDir = FVector(
        FMath::Cos(FMath::DegreesToRadians(FlankOffset)),
        FMath::Sin(FMath::DegreesToRadians(FlankOffset)),
        0.0f
    );
    FlankTargetPosition = TargetLoc + FlankDir * 300.0f;

    UE_LOG(LogTemp, Log, TEXT("Raptor[%d] received pack alert — flanking from angle %.0f"),
        PackIndex, FlankOffset);
}

void ARaptorCharacter::PerformClawAttack()
{
    if (CurrentAttackCooldown > 0.0f) return;
    if (!TargetActor) return;

    float Dist = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
    if (Dist > ClawRange) return;

    UGameplayStatics::ApplyDamage(
        TargetActor,
        ClawDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    CurrentAttackCooldown = AttackCooldown;
    CurrentBehaviorState = ECore_DinoState::Attacking;

    UE_LOG(LogTemp, Warning, TEXT("Raptor[%d] CLAW ATTACK — %.0f damage"),
        PackIndex, ClawDamage);

    FTimerHandle AttackResetTimer;
    GetWorldTimerManager().SetTimer(AttackResetTimer, [this]()
    {
        if (TargetActor) CurrentBehaviorState = ECore_DinoState::Chasing;
    }, 0.8f, false);
}

void ARaptorCharacter::PerformJumpAttack()
{
    if (CurrentJumpAttackCooldown > 0.0f) return;
    if (!TargetActor) return;

    // Launch toward target
    FVector ToTarget = (TargetActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector JumpVelocity = ToTarget * 1200.0f + FVector(0, 0, 600.0f);
    GetCharacterMovement()->Launch(JumpVelocity);

    // Apply damage on landing (slight delay)
    FTimerHandle JumpDamageTimer;
    GetWorldTimerManager().SetTimer(JumpDamageTimer, [this]()
    {
        if (!TargetActor) return;
        float Dist = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
        if (Dist < ClawRange * 1.5f)
        {
            UGameplayStatics::ApplyDamage(
                TargetActor,
                JumpAttackDamage,
                GetController(),
                this,
                UDamageType::StaticClass()
            );
            UE_LOG(LogTemp, Warning, TEXT("Raptor[%d] JUMP ATTACK — %.0f damage!"),
                PackIndex, JumpAttackDamage);
        }
    }, 0.5f, false);

    CurrentJumpAttackCooldown = JumpAttackCooldown;
    bCanJumpAttack = false;

    // Re-enable jump attack after cooldown
    FTimerHandle JumpResetTimer;
    GetWorldTimerManager().SetTimer(JumpResetTimer, [this]()
    {
        bCanJumpAttack = true;
    }, JumpAttackCooldown, false);
}

void ARaptorCharacter::PerformAlertChirp()
{
    if (CurrentChirpCooldown > 0.0f) return;
    CurrentChirpCooldown = ChirpCooldown;
    UE_LOG(LogTemp, Log, TEXT("Raptor[%d] CHIRP — alerting pack"), PackIndex);
}

void ARaptorCharacter::UpdateHunger()
{
    CurrentHunger -= HungerDrainRate;
    if (CurrentHunger < 0.0f) CurrentHunger = 0.0f;

    bool bWasHungry = bIsHungry;
    bIsHungry = (CurrentHunger < MaxHunger * 0.35f);

    if (bIsHungry && !bWasHungry && CurrentBehaviorState == ECore_DinoState::Idle)
    {
        CurrentBehaviorState = ECore_DinoState::Hunting;
        UE_LOG(LogTemp, Log, TEXT("Raptor[%d] is HUNGRY — entering hunt mode"), PackIndex);
    }
}

void ARaptorCharacter::UpdatePackCoordination()
{
    if (!bIsPackLeader) return;
    if (CurrentBehaviorState != ECore_DinoState::Chasing) return;
    if (!TargetActor) return;

    // Re-alert pack periodically during chase
    AlertPackMembers(TargetActor);
}

void ARaptorCharacter::UpdatePatrol()
{
    if (CurrentBehaviorState != ECore_DinoState::Idle &&
        CurrentBehaviorState != ECore_DinoState::Patrolling) return;

    // Raptors patrol in tighter formation than T-Rex
    FVector RandomOffset = FVector(
        FMath::RandRange(-TerritoryRadius * 0.4f, TerritoryRadius * 0.4f),
        FMath::RandRange(-TerritoryRadius * 0.4f, TerritoryRadius * 0.4f),
        0.0f
    );

    FVector PatrolTarget = TerritoryCenter + RandomOffset;
    CurrentBehaviorState = ECore_DinoState::Patrolling;

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLoc;
        if (NavSys->GetRandomPointInNavigableRadius(PatrolTarget, 400.0f, NavLoc))
        {
            // AI controller handles actual movement
        }
    }
}

float ARaptorCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth -= ActualDamage;
    CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        OnDeath();
    }
    else if (DamageCauser)
    {
        // Raptors retaliate and call pack
        TargetActor = DamageCauser;
        CurrentBehaviorState = ECore_DinoState::Chasing;
        if (bIsPackLeader) AlertPackMembers(DamageCauser);
        PerformAlertChirp();
    }

    return ActualDamage;
}

void ARaptorCharacter::OnDeath()
{
    CurrentBehaviorState = ECore_DinoState::Dead;

    GetWorldTimerManager().ClearTimer(HungerTimerHandle);
    GetWorldTimerManager().ClearTimer(PackCoordTimerHandle);
    GetWorldTimerManager().ClearTimer(PatrolTimerHandle);

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    UE_LOG(LogTemp, Warning, TEXT("Raptor[%d] DIED at %s"), PackIndex, *GetActorLocation().ToString());

    SetLifeSpan(20.0f);
}
