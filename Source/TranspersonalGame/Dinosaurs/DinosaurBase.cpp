// DinosaurBase.cpp
// Base implementation for all dinosaur pawns in Transpersonal Game Studio
// Handles survival stats, behavior state machine, detection, and combat

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ============================================================
// Constructor
// ============================================================
ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default capsule — subclasses override for their species size
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);

    // Movement defaults — subclasses tune these per species
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = DinoStats.MoveSpeed;
        MoveComp->JumpZVelocity = 400.0f;
        MoveComp->GravityScale = 1.5f;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    }

    // No controller rotation yaw — let movement component handle it
    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
}

// ============================================================
// BeginPlay
// ============================================================
void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Sync movement speed from stats
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = DinoStats.MoveSpeed;
    }

    // Start in Idle state
    SetBehaviorState(EEng_DinoBehaviorState::Idle);

    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s spawned — Species: %s, Health: %.0f, Carnivore: %s"),
        *GetName(),
        *SpeciesDisplayName,
        DinoStats.CurrentHealth,
        bIsCarnivore ? TEXT("Yes") : TEXT("No"));
}

// ============================================================
// Tick
// ============================================================
void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsAlive()) return;

    UpdateHunger(DeltaTime);

    AIUpdateAccumulator += DeltaTime;
    if (AIUpdateAccumulator >= AIUpdateInterval)
    {
        AIUpdateAccumulator = 0.0f;
        UpdateBehaviorAI(AIUpdateAccumulator);
        DetectNearbyThreats();
    }
}

// ============================================================
// TakeDinoHit — apply damage from a source
// ============================================================
void ADinosaurBase::TakeDinoHit(float Damage, AActor* DamageSource)
{
    if (!IsAlive()) return;

    DinoStats.CurrentHealth = FMath::Max(0.0f, DinoStats.CurrentHealth - Damage);

    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s hit for %.1f damage — Health: %.1f/%.1f"),
        *GetName(), Damage, DinoStats.CurrentHealth, DinoStats.MaxHealth);

    if (DamageSource && BehaviorState != EEng_DinoBehaviorState::Attacking)
    {
        CurrentTarget = DamageSource;
        SetBehaviorState(bIsCarnivore ? EEng_DinoBehaviorState::Attacking : EEng_DinoBehaviorState::Fleeing);
    }

    if (!IsAlive())
    {
        OnDinoDeath();
    }
}

// ============================================================
// PerformAttack — deal damage to a target actor
// ============================================================
void ADinosaurBase::PerformAttack(AActor* Target)
{
    if (!Target || !IsAlive()) return;

    float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistToTarget > DinoStats.AttackRange)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DinosaurBase] %s attack failed — target out of range (%.0f > %.0f)"),
            *GetName(), DistToTarget, DinoStats.AttackRange);
        return;
    }

    // Apply damage via UE5 damage system
    UGameplayStatics::ApplyDamage(Target, DinoStats.AttackDamage, GetController(), this, nullptr);

    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s attacked %s for %.1f damage"),
        *GetName(), *Target->GetName(), DinoStats.AttackDamage);
}

// ============================================================
// SetBehaviorState — transition to a new AI state
// ============================================================
void ADinosaurBase::SetBehaviorState(EEng_DinoBehaviorState NewState)
{
    if (BehaviorState == NewState) return;

    EEng_DinoBehaviorState OldState = BehaviorState;
    BehaviorState = NewState;

    // Adjust movement speed based on state
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        switch (NewState)
        {
        case EEng_DinoBehaviorState::Hunting:
        case EEng_DinoBehaviorState::Attacking:
        case EEng_DinoBehaviorState::Fleeing:
            MoveComp->MaxWalkSpeed = DinoStats.SprintSpeed;
            break;
        case EEng_DinoBehaviorState::Resting:
            MoveComp->MaxWalkSpeed = 0.0f;
            break;
        default:
            MoveComp->MaxWalkSpeed = DinoStats.MoveSpeed;
            break;
        }
    }

    UE_LOG(LogTemp, Verbose, TEXT("[DinosaurBase] %s behavior: %d -> %d"),
        *GetName(), (int32)OldState, (int32)NewState);
}

