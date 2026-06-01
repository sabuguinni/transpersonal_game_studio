#include "NPC_DinosaurBehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

// Blackboard key constants
const FString UNPC_DinosaurBehaviorTree::BB_CurrentState = TEXT("CurrentState");
const FString UNPC_DinosaurBehaviorTree::BB_TargetLocation = TEXT("TargetLocation");
const FString UNPC_DinosaurBehaviorTree::BB_TargetActor = TEXT("TargetActor");
const FString UNPC_DinosaurBehaviorTree::BB_ThreatLevel = TEXT("ThreatLevel");
const FString UNPC_DinosaurBehaviorTree::BB_PackLeader = TEXT("PackLeader");
const FString UNPC_DinosaurBehaviorTree::BB_IsInPack = TEXT("IsInPack");

UNPC_DinosaurBehaviorTree::UNPC_DinosaurBehaviorTree()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize default values
    CurrentState = ENPCBehaviorState::Idle;
    PreviousState = ENPCBehaviorState::Idle;
    StateTimer = 0.0f;
    StateTransitionDelay = 0.5f;

    // Pack behavior defaults
    PackLeader = nullptr;
    bIsPackLeader = false;
    MaxPackSize = 6; // Typical velociraptor pack size

    // Threat detection defaults
    ThreatDetectionRadius = 2000.0f; // 20 meters
    ThreatDecayRate = 10.0f; // Threat reduces by 10 per second

    // Behavior parameters
    PatrolRadius = 5000.0f; // 50 meters patrol radius
    HuntRange = 3000.0f; // 30 meters hunt range
    FleeDistance = 1500.0f; // 15 meters flee distance
    AggressionLevel = 50.0f; // Medium aggression
    FearLevel = 30.0f; // Low fear

    // Initialize components
    BehaviorTreeComponent = nullptr;
    BlackboardComponent = nullptr;
    BehaviorTreeAsset = nullptr;
}

void UNPC_DinosaurBehaviorTree::BeginPlay()
{
    Super::BeginPlay();

    // Get AI Controller and initialize behavior tree
    if (AActor* Owner = GetOwner())
    {
        if (APawn* OwnerPawn = Cast<APawn>(Owner))
        {
            if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
            {
                BehaviorTreeComponent = AIController->GetBehaviorTreeComponent();
                BlackboardComponent = AIController->GetBlackboardComponent();
            }
        }
    }

    InitializeBehaviorTree();
}

void UNPC_DinosaurBehaviorTree::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateStateTimer(DeltaTime);
    UpdateThreatDecay(DeltaTime);
    UpdatePackBehavior();
    DetectThreats();
}

