#include "Combat_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

ACombat_AIController::ACombat_AIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Perception Component
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Initialize Sight Configuration
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1500.0f;
    SightConfig->LoseSightRadius = 1600.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Initialize Hearing Configuration
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1200.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure AI Perception
    AIPerceptionComponent->SetDominantSense(*SightConfig->GetSenseImplementation());
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);

    // Initialize Combat Parameters
    AttackRange = 200.0f;
    FleeHealthThreshold = 0.3f;
    CirclingDistance = 400.0f;
    CoordinationRange = 800.0f;
    bUsePackTactics = true;

    // Initialize State
    CurrentState = ECombat_AIState::Idle;
    LastUpdateTime = 0.0f;
    CurrentTarget = nullptr;

    // Initialize Blackboard Keys
    PlayerActorKey = FName("PlayerActor");
    LastKnownLocationKey = FName("LastKnownLocation");
    CombatStateKey = FName("CombatState");
    CanSeePlayerKey = FName("CanSeePlayer");
    ThreatLevelKey = FName("ThreatLevel");
}

void ACombat_AIController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePerception();
    InitializeBlackboard();
    
    // Start behavior tree if assigned
    if (BehaviorTree)
    {
        RunBehaviorTree(BehaviorTree);
    }
}

void ACombat_AIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    
    // Update tactical data every 0.1 seconds
    if (LastUpdateTime >= 0.1f)
    {
        UpdateTacticalData();
        HandleCombatLogic();
        UpdateBlackboardData();
        LastUpdateTime = 0.0f;
    }
}

void ACombat_AIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (Blackboard)
    {
        InitializeBlackboard();
    }
}

void ACombat_AIController::InitializePerception()
{
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombat_AIController::OnPerceptionUpdated);
    }
}

void ACombat_AIController::InitializeBlackboard()
{
    if (Blackboard)
    {
        Blackboard->SetValueAsEnum(CombatStateKey, static_cast<uint8>(CurrentState));
        Blackboard->SetValueAsBool(CanSeePlayerKey, false);
        Blackboard->SetValueAsEnum(ThreatLevelKey, static_cast<uint8>(ECombat_ThreatLevel::None));
    }
}

void ACombat_AIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            // Check if this is the player character
            if (Actor == UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
            {
                ProcessPlayerSighting(Actor);
            }
        }
    }
}

void ACombat_AIController::ProcessPlayerSighting(AActor* PlayerActor)
{
    if (!PlayerActor) return;

    FActorPerceptionBlueprintInfo Info;
    AIPerceptionComponent->GetActorsPerception(PlayerActor, Info);
    
    bool bCanCurrentlySeePlayer = Info.LastSensedStimuli.Num() > 0 && 
                                  Info.LastSensedStimuli[0].WasSuccessfullySensed();

    if (bCanCurrentlySeePlayer)
    {
        TacticalData.LastKnownPlayerLocation = PlayerActor->GetActorLocation();
        TacticalData.TimeSinceLastSighting = 0.0f;
        TacticalData.bCanSeePlayer = true;
        CurrentTarget = PlayerActor;

        // Calculate threat level based on distance
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), PlayerActor->GetActorLocation());
        TacticalData.DistanceToPlayer = Distance;

        if (Distance < AttackRange)
        {
            TacticalData.ThreatLevel = ECombat_ThreatLevel::Critical;
        }
        else if (Distance < AttackRange * 2.0f)
        {
            TacticalData.ThreatLevel = ECombat_ThreatLevel::High;
        }
        else if (Distance < AttackRange * 4.0f)
        {
            TacticalData.ThreatLevel = ECombat_ThreatLevel::Medium;
        }
        else
        {
            TacticalData.ThreatLevel = ECombat_ThreatLevel::Low;
        }
    }
    else
    {
        TacticalData.bCanSeePlayer = false;
        TacticalData.TimeSinceLastSighting += GetWorld()->GetDeltaSeconds();
        
        // Reduce threat level over time when player is not visible
        if (TacticalData.TimeSinceLastSighting > 5.0f)
        {
            TacticalData.ThreatLevel = ECombat_ThreatLevel::Low;
        }
        if (TacticalData.TimeSinceLastSighting > 10.0f)
        {
            TacticalData.ThreatLevel = ECombat_ThreatLevel::None;
            CurrentTarget = nullptr;
        }
    }
}

void ACombat_AIController::UpdateTacticalData()
{
    if (!GetPawn()) return;

    // Update time since last sighting
    if (!TacticalData.bCanSeePlayer)
    {
        TacticalData.TimeSinceLastSighting += 0.1f;
    }

    // Find nearby allies for pack coordination
    FindNearbyAllies();

    // Update combat status
    TacticalData.bIsInCombat = (TacticalData.ThreatLevel >= ECombat_ThreatLevel::Medium && 
                                TacticalData.TimeSinceLastSighting < 5.0f);
}

