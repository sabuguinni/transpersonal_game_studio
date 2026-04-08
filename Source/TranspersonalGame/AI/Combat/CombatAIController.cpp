#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "NavigationSystem.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize Behavior Tree Component
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    SetPerceptionComponent(*AIPerceptionComponent);

    // Initialize default values
    CurrentCombatState = ECombatState::Idle;
    
    // Blackboard Key Names
    PlayerActorKey = TEXT("PlayerActor");
    LastKnownLocationKey = TEXT("LastKnownLocation");
    CombatStateKey = TEXT("CombatState");
    ThreatLevelKey = TEXT("ThreatLevel");
    PatrolLocationKey = TEXT("PatrolLocation");
    IsPlayerVisibleKey = TEXT("IsPlayerVisible");
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePerception();
    
    if (BehaviorTree)
    {
        RunBehaviorTree(BehaviorTree);
    }
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateCombatMemory(DeltaTime);
    ProcessTacticalDecisions();
}

void ACombatAIController::InitializePerception()
{
    if (!AIPerceptionComponent)
        return;

    // Configure Sight Sense
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2200.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure Hearing Sense
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure Damage Sense
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));

    // Add senses to perception component
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->ConfigureSense(*DamageConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnTargetPerceptionUpdated);
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            FAIStimulus Stimulus;
            bool bCanSenseActor = AIPerceptionComponent->GetActorsPerception(Actor, Stimulus);
            
            if (bCanSenseActor && Stimulus.WasSuccessfullySensed())
            {
                UpdatePlayerMemory(Actor, true);
            }
            else
            {
                UpdatePlayerMemory(Actor, false);
            }
        }
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || !Actor->IsA<ACharacter>())
        return;

    bool bIsVisible = Stimulus.WasSuccessfullySensed();
    UpdatePlayerMemory(Actor, bIsVisible);

    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(PlayerActorKey, bIsVisible ? Actor : nullptr);
        BlackboardComponent->SetValueAsBool(IsPlayerVisibleKey, bIsVisible);
        
        if (bIsVisible)
        {
            BlackboardComponent->SetValueAsVector(LastKnownLocationKey, Actor->GetActorLocation());
        }
    }
}

void ACombatAIController::SetCombatState(ECombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;
        
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(CombatStateKey, static_cast<uint8>(NewState));
        }
    }
}

void ACombatAIController::UpdatePlayerMemory(AActor* Player, bool bIsVisible)
{
    if (!Player)
        return;

    if (bIsVisible)
    {
        CombatMemory.LastKnownPlayerLocation = Player->GetActorLocation();
        CombatMemory.TimeSinceLastSeen = 0.0f;
        CombatMemory.bPlayerIsHiding = false;
    }
    else
    {
        CombatMemory.bPlayerIsHiding = true;
    }

    CombatMemory.PlayerThreatLevel = CalculateThreatLevel();
}

void ACombatAIController::OnTakeDamage(float Damage, FVector DamageLocation)
{
    CombatMemory.LastDamageTime = GetWorld()->GetTimeSeconds();
    CombatMemory.LastDamageLocation = DamageLocation;
    
    // Increase threat level when taking damage
    if (CombatMemory.PlayerThreatLevel < EThreatLevel::High)
    {
        CombatMemory.PlayerThreatLevel = static_cast<EThreatLevel>(
            static_cast<uint8>(CombatMemory.PlayerThreatLevel) + 1
        );
    }

    // Update combat state to attacking if not already
    if (CurrentCombatState == ECombatState::Idle || CurrentCombatState == ECombatState::Hunting)
    {
        SetCombatState(ECombatState::Attacking);
    }
}

