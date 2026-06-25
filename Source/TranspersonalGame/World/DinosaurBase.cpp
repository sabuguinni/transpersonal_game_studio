#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Configure movement for a large dinosaur
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = DinoStats.WalkSpeed;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.f, 180.f, 0.f);
        MoveComp->GravityScale = 1.5f;
        MoveComp->JumpZVelocity = 0.f; // Dinosaurs don't jump
        MoveComp->AirControl = 0.f;
    }

    // Don't use controller rotation for movement
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Store patrol origin at spawn location
    PatrolOrigin = GetActorLocation();
    CurrentPatrolTarget = PatrolOrigin;

    // Apply stats to movement component
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = DinoStats.WalkSpeed;
    }

    // Start patrolling by default
    if (CurrentState == EEng_DinoState::Idle)
    {
        SetDinoState(EEng_DinoState::Patrolling);
    }
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update AI state
    switch (CurrentState)
    {
        case EEng_DinoState::Patrolling:
            UpdatePatrol(DeltaTime);
            break;
        case EEng_DinoState::Idle:
            // Transition to patrol after a short wait
            PatrolWaitTimer += DeltaTime;
            if (PatrolWaitTimer >= PatrolWaitTime)
            {
                PatrolWaitTimer = 0.f;
                SetDinoState(EEng_DinoState::Patrolling);
            }
            break;
        default:
            break;
    }

    // Hunger decay
    DinoStats.HungerLevel = FMath::Max(0.f, DinoStats.HungerLevel - DeltaTime * 0.5f);
}

void ADinosaurBase::TakeDinoHit(float Damage, AActor* DamageSource)
{
    if (IsDead()) return;

    DinoStats.CurrentHealth = FMath::Max(0.f, DinoStats.CurrentHealth - Damage);

    if (IsDead())
    {
        SetDinoState(EEng_DinoState::Idle);
        OnDinoDeath();
    }
    else if (CurrentState != EEng_DinoState::Hunting && DamageSource)
    {
        // React to being hit — switch to hunting attacker
        SetDinoState(EEng_DinoState::Hunting);
    }
}

void ADinosaurBase::SetDinoState(EEng_DinoState NewState)
{
    if (NewState == CurrentState) return;

    EEng_DinoState OldState = CurrentState;
    CurrentState = NewState;

    // Update movement speed based on state
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        switch (NewState)
        {
            case EEng_DinoState::Hunting:
            case EEng_DinoState::Fleeing:
                MoveComp->MaxWalkSpeed = DinoStats.RunSpeed;
                break;
            default:
                MoveComp->MaxWalkSpeed = DinoStats.WalkSpeed;
                break;
        }
    }

    OnDinoStateChanged(OldState, NewState);
}

float ADinosaurBase::GetHealthPercent() const
{
    if (DinoStats.MaxHealth <= 0.f) return 0.f;
    return DinoStats.CurrentHealth / DinoStats.MaxHealth;
}

bool ADinosaurBase::IsDead() const
{
    return DinoStats.CurrentHealth <= 0.f;
}

void ADinosaurBase::StartPatrol()
{
    SetDinoState(EEng_DinoState::Patrolling);
    CurrentPatrolTarget = GetRandomPatrolPoint();
}

void ADinosaurBase::StopPatrol()
{
    SetDinoState(EEng_DinoState::Idle);
}

void ADinosaurBase::UpdatePatrol(float DeltaTime)
{
    if (bWaitingAtPatrolPoint)
    {
        PatrolWaitTimer += DeltaTime;
        if (PatrolWaitTimer >= PatrolWaitTime)
        {
            PatrolWaitTimer = 0.f;
            bWaitingAtPatrolPoint = false;
            CurrentPatrolTarget = GetRandomPatrolPoint();
        }
        return;
    }

    // Move toward patrol target
    FVector CurrentLoc = GetActorLocation();
    FVector ToTarget = CurrentPatrolTarget - CurrentLoc;
    ToTarget.Z = 0.f;
    float DistToTarget = ToTarget.Size();

    if (DistToTarget < 200.f)
    {
        // Reached patrol point — wait
        bWaitingAtPatrolPoint = true;
        PatrolWaitTimer = 0.f;
        return;
    }

    // Add movement input toward target
    FVector Direction = ToTarget.GetSafeNormal();
    AddMovementInput(Direction, 1.f);
}

FVector ADinosaurBase::GetRandomPatrolPoint() const
{
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLoc;
        if (NavSys->GetRandomReachablePointInRadius(PatrolOrigin, PatrolRadius, NavLoc))
        {
            return NavLoc.Location;
        }
    }

    // Fallback: random point in radius without NavMesh
    float Angle = FMath::RandRange(0.f, 2.f * PI);
    float Dist = FMath::RandRange(200.f, PatrolRadius);
    return PatrolOrigin + FVector(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);
}

void ADinosaurBase::OnDinoStateChanged_Implementation(EEng_DinoState OldState, EEng_DinoState NewState)
{
    // Blueprint can override this for visual/audio feedback
}

void ADinosaurBase::OnDinoDeath_Implementation()
{
    // Disable collision and movement on death
    SetActorEnableCollision(false);
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->DisableMovement();
    }

    // Destroy after a delay (allow death animation to play)
    SetLifeSpan(10.f);
}
