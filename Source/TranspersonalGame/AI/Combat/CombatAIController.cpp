#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Perception/AIDamageConfig.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configure perception
    ConfigurePerception();
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnTargetPerceptionUpdated);
    }
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update aggression decay
    UpdateAggressionLevel(DeltaTime);

    // Analyze tactical situation periodically
    static float AnalysisTimer = 0.0f;
    AnalysisTimer += DeltaTime;
    if (AnalysisTimer >= 0.5f) // Analyze twice per second
    {
        AnalyzeTacticalSituation();
        AnalysisTimer = 0.0f;
    }

    // Update blackboard with current combat data
    UpdateBlackboardCombatData();
}

void ACombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BlackboardAsset && BehaviorTree)
    {
        UseBlackboard(BlackboardAsset);
        RunBehaviorTree(BehaviorTree);
    }
}

void ACombatAIController::ConfigurePerception()
{
    if (!AIPerceptionComponent) return;

    SetupSightSense();
    SetupHearingSense();

    // Set perception as dominant sense
    AIPerceptionComponent->SetDominantSense(*AIPerceptionComponent->GetSenseConfig<UAISightConfig>()->GetSenseImplementation());
}

void ACombatAIController::SetupSightSense()
{
    UAISightConfig* SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    
    // Predator sight configuration - wide field of view, long range
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2200.0f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->SetMaxAge(10.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
    
    // Detect players and other dinosaurs
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    AIPerceptionComponent->SetSenseConfig(*SightConfig);
}

void ACombatAIController::SetupHearingSense()
{
    UAIHearingConfig* HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    
    // Enhanced hearing for predators
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(5.0f);
    
    // Detect all sound types
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    AIPerceptionComponent->ConfigureSense(*HearingConfig);
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor) continue;

        FAIStimulus Stimulus;
        if (AIPerceptionComponent->GetActorsPerception(Actor, Stimulus))
        {
            if (Stimulus.WasSuccessfullySensed())
            {
                // Remember this target
                RememberTarget(Actor, Actor->GetActorLocation());
                
                // Assess threat level
                EThreatLevel ThreatLevel = AssessThreat(Actor);
                UpdateThreatLevel(Actor, ThreatLevel);
                
                // Update combat state based on threat
                if (ThreatLevel >= EThreatLevel::Medium && CurrentCombatState == ECombatState::Idle)
                {
                    SetCombatState(ECombatState::Investigating);
                }
            }
        }
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Update memory with fresh information
        RememberTarget(Actor, Stimulus.StimulusLocation);
        
        // Increase aggression if this is a repeated sighting
        if (HasMemoryOf(Actor))
        {
            CombatMemory.AggressionLevel = FMath::Min(1.0f, CombatMemory.AggressionLevel + 0.1f);
        }
    }
    else
    {
        // Lost sight - enter investigation mode if we were hunting
        if (CurrentCombatState == ECombatState::Hunting)
        {
            SetCombatState(ECombatState::Investigating);
        }
    }
}

void ACombatAIController::SetCombatState(ECombatState NewState)
{
    if (CurrentCombatState == NewState) return;

    // Prevent rapid state switching
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastCombatStateChange < StateChangeDelay) return;

    ECombatState PreviousState = CurrentCombatState;
    CurrentCombatState = NewState;
    LastCombatStateChange = CurrentTime;

    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
    }

    // State-specific logic
    switch (NewState)
    {
        case ECombatState::Hunting:
            CombatMemory.AggressionLevel = FMath::Min(1.0f, CombatMemory.AggressionLevel + 0.2f);
            break;
            
        case ECombatState::Fleeing:
            CombatMemory.AggressionLevel = FMath::Max(0.0f, CombatMemory.AggressionLevel - 0.3f);
            break;
            
        case ECombatState::Attacking:
            CombatMemory.AggressionLevel = 1.0f;
            break;
    }
}

EThreatLevel ACombatAIController::AssessThreat(AActor* Target)
{
    if (!Target || !GetPawn()) return EThreatLevel::None;

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    EThreatLevel ThreatLevel = EThreatLevel::None;

    // Check if target is player
    if (Target->IsA<ACharacter>() && Target->IsA<APawn>())
    {
        APawn* TargetPawn = Cast<APawn>(Target);
        if (TargetPawn && TargetPawn->IsPlayerControlled())
        {
            // Player is always a significant threat
            if (Distance < 500.0f)
                ThreatLevel = EThreatLevel::High;
            else if (Distance < 1000.0f)
                ThreatLevel = EThreatLevel::Medium;
            else
                ThreatLevel = EThreatLevel::Low;
        }
    }

    // Modify threat based on our current health
    if (GetPawn()->IsA<ACharacter>())
    {
        ACharacter* MyCharacter = Cast<ACharacter>(GetPawn());
        // Assume health component exists - this would need proper health system integration
        float HealthPercentage = 1.0f; // Placeholder
        
        if (HealthPercentage < FleeHealthThreshold)
        {
            // When low on health, everything is more threatening
            ThreatLevel = static_cast<EThreatLevel>(FMath::Min(static_cast<int32>(EThreatLevel::Critical), 
                                                               static_cast<int32>(ThreatLevel) + 1));
        }
    }

    return ThreatLevel;
}

void ACombatAIController::UpdateThreatLevel(AActor* Target, EThreatLevel NewThreat)
{
    if (!Target) return;

    if (TargetMemories.Contains(Target))
    {
        TargetMemories[Target].ThreatLevel = NewThreat;
    }
}

