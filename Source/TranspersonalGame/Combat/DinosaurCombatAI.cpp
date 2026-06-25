#include "DinosaurCombatAI.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDinosaurCombatAI::UDinosaurCombatAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz — enough for AI, saves perf
}

void UDinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();
    ApplySpeciesDefaults();
    TransitionToState(ECombat_DinoState::Patrolling);
}

void UDinosaurCombatAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeSinceLastAttack += DeltaTime;
    StateTimer += DeltaTime;

    // Flee if health critically low
    if (Stats.CurrentHealth / Stats.MaxHealth <= Stats.FleeHealthThreshold
        && CurrentState != ECombat_DinoState::Fleeing)
    {
        TransitionToState(ECombat_DinoState::Fleeing);
        return;
    }

    switch (CurrentState)
    {
        case ECombat_DinoState::Idle:        UpdateIdle(DeltaTime);       break;
        case ECombat_DinoState::Patrolling:  UpdatePatrolling(DeltaTime); break;
        case ECombat_DinoState::Alerted:     UpdateAlerted(DeltaTime);    break;
        case ECombat_DinoState::Stalking:    UpdateStalking(DeltaTime);   break;
        case ECombat_DinoState::Charging:    UpdateCharging(DeltaTime);   break;
        case ECombat_DinoState::Attacking:   UpdateAttacking(DeltaTime);  break;
        case ECombat_DinoState::Retreating:  UpdateRetreating(DeltaTime); break;
        case ECombat_DinoState::Fleeing:     UpdateFleeing(DeltaTime);    break;
        default: break;
    }
}

void UDinosaurCombatAI::ApplySpeciesDefaults()
{
    switch (Species)
    {
        case ECombat_DinoSpecies::TRex:
            Stats.MaxHealth = 1200.0f;
            Stats.CurrentHealth = 1200.0f;
            Stats.AttackDamage = 150.0f;
            Stats.AttackRange = 350.0f;
            Stats.DetectionRange = 2500.0f;
            Stats.ChargeSpeed = 800.0f;
            Stats.PatrolSpeed = 250.0f;
            Stats.AttackCooldown = 3.0f;
            bIsPackAnimal = false;
            break;

        case ECombat_DinoSpecies::Velociraptor:
            Stats.MaxHealth = 200.0f;
            Stats.CurrentHealth = 200.0f;
            Stats.AttackDamage = 45.0f;
            Stats.AttackRange = 120.0f;
            Stats.DetectionRange = 1800.0f;
            Stats.ChargeSpeed = 1200.0f;
            Stats.PatrolSpeed = 500.0f;
            Stats.AttackCooldown = 1.0f;
            bIsPackAnimal = true;
            PackAlertRadius = 2500.0f;
            break;

        case ECombat_DinoSpecies::Triceratops:
            Stats.MaxHealth = 800.0f;
            Stats.CurrentHealth = 800.0f;
            Stats.AttackDamage = 100.0f;
            Stats.AttackRange = 280.0f;
            Stats.DetectionRange = 1000.0f;
            Stats.ChargeSpeed = 700.0f;
            Stats.PatrolSpeed = 200.0f;
            Stats.AttackCooldown = 4.0f;
            Stats.FleeHealthThreshold = 0.1f; // Triceratops fights to the end
            bIsPackAnimal = false;
            break;

        case ECombat_DinoSpecies::Brachiosaurus:
            Stats.MaxHealth = 2000.0f;
            Stats.CurrentHealth = 2000.0f;
            Stats.AttackDamage = 60.0f; // Tail swipe
            Stats.AttackRange = 400.0f;
            Stats.DetectionRange = 800.0f;
            Stats.ChargeSpeed = 400.0f;
            Stats.PatrolSpeed = 150.0f;
            Stats.FleeHealthThreshold = 0.3f;
            bIsPackAnimal = false;
            break;

        case ECombat_DinoSpecies::Ankylosaurus:
            Stats.MaxHealth = 900.0f;
            Stats.CurrentHealth = 900.0f;
            Stats.AttackDamage = 120.0f;
            Stats.AttackRange = 220.0f;
            Stats.DetectionRange = 900.0f;
            Stats.ChargeSpeed = 500.0f;
            Stats.PatrolSpeed = 180.0f;
            Stats.FleeHealthThreshold = 0.05f;
            bIsPackAnimal = false;
            break;

        default:
            // Generic defaults already set in struct
            break;
    }
}

