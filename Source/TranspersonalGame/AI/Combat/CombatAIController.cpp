#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "EnvironmentalQuery/EnvQuery.h"
#include "EnvironmentalQuery/EnvQueryManager.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Perception Component
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    SetPerceptionComponent(*AIPerceptionComponent);

    // Configure Sight Sense
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2200.0f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->SetMaxAge(10.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure Hearing Sense
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(5.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure Damage Sense
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    DamageConfig->SetMaxAge(10.0f);

    // Add senses to perception component
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->ConfigureSense(*DamageConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize personality with random traits
    InitializePersonality();
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

    // Start behavior tree if assigned
    if (BehaviorTree)
    {
        RunBehaviorTree(BehaviorTree);
    }

    // Initialize blackboard values
    UpdateBlackboardValues();
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update combat memory
    UpdateCombatMemory(DeltaTime);

    // Evaluate current combat situation
    EvaluateCombatSituation();

    // Update blackboard with current state
    UpdateBlackboardValues();

    StateChangeTimer += DeltaTime;
    LastPerceptionUpdate += DeltaTime;
}

void ACombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // Apply personality-based modifications to the possessed pawn
    ModifyBehaviorByPersonality();
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    LastPerceptionUpdate = 0.0f;

    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            // Check if this is the player character
            if (Actor->ActorHasTag("Player"))
            {
                UpdatePlayerMemory(Actor->GetActorLocation());
                
                // Assess threat level
                EThreatLevel ThreatLevel = AssessThreatLevel(Actor);
                CombatMemory.PlayerThreatLevel = ThreatLevel;

                // Decide on combat response based on personality
                if (ShouldEngageTarget(Actor))
                {
                    SetCombatState(ECombatState::Aggressive);
                }
                else if (ShouldStalkTarget(Actor))
                {
                    SetCombatState(ECombatState::Stalking);
                }
                else if (ShouldFleeFromTarget(Actor))
                {
                    SetCombatState(ECombatState::Fleeing);
                }
                else
                {
                    SetCombatState(ECombatState::Alert);
                }
            }
        }
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    if (!BlackboardComp) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Target acquired
        BlackboardComp->SetValueAsObject("TargetActor", Actor);
        BlackboardComp->SetValueAsVector("TargetLocation", Actor->GetActorLocation());
        
        if (Actor->ActorHasTag("Player"))
        {
            UpdatePlayerMemory(Actor->GetActorLocation());
            
            // Personality-driven reaction to player detection
            float ReactionIntensity = Personality.Aggression + Personality.Curiosity;
            if (ReactionIntensity > 1.0f)
            {
                SetCombatState(ECombatState::Hunting);
            }
            else if (Personality.FearThreshold < 0.3f)
            {
                SetCombatState(ECombatState::Alert);
            }
        }
    }
    else
    {
        // Target lost
        if (CurrentCombatState == ECombatState::Aggressive || CurrentCombatState == ECombatState::Hunting)
        {
            // Use memory to continue pursuit
            if (CombatMemory.TimeSinceLastPlayerSighting < 30.0f * Personality.Persistence)
            {
                SetCombatState(ECombatState::Stalking);
                BlackboardComp->SetValueAsVector("LastKnownLocation", CombatMemory.LastKnownPlayerLocation);
            }
            else
            {
                SetCombatState(ECombatState::Alert);
                BlackboardComp->ClearValue("TargetActor");
            }
        }
    }
}

void ACombatAIController::SetCombatState(ECombatState NewState)
{
    if (CurrentCombatState == NewState) return;

    ECombatState PreviousState = CurrentCombatState;
    CurrentCombatState = NewState;
    StateChangeTimer = 0.0f;

    HandleStateTransition(NewState);

    // Update blackboard
    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsEnum("CombatState", static_cast<uint8>(NewState));
    }

    // Debug logging
    UE_LOG(LogTemp, Log, TEXT("Combat AI State Changed: %s -> %s"), 
           *UEnum::GetValueAsString(PreviousState), 
           *UEnum::GetValueAsString(NewState));
}

