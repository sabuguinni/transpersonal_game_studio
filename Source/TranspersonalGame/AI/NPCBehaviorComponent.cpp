#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for performance
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache home location
    if (AActor* Owner = GetOwner())
    {
        HomeLocation = Owner->GetActorLocation();
        CurrentPatrolTarget = PickNewPatrolPoint();
    }

    // Cache player reference
    CachedPlayerActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

    // Apply species-specific defaults
    switch (Species)
    {
    case ENPC_DinosaurSpecies::TRex:
        BehaviorConfig.PatrolRadius = 5000.0f;
        BehaviorConfig.DetectionRange = 3000.0f;
        BehaviorConfig.AttackRange = 350.0f;
        BehaviorConfig.ChaseSpeed = 900.0f;
        BehaviorConfig.AttackDamage = 80.0f;
        BehaviorConfig.bIsPackHunter = false;
        break;

    case ENPC_DinosaurSpecies::Velociraptor:
        BehaviorConfig.PatrolRadius = 3000.0f;
        BehaviorConfig.DetectionRange = 2000.0f;
        BehaviorConfig.AttackRange = 200.0f;
        BehaviorConfig.ChaseSpeed = 1200.0f;
        BehaviorConfig.AttackDamage = 35.0f;
        BehaviorConfig.bIsPackHunter = true;
        BehaviorConfig.PackAlertRadius = 1500.0f;
        break;

    case ENPC_DinosaurSpecies::Brachiosaurus:
        BehaviorConfig.PatrolRadius = 2000.0f;
        BehaviorConfig.DetectionRange = 800.0f;
        BehaviorConfig.AttackRange = 500.0f;
        BehaviorConfig.ChaseSpeed = 400.0f;
        BehaviorConfig.AttackDamage = 60.0f;
        BehaviorConfig.bIsPackHunter = false;
        CurrentState = ENPC_BehaviorState::Graze; // Herbivores start grazing
        break;

    case ENPC_DinosaurSpecies::Triceratops:
        BehaviorConfig.PatrolRadius = 1500.0f;
        BehaviorConfig.DetectionRange = 1000.0f;
        BehaviorConfig.AttackRange = 400.0f;
        BehaviorConfig.ChaseSpeed = 700.0f;
        BehaviorConfig.AttackDamage = 55.0f;
        BehaviorConfig.bIsPackHunter = false;
        break;

    default:
        break;
    }
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateMemoryDecay(DeltaTime);
    UpdateBehaviorLogic(DeltaTime);
}

void UNPCBehaviorComponent::UpdateBehaviorLogic(float DeltaTime)
{
    // Refresh player cache periodically
    if (!CachedPlayerActor)
    {
        CachedPlayerActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (!CachedPlayerActor) return;
    }

    float DistToPlayer = GetDistanceToPlayer();
    bool bSeePlayer = CanSeePlayer();

    // Update stimulus memory
    if (bSeePlayer)
    {
        StimulusMemory.bPlayerDetected = true;
        StimulusMemory.LastKnownPlayerLocation = CachedPlayerActor->GetActorLocation();
        StimulusMemory.TimeSinceLastSighting = 0.0f;
        StimulusMemory.ThreatLevel = FMath::Min(StimulusMemory.ThreatLevel + DeltaTime * 2.0f, 1.0f);

        // Alert pack if pack hunter
        if (BehaviorConfig.bIsPackHunter && CurrentState != ENPC_BehaviorState::Chase)
        {
            AlertPackMembers();
        }
    }

    // State machine transitions
    switch (CurrentState)
    {
    case ENPC_BehaviorState::Idle:
    case ENPC_BehaviorState::Patrol:
    case ENPC_BehaviorState::Graze:
        if (bSeePlayer && DistToPlayer < BehaviorConfig.DetectionRange)
        {
            SetBehaviorState(ENPC_BehaviorState::Chase);
        }
        else if (StimulusMemory.bAlertedByPackMember)
        {
            SetBehaviorState(ENPC_BehaviorState::Investigate);
        }
        else
        {
            UpdatePatrol(DeltaTime);
        }
        break;

    case ENPC_BehaviorState::Investigate:
        if (bSeePlayer && DistToPlayer < BehaviorConfig.DetectionRange)
        {
            SetBehaviorState(ENPC_BehaviorState::Chase);
        }
        else if (StimulusMemory.TimeSinceLastSighting > 10.0f)
        {
            SetBehaviorState(ENPC_BehaviorState::Patrol);
            StimulusMemory.bAlertedByPackMember = false;
        }
        break;

    case ENPC_BehaviorState::Chase:
        if (DistToPlayer <= BehaviorConfig.AttackRange)
        {
            SetBehaviorState(ENPC_BehaviorState::Attack);
        }
        else if (!bSeePlayer && StimulusMemory.TimeSinceLastSighting > 8.0f)
        {
            SetBehaviorState(ENPC_BehaviorState::Patrol);
        }
        else
        {
            UpdateChase(DeltaTime);
        }
        break;

    case ENPC_BehaviorState::Attack:
        if (DistToPlayer > BehaviorConfig.AttackRange * 1.5f)
        {
            SetBehaviorState(ENPC_BehaviorState::Chase);
        }
        else
        {
            UpdateAttack(DeltaTime);
        }
        break;

    case ENPC_BehaviorState::Flee:
        // Flee logic — move away from player
        if (DistToPlayer > BehaviorConfig.DetectionRange * 2.0f)
        {
            SetBehaviorState(ENPC_BehaviorState::Patrol);
        }
        break;

    default:
        break;
    }
}

