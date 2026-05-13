#include "NPC_BehaviorTreeManager.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNPC_BehaviorTreeManager::UNPC_BehaviorTreeManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize components
    BehaviorTreeComponent = nullptr;
    BlackboardComponent = nullptr;
    CurrentBehaviorTree = nullptr;
    bIsPaused = false;
    LastTransitionTime = 0.0f;
    CurrentPriority = ENPC_BehaviorPriority::Normal;
    CurrentBehaviorName = TEXT("");

    // Initialize behavior context
    BehaviorContext = FNPC_BehaviorTreeConfig();
}

void UNPC_BehaviorTreeManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize behavior tree components
    InitializeBehaviorTreeComponent();
    InitializeBlackboardComponent();

    // Auto-start default behavior if configured
    if (bAutoStartDefaultBehavior && !DefaultBehaviorName.IsEmpty())
    {
        ActivateBehaviorByName(DefaultBehaviorName);
    }
}

void UNPC_BehaviorTreeManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update behavior context
    UpdateBehaviorContext(DeltaTime);

    // Update blackboard values from context
    if (BlackboardComponent)
    {
        // Update float values
        for (const auto& FloatPair : BehaviorContext.BlackboardFloats)
        {
            BlackboardComponent->SetValueAsFloat(FName(*FloatPair.Key), FloatPair.Value);
        }

        // Update bool values
        for (const auto& BoolPair : BehaviorContext.BlackboardBools)
        {
            BlackboardComponent->SetValueAsBool(FName(*BoolPair.Key), BoolPair.Value);
        }

        // Update vector values
        for (const auto& VectorPair : BehaviorContext.BlackboardVectors)
        {
            BlackboardComponent->SetValueAsVector(FName(*VectorPair.Key), VectorPair.Value);
        }
    }
}

void UNPC_BehaviorTreeManager::InitializeBehaviorTreeComponent()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Try to get existing behavior tree component from AI controller
    if (APawn* OwnerPawn = Cast<APawn>(Owner))
    {
        if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
        {
            BehaviorTreeComponent = AIController->GetBehaviorTreeComponent();
        }
    }

    // If no existing component, create one
    if (!BehaviorTreeComponent)
    {
        BehaviorTreeComponent = NewObject<UBehaviorTreeComponent>(Owner);
        if (BehaviorTreeComponent)
        {
            BehaviorTreeComponent->RegisterComponent();
        }
    }
}

void UNPC_BehaviorTreeManager::InitializeBlackboardComponent()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Try to get existing blackboard component from AI controller
    if (APawn* OwnerPawn = Cast<APawn>(Owner))
    {
        if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
        {
            BlackboardComponent = AIController->GetBlackboardComponent();
        }
    }

    // If no existing component, create one
    if (!BlackboardComponent)
    {
        BlackboardComponent = NewObject<UBlackboardComponent>(Owner);
        if (BlackboardComponent)
        {
            BlackboardComponent->RegisterComponent();
        }
    }
}

bool UNPC_BehaviorTreeManager::StartBehaviorTree(UBehaviorTree* BehaviorTree)
{
    if (!IsValidBehaviorTree(BehaviorTree) || !BehaviorTreeComponent)
    {
        return false;
    }

    // Stop current behavior tree if running
    if (IsBehaviorTreeRunning())
    {
        StopBehaviorTree();
    }

    // Start new behavior tree
    CurrentBehaviorTree = BehaviorTree;
    BehaviorTreeComponent->StartTree(*BehaviorTree);

    // Initialize blackboard if the behavior tree has one
    if (BehaviorTree->BlackboardAsset && BlackboardComponent)
    {
        BlackboardComponent->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
    }

    return true;
}

void UNPC_BehaviorTreeManager::StopBehaviorTree()
{
    if (BehaviorTreeComponent && IsBehaviorTreeRunning())
    {
        BehaviorTreeComponent->StopTree();
        CurrentBehaviorTree = nullptr;
        CurrentBehaviorName = TEXT("");
    }
}

void UNPC_BehaviorTreeManager::PauseBehaviorTree()
{
    if (BehaviorTreeComponent && IsBehaviorTreeRunning() && !bIsPaused)
    {
        BehaviorTreeComponent->PauseLogic(TEXT("Manual Pause"));
        bIsPaused = true;
    }
}

