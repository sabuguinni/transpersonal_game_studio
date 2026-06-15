#include "NPCBehaviorTree.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "TranspersonalGame/TranspersonalGameState.h"

ANPCBehaviorTreeController::ANPCBehaviorTreeController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize components
    BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
    
    // Set up AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Configure sight sense
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1500.0f;
    SightConfig->LoseSightRadius = 1600.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Configure hearing sense
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1200.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    
    // Initialize behavior parameters
    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    DetectionRadius = 1000.0f;
    InteractionRadius = 300.0f;
    FleeRadius = 2000.0f;
    LastUpdateTime = 0.0f;
    CurrentPatrolIndex = 0;
    bIsNightTime = false;
    
    // Initialize daily routine defaults
    DailyRoutine.WakeUpTime = 6.0f;
    DailyRoutine.SleepTime = 22.0f;
    DailyRoutine.HomeLocation = FVector::ZeroVector;
}

void ANPCBehaviorTreeController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBehaviorTree();
    
    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ANPCBehaviorTreeController::OnPerceptionUpdated);
    }
    
    // Set initial blackboard values
    UpdateBlackboardValues();
}

void ANPCBehaviorTreeController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    ProcessBehaviorLogic(DeltaTime);
    UpdateBlackboardValues();
    
    // Process time-based behaviors
    if (UWorld* World = GetWorld())
    {
        ATranspersonalGameState* GameState = World->GetGameState<ATranspersonalGameState>();
        if (GameState)
        {
            ProcessTimeOfDay(GameState->GetTimeOfDay());
        }
    }
}

void ANPCBehaviorTreeController::InitializeBehaviorTree()
{
    if (BehaviorTreeAsset && Blackboard)
    {
        UseBlackboard(Blackboard);
        RunBehaviorTree(BehaviorTreeAsset);
    }
}

void ANPCBehaviorTreeController::UpdateBlackboardValues()
{
    if (!BlackboardComp) return;
    
    // Update current behavior state
    BlackboardComp->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(CurrentBehaviorState));
    
    // Update fear and trust levels
    BlackboardComp->SetValueAsFloat(TEXT("FearLevel"), NPCMemory.FearLevel);
    BlackboardComp->SetValueAsFloat(TEXT("TrustLevel"), NPCMemory.TrustLevel);
    
    // Update home location
    BlackboardComp->SetValueAsVector(TEXT("HomeLocation"), DailyRoutine.HomeLocation);
    
    // Update patrol points if available
    if (DailyRoutine.PatrolPoints.Num() > 0)
    {
        FVector NextPatrolPoint = GetNextPatrolPoint();
        BlackboardComp->SetValueAsVector(TEXT("PatrolTarget"), NextPatrolPoint);
    }
    
    // Update time-based flags
    BlackboardComp->SetValueAsBool(TEXT("IsNightTime"), bIsNightTime);
}

void ANPCBehaviorTreeController::ProcessBehaviorLogic(float DeltaTime)
{
    LastUpdateTime += DeltaTime;
    
    // Process behavior every 0.1 seconds
    if (LastUpdateTime >= 0.1f)
    {
        switch (CurrentBehaviorState)
        {
            case ENPC_BehaviorState::Idle:
                HandleIdleBehavior();
                break;
            case ENPC_BehaviorState::Patrolling:
                HandlePatrolBehavior();
                break;
            case ENPC_BehaviorState::Socializing:
                HandleSocialBehavior();
                break;
            case ENPC_BehaviorState::Fleeing:
                HandleFleeingBehavior();
                break;
            default:
                break;
        }
        
        LastUpdateTime = 0.0f;
    }
}

void ANPCBehaviorTreeController::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        UpdateBlackboardValues();
    }
}

ENPC_BehaviorState ANPCBehaviorTreeController::GetCurrentBehaviorState() const
{
    return CurrentBehaviorState;
}

void ANPCBehaviorTreeController::UpdateMemory(AActor* Actor, FVector Location, float ThreatLevel)
{
    if (!Actor) return;
    
    // Update last seen player locations
    if (Actor->IsA<ACharacter>())
    {
        NPCMemory.LastSeenPlayerLocations.Add(Location);
        NPCMemory.LastPlayerInteractionTime = GetWorld()->GetTimeSeconds();
        
        // Limit memory to last 10 locations
        if (NPCMemory.LastSeenPlayerLocations.Num() > 10)
        {
            NPCMemory.LastSeenPlayerLocations.RemoveAt(0);
        }
    }
    
    // Update threat assessment
    if (ThreatLevel > 0.5f)
    {
        NPCMemory.KnownThreats.AddUnique(Actor);
        NPCMemory.FearLevel = FMath::Clamp(NPCMemory.FearLevel + ThreatLevel * 0.1f, 0.0f, 1.0f);
    }
    else
    {
        NPCMemory.TrustLevel = FMath::Clamp(NPCMemory.TrustLevel + 0.05f, 0.0f, 1.0f);
    }
}

