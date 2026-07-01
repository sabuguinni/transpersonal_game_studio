// DinosaurBase.cpp
// Transpersonal Game Studio — Engine Architect #02
// Full implementation of the dinosaur base class for all species

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz tick for AI efficiency

    // Capsule defaults — overridden per species in child classes
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // Skeletal mesh setup
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -120.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    // Movement component defaults
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        MoveComp->MaxWalkSpeed = 400.0f;
        MoveComp->MaxWalkSpeedCrouched = 200.0f;
        MoveComp->JumpZVelocity = 500.0f;
        MoveComp->GravityScale = 1.0f;
        MoveComp->bCanWalkOffLedges = true;
        MoveComp->NavAgentProps.bCanJump = false;
        MoveComp->NavAgentProps.bCanCrouch = false;
    }

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // AI Perception stimuli source
    PerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("PerceptionStimuliSource"));
    if (PerceptionStimuliSource)
    {
        PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense_Sight>());
        PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense_Hearing>());
        PerceptionStimuliSource->bAutoRegister = true;
    }

    // Default species stats (Raptor-sized defaults)
    MaxHealth = 100.0f;
    CurrentHealth = 100.0f;
    MaxStamina = 100.0f;
    CurrentStamina = 100.0f;
    AttackDamage = 25.0f;
    AttackRange = 150.0f;
    DetectionRadius = 1500.0f;
    ChaseRadius = 2500.0f;
    PatrolRadius = 800.0f;
    Species = EDinoSpecies::Raptor;
    BehaviorState = EDinoBehaviorState::Idle;
    DietType = EDinoDietType::Carnivore;
    bIsAlpha = false;
    bIsInPack = false;
    PackSize = 1;
    bIsDead = false;
    bIsAttacking = false;
    bIsAggressive = false;
    TerritoryCenter = FVector::ZeroVector;
    LastKnownPlayerLocation = FVector::ZeroVector;
    CurrentTarget = nullptr;
    HungerLevel = 50.0f;
    MaxHunger = 100.0f;
    HungerDecayRate = 2.0f; // per minute
    FearLevel = 0.0f;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Set territory center to spawn location
    TerritoryCenter = GetActorLocation();
    CurrentHealth = MaxHealth;
    CurrentStamina = MaxStamina;

    // Start hunger decay timer
    GetWorldTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ADinosaurBase::UpdateHunger,
        60.0f, // every 60 seconds
        true
    );

    // Start behavior update timer
    GetWorldTimerManager().SetTimer(
        BehaviorUpdateHandle,
        this,
        &ADinosaurBase::UpdateBehaviorState,
        2.0f, // every 2 seconds
        true
    );
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Stamina regeneration when not sprinting
    if (CurrentStamina < MaxStamina && !bIsAttacking)
    {
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + (DeltaTime * 10.0f));
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead)
        return 0.0f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

    OnDamageTaken(ActualDamage, DamageCauser);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
    else
    {
        // Become aggressive when damaged
        bIsAggressive = true;
        if (DamageCauser)
        {
            CurrentTarget = DamageCauser;
            SetBehaviorState(EDinoBehaviorState::Attacking);
        }
    }

    return ActualDamage;
}

void ADinosaurBase::SetBehaviorState(EDinoBehaviorState NewState)
{
    if (BehaviorState == NewState)
        return;

    EDinoBehaviorState OldState = BehaviorState;
    BehaviorState = NewState;

    OnBehaviorStateChanged(OldState, NewState);
}

void ADinosaurBase::PerformAttack()
{
    if (bIsDead || bIsAttacking)
        return;

    if (CurrentStamina < 20.0f)
        return; // Not enough stamina

    bIsAttacking = true;
    CurrentStamina -= 20.0f;

    // Deal damage to target if in range
    if (CurrentTarget)
    {
        float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
        if (DistToTarget <= AttackRange)
        {
            FDamageEvent DmgEvent;
            CurrentTarget->TakeDamage(AttackDamage, DmgEvent, GetController(), this);
        }
    }

    // Reset attack flag after animation would complete
    FTimerHandle AttackResetHandle;
    GetWorldTimerManager().SetTimer(AttackResetHandle, [this]()
    {
        bIsAttacking = false;
    }, 1.5f, false);
}

void ADinosaurBase::Roar()
{
    // Alert nearby pack members
    if (bIsInPack)
    {
        TArray<AActor*> NearbyActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADinosaurBase::StaticClass(), NearbyActors);

        for (AActor* Actor : NearbyActors)
        {
            ADinosaurBase* PackMember = Cast<ADinosaurBase>(Actor);
            if (PackMember && PackMember != this && PackMember->Species == Species)
            {
                float Dist = FVector::Dist(GetActorLocation(), PackMember->GetActorLocation());
                if (Dist <= 3000.0f) // Roar radius
                {
                    PackMember->OnHearRoar(this);
                }
            }
        }
    }

    OnRoar();
}

