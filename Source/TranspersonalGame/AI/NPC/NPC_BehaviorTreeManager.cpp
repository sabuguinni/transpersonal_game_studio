#include "NPC_BehaviorTreeManager.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UNPC_BehaviorTreeManager::UNPC_BehaviorTreeManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    CurrentTarget = nullptr;
    CurrentPatrolIndex = 0;
    StateTimer = 0.0f;
    AIController = nullptr;
    BehaviorTreeComponent = nullptr;
    BlackboardComponent = nullptr;
}

void UNPC_BehaviorTreeManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAIComponents();
    
    // Start default behavior tree if available
    if (DefaultBehaviorTree && AIController)
    {
        StartBehaviorTree(DefaultBehaviorTree);
    }
    
    // Set initial blackboard values
    if (BlackboardComponent)
    {
        SetBlackboardValue(TEXT("PatrolRadius"), BehaviorConfig.PatrolRadius);
        SetBlackboardValue(TEXT("ChaseDistance"), BehaviorConfig.ChaseDistance);
        SetBlackboardValue(TEXT("AttackRange"), BehaviorConfig.AttackRange);
        SetBlackboardValue(TEXT("PatrolSpeed"), BehaviorConfig.PatrolSpeed);
        SetBlackboardValue(TEXT("ChaseSpeed"), BehaviorConfig.ChaseSpeed);
    }
}

void UNPC_BehaviorTreeManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    StateTimer += DeltaTime;
    UpdateMemory(DeltaTime);
    UpdateBehaviorState(DeltaTime);
}

void UNPC_BehaviorTreeManager::InitializeAIComponents()
{
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        AIController = Cast<AAIController>(OwnerPawn->GetController());
        
        if (AIController)
        {
            BehaviorTreeComponent = AIController->FindComponentByClass<UBehaviorTreeComponent>();
            BlackboardComponent = AIController->FindComponentByClass<UBlackboardComponent>();
            
            if (!BehaviorTreeComponent)
            {
                BehaviorTreeComponent = AIController->GetBrainComponent();
            }
        }
    }
}

void UNPC_BehaviorTreeManager::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        StateTimer = 0.0f;
        
        // Update blackboard with new state
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("BehaviorState"), (uint8)NewState);
        }
        
        // Log state change for debugging
        UE_LOG(LogTemp, Log, TEXT("NPC %s changed behavior state to %d"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), (int32)NewState);
    }
}

void UNPC_BehaviorTreeManager::StartBehaviorTree(UBehaviorTree* BehaviorTreeAsset)
{
    if (!BehaviorTreeAsset || !AIController)
    {
        return;
    }
    
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StartTree(*BehaviorTreeAsset);
    }
    else
    {
        AIController->RunBehaviorTree(BehaviorTreeAsset);
    }
}

void UNPC_BehaviorTreeManager::StopBehaviorTree()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }
}

void UNPC_BehaviorTreeManager::SetBlackboardValue(const FName& KeyName, float Value)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsFloat(KeyName, Value);
    }
}

void UNPC_BehaviorTreeManager::SetBlackboardVector(const FName& KeyName, const FVector& Value)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(KeyName, Value);
    }
}

void UNPC_BehaviorTreeManager::SetBlackboardObject(const FName& KeyName, UObject* Value)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(KeyName, Value);
    }
}

float UNPC_BehaviorTreeManager::GetBlackboardValue(const FName& KeyName) const
{
    if (BlackboardComponent)
    {
        return BlackboardComponent->GetValueAsFloat(KeyName);
    }
    return 0.0f;
}

void UNPC_BehaviorTreeManager::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), NewTarget);
        
        if (NewTarget)
        {
            BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), NewTarget->GetActorLocation());
        }
    }
}

void UNPC_BehaviorTreeManager::ClearTarget()
{
    SetTarget(nullptr);
}

void UNPC_BehaviorTreeManager::SetPatrolPoints(const TArray<FVector>& Points)
{
    PatrolPoints = Points;
    CurrentPatrolIndex = 0;
    
    if (PatrolPoints.Num() > 0 && BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("PatrolPoint"), PatrolPoints[0]);
    }
}

FVector UNPC_BehaviorTreeManager::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    }
    
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    FVector NextPoint = PatrolPoints[CurrentPatrolIndex];
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("PatrolPoint"), NextPoint);
    }
    
    return NextPoint;
}

void UNPC_BehaviorTreeManager::AddPatrolPoint(const FVector& Point)
{
    PatrolPoints.Add(Point);
}

void UNPC_BehaviorTreeManager::RememberLocation(const FVector& Location, float Duration)
{
    LocationMemory.Add(Location, GetWorld()->GetTimeSeconds() + Duration);
}

void UNPC_BehaviorTreeManager::RememberActor(AActor* Actor, float Duration)
{
    if (Actor)
    {
        ActorMemory.Add(Actor, GetWorld()->GetTimeSeconds() + Duration);
    }
}

bool UNPC_BehaviorTreeManager::HasMemoryOf(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }
    
    const float* ExpiryTime = ActorMemory.Find(Actor);
    if (ExpiryTime && *ExpiryTime > GetWorld()->GetTimeSeconds())
    {
        return true;
    }
    
    return false;
}

void UNPC_BehaviorTreeManager::UpdateMemory(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Clean up expired actor memories
    for (auto It = ActorMemory.CreateIterator(); It; ++It)
    {
        if (It->Value < CurrentTime)
        {
            It.RemoveCurrent();
        }
    }
    
    // Clean up expired location memories
    for (auto It = LocationMemory.CreateIterator(); It; ++It)
    {
        if (It->Value < CurrentTime)
        {
            It.RemoveCurrent();
        }
    }
}

void UNPC_BehaviorTreeManager::UpdateBehaviorState(float DeltaTime)
{
    // Update blackboard with current state timer
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsFloat(TEXT("StateTimer"), StateTimer);
    }
    
    // Handle state-specific logic
    switch (CurrentBehaviorState)
    {
        case ENPC_BehaviorState::Idle:
            if (StateTimer > BehaviorConfig.IdleDuration)
            {
                SetBehaviorState(ENPC_BehaviorState::Patrol);
            }
            break;
            
        case ENPC_BehaviorState::Patrol:
            // Patrol logic handled by behavior tree
            break;
            
        case ENPC_BehaviorState::Chase:
            // Validate target still exists and is in range
            if (!CurrentTarget || !IsValid(CurrentTarget))
            {
                ClearTarget();
                SetBehaviorState(ENPC_BehaviorState::Patrol);
            }
            break;
            
        case ENPC_BehaviorState::Attack:
            // Attack logic handled by behavior tree
            break;
            
        case ENPC_BehaviorState::Flee:
            // Flee for a minimum duration
            if (StateTimer > 5.0f)
            {
                SetBehaviorState(ENPC_BehaviorState::Patrol);
            }
            break;
            
        default:
            break;
    }
}