void ANPCBehaviorTreeController::SetDailyRoutine(const FNPC_DailyRoutine& NewRoutine)
{
    DailyRoutine = NewRoutine;
    CurrentPatrolIndex = 0;
    UpdateBlackboardValues();
}

bool ANPCBehaviorTreeController::ShouldFlee(AActor* ThreatActor)
{
    if (!ThreatActor) return false;
    
    float DistanceToThreat = FVector::Dist(GetPawn()->GetActorLocation(), ThreatActor->GetActorLocation());
    bool bIsThreatClose = DistanceToThreat < FleeRadius;
    bool bIsKnownThreat = NPCMemory.KnownThreats.Contains(ThreatActor);
    bool bHighFear = NPCMemory.FearLevel > 0.7f;
    
    return bIsThreatClose && (bIsKnownThreat || bHighFear);
}

FVector ANPCBehaviorTreeController::GetNextPatrolPoint()
{
    if (DailyRoutine.PatrolPoints.Num() == 0)
    {
        return GetPawn()->GetActorLocation();
    }
    
    FVector NextPoint = DailyRoutine.PatrolPoints[CurrentPatrolIndex];
    
    // Check if we've reached the current patrol point
    float DistanceToPoint = FVector::Dist(GetPawn()->GetActorLocation(), NextPoint);
    if (DistanceToPoint < 200.0f)
    {
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % DailyRoutine.PatrolPoints.Num();
        NextPoint = DailyRoutine.PatrolPoints[CurrentPatrolIndex];
    }
    
    return NextPoint;
}

void ANPCBehaviorTreeController::ProcessTimeOfDay(float CurrentTime)
{
    bIsNightTime = (CurrentTime < DailyRoutine.WakeUpTime || CurrentTime > DailyRoutine.SleepTime);
    
    // Adjust behavior based on time of day
    if (bIsNightTime && CurrentBehaviorState != ENPC_BehaviorState::Sleeping)
    {
        // NPCs become more cautious at night
        NPCMemory.FearLevel = FMath::Clamp(NPCMemory.FearLevel + 0.1f, 0.0f, 1.0f);
        
        // Consider going home or finding shelter
        if (CurrentBehaviorState == ENPC_BehaviorState::Patrolling)
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
    }
}

void ANPCBehaviorTreeController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            float ThreatLevel = 0.3f; // Base threat level for players
            
            // Increase threat if player is armed or moving aggressively
            // This would be expanded with more sophisticated threat assessment
            
            UpdateMemory(Actor, Actor->GetActorLocation(), ThreatLevel);
            
            // Decide on reaction
            if (ShouldFlee(Actor))
            {
                SetBehaviorState(ENPC_BehaviorState::Fleeing);
            }
            else if (NPCMemory.TrustLevel > 0.6f)
            {
                SetBehaviorState(ENPC_BehaviorState::Socializing);
            }
        }
    }
}

void ANPCBehaviorTreeController::HandleIdleBehavior()
{
    // Random chance to start patrolling or other activities
    if (FMath::RandRange(0.0f, 1.0f) < 0.1f && DailyRoutine.PatrolPoints.Num() > 0)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrolling);
    }
}

void ANPCBehaviorTreeController::HandlePatrolBehavior()
{
    // Continue patrolling unless interrupted by threats or social interactions
    if (NPCMemory.FearLevel > 0.8f)
    {
        SetBehaviorState(ENPC_BehaviorState::Fleeing);
    }
}

void ANPCBehaviorTreeController::HandleSocialBehavior()
{
    // Social interaction timeout
    if (GetWorld()->GetTimeSeconds() - NPCMemory.LastPlayerInteractionTime > 30.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}

void ANPCBehaviorTreeController::HandleFleeingBehavior()
{
    // Return to normal behavior after fleeing for a while
    if (NPCMemory.FearLevel < 0.3f)
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
    
    // Gradually reduce fear over time
    NPCMemory.FearLevel = FMath::Clamp(NPCMemory.FearLevel - 0.01f, 0.0f, 1.0f);
}