EThreatLevel ACombatAIController::AssessThreatLevel(AActor* Target)
{
    if (!Target) return EThreatLevel::None;

    float Distance = CalculateDistanceToPlayer();
    bool bInLineOfSight = IsPlayerInLineOfSight();
    
    // Base threat assessment
    EThreatLevel ThreatLevel = EThreatLevel::Low;

    // Distance-based threat
    if (Distance < 500.0f)
    {
        ThreatLevel = EThreatLevel::High;
    }
    else if (Distance < 1000.0f)
    {
        ThreatLevel = EThreatLevel::Medium;
    }

    // Line of sight modifier
    if (bInLineOfSight)
    {
        ThreatLevel = static_cast<EThreatLevel>(FMath::Min(static_cast<int32>(ThreatLevel) + 1, static_cast<int32>(EThreatLevel::Extreme)));
    }

    // Personality modifier
    if (Personality.FearThreshold < 0.3f)
    {
        // Easily spooked - increase threat perception
        ThreatLevel = static_cast<EThreatLevel>(FMath::Min(static_cast<int32>(ThreatLevel) + 1, static_cast<int32>(EThreatLevel::Extreme)));
    }
    else if (Personality.Aggression > 0.7f)
    {
        // Highly aggressive - decrease threat perception
        ThreatLevel = static_cast<EThreatLevel>(FMath::Max(static_cast<int32>(ThreatLevel) - 1, static_cast<int32>(EThreatLevel::None)));
    }

    return ThreatLevel;
}

void ACombatAIController::UpdatePlayerMemory(const FVector& PlayerLocation)
{
    CombatMemory.LastKnownPlayerLocation = PlayerLocation;
    CombatMemory.TimeSinceLastPlayerSighting = 0.0f;
    CombatMemory.PlayerSpottingCount++;

    // Add to sighting history (keep last 10 sightings)
    CombatMemory.PlayerSightingHistory.Add(PlayerLocation);
    CombatMemory.SightingTimeStamps.Add(GetWorld()->GetTimeSeconds());

    if (CombatMemory.PlayerSightingHistory.Num() > 10)
    {
        CombatMemory.PlayerSightingHistory.RemoveAt(0);
        CombatMemory.SightingTimeStamps.RemoveAt(0);
    }
}

bool ACombatAIController::ShouldEngageTarget(AActor* Target)
{
    if (!Target) return false;

    float Distance = CalculateDistanceToPlayer();
    EThreatLevel ThreatLevel = AssessThreatLevel(Target);

    // Personality-driven decision
    float EngagementScore = Personality.Aggression * 0.4f + 
                           Personality.Territoriality * 0.3f + 
                           (1.0f - Personality.FearThreshold) * 0.3f;

    // Distance modifier
    if (Distance < 300.0f)
    {
        EngagementScore += 0.3f;
    }
    else if (Distance > 1500.0f)
    {
        EngagementScore -= 0.2f;
    }

    // Threat level modifier
    if (ThreatLevel == EThreatLevel::High || ThreatLevel == EThreatLevel::Extreme)
    {
        EngagementScore += 0.2f;
    }

    return EngagementScore > 0.6f;
}

bool ACombatAIController::ShouldFleeFromTarget(AActor* Target)
{
    if (!Target) return false;

    float Distance = CalculateDistanceToPlayer();
    EThreatLevel ThreatLevel = AssessThreatLevel(Target);

    // Personality-driven decision
    float FleeScore = Personality.FearThreshold * 0.5f + 
                     (1.0f - Personality.Aggression) * 0.3f + 
                     (1.0f - Personality.Territoriality) * 0.2f;

    // Distance modifier
    if (Distance < 200.0f)
    {
        FleeScore += 0.4f;
    }

    // Threat level modifier
    if (ThreatLevel == EThreatLevel::Extreme)
    {
        FleeScore += 0.3f;
    }

    // Health modifier (if implemented)
    // if (CurrentHealth < MaxHealth * 0.3f) FleeScore += 0.3f;

    return FleeScore > 0.7f;
}

bool ACombatAIController::ShouldStalkTarget(AActor* Target)
{
    if (!Target) return false;

    // Stalking is preferred when curious but not overly aggressive
    float StalkScore = Personality.Curiosity * 0.4f + 
                      Personality.Persistence * 0.3f + 
                      (1.0f - Personality.FearThreshold) * 0.2f - 
                      Personality.Aggression * 0.1f;

    float Distance = CalculateDistanceToPlayer();
    
    // Distance sweet spot for stalking
    if (Distance > 500.0f && Distance < 1500.0f)
    {
        StalkScore += 0.2f;
    }

    return StalkScore > 0.6f && !ShouldEngageTarget(Target) && !ShouldFleeFromTarget(Target);
}

