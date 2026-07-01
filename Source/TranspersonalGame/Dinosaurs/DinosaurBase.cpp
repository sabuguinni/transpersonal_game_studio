#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default capsule size — overridden per species in child classes
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    // Movement defaults
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = DinoStats.PatrolSpeed;
    GetCharacterMovement()->JumpZVelocity = 0.0f; // Most dinosaurs cannot jump
    GetCharacterMovement()->AirControl = 0.0f;
    GetCharacterMovement()->GravityScale = 1.5f;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Initialize health from stats
    DinoStats.CurrentHealth = DinoStats.MaxHealth;

    // Set movement speed for patrol
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = DinoStats.PatrolSpeed;
    }

    // Start patrolling if waypoints exist
    if (PatrolWaypoints.Num() > 0)
    {
        SetBehaviorState(EEng_DinosaurBehaviorState::Patrolling);
    }
    else
    {
        SetBehaviorState(EEng_DinosaurBehaviorState::Idle);
    }

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase BeginPlay: %s [Species=%d, HP=%.0f, State=%d]"),
        *GetName(), (int32)Species, DinoStats.CurrentHealth, (int32)BehaviorState);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsAlive()) return;

    // Throttled behavior update
    BehaviorUpdateTimer += DeltaTime;
    if (BehaviorUpdateTimer >= BehaviorUpdateInterval)
    {
        BehaviorUpdateTimer = 0.0f;
        ScanForTargets();
    }

    // Per-frame behavior execution
    switch (BehaviorState)
    {
        case EEng_DinosaurBehaviorState::Patrolling:
            UpdatePatrol(DeltaTime);
            break;
        case EEng_DinosaurBehaviorState::Hunting:
        case EEng_DinosaurBehaviorState::Attacking:
            UpdateHunting(DeltaTime);
            break;
        case EEng_DinosaurBehaviorState::Idle:
        case EEng_DinosaurBehaviorState::Resting:
        case EEng_DinosaurBehaviorState::Feeding:
        case EEng_DinosaurBehaviorState::Fleeing:
        default:
            break;
    }
}

void ADinosaurBase::TakeDinosaurDamage(float DamageAmount, AActor* DamageInstigator)
{
    if (!IsAlive()) return;

    DinoStats.CurrentHealth = FMath::Max(0.0f, DinoStats.CurrentHealth - DamageAmount);

    UE_LOG(LogTemp, Log, TEXT("%s took %.1f damage. HP: %.1f/%.1f"),
        *GetName(), DamageAmount, DinoStats.CurrentHealth, DinoStats.MaxHealth);

    if (DinoStats.CurrentHealth <= 0.0f)
    {
        OnDinosaurDeath();
    }
    else if (DamageInstigator && DinoStats.bIsCarnivore)
    {
        // Retaliate — set the attacker as target
        CurrentTarget = DamageInstigator;
        SetBehaviorState(EEng_DinosaurBehaviorState::Attacking);
    }
}

void ADinosaurBase::SetBehaviorState(EEng_DinosaurBehaviorState NewState)
{
    if (BehaviorState == NewState) return;

    BehaviorState = NewState;

    // Adjust movement speed based on state
    if (GetCharacterMovement())
    {
        switch (NewState)
        {
            case EEng_DinosaurBehaviorState::Hunting:
            case EEng_DinosaurBehaviorState::Attacking:
                GetCharacterMovement()->MaxWalkSpeed = DinoStats.ChaseSpeed;
                break;
            case EEng_DinosaurBehaviorState::Fleeing:
                GetCharacterMovement()->MaxWalkSpeed = DinoStats.ChaseSpeed * 1.2f;
                break;
            default:
                GetCharacterMovement()->MaxWalkSpeed = DinoStats.PatrolSpeed;
                break;
        }
    }
}

float ADinosaurBase::GetHealthPercent() const
{
    if (DinoStats.MaxHealth <= 0.0f) return 0.0f;
    return DinoStats.CurrentHealth / DinoStats.MaxHealth;
}

bool ADinosaurBase::IsAlive() const
{
    return DinoStats.CurrentHealth > 0.0f;
}

