#include "AI/DinosaurCombatAI.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

ADinosaurCombatAI::ADinosaurCombatAI()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz AI tick for performance
}

void ADinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();
    PatrolOrigin = GetActorLocation();
    CurrentHealth = MaxHealth;

    // Auto-configure traits by species
    switch (Species)
    {
    case ECombat_DinoSpecies::Velociraptor:
        InitializePackHunterTraits();
        break;
    case ECombat_DinoSpecies::TyrannosaurusRex:
        InitializeAmbushPredatorTraits();
        break;
    default:
        break;
    }
}

void ADinosaurCombatAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    StateTimer += DeltaTime;

    // Find player each tick if no target
    if (!CurrentTarget)
    {
        ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
        if (PlayerChar && IsPlayerInDetectionRange(PlayerChar))
        {
            DetectPlayer(PlayerChar);
        }
    }

    // State machine dispatch
    switch (CurrentState)
    {
    case ECombat_DinoState::Idle:
        TickIdle(DeltaTime);
        break;
    case ECombat_DinoState::Stalking:
        TickStalking(DeltaTime);
        break;
    case ECombat_DinoState::Flanking:
        TickFlanking(DeltaTime);
        break;
    case ECombat_DinoState::Charging:
        TickCharging(DeltaTime);
        break;
    case ECombat_DinoState::Attacking:
        TickAttacking(DeltaTime);
        break;
    default:
        break;
    }

    if (ShouldFlee())
    {
        SetCombatState(ECombat_DinoState::Retreating);
    }
}

void ADinosaurCombatAI::SetCombatState(ECombat_DinoState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
    StateTimer = 0.0f;
}

void ADinosaurCombatAI::DetectPlayer(AActor* Player)
{
    if (!Player) return;
    CurrentTarget = Player;
    LastKnownTargetLocation = Player->GetActorLocation();

    if (Traits.bIsPackHunter)
    {
        NotifyPackMembers(ECombat_DinoState::Flanking);
        SetCombatState(ECombat_DinoState::Flanking);
    }
    else if (Traits.bIsAmbushPredator)
    {
        SetCombatState(ECombat_DinoState::Stalking);
    }
    else
    {
        SetCombatState(ECombat_DinoState::Charging);
    }
}

void ADinosaurCombatAI::ExecuteFlankingManeuver(AActor* Target)
{
    if (!Target) return;

    // Find this member's index in pack
    int32 MyIndex = 0;
    if (bIsAlpha)
    {
        MyIndex = 0;
    }
    else
    {
        for (int32 i = 0; i < PackMembers.Num(); ++i)
        {
            if (PackMembers[i] == this)
            {
                MyIndex = i + 1;
                break;
            }
        }
    }

    FVector TargetPos = GetFlankingPosition(MyIndex, Target);
    // Move toward flanking position (simplified — real impl uses NavMesh)
    FVector Direction = (TargetPos - GetActorLocation()).GetSafeNormal();
    AddActorWorldOffset(Direction * Traits.MovementSpeedBase * 0.1f, true);
}

void ADinosaurCombatAI::ExecuteAmbushCharge(AActor* Target)
{
    if (!Target) return;
    SetCombatState(ECombat_DinoState::Charging);
    LastKnownTargetLocation = Target->GetActorLocation();
}

float ADinosaurCombatAI::ApplyDamage(float DamageAmount)
{
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);
    if (CurrentHealth <= 0.0f)
    {
        SetCombatState(ECombat_DinoState::Retreating);
    }
    return CurrentHealth;
}

void ADinosaurCombatAI::NotifyPackMembers(ECombat_DinoState AlertState)
{
    for (ADinosaurCombatAI* Member : PackMembers)
    {
        if (Member && Member != this)
        {
            Member->CurrentTarget = CurrentTarget;
            Member->SetCombatState(AlertState);
        }
    }
}

bool ADinosaurCombatAI::IsPlayerInDetectionRange(AActor* Player) const
{
    if (!Player) return false;
    float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    return Dist <= Traits.DetectionRadius;
}

bool ADinosaurCombatAI::IsPlayerInAttackRange(AActor* Player) const
{
    if (!Player) return false;
    float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    return Dist <= Traits.AttackRadius;
}

