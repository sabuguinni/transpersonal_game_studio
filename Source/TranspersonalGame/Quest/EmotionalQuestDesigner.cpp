#include "EmotionalQuestDesigner.h"
#include "QuestSystem.h"
#include "DynamicQuestGenerator.h"
#include "../Narrative/GameBible.h"
#include "../CrowdSimulation/MassDinosaurFragments.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void UEmotionalQuestDesigner::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize quest design patterns
    InitializeDefaultPatterns();
    
    // Initialize player emotional profile with defaults
    CurrentPlayerProfile = FPlayerEmotionalProfile();
    CurrentPlayerProfile.EmotionalPreferences.Add(EEmotionalState::Wonder, 0.7f);
    CurrentPlayerProfile.EmotionalPreferences.Add(EEmotionalState::Fear, 0.5f);
    CurrentPlayerProfile.EmotionalPreferences.Add(EEmotionalState::Curiosity, 0.8f);
    CurrentPlayerProfile.EmotionalPreferences.Add(EEmotionalState::Discovery, 0.9f);
    
    CurrentPlayerProfile.QuestTypePreferences.Add(EQuestType::Observation, 0.8f);
    CurrentPlayerProfile.QuestTypePreferences.Add(EQuestType::Discovery, 0.9f);
    CurrentPlayerProfile.QuestTypePreferences.Add(EQuestType::Survival, 0.6f);
    CurrentPlayerProfile.QuestTypePreferences.Add(EQuestType::Domestication, 0.7f);
    
    UE_LOG(LogTemp, Log, TEXT("EmotionalQuestDesigner: Initialized with default patterns and player profile"));
}

void UEmotionalQuestDesigner::Deinitialize()
{
    DesignPatterns.Empty();
    Super::Deinitialize();
}

