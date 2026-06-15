#include "NPCBehaviorTreeManager.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"

UNPC_BehaviorTreeManager::UNPC_BehaviorTreeManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize defaults
    MaxMemories = 50;
    MemoryDecayRate = 0.1f;
    SocialUpdateInterval = 2.0f;
    CurrentRoutineIndex = 0;
    RoutineStartTime = 0.0f;
    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    PreviousBehaviorState = ENPC_BehaviorState::Idle;

    // Component references will be set in BeginPlay
    BehaviorTreeComponent = nullptr;
    BlackboardComponent = nullptr;
    DefaultBehaviorTree = nullptr;
    CurrentBehaviorTree = nullptr;
}

void UNPC_BehaviorTreeManager::BeginPlay()
{
    Super::BeginPlay();

    // Get AI Controller and components
    AAIController* AIController = GetOwnerAIController();
    if (AIController)
    {
        BehaviorTreeComponent = AIController->GetBehaviorTreeComponent();
        BlackboardComponent = AIController->GetBlackboardComponent();

        // Initialize behavior tree if we have a default one
        if (DefaultBehaviorTree)
        {
            InitializeBehaviorTree(DefaultBehaviorTree);
        }
    }

    // Set up timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(MemoryCleanupTimer, this, &UNPC_BehaviorTreeManager::CleanupExpiredMemories, 30.0f, true);
        World->GetTimerManager().SetTimer(SocialUpdateTimer, [this]() { UpdateSocialSystem(SocialUpdateInterval); }, SocialUpdateInterval, true);
    }

    // Initialize routine start time
    RoutineStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

void UNPC_BehaviorTreeManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateMemorySystem(DeltaTime);
    UpdateRoutineSystem(DeltaTime);
}

void UNPC_BehaviorTreeManager::InitializeBehaviorTree(UBehaviorTree* NewBehaviorTree)
{
    if (!NewBehaviorTree)
    {
        return;
    }

    CurrentBehaviorTree = NewBehaviorTree;
    
    AAIController* AIController = GetOwnerAIController();
    if (AIController && BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StartTree(*NewBehaviorTree);
    }
}

void UNPC_BehaviorTreeManager::SwitchBehaviorTree(UBehaviorTree* NewBehaviorTree, bool bForceRestart)
{
    if (!NewBehaviorTree || NewBehaviorTree == CurrentBehaviorTree)
    {
        return;
    }

    if (BehaviorTreeComponent)
    {
        if (bForceRestart)
        {
            BehaviorTreeComponent->StopTree();
        }
        
        CurrentBehaviorTree = NewBehaviorTree;
        BehaviorTreeComponent->StartTree(*NewBehaviorTree);
    }
}

void UNPC_BehaviorTreeManager::PauseBehaviorTree()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->PauseLogic(TEXT("NPCBehaviorManager"));
    }
}

void UNPC_BehaviorTreeManager::ResumeBehaviorTree()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->ResumeLogic(TEXT("NPCBehaviorManager"));
    }
}

void UNPC_BehaviorTreeManager::AddMemory(FVector Location, ENPC_MemoryType MemoryType, float Intensity)
{
    FNPC_MemoryEntry NewMemory;
    NewMemory.Location = Location;
    NewMemory.MemoryType = MemoryType;
    NewMemory.Intensity = Intensity;
    NewMemory.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    Memories.Add(NewMemory);

    // Remove oldest memories if we exceed the limit
    if (Memories.Num() > MaxMemories)
    {
        Memories.RemoveAt(0);
    }
}

TArray<FNPC_MemoryEntry> UNPC_BehaviorTreeManager::GetMemoriesInRadius(FVector Center, float Radius)
{
    TArray<FNPC_MemoryEntry> NearbyMemories;
    
    for (const FNPC_MemoryEntry& Memory : Memories)
    {
        float Distance = FVector::Dist(Memory.Location, Center);
        if (Distance <= Radius)
        {
            NearbyMemories.Add(Memory);
        }
    }

    return NearbyMemories;
}

