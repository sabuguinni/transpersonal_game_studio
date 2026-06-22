#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

AEng_DinosaurBase::AEng_DinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default capsule size — overridden per species
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    // Movement defaults — overridden per species in BeginPlay
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = Stats.WalkSpeed;
        MoveComp->MaxAcceleration = 1200.0f;
        MoveComp->BrakingDecelerationWalking = 800.0f;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->bUseControllerDesiredRotation = false;
        MoveComp->GravityScale = 1.5f;
    }

    // Auto-possess AI
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEng_DinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Apply stats to movement component
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = Stats.WalkSpeed;
    }

    // Start periodic detection
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            DetectionTimerHandle,
            this,
            &AEng_DinosaurBase::DetectNearbyActors,
            DetectionInterval,
            true
        );
    }
}

void AEng_DinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Basic state-driven movement logic
    if (!IsAlive())
    {
        return;
    }

    if (BehaviorState == EEng_DinosaurBehaviorState::Hunting && CurrentTarget)
    {
        // Move toward target
        FVector TargetLoc = CurrentTarget->GetActorLocation();
        FVector MyLoc = GetActorLocation();
        FVector Direction = (TargetLoc - MyLoc).GetSafeNormal();
        AddMovementInput(Direction, 1.0f);

        // Switch to attack if in range
        float DistSq = FVector::DistSquared(MyLoc, TargetLoc);
        if (DistSq <= FMath::Square(Stats.AttackRange))
        {
            SetBehaviorState(EEng_DinosaurBehaviorState::Attacking);
            Attack(CurrentTarget);
        }
    }
    else if (BehaviorState == EEng_DinosaurBehaviorState::Fleeing && CurrentTarget)
    {
        // Move away from threat
        FVector ThreatLoc = CurrentTarget->GetActorLocation();
        FVector MyLoc = GetActorLocation();
        FVector Direction = (MyLoc - ThreatLoc).GetSafeNormal();
        AddMovementInput(Direction, 1.0f);
    }
}

float AEng_DinosaurBase::TakeDamageFromAttack(float DamageAmount, AActor* DamageInstigator)
{
    if (!IsAlive())
    {
        return 0.0f;
    }

    float ActualDamage = FMath::Max(0.0f, DamageAmount);
    Stats.CurrentHealth = FMath::Max(0.0f, Stats.CurrentHealth - ActualDamage);

    // React to damage — set target and hunt if carnivore
    if (DamageInstigator && !Stats.bIsHerbivore)
    {
        CurrentTarget = DamageInstigator;
        SetBehaviorState(EEng_DinosaurBehaviorState::Hunting);

        // Increase speed when hunting
        UCharacterMovementComponent* MoveComp = GetCharacterMovement();
        if (MoveComp)
        {
            MoveComp->MaxWalkSpeed = Stats.RunSpeed;
        }
    }
    else if (DamageInstigator && Stats.bIsHerbivore)
    {
        // Herbivores flee
        CurrentTarget = DamageInstigator;
        SetBehaviorState(EEng_DinosaurBehaviorState::Fleeing);

        UCharacterMovementComponent* MoveComp = GetCharacterMovement();
        if (MoveComp)
        {
            MoveComp->MaxWalkSpeed = Stats.RunSpeed;
        }
    }

    if (!IsAlive())
    {
        Die();
    }

    return ActualDamage;
}

void AEng_DinosaurBase::Attack(AActor* Target)
{
    if (!Target || !IsAlive())
    {
        return;
    }

    // Apply damage to target — target must implement TakeDamage
    Target->TakeDamage(Stats.AttackDamage, FDamageEvent(), GetController(), this);

    // Return to hunting after attack
    SetBehaviorState(EEng_DinosaurBehaviorState::Hunting);
}

void AEng_DinosaurBase::SetBehaviorState(EEng_DinosaurBehaviorState NewState)
{
    if (BehaviorState == NewState)
    {
        return;
    }

    BehaviorState = NewState;

    // Reset speed to walk when idle/patrolling
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        if (NewState == EEng_DinosaurBehaviorState::Idle ||
            NewState == EEng_DinosaurBehaviorState::Patrolling ||
            NewState == EEng_DinosaurBehaviorState::Eating ||
            NewState == EEng_DinosaurBehaviorState::Sleeping)
        {
            MoveComp->MaxWalkSpeed = Stats.WalkSpeed;
        }
        else if (NewState == EEng_DinosaurBehaviorState::Hunting ||
                 NewState == EEng_DinosaurBehaviorState::Fleeing ||
                 NewState == EEng_DinosaurBehaviorState::Attacking)
        {
            MoveComp->MaxWalkSpeed = Stats.RunSpeed;
        }
    }
}

void AEng_DinosaurBase::DetectNearbyActors()
{
    if (!IsAlive() || !GetWorld())
    {
        return;
    }

    // Skip detection if already hunting/attacking
    if (BehaviorState == EEng_DinosaurBehaviorState::Hunting ||
        BehaviorState == EEng_DinosaurBehaviorState::Attacking)
    {
        return;
    }

    // Sphere overlap to find nearby actors
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(Stats.DetectionRadius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bFound = GetWorld()->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        Sphere,
        Params
    );

    if (!bFound)
    {
        return;
    }

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* FoundActor = Overlap.GetActor();
        if (!FoundActor)
        {
            continue;
        }

        // Check if it's a player character (ACharacter subclass that isn't a dinosaur)
        if (FoundActor->IsA<AEng_DinosaurBase>())
        {
            continue; // Ignore other dinosaurs for now
        }

        if (FoundActor->IsA<ACharacter>())
        {
            if (!Stats.bIsHerbivore)
            {
                // Carnivores hunt players
                CurrentTarget = FoundActor;
                SetBehaviorState(EEng_DinosaurBehaviorState::Hunting);
            }
            else
            {
                // Herbivores flee from players
                CurrentTarget = FoundActor;
                SetBehaviorState(EEng_DinosaurBehaviorState::Fleeing);
            }
            return;
        }
    }

    // No threat found — return to patrolling
    if (BehaviorState == EEng_DinosaurBehaviorState::Fleeing ||
        BehaviorState == EEng_DinosaurBehaviorState::Hunting)
    {
        CurrentTarget = nullptr;
        SetBehaviorState(EEng_DinosaurBehaviorState::Patrolling);
    }
}

bool AEng_DinosaurBase::IsAlive() const
{
    return Stats.CurrentHealth > 0.0f;
}

float AEng_DinosaurBase::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.0f)
    {
        return 0.0f;
    }
    return Stats.CurrentHealth / Stats.MaxHealth;
}

void AEng_DinosaurBase::RestoreHealth(float Amount)
{
    Stats.CurrentHealth = FMath::Min(Stats.MaxHealth, Stats.CurrentHealth + Amount);
}

void AEng_DinosaurBase::Die()
{
    SetBehaviorState(EEng_DinosaurBehaviorState::Dead);

    // Stop detection timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(DetectionTimerHandle);
    }

    // Disable movement
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->DisableMovement();
    }

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Ragdoll the mesh
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (MeshComp)
    {
        MeshComp->SetSimulatePhysics(true);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}
