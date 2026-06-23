#include "RaptorCombatComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

URaptorCombatComponent::URaptorCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f; // 20Hz tick — sufficient for AI

    CurrentState = ECombat_RaptorState::Idle;
    PackRole = ECombat_RaptorRole::Alpha;
    CurrentTarget = nullptr;
    LastAttackTime = 0.0f;
    StateTimer = 0.0f;
    bHasLineOfSight = false;
    FlankDestination = FVector::ZeroVector;
    PatrolOrigin = FVector::ZeroVector;
}

void URaptorCombatComponent::BeginPlay()
{
    Super::BeginPlay();

    if (GetOwner())
    {
        PatrolOrigin = GetOwner()->GetActorLocation();
    }

    // Assign flank offset based on role
    switch (PackRole)
    {
    case ECombat_RaptorRole::FlankLeft:
        FlankOffset = -450.0f;
        break;
    case ECombat_RaptorRole::FlankRight:
        FlankOffset = 450.0f;
        break;
    case ECombat_RaptorRole::Ambush:
        FlankOffset = 0.0f; // Ambush comes from behind
        break;
    default:
        FlankOffset = 0.0f;
        break;
    }
}

void URaptorCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!IsAlive()) return;

    StateTimer += DeltaTime;

    // Always try to detect player if no target
    if (!CurrentTarget)
    {
        DetectPlayer();
    }

    UpdateStateMachine(DeltaTime);
}

void URaptorCombatComponent::UpdateStateMachine(float DeltaTime)
{
    switch (CurrentState)
    {
    case ECombat_RaptorState::Idle:
        // Idle raptors periodically scan for targets
        if (StateTimer > 2.0f)
        {
            DetectPlayer();
            StateTimer = 0.0f;
        }
        break;

    case ECombat_RaptorState::Stalking:
        UpdateStalkingBehavior(DeltaTime);
        break;

    case ECombat_RaptorState::Flanking:
        UpdateFlankingBehavior(DeltaTime);
        break;

    case ECombat_RaptorState::Charging:
        UpdateChargingBehavior(DeltaTime);
        break;

    case ECombat_RaptorState::Attacking:
        UpdateAttackBehavior(DeltaTime);
        break;

    case ECombat_RaptorState::Retreating:
        UpdateRetreatBehavior(DeltaTime);
        break;

    case ECombat_RaptorState::PackCall:
        // Broadcast pack signal then transition to flanking
        BroadcastPackSignal();
        TransitionToState(ECombat_RaptorState::Flanking);
        break;
    }
}

void URaptorCombatComponent::UpdateStalkingBehavior(float DeltaTime)
{
    if (!CurrentTarget) 
    {
        TransitionToState(ECombat_RaptorState::Idle);
        return;
    }

    float DistToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());

    // If close enough, transition based on role
    if (DistToTarget < Sensory.AggroRange)
    {
        if (PackRole == ECombat_RaptorRole::Alpha)
        {
            // Alpha triggers pack call first
            TransitionToState(ECombat_RaptorState::PackCall);
        }
        else
        {
            // Flankers go directly to flanking
            TransitionToState(ECombat_RaptorState::Flanking);
        }
    }
}

void URaptorCombatComponent::UpdateFlankingBehavior(float DeltaTime)
{
    if (!CurrentTarget)
    {
        TransitionToState(ECombat_RaptorState::Idle);
        return;
    }

    FVector TargetLoc = CurrentTarget->GetActorLocation();
    FVector OwnerLoc = GetOwner()->GetActorLocation();

    // Calculate flank position
    bool bLeftFlank = (PackRole == ECombat_RaptorRole::FlankLeft);
    FlankDestination = CalculateFlankPosition(TargetLoc, bLeftFlank);

    float DistToFlankPos = FVector::Dist(OwnerLoc, FlankDestination);
    float DistToTarget = FVector::Dist(OwnerLoc, TargetLoc);

    // If we've reached flank position OR close enough to target, charge
    if (DistToFlankPos < 200.0f || DistToTarget < Stats.AttackRange * 1.5f)
    {
        TransitionToState(ECombat_RaptorState::Charging);
    }
}