void UEmotionalQuestDesigner::InitializeDefaultPatterns()
{
    // Pattern 1: First Contact - Wonder and Fear
    FQuestDesignPattern FirstContactPattern;
    FirstContactPattern.PatternID = FName("FirstContact");
    FirstContactPattern.PatternName = FText::FromString("First Contact");
    FirstContactPattern.PatternDescription = FText::FromString("Player encounters a new dinosaur species for the first time");
    
    FirstContactPattern.EmotionalArc.OpeningEmotion = EEmotionalState::Curiosity;
    FirstContactPattern.EmotionalArc.MiddleEmotion = EEmotionalState::Fear;
    FirstContactPattern.EmotionalArc.ResolutionEmotion = EEmotionalState::Wonder;
    FirstContactPattern.EmotionalArc.IntensityCurve = 1.2f;
    FirstContactPattern.EmotionalArc.bHasEmotionalTwist = true;
    FirstContactPattern.EmotionalArc.EmotionalPayoff = FText::FromString("The awe of witnessing a living dinosaur");
    
    FirstContactPattern.ObjectiveSequence.Add(EObjectiveType::Reach);
    FirstContactPattern.ObjectiveSequence.Add(EObjectiveType::Observe);
    FirstContactPattern.ObjectiveSequence.Add(EObjectiveType::Survive);
    
    FirstContactPattern.PrimaryTheme = ENarrativeTheme::WonderOfNature;
    FirstContactPattern.OptimalDuration = 480.0f; // 8 minutes
    FirstContactPattern.EmotionalWeight = 1.5f;
    
    DesignPatterns.Add(FirstContactPattern.PatternID, FirstContactPattern);
    
    // Pattern 2: Silent Observer - Patience and Understanding
    FQuestDesignPattern ObserverPattern;
    ObserverPattern.PatternID = FName("SilentObserver");
    ObserverPattern.PatternName = FText::FromString("Silent Observer");
    ObserverPattern.PatternDescription = FText::FromString("Study dinosaur behavior without being detected");
    
    ObserverPattern.EmotionalArc.OpeningEmotion = EEmotionalState::Curiosity;
    ObserverPattern.EmotionalArc.MiddleEmotion = EEmotionalState::Tension;
    ObserverPattern.EmotionalArc.ResolutionEmotion = EEmotionalState::Connection;
    ObserverPattern.EmotionalArc.IntensityCurve = 0.8f;
    ObserverPattern.EmotionalArc.EmotionalPayoff = FText::FromString("Understanding these magnificent creatures");
    
    ObserverPattern.ObjectiveSequence.Add(EObjectiveType::Observe);
    ObserverPattern.ObjectiveSequence.Add(EObjectiveType::StayHidden);
    
    ObserverPattern.PrimaryTheme = ENarrativeTheme::ScientificDiscovery;
    ObserverPattern.OptimalDuration = 600.0f; // 10 minutes
    ObserverPattern.EmotionalWeight = 1.0f;
    
    DesignPatterns.Add(ObserverPattern.PatternID, ObserverPattern);
    
    // Pattern 3: Gentle Bond - Trust and Connection
    FQuestDesignPattern GentleBondPattern;
    GentleBondPattern.PatternID = FName("GentleBond");
    GentleBondPattern.PatternName = FText::FromString("Gentle Bond");
    GentleBondPattern.PatternDescription = FText::FromString("Slowly build trust with a herbivorous dinosaur");
    
    GentleBondPattern.EmotionalArc.OpeningEmotion = EEmotionalState::Curiosity;
    GentleBondPattern.EmotionalArc.MiddleEmotion = EEmotionalState::Empathy;
    GentleBondPattern.EmotionalArc.ResolutionEmotion = EEmotionalState::Connection;
    GentleBondPattern.EmotionalArc.IntensityCurve = 0.9f;
    GentleBondPattern.EmotionalArc.EmotionalPayoff = FText::FromString("A bond that transcends time");
    
    GentleBondPattern.ObjectiveSequence.Add(EObjectiveType::Approach);
    GentleBondPattern.ObjectiveSequence.Add(EObjectiveType::Interact);
    GentleBondPattern.ObjectiveSequence.Add(EObjectiveType::Domesticate);
    
    GentleBondPattern.PrimaryTheme = ENarrativeTheme::ConnectionWithNature;
    GentleBondPattern.OptimalDuration = 900.0f; // 15 minutes
    GentleBondPattern.EmotionalWeight = 1.3f;
    
    DesignPatterns.Add(GentleBondPattern.PatternID, GentleBondPattern);
    
    UE_LOG(LogTemp, Log, TEXT("EmotionalQuestDesigner: Loaded %d default patterns"), DesignPatterns.Num());
}

FName UEmotionalQuestDesigner::DesignQuestFromNarrativeBeat(const FStoryBeat& NarrativeBeat, const FNarrativeContext& Context)
{
    // Select appropriate pattern based on narrative beat
    FQuestDesignPattern SelectedPattern = SelectOptimalPattern(Context, NarrativeBeat.PrimaryTheme);
    
    // Create context variables from narrative beat
    TMap<FString, FString> ContextVariables;
    ContextVariables.Add("BeatTitle", NarrativeBeat.Title.ToString());
    ContextVariables.Add("BeatDescription", NarrativeBeat.Description.ToString());
    ContextVariables.Add("PlayerLocation", Context.PlayerLocation.ToString());
    ContextVariables.Add("CurrentAct", UEnum::GetValueAsString(Context.CurrentAct));
    
    // Generate quest using selected pattern
    FName QuestID = InstantiatePattern(SelectedPattern, ContextVariables);
    
    UE_LOG(LogTemp, Log, TEXT("EmotionalQuestDesigner: Created quest %s from narrative beat %s"), 
           *QuestID.ToString(), *NarrativeBeat.Title.ToString());
    
    return QuestID;
}

