#include "NPC_DinosaurController.h"
#include "NPC_BehaviorTreeManager.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

ANPC_DinosaurController::ANPC_DinosaurController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create AI components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Set default values
    DinosaurSpecies = ENPCSpecies::Raptor;
    SightRadius = 2000.0f;
    HearingRadius = 1500.0f;
    FieldOfView = 90.0f;
    CurrentPatrolIndex = 0;
    
    // Initialize perception
    InitializePerception();
}

void ANPC_DinosaurController::BeginPlay()
{
    Super::BeginPlay();
    
    SetupBehaviorTree();
    
    // Register with behavior manager
    if (UNPC_BehaviorTreeManager* BehaviorManager = GetBehaviorManager())
    {
        BehaviorManager->RegisterAIController(this, DinosaurSpecies);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Controller initialized for species: %d"), static_cast<int32>(DinosaurSpecies));
}

void ANPC_DinosaurController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Unregister from behavior manager
    if (UNPC_BehaviorTreeManager* BehaviorManager = GetBehaviorManager())
    {
        BehaviorManager->UnregisterAIController(this);
    }
    
    Super::EndPlay(EndPlayReason);
}

void ANPC_DinosaurController::Possess(APawn* InPawn)
{
    Super::Possess(InPawn);
    
    if (InPawn)
    {
        UpdateBlackboardValues();
        
        // Start behavior tree if we have one
        if (DefaultBehaviorTree.LoadSynchronous())
        {
            RunBehaviorTree(DefaultBehaviorTree.Get());
        }
    }
}

void ANPC_DinosaurController::UnPossess()
{
    // Stop behavior tree
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }
    
    Super::UnPossess();
}

void ANPC_DinosaurController::InitializePerception()
{
    if (!AIPerceptionComponent)
    {
        return;
    }
    
    // Configure sight
    UAISightConfig* SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = SightRadius * 1.2f;
    SightConfig->PeripheralVisionAngleDegrees = FieldOfView;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Configure hearing
    UAIHearingConfig* HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRadius;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Add configs to perception component
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    
    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ANPC_DinosaurController::OnPerceptionUpdated);
}

void ANPC_DinosaurController::SetupBehaviorTree()
{
    if (BlackboardComponent && DefaultBlackboard.LoadSynchronous())
    {
        UseBlackboard(DefaultBlackboard.Get());
        UpdateBlackboardValues();
    }
}

void ANPC_DinosaurController::UpdateBlackboardValues()
{
    if (!BlackboardComponent || !GetPawn())
    {
        return;
    }
    
    // Set basic values
    BlackboardComponent->SetValueAsVector(TEXT("SelfLocation"), GetPawn()->GetActorLocation());
    BlackboardComponent->SetValueAsEnum(TEXT("Species"), static_cast<uint8>(DinosaurSpecies));
    BlackboardComponent->SetValueAsFloat(TEXT("SightRadius"), SightRadius);
    BlackboardComponent->SetValueAsFloat(TEXT("HearingRadius"), HearingRadius);
    
    // Set patrol points if available
    if (PatrolPoints.Num() > 0)
    {
        BlackboardComponent->SetValueAsVector(TEXT("PatrolTarget"), PatrolPoints[CurrentPatrolIndex]);
    }
}

void ANPC_DinosaurController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    if (!BlackboardComponent)
    {
        return;
    }
    
    // Find the closest perceived actor (likely the player)
    AActor* ClosestActor = nullptr;
    float ClosestDistance = FLT_MAX;
    
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && GetPawn())
        {
            float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                ClosestActor = Actor;
            }
        }
    }
    
    if (ClosestActor)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), ClosestActor);
        BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), ClosestActor->GetActorLocation());
        BlackboardComponent->SetValueAsFloat(TEXT("TargetDistance"), ClosestDistance);
        
        // Update behavior state based on species and distance
        ENPCBehaviorState NewState = ENPCBehaviorState::Alert;
        
        if (DinosaurSpecies == ENPCSpecies::TRex && ClosestDistance < 1500.0f)
        {
            NewState = ENPCBehaviorState::Hunting;
        }
        else if (DinosaurSpecies == ENPCSpecies::Raptor && ClosestDistance < 1000.0f)
        {
            NewState = ENPCBehaviorState::Hunting;
            TriggerPackAlert(ClosestActor->GetActorLocation());
        }
        else if ((DinosaurSpecies == ENPCSpecies::Triceratops || DinosaurSpecies == ENPCSpecies::Brachiosaurus) && ClosestDistance < 800.0f)
        {
            NewState = ENPCBehaviorState::Fleeing;
        }
        
        SetBehaviorState(NewState);
    }
}

void ANPC_DinosaurController::SetDinosaurSpecies(ENPCSpecies NewSpecies)
{
    DinosaurSpecies = NewSpecies;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("Species"), static_cast<uint8>(DinosaurSpecies));
    }
    
    // Re-register with behavior manager
    if (UNPC_BehaviorTreeManager* BehaviorManager = GetBehaviorManager())
    {
        BehaviorManager->UnregisterAIController(this);
        BehaviorManager->RegisterAIController(this, DinosaurSpecies);
    }
}

void ANPC_DinosaurController::SetBehaviorState(ENPCBehaviorState NewState)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(NewState));
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur behavior state changed to: %d"), static_cast<int32>(NewState));
    }
}

ENPCBehaviorState ANPC_DinosaurController::GetBehaviorState() const
{
    if (BlackboardComponent)
    {
        return static_cast<ENPCBehaviorState>(BlackboardComponent->GetValueAsEnum(TEXT("BehaviorState")));
    }
    return ENPCBehaviorState::Idle;
}

void ANPC_DinosaurController::TriggerPackAlert(const FVector& ThreatLocation)
{
    if (UNPC_BehaviorTreeManager* BehaviorManager = GetBehaviorManager())
    {
        BehaviorManager->TriggerPackBehavior(DinosaurSpecies, ThreatLocation, ENPCBehaviorState::Hunting);
    }
}

void ANPC_DinosaurController::SetPatrolRoute(const TArray<FVector>& PatrolPoints)
{
    this->PatrolPoints = PatrolPoints;
    CurrentPatrolIndex = 0;
    
    if (BlackboardComponent && PatrolPoints.Num() > 0)
    {
        BlackboardComponent->SetValueAsVector(TEXT("PatrolTarget"), PatrolPoints[0]);
    }
}

void ANPC_DinosaurController::StartHunting(AActor* Target)
{
    if (Target && BlackboardComponent)
    {
        CurrentTarget = Target;
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), Target);
        BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), Target->GetActorLocation());
        SetBehaviorState(ENPCBehaviorState::Hunting);
    }
}

void ANPC_DinosaurController::FleeFromThreat(const FVector& ThreatLocation)
{
    if (BlackboardComponent && GetPawn())
    {
        // Calculate flee direction (opposite to threat)
        FVector FleeDirection = (GetPawn()->GetActorLocation() - ThreatLocation).GetSafeNormal();
        FVector FleeTarget = GetPawn()->GetActorLocation() + (FleeDirection * 3000.0f);
        
        BlackboardComponent->SetValueAsVector(TEXT("FleeTarget"), FleeTarget);
        BlackboardComponent->SetValueAsVector(TEXT("ThreatLocation"), ThreatLocation);
        SetBehaviorState(ENPCBehaviorState::Fleeing);
    }
}

UNPC_BehaviorTreeManager* ANPC_DinosaurController::GetBehaviorManager() const
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            return GameInstance->GetSubsystem<UNPC_BehaviorTreeManager>();
        }
    }
    return nullptr;
}