void ADinosaurCombatAI::InitializePackHunterTraits()
{
    Traits.bIsPackHunter = true;
    Traits.bIsAmbushPredator = false;
    Traits.DetectionRadius = 1200.0f;
    Traits.AttackRadius = 180.0f;
    Traits.MovementSpeedBase = 500.0f;
    Traits.MovementSpeedCharge = 750.0f;
    Traits.AttackDamageBase = 25.0f;
    Traits.FleeHealthThreshold = 0.15f;

    PackFormation.PackSize = 3;
    PackFormation.FlankingAngleDeg = 60.0f;
    PackFormation.bAlphaLeads = true;
}

void ADinosaurCombatAI::InitializeAmbushPredatorTraits()
{
    Traits.bIsPackHunter = false;
    Traits.bIsAmbushPredator = true;
    Traits.DetectionRadius = 2000.0f;
    Traits.AttackRadius = 350.0f;
    Traits.StalkingRadius = 900.0f;
    Traits.MovementSpeedBase = 350.0f;
    Traits.MovementSpeedCharge = 900.0f;
    Traits.AttackDamageBase = 75.0f;
    Traits.FleeHealthThreshold = 0.10f;
}

// --- Private state tick implementations ---

void ADinosaurCombatAI::TickIdle(float DeltaTime)
{
    // Passive patrol — return to origin if wandered
    FVector ToOrigin = PatrolOrigin - GetActorLocation();
    if (ToOrigin.Size() > 500.0f)
    {
        FVector Dir = ToOrigin.GetSafeNormal();
        AddActorWorldOffset(Dir * 150.0f * DeltaTime, true);
    }
}

void ADinosaurCombatAI::TickStalking(float DeltaTime)
{
    if (!CurrentTarget) { SetCombatState(ECombat_DinoState::Idle); return; }

    LastKnownTargetLocation = CurrentTarget->GetActorLocation();
    float Dist = FVector::Dist(GetActorLocation(), LastKnownTargetLocation);

    // Move slowly toward stalk radius
    if (Dist > Traits.StalkingRadius)
    {
        FVector Dir = (LastKnownTargetLocation - GetActorLocation()).GetSafeNormal();
        AddActorWorldOffset(Dir * 100.0f * DeltaTime, true);
    }
    else if (StateTimer > 3.0f)
    {
        // After 3s of stalking, ambush charge
        ExecuteAmbushCharge(CurrentTarget);
    }
}

void ADinosaurCombatAI::TickFlanking(float DeltaTime)
{
    if (!CurrentTarget) { SetCombatState(ECombat_DinoState::Idle); return; }

    ExecuteFlankingManeuver(CurrentTarget);

    if (IsPlayerInAttackRange(CurrentTarget))
    {
        SetCombatState(ECombat_DinoState::Attacking);
    }
}

void ADinosaurCombatAI::TickCharging(float DeltaTime)
{
    if (!CurrentTarget) { SetCombatState(ECombat_DinoState::Idle); return; }

    FVector Dir = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    AddActorWorldOffset(Dir * Traits.MovementSpeedCharge * DeltaTime, true);

    if (IsPlayerInAttackRange(CurrentTarget))
    {
        SetCombatState(ECombat_DinoState::Attacking);
    }
}

void ADinosaurCombatAI::TickAttacking(float DeltaTime)
{
    if (!CurrentTarget) { SetCombatState(ECombat_DinoState::Idle); return; }

    if (!IsPlayerInAttackRange(CurrentTarget))
    {
        SetCombatState(ECombat_DinoState::Charging);
        return;
    }

    // Deal damage every 1.5s
    if (StateTimer >= 1.5f)
    {
        UGameplayStatics::ApplyDamage(
            CurrentTarget,
            Traits.AttackDamageBase,
            nullptr,
            this,
            nullptr
        );
        StateTimer = 0.0f;
    }
}

FVector ADinosaurCombatAI::GetFlankingPosition(int32 MemberIndex, AActor* Target) const
{
    if (!Target) return GetActorLocation();

    FVector TargetLoc = Target->GetActorLocation();
    FVector ForwardToTarget = (TargetLoc - PatrolOrigin).GetSafeNormal();

    float AngleRad = FMath::DegreesToRadians(PackFormation.FlankingAngleDeg * MemberIndex);
    FVector Offset = ForwardToTarget.RotateAngleAxis(AngleRad, FVector::UpVector);
    Offset *= Traits.AttackRadius * 1.5f;

    return TargetLoc + Offset;
}

bool ADinosaurCombatAI::ShouldFlee() const
{
    return (CurrentHealth / MaxHealth) <= Traits.FleeHealthThreshold;
}