FVector ACombatAIController::GetBestAttackPosition()
{
    if (!GetPawn())
        return FVector::ZeroVector;

    FVector CurrentLocation = GetPawn()->GetActorLocation();
    FVector TargetLocation = CombatMemory.LastKnownPlayerLocation;
    
    // Calculate optimal attack distance based on dinosaur type
    float OptimalDistance = 500.0f; // Base distance, should be overridden by specific dinosaur types
    
    // Find position that maintains optimal distance while having line of sight
    FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
    FVector OptimalPosition = TargetLocation - (Direction * OptimalDistance);
    
    // Use navigation system to find valid position
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSystem)
    {
        FNavLocation NavLocation;
        if (NavSystem->ProjectPointToNavigation(OptimalPosition, NavLocation, FVector(200.0f)))
        {
            return NavLocation.Location;
        }
    }
    
    return OptimalPosition;
}

bool ACombatAIController::ShouldFlee()
{
    // Flee if threat level is extreme or if heavily damaged
    if (CombatMemory.PlayerThreatLevel == EThreatLevel::Extreme)
        return true;
    
    // Check pawn health if available
    if (APawn* ControlledPawn = GetPawn())
    {
        // This would need to be implemented based on your health system
        // For now, return false
    }
    
    return false;
}

EThreatLevel ACombatAIController::CalculateThreatLevel()
{
    EThreatLevel ThreatLevel = EThreatLevel::None;
    
    // Base threat on player visibility and proximity
    if (CombatMemory.TimeSinceLastSeen < 5.0f)
    {
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), CombatMemory.LastKnownPlayerLocation);
        
        if (Distance < 300.0f)
            ThreatLevel = EThreatLevel::Extreme;
        else if (Distance < 600.0f)
            ThreatLevel = EThreatLevel::High;
        else if (Distance < 1000.0f)
            ThreatLevel = EThreatLevel::Medium;
        else
            ThreatLevel = EThreatLevel::Low;
    }
    
    // Increase threat if recently damaged
    float TimeSinceDamage = GetWorld()->GetTimeSeconds() - CombatMemory.LastDamageTime;
    if (TimeSinceDamage < 10.0f && ThreatLevel < EThreatLevel::High)
    {
        ThreatLevel = static_cast<EThreatLevel>(static_cast<uint8>(ThreatLevel) + 1);
    }
    
    return ThreatLevel;
}

void ACombatAIController::UpdateCombatMemory(float DeltaTime)
{
    CombatMemory.TimeSinceLastSeen += DeltaTime;
    
    // Update threat level over time
    if (CombatMemory.TimeSinceLastSeen > 30.0f)
    {
        CombatMemory.PlayerThreatLevel = EThreatLevel::None;
        if (CurrentCombatState == ECombatState::Attacking)
        {
            SetCombatState(ECombatState::Hunting);
        }
    }
    
    // Update blackboard with current memory state
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(LastKnownLocationKey, CombatMemory.LastKnownPlayerLocation);
        BlackboardComponent->SetValueAsEnum(ThreatLevelKey, static_cast<uint8>(CombatMemory.PlayerThreatLevel));
    }
}

void ACombatAIController::ProcessTacticalDecisions()
{
    // Process tactical decisions based on current state and memory
    switch (CurrentCombatState)
    {
        case ECombatState::Idle:
            if (CombatMemory.PlayerThreatLevel > EThreatLevel::None)
            {
                SetCombatState(ECombatState::Hunting);
            }
            break;
            
        case ECombatState::Hunting:
            if (CombatMemory.TimeSinceLastSeen < 2.0f)
            {
                SetCombatState(ECombatState::Attacking);
            }
            else if (CombatMemory.TimeSinceLastSeen > 20.0f)
            {
                SetCombatState(ECombatState::Idle);
            }
            break;
            
        case ECombatState::Attacking:
            if (ShouldFlee())
            {
                SetCombatState(ECombatState::Fleeing);
            }
            else if (CombatMemory.TimeSinceLastSeen > 10.0f)
            {
                SetCombatState(ECombatState::Hunting);
            }
            break;
            
        case ECombatState::Fleeing:
            if (CombatMemory.TimeSinceLastSeen > 15.0f && !ShouldFlee())
            {
                SetCombatState(ECombatState::Hunting);
            }
            break;
    }
}