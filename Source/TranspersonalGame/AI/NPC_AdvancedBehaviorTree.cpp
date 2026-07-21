#include "NPC_AdvancedBehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"

ANPC_AdvancedBehaviorTreeController::ANPC_AdvancedBehaviorTreeController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create behavior tree component
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    // Create AI perception component
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Initialize default values
    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    SightRadius = 1500.0f;
    HearingRadius = 800.0f;
    PatrolRadius = 1000.0f;
    FleeDistance = 2000.0f;
    HomeLocation = FVector::ZeroVector;
    LastMemoryCleanup = 0.0f;
    StateChangeTimer = 0.0f;
    TribalLeader = nullptr;

    // Initialize memory arrays
    ShortTermMemory.Empty();
    LongTermMemory.Empty();
    TribeMembers.Empty();
}

void ANPC_AdvancedBehaviorTreeController::BeginPlay()
{
    Super::BeginPlay();

    InitializePerception();
    
    if (GetPawn())
    {
        HomeLocation = GetPawn()->GetActorLocation();
    }

    // Start behavior tree after a short delay
    FTimerHandle StartTimer;
    GetWorld()->GetTimerManager().SetTimer(StartTimer, this, &ANPC_AdvancedBehaviorTreeController::StartBehaviorTree, 1.0f, false);
}

void ANPC_AdvancedBehaviorTreeController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateBehaviorState(DeltaTime);
    ProcessMemories();

    // Clean up old memories every 30 seconds
    if (GetWorld()->GetTimeSeconds() - LastMemoryCleanup > 30.0f)
    {
        ClearOldMemories(60.0f); // Remove memories older than 60 seconds
        LastMemoryCleanup = GetWorld()->GetTimeSeconds();
    }
}

void ANPC_AdvancedBehaviorTreeController::InitializePerception()
{
    if (!AIPerceptionComponent)
    {
        return;
    }

    // Configure sight sense
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = SightRadius;
        SightConfig->LoseSightRadius = SightRadius * 1.2f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Configure hearing sense
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = HearingRadius;
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Set sight as dominant sense
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ANPC_AdvancedBehaviorTreeController::OnPerceptionUpdated);
}

void ANPC_AdvancedBehaviorTreeController::StartBehaviorTree()
{
    if (BehaviorTreeAsset && BlackboardAsset)
    {
        if (BlackboardComponent)
        {
            BlackboardComponent->InitializeBlackboard(*BlackboardAsset);
        }

        if (BehaviorTreeComponent)
        {
            BehaviorTreeComponent->StartTree(*BehaviorTreeAsset);
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                    FString::Printf(TEXT("NPC %s: Started behavior tree"), 
                    GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown")));
            }
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                TEXT("NPC: Missing behavior tree or blackboard asset"));
        }
    }
}

void ANPC_AdvancedBehaviorTreeController::StopBehaviorTree()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }
}

void ANPC_AdvancedBehaviorTreeController::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        StateChangeTimer = GetWorld()->GetTimeSeconds();

        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("BehaviorState"), (uint8)NewState);
        }

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                FString::Printf(TEXT("NPC %s: Behavior state changed to %d"), 
                GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"), (int32)NewState));
        }
    }
}

void ANPC_AdvancedBehaviorTreeController::AddMemoryEntry(const FNPC_MemoryEntry& NewEntry)
{
    FNPC_MemoryEntry Entry = NewEntry;
    Entry.Timestamp = GetWorld()->GetTimeSeconds();

    // Add to short-term memory
    ShortTermMemory.Add(Entry);

    // Move high priority memories to long-term
    if (Entry.Priority >= ENPC_Priority::High)
    {
        LongTermMemory.Add(Entry);
    }

    // Limit short-term memory size
    if (ShortTermMemory.Num() > 20)
    {
        ShortTermMemory.RemoveAt(0);
    }

    // Limit long-term memory size
    if (LongTermMemory.Num() > 50)
    {
        LongTermMemory.RemoveAt(0);
    }
}

