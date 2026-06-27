#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default capsule size (T-Rex scale — subclasses override)
    GetCapsuleComponent()->InitCapsuleSize(80.0f, 120.0f);

    // Movement defaults
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = Stats.WalkSpeed;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        MoveComp->GravityScale = 1.0f;
        MoveComp->MaxAcceleration = 1200.0f;
    }

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Store patrol origin at spawn location
    PatrolOrigin = GetActorLocation();
    ChooseNewPatrolDestination();

    // Set initial behavior
    SetBehaviorState(EEng_DinosaurBehaviorState::Patrolling);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateBehavior(DeltaTime);
}

void ADinosaurBase::UpdateBehavior(float DeltaTime)
{
    if (!IsAlive()) return;

    // Hunger decay
    Stats.Hunger = FMath::Max(0.0f, Stats.Hunger - HungerDecayRate * DeltaTime);

    // Detection tick
    TimeSinceLastDetection += DeltaTime;
    if (TimeSinceLastDetection >= DetectionInterval)
    {
        TimeSinceLastDetection = 0.0f;
        DetectNearbyActors();
    }

    // Attack cooldown
    TimeSinceLastAttack += DeltaTime;

    // State machine
    switch (BehaviorState)
    {
    case EEng_DinosaurBehaviorState::Patrolling:
        UpdatePatrol(DeltaTime);
        break;
    case EEng_DinosaurBehaviorState::Hunting:
        UpdateHunting(DeltaTime);
        break;
    case EEng_DinosaurBehaviorState::Fleeing:
        UpdateFleeing(DeltaTime);
        break;
    case EEng_DinosaurBehaviorState::Idle:
        // Transition to patrol after brief idle
        SetBehaviorState(EEng_DinosaurBehaviorState::Patrolling);
        break;
    default:
        break;
    }
}

void ADinosaurBase::DetectNearbyActors()
{
    if (!IsAlive()) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Sphere overlap to find player/prey
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(Stats.DetectionRadius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = World->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        Sphere,
        Params
    );

    if (bHit)
    {
        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* OtherActor = Overlap.GetActor();
            if (!OtherActor) continue;

            // Check if it's the player
            APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
            if (OtherActor == PlayerPawn && Stats.bIsCarnivore)
            {
                CurrentTarget = OtherActor;
                SetBehaviorState(EEng_DinosaurBehaviorState::Hunting);
                return;
            }
        }
    }

    // No target found — return to patrol if was hunting
    if (BehaviorState == EEng_DinosaurBehaviorState::Hunting && !CurrentTarget)
    {
        SetBehaviorState(EEng_DinosaurBehaviorState::Patrolling);
    }
}

void ADinosaurBase::UpdatePatrol(float DeltaTime)
{
    FVector CurrentLoc = GetActorLocation();
    FVector ToDestination = PatrolDestination - CurrentLoc;
    ToDestination.Z = 0.0f;
    float DistToDestination = ToDestination.Size();

    if (DistToDestination < 200.0f)
    {
        // Reached patrol point — choose a new one
        ChooseNewPatrolDestination();
    }
    else
    {
        // Move toward patrol destination
        FVector Direction = ToDestination.GetSafeNormal();
        AddMovementInput(Direction, 0.6f);
    }
}

void ADinosaurBase::ChooseNewPatrolDestination()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Pick random point within patrol radius
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Dist = FMath::RandRange(300.0f, Stats.PatrolRadius);
    FVector Offset(
        FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
        FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist,
        0.0f
    );
    PatrolDestination = PatrolOrigin + Offset;

    // Project to ground
    FHitResult GroundHit;
    FVector TraceStart = PatrolDestination + FVector(0, 0, 500);
    FVector TraceEnd = PatrolDestination - FVector(0, 0, 1000);
    if (World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_WorldStatic))
    {
        PatrolDestination = GroundHit.Location + FVector(0, 0, 10);
    }
}

