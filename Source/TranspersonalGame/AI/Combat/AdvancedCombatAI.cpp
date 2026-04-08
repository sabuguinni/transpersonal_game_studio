#include "AdvancedCombatAI.h"
#include "TacticalCombatAnalyzer.h"
#include "CombatTensionManager.h"
#include "PlayerBehaviorAnalyzer.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AAdvancedCombatAI::AAdvancedCombatAI()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    SetPerceptionComponent(*AIPerceptionComponent);
    
    // Initialize tactical components
    TacticalAnalyzer = CreateDefaultSubobject<UTacticalCombatAnalyzer>(TEXT("TacticalAnalyzer"));
    TensionManager = CreateDefaultSubobject<UCombatTensionManager>(TEXT("TensionManager"));
    BehaviorAnalyzer = CreateDefaultSubobject<UPlayerBehaviorAnalyzer>(TEXT("BehaviorAnalyzer"));
    
    // Configure perception senses
    SetupPerceptionSenses();
    
    // Initialize combat personality with random variation
    CombatPersonality = FAdvancedCombatPersonality();
    
    // Set default behavior tree and blackboard
    static ConstructorHelpers::FObjectFinder<UBehaviorTree> BehaviorTreeAsset(TEXT("/Game/AI/BehaviorTrees/BT_AdvancedCombat"));
    if (BehaviorTreeAsset.Succeeded())
    {
        CombatBehaviorTree = BehaviorTreeAsset.Object;
    }
    
    static ConstructorHelpers::FObjectFinder<UBlackboardAsset> BlackboardAsset(TEXT("/Game/AI/Blackboards/BB_AdvancedCombat"));
    if (BlackboardAsset.Succeeded())
    {
        CombatBlackboard = BlackboardAsset.Object;
    }
}

void AAdvancedCombatAI::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize combat memory
    CombatMemory.Reset();
    
    // Set up perception delegates
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AAdvancedCombatAI::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionForgotten.AddDynamic(this, &AAdvancedCombatAI::OnTargetForgotten);
    }
    
    // Start behavior tree
    if (CombatBehaviorTree && CombatBlackboard)
    {
        UseBlackboard(CombatBlackboard);
        RunBehaviorTree(CombatBehaviorTree);
    }
    
    // Initialize tactical analysis timer
    GetWorld()->GetTimerManager().SetTimer(TacticalAnalysisTimer, this, &AAdvancedCombatAI::PerformTacticalAnalysis, 0.5f, true);
    
    UE_LOG(LogTemp, Log, TEXT("AdvancedCombatAI: %s initialized with personality type: %s"), 
           *GetName(), 
           *UEnum::GetValueAsString(CombatPersonality.PersonalityType));
}

void AAdvancedCombatAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update tension and behavior adaptation
    if (TensionManager)
    {
        TensionManager->UpdateTension(DeltaTime, PrimaryTarget, GetPawn());
        CurrentTensionLevel = TensionManager->GetCurrentTensionLevel();
    }
    
    // Analyze player behavior if we have a target
    if (BehaviorAnalyzer && PrimaryTarget)
    {
        BehaviorAnalyzer->AnalyzePlayerBehavior(PrimaryTarget, DeltaTime);
        UpdateCombatMemoryFromAnalysis();
    }
    
    // Update blackboard with current state
    UpdateBlackboardData();
    
    // Adapt personality based on encounter outcomes
    AdaptPersonalityBasedOnPerformance(DeltaTime);
}

void AAdvancedCombatAI::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (InPawn)
    {
        // Configure AI for the specific pawn type
        ConfigureForPawnType(InPawn);
        
        UE_LOG(LogTemp, Log, TEXT("AdvancedCombatAI: Possessed %s"), *InPawn->GetName());
    }
}

void AAdvancedCombatAI::SetupPerceptionSenses()
{
    if (!AIPerceptionComponent)
        return;
    
    // Configure sight sense
    UAISightConfig* SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2200.0f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->SetMaxAge(10.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    
    // Configure hearing sense
    UAIHearingConfig* HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(5.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AAdvancedCombatAI::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
        return;
    
    // Check if this is the player or another significant target
    bool bIsPlayer = Actor->IsA<APawn>() && Cast<APawn>(Actor)->IsPlayerControlled();
    
    if (Stimulus.WasSuccessfullySensed())
    {
        if (bIsPlayer)
        {
            SetPrimaryTarget(Actor);
            TransitionCombatPhase(ECombatPhase::Detection);
            
            // Record detection in combat memory
            CombatMemory.LastEncounterTime = GetWorld()->GetTimeSeconds();
            
            UE_LOG(LogTemp, Warning, TEXT("AdvancedCombatAI: %s detected player at %s"), 
                   *GetName(), *Stimulus.StimulusLocation.ToString());
        }
        else
        {
            // Add to secondary targets
            AddSecondaryTarget(Actor);
        }
        
        // Update blackboard immediately
        if (GetBlackboardComponent())
        {
            GetBlackboardComponent()->SetValueAsVector(TEXT("LastKnownTargetLocation"), Stimulus.StimulusLocation);
            GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), Actor);
        }
    }
    else
    {
        // Target lost
        if (Actor == PrimaryTarget)
        {
            OnTargetLost();
        }
        else
        {
            RemoveSecondaryTarget(Actor);
        }
    }
}

