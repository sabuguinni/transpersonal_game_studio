#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Set default values
    CurrentCombatState = ECombatState::Idle;
    CurrentThreatLevel = EThreatLevel::None;
    DinosaurArchetype = EDinosaurArchetype::PackHunter;
    
    LastPlayerSightingTime = -1.0f;
    PlayerEncounterCount = 0;
    bHasBeenHurtByPlayer = false;
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();
    
    SetupPerception();
    InitializeIndividualTraits();
    SetupBlackboardKeys();
    
    // Start appropriate behavior tree based on archetype
    if (ArchetypeBehaviorTrees.Contains(DinosaurArchetype))
    {
        RunBehaviorTree(ArchetypeBehaviorTrees[DinosaurArchetype]);
    }
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateCombatLogic(DeltaTime);
}

void ACombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (CombatBlackboard)
    {
        UseBlackboard(CombatBlackboard);
    }
}

void ACombatAIController::SetupPerception()
{
    if (!AIPerceptionComponent)
        return;

    // Configure Sight Sense
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 3000.0f;
    SightConfig->LoseSightRadius = 3500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure Hearing Sense
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 2000.0f;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure Damage Sense
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    DamageConfig->Implementation = UAISense_Damage::StaticClass();

    // Add senses to perception component
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->ConfigureSense(*DamageConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnTargetPerceptionUpdated);
}

void ACombatAIController::SetupBlackboardKeys()
{
    if (!GetBlackboardComponent())
        return;

    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    
    // Set initial blackboard values
    BlackboardComp->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
    BlackboardComp->SetValueAsEnum(TEXT("ThreatLevel"), static_cast<uint8>(CurrentThreatLevel));
    BlackboardComp->SetValueAsEnum(TEXT("DinosaurArchetype"), static_cast<uint8>(DinosaurArchetype));
    
    BlackboardComp->SetValueAsFloat(TEXT("AggressionLevel"), IndividualAggression);
    BlackboardComp->SetValueAsFloat(TEXT("CourageLevel"), IndividualCourage);
    BlackboardComp->SetValueAsFloat(TEXT("IntelligenceLevel"), IndividualIntelligence);
    BlackboardComp->SetValueAsFloat(TEXT("TerritorialLevel"), IndividualTerritoriality);
    
    BlackboardComp->SetValueAsFloat(TEXT("TerritorialRadius"), TerritorialRadius);
    BlackboardComp->SetValueAsFloat(TEXT("HuntingRange"), HuntingRange);
    BlackboardComp->SetValueAsFloat(TEXT("FleeDistance"), FleeDistance);
}

void ACombatAIController::InitializeIndividualTraits()
{
    // Generate individual personality traits with some randomization
    // This creates unique dinosaurs within the same species
    
    float BaseAggression = 0.5f;
    float BaseCourage = 0.5f;
    float BaseIntelligence = 0.5f;
    float BaseTerritoriality = 0.5f;
    
    // Adjust base values based on archetype
    switch (DinosaurArchetype)
    {
        case EDinosaurArchetype::ApexPredator:
            BaseAggression = 0.8f;
            BaseCourage = 0.9f;
            BaseIntelligence = 0.6f;
            BaseTerritoriality = 0.7f;
            break;
            
        case EDinosaurArchetype::PackHunter:
            BaseAggression = 0.7f;
            BaseCourage = 0.6f;
            BaseIntelligence = 0.8f;
            BaseTerritoriality = 0.4f;
            break;
            
        case EDinosaurArchetype::AmbushPredator:
            BaseAggression = 0.6f;
            BaseCourage = 0.5f;
            BaseIntelligence = 0.7f;
            BaseTerritoriality = 0.6f;
            break;
            
        case EDinosaurArchetype::HerbivoreDefensive:
            BaseAggression = 0.3f;
            BaseCourage = 0.7f;
            BaseIntelligence = 0.5f;
            BaseTerritoriality = 0.8f;
            break;
            
        case EDinosaurArchetype::HerbivoreFlighty:
            BaseAggression = 0.1f;
            BaseCourage = 0.2f;
            BaseIntelligence = 0.6f;
            BaseTerritoriality = 0.3f;
            break;
            
        case EDinosaurArchetype::Scavenger:
            BaseAggression = 0.4f;
            BaseCourage = 0.3f;
            BaseIntelligence = 0.7f;
            BaseTerritoriality = 0.2f;
            break;
    }
    
    // Add individual variation (±30% of base values)
    float Variation = 0.3f;
    IndividualAggression = FMath::Clamp(BaseAggression + FMath::RandRange(-Variation, Variation), 0.0f, 1.0f);
    IndividualCourage = FMath::Clamp(BaseCourage + FMath::RandRange(-Variation, Variation), 0.0f, 1.0f);
    IndividualIntelligence = FMath::Clamp(BaseIntelligence + FMath::RandRange(-Variation, Variation), 0.0f, 1.0f);
    IndividualTerritoriality = FMath::Clamp(BaseTerritoriality + FMath::RandRange(-Variation, Variation), 0.0f, 1.0f);
}

