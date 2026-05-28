#include "NPC_DinosaurBehaviorController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ANPC_DinosaurBehaviorController::ANPC_DinosaurBehaviorController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configure AI Sight
    UAISightConfig* SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = SightRange;
        SightConfig->LoseSightRadius = SightRange * 1.2f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        AIPerceptionComponent->ConfigureSense(*SightConfig);
        AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    }

    // Configure AI Hearing
    UAIHearingConfig* HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = HearingRange;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Initialize stats
    DinosaurStats.Health = 100.0f;
    DinosaurStats.Hunger = 50.0f;
    DinosaurStats.Aggression = 30.0f;
    DinosaurStats.Fear = 20.0f;
    DinosaurStats.Energy = 80.0f;

    CurrentTarget = nullptr;
    CurrentThreat = nullptr;
    StateTimer = 0.0f;
    StatsUpdateTimer = 0.0f;
}

void ANPC_DinosaurBehaviorController::BeginPlay()
{
    Super::BeginPlay();

    // Store home location
    if (GetPawn())
    {
        HomeLocation = GetPawn()->GetActorLocation();
    }

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ANPC_DinosaurBehaviorController::OnPerceptionUpdated);
    }

    // Start behavior tree if assigned
    if (BehaviorTree && BlackboardComponent)
    {
        UseBlackboard(BehaviorTree->BlackboardAsset);
        RunBehaviorTree(BehaviorTree);
        
        // Set initial blackboard values
        BlackboardComponent->SetValueAsEnum(TEXT("DinosaurState"), static_cast<uint8>(CurrentState));
        BlackboardComponent->SetValueAsVector(TEXT("HomeLocation"), HomeLocation);
        BlackboardComponent->SetValueAsFloat(TEXT("PatrolRadius"), PatrolRadius);
    }

    // Start in idle state
    SetDinosaurState(ENPC_DinosaurState::Idle);
}

void ANPC_DinosaurBehaviorController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    StateTimer += DeltaTime;
    UpdateStats(DeltaTime);

    // Update blackboard with current stats
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsFloat(TEXT("Health"), DinosaurStats.Health);
        BlackboardComponent->SetValueAsFloat(TEXT("Hunger"), DinosaurStats.Hunger);
        BlackboardComponent->SetValueAsFloat(TEXT("Energy"), DinosaurStats.Energy);
        BlackboardComponent->SetValueAsFloat(TEXT("Fear"), DinosaurStats.Fear);
        BlackboardComponent->SetValueAsFloat(TEXT("Aggression"), DinosaurStats.Aggression);
    }
}

void ANPC_DinosaurBehaviorController::SetDinosaurState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;

        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("DinosaurState"), static_cast<uint8>(CurrentState));
        }

        // Handle state-specific logic
        switch (CurrentState)
        {
            case ENPC_DinosaurState::Patrolling:
                StartPatrolling();
                break;
            case ENPC_DinosaurState::Hunting:
                if (CurrentTarget)
                {
                    StartHunting(CurrentTarget);
                }
                break;
            case ENPC_DinosaurState::Fleeing:
                if (CurrentThreat)
                {
                    StartFleeing(CurrentThreat);
                }
                break;
        }
    }
}

void ANPC_DinosaurBehaviorController::UpdateStats(float DeltaTime)
{
    StatsUpdateTimer += DeltaTime;
    
    if (StatsUpdateTimer >= 1.0f) // Update every second
    {
        // Hunger increases over time
        DinosaurStats.Hunger = FMath::Clamp(DinosaurStats.Hunger + 0.5f, 0.0f, 100.0f);
        
        // Energy decreases based on activity
        float EnergyDrain = 0.2f;
        if (CurrentState == ENPC_DinosaurState::Hunting || CurrentState == ENPC_DinosaurState::Fleeing)
        {
            EnergyDrain = 1.0f;
        }
        else if (CurrentState == ENPC_DinosaurState::Patrolling)
        {
            EnergyDrain = 0.5f;
        }
        
        DinosaurStats.Energy = FMath::Clamp(DinosaurStats.Energy - EnergyDrain, 0.0f, 100.0f);
        
        // Health decreases if hungry or exhausted
        if (DinosaurStats.Hunger > 90.0f || DinosaurStats.Energy < 10.0f)
        {
            DinosaurStats.Health = FMath::Clamp(DinosaurStats.Health - 1.0f, 0.0f, 100.0f);
        }
        
        // Fear decreases over time
        DinosaurStats.Fear = FMath::Clamp(DinosaurStats.Fear - 0.3f, 0.0f, 100.0f);
        
        StatsUpdateTimer = 0.0f;
    }
}