void UDinosaurCombatAI::UpdateIdle(float DeltaTime)
{
    if (StateTimer > 3.0f)
    {
        TransitionToState(ECombat_DinoState::Patrolling);
    }

    AActor* Threat = FindNearestThreat();
    if (Threat)
    {
        CurrentTarget = Threat;
        TransitionToState(ECombat_DinoState::Alerted);
    }
}

void UDinosaurCombatAI::UpdatePatrolling(float DeltaTime)
{
    AActor* Threat = FindNearestThreat();
    if (Threat)
    {
        CurrentTarget = Threat;
        if (bIsPackAnimal)
        {
            AlertPackMembers();
        }
        TransitionToState(ECombat_DinoState::Alerted);
    }
}

void UDinosaurCombatAI::UpdateAlerted(float DeltaTime)
{
    if (!CurrentTarget)
    {
        TransitionToState(ECombat_DinoState::Patrolling);
        return;
    }

    float DistToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());

    // If target is within detection range and we can see it — stalk or charge
    if (DistToTarget <= Stats.DetectionRange && CanSeeTarget(CurrentTarget))
    {
        if (StateTimer > 1.5f) // Brief alert pause before committing
        {
            // T-Rex charges directly; raptors stalk first
            if (Species == ECombat_DinoSpecies::Velociraptor)
            {
                TransitionToState(ECombat_DinoState::Stalking);
            }
            else
            {
                TransitionToState(ECombat_DinoState::Charging);
            }
        }
    }
    else if (StateTimer > 8.0f)
    {
        // Lost sight — return to patrol
        CurrentTarget = nullptr;
        TransitionToState(ECombat_DinoState::Patrolling);
    }
}

void UDinosaurCombatAI::UpdateStalking(float DeltaTime)
{
    if (!CurrentTarget)
    {
        TransitionToState(ECombat_DinoState::Patrolling);
        return;
    }

    float DistToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());

    // Stalk until close enough, then charge
    if (DistToTarget < 600.0f)
    {
        TransitionToState(ECombat_DinoState::Charging);
    }
    else
    {
        // Move slowly toward target
        MoveToward(CurrentTarget->GetActorLocation(), Stats.PatrolSpeed * 1.5f);
    }

    if (StateTimer > 15.0f)
    {
        // Gave up stalking
        TransitionToState(ECombat_DinoState::Patrolling);
    }
}

void UDinosaurCombatAI::UpdateCharging(float DeltaTime)
{
    if (!CurrentTarget)
    {
        TransitionToState(ECombat_DinoState::Patrolling);
        return;
    }

    float DistToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());

    if (DistToTarget <= Stats.AttackRange)
    {
        TransitionToState(ECombat_DinoState::Attacking);
    }
    else
    {
        MoveToward(CurrentTarget->GetActorLocation(), Stats.ChargeSpeed);
    }
}

void UDinosaurCombatAI::UpdateAttacking(float DeltaTime)
{
    if (!CurrentTarget)
    {
        TransitionToState(ECombat_DinoState::Patrolling);
        return;
    }

    float DistToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());

    if (DistToTarget > Stats.AttackRange * 1.5f)
    {
        // Target moved away — charge again
        TransitionToState(ECombat_DinoState::Charging);
        return;
    }

    if (TimeSinceLastAttack >= Stats.AttackCooldown)
    {
        ExecuteAttack();
    }
}

