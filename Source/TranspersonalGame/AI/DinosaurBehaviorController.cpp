#include "DinosaurBehaviorController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"

ADinosaurBehaviorController::ADinosaurBehaviorController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create Behavior Tree Component
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));

    // Create AI Perception Component
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Create Sight Configuration
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 3000.0f;
        SightConfig->LoseSightRadius = 3500.0f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    }

    // Create Hearing Configuration
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = 2000.0f;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    }

    // Initialize default values
    SightRadius = 3000.0f;
    LoseSightRadius = 3500.0f;
    PeripheralVisionAngleDegrees = 90.0f;
    HearingRadius = 2000.0f;
    TerritoryRadius = 5000.0f;
    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    PackLeader = nullptr;

    SetupPerception();
}

void ADinosaurBehaviorController::BeginPlay()
{
    Super::BeginPlay();

    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ADinosaurBehaviorController::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurBehaviorController::OnTargetPerceptionUpdated);
    }
}

void ADinosaurBehaviorController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    SetupBlackboard();
    StartBehaviorTree();
}

void ADinosaurBehaviorController::SetupPerception()
{
    if (AIPerceptionComponent && SightConfig && HearingConfig)
    {
        AIPerceptionComponent->ConfigureSense(*SightConfig);
        AIPerceptionComponent->ConfigureSense(*HearingConfig);
        AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    }
}

void ADinosaurBehaviorController::SetupBlackboard()
{
    if (BlackboardAsset)
    {
        UseBlackboard(BlackboardAsset);
        
        // Set initial blackboard values
        if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
        {
            BlackboardComp->SetValueAsVector(TEXT("TerritoryCenter"), TerritoryCenter);
            BlackboardComp->SetValueAsFloat(TEXT("TerritoryRadius"), TerritoryRadius);
            BlackboardComp->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(CurrentBehaviorState));
        }
    }
}

void ADinosaurBehaviorController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<APawn>())
        {
            // Handle perception updates for pawns
            if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
            {
                // Check if this is a player character
                if (Actor->GetName().Contains(TEXT("Character")) || Actor->GetName().Contains(TEXT("Player")))
                {
                    BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Actor);
                    SetBehaviorState(ENPC_BehaviorState::Alert);
                }
            }
        }
    }
}

void ADinosaurBehaviorController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    if (!BlackboardComp) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Target detected
        BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Actor);
        BlackboardComp->SetValueAsVector(TEXT("LastKnownLocation"), Stimulus.StimulusLocation);
        
        // Determine behavior based on actor type and distance
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
        
        if (Distance < 500.0f)
        {
            SetBehaviorState(ENPC_BehaviorState::Aggressive);
        }
        else if (Distance < 1500.0f)
        {
            SetBehaviorState(ENPC_BehaviorState::Alert);
        }
        else
        {
            SetBehaviorState(ENPC_BehaviorState::Curious);
        }
    }
    else
    {
        // Target lost
        BlackboardComp->ClearValue(TEXT("TargetActor"));
        SetBehaviorState(ENPC_BehaviorState::Searching);
    }
}

void ADinosaurBehaviorController::StartBehaviorTree()
{
    if (BehaviorTreeAsset && BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StartTree(*BehaviorTreeAsset);
    }
}

void ADinosaurBehaviorController::StopBehaviorTree()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }
}

void ADinosaurBehaviorController::SetBehaviorState(ENPC_BehaviorState NewState)
{
    CurrentBehaviorState = NewState;
    
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        BlackboardComp->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(NewState));
    }

    // Notify pack members of state change if this is a pack leader
    if (PackMembers.Num() > 0)
    {
        for (ADinosaurBehaviorController* Member : PackMembers)
        {
            if (Member && Member != this)
            {
                Member->SetBehaviorState(NewState);
            }
        }
    }
}

void ADinosaurBehaviorController::SetTargetActor(AActor* Target)
{
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Target);
    }
}

AActor* ADinosaurBehaviorController::GetTargetActor() const
{
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        return Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("TargetActor")));
    }
    return nullptr;
}

void ADinosaurBehaviorController::SetTerritoryCenter(FVector Center, float Radius)
{
    TerritoryCenter = Center;
    TerritoryRadius = Radius;

    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        BlackboardComp->SetValueAsVector(TEXT("TerritoryCenter"), TerritoryCenter);
        BlackboardComp->SetValueAsFloat(TEXT("TerritoryRadius"), TerritoryRadius);
    }
}

void ADinosaurBehaviorController::SetPackLeader(ADinosaurBehaviorController* Leader)
{
    PackLeader = Leader;
    
    if (Leader && Leader != this)
    {
        Leader->AddPackMember(this);
    }
}

void ADinosaurBehaviorController::AddPackMember(ADinosaurBehaviorController* Member)
{
    if (Member && !PackMembers.Contains(Member))
    {
        PackMembers.Add(Member);
        Member->PackLeader = this;
    }
}

void ADinosaurBehaviorController::RemovePackMember(ADinosaurBehaviorController* Member)
{
    if (Member)
    {
        PackMembers.Remove(Member);
        Member->PackLeader = nullptr;
    }
}