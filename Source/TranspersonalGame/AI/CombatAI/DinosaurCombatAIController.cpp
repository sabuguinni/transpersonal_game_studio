#include "DinosaurCombatAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─────────────────────────────────────────────────────────────────────────────
ADinosaurCombatAIController::ADinosaurCombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Perception component
    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
    SetPerceptionComponent(*PerceptionComp);

    // Sight config
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(8.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComp->ConfigureSense(*SightConfig);

    // Hearing config
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1200.0f;
    HearingConfig->SetMaxAge(5.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    PerceptionComp->ConfigureSense(*HearingConfig);

    PerceptionComp->SetDominantSense(*SightConfig->GetSenseImplementation());

    // Initial state
    CurrentPhase = ECombat_Phase::Idle;
    MyTacticalRole = ECombat_TacticalRole::Alpha;
    CurrentTarget = nullptr;
    AttackCooldownRemaining = 0.0f;
    CirclingAngle = 0.0f;
    PhaseTimer = 0.0f;
    bChargeInitiated = false;
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurCombatAIController::BeginPlay()
{
    Super::BeginPlay();

    if (PerceptionComp)
    {
        PerceptionComp->OnPerceptionUpdated.AddDynamic(
            this, &ADinosaurCombatAIController::OnPerceptionUpdated);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurCombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // Scan for pack members within 3000 units at possession time
    ScanForPackMembers();
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurCombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Cooldown tick
    if (AttackCooldownRemaining > 0.0f)
    {
        AttackCooldownRemaining -= DeltaTime;
    }

    PhaseTimer += DeltaTime;

    TickCombatPhase(DeltaTime);
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurCombatAIController::TickCombatPhase(float DeltaTime)
{
    if (!CurrentTarget || !GetPawn())
    {
        if (CurrentPhase != ECombat_Phase::Idle)
        {
            EnterCombatPhase(ECombat_Phase::Idle);
        }
        return;
    }

    float Dist = GetDistanceToTarget();

    // Retreat check — always highest priority
    if (ShouldRetreat())
    {
        EnterCombatPhase(ECombat_Phase::Retreating);
    }

    switch (CurrentPhase)
    {
    case ECombat_Phase::Stalking:
        // Move toward target at walk speed; switch to charge when close
        MoveToActor(CurrentTarget, CombatStats.AttackRange * 2.0f);
        if (Dist < CombatStats.AttackRange * 2.5f)
        {
            BeginCharge(CurrentTarget);
        }
        break;

    case ECombat_Phase::Charging:
        MoveToActor(CurrentTarget, CombatStats.AttackRange * 0.8f);
        if (Dist <= CombatStats.AttackRange)
        {
            EnterCombatPhase(ECombat_Phase::Attacking);
        }
        break;

    case ECombat_Phase::Attacking:
        ExecuteAttack();
        // After attack, circle if pack support, else charge again
        if (PhaseTimer > CombatStats.AttackCooldown + 0.5f)
        {
            if (HasPackSupport())
            {
                EnterCombatPhase(ECombat_Phase::Circling);
            }
            else
            {
                EnterCombatPhase(ECombat_Phase::Charging);
            }
        }
        break;

    case ECombat_Phase::Circling:
        CircleTarget(CurrentTarget, DeltaTime);
        // After 3 seconds of circling, pick a moment to charge
        if (PhaseTimer > 3.0f)
        {
            EnterCombatPhase(ECombat_Phase::Charging);
        }
        break;

    case ECombat_Phase::Retreating:
    {
        // Move away from target
        FVector AwayDir = (GetPawn()->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
        FVector RetreatDest = GetPawn()->GetActorLocation() + AwayDir * 1500.0f;
        MoveToLocation(RetreatDest);
        // Clear target after retreating
        if (Dist > 3000.0f)
        {
            CurrentTarget = nullptr;
            EnterCombatPhase(ECombat_Phase::Idle);
        }
        break;
    }

    case ECombat_Phase::PackCoordinate:
        // Execute assigned role
        if (ActivePackSignal.bIsActive)
        {
            switch (MyTacticalRole)
            {
            case ECombat_TacticalRole::Flanker:
            {
                // Move to flanking position (90 degrees offset from alpha)
                FVector FlankOffset = FVector(0, CombatStats.CirclingRadius, 0);
                FVector FlankPos = ActivePackSignal.TargetLocation + FlankOffset;
                MoveToLocation(FlankPos);
                break;
            }
            case ECombat_TacticalRole::Ambusher:
                // Hold position, wait for target to move within range
                StopMovement();
                if (Dist <= CombatStats.AttackRange * 1.5f)
                {
                    EnterCombatPhase(ECombat_Phase::Charging);
                }
                break;
            case ECombat_TacticalRole::Distractor:
                // Move toward target aggressively to draw attention
                MoveToActor(CurrentTarget, CombatStats.AttackRange);
                break;
            default:
                // Alpha — direct charge
                BeginCharge(CurrentTarget);
                break;
            }
        }
        break;

    default:
        break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurCombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor) continue;

        // Check if it's the player character
        ACharacter* AsCharacter = Cast<ACharacter>(Actor);
        if (!AsCharacter) continue;

        // Verify it's actually perceived (not just updated to "lost")
        FActorPerceptionBlueprintInfo Info;
        PerceptionComp->GetActorsPerception(Actor, Info);

        bool bCurrentlySensed = false;
        for (const FAIStimulus& Stim : Info.LastSensedStimuli)
        {
            if (Stim.WasSuccessfullySensed())
            {
                bCurrentlySensed = true;
                break;
            }
        }

        if (bCurrentlySensed)
        {
            CurrentTarget = Actor;

            // Pack coordination — if we have pack support, assign roles
            if (CombatStats.bCanPackHunt && HasPackSupport())
            {
                BroadcastPackSignal(ECombat_TacticalRole::Flanker);
                EnterCombatPhase(ECombat_Phase::PackCoordinate);
            }
            else
            {
                EnterCombatPhase(ECombat_Phase::Stalking);
            }
        }
        else
        {
            // Lost sight
            if (CurrentTarget == Actor)
            {
                CurrentTarget = nullptr;
                EnterCombatPhase(ECombat_Phase::Idle);
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurCombatAIController::EnterCombatPhase(ECombat_Phase NewPhase)
{
    if (CurrentPhase == NewPhase) return;

    CurrentPhase = NewPhase;
    PhaseTimer = 0.0f;
    bChargeInitiated = false;

    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    ACharacter* MyChar = Cast<ACharacter>(MyPawn);
    if (!MyChar) return;

    UCharacterMovementComponent* MoveComp = MyChar->GetCharacterMovement();
    if (!MoveComp) return;

    // Adjust movement speed per phase
    switch (NewPhase)
    {
    case ECombat_Phase::Stalking:
        MoveComp->MaxWalkSpeed = 300.0f;
        break;
    case ECombat_Phase::Charging:
        MoveComp->MaxWalkSpeed = CombatStats.ChargeSpeed;
        break;
    case ECombat_Phase::Circling:
        MoveComp->MaxWalkSpeed = 400.0f;
        break;
    case ECombat_Phase::Retreating:
        MoveComp->MaxWalkSpeed = 700.0f;
        break;
    case ECombat_Phase::Idle:
        MoveComp->MaxWalkSpeed = 200.0f;
        break;
    default:
        MoveComp->MaxWalkSpeed = 500.0f;
        break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurCombatAIController::ExecuteAttack()
{
    if (AttackCooldownRemaining > 0.0f) return;
    if (!CurrentTarget || !GetPawn()) return;

    float Dist = GetDistanceToTarget();
    if (Dist > CombatStats.AttackRange) return;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(
        CurrentTarget,
        CombatStats.AttackDamage,
        this,
        GetPawn(),
        UDamageType::StaticClass()
    );

    AttackCooldownRemaining = CombatStats.AttackCooldown;

    // Roar/vocalise — trigger via gameplay tag or notify (stub for audio agent)
    UE_LOG(LogTemp, Log, TEXT("[CombatAI] %s attacked %s for %.1f damage"),
        *GetPawn()->GetName(), *CurrentTarget->GetName(), CombatStats.AttackDamage);
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurCombatAIController::BeginCharge(AActor* Target)
{
    if (!Target || bChargeInitiated) return;

    bChargeInitiated = true;
    EnterCombatPhase(ECombat_Phase::Charging);
    MoveToActor(Target, CombatStats.AttackRange * 0.8f);
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurCombatAIController::CircleTarget(AActor* Target, float DeltaTime)
{
    if (!Target || !GetPawn()) return;

    CirclingAngle += DeltaTime * 60.0f; // 60 deg/sec
    if (CirclingAngle > 360.0f) CirclingAngle -= 360.0f;

    FVector CirclePos = GetCirclingPosition(CirclingAngle);
    MoveToLocation(CirclePos, 50.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
FVector ADinosaurCombatAIController::GetCirclingPosition(float Angle) const
{
    if (!CurrentTarget) return FVector::ZeroVector;

    float RadAngle = FMath::DegreesToRadians(Angle);
    FVector Offset(
        FMath::Cos(RadAngle) * CombatStats.CirclingRadius,
        FMath::Sin(RadAngle) * CombatStats.CirclingRadius,
        0.0f
    );
    return CurrentTarget->GetActorLocation() + Offset;
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurCombatAIController::BroadcastPackSignal(ECombat_TacticalRole RoleForOthers)
{
    if (!CurrentTarget) return;

    FCombat_PackSignal Signal;
    Signal.TargetLocation = CurrentTarget->GetActorLocation();
    Signal.AssignedRole = RoleForOthers;
    Signal.SignalTimestamp = GetWorld()->GetTimeSeconds();
    Signal.bIsActive = true;

    // Broadcast to nearby pack members
    for (ADinosaurCombatAIController* PackMember : NearbyPackMembers)
    {
        if (PackMember && PackMember != this)
        {
            PackMember->ReceivePackSignal(Signal);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurCombatAIController::ReceivePackSignal(const FCombat_PackSignal& Signal)
{
    ActivePackSignal = Signal;

    if (Signal.bIsActive && CurrentTarget)
    {
        MyTacticalRole = Signal.AssignedRole;
        EnterCombatPhase(ECombat_Phase::PackCoordinate);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurCombatAIController::ScanForPackMembers()
{
    NearbyPackMembers.Empty();

    if (!GetPawn()) return;

    TArray<AActor*> NearbyControllers;
    UGameplayStatics::GetAllActorsOfClass(
        GetWorld(), ADinosaurCombatAIController::StaticClass(), NearbyControllers);

    for (AActor* A : NearbyControllers)
    {
        ADinosaurCombatAIController* OtherCtrl = Cast<ADinosaurCombatAIController>(A);
        if (!OtherCtrl || OtherCtrl == this) continue;

        float Dist = FVector::Dist(
            GetPawn()->GetActorLocation(),
            OtherCtrl->GetPawn() ? OtherCtrl->GetPawn()->GetActorLocation() : FVector::ZeroVector
        );

        if (Dist < 3000.0f)
        {
            NearbyPackMembers.Add(OtherCtrl);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
bool ADinosaurCombatAIController::ShouldRetreat() const
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return false;

    // Check health via damage interface (simplified — check float property)
    float CurrentHP = 100.0f;
    // In full implementation, cast to IDamageInterface or check component
    // For now, retreat threshold is evaluated by external systems setting HP

    return (CurrentHP / 100.0f) < CombatStats.RetreatHealthThreshold;
}

// ─────────────────────────────────────────────────────────────────────────────
bool ADinosaurCombatAIController::IsInCombat() const
{
    return CurrentPhase != ECombat_Phase::Idle && CurrentTarget != nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
float ADinosaurCombatAIController::GetDistanceToTarget() const
{
    if (!CurrentTarget || !GetPawn()) return TNumericLimits<float>::Max();
    return FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
}