FName UEmotionalQuestDesigner::DesignEmergentQuest(const FEcosystemEvent& Event, const FNarrativeContext& Context)
{
    // Determine emotional response to ecosystem event
    EEmotionalState TargetEmotion = EEmotionalState::Wonder;
    ENarrativeTheme Theme = ENarrativeTheme::WonderOfNature;
    
    switch (Event.EventType)
    {
        case EEcosystemEventType::PredatorHunt:
            TargetEmotion = EEmotionalState::Fear;
            Theme = ENarrativeTheme::SurvivalInstinct;
            break;
        case EEcosystemEventType::HerbivoreGrazing:
            TargetEmotion = EEmotionalState::Connection;
            Theme = ENarrativeTheme::ConnectionWithNature;
            break;
        case EEcosystemEventType::Migration:
            TargetEmotion = EEmotionalState::Wonder;
            Theme = ENarrativeTheme::WonderOfNature;
            break;
        case EEcosystemEventType::TerritorialDispute:
            TargetEmotion = EEmotionalState::Tension;
            Theme = ENarrativeTheme::SurvivalInstinct;
            break;
    }
    
    // Find pattern that matches desired emotion
    FQuestDesignPattern* BestPattern = nullptr;
    float BestScore = 0.0f;
    
    for (auto& PatternPair : DesignPatterns)
    {
        FQuestDesignPattern& Pattern = PatternPair.Value;
        float Score = CalculatePatternEmotionalMatch(Pattern, TargetEmotion, Theme);
        
        if (Score > BestScore)
        {
            BestScore = Score;
            BestPattern = &Pattern;
        }
    }
    
    if (!BestPattern)
    {
        UE_LOG(LogTemp, Warning, TEXT("EmotionalQuestDesigner: No suitable pattern found for emergent quest"));
        return NAME_None;
    }
    
    // Create context variables from ecosystem event
    TMap<FString, FString> ContextVariables;
    ContextVariables.Add("EventType", UEnum::GetValueAsString(Event.EventType));
    ContextVariables.Add("EventLocation", Event.Location.ToString());
    ContextVariables.Add("EventIntensity", FString::SanitizeFloat(Event.Intensity));
    ContextVariables.Add("TargetEmotion", UEnum::GetValueAsString(TargetEmotion));
    
    FName QuestID = InstantiatePattern(*BestPattern, ContextVariables);
    
    UE_LOG(LogTemp, Log, TEXT("EmotionalQuestDesigner: Created emergent quest %s from ecosystem event"), 
           *QuestID.ToString());
    
    return QuestID;
}

FName UEmotionalQuestDesigner::DesignPersonalizedQuest(const FPlayerEmotionalProfile& PlayerProfile, const FNarrativeContext& Context)
{
    // Find the emotion the player needs most based on their recent experiences
    EEmotionalState NeededEmotion = PredictPlayerEmotionalNeed(Context);
    
    // Find the quest type they prefer most
    EQuestType PreferredType = EQuestType::Discovery;
    float HighestPreference = 0.0f;
    
    for (auto& Preference : PlayerProfile.QuestTypePreferences)
    {
        if (Preference.Value > HighestPreference)
        {
            HighestPreference = Preference.Value;
            PreferredType = Preference.Key;
        }
    }
    
    // Select pattern that combines needed emotion with preferred type
    FQuestDesignPattern* BestPattern = nullptr;
    float BestScore = 0.0f;
    
    for (auto& PatternPair : DesignPatterns)
    {
        FQuestDesignPattern& Pattern = PatternPair.Value;
        float EmotionalScore = CalculateEmotionalResonance(Pattern, NeededEmotion);
        float TypeScore = CalculateQuestTypeMatch(Pattern, PreferredType);
        float TotalScore = (EmotionalScore * 0.6f) + (TypeScore * 0.4f);
        
        if (TotalScore > BestScore)
        {
            BestScore = TotalScore;
            BestPattern = &Pattern;
        }
    }
    
    if (!BestPattern)
    {
        UE_LOG(LogTemp, Warning, TEXT("EmotionalQuestDesigner: No suitable pattern found for personalized quest"));
        return NAME_None;
    }
    
    // Create personalized context variables
    TMap<FString, FString> ContextVariables;
    ContextVariables.Add("NeededEmotion", UEnum::GetValueAsString(NeededEmotion));
    ContextVariables.Add("PreferredType", UEnum::GetValueAsString(PreferredType));
    ContextVariables.Add("AttentionSpan", FString::SanitizeFloat(PlayerProfile.AttentionSpan));
    ContextVariables.Add("ChallengePreference", FString::SanitizeFloat(PlayerProfile.ChallengePreference));
    
    FName QuestID = InstantiatePattern(*BestPattern, ContextVariables);
    
    UE_LOG(LogTemp, Log, TEXT("EmotionalQuestDesigner: Created personalized quest %s targeting emotion %s"), 
           *QuestID.ToString(), *UEnum::GetValueAsString(NeededEmotion));
    
    return QuestID;
}

