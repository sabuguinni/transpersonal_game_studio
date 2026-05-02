#include "DinosaurBehaviorController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Math/UnrealMathUtility.h"

ADinosaurBehaviorController::ADinosaurBehaviorController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create AI Components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

    // Set default values
    SightRadius = 2000.0f;
    LoseSightRadius = 2500.0f;
    PeripheralVisionAngleDegrees = 90.0f;
    HearingRange = 1500.0f;
    PatrolRadius = 1000.0f;
    ChaseRange = 3000.0f;
    
    DinosaurType = EDinosaurType::TRex;
    CurrentBehaviorState = EDinosaurBehaviorState::Idle;
    CurrentTarget = nullptr;
    PatrolCenter = FVector::ZeroVector;
    LastKnownPlayerLocation = FVector::ZeroVector;

    // Pack settings
    PackLeader = nullptr;
    MaxPackSize = 6;
    PackCoordinationRange = 500.0f;
}

void ADinosaurBehaviorController::BeginPlay()
{
    Super::BeginPlay();
    
    SetupPerception();
    SetupBlackboard();
    
    // Set patrol center to current location
    if (GetPawn())
    {
        PatrolCenter = GetPawn()->GetActorLocation();
    }
}

void ADinosaurBehaviorController::Possess(APawn* InPawn)
{
    Super::Possess(InPawn);
    
    if (InPawn)
    {
        PatrolCenter = InPawn->GetActorLocation();
        StartBehaviorTree();
    }
}

void ADinosaurBehaviorController::SetupPerception()
{
    if (!PerceptionComponent)
        return;

    // Setup sight configuration
    UAISightConfig* SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
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

        PerceptionComponent->SetDominantSense(*SightConfig->GetSenseImplementation());
        PerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Setup hearing configuration
    UAIHearingConfig* HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = HearingRange;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

        PerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Bind perception events
    PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ADinosaurBehaviorController::OnPerceptionUpdated);
    PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurBehaviorController::OnTargetPerceptionUpdated);
}

void ADinosaurBehaviorController::SetupBlackboard()
{
    if (BlackboardComponent)
    {
        // Initialize blackboard keys with default values
        BlackboardComponent->SetValueAsEnum(FName("BehaviorState"), static_cast<uint8>(CurrentBehaviorState));
        BlackboardComponent->SetValueAsVector(FName("PatrolCenter"), PatrolCenter);
        BlackboardComponent->SetValueAsFloat(FName("PatrolRadius"), PatrolRadius);
        BlackboardComponent->SetValueAsFloat(FName("ChaseRange"), ChaseRange);
        BlackboardComponent->SetValueAsEnum(FName("DinosaurType"), static_cast<uint8>(DinosaurType));
    }
}

void ADinosaurBehaviorController::StartBehaviorTree()
{
    UBehaviorTree* BehaviorTree = GetBehaviorTreeForType();
    
    if (BehaviorTree && BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StartTree(*BehaviorTree);
        UpdateBlackboardValues();
    }
}

UBehaviorTree* ADinosaurBehaviorController::GetBehaviorTreeForType() const
{
    switch (DinosaurType)
    {
        case EDinosaurType::TRex:
            return TRexBehaviorTree;
        case EDinosaurType::Raptor:
            return RaptorBehaviorTree;
        case EDinosaurType::Herbivore:
            return HerbivoreeBehaviorTree;
        default:
            return TRexBehaviorTree;
    }
}

void ADinosaurBehaviorController::SetBehaviorState(EDinosaurBehaviorState NewState)
{
    CurrentBehaviorState = NewState;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(FName("BehaviorState"), static_cast<uint8>(NewState));
    }
}

void ADinosaurBehaviorController::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(FName("TargetActor"), NewTarget);
        
        if (NewTarget)
        {
            BlackboardComponent->SetValueAsVector(FName("TargetLocation"), NewTarget->GetActorLocation());
            LastKnownPlayerLocation = NewTarget->GetActorLocation();
            BlackboardComponent->SetValueAsVector(FName("LastKnownPlayerLocation"), LastKnownPlayerLocation);
        }
    }
}

void ADinosaurBehaviorController::ClearTarget()
{
    CurrentTarget = nullptr;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->ClearValue(FName("TargetActor"));
        BlackboardComponent->ClearValue(FName("TargetLocation"));
    }
}