void ADinosaurBase::AddPatrolWaypoint(FVector WaypointLocation)
{
    PatrolWaypoints.Add(WaypointLocation);
}

void ADinosaurBase::ClearPatrolWaypoints()
{
    PatrolWaypoints.Empty();
    CurrentWaypointIndex = 0;
}

void ADinosaurBase::OnDinosaurDeath_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Dinosaur died: %s"), *GetName());
    SetBehaviorState(EEng_DinosaurBehaviorState::Idle);

    // Disable collision and movement on death
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->DisableMovement();
    }

    // Ragdoll on death — enable physics on mesh
    if (GetMesh())
    {
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        GetMesh()->SetSimulatePhysics(true);
    }
}

void ADinosaurBase::OnTargetDetected_Implementation(AActor* DetectedActor)
{
    if (!DetectedActor) return;

    UE_LOG(LogTemp, Log, TEXT("%s detected target: %s"), *GetName(), *DetectedActor->GetName());
    CurrentTarget = DetectedActor;

    if (DinoStats.bIsCarnivore)
    {
        SetBehaviorState(EEng_DinosaurBehaviorState::Hunting);
    }
}

void ADinosaurBase::UpdatePatrol(float DeltaTime)
{
    if (PatrolWaypoints.Num() == 0) return;

    FVector CurrentWaypoint = PatrolWaypoints[CurrentWaypointIndex];
    FVector CurrentLocation = GetActorLocation();
    float DistanceToWaypoint = FVector::Dist2D(CurrentLocation, CurrentWaypoint);

    if (DistanceToWaypoint <= WaypointAcceptanceRadius)
    {
        MoveToNextWaypoint();
    }
    else
    {
        // Move toward waypoint
        FVector Direction = (CurrentWaypoint - CurrentLocation).GetSafeNormal2D();
        AddMovementInput(Direction, 1.0f);
    }
}

void ADinosaurBase::UpdateHunting(float DeltaTime)
{
    if (!CurrentTarget || !IsValid(CurrentTarget))
    {
        CurrentTarget = nullptr;
        SetBehaviorState(EEng_DinosaurBehaviorState::Patrolling);
        return;
    }

    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector MyLocation = GetActorLocation();
    float DistanceToTarget = FVector::Dist2D(MyLocation, TargetLocation);

    if (DistanceToTarget <= DinoStats.AttackRadius)
    {
        // In attack range — deal damage
        SetBehaviorState(EEng_DinosaurBehaviorState::Attacking);
        UGameplayStatics::ApplyDamage(CurrentTarget, DinoStats.AttackDamage, GetController(), this, nullptr);
    }
    else if (DistanceToTarget > DinoStats.DetectionRadius * 1.5f)
    {
        // Lost target — return to patrol
        CurrentTarget = nullptr;
        SetBehaviorState(EEng_DinosaurBehaviorState::Patrolling);
    }
    else
    {
        // Chase target
        FVector Direction = (TargetLocation - MyLocation).GetSafeNormal2D();
        AddMovementInput(Direction, 1.0f);
    }
}

void ADinosaurBase::ScanForTargets()
{
    if (!DinoStats.bIsCarnivore) return;
    if (BehaviorState == EEng_DinosaurBehaviorState::Hunting ||
        BehaviorState == EEng_DinosaurBehaviorState::Attacking) return;

    // Sphere overlap to find player
    TArray<AActor*> OverlappingActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), OverlappingActors);

    for (AActor* Actor : OverlappingActors)
    {
        if (Actor == this) continue;

        float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Distance <= DinoStats.DetectionRadius)
        {
            OnTargetDetected(Actor);
            return;
        }
    }
}

bool ADinosaurBase::IsTargetInRange(AActor* Target, float Range) const
{
    if (!Target) return false;
    return FVector::Dist(GetActorLocation(), Target->GetActorLocation()) <= Range;
}

void ADinosaurBase::MoveToNextWaypoint()
{
    if (PatrolWaypoints.Num() == 0) return;
    CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();
}