FQuestEmotionalArc UEmotionalQuestDesigner::DesignEmotionalArc(ENarrativeTheme Theme, EEmotionalState DesiredOutcome)
{
    FQuestEmotionalArc Arc;
    
    // Design emotional progression based on theme
    switch (Theme)
    {
        case ENarrativeTheme::WonderOfNature:
            Arc.OpeningEmotion = EEmotionalState::Curiosity;
            Arc.MiddleEmotion = EEmotionalState::Wonder;
            Arc.ResolutionEmotion = DesiredOutcome;
            Arc.IntensityCurve = 1.1f;
            Arc.EmotionalPayoff = FText::FromString("The beauty and majesty of prehistoric life");
            break;
            
        case ENarrativeTheme::SurvivalInstinct:
            Arc.OpeningEmotion = EEmotionalState::Tension;
            Arc.MiddleEmotion = EEmotionalState::Fear;
            Arc.ResolutionEmotion = DesiredOutcome;
            Arc.IntensityCurve = 1.3f;
            Arc.bHasEmotionalTwist = true;
            Arc.EmotionalPayoff = FText::FromString("The triumph of overcoming primal fear");
            break;
            
        case ENarrativeTheme::ScientificDiscovery:
            Arc.OpeningEmotion = EEmotionalState::Curiosity;
            Arc.MiddleEmotion = EEmotionalState::Discovery;
            Arc.ResolutionEmotion = DesiredOutcome;
            Arc.IntensityCurve = 0.9f;
            Arc.EmotionalPayoff = FText::FromString("The satisfaction of understanding");
            break;
            
        case ENarrativeTheme::ConnectionWithNature:
            Arc.OpeningEmotion = EEmotionalState::Loneliness;
            Arc.MiddleEmotion = EEmotionalState::Empathy;
            Arc.ResolutionEmotion = DesiredOutcome;
            Arc.IntensityCurve = 0.8f;
            Arc.EmotionalPayoff = FText::FromString("A bond that transcends species");
            break;
            
        default:
            Arc.OpeningEmotion = EEmotionalState::Curiosity;
            Arc.MiddleEmotion = EEmotionalState::Wonder;
            Arc.ResolutionEmotion = DesiredOutcome;
            Arc.IntensityCurve = 1.0f;
            Arc.EmotionalPayoff = FText::FromString("A meaningful experience");
            break;
    }
    
    return Arc;
}