FNPC_MemoryEntry UNPC_BehaviorTreeManager::GetStrongestMemory(ENPC_MemoryType MemoryType)
{
    FNPC_MemoryEntry StrongestMemory;
    float HighestIntensity = 0.0f;

    for (const FNPC_MemoryEntry& Memory : Memories)
    {
        if (Memory.MemoryType == MemoryType && Memory.Intensity > HighestIntensity)
        {
            StrongestMemory = Memory;
            HighestIntensity = Memory.Intensity;
        }
    }

    return StrongestMemory;
}

void UNPC_BehaviorTreeManager::ClearOldMemories(float MaxAge)
{
    if (!GetWorld())
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    Memories.RemoveAll([CurrentTime, MaxAge](const FNPC_MemoryEntry& Memory)
    {
        return (CurrentTime - Memory.Timestamp) > MaxAge;
    });
}

void UNPC_BehaviorTreeManager::UpdateRelationship(APawn* TargetPawn, float DeltaValue, ENPC_RelationType NewRelationType)
{
    if (!TargetPawn)
    {
        return;
    }

    // Find existing relationship
    FNPC_SocialRelation* ExistingRelation = SocialRelations.FindByPredicate([TargetPawn](const FNPC_SocialRelation& Relation)
    {
        return Relation.TargetPawn.Get() == TargetPawn;
    });

    if (ExistingRelation)
    {
        ExistingRelation->RelationshipValue += DeltaValue;
        ExistingRelation->RelationType = NewRelationType;
        ExistingRelation->LastInteractionTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    }
    else
    {
        // Create new relationship
        FNPC_SocialRelation NewRelation;
        NewRelation.TargetPawn = TargetPawn;
        NewRelation.RelationshipValue = DeltaValue;
        NewRelation.RelationType = NewRelationType;
        NewRelation.LastInteractionTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        
        SocialRelations.Add(NewRelation);
    }
}

FNPC_SocialRelation UNPC_BehaviorTreeManager::GetRelationship(APawn* TargetPawn)
{
    if (!TargetPawn)
    {
        return FNPC_SocialRelation();
    }

    const FNPC_SocialRelation* FoundRelation = SocialRelations.FindByPredicate([TargetPawn](const FNPC_SocialRelation& Relation)
    {
        return Relation.TargetPawn.Get() == TargetPawn;
    });

    return FoundRelation ? *FoundRelation : FNPC_SocialRelation();
}

TArray<FNPC_SocialRelation> UNPC_BehaviorTreeManager::GetAllRelationships()
{
    // Clean up null references
    SocialRelations.RemoveAll([](const FNPC_SocialRelation& Relation)
    {
        return !Relation.TargetPawn.IsValid();
    });

    return SocialRelations;
}

void UNPC_BehaviorTreeManager::ProcessSocialInteraction(APawn* TargetPawn, ENPC_InteractionType InteractionType)
{
    if (!TargetPawn)
    {
        return;
    }

    float RelationshipDelta = 0.0f;
    ENPC_RelationType NewRelationType = ENPC_RelationType::Neutral;

    switch (InteractionType)
    {
        case ENPC_InteractionType::Friendly:
            RelationshipDelta = 0.1f;
            NewRelationType = ENPC_RelationType::Friendly;
            break;
        case ENPC_InteractionType::Hostile:
            RelationshipDelta = -0.2f;
            NewRelationType = ENPC_RelationType::Hostile;
            break;
        case ENPC_InteractionType::Trade:
            RelationshipDelta = 0.05f;
            NewRelationType = ENPC_RelationType::Neutral;
            break;
        case ENPC_InteractionType::Help:
            RelationshipDelta = 0.15f;
            NewRelationType = ENPC_RelationType::Friendly;
            break;
        default:
            break;
    }

    UpdateRelationship(TargetPawn, RelationshipDelta, NewRelationType);
}

