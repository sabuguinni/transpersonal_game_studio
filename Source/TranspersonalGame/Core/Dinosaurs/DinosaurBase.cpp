// DinosaurBase.cpp
// Core Systems Programmer — Agent #3
// Full implementation of ADinosaurBase — prehistoric survival game.

#include "Core/Dinosaurs/DinosaurBase.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Detection sphere ───────────────────────────────────────────────────
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(1500.f);
    DetectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ADinosaurBase::OnDetectionSphereOverlap);
    DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ADinosaurBase::OnDetectionSphereEndOverlap);

    // ── Territory sphere ───────────────────────────────────────────────────
    TerritorySphere = CreateDefaultSubobject<USphereComponent>(TEXT("TerritorySphere"));
    TerritorySphere->SetupAttachment(RootComponent);
    TerritorySphere->SetSphereRadius(3000.f);
    TerritorySphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    TerritorySphere->SetVisibility(false);

    // ── Movement defaults ──────────────────────────────────────────────────
    GetCharacterMovement()->MaxWalkSpeed = DinoStats.WalkSpeed;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 180.f, 0.f);
    GetCharacterMovement()->GravityScale = 1.2f;
    GetCharacterMovement()->JumpZVelocity = 400.f;

    // ── Capsule defaults ───────────────────────────────────────────────────
    GetCapsuleComponent()->SetCapsuleHalfHeight(88.f);
    GetCapsuleComponent()->SetCapsuleRadius(34.f);

    // ── Rotation ───────────────────────────────────────────────────────────
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Set territory center to spawn location
    TerritoryCenter = GetActorLocation();

    // Apply stats to movement
    GetCharacterMovement()->MaxWalkSpeed = DinoStats.WalkSpeed;
    DetectionSphere->SetSphereRadius(DinoStats.DetectionRadius);
    TerritorySphere->SetSphereRadius(DinoStats.TerritoryRadius);

    // Start hunger drain timer
    GetWorldTimerManager().SetTimer(
        HungerTimer,
        this,
        &ADinosaurBase::DrainHunger,
        5.f,   // every 5 seconds
        true
    );

    // Auto-start patrol if points are defined
    if (PatrolPoints.Num() > 0)
    {
        StartPatrol();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsAlive) return;

    TickAggression(DeltaTime);

    if (bIsPatrolling && AggressionState == EDinoAggressionState::Passive)
    {
        TickPatrol(DeltaTime);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// TakeDamage
// ─────────────────────────────────────────────────────────────────────────────

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                  AController* EventInstigator, AActor* DamageCauser)
{
    if (!bIsAlive) return 0.f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    DinoStats.Health = FMath::Clamp(DinoStats.Health - DamageAmount, 0.f, DinoStats.MaxHealth);

    // React to damage — become aggressive
    if (DamageCauser && AggressionState != EDinoAggressionState::Fleeing)
    {
        if (bIsHerbivore && DinoStats.Health < DinoStats.MaxHealth * 0.3f)
        {
            // Herbivores flee when badly hurt
            SetAggressionState(EDinoAggressionState::Fleeing);
        }
        else
        {
            AlertToThreat(DamageCauser);
        }
    }

    if (DinoStats.Health <= 0.f)
    {
        Die();
    }

    return DamageAmount;
}

// ─────────────────────────────────────────────────────────────────────────────
// Combat
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurBase::PerformAttack()
{
    if (!bIsAlive || !bCanAttack || !CurrentTarget) return;

    if (IsInAttackRange(CurrentTarget))
    {
        // Apply damage to target
        UGameplayStatics::ApplyDamage(
            CurrentTarget,
            DinoStats.AttackDamage,
            GetController(),
            this,
            UDamageType::StaticClass()
        );

        // Attack cooldown (1.5 seconds between attacks)
        bCanAttack = false;
        GetWorldTimerManager().SetTimer(
            AttackCooldownTimer,
            this,
            &ADinosaurBase::EnableAttack,
            1.5f,
            false
        );
    }
}