FQuestDesignPattern UEmotionalQuestDesigner::SelectOptimalPattern(const FNarrativeContext& Context, ENarrativeTheme DesiredTheme)
{
    FQuestDesignPattern* BestPattern = nullptr;
    float BestScore = 0.0f;
    
    for (auto& PatternPair : DesignPatterns)
    {
        FQuestDesignPattern& Pattern = PatternPair.Value;
        
        // Calculate pattern suitability score
        float ThemeScore = (Pattern.PrimaryTheme == DesiredTheme) ? 1.0f : 0.3f;
        float ContextScore = CalculateContextualFit(Pattern, Context);
        float PlayerScore = CalculatePlayerFit(Pattern, CurrentPlayerProfile);
        
        float TotalScore = (ThemeScore * 0.4f) + (ContextScore * 0.3f) + (PlayerScore * 0.3f);
        
        if (TotalScore > BestScore)
        {
            BestScore = TotalScore;
            BestPattern = &Pattern;
        }
    }
    
    if (BestPattern)
    {
        return *BestPattern;
    }
    
    // Fallback to first available pattern
    if (DesignPatterns.Num() > 0)
    {
        return DesignPatterns.begin().Value();
    }
    
    // Return empty pattern as last resort
    return FQuestDesignPattern();
}

FName UEmotionalQuestDesigner::InstantiatePattern(const FQuestDesignPattern& Pattern, const TMap<FString, FString>& ContextVariables)
{
    UQuestSystem* QuestSystem = GetGameInstance()->GetSubsystem<UQuestSystem>();
    if (!QuestSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("EmotionalQuestDesigner: QuestSystem not found"));
        return NAME_None;
    }
    
    // Create quest data from pattern
    FQuestData QuestData;
    QuestData.QuestID = FName(*FString::Printf(TEXT("%s_%d"), *Pattern.PatternID.ToString(), FMath::RandRange(1000, 9999)));
    QuestData.Title = FText::Format(FText::FromString("{0} - {1}"), Pattern.PatternName, FText::FromString(ContextVariables.FindRef("BeatTitle")));
    QuestData.Description = Pattern.PatternDescription;
    QuestData.Type = DetermineQuestTypeFromPattern(Pattern);
    QuestData.Priority = EQuestPriority::Normal;
    QuestData.bIsActive = false; // Will be activated by quest system
    
    // Create objectives from pattern
    for (int32 i = 0; i < Pattern.ObjectiveSequence.Num(); i++)
    {
        FQuestObjective Objective;
        Objective.ObjectiveID = i;
        Objective.Type = Pattern.ObjectiveSequence[i];
        Objective.Description = GenerateObjectiveDescription(Pattern.ObjectiveSequence[i], ContextVariables);
        Objective.bIsCompleted = false;
        Objective.bIsOptional = false;
        Objective.Progress = 0.0f;
        
        QuestData.Objectives.Add(Objective);
    }
    
    // Set emotional context
    QuestData.EmotionalContext.Add("EmotionalArc", Pattern.EmotionalArc.EmotionalPayoff.ToString());
    QuestData.EmotionalContext.Add("TargetDuration", FString::SanitizeFloat(Pattern.OptimalDuration));
    QuestData.EmotionalContext.Add("EmotionalWeight", FString::SanitizeFloat(Pattern.EmotionalWeight));
    
    // Register quest with system
    bool bSuccess = QuestSystem->CreateQuest(QuestData);
    
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("EmotionalQuestDesigner: Successfully instantiated quest %s from pattern %s"), 
               *QuestData.QuestID.ToString(), *Pattern.PatternID.ToString());
        return QuestData.QuestID;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("EmotionalQuestDesigner: Failed to create quest from pattern %s"), 
               *Pattern.PatternID.ToString());
        return NAME_None;
    }
}

