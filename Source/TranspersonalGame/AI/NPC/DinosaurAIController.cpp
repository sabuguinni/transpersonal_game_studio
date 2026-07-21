#include "DinosaurAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// --- Blackboard Key Names ---
const FName ADinosaurAIController::BB_TargetActor      = TEXT("TargetActor");
const FName ADinosaurAIController::BB_PatrolLocation   = TEXT("PatrolLocation");
const FName ADinosaurAIController::BB_HomeLocation     = TEXT("HomeLocation");
const FName ADinosaurAIController::BB_AIState          = TEXT("AIState");
const FName ADinosaurAIController::BB_bPlayerDetected  = TEXT("bPlayerDetected");
const FName ADinosaurAIController::BB_bIsAggressive    = TEXT("bIsAggressive");
const FName ADinosaurAIController::BB_DistanceToTarget = TEXT("DistanceToTarget");

ADinosaurAIController::ADinosaurAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentAIState = ENPC_DinoAIState::Idle;
    PatrolWaitTimer = 0.0f;
    bWaitingAtPatrolPoint = false;

    // Create perception component
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
    SetPerceptionComponent(*PerceptionComponent);

    // Sight config
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius                = 3000.0f;
    SightConfig->LoseSightRadius            = 4500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies    = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals   = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComponent->ConfigureSense(*SightConfig);

    // Hearing config
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies    = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals   = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComponent->ConfigureSense(*HearingConfig);

    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ADinosaurAIController::BeginPlay()
{
    Super::BeginPlay();

    if (PerceptionComponent)
    {
        PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ADinosaurAIController::OnPerceptionUpdated);
    }
}

void ADinosaurAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (InPawn)
    {
        HomeLocation = InPawn->GetActorLocation();
        CurrentPatrolTarget = HomeLocation;
    }

    // Start behavior tree if assigned
    if (BehaviorTree && GetBlackboardComponent())
    {
        RunBehaviorTree(BehaviorTree);
        UpdateBlackboard();
    }
    else if (BehaviorTree)
    {
        UseBlackboard(BehaviorTree->BlackboardAsset, Blackboard);
        RunBehaviorTree(BehaviorTree);
        UpdateBlackboard();
    }

    SetAIState(ENPC_DinoAIState::Patrol);
    ChooseNextPatrolPoint();
}

void ADinosaurAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Patrol wait logic
    if (bWaitingAtPatrolPoint)
    {
        PatrolWaitTimer -= DeltaTime;
        if (PatrolWaitTimer <= 0.0f)
        {
            bWaitingAtPatrolPoint = false;
            ChooseNextPatrolPoint();
        }
        return;
    }

    // State-based movement
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    switch (CurrentAIState)
    {
        case ENPC_DinoAIState::Patrol:
        {
            float DistToPatrol = FVector::Dist(ControlledPawn->GetActorLocation(), CurrentPatrolTarget);
            if (DistToPatrol < 200.0f)
            {
                bWaitingAtPatrolPoint = true;
                PatrolWaitTimer = PatrolWaitTime;
            }
            else
            {
                MoveToLocation(CurrentPatrolTarget, 150.0f);
            }
            break;
        }
        case ENPC_DinoAIState::Chase:
        {
            AActor* Target = nullptr;
            if (GetBlackboardComponent())
            {
                Target = Cast<AActor>(GetBlackboardComponent()->GetValueAsObject(BB_TargetActor));
            }
            if (Target)
            {
                float DistToTarget = FVector::Dist(ControlledPawn->GetActorLocation(), Target->GetActorLocation());
                if (GetBlackboardComponent())
                {
                    GetBlackboardComponent()->SetValueAsFloat(BB_DistanceToTarget, DistToTarget);
                }
                if (DistToTarget <= PerceptionConfig.AttackRadius)
                {
                    SetAIState(ENPC_DinoAIState::Attack);
                }
                else
                {
                    MoveToActor(Target, PerceptionConfig.AttackRadius - 50.0f);
                }
            }
            else
            {
                OnTargetLost();
            }
            break;
        }
        case ENPC_DinoAIState::Attack:
        {
            // Attack logic handled by animation blueprint + damage component
            // Return to chase if target moved away
            AActor* Target = nullptr;
            if (GetBlackboardComponent())
            {
                Target = Cast<AActor>(GetBlackboardComponent()->GetValueAsObject(BB_TargetActor));
            }
            if (Target)
            {
                float Dist = FVector::Dist(ControlledPawn->GetActorLocation(), Target->GetActorLocation());
                if (Dist > PerceptionConfig.AttackRadius * 1.5f)
                {
                    SetAIState(ENPC_DinoAIState::Chase);
                }
            }
            break;
        }
        case ENPC_DinoAIState::Flee:
        {
            // Move away from home — flee toward map edge
            FVector FleeDir = (ControlledPawn->GetActorLocation() - HomeLocation).GetSafeNormal();
            FVector FleeTarget = ControlledPawn->GetActorLocation() + FleeDir * 8000.0f;
            MoveToLocation(FleeTarget, 200.0f);
            break;
        }
        default:
            break;
    }
}

