#include "DinosaurBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Detection sphere for threat awareness
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(1500.0f);
    DetectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ADinosaurBase::OnDetectionSphereOverlap);

    // Default movement settings
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = Stats.MoveSpeed;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        MoveComp->GravityScale = 1.0f;
        MoveComp->MaxStepHeight = 45.0f;
        MoveComp->SetWalkableFloorAngle(50.0f);
    }

    // Don't use controller rotation — orient to movement instead
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    PatrolOrigin = GetActorLocation();
    Stats.CurrentHealth = Stats.MaxHealth;

    // Sync detection sphere radius with stats
    if (DetectionSphere)
    {
        DetectionSphere->SetSphereRadius(Stats.DetectionRadius);
    }

    // Sync movement speed with stats
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = Stats.MoveSpeed;
    }

    // Start idle patrol after a short delay
    FTimerHandle PatrolStartTimer;
    GetWorldTimerManager().SetTimer(PatrolStartTimer, this, &ADinosaurBase::StartPatrol, 2.0f, false);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update attack cooldown
    if (AttackCooldownTimer > 0.0f)
    {
        AttackCooldownTimer -= DeltaTime;
    }

    // Update behavior each tick
    if (IsAlive())
    {
        UpdateBehavior(DeltaTime);
    }
}

void ADinosaurBase::UpdateBehavior(float DeltaTime)
{
    switch (BehaviorState)
    {
        case EEng_DinosaurBehaviorState::Idle:
            // Transition to patrol after settling
            break;

        case EEng_DinosaurBehaviorState::Patrolling:
            // Patrol logic handled by AI controller / behavior tree
            break;

        case EEng_DinosaurBehaviorState::Hunting:
            if (CurrentTarget && IsValid(CurrentTarget))
            {
                float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
                if (DistToTarget <= Stats.AttackRange && AttackCooldownTimer <= 0.0f)
                {
                    PerformAttack();
                }
            }
            else
            {
                // Lost target — return to patrol
                SetBehaviorState(EEng_DinosaurBehaviorState::Patrolling);
                CurrentTarget = nullptr;
            }
            break;

        case EEng_DinosaurBehaviorState::Fleeing:
            // Flee logic — move away from threat
            break;

        case EEng_DinosaurBehaviorState::Dead:
            // No updates when dead
            break;

        default:
            break;
    }
}

float ADinosaurBase::TakeDamageFromPlayer(float DamageAmount, AActor* DamageInstigator)
{
    if (!IsAlive()) return 0.0f;

    float ActualDamage = FMath::Max(0.0f, DamageAmount);
    Stats.CurrentHealth -= ActualDamage;

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase [%s]: Took %.1f damage. Health: %.1f/%.1f"),
           *GetName(), ActualDamage, Stats.CurrentHealth, Stats.MaxHealth);

    // React to being hit — switch to hunting if carnivore
    if (DamageInstigator && Stats.bIsCarnivore)
    {
        CurrentTarget = DamageInstigator;
        SetBehaviorState(EEng_DinosaurBehaviorState::Hunting);
    }
    else if (DamageInstigator && !Stats.bIsCarnivore)
    {
        // Herbivores flee when hit
        SetBehaviorState(EEng_DinosaurBehaviorState::Fleeing);
    }

    if (Stats.CurrentHealth <= 0.0f)
    {
        OnDeath();
    }

    return ActualDamage;
}

void ADinosaurBase::SetBehaviorState(EEng_DinosaurBehaviorState NewState)
{
    if (BehaviorState == NewState) return;

    EEng_DinosaurBehaviorState OldState = BehaviorState;
    BehaviorState = NewState;

    UE_LOG(LogTemp, Verbose, TEXT("DinosaurBase [%s]: State %d -> %d"),
           *GetName(), (int32)OldState, (int32)NewState);

    // Update movement speed based on state
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        switch (NewState)
        {
            case EEng_DinosaurBehaviorState::Hunting:
            case EEng_DinosaurBehaviorState::Fleeing:
            case EEng_DinosaurBehaviorState::Attacking:
                MoveComp->MaxWalkSpeed = Stats.SprintSpeed;
                break;
            default:
                MoveComp->MaxWalkSpeed = Stats.MoveSpeed;
                break;
        }
    }
}