void ACombatAIController::ExecuteTacticalQuery(UEnvQuery* Query, FName BlackboardKey)
{
    if (!Query || !GetWorld()) return;

    FEnvQueryRequest QueryRequest(Query, GetPawn());
    QueryRequest.SetFloatParam("SearchRadius", 1000.0f);
    QueryRequest.SetFloatParam("MinDistance", 300.0f);
    
    FQueryFinishedSignature QueryFinishedDelegate;
    QueryFinishedDelegate.AddDynamic(this, &ACombatAIController::OnEQSQueryComplete);
    
    FEnvQueryResult QueryResult = FEnvQueryManager::RunInstantQuery(GetWorld(), QueryRequest, EEnvQueryRunMode::SingleResult);
    
    if (QueryResult.IsSuccessful())
    {
        UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
        if (BlackboardComp && QueryResult.Items.Num() > 0)
        {
            BlackboardComp->SetValueAsVector(BlackboardKey, QueryResult.GetItemAsLocation(0));
        }
    }
}

float ACombatAIController::GetPersonalityModifier(const FString& TraitName) const
{
    if (TraitName == "Aggression") return Personality.Aggression;
    if (TraitName == "Curiosity") return Personality.Curiosity;
    if (TraitName == "FearThreshold") return Personality.FearThreshold;
    if (TraitName == "Territoriality") return Personality.Territoriality;
    if (TraitName == "PackMentality") return Personality.PackMentality;
    if (TraitName == "Persistence") return Personality.Persistence;
    
    return 0.5f; // Default value
}

void ACombatAIController::ModifyBehaviorByPersonality()
{
    // Apply personality traits to AI behavior
    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    if (!BlackboardComp) return;

    // Set personality values in blackboard for behavior tree access
    BlackboardComp->SetValueAsFloat("Aggression", Personality.Aggression);
    BlackboardComp->SetValueAsFloat("Curiosity", Personality.Curiosity);
    BlackboardComp->SetValueAsFloat("FearThreshold", Personality.FearThreshold);
    BlackboardComp->SetValueAsFloat("Territoriality", Personality.Territoriality);
    BlackboardComp->SetValueAsFloat("PackMentality", Personality.PackMentality);
    BlackboardComp->SetValueAsFloat("Persistence", Personality.Persistence);

    // Modify AI perception based on personality
    if (AIPerceptionComponent)
    {
        // Aggressive dinosaurs have better sight range
        if (Personality.Aggression > 0.7f)
        {
            // Increase sight radius for aggressive individuals
            // This would require accessing the sight config and modifying it
        }

        // Curious dinosaurs have better hearing
        if (Personality.Curiosity > 0.7f)
        {
            // Increase hearing range for curious individuals
        }
    }
}

void ACombatAIController::InitializePersonality()
{
    // Generate unique personality traits
    Personality = FCombatPersonality();
    
    // Ensure some variation in personality archetypes
    float PersonalityType = FMath::RandRange(0.0f, 1.0f);
    
    if (PersonalityType < 0.2f)
    {
        // Aggressive Hunter
        Personality.Aggression = FMath::RandRange(0.7f, 1.0f);
        Personality.FearThreshold = FMath::RandRange(0.1f, 0.4f);
        Personality.Persistence = FMath::RandRange(0.6f, 1.0f);
    }
    else if (PersonalityType < 0.4f)
    {
        // Cautious Stalker
        Personality.Curiosity = FMath::RandRange(0.6f, 0.9f);
        Personality.FearThreshold = FMath::RandRange(0.5f, 0.8f);
        Personality.Aggression = FMath::RandRange(0.2f, 0.5f);
    }
    else if (PersonalityType < 0.6f)
    {
        // Territorial Guardian
        Personality.Territoriality = FMath::RandRange(0.7f, 1.0f);
        Personality.Aggression = FMath::RandRange(0.5f, 0.8f);
        Personality.FearThreshold = FMath::RandRange(0.2f, 0.5f);
    }
    else if (PersonalityType < 0.8f)
    {
        // Skittish Runner
        Personality.FearThreshold = FMath::RandRange(0.7f, 1.0f);
        Personality.Aggression = FMath::RandRange(0.1f, 0.3f);
        Personality.Curiosity = FMath::RandRange(0.3f, 0.6f);
    }
    else
    {
        // Balanced Individual
        // Keep the random values generated in the constructor
    }
}