void UNPC_BehaviorTreeManager::ResumeBehaviorTree()
{
    if (BehaviorTreeComponent && IsBehaviorTreeRunning() && bIsPaused)
    {
        BehaviorTreeComponent->ResumeLogic(TEXT("Manual Resume"));
        bIsPaused = false;
    }
}

bool UNPC_BehaviorTreeManager::SwitchBehaviorTree(UBehaviorTree* NewBehaviorTree, ENPC_BehaviorPriority Priority)
{
    if (!IsValidBehaviorTree(NewBehaviorTree))
    {
        return false;
    }

    // Check if we can switch based on priority
    if (Priority < CurrentPriority && IsBehaviorTreeRunning())
    {
        return false; // Cannot interrupt higher priority behavior
    }

    // Check transition cooldown
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (CurrentTime - LastTransitionTime < StateTransitionCooldown)
    {
        return false;
    }

    // Perform the switch
    bool bSuccess = StartBehaviorTree(NewBehaviorTree);
    if (bSuccess)
    {
        CurrentPriority = Priority;
        LastTransitionTime = CurrentTime;
    }

    return bSuccess;
}

void UNPC_BehaviorTreeManager::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CanTransitionToState(NewState))
    {
        ENPC_BehaviorState OldState = BehaviorContext.CurrentState;
        BehaviorContext.CurrentState = NewState;
        BehaviorContext.LastStateChangeTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        BehaviorContext.StateTimer = 0.0f;

        // Update blackboard with new state
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(NewState));
        }
    }
}

bool UNPC_BehaviorTreeManager::CanTransitionToState(ENPC_BehaviorState NewState) const
{
    // Check transition cooldown
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (CurrentTime - BehaviorContext.LastStateChangeTime < StateTransitionCooldown)
    {
        return false;
    }

    // Validate state transition logic
    return ValidateStateTransition(BehaviorContext.CurrentState, NewState);
}

float UNPC_BehaviorTreeManager::GetTimeInCurrentState() const
{
    return BehaviorContext.StateTimer;
}

void UNPC_BehaviorTreeManager::SetBlackboardValueAsFloat(const FString& KeyName, float Value)
{
    BehaviorContext.BlackboardFloats.Add(KeyName, Value);
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsFloat(FName(*KeyName), Value);
    }
}

void UNPC_BehaviorTreeManager::SetBlackboardValueAsBool(const FString& KeyName, bool Value)
{
    BehaviorContext.BlackboardBools.Add(KeyName, Value);
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(FName(*KeyName), Value);
    }
}

void UNPC_BehaviorTreeManager::SetBlackboardValueAsVector(const FString& KeyName, FVector Value)
{
    BehaviorContext.BlackboardVectors.Add(KeyName, Value);
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(FName(*KeyName), Value);
    }
}

void UNPC_BehaviorTreeManager::SetBlackboardValueAsObject(const FString& KeyName, UObject* Value)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(FName(*KeyName), Value);
    }
}

float UNPC_BehaviorTreeManager::GetBlackboardValueAsFloat(const FString& KeyName) const
{
    if (BehaviorContext.BlackboardFloats.Contains(KeyName))
    {
        return BehaviorContext.BlackboardFloats[KeyName];
    }
    
    if (BlackboardComponent)
    {
        return BlackboardComponent->GetValueAsFloat(FName(*KeyName));
    }
    
    return 0.0f;
}

bool UNPC_BehaviorTreeManager::GetBlackboardValueAsBool(const FString& KeyName) const
{
    if (BehaviorContext.BlackboardBools.Contains(KeyName))
    {
        return BehaviorContext.BlackboardBools[KeyName];
    }
    
    if (BlackboardComponent)
    {
        return BlackboardComponent->GetValueAsBool(FName(*KeyName));
    }
    
    return false;
}

FVector UNPC_BehaviorTreeManager::GetBlackboardValueAsVector(const FString& KeyName) const
{
    if (BehaviorContext.BlackboardVectors.Contains(KeyName))
    {
        return BehaviorContext.BlackboardVectors[KeyName];
    }
    
    if (BlackboardComponent)
    {
        return BlackboardComponent->GetValueAsVector(FName(*KeyName));
    }
    
    return FVector::ZeroVector;
}

void UNPC_BehaviorTreeManager::RegisterBehaviorTree(const FString& BehaviorName, const FNPC_BehaviorTreeConfig& Config)
{
    RegisteredBehaviors.Add(BehaviorName, Config);
}