void ANPC_AdvancedBehaviorTreeController::ClearOldMemories(float MaxAge)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Clear old short-term memories
    for (int32 i = ShortTermMemory.Num() - 1; i >= 0; i--)
    {
        if (CurrentTime - ShortTermMemory[i].Timestamp > MaxAge)
        {
            ShortTermMemory.RemoveAt(i);
        }
    }

    // Clear old long-term memories (longer retention)
    for (int32 i = LongTermMemory.Num() - 1; i >= 0; i--)
    {
        if (CurrentTime - LongTermMemory[i].Timestamp > MaxAge * 5.0f)
        {
            LongTermMemory.RemoveAt(i);
        }
    }
}

TArray<FNPC_MemoryEntry> ANPC_AdvancedBehaviorTreeController::GetMemoriesByPriority(ENPC_Priority MinPriority)
{
    TArray<FNPC_MemoryEntry> FilteredMemories;

    // Check short-term memories
    for (const FNPC_MemoryEntry& Memory : ShortTermMemory)
    {
        if (Memory.Priority >= MinPriority)
        {
            FilteredMemories.Add(Memory);
        }
    }

    // Check long-term memories
    for (const FNPC_MemoryEntry& Memory : LongTermMemory)
    {
        if (Memory.Priority >= MinPriority)
        {
            FilteredMemories.Add(Memory);
        }
    }

    return FilteredMemories;
}

void ANPC_AdvancedBehaviorTreeController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor || Actor == GetPawn())
        {
            continue;
        }

        // Create memory entry for perceived actor
        FNPC_MemoryEntry NewMemory;
        NewMemory.TargetActor = Actor;
        NewMemory.Location = Actor->GetActorLocation();
        NewMemory.Priority = ENPC_Priority::Normal;

        // Determine priority based on actor type
        FString ActorName = Actor->GetName();
        if (ActorName.Contains(TEXT("Dinosaur")) || ActorName.Contains(TEXT("TRex")) || ActorName.Contains(TEXT("Raptor")))
        {
            NewMemory.Priority = ENPC_Priority::Critical;
            NewMemory.Description = TEXT("Dangerous predator spotted");
        }
        else if (ActorName.Contains(TEXT("Human")) || ActorName.Contains(TEXT("NPC")))
        {
            NewMemory.Priority = ENPC_Priority::High;
            NewMemory.Description = TEXT("Fellow human encountered");
        }
        else
        {
            NewMemory.Description = TEXT("Unknown entity perceived");
        }

        AddMemoryEntry(NewMemory);
    }
}

ENPC_BehaviorState ANPC_AdvancedBehaviorTreeController::DecideNextBehavior()
{
    // Check for immediate threats
    if (ShouldFlee())
    {
        return ENPC_BehaviorState::Flee;
    }

    // Check high priority memories
    TArray<FNPC_MemoryEntry> HighPriorityMemories = GetMemoriesByPriority(ENPC_Priority::High);
    
    for (const FNPC_MemoryEntry& Memory : HighPriorityMemories)
    {
        if (Memory.TargetActor && Memory.Description.Contains(TEXT("predator")))
        {
            return ENPC_BehaviorState::Alert;
        }
    }

    // Social behavior if near tribe
    if (IsNearTribe())
    {
        return ENPC_BehaviorState::Social;
    }

    // Default behaviors based on current state and time
    float TimeSinceStateChange = GetWorld()->GetTimeSeconds() - StateChangeTimer;
    
    switch (CurrentBehaviorState)
    {
        case ENPC_BehaviorState::Idle:
            if (TimeSinceStateChange > 10.0f)
            {
                return FMath::RandBool() ? ENPC_BehaviorState::Patrol : ENPC_BehaviorState::Gather;
            }
            break;
            
        case ENPC_BehaviorState::Patrol:
            if (TimeSinceStateChange > 30.0f)
            {
                return ENPC_BehaviorState::Idle;
            }
            break;
            
        case ENPC_BehaviorState::Gather:
            if (TimeSinceStateChange > 20.0f)
            {
                return ENPC_BehaviorState::Idle;
            }
            break;
            
        default:
            break;
    }

    return CurrentBehaviorState;
}

bool ANPC_AdvancedBehaviorTreeController::ShouldFlee()
{
    AActor* Threat = FindNearestThreat();
    if (Threat && GetPawn())
    {
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Threat->GetActorLocation());
        return Distance < FleeDistance;
    }
    return false;
}

