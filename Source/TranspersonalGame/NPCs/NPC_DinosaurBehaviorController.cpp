#include "NPC_DinosaurBehaviorController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

ANPC_DinosaurBehaviorController::ANPC_DinosaurBehaviorController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create AI components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configure sight perception
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = SightRadius;
        SightConfig->LoseSightRadius = LoseSightRadius;
        SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->SetDominantSense(*SightConfig->GetSenseImplementation());
        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Configure hearing perception
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = HearingRange;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ANPC_DinosaurBehaviorController::OnPerceptionUpdated);
    }
}

void ANPC_DinosaurBehaviorController::BeginPlay()
{
    Super::BeginPlay();

    // Set home location to current position
    if (GetPawn())
    {
        HomeLocation = GetPawn()->GetActorLocation();
        CurrentPatrolLocation = HomeLocation;
    }

    // Initialize blackboard values
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(HomeLocationKey, HomeLocation);
        BlackboardComponent->SetValueAsVector(PatrolLocationKey, CurrentPatrolLocation);
        BlackboardComponent->SetValueAsEnum(CurrentStateKey, static_cast<uint8>(CurrentDinosaurState));
        BlackboardComponent->SetValueAsFloat(AlertLevelKey, AlertLevel);
    }
}

void ANPC_DinosaurBehaviorController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update alert level decay
    if (AlertLevel > 0.0f)
    {
        AlertLevel = FMath::Max(0.0f, AlertLevel - DeltaTime * 0.2f); // Decay over 5 seconds
    }

    // Check for target validity
    if (CurrentTarget && !IsValid(CurrentTarget))
    {
        HandleTargetLost();
    }

    // Update blackboard values
    UpdateBlackboardValues();

    // State-specific logic
    switch (CurrentDinosaurState)
    {
        case ENPC_DinosaurState::Patrolling:
        {
            if (GetPawn() && FVector::Dist(GetPawn()->GetActorLocation(), CurrentPatrolLocation) < 200.0f)
            {
                // Reached patrol point, get new one
                CurrentPatrolLocation = GetRandomPatrolLocation();
                if (BlackboardComponent)
                {
                    BlackboardComponent->SetValueAsVector(PatrolLocationKey, CurrentPatrolLocation);
                }
            }
            break;
        }
        case ENPC_DinosaurState::Chasing:
        {
            if (!bHasValidTarget || (CurrentTarget && FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation()) > ChaseRange))
            {
                SetDinosaurState(ENPC_DinosaurState::Searching);
            }
            break;
        }
        case ENPC_DinosaurState::Searching:
        {
            // Return to patrol after searching for a while
            if (GetWorld()->GetTimeSeconds() - StateChangeTime > 10.0f)
            {
                SetDinosaurState(ENPC_DinosaurState::Patrolling);
            }
            break;
        }
    }
}

void ANPC_DinosaurBehaviorController::StartBehaviorTree(UBehaviorTree* BehaviorTree)
{
    if (BehaviorTree && BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StartTree(*BehaviorTree);
    }
}

void ANPC_DinosaurBehaviorController::StopBehaviorTree()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }
}

void ANPC_DinosaurBehaviorController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            // Check if this is the player character
            ACharacter* Character = Cast<ACharacter>(Actor);
            if (Character && Character->IsPlayerControlled())
            {
                FAIStimulus Stimulus;
                if (AIPerceptionComponent->GetActorsPerception(Actor, Stimulus))
                {
                    if (Stimulus.WasSuccessfullySensed())
                    {
                        HandleTargetFound(Actor);
                    }
                    else
                    {
                        HandleTargetLost();
                    }
                }
            }
        }
    }
}

void ANPC_DinosaurBehaviorController::UpdateBlackboardValues()
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TargetActorKey, CurrentTarget);
        BlackboardComponent->SetValueAsVector(PatrolLocationKey, CurrentPatrolLocation);
        BlackboardComponent->SetValueAsVector(HomeLocationKey, HomeLocation);
        BlackboardComponent->SetValueAsEnum(CurrentStateKey, static_cast<uint8>(CurrentDinosaurState));
        BlackboardComponent->SetValueAsFloat(AlertLevelKey, AlertLevel);
    }
}

void ANPC_DinosaurBehaviorController::SetDinosaurState(ENPC_DinosaurState NewState)
{
    if (CurrentDinosaurState != NewState)
    {
        CurrentDinosaurState = NewState;
        StateChangeTime = GetWorld()->GetTimeSeconds();
        
        // State-specific initialization
        switch (NewState)
        {
            case ENPC_DinosaurState::Patrolling:
                CurrentPatrolLocation = GetRandomPatrolLocation();
                break;
            case ENPC_DinosaurState::Idle:
                CurrentTarget = nullptr;
                bHasValidTarget = false;
                break;
        }
    }
}

bool ANPC_DinosaurBehaviorController::IsPlayerInRange(float Range) const
{
    if (!GetPawn()) return false;

    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerCharacter) return false;

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), PlayerCharacter->GetActorLocation());
    return Distance <= Range;
}

FVector ANPC_DinosaurBehaviorController::GetRandomPatrolLocation() const
{
    if (!GetPawn()) return HomeLocation;

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys) return HomeLocation;

    FNavLocation RandomLocation;
    if (NavSys->GetRandomReachablePointInRadius(HomeLocation, PatrolRadius, RandomLocation))
    {
        return RandomLocation.Location;
    }

    return HomeLocation;
}

void ANPC_DinosaurBehaviorController::HandleTargetLost()
{
    CurrentTarget = nullptr;
    bHasValidTarget = false;
    LastTargetSeenTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentDinosaurState == ENPC_DinosaurState::Chasing)
    {
        SetDinosaurState(ENPC_DinosaurState::Searching);
    }
}

void ANPC_DinosaurBehaviorController::HandleTargetFound(AActor* Target)
{
    CurrentTarget = Target;
    bHasValidTarget = true;
    LastTargetSeenTime = GetWorld()->GetTimeSeconds();
    AlertLevel = FMath::Min(1.0f, AlertLevel + 0.5f);

    if (IsPlayerInRange(ChaseRange))
    {
        SetDinosaurState(ENPC_DinosaurState::Chasing);
    }
    else
    {
        SetDinosaurState(ENPC_DinosaurState::Alert);
    }
}