void ACombatAIController::UpdateCombatLogic(float DeltaTime)
{
    if (!GetPawn())
        return;

    // Update memory decay
    if (LastPlayerSightingTime > 0)
    {
        float TimeSinceLastSighting = GetWorld()->GetTimeSeconds() - LastPlayerSightingTime;
        
        // Gradually forget player location if not seen for a while
        if (TimeSinceLastSighting > 60.0f) // 1 minute
        {
            if (KnownPlayerLocations.Num() > 0)
            {
                KnownPlayerLocations.RemoveAt(0);
            }
        }
    }
    
    // Adapt behavior based on experience
    AdaptBehaviorBasedOnExperience();
    
    // Update blackboard with current state
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        BlackboardComp->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
        BlackboardComp->SetValueAsEnum(TEXT("ThreatLevel"), static_cast<uint8>(CurrentThreatLevel));
    }
}

void ACombatAIController::SetCombatState(ECombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;
        
        if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
        {
            BlackboardComp->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
        }
        
        // Trigger state-specific behaviors
        switch (NewState)
        {
            case ECombatState::Hunting:
                ExecutePackHuntingBehavior();
                break;
            case ECombatState::Territorial:
                ExecuteTerritoralBehavior();
                break;
            case ECombatState::Stalking:
                ExecuteAmbushBehavior();
                break;
        }
    }
}

void ACombatAIController::UpdateThreatLevel(AActor* ThreatActor)
{
    if (!ThreatActor)
    {
        CurrentThreatLevel = EThreatLevel::None;
        return;
    }
    
    float ThreatScore = CalculateThreatScore(ThreatActor);
    
    if (ThreatScore >= 0.8f)
        CurrentThreatLevel = EThreatLevel::Critical;
    else if (ThreatScore >= 0.6f)
        CurrentThreatLevel = EThreatLevel::High;
    else if (ThreatScore >= 0.4f)
        CurrentThreatLevel = EThreatLevel::Medium;
    else if (ThreatScore >= 0.2f)
        CurrentThreatLevel = EThreatLevel::Low;
    else
        CurrentThreatLevel = EThreatLevel::None;
}

