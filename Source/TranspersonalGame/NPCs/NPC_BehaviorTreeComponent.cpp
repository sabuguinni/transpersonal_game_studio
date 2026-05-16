#include "NPC_BehaviorTreeComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UNPC_BehaviorTreeComponent::UNPC_BehaviorTreeComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize default values
    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    DetectionRadius = 3000.0f;
    AggressionLevel = 0.5f;
    PatrolSpeed = 300.0f;
    ChaseSpeed = 600.0f;
    CurrentPatrolIndex = 0;
    StateChangeTimer = 0.0f;
    MemoryUpdateTimer = 0.0f;

    // Initialize memory
    NPCMemory = FNPC_BehaviorMemory();
    
    // Set default patrol points (will be overridden per NPC)
    PatrolPoints.Add(FVector(0, 0, 0));
    PatrolPoints.Add(FVector(1000, 0, 0));
    PatrolPoints.Add(FVector(1000, 1000, 0));
    PatrolPoints.Add(FVector(0, 1000, 0));

    AIControllerRef = nullptr;
    BehaviorTreeComponent = nullptr;
    BlackboardComponent = nullptr;
}

void UNPC_BehaviorTreeComponent::BeginPlay()
{
    Super::BeginPlay();

    // Get AI Controller reference
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        AIControllerRef = Cast<AAIController>(OwnerPawn->GetController());
        if (AIControllerRef)
        {
            BehaviorTreeComponent = AIControllerRef->GetBehaviorTreeComponent();
            BlackboardComponent = AIControllerRef->GetBlackboardComponent();
        }
    }

    // Set home location to current position
    NPCMemory.HomeLocation = GetOwner()->GetActorLocation();

    // Initialize behavior tree if assets are set
    if (BehaviorTreeAsset && BlackboardAsset)
    {
        InitializeBehaviorTree();
    }
}

void UNPC_BehaviorTreeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateChangeTimer += DeltaTime;
    MemoryUpdateTimer += DeltaTime;

    // Update behavior logic
    ProcessBehaviorLogic(DeltaTime);

    // Update AI blackboard every 0.5 seconds
    if (MemoryUpdateTimer >= 0.5f)
    {
        UpdateAIBlackboard();
        MemoryUpdateTimer = 0.0f;
    }
}

void UNPC_BehaviorTreeComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        StateChangeTimer = 0.0f;

        // Log state change for debugging
        UE_LOG(LogTemp, Log, TEXT("NPC %s changed behavior state to %s"), 
               *GetOwner()->GetName(), 
               *UEnum::GetValueAsString(NewState));

        // Update blackboard with new state
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("BehaviorState"), (uint8)NewState);
        }
    }
}

void UNPC_BehaviorTreeComponent::UpdatePlayerMemory(FVector PlayerLocation, bool bIsHostile)
{
    // Add to memory with timestamp
    NPCMemory.LastKnownPlayerPositions.Add(PlayerLocation);
    NPCMemory.LastPlayerEncounterTime = GetWorld()->GetTimeSeconds();
    NPCMemory.bPlayerIsHostile = bIsHostile;

    // Limit memory size to prevent infinite growth
    if (NPCMemory.LastKnownPlayerPositions.Num() > 10)
    {
        NPCMemory.LastKnownPlayerPositions.RemoveAt(0);
    }

    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("LastKnownPlayerLocation"), PlayerLocation);
        BlackboardComponent->SetValueAsBool(TEXT("PlayerIsHostile"), bIsHostile);
        BlackboardComponent->SetValueAsFloat(TEXT("LastPlayerEncounterTime"), NPCMemory.LastPlayerEncounterTime);
    }
}

bool UNPC_BehaviorTreeComponent::IsPlayerInTerritory(FVector PlayerLocation)
{
    float DistanceToHome = FVector::Dist(PlayerLocation, NPCMemory.HomeLocation);
    return DistanceToHome <= NPCMemory.TerritoryRadius;
}

FVector UNPC_BehaviorTreeComponent::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        return NPCMemory.HomeLocation;
    }

    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    return PatrolPoints[CurrentPatrolIndex];
}

void UNPC_BehaviorTreeComponent::InitializeBehaviorTree()
{
    if (!AIControllerRef || !BehaviorTreeAsset || !BlackboardAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot initialize behavior tree - missing references"));
        return;
    }

    // Initialize blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->InitializeBlackboard(*BlackboardAsset);
        
        // Set initial values
        BlackboardComponent->SetValueAsVector(TEXT("HomeLocation"), NPCMemory.HomeLocation);
        BlackboardComponent->SetValueAsFloat(TEXT("TerritoryRadius"), NPCMemory.TerritoryRadius);
        BlackboardComponent->SetValueAsFloat(TEXT("DetectionRadius"), DetectionRadius);
        BlackboardComponent->SetValueAsFloat(TEXT("AggressionLevel"), AggressionLevel);
        BlackboardComponent->SetValueAsEnum(TEXT("BehaviorState"), (uint8)CurrentBehaviorState);
    }

    UE_LOG(LogTemp, Log, TEXT("Behavior tree initialized for %s"), *GetOwner()->GetName());
}