void URaptorCombatComponent::UpdateChargingBehavior(float DeltaTime)
{
    if (!CurrentTarget)
    {
        TransitionToState(ECombat_RaptorState::Idle);
        return;
    }

    float DistToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());

    // In attack range — switch to attacking
    if (DistToTarget <= Stats.AttackRange)
    {
        TransitionToState(ECombat_RaptorState::Attacking);
    }

    // Health low — retreat
    if (Stats.CurrentHealth < Stats.MaxHealth * 0.25f)
    {
        TransitionToState(ECombat_RaptorState::Retreating);
    }
}

void URaptorCombatComponent::UpdateAttackBehavior(float DeltaTime)
{
    if (!CurrentTarget)
    {
        TransitionToState(ECombat_RaptorState::Idle);
        return;
    }

    float DistToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());

    // Target moved out of range — charge again
    if (DistToTarget > Stats.AttackRange * 1.5f)
    {
        TransitionToState(ECombat_RaptorState::Charging);
        return;
    }

    // Execute attack if cooldown elapsed
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (CurrentTime - LastAttackTime >= Stats.AttackCooldown)
    {
        ExecuteAttack();
        LastAttackTime = CurrentTime;
    }

    // Health low — retreat
    if (Stats.CurrentHealth < Stats.MaxHealth * 0.25f)
    {
        TransitionToState(ECombat_RaptorState::Retreating);
    }
}

void URaptorCombatComponent::UpdateRetreatBehavior(float DeltaTime)
{
    // Retreat for 3 seconds then re-evaluate
    if (StateTimer > 3.0f)
    {
        if (Stats.CurrentHealth > Stats.MaxHealth * 0.4f && CurrentTarget)
        {
            // Recovered enough — re-engage
            TransitionToState(ECombat_RaptorState::Stalking);
        }
        else
        {
            TransitionToState(ECombat_RaptorState::Idle);
            CurrentTarget = nullptr;
        }
    }
}

void URaptorCombatComponent::DetectPlayer()
{
    if (!GetWorld()) return;

    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerChar) return;

    FVector OwnerLoc = GetOwner()->GetActorLocation();
    FVector PlayerLoc = PlayerChar->GetActorLocation();
    float Distance = FVector::Dist(OwnerLoc, PlayerLoc);

    // Distance check
    if (Distance > Sensory.SightRange) return;

    // FOV check
    FVector ToPlayer = (PlayerLoc - OwnerLoc).GetSafeNormal();
    FVector ForwardVec = GetOwner()->GetActorForwardVector();
    float DotProduct = FVector::DotProduct(ForwardVec, ToPlayer);
    float HalfFOVCos = FMath::Cos(FMath::DegreesToRadians(Sensory.FieldOfViewDegrees * 0.5f));

    if (DotProduct < HalfFOVCos && Distance > Sensory.HearingRange) return;

    // Line of sight check
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());

    bool bBlocked = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        OwnerLoc,
        PlayerLoc,
        ECC_Visibility,
        QueryParams
    );

    bHasLineOfSight = !bBlocked || (bBlocked && HitResult.GetActor() == PlayerChar);

    if (bHasLineOfSight || Distance < Sensory.HearingRange)
    {
        CurrentTarget = PlayerChar;
        TransitionToState(ECombat_RaptorState::Stalking);
    }
}

void URaptorCombatComponent::ExecuteAttack()
{
    if (!CurrentTarget || !GetWorld()) return;

    float DistToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    if (DistToTarget > Stats.AttackRange) return;

    // Alternate between claw and bite based on pack role
    float DamageAmount = (PackRole == ECombat_RaptorRole::Alpha) ? Stats.BiteDamage : Stats.ClawDamage;

    // Apply damage via UE5 damage system
    UGameplayStatics::ApplyDamage(
        CurrentTarget,
        DamageAmount,
        nullptr,
        GetOwner(),
        nullptr
    );

    UE_LOG(LogTemp, Log, TEXT("Raptor [%s] attacked %s for %.1f damage"),
        *GetOwner()->GetActorLabel(),
        *CurrentTarget->GetName(),
        DamageAmount);
}

void URaptorCombatComponent::TransitionToState(ECombat_RaptorState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;
    StateTimer = 0.0f;
}

