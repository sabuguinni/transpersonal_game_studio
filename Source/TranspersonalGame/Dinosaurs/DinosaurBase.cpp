// DinosaurBase.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Base class for all dinosaur pawns. Implements survival-game-relevant
// behaviour: territory radius, threat detection, patrol state machine.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Capsule ──────────────────────────────────────────────────────────
    GetCapsuleComponent()->InitCapsuleSize(60.f, 120.f);

    // ── Movement defaults (overridden per species in child classes) ──────
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed        = 400.f;   // ~14 km/h idle patrol
        MoveComp->MaxSprintSpeed      = 900.f;   // sprint (set in child)
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate        = FRotator(0.f, 360.f, 0.f);
        MoveComp->GravityScale        = 1.0f;
        MoveComp->JumpZVelocity       = 0.f;     // most dinos don't jump
    }

    // ── Survival stats defaults ──────────────────────────────────────────
    MaxHealth          = 500.f;
    CurrentHealth      = MaxHealth;
    TerritoryRadius    = 2000.f;   // 20 m in UE units (1 UU = 1 cm)
    ThreatDetectRadius = 1500.f;
    PatrolSpeed        = 300.f;
    ChaseSpeed         = 800.f;
    AttackDamage       = 40.f;
    AttackCooldown     = 2.f;
    bIsAggressive      = true;
    CurrentState       = EDinoState::Patrolling;

    // ── Mesh placeholder (child classes assign real mesh) ────────────────
    // Body mesh is inherited from ACharacter's GetMesh()
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    HomeLocation = GetActorLocation();

    // Start patrol timer
    GetWorldTimerManager().SetTimer(
        PatrolTimerHandle,
        this,
        &ADinosaurBase::UpdateBehavior,
        0.5f,   // tick every 0.5 s (cheap AI)
        true
    );
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Fine-grained per-frame logic (animation blend, foot IK) goes here later
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::UpdateBehavior()
{
    if (CurrentHealth <= 0.f)
    {
        SetDinoState(EDinoState::Dead);
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    const float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

    switch (CurrentState)
    {
        case EDinoState::Patrolling:
            if (bIsAggressive && DistToPlayer < ThreatDetectRadius)
            {
                SetDinoState(EDinoState::Chasing);
            }
            else
            {
                DoPatrol();
            }
            break;

        case EDinoState::Chasing:
            if (DistToPlayer > ThreatDetectRadius * 1.5f)
            {
                // Player escaped — return home
                SetDinoState(EDinoState::Returning);
            }
            else if (DistToPlayer < 200.f)
            {
                SetDinoState(EDinoState::Attacking);
            }
            else
            {
                // Move toward player
                GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
                AddMovementInput(
                    (PlayerPawn->GetActorLocation() - GetActorLocation()).GetSafeNormal(),
                    1.f
                );
            }
            break;

        case EDinoState::Attacking:
            if (DistToPlayer > 300.f)
            {
                SetDinoState(EDinoState::Chasing);
            }
            else
            {
                TryAttackPlayer(PlayerPawn);
            }
            break;

        case EDinoState::Returning:
        {
            const float DistHome = FVector::Dist(GetActorLocation(), HomeLocation);
            if (DistHome < 200.f)
            {
                SetDinoState(EDinoState::Patrolling);
            }
            else
            {
                GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
                AddMovementInput(
                    (HomeLocation - GetActorLocation()).GetSafeNormal(),
                    1.f
                );
            }
            break;
        }

        case EDinoState::Dead:
            // Nothing — ragdoll handled by physics
            break;

        default:
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::DoPatrol()
{
    GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;

    // Simple wander: if close to current waypoint, pick a new random offset
    const float DistToWaypoint = FVector::Dist(GetActorLocation(), CurrentWaypoint);
    if (DistToWaypoint < 150.f || CurrentWaypoint.IsZero())
    {
        const float Angle  = FMath::RandRange(0.f, 360.f);
        const float Radius = FMath::RandRange(300.f, TerritoryRadius * 0.5f);
        CurrentWaypoint = HomeLocation + FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Radius,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Radius,
            0.f
        );
    }

    AddMovementInput(
        (CurrentWaypoint - GetActorLocation()).GetSafeNormal(),
        1.f
    );
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::TryAttackPlayer(APawn* PlayerPawn)
{
    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastAttackTime < AttackCooldown) return;

    LastAttackTime = Now;

    // Apply damage via UE5 damage system
    UGameplayStatics::ApplyDamage(
        PlayerPawn,
        AttackDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    UE_LOG(LogTemp, Warning, TEXT("[%s] attacked player for %.0f damage"),
        *GetActorLabel(), AttackDamage);
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::SetDinoState(EDinoState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
    OnDinoStateChanged(NewState);
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::OnDinoStateChanged(EDinoState NewState)
{
    // Override in child classes for species-specific reactions
    UE_LOG(LogTemp, Log, TEXT("[%s] state -> %d"), *GetActorLabel(), (int32)NewState);
}

// ─────────────────────────────────────────────────────────────────────────────
float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHealth = FMath::Max(0.f, CurrentHealth - ActualDamage);

    if (CurrentHealth <= 0.f)
    {
        SetDinoState(EDinoState::Dead);
        // Enable ragdoll
        if (GetMesh())
        {
            GetMesh()->SetSimulatePhysics(true);
            GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        GetWorldTimerManager().ClearTimer(PatrolTimerHandle);

        // Destroy after 30 s to free resources
        SetLifeSpan(30.f);
    }
    else if (CurrentState == EDinoState::Patrolling && bIsAggressive)
    {
        SetDinoState(EDinoState::Chasing);
    }

    return ActualDamage;
}