void UNPC_BehaviorTreeComponent::StartBehaviorTree()
{
    if (AIControllerRef && BehaviorTreeComponent && BehaviorTreeAsset)
    {
        BehaviorTreeComponent->StartTree(*BehaviorTreeAsset);
        UE_LOG(LogTemp, Log, TEXT("Behavior tree started for %s"), *GetOwner()->GetName());
    }
}

void UNPC_BehaviorTreeComponent::StopBehaviorTree()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
        UE_LOG(LogTemp, Log, TEXT("Behavior tree stopped for %s"), *GetOwner()->GetName());
    }
}

void UNPC_BehaviorTreeComponent::UpdateAIBlackboard()
{
    if (!BlackboardComponent)
    {
        return;
    }

    // Find nearest player
    AActor* NearestPlayer = FindNearestPlayer();
    if (NearestPlayer)
    {
        FVector PlayerLocation = NearestPlayer->GetActorLocation();
        float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), PlayerLocation);
        
        BlackboardComponent->SetValueAsObject(TEXT("TargetPlayer"), NearestPlayer);
        BlackboardComponent->SetValueAsVector(TEXT("PlayerLocation"), PlayerLocation);
        BlackboardComponent->SetValueAsFloat(TEXT("DistanceToPlayer"), DistanceToPlayer);
        
        // Check if player is within detection range
        bool bPlayerDetected = DistanceToPlayer <= DetectionRadius;
        BlackboardComponent->SetValueAsBool(TEXT("PlayerDetected"), bPlayerDetected);
        
        // Update memory if player is detected
        if (bPlayerDetected)
        {
            UpdatePlayerMemory(PlayerLocation, NPCMemory.bPlayerIsHostile);
        }
    }
    else
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetPlayer"), nullptr);
        BlackboardComponent->SetValueAsBool(TEXT("PlayerDetected"), false);
    }

    // Update current location and next patrol point
    BlackboardComponent->SetValueAsVector(TEXT("CurrentLocation"), GetOwner()->GetActorLocation());
    BlackboardComponent->SetValueAsVector(TEXT("NextPatrolPoint"), GetNextPatrolPoint());
}

void UNPC_BehaviorTreeComponent::ProcessBehaviorLogic(float DeltaTime)
{
    // Simple state machine logic - can be expanded based on specific NPC types
    AActor* NearestPlayer = FindNearestPlayer();
    
    if (NearestPlayer)
    {
        float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), NearestPlayer->GetActorLocation());
        
        // State transitions based on distance and aggression
        switch (CurrentBehaviorState)
        {
            case ENPC_BehaviorState::Idle:
            case ENPC_BehaviorState::Patrolling:
                if (DistanceToPlayer <= DetectionRadius)
                {
                    if (AggressionLevel > 0.7f)
                    {
                        SetBehaviorState(ENPC_BehaviorState::Hunting);
                    }
                    else if (AggressionLevel < 0.3f)
                    {
                        SetBehaviorState(ENPC_BehaviorState::Fleeing);
                    }
                    else
                    {
                        SetBehaviorState(ENPC_BehaviorState::Territorial);
                    }
                }
                else if (StateChangeTimer > 10.0f && CurrentBehaviorState == ENPC_BehaviorState::Idle)
                {
                    SetBehaviorState(ENPC_BehaviorState::Patrolling);
                }
                break;
                
            case ENPC_BehaviorState::Hunting:
                if (DistanceToPlayer > DetectionRadius * 1.5f)
                {
                    SetBehaviorState(ENPC_BehaviorState::Patrolling);
                }
                break;
                
            case ENPC_BehaviorState::Fleeing:
                if (DistanceToPlayer > DetectionRadius * 2.0f)
                {
                    SetBehaviorState(ENPC_BehaviorState::Idle);
                }
                break;
                
            case ENPC_BehaviorState::Territorial:
                if (DistanceToPlayer > DetectionRadius * 1.2f)
                {
                    SetBehaviorState(ENPC_BehaviorState::Patrolling);
                }
                else if (DistanceToPlayer < DetectionRadius * 0.5f && AggressionLevel > 0.5f)
                {
                    SetBehaviorState(ENPC_BehaviorState::Hunting);
                }
                break;
        }
    }
}

AActor* UNPC_BehaviorTreeComponent::FindNearestPlayer()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    AActor* NearestPlayer = nullptr;
    float NearestDistance = FLT_MAX;
    FVector MyLocation = GetOwner()->GetActorLocation();

    // Find all player controllers
    for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        APlayerController* PC = Iterator->Get();
        if (PC && PC->GetPawn())
        {
            FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
            float Distance = FVector::Dist(MyLocation, PlayerLocation);
            
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestPlayer = PC->GetPawn();
            }
        }
    }

    return NearestPlayer;
}