void UEmotionalQuestDesigner::UpdatePlayerEmotionalProfile(FName CompletedQuestID, float EmotionalResponse, float EngagementTime)
{
    // Update player preferences based on completed quest
    UQuestSystem* QuestSystem = GetGameInstance()->GetSubsystem<UQuestSystem>();
    if (!QuestSystem)
    {
        return;
    }
    
    FQuestData* QuestData = QuestSystem->GetQuestData(CompletedQuestID);
    if (!QuestData)
    {
        return;
    }
    
    // Update quest type preferences
    if (CurrentPlayerProfile.QuestTypePreferences.Contains(QuestData->Type))
    {
        float CurrentPreference = CurrentPlayerProfile.QuestTypePreferences[QuestData->Type];
        float NewPreference = FMath::Lerp(CurrentPreference, EmotionalResponse, 0.1f);
        CurrentPlayerProfile.QuestTypePreferences[QuestData->Type] = FMath::Clamp(NewPreference, 0.0f, 1.0f);
    }
    
    // Update attention span based on engagement time
    float ExpectedDuration = FCString::Atof(*QuestData->EmotionalContext.FindRef("TargetDuration"));
    if (ExpectedDuration > 0.0f)
    {
        float EngagementRatio = EngagementTime / ExpectedDuration;
        CurrentPlayerProfile.AttentionSpan = FMath::Lerp(CurrentPlayerProfile.AttentionSpan, EngagementTime, 0.05f);
    }
    
    // Add to memorable quests if high emotional response
    if (EmotionalResponse > 0.8f)
    {
        CurrentPlayerProfile.MemorableQuests.AddUnique(CompletedQuestID);
        
        // Keep only the most recent memorable quests
        if (CurrentPlayerProfile.MemorableQuests.Num() > 10)
        {
            CurrentPlayerProfile.MemorableQuests.RemoveAt(0);
        }
    }
    
    CurrentPlayerProfile.LastEmotionalPeak = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("EmotionalQuestDesigner: Updated player profile after quest %s completion"), 
           *CompletedQuestID.ToString());
}

EEmotionalState UEmotionalQuestDesigner::PredictPlayerEmotionalNeed(const FNarrativeContext& Context)
{
    float TimeSinceLastPeak = GetWorld()->GetTimeSeconds() - CurrentPlayerProfile.LastEmotionalPeak;
    
    // If it's been a while since last emotional peak, player might need stimulation
    if (TimeSinceLastPeak > 600.0f) // 10 minutes
    {
        return EEmotionalState::Wonder;
    }
    
    // Check recent quest completions for emotional balance
    if (Context.RecentlyCompletedQuests.Num() > 0)
    {
        // If recent quests were high-tension, provide calming experience
        return EEmotionalState::Connection;
    }
    
    // Default to curiosity for exploration
    return EEmotionalState::Curiosity;
}

float UEmotionalQuestDesigner::CalculateEmotionalResonance(const FQuestData& Quest, const FNarrativeContext& Context)
{
    // This is a simplified version - would need quest emotional data
    float BaseResonance = 0.5f;
    
    // Adjust based on quest type preferences
    if (CurrentPlayerProfile.QuestTypePreferences.Contains(Quest.Type))
    {
        BaseResonance *= CurrentPlayerProfile.QuestTypePreferences[Quest.Type];
    }
    
    // Adjust based on current emotional state
    BaseResonance *= (1.0f + Context.PlayerEmotionalState);
    
    return FMath::Clamp(BaseResonance, 0.0f, 1.0f);
}

// Helper functions
float UEmotionalQuestDesigner::CalculatePatternEmotionalMatch(const FQuestDesignPattern& Pattern, EEmotionalState TargetEmotion, ENarrativeTheme Theme)
{
    float Score = 0.0f;
    
    // Check if pattern's resolution emotion matches target
    if (Pattern.EmotionalArc.ResolutionEmotion == TargetEmotion)
    {
        Score += 0.5f;
    }
    
    // Check theme match
    if (Pattern.PrimaryTheme == Theme)
    {
        Score += 0.3f;
    }
    
    // Check emotional progression
    if (Pattern.EmotionalArc.MiddleEmotion == TargetEmotion || Pattern.EmotionalArc.OpeningEmotion == TargetEmotion)
    {
        Score += 0.2f;
    }
    
    return Score;
}