float ACombatAIController::CalculateThreatScore(AActor* Actor)
{
    if (!Actor || !GetPawn())
        return 0.0f;
    
    float ThreatScore = 0.0f;
    
    // Distance factor (closer = more threatening)
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
    float DistanceFactor = FMath::Clamp(1.0f - (Distance / 2000.0f), 0.0f, 1.0f);
    ThreatScore += DistanceFactor * 0.4f;
    
    // Player-specific threat
    if (Actor->IsA<ACharacter>() && Cast<ACharacter>(Actor)->IsPlayerControlled())
    {
        ThreatScore += 0.3f;
        
        // Increase threat if player has hurt this dinosaur before
        if (bHasBeenHurtByPlayer)
        {
            ThreatScore += 0.2f;
        }
        
        // Increase threat based on encounter count
        ThreatScore += FMath::Min(PlayerEncounterCount * 0.1f, 0.3f);
    }
    
    // Modify based on individual courage (less courageous = higher threat perception)
    ThreatScore *= (2.0f - IndividualCourage);
    
    return FMath::Clamp(ThreatScore, 0.0f, 1.0f);
}

bool ACombatAIController::ShouldEngageTarget(AActor* Target)
{
    if (!Target)
        return false;
    
    float ThreatScore = CalculateThreatScore(Target);
    float EngageThreshold = 1.0f - IndividualAggression;
    
    // Apex predators are more likely to engage
    if (DinosaurArchetype == EDinosaurArchetype::ApexPredator)
    {
        EngageThreshold *= 0.7f;
    }
    
    // Pack hunters engage when in groups
    if (DinosaurArchetype == EDinosaurArchetype::PackHunter)
    {
        // TODO: Check for nearby allies
        EngageThreshold *= 0.8f;
    }
    
    return ThreatScore < EngageThreshold;
}

bool ACombatAIController::ShouldFleeFromTarget(AActor* Target)
{
    if (!Target)
        return false;
    
    float ThreatScore = CalculateThreatScore(Target);
    float FleeThreshold = IndividualCourage;
    
    // Herbivores flee more easily
    if (DinosaurArchetype == EDinosaurArchetype::HerbivoreFlighty)
    {
        FleeThreshold *= 1.5f;
    }
    
    return ThreatScore > FleeThreshold;
}

FVector ACombatAIController::GetOptimalAttackPosition(AActor* Target)
{
    if (!Target || !GetPawn())
        return GetPawn()->GetActorLocation();
    
    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    // Different archetypes prefer different attack positions
    switch (DinosaurArchetype)
    {
        case EDinosaurArchetype::AmbushPredator:
        {
            // Find cover near target
            // TODO: Use EQS to find optimal ambush position
            FVector Direction = (TargetLocation - MyLocation).GetSafeNormal();
            return TargetLocation - Direction * 800.0f; // Stay at medium distance
        }
        
        case EDinosaurArchetype::PackHunter:
        {
            // Flank the target
            FVector Direction = (TargetLocation - MyLocation).GetSafeNormal();
            FVector RightVector = FVector::CrossProduct(Direction, FVector::UpVector);
            return TargetLocation + RightVector * 500.0f; // Flank position
        }
        
        case EDinosaurArchetype::ApexPredator:
        {
            // Direct approach
            FVector Direction = (TargetLocation - MyLocation).GetSafeNormal();
            return TargetLocation - Direction * 300.0f; // Close range
        }
        
        default:
            return TargetLocation;
    }
}

FVector ACombatAIController::GetFleeDirection(AActor* ThreatSource)
{
    if (!ThreatSource || !GetPawn())
        return GetPawn()->GetActorLocation();
    
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector ThreatLocation = ThreatSource->GetActorLocation();
    FVector FleeDirection = (MyLocation - ThreatLocation).GetSafeNormal();
    
    return MyLocation + FleeDirection * FleeDistance;
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>() && Cast<ACharacter>(Actor)->IsPlayerControlled())
        {
            RememberPlayerLocation(Actor->GetActorLocation());
            LastPlayerSightingTime = GetWorld()->GetTimeSeconds();
            PlayerEncounterCount++;
            
            UpdateThreatLevel(Actor);
            
            // Update blackboard with player reference
            if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
            {
                BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Actor);
                BlackboardComp->SetValueAsVector(TEXT("LastKnownPlayerLocation"), Actor->GetActorLocation());
            }
        }
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
        return;
    
    // Handle different stimulus types
    if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            // Target spotted
            if (Actor->IsA<ACharacter>() && Cast<ACharacter>(Actor)->IsPlayerControlled())
            {
                SetCombatState(ECombatState::Stalking);
            }
        }
        else
        {
            // Target lost
            SetCombatState(ECombatState::Investigating);
        }
    }
    else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
    {
        // Investigate sound
        SetCombatState(ECombatState::Investigating);
        
        if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
        {
            BlackboardComp->SetValueAsVector(TEXT("InvestigateLocation"), Stimulus.StimulusLocation);
        }
    }
    else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
    {
        // React to damage
        bHasBeenHurtByPlayer = true;
        
        if (ShouldFleeFromTarget(Actor))
        {
            SetCombatState(ECombatState::Fleeing);
        }
        else
        {
            SetCombatState(ECombatState::Attacking);
        }
    }
}