bool ACombatAIController::ShouldAttack(AActor* Target)
{
    if (!Target || !GetPawn()) return false;

    // Check if we're in an aggressive state
    if (CurrentCombatState != ECombatState::Hunting && CurrentCombatState != ECombatState::Attacking)
        return false;

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    
    // Check if target is in optimal attack range
    if (Distance > OptimalAttackRange * 1.5f) return false;

    // Check threat level
    EThreatLevel ThreatLevel = AssessThreat(Target);
    if (ThreatLevel < EThreatLevel::Medium) return false;

    // Check aggression level
    if (CombatMemory.AggressionLevel < 0.5f) return false;

    return true;
}

bool ACombatAIController::ShouldFlee(AActor* Target)
{
    if (!Target || !GetPawn()) return false;

    // Always flee if health is critically low
    // This would need proper health system integration
    float HealthPercentage = 1.0f; // Placeholder
    if (HealthPercentage < FleeHealthThreshold) return true;

    // Flee from overwhelming threats
    EThreatLevel ThreatLevel = AssessThreat(Target);
    if (ThreatLevel >= EThreatLevel::Critical) return true;

    // Flee if we've been severely damaged recently
    if (CombatMemory.bHasBeenAttacked && CombatMemory.AggressionLevel < 0.3f)
        return true;

    return false;
}

FVector ACombatAIController::FindOptimalAttackPosition(AActor* Target)
{
    if (!Target || !GetPawn()) return GetPawn()->GetActorLocation();

    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    // Find position at optimal attack range
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();
    FVector OptimalPosition = TargetLocation - (DirectionToTarget * OptimalAttackRange);

    // Use navigation system to find valid location
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLocation;
        if (NavSys->ProjectPointToNavigation(OptimalPosition, NavLocation, FVector(200.0f)))
        {
            return NavLocation.Location;
        }
    }

    return OptimalPosition;
}

FVector ACombatAIController::FindFleePosition(AActor* Threat)
{
    if (!Threat || !GetPawn()) return GetPawn()->GetActorLocation();

    FVector ThreatLocation = Threat->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    // Find position away from threat
    FVector FleeDirection = (MyLocation - ThreatLocation).GetSafeNormal();
    FVector FleePosition = MyLocation + (FleeDirection * 1000.0f);

    // Use navigation system to find valid location
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLocation;
        if (NavSys->ProjectPointToNavigation(FleePosition, NavLocation, FVector(500.0f)))
        {
            return NavLocation.Location;
        }
    }

    return FleePosition;
}

void ACombatAIController::RememberTarget(AActor* Target, const FVector& Location)
{
    if (!Target) return;

    FCombatMemory& Memory = TargetMemories.FindOrAdd(Target);
    Memory.LastKnownTarget = Target;
    Memory.LastKnownLocation = Location;
    Memory.LastSeenTime = GetWorld()->GetTimeSeconds();
}

void ACombatAIController::ForgetTarget(AActor* Target)
{
    if (Target)
    {
        TargetMemories.Remove(Target);
    }
}

bool ACombatAIController::HasMemoryOf(AActor* Target) const
{
    return Target && TargetMemories.Contains(Target);
}

void ACombatAIController::UpdateBlackboardCombatData()
{
    if (!BlackboardComponent) return;

    // Update combat state
    BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
    
    // Update aggression level
    BlackboardComponent->SetValueAsFloat(TEXT("AggressionLevel"), CombatMemory.AggressionLevel);
    
    // Update target information
    if (CombatMemory.LastKnownTarget)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), CombatMemory.LastKnownTarget);
        BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), CombatMemory.LastKnownLocation);
    }
    
    // Update threat level
    BlackboardComponent->SetValueAsEnum(TEXT("ThreatLevel"), static_cast<uint8>(CombatMemory.ThreatLevel));
}

void ACombatAIController::AnalyzeTacticalSituation()
{
    if (!GetPawn()) return;

    // Check if we're in our territory
    bool bInTerritory = IsInTerritory(GetPawn()->GetActorLocation());
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("InTerritory"), bInTerritory);
    }

    // Analyze nearby threats
    TArray<AActor*> PerceivedActors;
    AIPerceptionComponent->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);

    int32 ThreatCount = 0;
    for (AActor* Actor : PerceivedActors)
    {
        if (AssessThreat(Actor) >= EThreatLevel::Medium)
        {
            ThreatCount++;
        }
    }

    // Update tactical state based on analysis
    if (ThreatCount > 1 && CurrentCombatState == ECombatState::Hunting)
    {
        // Multiple threats - consider retreating
        SetCombatState(ECombatState::Fleeing);
    }
    else if (ThreatCount == 0 && CurrentCombatState == ECombatState::Investigating)
    {
        // No threats found - return to patrol
        SetCombatState(ECombatState::Patrolling);
    }
}

void ACombatAIController::UpdateAggressionLevel(float DeltaTime)
{
    // Gradually decay aggression over time
    CombatMemory.AggressionLevel = FMath::Max(0.0f, CombatMemory.AggressionLevel - (AggressionDecayRate * DeltaTime));
    
    // Reset attack flag if aggression is very low
    if (CombatMemory.AggressionLevel < 0.1f)
    {
        CombatMemory.bHasBeenAttacked = false;
    }
}

bool ACombatAIController::IsInTerritory(const FVector& Location) const
{
    if (!GetPawn()) return false;

    // Simple territorial check - could be enhanced with actual territory data
    FVector HomeLocation = GetPawn()->GetActorLocation(); // This should be the spawn location or territory center
    float DistanceFromHome = FVector::Dist(Location, HomeLocation);
    
    return DistanceFromHome <= TerritorialRadius;
}