void UDinosaurCombatAI::UpdateRetreating(float DeltaTime)
{
    if (!CurrentTarget) return;

    // Move away from threat
    FVector AwayDir = (GetOwner()->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
    FVector RetreatDest = GetOwner()->GetActorLocation() + AwayDir * 1000.0f;
    MoveToward(RetreatDest, Stats.PatrolSpeed);

    if (StateTimer > 5.0f)
    {
        TransitionToState(ECombat_DinoState::Alerted);
    }
}

void UDinosaurCombatAI::UpdateFleeing(float DeltaTime)
{
    if (!CurrentTarget) return;

    FVector AwayDir = (GetOwner()->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
    FVector FleeDestination = GetOwner()->GetActorLocation() + AwayDir * 3000.0f;
    MoveToward(FleeDestination, Stats.ChargeSpeed * 0.8f);
}

AActor* UDinosaurCombatAI::FindNearestThreat() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    // Find player pawn as primary threat
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return nullptr;

    float DistToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
    if (DistToPlayer <= Stats.DetectionRange)
    {
        return PlayerPawn;
    }

    return nullptr;
}

bool UDinosaurCombatAI::CanSeeTarget(AActor* Target) const
{
    if (!Target || !GetOwner()) return false;

    UWorld* World = GetWorld();
    if (!World) return false;

    FVector Start = GetOwner()->GetActorLocation() + FVector(0, 0, 100.0f);
    FVector End = Target->GetActorLocation() + FVector(0, 0, 100.0f);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

    // Can see if no obstruction, or the obstruction IS the target
    return !bHit || Hit.GetActor() == Target;
}

void UDinosaurCombatAI::TransitionToState(ECombat_DinoState NewState)
{
    CurrentState = NewState;
    StateTimer = 0.0f;
}

void UDinosaurCombatAI::ExecuteAttack()
{
    if (!CurrentTarget) return;

    TimeSinceLastAttack = 0.0f;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(
        CurrentTarget,
        Stats.AttackDamage,
        nullptr,
        GetOwner(),
        nullptr
    );

    UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI: %s attacked %s for %.1f damage"),
        *GetOwner()->GetActorLabel(),
        *CurrentTarget->GetActorLabel(),
        Stats.AttackDamage);
}

void UDinosaurCombatAI::MoveToward(const FVector& Destination, float Speed)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    ACharacter* OwnerChar = Cast<ACharacter>(Owner);
    if (OwnerChar && OwnerChar->GetCharacterMovement())
    {
        OwnerChar->GetCharacterMovement()->MaxWalkSpeed = Speed;
    }

    FVector Direction = (Destination - Owner->GetActorLocation()).GetSafeNormal();
    FVector NewLocation = Owner->GetActorLocation() + Direction * Speed * 0.1f; // 0.1s tick
    Owner->SetActorLocation(NewLocation, true);
}

void UDinosaurCombatAI::TakeCombatDamage(float Damage, AActor* DamageSource)
{
    Stats.CurrentHealth = FMath::Max(0.0f, Stats.CurrentHealth - Damage);

    if (DamageSource && CurrentState == ECombat_DinoState::Patrolling)
    {
        CurrentTarget = DamageSource;
        if (bIsPackAnimal)
        {
            AlertPackMembers();
        }
        TransitionToState(ECombat_DinoState::Charging);
    }

    if (Stats.CurrentHealth <= 0.0f)
    {
        // Dino is dead — disable AI
        PrimaryComponentTick.bCanEverTick = false;
        UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI: %s has been defeated."), *GetOwner()->GetActorLabel());
    }
}

void UDinosaurCombatAI::AlertPackMembers()
{
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == GetOwner()) continue;

        float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
        if (Dist > PackAlertRadius) continue;

        UDinosaurCombatAI* PackAI = Actor->FindComponentByClass<UDinosaurCombatAI>();
        if (PackAI && PackAI->Species == Species && PackAI->CurrentState == ECombat_DinoState::Patrolling)
        {
            PackAI->CurrentTarget = CurrentTarget;
            PackAI->TransitionToState(ECombat_DinoState::Alerted);
        }
    }
}

float UDinosaurCombatAI::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.0f) return 0.0f;
    return Stats.CurrentHealth / Stats.MaxHealth;
}

bool UDinosaurCombatAI::IsHostile() const
{
    return CurrentState == ECombat_DinoState::Charging
        || CurrentState == ECombat_DinoState::Attacking
        || CurrentState == ECombat_DinoState::Stalking;
}

void UDinosaurCombatAI::ForceState(ECombat_DinoState NewState)
{
    TransitionToState(NewState);
}