void ADinosaurBase::Die()
{
    if (bIsDead)
        return;

    bIsDead = true;
    bIsAttacking = false;
    SetBehaviorState(EDinoBehaviorState::Dead);

    // Stop all timers
    GetWorldTimerManager().ClearTimer(HungerTimerHandle);
    GetWorldTimerManager().ClearTimer(BehaviorUpdateHandle);

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    OnDeath();

    // Destroy after corpse timer (60 seconds)
    FTimerHandle DestroyHandle;
    GetWorldTimerManager().SetTimer(DestroyHandle, [this]()
    {
        Destroy();
    }, 60.0f, false);
}

void ADinosaurBase::UpdateHunger()
{
    HungerLevel = FMath::Max(0.0f, HungerLevel - HungerDecayRate);

    if (HungerLevel < 20.0f && DietType == EDinoDietType::Carnivore)
    {
        // Hungry carnivore becomes more aggressive
        bIsAggressive = true;
        DetectionRadius *= 1.5f; // Expanded search radius when hungry
    }
}

void ADinosaurBase::UpdateBehaviorState()
{
    if (bIsDead)
        return;

    // Find player
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
        return;

    float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

    switch (BehaviorState)
    {
        case EDinoBehaviorState::Idle:
        case EDinoBehaviorState::Wandering:
            if (DietType == EDinoDietType::Carnivore || bIsAggressive)
            {
                if (DistToPlayer <= DetectionRadius)
                {
                    LastKnownPlayerLocation = PlayerPawn->GetActorLocation();
                    CurrentTarget = PlayerPawn;
                    SetBehaviorState(EDinoBehaviorState::Hunting);
                }
            }
            break;

        case EDinoBehaviorState::Hunting:
            if (DistToPlayer <= AttackRange)
            {
                SetBehaviorState(EDinoBehaviorState::Attacking);
            }
            else if (DistToPlayer > ChaseRadius)
            {
                // Lost the target
                CurrentTarget = nullptr;
                SetBehaviorState(EDinoBehaviorState::Returning);
            }
            else
            {
                LastKnownPlayerLocation = PlayerPawn->GetActorLocation();
            }
            break;

        case EDinoBehaviorState::Attacking:
            PerformAttack();
            if (DistToPlayer > AttackRange * 2.0f)
            {
                SetBehaviorState(EDinoBehaviorState::Hunting);
            }
            break;

        case EDinoBehaviorState::Returning:
        {
            float DistToTerritory = FVector::Dist(GetActorLocation(), TerritoryCenter);
            if (DistToTerritory < 200.0f)
            {
                bIsAggressive = false;
                SetBehaviorState(EDinoBehaviorState::Idle);
            }
            break;
        }

        case EDinoBehaviorState::Fleeing:
            // Herbivores flee from player
            if (DistToPlayer > DetectionRadius * 2.0f)
            {
                SetBehaviorState(EDinoBehaviorState::Idle);
            }
            break;

        default:
            break;
    }
}

void ADinosaurBase::OnHearRoar(ADinosaurBase* Caller)
{
    if (bIsDead || !Caller)
        return;

    // Join the hunt if pack member is hunting
    if (Caller->BehaviorState == EDinoBehaviorState::Hunting ||
        Caller->BehaviorState == EDinoBehaviorState::Attacking)
    {
        CurrentTarget = Caller->CurrentTarget;
        if (CurrentTarget)
        {
            SetBehaviorState(EDinoBehaviorState::Hunting);
        }
    }
}

// Blueprint-overridable events — default implementations
void ADinosaurBase::OnDamageTaken_Implementation(float DamageAmount, AActor* DamageCauser)
{
    // Default: nothing — override in Blueprint for visual/audio feedback
}

void ADinosaurBase::OnDeath_Implementation()
{
    // Default: nothing — override in Blueprint for death animation/loot spawn
}

void ADinosaurBase::OnBehaviorStateChanged_Implementation(EDinoBehaviorState OldState, EDinoBehaviorState NewState)
{
    // Default: nothing — override in Blueprint for animation state machine
}

void ADinosaurBase::OnRoar_Implementation()
{
    // Default: nothing — override in Blueprint for roar sound/animation
}

float ADinosaurBase::GetHealthPercent() const
{
    if (MaxHealth <= 0.0f) return 0.0f;
    return CurrentHealth / MaxHealth;
}

float ADinosaurBase::GetStaminaPercent() const
{
    if (MaxStamina <= 0.0f) return 0.0f;
    return CurrentStamina / MaxStamina;
}

bool ADinosaurBase::IsHostileToPlayer() const
{
    if (bIsDead) return false;
    if (bIsAggressive) return true;
    if (DietType == EDinoDietType::Carnivore && HungerLevel < 30.0f) return true;
    return false;
}
