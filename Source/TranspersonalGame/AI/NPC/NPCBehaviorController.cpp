#include "NPCBehaviorController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ANPCBehaviorController::ANPCBehaviorController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Set default values
    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    PatrolRadius = 1000.0f;
    AlertRadius = 1500.0f;
    FleeRadius = 500.0f;
    WalkSpeed = 150.0f;
    RunSpeed = 400.0f;
    
    MaxShortTermMemories = 10;
    MemoryDecayRate = 0.1f;
    
    TimeSinceLastPlayerSighting = 0.0f;
    bPlayerInSight = false;
    CurrentPatrolIndex = 0;
}

void ANPCBehaviorController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAIPerception();
    
    // Set home location
    if (GetPawn())
    {
        HomeLocation = GetPawn()->GetActorLocation();
        GeneratePatrolPoints();
    }
    
    // Start behavior tree if available
    if (DefaultBehaviorTree && BlackboardData)
    {
        UseBlackboard(BlackboardData);
        RunBehaviorTree(DefaultBehaviorTree);
        
        // Initialize blackboard values
        if (GetBlackboardComponent())
        {
            GetBlackboardComponent()->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(CurrentBehaviorState));
            GetBlackboardComponent()->SetValueAsVector(TEXT("HomeLocation"), HomeLocation);
            GetBlackboardComponent()->SetValueAsFloat(TEXT("PatrolRadius"), PatrolRadius);
        }
    }
}

void ANPCBehaviorController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateMemorySystem(DeltaTime);
    
    // Update time since last player sighting
    if (!bPlayerInSight)
    {
        TimeSinceLastPlayerSighting += DeltaTime;
    }
}

void ANPCBehaviorController::InitializeAIPerception()
{
    if (!AIPerceptionComponent)
        return;
    
    // Configure sight sense
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = AlertRadius;
        SightConfig->LoseSightRadius = AlertRadius + 200.0f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }
    
    // Configure hearing sense
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = AlertRadius * 0.8f;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }
    
    // Set sight as dominant sense
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    
    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ANPCBehaviorController::OnPerceptionUpdated);
}

void ANPCBehaviorController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor)
            continue;
            
        // Check if this is the player
        if (Actor->IsA<ACharacter>() && Cast<ACharacter>(Actor)->IsPlayerControlled())
        {
            FActorPerceptionBlueprintInfo PerceptionInfo;
            AIPerceptionComponent->GetActorsPerception(Actor, PerceptionInfo);
            
            if (PerceptionInfo.LastSensedStimuli.Num() > 0)
            {
                FAIStimulus LastStimulus = PerceptionInfo.LastSensedStimuli[0];
                
                if (LastStimulus.WasSuccessfullySensed())
                {
                    // Player spotted
                    bPlayerInSight = true;
                    TimeSinceLastPlayerSighting = 0.0f;
                    LastKnownPlayerLocation = Actor->GetActorLocation();
                    
                    // Add to memory
                    FNPC_MemoryEntry NewMemory;
                    NewMemory.ActorReference = Actor;
                    NewMemory.Location = Actor->GetActorLocation();
                    NewMemory.MemoryType = ENPC_MemoryType::PlayerSighting;
                    NewMemory.Timestamp = GetWorld()->GetTimeSeconds();
                    NewMemory.Importance = 1.0f;
                    NewMemory.EmotionalWeight = 0.8f;
                    
                    AddMemory(NewMemory);
                    
                    // Update behavior based on distance
                    float DistanceToPlayer = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
                    
                    if (DistanceToPlayer <= FleeRadius)
                    {
                        SetBehaviorState(ENPC_BehaviorState::Fleeing);
                    }
                    else if (DistanceToPlayer <= AlertRadius)
                    {
                        SetBehaviorState(ENPC_BehaviorState::Alert);
                    }
                }
                else
                {
                    // Lost sight of player
                    bPlayerInSight = false;
                }
            }
        }
    }
}

void ANPCBehaviorController::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentBehaviorState == NewState)
        return;
        
    CurrentBehaviorState = NewState;
    
    // Update blackboard
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(NewState));
    }
    
    // Update movement speed based on state
    if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
    {
        UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
        if (MovementComponent)
        {
            switch (NewState)
            {
                case ENPC_BehaviorState::Patrolling:
                case ENPC_BehaviorState::Idle:
                    MovementComponent->MaxWalkSpeed = WalkSpeed;
                    break;
                case ENPC_BehaviorState::Alert:
                case ENPC_BehaviorState::Fleeing:
                    MovementComponent->MaxWalkSpeed = RunSpeed;
                    break;
                default:
                    MovementComponent->MaxWalkSpeed = WalkSpeed;
                    break;
            }
        }
    }
}