AActor* ANPC_AdvancedBehaviorTreeController::FindNearestThreat()
{
    if (!GetPawn())
    {
        return nullptr;
    }

    AActor* NearestThreat = nullptr;
    float NearestDistance = FLT_MAX;

    TArray<FNPC_MemoryEntry> CriticalMemories = GetMemoriesByPriority(ENPC_Priority::Critical);
    
    for (const FNPC_MemoryEntry& Memory : CriticalMemories)
    {
        if (Memory.TargetActor && Memory.Description.Contains(TEXT("predator")))
        {
            float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Memory.TargetActor->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestThreat = Memory.TargetActor;
            }
        }
    }

    return NearestThreat;
}

AActor* ANPC_AdvancedBehaviorTreeController::FindNearestResource()
{
    // Implementation for finding resources (food, materials, etc.)
    // This would be expanded based on the resource system
    return nullptr;
}

void ANPC_AdvancedBehaviorTreeController::RegisterTribeMember(AActor* NewMember)
{
    if (NewMember && !TribeMembers.Contains(NewMember))
    {
        TribeMembers.Add(NewMember);
    }
}

void ANPC_AdvancedBehaviorTreeController::SetTribalLeader(AActor* Leader)
{
    TribalLeader = Leader;
    if (Leader && !TribeMembers.Contains(Leader))
    {
        TribeMembers.Add(Leader);
    }
}

bool ANPC_AdvancedBehaviorTreeController::IsNearTribe()
{
    if (!GetPawn())
    {
        return false;
    }

    for (AActor* TribeMember : TribeMembers)
    {
        if (TribeMember)
        {
            float Distance = FVector::Dist(GetPawn()->GetActorLocation(), TribeMember->GetActorLocation());
            if (Distance < 500.0f) // Within 5 meters
            {
                return true;
            }
        }
    }

    return false;
}

void ANPC_AdvancedBehaviorTreeController::UpdateBehaviorState(float DeltaTime)
{
    // Decide next behavior every few seconds
    if (GetWorld()->GetTimeSeconds() - StateChangeTimer > 5.0f)
    {
        ENPC_BehaviorState NextBehavior = DecideNextBehavior();
        if (NextBehavior != CurrentBehaviorState)
        {
            SetBehaviorState(NextBehavior);
        }
    }
}

void ANPC_AdvancedBehaviorTreeController::ProcessMemories()
{
    // Process and consolidate memories
    // This could include pattern recognition, threat assessment, etc.
}

// Component implementation
UNPC_AdvancedBehaviorComponent::UNPC_AdvancedBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    BehaviorController = nullptr;
    bAutoStartBehavior = true;
    InitialBehaviorState = ENPC_BehaviorState::Idle;
    BehaviorUpdateInterval = 1.0f;
    LastBehaviorUpdate = 0.0f;
}

void UNPC_AdvancedBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoStartBehavior)
    {
        InitializeBehavior();
    }
}

void UNPC_AdvancedBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update behavior at specified intervals
    if (GetWorld()->GetTimeSeconds() - LastBehaviorUpdate > BehaviorUpdateInterval)
    {
        if (BehaviorController)
        {
            // Perform any component-specific behavior updates
        }
        LastBehaviorUpdate = GetWorld()->GetTimeSeconds();
    }
}

void UNPC_AdvancedBehaviorComponent::InitializeBehavior()
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn)
    {
        ANPC_AdvancedBehaviorTreeController* AIController = Cast<ANPC_AdvancedBehaviorTreeController>(OwnerPawn->GetController());
        if (AIController)
        {
            SetBehaviorController(AIController);
            AIController->SetBehaviorState(InitialBehaviorState);
        }
    }
}

void UNPC_AdvancedBehaviorComponent::SetBehaviorController(ANPC_AdvancedBehaviorTreeController* Controller)
{
    BehaviorController = Controller;
}

ANPC_AdvancedBehaviorTreeController* UNPC_AdvancedBehaviorComponent::GetBehaviorController() const
{
    return BehaviorController;
}