void UNPCBehaviorComponent::UpdatePatrol(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector CurrentLoc = Owner->GetActorLocation();
    float DistToTarget = FVector::Dist(CurrentLoc, CurrentPatrolTarget);

    if (DistToTarget < 200.0f)
    {
        PatrolWaitTimer += DeltaTime;
        if (PatrolWaitTimer > 3.0f)
        {
            CurrentPatrolTarget = PickNewPatrolPoint();
            PatrolWaitTimer = 0.0f;
        }
    }
    else
    {
        // Move toward patrol target
        FVector Direction = (CurrentPatrolTarget - CurrentLoc).GetSafeNormal();
        FVector NewLoc = CurrentLoc + Direction * BehaviorConfig.PatrolSpeed * DeltaTime;
        Owner->SetActorLocation(NewLoc, true);

        // Face movement direction
        FRotator FaceRot = UKismetMathLibrary::FindLookAtRotation(CurrentLoc, CurrentPatrolTarget);
        Owner->SetActorRotation(FRotator(0, FaceRot.Yaw, 0));
    }
}

void UNPCBehaviorComponent::UpdateChase(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner || !CachedPlayerActor) return;

    FVector CurrentLoc = Owner->GetActorLocation();
    FVector TargetLoc = StimulusMemory.bPlayerDetected
        ? StimulusMemory.LastKnownPlayerLocation
        : CurrentPatrolTarget;

    FVector Direction = (TargetLoc - CurrentLoc).GetSafeNormal();
    FVector NewLoc = CurrentLoc + Direction * BehaviorConfig.ChaseSpeed * DeltaTime;
    Owner->SetActorLocation(NewLoc, true);

    FRotator FaceRot = UKismetMathLibrary::FindLookAtRotation(CurrentLoc, TargetLoc);
    Owner->SetActorRotation(FRotator(0, FaceRot.Yaw, 0));
}

void UNPCBehaviorComponent::UpdateAttack(float DeltaTime)
{
    AttackCooldownTimer -= DeltaTime;
    if (AttackCooldownTimer <= 0.0f)
    {
        // Apply damage to player
        if (CachedPlayerActor)
        {
            UGameplayStatics::ApplyDamage(
                CachedPlayerActor,
                BehaviorConfig.AttackDamage,
                nullptr,
                GetOwner(),
                nullptr
            );
        }
        AttackCooldownTimer = BehaviorConfig.AttackCooldown;
    }
}

void UNPCBehaviorComponent::UpdateMemoryDecay(float DeltaTime)
{
    if (StimulusMemory.TimeSinceLastSighting < 60.0f)
    {
        StimulusMemory.TimeSinceLastSighting += DeltaTime;
    }

    if (!StimulusMemory.bPlayerDetected) return;

    StimulusMemory.ThreatLevel = FMath::Max(
        StimulusMemory.ThreatLevel - MemoryDecayRate * DeltaTime,
        0.0f
    );

    if (StimulusMemory.ThreatLevel <= 0.0f)
    {
        StimulusMemory.bPlayerDetected = false;
    }
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
}

bool UNPCBehaviorComponent::CanSeePlayer() const
{
    if (!CachedPlayerActor || !GetOwner()) return false;

    float Dist = GetDistanceToPlayer();
    if (Dist > BehaviorConfig.DetectionRange) return false;

    // Line of sight check
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    FVector Start = GetOwner()->GetActorLocation();
    FVector End = CachedPlayerActor->GetActorLocation();

    bool bBlocked = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);
    if (bBlocked && HitResult.GetActor() != CachedPlayerActor)
    {
        return false; // Obstacle blocking line of sight
    }

    return true;
}

float UNPCBehaviorComponent::GetDistanceToPlayer() const
{
    if (!CachedPlayerActor || !GetOwner()) return TNumericLimits<float>::Max();
    return FVector::Dist(GetOwner()->GetActorLocation(), CachedPlayerActor->GetActorLocation());
}

void UNPCBehaviorComponent::AlertPackMembers()
{
    if (!GetOwner()) return;

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), GetOwner()->GetClass(), NearbyActors);

    for (AActor* PackMember : NearbyActors)
    {
        if (PackMember == GetOwner()) continue;

        float Dist = FVector::Dist(GetOwner()->GetActorLocation(), PackMember->GetActorLocation());
        if (Dist <= BehaviorConfig.PackAlertRadius)
        {
            UNPCBehaviorComponent* MemberBehavior = PackMember->FindComponentByClass<UNPCBehaviorComponent>();
            if (MemberBehavior)
            {
                MemberBehavior->OnPackAlert(StimulusMemory.LastKnownPlayerLocation);
            }
        }
    }
}

void UNPCBehaviorComponent::OnPackAlert(FVector ThreatLocation)
{
    StimulusMemory.LastKnownPlayerLocation = ThreatLocation;
    StimulusMemory.bAlertedByPackMember = true;
    StimulusMemory.ThreatLevel = 0.6f;

    if (CurrentState == ENPC_BehaviorState::Idle ||
        CurrentState == ENPC_BehaviorState::Patrol ||
        CurrentState == ENPC_BehaviorState::Graze)
    {
        SetBehaviorState(ENPC_BehaviorState::Investigate);
    }
}

FVector UNPCBehaviorComponent::PickNewPatrolPoint()
{
    FVector RandomOffset = FVector(
        FMath::RandRange(-BehaviorConfig.PatrolRadius, BehaviorConfig.PatrolRadius),
        FMath::RandRange(-BehaviorConfig.PatrolRadius, BehaviorConfig.PatrolRadius),
        0.0f
    );
    return HomeLocation + RandomOffset;
}