void ADinosaurAIController::SetAIState(ENPC_DinoAIState NewState)
{
    CurrentAIState = NewState;
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsEnum(BB_AIState, static_cast<uint8>(NewState));
        GetBlackboardComponent()->SetValueAsBool(BB_bIsAggressive,
            NewState == ENPC_DinoAIState::Chase || NewState == ENPC_DinoAIState::Attack);
    }
}

void ADinosaurAIController::OnTargetDetected(AActor* Target)
{
    if (!Target) return;

    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsObject(BB_TargetActor, Target);
        GetBlackboardComponent()->SetValueAsBool(BB_bPlayerDetected, true);
    }
    SetAIState(ENPC_DinoAIState::Chase);
}

void ADinosaurAIController::OnTargetLost()
{
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsObject(BB_TargetActor, nullptr);
        GetBlackboardComponent()->SetValueAsBool(BB_bPlayerDetected, false);
    }
    SetAIState(ENPC_DinoAIState::Alert);

    // After alert, return to patrol
    FTimerHandle AlertTimer;
    GetWorld()->GetTimerManager().SetTimer(AlertTimer, [this]()
    {
        if (CurrentAIState == ENPC_DinoAIState::Alert)
        {
            SetAIState(ENPC_DinoAIState::Patrol);
            ChooseNextPatrolPoint();
        }
    }, 5.0f, false);
}

void ADinosaurAIController::ForcePatrol()
{
    SetAIState(ENPC_DinoAIState::Patrol);
    ChooseNextPatrolPoint();
}

void ADinosaurAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor) continue;

        // Check if it's the player
        ACharacter* PlayerChar = Cast<ACharacter>(Actor);
        if (!PlayerChar) continue;

        FActorPerceptionBlueprintInfo PerceptionInfo;
        PerceptionComponent->GetActorsPerception(Actor, PerceptionInfo);

        bool bCurrentlySensed = false;
        for (const FAIStimulus& Stimulus : PerceptionInfo.LastSensedStimuli)
        {
            if (Stimulus.WasSuccessfullySensed())
            {
                bCurrentlySensed = true;
                break;
            }
        }

        if (bCurrentlySensed)
        {
            OnTargetDetected(Actor);
        }
        else if (CurrentAIState == ENPC_DinoAIState::Chase || CurrentAIState == ENPC_DinoAIState::Alert)
        {
            AActor* CurrentTarget = nullptr;
            if (GetBlackboardComponent())
            {
                CurrentTarget = Cast<AActor>(GetBlackboardComponent()->GetValueAsObject(BB_TargetActor));
            }
            if (CurrentTarget == Actor)
            {
                OnTargetLost();
            }
        }
    }
}

void ADinosaurAIController::UpdateBlackboard()
{
    if (!GetBlackboardComponent()) return;

    GetBlackboardComponent()->SetValueAsVector(BB_HomeLocation, HomeLocation);
    GetBlackboardComponent()->SetValueAsVector(BB_PatrolLocation, CurrentPatrolTarget);
    GetBlackboardComponent()->SetValueAsEnum(BB_AIState, static_cast<uint8>(CurrentAIState));
    GetBlackboardComponent()->SetValueAsBool(BB_bPlayerDetected, false);
    GetBlackboardComponent()->SetValueAsBool(BB_bIsAggressive, false);
}

void ADinosaurAIController::ChooseNextPatrolPoint()
{
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys) return;

    FNavLocation NavLoc;
    bool bFound = NavSys->GetRandomReachablePointInRadius(HomeLocation, PatrolRadius, NavLoc);
    if (bFound)
    {
        CurrentPatrolTarget = NavLoc.Location;
        if (GetBlackboardComponent())
        {
            GetBlackboardComponent()->SetValueAsVector(BB_PatrolLocation, CurrentPatrolTarget);
        }
    }
}
