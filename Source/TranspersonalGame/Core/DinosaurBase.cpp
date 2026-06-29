#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Configure capsule for a large dinosaur by default (TRex scale)
    GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
    GetCapsuleComponent()->SetCapsuleRadius(55.0f);

    // Configure movement
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = DinoStats.MoveSpeed;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 180.0f, 0.0f);
        MoveComp->GravityScale = 1.0f;
        MoveComp->MaxStepHeight = 45.0f;
        MoveComp->SetWalkableFloorAngle(45.0f);
    }

    // Don't use controller rotation for movement — let movement component handle it
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Set default territory center to spawn location (updated in BeginPlay)
    TerritoryCenter = FVector::ZeroVector;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Set territory center to spawn location
    TerritoryCenter = GetActorLocation();

    // Apply movement speed from stats
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = DinoStats.MoveSpeed;
    }

    // Start in idle state
    SetBehaviorState(EEng_DinosaurBehaviorState::Idle);

    // Initialize roam target
    RoamTarget = GetRandomRoamTarget();
    bHasRoamTarget = true;
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsAlive())
    {
        return;
    }

    // Throttled state update
    StateUpdateTimer += DeltaTime;
    if (StateUpdateTimer >= StateUpdateInterval)
    {
        StateUpdateTimer = 0.0f;
        ScanForPlayer();
        UpdateBehavior(DeltaTime);
    }
}

float ADinosaurBase::TakeDamageFromPlayer(float DamageAmount, AActor* DamageInstigator)
{
    if (!IsAlive())
    {
        return 0.0f;
    }

    DinoStats.CurrentHealth = FMath::Max(0.0f, DinoStats.CurrentHealth - DamageAmount);

    // React to damage — become aggressive
    if (DinoStats.bIsCarnivore && DamageInstigator)
    {
        DetectedPlayer = DamageInstigator;
        SetBehaviorState(EEng_DinosaurBehaviorState::Attacking);
    }

    if (!IsAlive())
    {
        Die();
    }

    return DamageAmount;
}

void ADinosaurBase::AttackTarget(AActor* Target)
{
    if (!Target || !IsAlive())
    {
        return;
    }

    // Apply damage to target
    UGameplayStatics::ApplyDamage(
        Target,
        DinoStats.AttackDamage,
        GetController(),
        this,
        nullptr
    );
}