FVector URaptorCombatComponent::CalculateFlankPosition(FVector TargetLocation, bool bLeftFlank) const
{
    if (!CurrentTarget) return TargetLocation;

    // Get direction from target to owner
    FVector OwnerLoc = GetOwner()->GetActorLocation();
    FVector ToOwner = (OwnerLoc - TargetLocation).GetSafeNormal();

    // Perpendicular flank direction
    FVector FlankDir = bLeftFlank
        ? FVector(-ToOwner.Y, ToOwner.X, 0.0f).GetSafeNormal()
        : FVector(ToOwner.Y, -ToOwner.X, 0.0f).GetSafeNormal();

    // Flank position: beside and slightly behind target
    return TargetLocation + FlankDir * FMath::Abs(FlankOffset) + ToOwner * 200.0f;
}

void URaptorCombatComponent::SetPackRole(ECombat_RaptorRole NewRole)
{
    PackRole = NewRole;

    switch (NewRole)
    {
    case ECombat_RaptorRole::FlankLeft:
        FlankOffset = -450.0f;
        break;
    case ECombat_RaptorRole::FlankRight:
        FlankOffset = 450.0f;
        break;
    default:
        FlankOffset = 0.0f;
        break;
    }
}

void URaptorCombatComponent::EngageTarget(AActor* Target)
{
    if (!Target) return;
    CurrentTarget = Target;
    TransitionToState(ECombat_RaptorState::Stalking);
}

void URaptorCombatComponent::ReceivePackSignal(const FCombat_PackSignal& Signal)
{
    PackSignal = Signal;

    if (Signal.bPackEngaged && CurrentTarget == nullptr)
    {
        // Find the player at the signaled location
        if (GetWorld())
        {
            ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
            if (PlayerChar)
            {
                float DistToSignal = FVector::Dist(PlayerChar->GetActorLocation(), Signal.TargetLocation);
                if (DistToSignal < 500.0f)
                {
                    CurrentTarget = PlayerChar;
                    TransitionToState(ECombat_RaptorState::Flanking);
                }
            }
        }
    }
}

void URaptorCombatComponent::BroadcastPackSignal()
{
    if (!CurrentTarget) return;

    FCombat_PackSignal Signal;
    Signal.TargetLocation = CurrentTarget->GetActorLocation();
    Signal.bPackEngaged = true;
    Signal.PackMembersAlive = 0;

    // Count alive pack members and assign roles
    int32 MemberIndex = 0;
    for (URaptorCombatComponent* Member : PackMembers)
    {
        if (Member && Member->IsAlive())
        {
            Signal.PackMembersAlive++;

            // Assign flanking roles
            ECombat_RaptorRole AssignedRole = ECombat_RaptorRole::Alpha;
            if (MemberIndex == 0) AssignedRole = ECombat_RaptorRole::FlankLeft;
            else if (MemberIndex == 1) AssignedRole = ECombat_RaptorRole::FlankRight;
            else AssignedRole = ECombat_RaptorRole::Ambush;

            Signal.AssignedRole = AssignedRole;
            Member->SetPackRole(AssignedRole);
            Member->ReceivePackSignal(Signal);
            MemberIndex++;
        }
    }
}

float URaptorCombatComponent::TakeDamage_Raptor(float DamageAmount, AActor* DamageCauser)
{
    Stats.CurrentHealth = FMath::Max(0.0f, Stats.CurrentHealth - DamageAmount);

    UE_LOG(LogTemp, Log, TEXT("Raptor [%s] took %.1f damage. HP: %.1f/%.1f"),
        *GetOwner()->GetActorLabel(),
        DamageAmount,
        Stats.CurrentHealth,
        Stats.MaxHealth);

    if (!IsAlive())
    {
        TransitionToState(ECombat_RaptorState::Idle);
        CurrentTarget = nullptr;
        UE_LOG(LogTemp, Log, TEXT("Raptor [%s] has been killed."), *GetOwner()->GetActorLabel());
    }
    else if (DamageCauser && CurrentTarget == nullptr)
    {
        // Aggro on hit
        CurrentTarget = DamageCauser;
        TransitionToState(ECombat_RaptorState::Charging);
    }

    return Stats.CurrentHealth;
}

bool URaptorCombatComponent::IsAlive() const
{
    return Stats.CurrentHealth > 0.0f;
}
