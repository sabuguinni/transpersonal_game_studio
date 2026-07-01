// DinosaurBase.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Base class for all dinosaur species. Implements survival AI, combat, detection.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Configure movement for dinosaur locomotion
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = DinoStats.MoveSpeed;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        MoveComp->GravityScale = 1.0f;
        MoveComp->JumpZVelocity = 0.0f; // Most dinos don't jump
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
    DinoStats.CurrentHealth = DinoStats.MaxHealth;

    // Apply movement speed from stats
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = DinoStats.MoveSpeed;
    }

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase BeginPlay: %s spawned. Health=%.0f, Diet=%d, Aggressive=%d"),
        *SpeciesName, DinoStats.CurrentHealth, (int32)Diet, (int32)bIsAggressive);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Throttle AI updates to every 0.5 seconds for performance
    AIUpdateTimer += DeltaTime;
    if (AIUpdateTimer >= AIUpdateInterval)
    {
        AIUpdateTimer = 0.0f;
        UpdateBehaviorAI(DeltaTime);
    }
}

void ADinosaurBase::UpdateBehaviorAI(float DeltaTime)
{
    if (!IsAlive())
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Scan for player within detection radius
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC)
    {
        return;
    }

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }

    float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

    if (bIsAggressive && DistToPlayer <= DinoStats.DetectionRadius)
    {
        // Player detected — switch to hunting if not already
        if (BehaviorState != EEng_DinosaurBehaviorState::Hunting)
        {
            CurrentTarget = PlayerPawn;
            SetBehaviorState(EEng_DinosaurBehaviorState::Hunting);
            OnPlayerDetected(PlayerPawn);
        }
    }
    else if (BehaviorState == EEng_DinosaurBehaviorState::Hunting && DistToPlayer > DinoStats.DetectionRadius * 1.5f)
    {
        // Player escaped detection range — return to patrol
        CurrentTarget = nullptr;
        SetBehaviorState(EEng_DinosaurBehaviorState::Patrolling);
    }
}

void ADinosaurBase::TakeDinosaurDamage(float DamageAmount, AActor* DamageCauser)
{
    if (!IsAlive())
    {
        return;
    }

    DinoStats.CurrentHealth = FMath::Max(0.0f, DinoStats.CurrentHealth - DamageAmount);

    UE_LOG(LogTemp, Log, TEXT("%s took %.1f damage. Health: %.1f/%.1f"),
        *SpeciesName, DamageAmount, DinoStats.CurrentHealth, DinoStats.MaxHealth);

    // If hit by player and not already hunting, become aggressive
    if (DamageCauser && !bIsAggressive)
    {
        bIsAggressive = true;
    }

    if (DamageCauser && BehaviorState != EEng_DinosaurBehaviorState::Hunting)
    {
        CurrentTarget = DamageCauser;
        SetBehaviorState(EEng_DinosaurBehaviorState::Hunting);
    }

    if (!IsAlive())
    {
        OnDinosaurDeath();
    }
}

bool ADinosaurBase::IsAlive() const
{
    return DinoStats.CurrentHealth > 0.0f;
}

void ADinosaurBase::SetBehaviorState(EEng_DinosaurBehaviorState NewState)
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
        case EEng_DinosaurBehaviorState::Hunting:
            MoveComp->MaxWalkSpeed = DinoStats.SprintSpeed;
            break;
        case EEng_DinosaurBehaviorState::Fleeing:
            MoveComp->MaxWalkSpeed = DinoStats.SprintSpeed * 1.2f;
            break;
        case EEng_DinosaurBehaviorState::Sleeping:
            MoveComp->MaxWalkSpeed = 0.0f;
            break;
        default:
            MoveComp->MaxWalkSpeed = DinoStats.MoveSpeed;
            break;
        }
    }

    UE_LOG(LogTemp, Verbose, TEXT("%s behavior state -> %d"), *SpeciesName, (int32)NewState);
}

float ADinosaurBase::GetDetectionRadius() const
{
    return DinoStats.DetectionRadius;
}

void ADinosaurBase::OnDinosaurDeath_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("%s has died."), *SpeciesName);

    // Disable collision and movement on death
    SetActorEnableCollision(false);
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->DisableMovement();
    }

    // Destroy actor after 5 seconds (corpse lingers briefly)
    SetLifeSpan(5.0f);
}

void ADinosaurBase::OnPlayerDetected_Implementation(AActor* DetectedPlayer)
{
    UE_LOG(LogTemp, Log, TEXT("%s detected player at distance %.0f — switching to HUNT"),
        *SpeciesName,
        DetectedPlayer ? FVector::Dist(GetActorLocation(), DetectedPlayer->GetActorLocation()) : 0.0f);
}