void ANPCBehaviorController::StartPatrolBehavior()
{
    SetBehaviorState(ENPC_BehaviorState::Patrolling);
    
    if (GetBlackboardComponent() && PatrolPoints.Num() > 0)
    {
        GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolTarget"), GetNextPatrolPoint());
    }
}

void ANPCBehaviorController::StartFleeBehavior(AActor* ThreatActor)
{
    SetBehaviorState(ENPC_BehaviorState::Fleeing);
    
    if (GetBlackboardComponent() && ThreatActor)
    {
        // Calculate flee direction (opposite to threat)
        FVector FleeDirection = GetPawn()->GetActorLocation() - ThreatActor->GetActorLocation();
        FleeDirection.Normalize();
        FVector FleeTarget = GetPawn()->GetActorLocation() + (FleeDirection * FleeRadius * 2.0f);
        
        GetBlackboardComponent()->SetValueAsVector(TEXT("FleeTarget"), FleeTarget);
        GetBlackboardComponent()->SetValueAsObject(TEXT("ThreatActor"), ThreatActor);
    }
}

void ANPCBehaviorController::StartIdleBehavior()
{
    SetBehaviorState(ENPC_BehaviorState::Idle);
}

void ANPCBehaviorController::AddMemory(const FNPC_MemoryEntry& NewMemory)
{
    // Add to short-term memory
    ShortTermMemory.Add(NewMemory);
    
    // Maintain memory limit
    if (ShortTermMemory.Num() > MaxShortTermMemories)
    {
        ShortTermMemory.RemoveAt(0);
    }
}

bool ANPCBehaviorController::HasMemoryOfActor(AActor* Actor) const
{
    if (!Actor)
        return false;
        
    // Check short-term memory
    for (const FNPC_MemoryEntry& Memory : ShortTermMemory)
    {
        if (Memory.ActorReference == Actor)
            return true;
    }
    
    // Check long-term memory
    for (const FNPC_MemoryEntry& Memory : LongTermMemory)
    {
        if (Memory.ActorReference == Actor)
            return true;
    }
    
    return false;
}

void ANPCBehaviorController::ClearMemories()
{
    ShortTermMemory.Empty();
    LongTermMemory.Empty();
}

void ANPCBehaviorController::UpdateMemorySystem(float DeltaTime)
{
    ProcessShortTermMemory();
    ConsolidateToLongTermMemory();
}

void ANPCBehaviorController::ProcessShortTermMemory()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Decay memory importance over time
    for (FNPC_MemoryEntry& Memory : ShortTermMemory)
    {
        float TimeSinceMemory = CurrentTime - Memory.Timestamp;
        Memory.Importance = FMath::Max(0.0f, Memory.Importance - (MemoryDecayRate * TimeSinceMemory));
    }
    
    // Remove memories with very low importance
    ShortTermMemory.RemoveAll([](const FNPC_MemoryEntry& Memory) {
        return Memory.Importance <= 0.1f;
    });
}

void ANPCBehaviorController::ConsolidateToLongTermMemory()
{
    // Move important memories to long-term storage
    for (int32 i = ShortTermMemory.Num() - 1; i >= 0; i--)
    {
        const FNPC_MemoryEntry& Memory = ShortTermMemory[i];
        
        // High importance or high emotional weight memories get consolidated
        if (Memory.Importance > 0.8f || Memory.EmotionalWeight > 0.7f)
        {
            LongTermMemory.Add(Memory);
            ShortTermMemory.RemoveAt(i);
        }
    }
    
    // Limit long-term memory size
    if (LongTermMemory.Num() > 50)
    {
        LongTermMemory.RemoveAt(0);
    }
}

void ANPCBehaviorController::GeneratePatrolPoints()
{
    PatrolPoints.Empty();
    
    // Generate 4-6 patrol points around home location
    int32 NumPoints = FMath::RandRange(4, 6);
    
    for (int32 i = 0; i < NumPoints; i++)
    {
        float Angle = (2.0f * PI * i) / NumPoints;
        float Distance = FMath::RandRange(PatrolRadius * 0.5f, PatrolRadius);
        
        FVector PatrolPoint = HomeLocation + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        PatrolPoints.Add(PatrolPoint);
    }
}

FVector ANPCBehaviorController::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
        return HomeLocation;
        
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    return PatrolPoints[CurrentPatrolIndex];
}