void ADinosaurBase::UpdateHunting(float DeltaTime)
{
    if (!CurrentTarget || !IsAlive())
    {
        CurrentTarget = nullptr;
        SetBehaviorState(EEng_DinosaurBehaviorState::Patrolling);
        return;
    }

    // Check if target is still alive
    if (!IsValid(CurrentTarget))
    {
        CurrentTarget = nullptr;
        SetBehaviorState(EEng_DinosaurBehaviorState::Patrolling);
        return;
    }

    FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
    ToTarget.Z = 0.0f;
    float DistToTarget = ToTarget.Size();

    // Lost target — too far
    if (DistToTarget > Stats.DetectionRadius * 1.5f)
    {
        CurrentTarget = nullptr;
        SetBehaviorState(EEng_DinosaurBehaviorState::Patrolling);
        return;
    }

    // In attack range
    if (DistToTarget <= Stats.AttackRange)
    {
        PerformAttack();
    }
    else
    {
        // Chase at run speed
        GetCharacterMovement()->MaxWalkSpeed = Stats.RunSpeed;
        FVector Direction = ToTarget.GetSafeNormal();
        AddMovementInput(Direction, 1.0f);
    }
}

void ADinosaurBase::UpdateFleeing(float DeltaTime)
{
    if (!CurrentTarget) return;

    // Flee away from threat
    FVector AwayFromThreat = GetActorLocation() - CurrentTarget->GetActorLocation();
    AwayFromThreat.Z = 0.0f;
    AwayFromThreat.Normalize();

    GetCharacterMovement()->MaxWalkSpeed = Stats.RunSpeed;
    AddMovementInput(AwayFromThreat, 1.0f);

    // Stop fleeing after getting far enough
    float Dist = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
    if (Dist > Stats.DetectionRadius * 2.0f)
    {
        CurrentTarget = nullptr;
        GetCharacterMovement()->MaxWalkSpeed = Stats.WalkSpeed;
        SetBehaviorState(EEng_DinosaurBehaviorState::Patrolling);
    }
}

void ADinosaurBase::PerformAttack()
{
    if (TimeSinceLastAttack < AttackCooldown) return;
    if (!CurrentTarget || !IsAlive()) return;

    TimeSinceLastAttack = 0.0f;
    SetBehaviorState(EEng_DinosaurBehaviorState::Attacking);

    // Apply damage to target
    UGameplayStatics::ApplyDamage(
        CurrentTarget,
        Stats.AttackDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    OnAttackHit(CurrentTarget);

    // Return to hunting after attack
    SetBehaviorState(EEng_DinosaurBehaviorState::Hunting);
}

void ADinosaurBase::TakeDinosaurDamage(float DamageAmount, AActor* DamageInstigator)
{
    if (!IsAlive()) return;

    Stats.CurrentHealth = FMath::Max(0.0f, Stats.CurrentHealth - DamageAmount);

    // React to damage — flee if herbivore, fight back if carnivore
    if (!Stats.bIsCarnivore)
    {
        CurrentTarget = DamageInstigator;
        SetBehaviorState(EEng_DinosaurBehaviorState::Fleeing);
    }
    else if (DamageInstigator)
    {
        CurrentTarget = DamageInstigator;
        SetBehaviorState(EEng_DinosaurBehaviorState::Hunting);
    }

    if (Stats.CurrentHealth <= 0.0f)
    {
        OnDeath();
    }
}

bool ADinosaurBase::IsAlive() const
{
    return Stats.CurrentHealth > 0.0f && BehaviorState != EEng_DinosaurBehaviorState::Dead;
}

void ADinosaurBase::SetBehaviorState(EEng_DinosaurBehaviorState NewState)
{
    if (BehaviorState == EEng_DinosaurBehaviorState::Dead) return;
    BehaviorState = NewState;

    // Adjust speed based on state
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        switch (NewState)
        {
        case EEng_DinosaurBehaviorState::Hunting:
        case EEng_DinosaurBehaviorState::Fleeing:
            MoveComp->MaxWalkSpeed = Stats.RunSpeed;
            break;
        default:
            MoveComp->MaxWalkSpeed = Stats.WalkSpeed;
            break;
        }
    }
}

void ADinosaurBase::OnDeath_Implementation()
{
    SetBehaviorState(EEng_DinosaurBehaviorState::Dead);
    GetCharacterMovement()->DisableMovement();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Ragdoll on death
    if (GetMesh())
    {
        GetMesh()->SetSimulatePhysics(true);
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    // Destroy after 10 seconds
    SetLifeSpan(10.0f);
}

void ADinosaurBase::OnAttackHit_Implementation(AActor* HitActor)
{
    // Base implementation — subclasses can override for specific effects
    if (HitActor)
    {
        unreal::log(FString::Printf(TEXT("DinosaurBase: %s attacked %s"), *GetName(), *HitActor->GetName()));
    }
}