void AAdvancedCombatAI::OnTargetForgotten(AActor* Actor)
{
    if (Actor == PrimaryTarget)
    {
        OnTargetLost();
    }
    else
    {
        RemoveSecondaryTarget(Actor);
    }
}

void AAdvancedCombatAI::SetPrimaryTarget(AActor* NewTarget)
{
    AActor* OldTarget = PrimaryTarget;
    PrimaryTarget = NewTarget;
    
    if (PrimaryTarget != OldTarget)
    {
        // Notify tactical analyzer of target change
        if (TacticalAnalyzer)
        {
            TacticalAnalyzer->OnTargetChanged(OldTarget, PrimaryTarget);
        }
        
        // Reset behavior analysis for new target
        if (BehaviorAnalyzer)
        {
            BehaviorAnalyzer->ResetAnalysis();
        }
        
        // Update blackboard
        if (GetBlackboardComponent())
        {
            GetBlackboardComponent()->SetValueAsObject(TEXT("PrimaryTarget"), PrimaryTarget);
        }
        
        UE_LOG(LogTemp, Log, TEXT("AdvancedCombatAI: %s acquired new primary target: %s"), 
               *GetName(), PrimaryTarget ? *PrimaryTarget->GetName() : TEXT("None"));
    }
}

void AAdvancedCombatAI::AddSecondaryTarget(AActor* Target)
{
    if (Target && !SecondaryTargets.Contains(Target))
    {
        SecondaryTargets.Add(Target);
        
        // Update blackboard array
        UpdateSecondaryTargetsInBlackboard();
    }
}

void AAdvancedCombatAI::RemoveSecondaryTarget(AActor* Target)
{
    if (SecondaryTargets.Remove(Target) > 0)
    {
        UpdateSecondaryTargetsInBlackboard();
    }
}

void AAdvancedCombatAI::OnTargetLost()
{
    if (PrimaryTarget)
    {
        // Store last known information
        FVector LastKnownLocation = PrimaryTarget->GetActorLocation();
        
        if (GetBlackboardComponent())
        {
            GetBlackboardComponent()->SetValueAsVector(TEXT("LastKnownTargetLocation"), LastKnownLocation);
            GetBlackboardComponent()->SetValueAsObject(TEXT("PrimaryTarget"), nullptr);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("AdvancedCombatAI: %s lost primary target, last known location: %s"), 
               *GetName(), *LastKnownLocation.ToString());
        
        PrimaryTarget = nullptr;
        TransitionCombatPhase(ECombatPhase::Assessment);
    }
}

void AAdvancedCombatAI::TransitionCombatPhase(ECombatPhase NewPhase)
{
    if (CurrentCombatPhase != NewPhase)
    {
        ECombatPhase OldPhase = CurrentCombatPhase;
        CurrentCombatPhase = NewPhase;
        
        // Update blackboard
        if (GetBlackboardComponent())
        {
            GetBlackboardComponent()->SetValueAsEnum(TEXT("CombatPhase"), static_cast<uint8>(CurrentCombatPhase));
        }
        
        // Notify tactical analyzer
        if (TacticalAnalyzer)
        {
            TacticalAnalyzer->OnCombatPhaseChanged(OldPhase, NewPhase);
        }
        
        // Execute phase-specific logic
        OnCombatPhaseChanged(OldPhase, NewPhase);
        
        UE_LOG(LogTemp, Log, TEXT("AdvancedCombatAI: %s transitioned from %s to %s"), 
               *GetName(), 
               *UEnum::GetValueAsString(OldPhase),
               *UEnum::GetValueAsString(NewPhase));
    }
}

