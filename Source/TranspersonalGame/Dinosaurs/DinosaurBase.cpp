// DinosaurBase.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Full implementation of the DinosaurBase actor (APawn subclass)
// Provides: species data, survival stats, patrol AI, threat response

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
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

    // Capsule root
    GetCapsuleComponent()->InitCapsuleSize(60.f, 120.f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // Skeletal mesh
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -120.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

    // Perception sphere — detects player within AggroRadius
    PerceptionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PerceptionSphere"));
    PerceptionSphere->SetupAttachment(RootComponent);
    PerceptionSphere->InitSphereRadius(1500.f);
    PerceptionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    PerceptionSphere->OnComponentBeginOverlap.AddDynamic(this, &ADinosaurBase::OnPerceptionOverlapBegin);
    PerceptionSphere->OnComponentEndOverlap.AddDynamic(this, &ADinosaurBase::OnPerceptionOverlapEnd);

    // Movement defaults — overridden per species in BeginPlay
    GetCharacterMovement()->MaxWalkSpeed = 400.f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
    GetCharacterMovement()->JumpZVelocity = 600.f;
    GetCharacterMovement()->GravityScale = 1.2f;

    // Default species data
    SpeciesData.SpeciesName = FName("UnknownDinosaur");
    SpeciesData.MaxHealth = 200.f;
    SpeciesData.MoveSpeed = 400.f;
    SpeciesData.AttackDamage = 30.f;
    SpeciesData.AggroRadius = 1500.f;
    SpeciesData.AttackRadius = 200.f;
    SpeciesData.Diet = EDinosaurDiet::Herbivore;
    SpeciesData.SocialBehavior = EDinosaurSocial::Solitary;
    SpeciesData.bIsNocturnal = false;

    // Runtime state
    CurrentHealth = SpeciesData.MaxHealth;
    CurrentState = EDinosaurState::Idle;
    bIsAlive = true;
    PatrolRadius = 2000.f;
    PatrolWaitTime = 3.f;
    bIsWaiting = false;
    ThreatActor = nullptr;
    HomeLocation = FVector::ZeroVector;
    LastKnownThreatLocation = FVector::ZeroVector;

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Record home location for patrol
    HomeLocation = GetActorLocation();

    // Apply species data to movement
    GetCharacterMovement()->MaxWalkSpeed = SpeciesData.MoveSpeed;
    PerceptionSphere->SetSphereRadius(SpeciesData.AggroRadius);

    // Initialize health
    CurrentHealth = SpeciesData.MaxHealth;

    // Start patrol loop
    GetWorldTimerManager().SetTimer(
        PatrolTimerHandle,
        this,
        &ADinosaurBase::ChooseNextPatrolPoint,
        PatrolWaitTime,
        false
    );

    // Hunger/thirst tick every 10s
    GetWorldTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ADinosaurBase::TickSurvivalNeeds,
        10.f,
        true
    );
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    switch (CurrentState)
    {
        case EDinosaurState::Patrolling:
            TickPatrol(DeltaTime);
            break;
        case EDinosaurState::Chasing:
            TickChase(DeltaTime);
            break;
        case EDinosaurState::Fleeing:
            TickFlee(DeltaTime);
            break;
        case EDinosaurState::Attacking:
            TickAttack(DeltaTime);
            break;
        case EDinosaurState::Idle:
        case EDinosaurState::Grazing:
        case EDinosaurState::Sleeping:
        default:
            break;
    }
}

// ─── SURVIVAL NEEDS ───────────────────────────────────────────────────────────

void ADinosaurBase::TickSurvivalNeeds()
{
    if (!bIsAlive) return;

    CurrentHunger = FMath::Clamp(CurrentHunger - 2.f, 0.f, 100.f);
    CurrentThirst = FMath::Clamp(CurrentThirst - 3.f, 0.f, 100.f);

    // Starvation damage
    if (CurrentHunger <= 0.f)
    {
        ApplyDamage(5.f, nullptr);
    }

    // Seek food if herbivore and hungry
    if (SpeciesData.Diet == EDinosaurDiet::Herbivore && CurrentHunger < 30.f)
    {
        if (CurrentState == EDinosaurState::Idle || CurrentState == EDinosaurState::Patrolling)
        {
            SetState(EDinosaurState::Grazing);
        }
    }
}

// ─── DAMAGE & DEATH ───────────────────────────────────────────────────────────

