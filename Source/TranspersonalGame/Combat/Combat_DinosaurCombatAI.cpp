#include "Combat_DinosaurCombatAI.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ACombat_DinosaurCombatAI::ACombat_DinosaurCombatAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Default values
    DetectionRadius = 2000.0f;
    AttackRange = 500.0f;
    ChaseSpeed = 600.0f;
    PatrolSpeed = 200.0f;

    // Initialize perception
    InitializePerception();
}

void ACombat_DinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();
    
    SetupBlackboard();
}

void ACombat_DinosaurCombatAI::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BehaviorTree && BlackboardComponent)
    {
        BlackboardComponent->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
        BehaviorTreeComponent->StartTree(*BehaviorTree);
        
        UE_LOG(LogTemp, Warning, TEXT("Combat AI Controller possessed pawn: %s"), 
               InPawn ? *InPawn->GetName() : TEXT("NULL"));
    }
}

void ACombat_DinosaurCombatAI::InitializePerception()
{
    if (AIPerceptionComponent)
    {
        // Setup sight configuration
        UAISightConfig* SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
        if (SightConfig)
        {
            SightConfig->SightRadius = DetectionRadius;
            SightConfig->LoseSightRadius = DetectionRadius + 500.0f;
            SightConfig->PeripheralVisionAngleDegrees = 90.0f;
            SightConfig->SetMaxAge(5.0f);
            SightConfig->AutoSuccessRangeFromLastSeenLocation = 300.0f;
            SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
            SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
            SightConfig->DetectionByAffiliation.bDetectEnemies = true;

            AIPerceptionComponent->SetDominantSense(*SightConfig->GetSenseImplementation());
            AIPerceptionComponent->ConfigureSense(*SightConfig);
        }

        // Bind perception events
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_DinosaurCombatAI::OnPerceptionUpdated);
    }
}

void ACombat_DinosaurCombatAI::SetupBlackboard()
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("StartLocation"), GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector);
        BlackboardComponent->SetValueAsFloat(TEXT("DetectionRadius"), DetectionRadius);
        BlackboardComponent->SetValueAsFloat(TEXT("AttackRange"), AttackRange);
        BlackboardComponent->SetValueAsFloat(TEXT("ChaseSpeed"), ChaseSpeed);
        BlackboardComponent->SetValueAsFloat(TEXT("PatrolSpeed"), PatrolSpeed);
    }
}

void ACombat_DinosaurCombatAI::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            // Found player character
            SetTarget(Actor);
            StartCombatBehavior();
            break;
        }
    }
}

void ACombat_DinosaurCombatAI::SetTarget(AActor* NewTarget)
{
    if (BlackboardComponent && NewTarget)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), NewTarget);
        BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), NewTarget->GetActorLocation());
        
        UE_LOG(LogTemp, Warning, TEXT("Combat AI set target: %s"), *NewTarget->GetName());
    }
}

AActor* ACombat_DinosaurCombatAI::GetCurrentTarget() const
{
    if (BlackboardComponent)
    {
        return Cast<AActor>(BlackboardComponent->GetValueAsObject(TEXT("TargetActor")));
    }
    return nullptr;
}

bool ACombat_DinosaurCombatAI::IsPlayerInRange(float Range) const
{
    AActor* Target = GetCurrentTarget();
    if (Target && GetPawn())
    {
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
        return Distance <= Range;
    }
    return false;
}

void ACombat_DinosaurCombatAI::StartCombatBehavior()
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("InCombat"), true);
        UE_LOG(LogTemp, Warning, TEXT("Combat AI started combat behavior"));
    }
}

void ACombat_DinosaurCombatAI::StopCombatBehavior()
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("InCombat"), false);
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), nullptr);
        UE_LOG(LogTemp, Warning, TEXT("Combat AI stopped combat behavior"));
    }
}