#include "NPCBehaviorTreeController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

ANPCBehaviorTreeController::ANPCBehaviorTreeController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Initialize behavior states
    bIsIdle = true;
    bIsPatrolling = false;
    bIsHunting = false;
    bIsFleeing = false;
    bIsGathering = false;

    CurrentPatrolIndex = 0;
    LastRoutineUpdate = 0.0f;

    // Configure AI Sight
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 1500.0f;
        SightConfig->LoseSightRadius = 1600.0f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Configure AI Hearing
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = 2000.0f;
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ANPCBehaviorTreeController::BeginPlay()
{
    Super::BeginPlay();

    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ANPCBehaviorTreeController::OnPerceptionUpdated);
    }

    // Start daily routine after a short delay
    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, this, &ANPCBehaviorTreeController::StartDailyRoutine, 2.0f, false);
}

void ANPCBehaviorTreeController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BlackboardAsset && BlackboardComponent)
    {
        BlackboardComponent->InitializeBlackboard(*BlackboardAsset);
    }

    if (BehaviorTree && BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StartTree(*BehaviorTree);
    }

    UE_LOG(LogTemp, Warning, TEXT("NPCBehaviorTreeController possessed pawn: %s"), InPawn ? *InPawn->GetName() : TEXT("NULL"));
}

void ANPCBehaviorTreeController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor) continue;

        // Check if it's the player
        if (Actor->IsA<APawn>() && Actor->Tags.Contains("Player"))
        {
            float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
            
            if (Distance < 500.0f)
            {
                // Player is very close - might be a threat
                RespondToThreat(Actor);
            }
            else if (Distance < 1000.0f)
            {
                // Player is nearby - be aware
                if (BlackboardComponent)
                {
                    BlackboardComponent->SetValueAsObject("PlayerActor", Actor);
                    BlackboardComponent->SetValueAsBool("PlayerNearby", true);
                }
            }
        }

        // Check for dinosaurs
        if (Actor->GetName().Contains("TRex") || Actor->GetName().Contains("Raptor"))
        {
            float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
            
            if (Distance < 800.0f)
            {
                // Dinosaur threat - flee immediately
                SetBehaviorState("Fleeing", true);
                SetBehaviorState("Idle", false);
                SetBehaviorState("Patrolling", false);
                
                if (BlackboardComponent)
                {
                    BlackboardComponent->SetValueAsObject("ThreatActor", Actor);
                    BlackboardComponent->SetValueAsBool("InDanger", true);
                }
            }
        }
    }
}

void ANPCBehaviorTreeController::SetBehaviorState(const FString& StateName, bool bValue)
{
    if (StateName == "Idle")
    {
        bIsIdle = bValue;
        if (BlackboardComponent) BlackboardComponent->SetValueAsBool("IsIdle", bValue);
    }
    else if (StateName == "Patrolling")
    {
        bIsPatrolling = bValue;
        if (BlackboardComponent) BlackboardComponent->SetValueAsBool("IsPatrolling", bValue);
    }
    else if (StateName == "Hunting")
    {
        bIsHunting = bValue;
        if (BlackboardComponent) BlackboardComponent->SetValueAsBool("IsHunting", bValue);
    }
    else if (StateName == "Fleeing")
    {
        bIsFleeing = bValue;
        if (BlackboardComponent) BlackboardComponent->SetValueAsBool("IsFleeing", bValue);
    }
    else if (StateName == "Gathering")
    {
        bIsGathering = bValue;
        if (BlackboardComponent) BlackboardComponent->SetValueAsBool("IsGathering", bValue);
    }

    UE_LOG(LogTemp, Log, TEXT("NPC Behavior State Changed: %s = %s"), *StateName, bValue ? TEXT("true") : TEXT("false"));
}

void ANPCBehaviorTreeController::StartDailyRoutine()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Simple day/night cycle behavior
    float DayProgress = FMath::Fmod(CurrentTime / 240.0f, 1.0f); // 4-minute day cycle
    
    if (DayProgress < 0.25f) // Early morning - gathering
    {
        SetBehaviorState("Gathering", true);
        SetBehaviorState("Idle", false);
    }
    else if (DayProgress < 0.75f) // Day - patrolling
    {
        SetBehaviorState("Patrolling", true);
        SetBehaviorState("Gathering", false);
    }
    else // Evening/Night - idle/rest
    {
        SetBehaviorState("Idle", true);
        SetBehaviorState("Patrolling", false);
    }

    LastRoutineUpdate = CurrentTime;
    
    // Schedule next routine update
    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, this, &ANPCBehaviorTreeController::StartDailyRoutine, 60.0f, false);
}

void ANPCBehaviorTreeController::RespondToThreat(AActor* ThreatActor)
{
    if (!ThreatActor || !BlackboardComponent) return;

    // Determine threat level
    bool bIsMajorThreat = ThreatActor->GetName().Contains("TRex") || ThreatActor->GetName().Contains("Raptor");
    
    if (bIsMajorThreat)
    {
        // Major threat - flee immediately
        SetBehaviorState("Fleeing", true);
        SetBehaviorState("Idle", false);
        SetBehaviorState("Patrolling", false);
        SetBehaviorState("Hunting", false);
        
        BlackboardComponent->SetValueAsObject("ThreatActor", ThreatActor);
        BlackboardComponent->SetValueAsBool("InDanger", true);
        
        UE_LOG(LogTemp, Warning, TEXT("NPC fleeing from major threat: %s"), *ThreatActor->GetName());
    }
    else
    {
        // Minor threat - be cautious
        BlackboardComponent->SetValueAsObject("SuspiciousActor", ThreatActor);
        BlackboardComponent->SetValueAsBool("Cautious", true);
        
        UE_LOG(LogTemp, Log, TEXT("NPC being cautious of: %s"), *ThreatActor->GetName());
    }
}

void ANPCBehaviorTreeController::SetPatrolRoute(const TArray<FVector>& NewPatrolPoints)
{
    PatrolPoints = NewPatrolPoints;
    CurrentPatrolIndex = 0;
    
    if (BlackboardComponent && PatrolPoints.Num() > 0)
    {
        BlackboardComponent->SetValueAsVector("PatrolTarget", PatrolPoints[0]);
        BlackboardComponent->SetValueAsBool("HasPatrolRoute", true);
    }
    
    UE_LOG(LogTemp, Log, TEXT("NPC patrol route set with %d points"), PatrolPoints.Num());
}