void ACombatAIController::RememberPlayerLocation(FVector Location)
{
    KnownPlayerLocations.Add(Location);
    
    // Keep only the last 10 locations to prevent memory bloat
    if (KnownPlayerLocations.Num() > 10)
    {
        KnownPlayerLocations.RemoveAt(0);
    }
}

FVector ACombatAIController::GetLastKnownPlayerLocation()
{
    if (KnownPlayerLocations.Num() > 0)
    {
        return KnownPlayerLocations.Last();
    }
    
    return FVector::ZeroVector;
}

bool ACombatAIController::HasRecentPlayerMemory(float TimeThreshold)
{
    if (LastPlayerSightingTime < 0)
        return false;
    
    float TimeSinceLastSighting = GetWorld()->GetTimeSeconds() - LastPlayerSightingTime;
    return TimeSinceLastSighting <= TimeThreshold;
}

void ACombatAIController::ExecuteAmbushBehavior()
{
    // Ambush predators wait in cover and strike when prey is close
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        BlackboardComp->SetValueAsBool(TEXT("ShouldAmbush"), true);
        BlackboardComp->SetValueAsFloat(TEXT("AmbushRange"), 600.0f);
    }
}

void ACombatAIController::ExecutePackHuntingBehavior()
{
    // Pack hunters coordinate with nearby allies
    // TODO: Implement pack coordination system
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        BlackboardComp->SetValueAsBool(TEXT("UsePackTactics"), true);
        BlackboardComp->SetValueAsFloat(TEXT("FlankDistance"), 800.0f);
    }
}

void ACombatAIController::ExecuteTerritoralBehavior()
{
    // Territorial dinosaurs defend their area aggressively
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        BlackboardComp->SetValueAsBool(TEXT("DefendTerritory"), true);
        BlackboardComp->SetValueAsFloat(TEXT("TerritoryRadius"), TerritorialRadius * IndividualTerritoriality);
    }
}

void ACombatAIController::AdaptBehaviorBasedOnExperience()
{
    // Dinosaurs become more cautious after negative encounters with the player
    if (bHasBeenHurtByPlayer)
    {
        // Reduce aggression and increase caution
        IndividualAggression = FMath::Max(IndividualAggression - 0.1f, 0.1f);
        IndividualCourage = FMath::Max(IndividualCourage - 0.1f, 0.1f);
        
        // Update blackboard
        if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
        {
            BlackboardComp->SetValueAsFloat(TEXT("AggressionLevel"), IndividualAggression);
            BlackboardComp->SetValueAsFloat(TEXT("CourageLevel"), IndividualCourage);
        }
    }
    
    // Increase intelligence slightly with each encounter
    if (PlayerEncounterCount > 0)
    {
        IndividualIntelligence = FMath::Min(IndividualIntelligence + 0.01f, 1.0f);
        
        if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
        {
            BlackboardComp->SetValueAsFloat(TEXT("IntelligenceLevel"), IndividualIntelligence);
        }
    }
}