void ACombatAIController::UpdateCombatMemory(float DeltaTime)
{
    CombatMemory.TimeSinceLastPlayerSighting += DeltaTime;

    // Decay old memories
    float CurrentTime = GetWorld()->GetTimeSeconds();
    for (int32 i = CombatMemory.SightingTimeStamps.Num() - 1; i >= 0; i--)
    {
        if (CurrentTime - CombatMemory.SightingTimeStamps[i] > 300.0f) // 5 minutes
        {
            CombatMemory.SightingTimeStamps.RemoveAt(i);
            CombatMemory.PlayerSightingHistory.RemoveAt(i);
        }
    }
}

void ACombatAIController::EvaluateCombatSituation()
{
    // Periodic evaluation of combat situation
    if (StateChangeTimer < 2.0f) return; // Don't change states too frequently

    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    if (!BlackboardComp) return;

    AActor* CurrentTarget = Cast<AActor>(BlackboardComp->GetValueAsObject("TargetActor"));
    
    if (!CurrentTarget && CombatMemory.TimeSinceLastPlayerSighting > 60.0f)
    {
        // No target and no recent memory - return to passive state
        if (CurrentCombatState != ECombatState::Passive)
        {
            SetCombatState(ECombatState::Passive);
        }
    }
    else if (CurrentTarget)
    {
        // Re-evaluate current target
        if (ShouldFleeFromTarget(CurrentTarget) && CurrentCombatState != ECombatState::Fleeing)
        {
            SetCombatState(ECombatState::Fleeing);
        }
        else if (ShouldEngageTarget(CurrentTarget) && CurrentCombatState != ECombatState::Aggressive)
        {
            SetCombatState(ECombatState::Aggressive);
        }
    }
}

void ACombatAIController::HandleStateTransition(ECombatState NewState)
{
    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    if (!BlackboardComp) return;

    switch (NewState)
    {
    case ECombatState::Aggressive:
        // Execute flanking query to find optimal attack position
        if (FindFlankingQuery)
        {
            ExecuteTacticalQuery(FindFlankingQuery, "FlankingPosition");
        }
        break;

    case ECombatState::Stalking:
        // Execute ambush query to find concealed position
        if (FindAmbushQuery)
        {
            ExecuteTacticalQuery(FindAmbushQuery, "AmbushPosition");
        }
        break;

    case ECombatState::Fleeing:
        // Execute escape query to find safe retreat
        if (FindEscapeQuery)
        {
            ExecuteTacticalQuery(FindEscapeQuery, "EscapePosition");
        }
        break;

    case ECombatState::Alert:
        // Look for cover while investigating
        if (FindCoverQuery)
        {
            ExecuteTacticalQuery(FindCoverQuery, "CoverPosition");
        }
        break;

    default:
        break;
    }
}

float ACombatAIController::CalculateDistanceToPlayer() const
{
    if (CombatMemory.LastKnownPlayerLocation != FVector::ZeroVector && GetPawn())
    {
        return FVector::Dist(GetPawn()->GetActorLocation(), CombatMemory.LastKnownPlayerLocation);
    }
    return 10000.0f; // Very large distance if no known location
}

bool ACombatAIController::IsPlayerInLineOfSight() const
{
    if (!GetPawn() || CombatMemory.LastKnownPlayerLocation == FVector::ZeroVector)
    {
        return false;
    }

    FHitResult HitResult;
    FVector StartLocation = GetPawn()->GetActorLocation();
    FVector EndLocation = CombatMemory.LastKnownPlayerLocation;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetPawn());

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_Visibility,
        QueryParams
    );

    return !bHit || (HitResult.GetActor() && HitResult.GetActor()->ActorHasTag("Player"));
}

void ACombatAIController::UpdateBlackboardValues()
{
    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    if (!BlackboardComp) return;

    // Update combat state
    BlackboardComp->SetValueAsEnum("CombatState", static_cast<uint8>(CurrentCombatState));
    
    // Update threat level
    BlackboardComp->SetValueAsEnum("ThreatLevel", static_cast<uint8>(CombatMemory.PlayerThreatLevel));
    
    // Update memory values
    BlackboardComp->SetValueAsVector("LastKnownPlayerLocation", CombatMemory.LastKnownPlayerLocation);
    BlackboardComp->SetValueAsFloat("TimeSinceLastSighting", CombatMemory.TimeSinceLastPlayerSighting);
    BlackboardComp->SetValueAsInt("PlayerSpottingCount", CombatMemory.PlayerSpottingCount);
    
    // Update distance to player
    BlackboardComp->SetValueAsFloat("DistanceToPlayer", CalculateDistanceToPlayer());
    
    // Update line of sight
    BlackboardComp->SetValueAsBool("HasLineOfSight", IsPlayerInLineOfSight());
}