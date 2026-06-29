#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Configure capsule for a medium-sized dinosaur (overridden by subclasses)
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);

    // Movement defaults — subclasses override for their species
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = Stats.WalkSpeed;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        MoveComp->GravityScale = 1.0f;
        MoveComp->Mass = Stats.Mass;
    }

    // Don't use controller rotation — movement component handles orientation
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Initialize health to max
    Stats.CurrentHealth = Stats.MaxHealth;
    Stats.CurrentHunger = 100.0f;
    bIsAlive = true;

    // Apply walk speed from stats
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = Stats.WalkSpeed;
        MoveComp->Mass = Stats.Mass;
    }

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase BeginPlay: %s (Species=%d, Health=%.0f)"),
        *GetActorLabel(), (int32)Species, Stats.MaxHealth);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsAlive)
    {
        return;
    }

    // Hunger decay — runs every 5 seconds
    HungerTickAccumulator += DeltaTime;
    if (HungerTickAccumulator >= 5.0f)
    {
        HungerTickAccumulator = 0.0f;
        UpdateHunger(5.0f);
    }

    // Player detection — runs every 1 second
    DetectionTickAccumulator += DeltaTime;
    if (DetectionTickAccumulator >= 1.0f)
    {
        DetectionTickAccumulator = 0.0f;
        CheckForNearbyPlayers();
    }
}

float ADinosaurBase::TakeDamageFromPlayer(float DamageAmount, AActor* DamageCauser)
{
    if (!bIsAlive || DamageAmount <= 0.0f)
    {
        return 0.0f;
    }

    Stats.CurrentHealth = FMath::Max(0.0f, Stats.CurrentHealth - DamageAmount);

    UE_LOG(LogTemp, Log, TEXT("%s took %.1f damage — Health: %.1f/%.1f"),
        *GetActorLabel(), DamageAmount, Stats.CurrentHealth, Stats.MaxHealth);

    // React to being hit — become aggressive
    if (BehaviorState != EEng_DinoBehaviorState::Aggressive && DamageCauser)
    {
        CurrentTarget = DamageCauser;
        SetBehaviorState(EEng_DinoBehaviorState::Aggressive);
    }

    if (Stats.CurrentHealth <= 0.0f)
    {
        Die();
    }

    return DamageAmount;
}

void ADinosaurBase::AttackTarget(AActor* Target)
{
    if (!bIsAlive || !Target)
    {
        return;
    }

    float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistToTarget <= Stats.AttackRange)
    {
        // Apply damage to target
        UGameplayStatics::ApplyDamage(Target, Stats.AttackDamage, GetController(), this, nullptr);
        UE_LOG(LogTemp, Log, TEXT("%s attacked %s for %.1f damage"),
            *GetActorLabel(), *Target->GetActorLabel(), Stats.AttackDamage);
    }
}

void ADinosaurBase::SetBehaviorState(EEng_DinoBehaviorState NewState)
{
    if (BehaviorState == NewState)
    {
        return;
    }

    EEng_DinoBehaviorState OldState = BehaviorState;
    BehaviorState = NewState;

    // Adjust movement speed based on state
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        switch (NewState)
        {
        case EEng_DinoBehaviorState::Hunting:
        case EEng_DinoBehaviorState::Aggressive:
            MoveComp->MaxWalkSpeed = Stats.RunSpeed;
            break;
        case EEng_DinoBehaviorState::Fleeing:
            MoveComp->MaxWalkSpeed = Stats.RunSpeed * 1.2f;
            break;
        case EEng_DinoBehaviorState::Sleeping:
        case EEng_DinoBehaviorState::Eating:
            MoveComp->MaxWalkSpeed = 0.0f;
            break;
        default:
            MoveComp->MaxWalkSpeed = Stats.WalkSpeed;
            break;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("%s behavior: %d -> %d"),
        *GetActorLabel(), (int32)OldState, (int32)NewState);
}

void ADinosaurBase::OnPlayerDetected(AActor* Player)
{
    if (!bIsAlive || !Player)
    {
        return;
    }

    CurrentTarget = Player;

    // Carnivores hunt; herbivores flee
    if (Diet == EEng_DinoDiet::Carnivore)
    {
        SetBehaviorState(EEng_DinoBehaviorState::Hunting);
    }
    else
    {
        SetBehaviorState(EEng_DinoBehaviorState::Fleeing);
    }
}

float ADinosaurBase::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.0f)
    {
        return 0.0f;
    }
    return Stats.CurrentHealth / Stats.MaxHealth;
}

void ADinosaurBase::HealDinosaur(float HealAmount)
{
    if (!bIsAlive || HealAmount <= 0.0f)
    {
        return;
    }

    Stats.CurrentHealth = FMath::Min(Stats.MaxHealth, Stats.CurrentHealth + HealAmount);
    UE_LOG(LogTemp, Log, TEXT("%s healed %.1f — Health: %.1f/%.1f"),
        *GetActorLabel(), HealAmount, Stats.CurrentHealth, Stats.MaxHealth);
}

void ADinosaurBase::UpdateHunger(float DeltaTime)
{
    Stats.CurrentHunger = FMath::Max(0.0f,
        Stats.CurrentHunger - (Stats.HungerDecayRate * DeltaTime));

    // Starving causes health loss
    if (Stats.CurrentHunger <= 0.0f)
    {
        Stats.CurrentHealth = FMath::Max(0.0f, Stats.CurrentHealth - 5.0f);
        if (Stats.CurrentHealth <= 0.0f)
        {
            Die();
        }
    }
}

void ADinosaurBase::CheckForNearbyPlayers()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find all player pawns within detection radius
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        // Skip self and non-player-controlled pawns
        APawn* Pawn = Cast<APawn>(Actor);
        if (!Pawn || !Pawn->IsPlayerControlled())
        {
            continue;
        }

        float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Distance <= Stats.DetectionRadius)
        {
            OnPlayerDetected(Actor);
            return; // React to first detected player
        }
    }

    // No player nearby — return to idle/patrol if we were hunting
    if (BehaviorState == EEng_DinoBehaviorState::Hunting && !CurrentTarget)
    {
        SetBehaviorState(EEng_DinoBehaviorState::Patrolling);
    }
}

void ADinosaurBase::Die()
{
    if (!bIsAlive)
    {
        return;
    }

    bIsAlive = false;
    Stats.CurrentHealth = 0.0f;
    BehaviorState = EEng_DinoBehaviorState::Idle;

    // Stop movement
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->DisableMovement();
    }

    UE_LOG(LogTemp, Log, TEXT("%s has died"), *GetActorLabel());

    // Ragdoll — enable physics on mesh
    USkeletalMeshComponent* SkelMesh = GetMesh();
    if (SkelMesh)
    {
        SkelMesh->SetSimulatePhysics(true);
        SkelMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}