// ============================================================
// DetectNearbyThreats — sphere overlap to find player/prey
// ============================================================
void ADinosaurBase::DetectNearbyThreats()
{
    if (!IsAlive()) return;

    UWorld* World = GetWorld();
    if (!World) return;

    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(DinoStats.DetectionRadius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = World->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ECollisionChannel::ECC_Pawn,
        Sphere,
        Params
    );

    if (!bHit) return;

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* OtherActor = Overlap.GetActor();
        if (!OtherActor) continue;

        // Carnivores hunt players and non-dino characters
        if (bIsCarnivore && OtherActor->ActorHasTag(FName("Player")))
        {
            if (CurrentTarget != OtherActor)
            {
                CurrentTarget = OtherActor;
                OnDinoSpotTarget(OtherActor);
                SetBehaviorState(EEng_DinoBehaviorState::Hunting);
            }
            break;
        }
    }
}

// ============================================================
// IsAlive — health check
// ============================================================
bool ADinosaurBase::IsAlive() const
{
    return DinoStats.CurrentHealth > 0.0f;
}

// ============================================================
// GetHealthPercent — normalized 0-1 health
// ============================================================
float ADinosaurBase::GetHealthPercent() const
{
    if (DinoStats.MaxHealth <= 0.0f) return 0.0f;
    return DinoStats.CurrentHealth / DinoStats.MaxHealth;
}

// ============================================================
// HealDino — restore health up to max
// ============================================================
void ADinosaurBase::HealDino(float Amount)
{
    DinoStats.CurrentHealth = FMath::Min(DinoStats.MaxHealth, DinoStats.CurrentHealth + Amount);
}

// ============================================================
// UpdateHunger — decay hunger over time
// ============================================================
void ADinosaurBase::UpdateHunger(float DeltaTime)
{
    DinoStats.CurrentHunger = FMath::Max(0.0f, DinoStats.CurrentHunger - (HungerDecayRate * DeltaTime));

    // Starving dinosaur becomes more aggressive
    if (DinoStats.CurrentHunger <= 10.0f && BehaviorState == EEng_DinoBehaviorState::Idle)
    {
        SetBehaviorState(EEng_DinoBehaviorState::Foraging);
    }
}

// ============================================================
// UpdateBehaviorAI — simple state machine tick
// ============================================================
void ADinosaurBase::UpdateBehaviorAI(float DeltaTime)
{
    switch (BehaviorState)
    {
    case EEng_DinoBehaviorState::Hunting:
        if (CurrentTarget && IsValid(CurrentTarget))
        {
            float Dist = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
            if (Dist <= DinoStats.AttackRange)
            {
                SetBehaviorState(EEng_DinoBehaviorState::Attacking);
                PerformAttack(CurrentTarget);
            }
        }
        else
        {
            CurrentTarget = nullptr;
            SetBehaviorState(EEng_DinoBehaviorState::Idle);
        }
        break;

    case EEng_DinoBehaviorState::Attacking:
        if (CurrentTarget && IsValid(CurrentTarget))
        {
            PerformAttack(CurrentTarget);
        }
        else
        {
            CurrentTarget = nullptr;
            SetBehaviorState(EEng_DinoBehaviorState::Idle);
        }
        break;

    default:
        break;
    }
}

// ============================================================
// BlueprintNativeEvent implementations
// ============================================================
void ADinosaurBase::OnDinoSpotTarget_Implementation(AActor* SpottedTarget)
{
    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s spotted target: %s"),
        *GetName(), SpottedTarget ? *SpottedTarget->GetName() : TEXT("NULL"));
}

void ADinosaurBase::OnDinoDeath_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s has died"), *GetName());
    SetBehaviorState(EEng_DinoBehaviorState::Dead);

    // Disable collision and movement on death
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->DisableMovement();
    }
}