void ADinosaurBase::ApplyDamage(float DamageAmount, AActor* DamageSource)
{
    if (!bIsAlive) return;

    CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.f, SpeciesData.MaxHealth);

    OnDamageReceived(DamageAmount, DamageSource);

    if (CurrentHealth <= 0.f)
    {
        Die(DamageSource);
    }
    else if (DamageSource && CurrentState != EDinosaurState::Chasing && CurrentState != EDinosaurState::Attacking)
    {
        // Carnivores fight back; herbivores flee
        if (SpeciesData.Diet == EDinosaurDiet::Carnivore)
        {
            ThreatActor = DamageSource;
            SetState(EDinosaurState::Chasing);
        }
        else
        {
            ThreatActor = DamageSource;
            SetState(EDinosaurState::Fleeing);
        }
    }
}

void ADinosaurBase::Die(AActor* Killer)
{
    bIsAlive = false;
    SetState(EDinosaurState::Dead);

    // Stop all timers
    GetWorldTimerManager().ClearTimer(PatrolTimerHandle);
    GetWorldTimerManager().ClearTimer(HungerTimerHandle);

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PerceptionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Disable movement
    GetCharacterMovement()->DisableMovement();

    // Ragdoll
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

    OnDeath(Killer);

    // Destroy after 30s (corpse decay)
    SetLifeSpan(30.f);
}

// ─── STATE MACHINE ────────────────────────────────────────────────────────────

void ADinosaurBase::SetState(EDinosaurState NewState)
{
    if (CurrentState == NewState) return;

    EDinosaurState OldState = CurrentState;
    CurrentState = NewState;

    OnStateChanged(OldState, NewState);
}

// ─── PATROL ───────────────────────────────────────────────────────────────────

void ADinosaurBase::ChooseNextPatrolPoint()
{
    if (!bIsAlive || CurrentState == EDinosaurState::Chasing || CurrentState == EDinosaurState::Fleeing)
        return;

    SetState(EDinosaurState::Patrolling);

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys) return;

    FNavLocation NavLoc;
    bool bFound = NavSys->GetRandomReachablePointInRadius(HomeLocation, PatrolRadius, NavLoc);

    if (bFound)
    {
        CurrentPatrolTarget = NavLoc.Location;
        MoveToLocation(CurrentPatrolTarget);
    }
    else
    {
        // Retry after wait
        GetWorldTimerManager().SetTimer(PatrolTimerHandle, this, &ADinosaurBase::ChooseNextPatrolPoint, PatrolWaitTime, false);
    }
}

void ADinosaurBase::TickPatrol(float DeltaTime)
{
    if (bIsWaiting) return;

    float DistToTarget = FVector::Dist(GetActorLocation(), CurrentPatrolTarget);
    if (DistToTarget < 150.f)
    {
        // Reached patrol point — wait then choose next
        bIsWaiting = true;
        SetState(EDinosaurState::Idle);
        GetWorldTimerManager().SetTimer(
            PatrolTimerHandle,
            this,
            &ADinosaurBase::OnPatrolWaitComplete,
            PatrolWaitTime,
            false
        );
    }
}

void ADinosaurBase::OnPatrolWaitComplete()
{
    bIsWaiting = false;
    ChooseNextPatrolPoint();
}

void ADinosaurBase::MoveToLocation(const FVector& Destination)
{
    AAIController* AIC = Cast<AAIController>(GetController());
    if (AIC)
    {
        AIC->MoveToLocation(Destination, 50.f);
    }
}

// ─── CHASE ────────────────────────────────────────────────────────────────────

void ADinosaurBase::TickChase(float DeltaTime)
{
    if (!ThreatActor || !bIsAlive) return;

    float DistToThreat = FVector::Dist(GetActorLocation(), ThreatActor->GetActorLocation());

    // Within attack range — switch to attacking
    if (DistToThreat <= SpeciesData.AttackRadius)
    {
        SetState(EDinosaurState::Attacking);
        return;
    }

    // Lost sight (> 2x aggro radius) — return to patrol
    if (DistToThreat > SpeciesData.AggroRadius * 2.f)
    {
        ThreatActor = nullptr;
        SetState(EDinosaurState::Patrolling);
        ChooseNextPatrolPoint();
        return;
    }

    LastKnownThreatLocation = ThreatActor->GetActorLocation();
    MoveToLocation(LastKnownThreatLocation);
}

// ─── FLEE ─────────────────────────────────────────────────────────────────────