bool ADinosaurBehaviorController::IsPlayerInSight() const
{
    if (!CurrentTarget || !GetPawn())
        return false;

    FVector StartLocation = GetPawn()->GetActorLocation();
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    float Distance = FVector::Dist(StartLocation, TargetLocation);

    return Distance <= SightRadius;
}

bool ADinosaurBehaviorController::IsPlayerInChaseRange() const
{
    if (!CurrentTarget || !GetPawn())
        return false;

    FVector StartLocation = GetPawn()->GetActorLocation();
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    float Distance = FVector::Dist(StartLocation, TargetLocation);

    return Distance <= ChaseRange;
}

FVector ADinosaurBehaviorController::GetRandomPatrolPoint() const
{
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f; // Keep on ground level
    RandomDirection.Normalize();
    
    float RandomDistance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    FVector PatrolPoint = PatrolCenter + (RandomDirection * RandomDistance);
    
    return PatrolPoint;
}

void ADinosaurBehaviorController::JoinPack(ADinosaurBehaviorController* NewPackLeader)
{
    if (!NewPackLeader || NewPackLeader == this)
        return;

    // Leave current pack if in one
    LeavePack();

    // Join new pack
    PackLeader = NewPackLeader;
    NewPackLeader->PackMembers.AddUnique(this);
    
    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(FName("PackLeader"), NewPackLeader);
        BlackboardComponent->SetValueAsBool(FName("IsInPack"), true);
    }
}

void ADinosaurBehaviorController::LeavePack()
{
    if (PackLeader)
    {
        PackLeader->PackMembers.Remove(this);
        PackLeader = nullptr;
    }

    // Clear pack members if this was a leader
    for (ADinosaurBehaviorController* Member : PackMembers)
    {
        if (Member && Member->PackLeader == this)
        {
            Member->PackLeader = nullptr;
            if (Member->BlackboardComponent)
            {
                Member->BlackboardComponent->ClearValue(FName("PackLeader"));
                Member->BlackboardComponent->SetValueAsBool(FName("IsInPack"), false);
            }
        }
    }
    PackMembers.Empty();

    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->ClearValue(FName("PackLeader"));
        BlackboardComponent->SetValueAsBool(FName("IsInPack"), false);
    }
}

bool ADinosaurBehaviorController::IsInPack() const
{
    return PackLeader != nullptr || PackMembers.Num() > 0;
}

TArray<ADinosaurBehaviorController*> ADinosaurBehaviorController::GetPackMembers() const
{
    return PackMembers;
}

void ADinosaurBehaviorController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            // Check if this is the player character
            ACharacter* Character = Cast<ACharacter>(Actor);
            if (Character && Character->IsPlayerControlled())
            {
                SetTarget(Actor);
                SetBehaviorState(EDinosaurBehaviorState::Alert);
                break;
            }
        }
    }
}

void ADinosaurBehaviorController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
        return;

    if (Actor->IsA<ACharacter>())
    {
        ACharacter* Character = Cast<ACharacter>(Actor);
        if (Character && Character->IsPlayerControlled())
        {
            if (Stimulus.WasSuccessfullySensed())
            {
                SetTarget(Actor);
                SetBehaviorState(EDinosaurBehaviorState::Hunting);
            }
            else
            {
                // Lost sight of player
                if (CurrentTarget == Actor)
                {
                    SetBehaviorState(EDinosaurBehaviorState::Searching);
                }
            }
        }
    }
}

void ADinosaurBehaviorController::UpdateBlackboardValues()
{
    if (!BlackboardComponent)
        return;

    BlackboardComponent->SetValueAsEnum(FName("BehaviorState"), static_cast<uint8>(CurrentBehaviorState));
    BlackboardComponent->SetValueAsVector(FName("PatrolCenter"), PatrolCenter);
    BlackboardComponent->SetValueAsFloat(FName("PatrolRadius"), PatrolRadius);
    BlackboardComponent->SetValueAsFloat(FName("ChaseRange"), ChaseRange);
    BlackboardComponent->SetValueAsEnum(FName("DinosaurType"), static_cast<uint8>(DinosaurType));
    BlackboardComponent->SetValueAsBool(FName("IsInPack"), IsInPack());
    
    if (CurrentTarget)
    {
        BlackboardComponent->SetValueAsObject(FName("TargetActor"), CurrentTarget);
        BlackboardComponent->SetValueAsVector(FName("TargetLocation"), CurrentTarget->GetActorLocation());
    }
}