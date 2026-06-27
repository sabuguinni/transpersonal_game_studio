#include "DinosaurBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Detection sphere for sensing nearby actors
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(1500.0f);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    DetectionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Configure movement
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = Stats.MoveSpeed;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        MoveComp->bUseControllerDesiredRotation = false;
        MoveComp->NavAgentProps.bCanCrouch = false;
        MoveComp->NavAgentProps.bCanJump = false;
    }

    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

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

    SetBehaviorState(EDinosaurBehaviorState::Idle);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsDead) return;

    UpdateHunger(DeltaTime);

    // Periodic detection (every 0.5s via tick accumulation)
    static float DetectionTimer = 0.0f;
    DetectionTimer += DeltaTime;
    if (DetectionTimer >= 0.5f)
    {
        DetectionTimer = 0.0f;
        DetectNearbyActors();
    }
}

float ADinosaurBase::TakeDamageFromAttack(float DamageAmount, AActor* DamageInstigator)
{
    if (bIsDead) return 0.0f;

    Stats.CurrentHealth = FMath::Max(0.0f, Stats.CurrentHealth - DamageAmount);

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase [%s] took %.1f damage. Health: %.1f/%.1f"),
        *GetActorLabel(), DamageAmount, Stats.CurrentHealth, Stats.MaxHealth);

    if (Stats.CurrentHealth <= 0.0f)
    {
        OnDeath();
    }
    else if (DamageInstigator && BehaviorState != EDinosaurBehaviorState::Attacking)
    {
        // React to being hit — switch to hunting/fleeing based on species
        if (Stats.bIsCarnivore)
        {
            CurrentTarget = DamageInstigator;
            SetBehaviorState(EDinosaurBehaviorState::Attacking);
        }
        else
        {
            SetBehaviorState(EDinosaurBehaviorState::Fleeing);
        }
    }

    return DamageAmount;
}

void ADinosaurBase::PerformAttack()
{
    if (bIsDead || !CurrentTarget) return;

    float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
    if (DistToTarget <= Stats.AttackRange)
    {
        // Apply damage to target
        UGameplayStatics::ApplyDamage(CurrentTarget, Stats.AttackDamage, GetController(), this, nullptr);

        UE_LOG(LogTemp, Log, TEXT("DinosaurBase [%s] attacked [%s] for %.1f damage"),
            *GetActorLabel(), *CurrentTarget->GetActorLabel(), Stats.AttackDamage);
    }
}

void ADinosaurBase::SetBehaviorState(EDinosaurBehaviorState NewState)
{
    if (BehaviorState == NewState) return;

    BehaviorState = NewState;

    // Adjust movement speed based on state
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        switch (NewState)
        {
        case EDinosaurBehaviorState::Hunting:
        case EDinosaurBehaviorState::Attacking:
            MoveComp->MaxWalkSpeed = Stats.SprintSpeed;
            break;
        case EDinosaurBehaviorState::Fleeing:
            MoveComp->MaxWalkSpeed = Stats.SprintSpeed * 1.2f;
            break;
        default:
            MoveComp->MaxWalkSpeed = Stats.MoveSpeed;
            break;
        }
    }

    UE_LOG(LogTemp, Verbose, TEXT("DinosaurBase [%s] behavior: %d"),
        *GetActorLabel(), (int32)NewState);
}

void ADinosaurBase::DetectNearbyActors()
{
    if (bIsDead) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Sphere overlap to find pawns in detection radius
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    bool bHit = World->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ECollisionChannel::ECC_Pawn,
        FCollisionShape::MakeSphere(Stats.DetectionRadius),
        QueryParams
    );

    if (bHit && Stats.bIsCarnivore && BehaviorState == EDinosaurBehaviorState::Idle)
    {
        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* DetectedActor = Overlap.GetActor();
            if (DetectedActor && DetectedActor->IsA(APawn::StaticClass()))
            {
                // Found a pawn — switch to hunting
                CurrentTarget = DetectedActor;
                SetBehaviorState(EDinosaurBehaviorState::Hunting);
                break;
            }
        }
    }
}

bool ADinosaurBase::IsAlive() const
{
    return !bIsDead && Stats.CurrentHealth > 0.0f;
}

float ADinosaurBase::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.0f) return 0.0f;
    return Stats.CurrentHealth / Stats.MaxHealth;
}

void ADinosaurBase::SetCurrentHealth(float NewHealth)
{
    Stats.CurrentHealth = FMath::Clamp(NewHealth, 0.0f, Stats.MaxHealth);
    if (Stats.CurrentHealth <= 0.0f)
    {
        OnDeath();
    }
}

void ADinosaurBase::OnDeath()
{
    if (bIsDead) return;
    bIsDead = true;

    SetBehaviorState(EDinosaurBehaviorState::Idle);
    CurrentTarget = nullptr;

    // Disable collision and movement
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->DisableMovement();
    }

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase [%s] has died."), *GetActorLabel());

    // Destroy after 10 seconds (corpse lingers briefly)
    SetLifeSpan(10.0f);
}

void ADinosaurBase::UpdateHunger(float DeltaTime)
{
    Stats.HungerLevel = FMath::Max(0.0f, Stats.HungerLevel - HungerDecayRate * DeltaTime);

    // When very hungry and carnivore, become more aggressive
    if (Stats.HungerLevel < 20.0f && Stats.bIsCarnivore && BehaviorState == EDinosaurBehaviorState::Idle)
    {
        SetBehaviorState(EDinosaurBehaviorState::Patrolling);
    }
}