bool ADinosaurBase::IsInAttackRange(AActor* Target) const
{
    if (!Target) return false;
    float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    return Distance <= DinoStats.AttackRange;
}

void ADinosaurBase::EnableAttack()
{
    bCanAttack = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Aggression
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurBase::SetAggressionState(EDinoAggressionState NewState)
{
    if (AggressionState == NewState) return;

    AggressionState = NewState;

    switch (NewState)
    {
    case EDinoAggressionState::Passive:
        GetCharacterMovement()->MaxWalkSpeed = DinoStats.WalkSpeed;
        CurrentTarget = nullptr;
        break;

    case EDinoAggressionState::Alerted:
        GetCharacterMovement()->MaxWalkSpeed = DinoStats.WalkSpeed * 1.3f;
        break;

    case EDinoAggressionState::Aggressive:
        GetCharacterMovement()->MaxWalkSpeed = DinoStats.RunSpeed;
        break;

    case EDinoAggressionState::Fleeing:
        GetCharacterMovement()->MaxWalkSpeed = DinoStats.RunSpeed * 1.2f;
        CurrentTarget = nullptr;
        break;

    case EDinoAggressionState::Feeding:
        GetCharacterMovement()->MaxWalkSpeed = 0.f;
        break;

    case EDinoAggressionState::Sleeping:
        GetCharacterMovement()->MaxWalkSpeed = 0.f;
        break;
    }
}

void ADinosaurBase::AlertToThreat(AActor* ThreatActor)
{
    if (!ThreatActor || !bIsAlive) return;

    CurrentTarget = ThreatActor;

    if (bIsHerbivore)
    {
        SetAggressionState(EDinoAggressionState::Fleeing);
    }
    else
    {
        SetAggressionState(EDinoAggressionState::Aggressive);
    }
}

void ADinosaurBase::TickAggression(float DeltaTime)
{
    if (AggressionState == EDinoAggressionState::Passive ||
        AggressionState == EDinoAggressionState::Sleeping) return;

    if (AggressionState == EDinoAggressionState::Aggressive && CurrentTarget)
    {
        // Move toward target
        FVector Direction = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        AddMovementInput(Direction, 1.f);

        // Attack if in range
        PerformAttack();

        // Check if target is too far from territory — give up chase
        if (!IsInTerritory(CurrentTarget->GetActorLocation()))
        {
            SetAggressionState(EDinoAggressionState::Passive);
        }
    }
    else if (AggressionState == EDinoAggressionState::Fleeing)
    {
        // Move away from territory center (flee outward)
        if (CurrentTarget)
        {
            FVector FleeDir = (GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
            AddMovementInput(FleeDir, 1.f);
        }

        // Return to passive after fleeing far enough
        float DistFromCenter = FVector::Dist(GetActorLocation(), TerritoryCenter);
        if (DistFromCenter > DinoStats.TerritoryRadius * 1.5f)
        {
            SetAggressionState(EDinoAggressionState::Passive);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Territory
// ─────────────────────────────────────────────────────────────────────────────

bool ADinosaurBase::IsInTerritory(FVector Location) const
{
    return FVector::Dist(Location, TerritoryCenter) <= DinoStats.TerritoryRadius;
}

void ADinosaurBase::SetTerritoryCenter(FVector Center)
{
    TerritoryCenter = Center;
    TerritorySphere->SetWorldLocation(Center);
}

// ─────────────────────────────────────────────────────────────────────────────
// Patrol
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurBase::AddPatrolPoint(FVector Location, float WaitTime)
{
    FDinoPatrolPoint Point;
    Point.Location = Location;
    Point.WaitTime = WaitTime;
    PatrolPoints.Add(Point);
}

void ADinosaurBase::StartPatrol()
{
    if (PatrolPoints.Num() == 0) return;
    bIsPatrolling = true;
    CurrentPatrolIndex = 0;
    MoveToNextPatrolPoint();
}

void ADinosaurBase::StopPatrol()
{
    bIsPatrolling = false;
    GetWorldTimerManager().ClearTimer(PatrolWaitTimer);
}

void ADinosaurBase::TickPatrol(float DeltaTime)
{
    if (PatrolPoints.Num() == 0 || !bIsPatrolling) return;

    FVector TargetLocation = PatrolPoints[CurrentPatrolIndex].Location;
    FVector CurrentLocation = GetActorLocation();
    float DistToTarget = FVector::Dist2D(CurrentLocation, TargetLocation);

    if (DistToTarget < 100.f)
    {
        // Reached patrol point — wait then move to next
        StopPatrol();
        float WaitTime = PatrolPoints[CurrentPatrolIndex].WaitTime;
        GetWorldTimerManager().SetTimer(
            PatrolWaitTimer,
            this,
            &ADinosaurBase::OnPatrolWaitComplete,
            WaitTime,
            false
        );
    }
    else
    {
        // Move toward patrol point
        FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
        AddMovementInput(Direction, 0.7f);
    }
}

void ADinosaurBase::MoveToNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0) return;
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    bIsPatrolling = true;
}

void ADinosaurBase::OnPatrolWaitComplete()
{
    MoveToNextPatrolPoint();
}

// ─────────────────────────────────────────────────────────────────────────────
// Survival
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurBase::DrainHunger()
{
    if (!bIsAlive) return;

    DinoStats.Hunger = FMath::Clamp(DinoStats.Hunger - 2.f, 0.f, 100.f);

    if (DinoStats.Hunger <= 0.f && !bIsHerbivore)
    {
        // Carnivores become aggressive when starving
        if (AggressionState == EDinoAggressionState::Passive)
        {
            SetAggressionState(EDinoAggressionState::Alerted);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Death
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurBase::Die()
{
    if (!bIsAlive) return;

    bIsAlive = false;
    SetAggressionState(EDinoAggressionState::Passive);

    // Stop all timers
    GetWorldTimerManager().ClearTimer(HungerTimer);
    GetWorldTimerManager().ClearTimer(PatrolWaitTimer);
    GetWorldTimerManager().ClearTimer(AttackCooldownTimer);

    // Enable ragdoll physics
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Disable movement
    GetCharacterMovement()->DisableMovement();

    // Destroy after 30 seconds (cleanup)
    SetLifeSpan(30.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Stat Getters
// ─────────────────────────────────────────────────────────────────────────────

float ADinosaurBase::GetHealthPercent() const
{
    if (DinoStats.MaxHealth <= 0.f) return 0.f;
    return DinoStats.Health / DinoStats.MaxHealth;
}

// ─────────────────────────────────────────────────────────────────────────────
// Detection Overlap Callbacks
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurBase::OnDetectionSphereOverlap(UPrimitiveComponent* OverlappedComp,
                                               AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp,
                                               int32 OtherBodyIndex,
                                               bool bFromSweep,
                                               const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this || !bIsAlive) return;

    // Check if the overlapping actor is a player (ACharacter that isn't a dinosaur)
    if (OtherActor->IsA(ACharacter::StaticClass()) && !OtherActor->IsA(ADinosaurBase::StaticClass()))
    {
        AlertToThreat(OtherActor);
    }
}

void ADinosaurBase::OnDetectionSphereEndOverlap(UPrimitiveComponent* OverlappedComp,
                                                  AActor* OtherActor,
                                                  UPrimitiveComponent* OtherComp,
                                                  int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    if (OtherActor == CurrentTarget)
    {
        // Target left detection range — return to passive after a delay
        CurrentTarget = nullptr;

        FTimerHandle ReturnTimer;
        GetWorldTimerManager().SetTimer(
            ReturnTimer,
            [this]()
            {
                if (!CurrentTarget && bIsAlive)
                {
                    SetAggressionState(EDinoAggressionState::Passive);
                    if (PatrolPoints.Num() > 0)
                    {
                        StartPatrol();
                    }
                }
            },
            5.f,
            false
        );
    }
}