void ADinosaurBase::TickFlee(float DeltaTime)
{
    if (!ThreatActor || !bIsAlive) return;

    float DistToThreat = FVector::Dist(GetActorLocation(), ThreatActor->GetActorLocation());

    // Safe distance reached — resume patrol
    if (DistToThreat > SpeciesData.AggroRadius * 1.5f)
    {
        ThreatActor = nullptr;
        SetState(EDinosaurState::Patrolling);
        ChooseNextPatrolPoint();
        return;
    }

    // Move away from threat
    FVector AwayDir = (GetActorLocation() - ThreatActor->GetActorLocation()).GetSafeNormal();
    FVector FleeTarget = GetActorLocation() + AwayDir * 2000.f;
    MoveToLocation(FleeTarget);
}

// ─── ATTACK ───────────────────────────────────────────────────────────────────

void ADinosaurBase::TickAttack(float DeltaTime)
{
    if (!ThreatActor || !bIsAlive) return;

    float DistToThreat = FVector::Dist(GetActorLocation(), ThreatActor->GetActorLocation());

    // Target moved out of attack range — resume chase
    if (DistToThreat > SpeciesData.AttackRadius * 1.5f)
    {
        SetState(EDinosaurState::Chasing);
        return;
    }

    // Face the target
    FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), ThreatActor->GetActorLocation());
    SetActorRotation(FRotator(0.f, LookAt.Yaw, 0.f));

    // Perform attack (damage applied via Blueprint or animation notify)
    PerformAttack();
}

void ADinosaurBase::PerformAttack()
{
    // Subclasses override this — base applies direct damage
    if (!ThreatActor) return;

    // Simple sphere overlap for melee hit
    TArray<AActor*> OverlappingActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        GetActorLocation() + GetActorForwardVector() * SpeciesData.AttackRadius * 0.5f,
        SpeciesData.AttackRadius,
        ObjectTypes,
        nullptr,
        TArray<AActor*>{ this },
        OverlappingActors
    );

    for (AActor* Hit : OverlappingActors)
    {
        if (Hit == ThreatActor)
        {
            UGameplayStatics::ApplyDamage(Hit, SpeciesData.AttackDamage, GetController(), this, nullptr);
            break;
        }
    }
}

// ─── PERCEPTION CALLBACKS ─────────────────────────────────────────────────────

void ADinosaurBase::OnPerceptionOverlapBegin(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!bIsAlive || OtherActor == this) return;

    // Only react to pawns (player or other dinos)
    APawn* OtherPawn = Cast<APawn>(OtherActor);
    if (!OtherPawn) return;

    // Carnivores aggro on player
    if (SpeciesData.Diet == EDinosaurDiet::Carnivore)
    {
        APlayerController* PC = Cast<APlayerController>(OtherPawn->GetController());
        if (PC && CurrentState != EDinosaurState::Chasing && CurrentState != EDinosaurState::Attacking)
        {
            ThreatActor = OtherActor;
            SetState(EDinosaurState::Chasing);
        }
    }
    // Herbivores flee from player
    else if (SpeciesData.Diet == EDinosaurDiet::Herbivore)
    {
        APlayerController* PC = Cast<APlayerController>(OtherPawn->GetController());
        if (PC && CurrentState != EDinosaurState::Fleeing)
        {
            ThreatActor = OtherActor;
            SetState(EDinosaurState::Fleeing);
        }
    }
}

void ADinosaurBase::OnPerceptionOverlapEnd(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    if (OtherActor == ThreatActor)
    {
        // Only clear threat if we're not actively attacking
        if (CurrentState == EDinosaurState::Chasing || CurrentState == EDinosaurState::Fleeing)
        {
            ThreatActor = nullptr;
            SetState(EDinosaurState::Patrolling);
            ChooseNextPatrolPoint();
        }
    }
}

// ─── BLUEPRINT EVENTS (default no-op implementations) ─────────────────────────

void ADinosaurBase::OnDamageReceived_Implementation(float DamageAmount, AActor* DamageSource)
{
    // Override in Blueprint for hit reactions, sounds, VFX
}

void ADinosaurBase::OnDeath_Implementation(AActor* Killer)
{
    // Override in Blueprint for death animations, loot spawning, sound
}

void ADinosaurBase::OnStateChanged_Implementation(EDinosaurState OldState, EDinosaurState NewState)
{
    // Override in Blueprint for animation state machine transitions
}
