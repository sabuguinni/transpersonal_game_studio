// DinosaurBase.cpp
// Base implementation for all dinosaur actors in Transpersonal Game Studio
// Inherits ACharacter for movement, capsule collision, NavMesh integration

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default capsule size — subclasses override for species-specific sizing
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);

    // Movement defaults — subclasses override via InitializeStats()
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = 300.0f;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        MoveComp->GravityScale = 1.0f;
        MoveComp->JumpZVelocity = 0.0f; // Most dinos don't jump
    }

    // Don't use controller rotation — movement component handles orientation
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Record home location for territory logic
    HomeLocation = GetActorLocation();

    // Apply species-specific stats
    InitializeStats();

    // Apply movement speed from stats
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = DinoStats.WalkSpeed;
    }

    // Start in idle state
    SetBehaviorState(EEng_DinoBehaviorState::Idle);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsAlive())
    {
        return;
    }

    // Periodic target scanning
    TimeSinceLastScan += DeltaTime;
    if (TimeSinceLastScan >= ScanInterval)
    {
        TimeSinceLastScan = 0.0f;
        ScanForTargets();
    }

    // Update behavior logic
    UpdateBehavior(DeltaTime);
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurBase::ApplyDamage(float Amount, AActor* Causer)
{
    if (!IsAlive() || Amount <= 0.0f)
    {
        return;
    }

    DinoStats.CurrentHealth = FMath::Max(0.0f, DinoStats.CurrentHealth - Amount);
    OnTakeDamage(Amount, Causer);

    if (DinoStats.CurrentHealth <= 0.0f)
    {
        OnDeath();
    }
    else if (Causer && BehaviorState != EEng_DinoBehaviorState::Attacking)
    {
        // Attacked — switch to hunting/fleeing depending on species traits
        if (DinoStats.bIsCarnivore)
        {
            CurrentTarget = Causer;
            SetBehaviorState(EEng_DinoBehaviorState::Hunting);
        }
        else
        {
            SetBehaviorState(EEng_DinoBehaviorState::Fleeing);
        }
    }
}

void ADinosaurBase::SetBehaviorState(EEng_DinoBehaviorState NewState)
{
    if (BehaviorState == NewState)
    {
        return;
    }

    BehaviorState = NewState;

    // Adjust movement speed based on state
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        switch (NewState)
        {
        case EEng_DinoBehaviorState::Hunting:
        case EEng_DinoBehaviorState::Fleeing:
        case EEng_DinoBehaviorState::Attacking:
            MoveComp->MaxWalkSpeed = DinoStats.RunSpeed;
            break;
        default:
            MoveComp->MaxWalkSpeed = DinoStats.WalkSpeed;
            break;
        }
    }
}

float ADinosaurBase::GetHealthPercent() const
{
    if (DinoStats.MaxHealth <= 0.0f)
    {
        return 0.0f;
    }
    return DinoStats.CurrentHealth / DinoStats.MaxHealth;
}

// ─────────────────────────────────────────────────────────────────────────────
// Blueprint Native Events
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurBase::OnTakeDamage_Implementation(float DamageAmount, AActor* DamageCauser)
{
    // Base implementation — Blueprint subclasses can override for VFX/SFX
    UE_LOG(LogTemp, Log, TEXT("ADinosaurBase::OnTakeDamage — %s took %.1f damage. HP: %.1f/%.1f"),
        *GetName(), DamageAmount, DinoStats.CurrentHealth, DinoStats.MaxHealth);
}

void ADinosaurBase::OnDeath_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("ADinosaurBase::OnDeath — %s has died."), *GetName());
    SetBehaviorState(EEng_DinoBehaviorState::Dead);

    // Disable movement and collision on death
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->DisableMovement();
    }
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ADinosaurBase::OnTargetDetected_Implementation(AActor* Target)
{
    UE_LOG(LogTemp, Log, TEXT("ADinosaurBase::OnTargetDetected — %s detected %s"),
        *GetName(), Target ? *Target->GetName() : TEXT("NULL"));
}

// ─────────────────────────────────────────────────────────────────────────────
// Protected Internal Methods
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurBase::InitializeStats()
{
    // Base implementation sets sensible defaults.
    // Concrete subclasses (ATRex, ARaptor, etc.) override this to set species stats.
    DinoStats.CurrentHealth = DinoStats.MaxHealth;
}

void ADinosaurBase::UpdateBehavior(float DeltaTime)
{
    // Base behavior tick — subclasses and Behavior Trees extend this.
    // For now: if target is gone, return to idle.
    if (BehaviorState == EEng_DinoBehaviorState::Hunting ||
        BehaviorState == EEng_DinoBehaviorState::Attacking)
    {
        if (!CurrentTarget || !IsValid(CurrentTarget))
        {
            CurrentTarget = nullptr;
            SetBehaviorState(EEng_DinoBehaviorState::Idle);
        }
    }
}

void ADinosaurBase::ScanForTargets()
{
    if (!DinoStats.bIsCarnivore)
    {
        return; // Herbivores don't hunt
    }

    if (BehaviorState == EEng_DinoBehaviorState::Dead ||
        BehaviorState == EEng_DinoBehaviorState::Resting)
    {
        return;
    }

    // Sphere overlap to find player pawns within detection radius
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    const bool bHit = World->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(DinoStats.DetectionRadius),
        Params
    );

    if (bHit)
    {
        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* OverlappedActor = Overlap.GetActor();
            if (OverlappedActor && OverlappedActor->IsA<APawn>())
            {
                // Found a pawn — set as target
                if (CurrentTarget != OverlappedActor)
                {
                    CurrentTarget = OverlappedActor;
                    OnTargetDetected(OverlappedActor);
                    SetBehaviorState(EEng_DinoBehaviorState::Hunting);
                }
                return;
            }
        }
    }
}