bool UNPC_BehaviorTreeManager::ActivateBehaviorByName(const FString& BehaviorName)
{
    if (!RegisteredBehaviors.Contains(BehaviorName))
    {
        return false;
    }

    const FNPC_BehaviorTreeConfig& Config = RegisteredBehaviors[BehaviorName];
    UBehaviorTree* BehaviorTree = Config.BehaviorTreeAsset.LoadSynchronous();
    
    if (!BehaviorTree)
    {
        return false;
    }

    bool bSuccess = SwitchBehaviorTree(BehaviorTree, Config.Priority);
    if (bSuccess)
    {
        CurrentBehaviorName = BehaviorName;
    }

    return bSuccess;
}

void UNPC_BehaviorTreeManager::SetBehaviorPriority(const FString& BehaviorName, ENPC_BehaviorPriority Priority)
{
    if (RegisteredBehaviors.Contains(BehaviorName))
    {
        RegisteredBehaviors[BehaviorName].Priority = Priority;
        
        // Update current priority if this is the active behavior
        if (CurrentBehaviorName == BehaviorName)
        {
            CurrentPriority = Priority;
        }
    }
}

void UNPC_BehaviorTreeManager::UpdateBehaviorContext(const FNPC_BehaviorContext& NewContext)
{
    BehaviorContext = NewContext;
}

void UNPC_BehaviorTreeManager::SetTargetActor(AActor* Target)
{
    BehaviorContext.TargetActor = Target;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), Target);
    }
}

void UNPC_BehaviorTreeManager::SetTargetLocation(FVector Location)
{
    BehaviorContext.TargetLocation = Location;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), Location);
    }
}

bool UNPC_BehaviorTreeManager::IsValidBehaviorTree(UBehaviorTree* BehaviorTree) const
{
    return BehaviorTree != nullptr && IsValid(BehaviorTree);
}

bool UNPC_BehaviorTreeManager::IsBehaviorTreeRunning() const
{
    return BehaviorTreeComponent && BehaviorTreeComponent->GetCurrentTree() != nullptr;
}

UBehaviorTree* UNPC_BehaviorTreeManager::GetCurrentBehaviorTree() const
{
    return CurrentBehaviorTree;
}

void UNPC_BehaviorTreeManager::LogBehaviorState() const
{
    UE_LOG(LogTemp, Warning, TEXT("NPC Behavior State: %s, Time in State: %.2f, Current Behavior: %s"),
        *UEnum::GetValueAsString(BehaviorContext.CurrentState),
        BehaviorContext.StateTimer,
        *CurrentBehaviorName);
}

void UNPC_BehaviorTreeManager::UpdateBehaviorContext(float DeltaTime)
{
    // Update state timer
    BehaviorContext.StateTimer += DeltaTime;

    // Update target location if we have a target actor
    if (BehaviorContext.TargetActor && IsValid(BehaviorContext.TargetActor))
    {
        BehaviorContext.TargetLocation = BehaviorContext.TargetActor->GetActorLocation();
    }
}

bool UNPC_BehaviorTreeManager::ValidateStateTransition(ENPC_BehaviorState FromState, ENPC_BehaviorState ToState) const
{
    // Basic state transition validation
    // Critical states like Fleeing can interrupt most other states
    if (ToState == ENPC_BehaviorState::Fleeing)
    {
        return true;
    }

    // Feeding can be interrupted by territorial or hunting behavior
    if (FromState == ENPC_BehaviorState::Feeding)
    {
        return (ToState == ENPC_BehaviorState::Territorial || 
                ToState == ENPC_BehaviorState::Hunting ||
                ToState == ENPC_BehaviorState::Fleeing);
    }

    // Resting can be interrupted by most active behaviors
    if (FromState == ENPC_BehaviorState::Resting)
    {
        return (ToState != ENPC_BehaviorState::Socializing);
    }

    // Default: allow most transitions
    return true;
}

void UNPC_BehaviorTreeManager::OnBehaviorTreeFinished()
{
    // Reset to default behavior when current behavior finishes
    if (!DefaultBehaviorName.IsEmpty() && CurrentBehaviorName != DefaultBehaviorName)
    {
        ActivateBehaviorByName(DefaultBehaviorName);
    }
}