void UNPC_DinosaurBehaviorTree::InitializeBehaviorTree()
{
    if (!BlackboardComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurBehaviorTree: No BlackboardComponent found"));
        return;
    }

    // Initialize blackboard values
    SetBlackboardValue(BB_CurrentState, static_cast<float>(CurrentState));
    SetBlackboardValue(BB_ThreatLevel, 0.0f);
    SetBlackboardObject(BB_PackLeader, PackLeader);
    SetBlackboardValue(BB_IsInPack, IsInPack() ? 1.0f : 0.0f);

    // Set initial target location (home position)
    if (AActor* Owner = GetOwner())
    {
        SetBlackboardVector(BB_TargetLocation, Owner->GetActorLocation());
    }

    UE_LOG(LogTemp, Log, TEXT("DinosaurBehaviorTree: Initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UNPC_DinosaurBehaviorTree::StartBehaviorTree()
{
    if (BehaviorTreeComponent && BehaviorTreeAsset)
    {
        BehaviorTreeComponent->StartTree(*BehaviorTreeAsset);
        UE_LOG(LogTemp, Log, TEXT("DinosaurBehaviorTree: Started behavior tree"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurBehaviorTree: Cannot start - missing component or asset"));
    }
}

void UNPC_DinosaurBehaviorTree::StopBehaviorTree()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
        UE_LOG(LogTemp, Log, TEXT("DinosaurBehaviorTree: Stopped behavior tree"));
    }
}

void UNPC_DinosaurBehaviorTree::RestartBehaviorTree()
{
    StopBehaviorTree();
    StartBehaviorTree();
}

void UNPC_DinosaurBehaviorTree::SetCurrentState(ENPCBehaviorState NewState)
{
    if (CurrentState != NewState)
    {
        PreviousState = CurrentState;
        CurrentState = NewState;
        StateTimer = 0.0f;

        // Update blackboard
        SetBlackboardValue(BB_CurrentState, static_cast<float>(CurrentState));

        UE_LOG(LogTemp, Log, TEXT("DinosaurBehaviorTree: State changed from %d to %d"), 
               static_cast<int32>(PreviousState), static_cast<int32>(CurrentState));
    }
}

void UNPC_DinosaurBehaviorTree::TransitionToState(ENPCBehaviorState NewState, float TransitionDelay)
{
    StateTransitionDelay = TransitionDelay;
    SetCurrentState(NewState);
}

void UNPC_DinosaurBehaviorTree::SetBlackboardValue(const FString& KeyName, float Value)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsFloat(*KeyName, Value);
    }
}

void UNPC_DinosaurBehaviorTree::SetBlackboardVector(const FString& KeyName, const FVector& Value)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(*KeyName, Value);
    }
}

void UNPC_DinosaurBehaviorTree::SetBlackboardObject(const FString& KeyName, UObject* Value)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(*KeyName, Value);
    }
}

float UNPC_DinosaurBehaviorTree::GetBlackboardValue(const FString& KeyName) const
{
    if (BlackboardComponent)
    {
        return BlackboardComponent->GetValueAsFloat(*KeyName);
    }
    return 0.0f;
}

bool UNPC_DinosaurBehaviorTree::IsInCombat() const
{
    return CurrentState == ENPCBehaviorState::Combat || CurrentState == ENPCBehaviorState::Hunting;
}

bool UNPC_DinosaurBehaviorTree::IsHunting() const
{
    return CurrentState == ENPCBehaviorState::Hunting;
}

bool UNPC_DinosaurBehaviorTree::IsFleeing() const
{
    return CurrentState == ENPCBehaviorState::Fleeing;
}

bool UNPC_DinosaurBehaviorTree::IsFeeding() const
{
    return CurrentState == ENPCBehaviorState::Feeding;
}

bool UNPC_DinosaurBehaviorTree::IsPatrolling() const
{
    return CurrentState == ENPCBehaviorState::Patrolling;
}