void AAdvancedCombatAI::OnCombatPhaseChanged(ECombatPhase OldPhase, ECombatPhase NewPhase)
{
    switch (NewPhase)
    {
    case ECombatPhase::Detection:
        // Begin threat assessment
        if (TacticalAnalyzer)
        {
            TacticalAnalyzer->BeginThreatAssessment(PrimaryTarget);
        }
        break;
        
    case ECombatPhase::Assessment:
        // Analyze tactical situation
        PerformTacticalAnalysis();
        break;
        
    case ECombatPhase::Positioning:
        // Find optimal position
        if (TacticalAnalyzer && PrimaryTarget)
        {
            FVector OptimalPosition = TacticalAnalyzer->FindOptimalPosition(PrimaryTarget, GetPawn());
            if (GetBlackboardComponent())
            {
                GetBlackboardComponent()->SetValueAsVector(TEXT("OptimalPosition"), OptimalPosition);
            }
        }
        break;
        
    case ECombatPhase::Engagement:
        // Begin active combat
        if (TensionManager)
        {
            TensionManager->OnCombatEngaged();
        }
        break;
        
    case ECombatPhase::Retreat:
        // Execute retreat strategy
        ExecuteRetreatStrategy();
        break;
        
    default:
        break;
    }
}

void AAdvancedCombatAI::PerformTacticalAnalysis()
{
    if (!TacticalAnalyzer || !GetPawn())
        return;
    
    // Analyze current tactical situation
    FTacticalSituation Situation = TacticalAnalyzer->AnalyzeSituation(PrimaryTarget, SecondaryTargets, GetPawn());
    
    // Update blackboard with analysis results
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsFloat(TEXT("ThreatLevel"), Situation.ThreatLevel);
        GetBlackboardComponent()->SetValueAsFloat(TEXT("AdvantageScore"), Situation.AdvantageScore);
        GetBlackboardComponent()->SetValueAsBool(TEXT("HasCoverNearby"), Situation.bHasCoverNearby);
        GetBlackboardComponent()->SetValueAsBool(TEXT("HasEscapeRoute"), Situation.bHasEscapeRoute);
    }
    
    // Determine next phase based on analysis
    DetermineNextCombatPhase(Situation);
}

void AAdvancedCombatAI::DetermineNextCombatPhase(const FTacticalSituation& Situation)
{
    // Decision logic based on personality and situation
    float RetreatThreshold = CombatPersonality.RetreatThreshold;
    
    if (Situation.ThreatLevel > RetreatThreshold && Situation.AdvantageScore < 0.3f)
    {
        TransitionCombatPhase(ECombatPhase::Retreat);
    }
    else if (CurrentCombatPhase == ECombatPhase::Assessment)
    {
        if (CombatPersonality.PersonalityType == ECombatPersonalityType::Stalker)
        {
            TransitionCombatPhase(ECombatPhase::Stalking);
        }
        else if (CombatPersonality.AmbushPreference > 0.6f && Situation.bHasCoverNearby)
        {
            TransitionCombatPhase(ECombatPhase::Positioning);
        }
        else
        {
            TransitionCombatPhase(ECombatPhase::Engagement);
        }
    }
}

void AAdvancedCombatAI::ExecuteRetreatStrategy()
{
    if (!TacticalAnalyzer || !GetPawn())
        return;
    
    // Find best retreat position
    FVector RetreatPosition = TacticalAnalyzer->FindRetreatPosition(GetPawn()->GetActorLocation(), PrimaryTarget);
    
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsVector(TEXT("RetreatPosition"), RetreatPosition);
        GetBlackboardComponent()->SetValueAsBool(TEXT("ShouldRetreat"), true);
    }
    
    // Record failed encounter
    CombatMemory.LastEncounterWon = false;
    if (!CombatMemory.LastUsedTactic.IsEmpty())
    {
        CombatMemory.FailedTactics.AddUnique(CombatMemory.LastUsedTactic);
    }
}

void AAdvancedCombatAI::UpdateBlackboardData()
{
    if (!GetBlackboardComponent())
        return;
    
    // Update personality data
    GetBlackboardComponent()->SetValueAsFloat(TEXT("Aggression"), CombatPersonality.Aggression);
    GetBlackboardComponent()->SetValueAsFloat(TEXT("Patience"), CombatPersonality.Patience);
    GetBlackboardComponent()->SetValueAsFloat(TEXT("Intelligence"), CombatPersonality.Intelligence);
    GetBlackboardComponent()->SetValueAsFloat(TEXT("Unpredictability"), CombatPersonality.Unpredictability);
    
    // Update combat state
    GetBlackboardComponent()->SetValueAsEnum(TEXT("CombatPhase"), static_cast<uint8>(CurrentCombatPhase));
    GetBlackboardComponent()->SetValueAsEnum(TEXT("TensionLevel"), static_cast<uint8>(CurrentTensionLevel));
    
    // Update target information
    if (PrimaryTarget)
    {
        GetBlackboardComponent()->SetValueAsObject(TEXT("PrimaryTarget"), PrimaryTarget);
        GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), PrimaryTarget->GetActorLocation());
        
        // Calculate distance to target
        float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), PrimaryTarget->GetActorLocation());
        GetBlackboardComponent()->SetValueAsFloat(TEXT("DistanceToTarget"), DistanceToTarget);
    }
}