void ACombat_AIController::HandleCombatLogic()
{
    if (!GetPawn()) return;

    ECombat_AIState NewState = CurrentState;

    switch (CurrentState)
    {
        case ECombat_AIState::Idle:
            if (TacticalData.bCanSeePlayer && TacticalData.ThreatLevel > ECombat_ThreatLevel::None)
            {
                NewState = ECombat_AIState::Investigating;
            }
            break;

        case ECombat_AIState::Investigating:
            if (TacticalData.ThreatLevel >= ECombat_ThreatLevel::High)
            {
                if (ShouldFlee())
                {
                    NewState = ECombat_AIState::Fleeing;
                }
                else if (CanAttackPlayer())
                {
                    NewState = ECombat_AIState::Attacking;
                }
                else
                {
                    NewState = ECombat_AIState::Chasing;
                }
            }
            else if (TacticalData.TimeSinceLastSighting > 8.0f)
            {
                NewState = ECombat_AIState::Idle;
            }
            break;

        case ECombat_AIState::Chasing:
            if (ShouldFlee())
            {
                NewState = ECombat_AIState::Fleeing;
            }
            else if (CanAttackPlayer())
            {
                NewState = ECombat_AIState::Attacking;
            }
            else if (bUsePackTactics && TacticalData.NearbyAllies.Num() > 0)
            {
                NewState = ECombat_AIState::Circling;
            }
            break;

        case ECombat_AIState::Attacking:
            if (ShouldFlee())
            {
                NewState = ECombat_AIState::Fleeing;
            }
            else if (!CanAttackPlayer())
            {
                NewState = ECombat_AIState::Chasing;
            }
            break;

        case ECombat_AIState::Circling:
            if (ShouldFlee())
            {
                NewState = ECombat_AIState::Fleeing;
            }
            else if (CanAttackPlayer())
            {
                NewState = ECombat_AIState::Attacking;
            }
            else if (TacticalData.NearbyAllies.Num() == 0)
            {
                NewState = ECombat_AIState::Chasing;
            }
            break;

        case ECombat_AIState::Fleeing:
            if (TacticalData.ThreatLevel <= ECombat_ThreatLevel::Low)
            {
                NewState = ECombat_AIState::Idle;
            }
            break;
    }

    if (NewState != CurrentState)
    {
        SetCombatState(NewState);
    }
}

void ACombat_AIController::SetCombatState(ECombat_AIState NewState)
{
    CurrentState = NewState;
    
    if (Blackboard)
    {
        Blackboard->SetValueAsEnum(CombatStateKey, static_cast<uint8>(CurrentState));
    }

    // Trigger pack coordination when entering combat states
    if (bUsePackTactics && (NewState == ECombat_AIState::Attacking || NewState == ECombat_AIState::Circling))
    {
        CoordinateWithPack();
    }
}

void ACombat_AIController::UpdateBlackboardData()
{
    if (!Blackboard) return;

    if (CurrentTarget)
    {
        Blackboard->SetValueAsObject(PlayerActorKey, CurrentTarget);
    }
    
    Blackboard->SetValueAsVector(LastKnownLocationKey, TacticalData.LastKnownPlayerLocation);
    Blackboard->SetValueAsBool(CanSeePlayerKey, TacticalData.bCanSeePlayer);
    Blackboard->SetValueAsEnum(ThreatLevelKey, static_cast<uint8>(TacticalData.ThreatLevel));
}

bool ACombat_AIController::CanAttackPlayer() const
{
    return TacticalData.bCanSeePlayer && 
           TacticalData.DistanceToPlayer <= AttackRange &&
           TacticalData.ThreatLevel >= ECombat_ThreatLevel::High;
}

bool ACombat_AIController::ShouldFlee() const
{
    if (!GetPawn()) return false;
    
    // Check health threshold (simplified - would need actual health component)
    // For now, flee if outnumbered and no pack support
    return TacticalData.ThreatLevel == ECombat_ThreatLevel::Critical && 
           TacticalData.NearbyAllies.Num() == 0;
}

FVector ACombat_AIController::GetFlankingPosition() const
{
    if (!GetPawn() || !CurrentTarget) return GetPawn()->GetActorLocation();

    FVector PlayerLocation = CurrentTarget->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector DirectionToPlayer = (PlayerLocation - MyLocation).GetSafeNormal();
    
    // Calculate perpendicular vector for flanking
    FVector FlankDirection = FVector::CrossProduct(DirectionToPlayer, FVector::UpVector).GetSafeNormal();
    
    // Alternate flanking side based on actor ID for variety
    if (GetPawn()->GetUniqueID() % 2 == 0)
    {
        FlankDirection *= -1.0f;
    }
    
    return PlayerLocation + (FlankDirection * CirclingDistance);
}

void ACombat_AIController::CoordinateWithPack()
{
    // Send coordination signals to nearby allies
    for (AActor* Ally : TacticalData.NearbyAllies)
    {
        if (ACombat_AIController* AllyController = Cast<ACombat_AIController>(Ally->GetInstigatorController()))
        {
            // Synchronize attack timing or positioning
            if (AllyController->GetCombatState() == ECombat_AIState::Circling)
            {
                AllyController->SetCombatState(ECombat_AIState::Coordinating);
            }
        }
    }
}

void ACombat_AIController::FindNearbyAllies()
{
    TacticalData.NearbyAllies.Empty();
    
    if (!GetPawn()) return;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor != GetPawn() && Actor->IsA<APawn>())
        {
            float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
            if (Distance <= CoordinationRange)
            {
                // Check if this actor has the same AI controller type (same species)
                if (Actor->GetInstigatorController() && 
                    Actor->GetInstigatorController()->IsA<ACombat_AIController>())
                {
                    TacticalData.NearbyAllies.Add(Actor);
                }
            }
        }
    }
}