void ADinosaurBase::DetectThreat(AActor* Threat)
{
    if (!Threat || !IsAlive()) return;

    float DistToThreat = FVector::Dist(GetActorLocation(), Threat->GetActorLocation());
    if (DistToThreat > Stats.DetectionRadius) return;

    if (Stats.bIsCarnivore && AggressionLevel > 0.3f)
    {
        CurrentTarget = Threat;
        SetBehaviorState(EEng_DinosaurBehaviorState::Hunting);
        UE_LOG(LogTemp, Log, TEXT("DinosaurBase [%s]: Detected threat [%s] — switching to HUNTING"),
               *GetName(), *Threat->GetName());
    }
    else if (!Stats.bIsCarnivore)
    {
        SetBehaviorState(EEng_DinosaurBehaviorState::Fleeing);
    }
}

bool ADinosaurBase::IsAlive() const
{
    return Stats.CurrentHealth > 0.0f && BehaviorState != EEng_DinosaurBehaviorState::Dead;
}

float ADinosaurBase::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.0f) return 0.0f;
    return FMath::Clamp(Stats.CurrentHealth / Stats.MaxHealth, 0.0f, 1.0f);
}

void ADinosaurBase::HealDinosaur(float HealAmount)
{
    if (!IsAlive()) return;
    Stats.CurrentHealth = FMath::Min(Stats.CurrentHealth + HealAmount, Stats.MaxHealth);
    UE_LOG(LogTemp, Log, TEXT("DinosaurBase [%s]: Healed %.1f. Health: %.1f/%.1f"),
           *GetName(), HealAmount, Stats.CurrentHealth, Stats.MaxHealth);
}

void ADinosaurBase::StartPatrol()
{
    if (!IsAlive()) return;
    bIsPatrolling = true;
    SetBehaviorState(EEng_DinosaurBehaviorState::Patrolling);
}

void ADinosaurBase::StopPatrol()
{
    bIsPatrolling = false;
    SetBehaviorState(EEng_DinosaurBehaviorState::Idle);
}

void ADinosaurBase::PerformAttack()
{
    if (!CurrentTarget || !IsAlive()) return;

    AttackCooldownTimer = AttackCooldown;
    SetBehaviorState(EEng_DinosaurBehaviorState::Attacking);

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase [%s]: Attacking [%s] for %.1f damage"),
           *GetName(), *CurrentTarget->GetName(), Stats.AttackDamage);

    // Apply damage to target via UE5 damage system
    UGameplayStatics::ApplyDamage(CurrentTarget, Stats.AttackDamage, GetController(), this, nullptr);

    // Return to hunting after attack
    FTimerHandle AttackRecoveryTimer;
    GetWorldTimerManager().SetTimer(AttackRecoveryTimer, [this]()
    {
        if (IsAlive() && CurrentTarget && IsValid(CurrentTarget))
        {
            SetBehaviorState(EEng_DinosaurBehaviorState::Hunting);
        }
        else
        {
            SetBehaviorState(EEng_DinosaurBehaviorState::Patrolling);
        }
    }, 0.5f, false);
}

void ADinosaurBase::OnDeath()
{
    SetBehaviorState(EEng_DinosaurBehaviorState::Dead);
    CurrentTarget = nullptr;
    bIsPatrolling = false;

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase [%s]: DIED"), *GetName());

    // Disable collision on death
    SetActorEnableCollision(false);

    // Disable tick
    SetActorTickEnabled(false);

    // Destroy after 10 seconds (corpse lingers for player to loot)
    SetLifeSpan(10.0f);
}

void ADinosaurBase::OnDetectionSphereOverlap(UPrimitiveComponent* OverlappedComponent,
                                              AActor* OtherActor,
                                              UPrimitiveComponent* OtherComp,
                                              int32 OtherBodyIndex,
                                              bool bFromSweep,
                                              const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;
    if (!IsAlive()) return;

    // Check if the overlapping actor is a player character
    if (OtherActor->ActorHasTag(TEXT("Player")))
    {
        DetectThreat(OtherActor);
    }
}