void AAdvancedCombatAI::UpdateSecondaryTargetsInBlackboard()
{
    // This would require a custom blackboard key type for actor arrays
    // For now, we'll use the count
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsInt(TEXT("SecondaryTargetCount"), SecondaryTargets.Num());
    }
}

void AAdvancedCombatAI::UpdateCombatMemoryFromAnalysis()
{
    if (!BehaviorAnalyzer)
        return;
    
    // Get player behavior patterns from analyzer
    TMap<FString, float> CurrentPatterns = BehaviorAnalyzer->GetBehaviorPatterns();
    
    // Update combat memory
    for (const auto& Pattern : CurrentPatterns)
    {
        CombatMemory.PlayerBehaviorPatterns.Add(Pattern.Key, Pattern.Value);
    }
    
    // Store preferred locations
    FVector PlayerLocation = PrimaryTarget ? PrimaryTarget->GetActorLocation() : FVector::ZeroVector;
    if (PlayerLocation != FVector::ZeroVector)
    {
        CombatMemory.PlayerPreferredLocations.AddUnique(PlayerLocation);
        
        // Keep only recent locations (max 10)
        if (CombatMemory.PlayerPreferredLocations.Num() > 10)
        {
            CombatMemory.PlayerPreferredLocations.RemoveAt(0);
        }
    }
}

void AAdvancedCombatAI::AdaptPersonalityBasedOnPerformance(float DeltaTime)
{
    // Gradually adapt personality based on success/failure patterns
    float AdaptationRate = CombatPersonality.LearningRate * DeltaTime;
    
    // If we're consistently failing, become more cautious
    if (CombatMemory.FailedTactics.Num() > CombatMemory.SuccessfulTactics.Num())
    {
        CombatPersonality.Patience = FMath::Clamp(CombatPersonality.Patience + AdaptationRate, 0.0f, 1.0f);
        CombatPersonality.RetreatThreshold = FMath::Clamp(CombatPersonality.RetreatThreshold + AdaptationRate * 0.5f, 0.0f, 1.0f);
    }
    
    // If we're consistently succeeding, become more aggressive
    else if (CombatMemory.SuccessfulTactics.Num() > CombatMemory.FailedTactics.Num() + 2)
    {
        CombatPersonality.Aggression = FMath::Clamp(CombatPersonality.Aggression + AdaptationRate * 0.5f, 0.0f, 1.0f);
        CombatPersonality.RetreatThreshold = FMath::Clamp(CombatPersonality.RetreatThreshold - AdaptationRate * 0.3f, 0.0f, 1.0f);
    }
}

void AAdvancedCombatAI::ConfigureForPawnType(APawn* InPawn)
{
    // Adjust AI parameters based on the type of creature we're controlling
    if (!InPawn)
        return;
    
    // This would be expanded based on specific dinosaur types
    FString PawnName = InPawn->GetClass()->GetName();
    
    if (PawnName.Contains(TEXT("Raptor")))
    {
        CombatPersonality.PersonalityType = ECombatPersonalityType::PackHunter;
        CombatPersonality.Aggression = FMath::RandRange(0.7f, 0.9f);
        CombatPersonality.Intelligence = FMath::RandRange(0.8f, 1.0f);
        CombatPersonality.SocialCoordination = FMath::RandRange(0.8f, 1.0f);
    }
    else if (PawnName.Contains(TEXT("Rex")))
    {
        CombatPersonality.PersonalityType = ECombatPersonalityType::Territorial;
        CombatPersonality.Aggression = FMath::RandRange(0.8f, 1.0f);
        CombatPersonality.Patience = FMath::RandRange(0.3f, 0.6f);
        CombatPersonality.PreferredEngagementDistance = FMath::RandRange(0.0f, 0.4f);
    }
    else if (PawnName.Contains(TEXT("Compy")))
    {
        CombatPersonality.PersonalityType = ECombatPersonalityType::Scavenger;
        CombatPersonality.Aggression = FMath::RandRange(0.3f, 0.6f);
        CombatPersonality.Patience = FMath::RandRange(0.6f, 0.9f);
        CombatPersonality.RetreatThreshold = FMath::RandRange(0.6f, 0.8f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("AdvancedCombatAI: Configured for %s with personality %s"), 
           *PawnName, *UEnum::GetValueAsString(CombatPersonality.PersonalityType));
}