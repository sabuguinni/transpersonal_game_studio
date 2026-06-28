#include "DinoAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "NavigationSystem.h"

// ─────────────────────────────────────────────────────────────────────────────
ADinoAIController::ADinoAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Perception setup ──────────────────────────────────────────────────
    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
    SetPerceptionComponent(*PerceptionComp);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius                = 3000.f;
    SightConfig->LoseSightRadius            = 4000.f;
    SightConfig->PeripheralVisionAngleDegrees = 70.f;
    SightConfig->SetMaxAge(10.f);
    SightConfig->DetectionByAffiliation.bDetectEnemies   = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals  = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComp->ConfigureSense(*SightConfig);

    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 2000.f;
    HearingConfig->SetMaxAge(8.f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies   = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals  = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComp->ConfigureSense(*HearingConfig);

    PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
    PerceptionComp->OnPerceptionUpdated.AddDynamic(this, &ADinoAIController::OnPerceptionUpdated);
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinoAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // Apply species defaults
    SetSpeciesProfile(Species);

    // Record patrol origin at spawn location
    if (InPawn)
    {
        PatrolOrigin = InPawn->GetActorLocation();
    }

    // Start behavior tree if assigned
    if (BehaviorTreeAsset)
    {
        RunBehaviorTree(BehaviorTreeAsset);

        // Seed blackboard
        if (UBlackboardComponent* BB = GetBlackboardComponent())
        {
            BB->SetValueAsVector(BB_PATROL_ORIGIN, PatrolOrigin);
            BB->SetValueAsFloat(BB_ALERT_LEVEL, 0.f);
            BB->SetValueAsBool(BB_IS_INVESTIGATING, false);
            BB->SetValueAsBool(BB_CAN_ATTACK, false);
        }
    }

    // Choose first patrol point
    ChooseNextPatrolPoint();

    UE_LOG(LogTemp, Log, TEXT("[DinoAI] Possessed %s — Species=%d PatrolOrigin=%s"),
           *InPawn->GetName(), (int32)Species, *PatrolOrigin.ToString());
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinoAIController::OnUnPossess()
{
    Super::OnUnPossess();
    MemoryLog.Empty();
    AlertLevel = 0.f;
    bIsChasing = false;
    bIsAttacking = false;
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinoAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // ── Alert decay ───────────────────────────────────────────────────────
    if (!bIsChasing)
    {
        TimeSinceLastStimulus += DeltaTime;
        if (TimeSinceLastStimulus > 3.f)
        {
            AlertLevel = FMath::Max(0.f, AlertLevel - PerceptionProfile.AlertDecayRate * DeltaTime);
        }
    }

    // Sync alert level to blackboard
    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->SetValueAsFloat(BB_ALERT_LEVEL, AlertLevel);
    }

    // ── Patrol logic (when no BT assigned) ───────────────────────────────
    if (!BehaviorTreeAsset && !bIsChasing)
    {
        PatrolWaitTimer -= DeltaTime;
        if (PatrolWaitTimer <= 0.f && IsAtDestination())
        {
            ChooseNextPatrolPoint();
        }
    }

    // ── Memory pruning ────────────────────────────────────────────────────
    PruneOldMemories();
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinoAIController::SetSpeciesProfile(ENPC_DinoSpecies InSpecies)
{
    Species = InSpecies;
    PerceptionProfile = GetDefaultProfileForSpecies(InSpecies);
    ApplyPerceptionProfile(PerceptionProfile);
}

// ─────────────────────────────────────────────────────────────────────────────
FNPC_DinoPerceptionProfile ADinoAIController::GetDefaultProfileForSpecies(ENPC_DinoSpecies InSpecies) const
{
    FNPC_DinoPerceptionProfile P;
    switch (InSpecies)
    {
    case ENPC_DinoSpecies::TyrannosaurusRex:
        P.SightRadius              = 4000.f;
        P.LoseSightRadius          = 5500.f;
        P.PeripheralVisionAngle    = 55.f;   // narrow — binocular predator
        P.HearingRadius            = 3000.f;
        P.ChaseSpeed               = 700.f;
        P.PatrolSpeed              = 180.f;
        P.AttackRange              = 450.f;
        P.PatrolRadius             = 8000.f;
        P.AlertDecayRate           = 0.2f;   // slow to calm down
        break;

    case ENPC_DinoSpecies::Velociraptor:
        P.SightRadius              = 2500.f;
        P.LoseSightRadius          = 3500.f;
        P.PeripheralVisionAngle    = 90.f;   // wide — pack hunter
        P.HearingRadius            = 2500.f;
        P.ChaseSpeed               = 900.f;
        P.PatrolSpeed              = 350.f;
        P.AttackRange              = 200.f;
        P.PatrolRadius             = 4000.f;
        P.AlertDecayRate           = 0.3f;
        break;

    case ENPC_DinoSpecies::Brachiosaurus:
        P.SightRadius              = 2000.f;
        P.LoseSightRadius          = 3000.f;
        P.PeripheralVisionAngle    = 120.f;  // wide — herbivore vigilance
        P.HearingRadius            = 1500.f;
        P.ChaseSpeed               = 400.f;  // stampede
        P.PatrolSpeed              = 120.f;
        P.AttackRange              = 600.f;  // tail sweep
        P.PatrolRadius             = 6000.f;
        P.AlertDecayRate           = 0.8f;   // calms quickly
        break;

    case ENPC_DinoSpecies::Triceratops:
        P.SightRadius              = 1800.f;
        P.LoseSightRadius          = 2500.f;
        P.PeripheralVisionAngle    = 100.f;
        P.HearingRadius            = 1200.f;
        P.ChaseSpeed               = 550.f;
        P.PatrolSpeed              = 150.f;
        P.AttackRange              = 350.f;
        P.PatrolRadius             = 3000.f;
        P.AlertDecayRate           = 0.4f;
        break;

    case ENPC_DinoSpecies::Pteranodon:
        P.SightRadius              = 5000.f;
        P.LoseSightRadius          = 7000.f;
        P.PeripheralVisionAngle    = 80.f;
        P.HearingRadius            = 800.f;
        P.ChaseSpeed               = 1200.f;
        P.PatrolSpeed              = 600.f;
        P.AttackRange              = 250.f;
        P.PatrolRadius             = 12000.f;
        P.AlertDecayRate           = 0.6f;
        break;

    case ENPC_DinoSpecies::Ankylosaurus:
        P.SightRadius              = 1500.f;
        P.LoseSightRadius          = 2000.f;
        P.PeripheralVisionAngle    = 110.f;
        P.HearingRadius            = 1000.f;
        P.ChaseSpeed               = 350.f;
        P.PatrolSpeed              = 100.f;
        P.AttackRange              = 400.f;  // tail club
        P.PatrolRadius             = 2000.f;
        P.AlertDecayRate           = 0.5f;
        break;

    case ENPC_DinoSpecies::Spinosaurus:
        P.SightRadius              = 3500.f;
        P.LoseSightRadius          = 5000.f;
        P.PeripheralVisionAngle    = 65.f;
        P.HearingRadius            = 2500.f;
        P.ChaseSpeed               = 750.f;
        P.PatrolSpeed              = 200.f;
        P.AttackRange              = 500.f;
        P.PatrolRadius             = 7000.f;
        P.AlertDecayRate           = 0.25f;
        break;

    default:
        break;
    }
    return P;
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinoAIController::ApplyPerceptionProfile(const FNPC_DinoPerceptionProfile& Profile)
{
    if (!SightConfig || !HearingConfig || !PerceptionComp) return;

    SightConfig->SightRadius                  = Profile.SightRadius;
    SightConfig->LoseSightRadius              = Profile.LoseSightRadius;
    SightConfig->PeripheralVisionAngleDegrees = Profile.PeripheralVisionAngle;
    HearingConfig->HearingRange               = Profile.HearingRadius;

    PerceptionComp->ConfigureSense(*SightConfig);
    PerceptionComp->ConfigureSense(*HearingConfig);
    PerceptionComp->RequestStimuliListenerUpdate();
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinoAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor) continue;

        FActorPerceptionBlueprintInfo Info;
        PerceptionComp->GetActorsPerception(Actor, Info);

        for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
        {
            if (!Stimulus.IsActive()) continue;

            const bool bVisual = (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>());
            const float ThreatScore = bVisual ? 1.0f : 0.5f;

            // Raise alert
            AlertLevel = FMath::Min(1.f, AlertLevel + ThreatScore * 0.6f);
            TimeSinceLastStimulus = 0.f;

            AddMemoryEntry(Stimulus.StimulusLocation, ThreatScore, bVisual);

            if (AlertLevel >= 0.75f)
            {
                OnPlayerDetected(Actor, Stimulus.StimulusLocation, bVisual);
            }
            else
            {
                // Investigate
                if (UBlackboardComponent* BB = GetBlackboardComponent())
                {
                    BB->SetValueAsVector(BB_TARGET_LOCATION, Stimulus.StimulusLocation);
                    BB->SetValueAsBool(BB_IS_INVESTIGATING, true);
                }
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinoAIController::OnPlayerDetected(AActor* Player, const FVector& DetectionLocation, bool bVisual)
{
    bIsChasing = true;

    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->SetValueAsObject(BB_TARGET_ACTOR, Player);
        BB->SetValueAsVector(BB_LAST_KNOWN_POS, DetectionLocation);
        BB->SetValueAsBool(BB_IS_INVESTIGATING, false);
        BB->SetValueAsBool(BB_CAN_ATTACK, true);
    }

    // Apply chase speed to pawn movement
    if (APawn* Pawn = GetPawn())
    {
        if (ACharacter* DinoChar = Cast<ACharacter>(Pawn))
        {
            DinoChar->GetCharacterMovement()->MaxWalkSpeed = PerceptionProfile.ChaseSpeed;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[DinoAI] %s DETECTED PLAYER at %s (visual=%d) AlertLevel=%.2f"),
           *GetName(), *DetectionLocation.ToString(), bVisual, AlertLevel);
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinoAIController::OnPlayerLost()
{
    bIsChasing = false;
    bIsAttacking = false;

    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->SetValueAsObject(BB_TARGET_ACTOR, nullptr);
        BB->SetValueAsBool(BB_CAN_ATTACK, false);
        BB->SetValueAsBool(BB_IS_INVESTIGATING, true);
        // Keep LAST_KNOWN_POS so dino investigates the last seen location
    }

    // Restore patrol speed
    if (APawn* Pawn = GetPawn())
    {
        if (ACharacter* DinoChar = Cast<ACharacter>(Pawn))
        {
            DinoChar->GetCharacterMovement()->MaxWalkSpeed = PerceptionProfile.PatrolSpeed;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[DinoAI] %s LOST PLAYER — returning to investigate"), *GetName());
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinoAIController::AddMemoryEntry(const FVector& Location, float ThreatScore, bool bVisual)
{
    FNPC_DinoMemoryEntry Entry;
    Entry.Location    = Location;
    Entry.Timestamp   = GetWorldTime();
    Entry.ThreatScore = ThreatScore;
    Entry.bWasVisual  = bVisual;

    MemoryLog.Add(Entry);

    // Trim oldest if over limit
    while (MemoryLog.Num() > MaxMemoryEntries)
    {
        MemoryLog.RemoveAt(0);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinoAIController::PruneOldMemories()
{
    const float Now = GetWorldTime();
    MemoryLog.RemoveAll([&](const FNPC_DinoMemoryEntry& E)
    {
        return (Now - E.Timestamp) > MemoryDecaySeconds;
    });
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinoAIController::ChooseNextPatrolPoint()
{
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys) return;

    FNavLocation NavLoc;
    const bool bFound = NavSys->GetRandomReachablePointInRadius(PatrolOrigin, PerceptionProfile.PatrolRadius, NavLoc);
    if (bFound)
    {
        CurrentPatrolTarget = NavLoc.Location;
        MoveToLocation(CurrentPatrolTarget, 100.f);
        PatrolWaitTimer = FMath::RandRange(3.f, 8.f);   // wait 3-8s at each point

        if (UBlackboardComponent* BB = GetBlackboardComponent())
        {
            BB->SetValueAsVector(BB_TARGET_LOCATION, CurrentPatrolTarget);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
bool ADinoAIController::IsAtDestination(float Tolerance) const
{
    if (APawn* Pawn = GetPawn())
    {
        return FVector::Dist2D(Pawn->GetActorLocation(), CurrentPatrolTarget) < Tolerance;
    }
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
float ADinoAIController::GetWorldTime() const
{
    if (UWorld* W = GetWorld())
    {
        return W->GetTimeSeconds();
    }
    return 0.f;
}