void ADinosaurBase::SetBehaviorState(EEng_DinosaurBehaviorState NewState)
{
    if (BehaviorState == NewState)
    {
        return;
    }

    BehaviorState = NewState;

    // Adjust movement speed based on state
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        switch (NewState)
        {
            case EEng_DinosaurBehaviorState::Idle:
            case EEng_DinosaurBehaviorState::Resting:
                MoveComp->MaxWalkSpeed = 0.0f;
                break;
            case EEng_DinosaurBehaviorState::Roaming:
                MoveComp->MaxWalkSpeed = DinoStats.MoveSpeed * 0.4f;
                break;
            case EEng_DinosaurBehaviorState::Hunting:
                MoveComp->MaxWalkSpeed = DinoStats.MoveSpeed * 0.7f;
                break;
            case EEng_DinosaurBehaviorState::Attacking:
                MoveComp->MaxWalkSpeed = DinoStats.MoveSpeed;
                break;
            case EEng_DinosaurBehaviorState::Fleeing:
                MoveComp->MaxWalkSpeed = DinoStats.MoveSpeed * 1.2f;
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

void ADinosaurBase::OnDetectPlayer_Implementation(AActor* Player)
{
    DetectedPlayer = Player;
    if (DinoStats.bIsCarnivore)
    {
        SetBehaviorState(EEng_DinosaurBehaviorState::Hunting);
    }
}

void ADinosaurBase::OnLosePlayer_Implementation()
{
    DetectedPlayer = nullptr;
    SetBehaviorState(EEng_DinosaurBehaviorState::Roaming);
}

void ADinosaurBase::OnDeath_Implementation()
{
    // Base death behavior — subclasses can override
    SetBehaviorState(EEng_DinosaurBehaviorState::Idle);

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Disable movement
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->DisableMovement();
    }
}

void ADinosaurBase::UpdateBehavior(float DeltaTime)
{
    switch (BehaviorState)
    {
        case EEng_DinosaurBehaviorState::Idle:
            // Occasionally start roaming
            if (FMath::RandRange(0.0f, 1.0f) < 0.1f)
            {
                SetBehaviorState(EEng_DinosaurBehaviorState::Roaming);
                RoamTarget = GetRandomRoamTarget();
                bHasRoamTarget = true;
            }
            break;

        case EEng_DinosaurBehaviorState::Roaming:
            UpdateRoaming(DeltaTime);
            break;

        case EEng_DinosaurBehaviorState::Hunting:
        case EEng_DinosaurBehaviorState::Attacking:
            UpdateHunting(DeltaTime);
            break;

        case EEng_DinosaurBehaviorState::Fleeing:
            // Move away from detected player
            if (DetectedPlayer)
            {
                FVector AwayDir = (GetActorLocation() - DetectedPlayer->GetActorLocation()).GetSafeNormal();
                AddMovementInput(AwayDir, 1.0f);
            }
            break;

        case EEng_DinosaurBehaviorState::Resting:
            // Rest for a while then go back to idle
            if (FMath::RandRange(0.0f, 1.0f) < 0.05f)
            {
                SetBehaviorState(EEng_DinosaurBehaviorState::Idle);
            }
            break;
    }
}

void ADinosaurBase::UpdateRoaming(float DeltaTime)
{
    if (!bHasRoamTarget)
    {
        RoamTarget = GetRandomRoamTarget();
        bHasRoamTarget = true;
        return;
    }

    FVector CurrentLoc = GetActorLocation();
    float DistToTarget = FVector::Dist2D(CurrentLoc, RoamTarget);

    if (DistToTarget < 200.0f)
    {
        // Reached roam target — idle briefly then get new target
        bHasRoamTarget = false;
        SetBehaviorState(EEng_DinosaurBehaviorState::Idle);
        return;
    }

    // Move toward roam target
    FVector Dir = (RoamTarget - CurrentLoc).GetSafeNormal2D();
    AddMovementInput(Dir, 1.0f);
}

void ADinosaurBase::UpdateHunting(float DeltaTime)
{
    if (!DetectedPlayer)
    {
        SetBehaviorState(EEng_DinosaurBehaviorState::Roaming);
        return;
    }

    FVector CurrentLoc = GetActorLocation();
    float DistToPlayer = FVector::Dist(CurrentLoc, DetectedPlayer->GetActorLocation());

    if (DistToPlayer <= DinoStats.AttackRadius)
    {
        // In attack range — attack
        SetBehaviorState(EEng_DinosaurBehaviorState::Attacking);
        AttackTarget(DetectedPlayer);
    }
    else if (DistToPlayer > DinoStats.DetectionRadius * 1.5f)
    {
        // Lost the player
        OnLosePlayer();
    }
    else
    {
        // Chase the player
        FVector Dir = (DetectedPlayer->GetActorLocation() - CurrentLoc).GetSafeNormal2D();
        AddMovementInput(Dir, 1.0f);
    }
}

void ADinosaurBase::ScanForPlayer()
{
    // Only carnivores actively hunt players
    if (!DinoStats.bIsCarnivore)
    {
        return;
    }

    // If already tracking, check if still in range
    if (DetectedPlayer)
    {
        float Dist = FVector::Dist(GetActorLocation(), DetectedPlayer->GetActorLocation());
        if (Dist > DinoStats.DetectionRadius * 1.5f)
        {
            OnLosePlayer();
        }
        return;
    }

    // Scan for player pawn
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        return;
    }

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }

    float Dist = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    if (Dist <= DinoStats.DetectionRadius)
    {
        OnDetectPlayer(PlayerPawn);
    }
}

FVector ADinosaurBase::GetRandomRoamTarget() const
{
    // Pick a random point within roam radius of territory center
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Radius = FMath::RandRange(500.0f, DinoStats.RoamRadius);

    FVector Offset(
        FMath::Cos(FMath::DegreesToRadians(Angle)) * Radius,
        FMath::Sin(FMath::DegreesToRadians(Angle)) * Radius,
        0.0f
    );

    return TerritoryCenter + Offset;
}

void ADinosaurBase::Die()
{
    OnDeath();
}