bool ANPC_DinosaurBehaviorController::CanSeeTarget(AActor* Target) const
{
    if (!Target || !AIPerceptionComponent)
    {
        return false;
    }

    FActorPerceptionBlueprintInfo Info;
    return AIPerceptionComponent->GetActorsPerception(Target, Info) && Info.LastSensedStimuli.Num() > 0;
}

void ANPC_DinosaurBehaviorController::StartPatrolling()
{
    if (BlackboardComponent)
    {
        // Set a random patrol point within radius
        FVector RandomDirection = FVector(
            FMath::RandRange(-1.0f, 1.0f),
            FMath::RandRange(-1.0f, 1.0f),
            0.0f
        ).GetSafeNormal();
        
        FVector PatrolPoint = HomeLocation + RandomDirection * FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
        BlackboardComponent->SetValueAsVector(TEXT("PatrolPoint"), PatrolPoint);
    }
}

void ANPC_DinosaurBehaviorController::StartHunting(AActor* Target)
{
    CurrentTarget = Target;
    if (BlackboardComponent && Target)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), Target);
        BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), Target->GetActorLocation());
        
        // Increase aggression when hunting
        DinosaurStats.Aggression = FMath::Clamp(DinosaurStats.Aggression + 20.0f, 0.0f, 100.0f);
    }
}

void ANPC_DinosaurBehaviorController::StartFleeing(AActor* Threat)
{
    CurrentThreat = Threat;
    if (BlackboardComponent && Threat)
    {
        // Calculate flee direction (away from threat)
        FVector ThreatLocation = Threat->GetActorLocation();
        FVector MyLocation = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
        FVector FleeDirection = (MyLocation - ThreatLocation).GetSafeNormal();
        FVector FleeLocation = MyLocation + FleeDirection * FleeDistance;
        
        BlackboardComponent->SetValueAsObject(TEXT("ThreatActor"), Threat);
        BlackboardComponent->SetValueAsVector(TEXT("FleeLocation"), FleeLocation);
        
        // Increase fear when fleeing
        DinosaurStats.Fear = FMath::Clamp(DinosaurStats.Fear + 30.0f, 0.0f, 100.0f);
    }
}

void ANPC_DinosaurBehaviorController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor)
        {
            continue;
        }

        // Check if it's a player character
        if (Actor->IsA<APawn>() && Actor->GetClass()->GetName().Contains(TEXT("Character")))
        {
            float DistanceToActor = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
            
            // Determine reaction based on dinosaur stats and distance
            if (DinosaurStats.Aggression > 60.0f && DistanceToActor < AttackRange * 2.0f)
            {
                // Aggressive dinosaur starts hunting
                SetDinosaurState(ENPC_DinosaurState::Hunting);
                StartHunting(Actor);
            }
            else if (DinosaurStats.Fear > 40.0f || DinosaurStats.Health < 30.0f)
            {
                // Fearful or wounded dinosaur flees
                SetDinosaurState(ENPC_DinosaurState::Fleeing);
                StartFleeing(Actor);
            }
            else if (CurrentState == ENPC_DinosaurState::Idle)
            {
                // Neutral reaction - start patrolling to investigate
                SetDinosaurState(ENPC_DinosaurState::Patrolling);
            }
        }
    }
}