float UEmotionalQuestDesigner::CalculateEmotionalResonance(const FQuestDesignPattern& Pattern, EEmotionalState TargetEmotion)
{
    float Score = 0.0f;
    
    if (Pattern.EmotionalArc.ResolutionEmotion == TargetEmotion)
    {
        Score += 0.6f;
    }
    if (Pattern.EmotionalArc.MiddleEmotion == TargetEmotion)
    {
        Score += 0.3f;
    }
    if (Pattern.EmotionalArc.OpeningEmotion == TargetEmotion)
    {
        Score += 0.1f;
    }
    
    return Score;
}

float UEmotionalQuestDesigner::CalculateQuestTypeMatch(const FQuestDesignPattern& Pattern, EQuestType PreferredType)
{
    EQuestType PatternType = DetermineQuestTypeFromPattern(Pattern);
    return (PatternType == PreferredType) ? 1.0f : 0.3f;
}

float UEmotionalQuestDesigner::CalculateContextualFit(const FQuestDesignPattern& Pattern, const FNarrativeContext& Context)
{
    float Score = 0.5f; // Base score
    
    // Check if pattern requirements are met
    for (auto& RequiredTag : Pattern.RequiredWorldState.GetGameplayTags())
    {
        if (Context.PlayerProgressTags.HasTag(RequiredTag))
        {
            Score += 0.1f;
        }
        else
        {
            Score -= 0.2f; // Penalize if requirements not met
        }
    }
    
    return FMath::Clamp(Score, 0.0f, 1.0f);
}

float UEmotionalQuestDesigner::CalculatePlayerFit(const FQuestDesignPattern& Pattern, const FPlayerEmotionalProfile& PlayerProfile)
{
    float Score = 0.5f; // Base score
    
    // Check emotional preferences
    if (PlayerProfile.EmotionalPreferences.Contains(Pattern.EmotionalArc.ResolutionEmotion))
    {
        Score += PlayerProfile.EmotionalPreferences[Pattern.EmotionalArc.ResolutionEmotion] * 0.3f;
    }
    
    // Check duration preferences
    float DurationRatio = Pattern.OptimalDuration / PlayerProfile.AttentionSpan;
    if (DurationRatio > 0.5f && DurationRatio < 1.5f)
    {
        Score += 0.2f; // Good duration match
    }
    
    return FMath::Clamp(Score, 0.0f, 1.0f);
}

EQuestType UEmotionalQuestDesigner::DetermineQuestTypeFromPattern(const FQuestDesignPattern& Pattern)
{
    // Analyze objective sequence to determine quest type
    if (Pattern.ObjectiveSequence.Contains(EObjectiveType::Observe))
    {
        return EQuestType::Observation;
    }
    if (Pattern.ObjectiveSequence.Contains(EObjectiveType::Domesticate))
    {
        return EQuestType::Domestication;
    }
    if (Pattern.ObjectiveSequence.Contains(EObjectiveType::Survive))
    {
        return EQuestType::Survival;
    }
    if (Pattern.ObjectiveSequence.Contains(EObjectiveType::Reach))
    {
        return EQuestType::Discovery;
    }
    
    return EQuestType::Discovery; // Default
}

FText UEmotionalQuestDesigner::GenerateObjectiveDescription(EObjectiveType Type, const TMap<FString, FString>& ContextVariables)
{
    switch (Type)
    {
        case EObjectiveType::Observe:
            return FText::FromString("Observe the dinosaur behavior without being detected");
        case EObjectiveType::Reach:
            return FText::FromString("Reach the designated location");
        case EObjectiveType::Survive:
            return FText::FromString("Survive the encounter");
        case EObjectiveType::Domesticate:
            return FText::FromString("Build trust with the creature");
        case EObjectiveType::Interact:
            return FText::FromString("Interact with the target");
        case EObjectiveType::StayHidden:
            return FText::FromString("Remain undetected");
        case EObjectiveType::Approach:
            return FText::FromString("Carefully approach the target");
        default:
            return FText::FromString("Complete the objective");
    }
}