void UNPC_BehaviorTreeManager::SetDailyRoutine(const TArray<FNPC_RoutineTask>& NewRoutine)
{
    DailyRoutine = NewRoutine;
    CurrentRoutineIndex = 0;
    RoutineStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

FNPC_RoutineTask UNPC_BehaviorTreeManager::GetCurrentRoutineTask()
{
    if (DailyRoutine.IsValidIndex(CurrentRoutineIndex))
    {
        return DailyRoutine[CurrentRoutineIndex];
    }
    
    return FNPC_RoutineTask();
}

void UNPC_BehaviorTreeManager::AdvanceToNextRoutineTask()
{
    if (DailyRoutine.Num() > 0)
    {
        CurrentRoutineIndex = (CurrentRoutineIndex + 1) % DailyRoutine.Num();
        RoutineStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    }
}

void UNPC_BehaviorTreeManager::SetBlackboardValue(FName KeyName, float Value)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsFloat(KeyName, Value);
    }
}

void UNPC_BehaviorTreeManager::SetBlackboardVector(FName KeyName, FVector Value)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(KeyName, Value);
    }
}

void UNPC_BehaviorTreeManager::SetBlackboardObject(FName KeyName, UObject* Value)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(KeyName, Value);
    }
}

float UNPC_BehaviorTreeManager::GetBlackboardValue(FName KeyName)
{
    if (BlackboardComponent)
    {
        return BlackboardComponent->GetValueAsFloat(KeyName);
    }
    
    return 0.0f;
}

void UNPC_BehaviorTreeManager::SetNPCState(ENPC_BehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        PreviousBehaviorState = CurrentBehaviorState;
        CurrentBehaviorState = NewState;

        // Update blackboard with new state
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(NewState));
        }
    }
}

void UNPC_BehaviorTreeManager::UpdateMemorySystem(float DeltaTime)
{
    DecayMemoryIntensity(DeltaTime);
}

void UNPC_BehaviorTreeManager::UpdateSocialSystem(float DeltaTime)
{
    // Clean up invalid relationships
    SocialRelations.RemoveAll([](const FNPC_SocialRelation& Relation)
    {
        return !Relation.TargetPawn.IsValid();
    });

    // Decay relationships over time
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    for (FNPC_SocialRelation& Relation : SocialRelations)
    {
        float TimeSinceInteraction = CurrentTime - Relation.LastInteractionTime;
        if (TimeSinceInteraction > 60.0f) // 1 minute
        {
            float DecayAmount = DeltaTime * 0.01f; // Slow decay
            if (Relation.RelationshipValue > 0.0f)
            {
                Relation.RelationshipValue = FMath::Max(0.0f, Relation.RelationshipValue - DecayAmount);
            }
            else if (Relation.RelationshipValue < 0.0f)
            {
                Relation.RelationshipValue = FMath::Min(0.0f, Relation.RelationshipValue + DecayAmount);
            }
        }
    }
}

void UNPC_BehaviorTreeManager::UpdateRoutineSystem(float DeltaTime)
{
    if (DailyRoutine.Num() == 0)
    {
        return;
    }

    FNPC_RoutineTask CurrentTask = GetCurrentRoutineTask();
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    float TaskDuration = CurrentTime - RoutineStartTime;

    // Check if current task should be completed
    if (TaskDuration >= CurrentTask.Duration)
    {
        AdvanceToNextRoutineTask();
    }

    // Update blackboard with current routine information
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("RoutineLocation"), CurrentTask.Location);
        BlackboardComponent->SetValueAsEnum(TEXT("RoutineActivity"), static_cast<uint8>(CurrentTask.Activity));
    }
}

void UNPC_BehaviorTreeManager::CleanupExpiredMemories()
{
    ClearOldMemories(300.0f); // Clear memories older than 5 minutes
}

void UNPC_BehaviorTreeManager::DecayMemoryIntensity(float DeltaTime)
{
    for (FNPC_MemoryEntry& Memory : Memories)
    {
        Memory.Intensity = FMath::Max(0.1f, Memory.Intensity - (MemoryDecayRate * DeltaTime));
    }
}

AAIController* UNPC_BehaviorTreeManager::GetOwnerAIController()
{
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        return Cast<AAIController>(OwnerPawn->GetController());
    }
    
    return nullptr;
}