void UNPC_DinosaurBehaviorTree::JoinPack(UNPC_DinosaurBehaviorTree* NewPackLeader)
{
    if (!NewPackLeader || NewPackLeader == this)
        return;

    // Leave current pack first
    LeavePack();

    // Join new pack
    PackLeader = NewPackLeader;
    bIsPackLeader = false;

    // Add to leader's pack members
    if (PackLeader->PackMembers.Num() < PackLeader->MaxPackSize)
    {
        PackLeader->PackMembers.AddUnique(this);
        SetBlackboardObject(BB_PackLeader, PackLeader);
        SetBlackboardValue(BB_IsInPack, 1.0f);

        UE_LOG(LogTemp, Log, TEXT("DinosaurBehaviorTree: %s joined pack led by %s"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
               PackLeader->GetOwner() ? *PackLeader->GetOwner()->GetName() : TEXT("Unknown"));
    }
}

void UNPC_DinosaurBehaviorTree::LeavePack()
{
    if (PackLeader)
    {
        // Remove from leader's pack members
        PackLeader->PackMembers.Remove(this);
        PackLeader = nullptr;
    }

    // If this was a pack leader, disband the pack
    if (bIsPackLeader)
    {
        for (UNPC_DinosaurBehaviorTree* Member : PackMembers)
        {
            if (Member)
            {
                Member->PackLeader = nullptr;
                Member->SetBlackboardObject(BB_PackLeader, nullptr);
                Member->SetBlackboardValue(BB_IsInPack, 0.0f);
            }
        }
        PackMembers.Empty();
        bIsPackLeader = false;
    }

    // Update blackboard
    SetBlackboardObject(BB_PackLeader, nullptr);
    SetBlackboardValue(BB_IsInPack, 0.0f);

    UE_LOG(LogTemp, Log, TEXT("DinosaurBehaviorTree: %s left pack"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UNPC_DinosaurBehaviorTree::DetectThreats()
{
    if (!GetOwner())
        return;

    UWorld* World = GetOwner()->GetWorld();
    if (!World)
        return;

    FVector OwnerLocation = GetOwner()->GetActorLocation();

    // Find all pawns within threat detection radius
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (!Actor || Actor == GetOwner())
            continue;

        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        if (Distance <= ThreatDetectionRadius)
        {
            // Calculate threat level based on distance and actor type
            float ThreatLevel = 100.0f - (Distance / ThreatDetectionRadius * 100.0f);
            
            // Increase threat for players
            if (Actor->IsA<ACharacter>())
            {
                ThreatLevel *= 1.5f;
            }

            AddThreat(Actor, ThreatLevel);
        }
    }
}

void UNPC_DinosaurBehaviorTree::AddThreat(AActor* ThreatActor, float ThreatLevel)
{
    if (!ThreatActor)
        return;

    ThreatMap.Add(ThreatActor, ThreatLevel);
    
    // Update blackboard with highest threat
    AActor* HighestThreat = GetHighestThreat();
    if (HighestThreat)
    {
        SetBlackboardObject(BB_TargetActor, HighestThreat);
        SetBlackboardValue(BB_ThreatLevel, ThreatMap[HighestThreat]);
    }
}

void UNPC_DinosaurBehaviorTree::RemoveThreat(AActor* ThreatActor)
{
    ThreatMap.Remove(ThreatActor);
}

AActor* UNPC_DinosaurBehaviorTree::GetHighestThreat() const
{
    AActor* HighestThreatActor = nullptr;
    float HighestThreatLevel = 0.0f;

    for (const auto& ThreatPair : ThreatMap)
    {
        if (ThreatPair.Value > HighestThreatLevel)
        {
            HighestThreatLevel = ThreatPair.Value;
            HighestThreatActor = ThreatPair.Key;
        }
    }

    return HighestThreatActor;
}

void UNPC_DinosaurBehaviorTree::UpdateStateTimer(float DeltaTime)
{
    StateTimer += DeltaTime;
}

void UNPC_DinosaurBehaviorTree::ProcessStateTransition()
{
    if (StateTimer >= StateTransitionDelay)
    {
        // State transition logic based on current conditions
        // This would be expanded based on specific behavior requirements
    }
}

void UNPC_DinosaurBehaviorTree::UpdateThreatDecay(float DeltaTime)
{
    TArray<AActor*> ThreatsToRemove;

    for (auto& ThreatPair : ThreatMap)
    {
        ThreatPair.Value -= ThreatDecayRate * DeltaTime;
        
        if (ThreatPair.Value <= 0.0f)
        {
            ThreatsToRemove.Add(ThreatPair.Key);
        }
    }

    // Remove expired threats
    for (AActor* ThreatToRemove : ThreatsToRemove)
    {
        RemoveThreat(ThreatToRemove);
    }
}

void UNPC_DinosaurBehaviorTree::UpdatePackBehavior()
{
    if (IsInPack() && PackLeader)
    {
        // Follow pack leader's behavior state
        if (PackLeader->CurrentState != CurrentState)
        {
            // Pack members should generally follow leader's state
            // but with some individual variation
            float StateFollowChance = 0.8f; // 80% chance to follow leader
            
            if (FMath::RandRange(0.0f, 1.0f) <= StateFollowChance)
            {
                SetCurrentState(PackLeader->CurrentState);
            }
        }
    }
}