#include "EmotionalQuestDesigner.h"
#include "QuestSystem.h"
#include "DynamicQuestGenerator.h"
#include "../Narrative/GameBible.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void UEmotionalQuestDesigner::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("EmotionalQuestDesigner: Initializing emotional quest design system"));
    
    // Initialize default player emotional profile
    CurrentPlayerProfile = FPlayerEmotionalProfile();
    CurrentPlayerProfile.EmotionalPreferences.Add(EEmotionalState::Wonder, 0.7f);
    CurrentPlayerProfile.EmotionalPreferences.Add(EEmotionalState::Curiosity, 0.8f);
    CurrentPlayerProfile.EmotionalPreferences.Add(EEmotionalState::Fear, 0.5f);
    CurrentPlayerProfile.EmotionalPreferences.Add(EEmotionalState::Discovery, 0.9f);
    
    CurrentPlayerProfile.QuestTypePreferences.Add(EQuestType::Observation, 0.8f);
    CurrentPlayerProfile.QuestTypePreferences.Add(EQuestType::Discovery, 0.9f);
    CurrentPlayerProfile.QuestTypePreferences.Add(EQuestType::Survival, 0.6f);
    CurrentPlayerProfile.QuestTypePreferences.Add(EQuestType::Domestication, 0.7f);
    
    // Load default design patterns
    LoadDefaultDesignPatterns();
    
    // Initialize narrative context
    CurrentNarrativeContext = FNarrativeContext();
    CurrentNarrativeContext.CurrentAct = EStoryAct::Act1_Arrival;
    CurrentNarrativeContext.PlayerEmotionalState = 0.5f;
    
    UE_LOG(LogTemp, Warning, TEXT("EmotionalQuestDesigner: Initialization complete"));
}

void UEmotionalQuestDesigner::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("EmotionalQuestDesigner: Shutting down emotional quest design system"));
    
    DesignPatterns.Empty();
    ActiveEmotionalArcs.Empty();
    
    Super::Deinitialize();
}

FName UEmotionalQuestDesigner::DesignQuestFromNarrativeBeat(const FStoryBeat& NarrativeBeat, const FNarrativeContext& Context)
{
    UE_LOG(LogTemp, Warning, TEXT("EmotionalQuestDesigner: Designing quest from narrative beat"));
    
    // Select appropriate emotional arc based on narrative beat
    FQuestEmotionalArc EmotionalArc = DesignEmotionalArc(NarrativeBeat.PrimaryTheme, NarrativeBeat.TargetEmotion);
    
    // Find matching design pattern
    FQuestDesignPattern SelectedPattern = SelectOptimalPattern(Context, NarrativeBeat.PrimaryTheme);
    
    // Create context variables for pattern instantiation
    TMap<FString, FString> ContextVariables;
    ContextVariables.Add(TEXT("Theme"), UEnum::GetValueAsString(NarrativeBeat.PrimaryTheme));
    ContextVariables.Add(TEXT("Emotion"), UEnum::GetValueAsString(NarrativeBeat.TargetEmotion));
    ContextVariables.Add(TEXT("Act"), UEnum::GetValueAsString(Context.CurrentAct));
    ContextVariables.Add(TEXT("Location"), Context.PlayerLocation.ToString());
    
    // Instantiate the pattern
    FName QuestID = InstantiatePattern(SelectedPattern, ContextVariables);
    
    // Store the emotional arc for this quest
    if (!QuestID.IsNone())
    {
        ActiveEmotionalArcs.Add(QuestID, EmotionalArc);
        UE_LOG(LogTemp, Warning, TEXT("EmotionalQuestDesigner: Created quest %s with emotional arc"), *QuestID.ToString());
    }
    
    return QuestID;
}

FName UEmotionalQuestDesigner::DesignEmergentQuest(const FEcosystemEvent& Event, const FNarrativeContext& Context)
{
    UE_LOG(LogTemp, Warning, TEXT("EmotionalQuestDesigner: Designing emergent quest from ecosystem event"));
    
    // Determine emotional response to ecosystem event
    EEmotionalState TargetEmotion = DetermineEmotionalResponseToEvent(Event);
    
    // Select theme based on event type
    ENarrativeTheme Theme = MapEventToTheme(Event);
    
    // Design emotional arc
    FQuestEmotionalArc EmotionalArc = DesignEmotionalArc(Theme, TargetEmotion);
    
    // Create emergent quest pattern
    FQuestDesignPattern EmergentPattern = CreateEmergentPattern(Event, Theme, EmotionalArc);
    
    // Create context variables
    TMap<FString, FString> ContextVariables;
    ContextVariables.Add(TEXT("EventType"), Event.EventType.ToString());
    ContextVariables.Add(TEXT("Location"), Event.Location.ToString());
    ContextVariables.Add(TEXT("Intensity"), FString::SanitizeFloat(Event.Intensity));
    
    // Instantiate the quest
    FName QuestID = InstantiatePattern(EmergentPattern, ContextVariables);
    
    if (!QuestID.IsNone())
    {
        ActiveEmotionalArcs.Add(QuestID, EmotionalArc);
        UE_LOG(LogTemp, Warning, TEXT("EmotionalQuestDesigner: Created emergent quest %s"), *QuestID.ToString());
    }
    
    return QuestID;
}

FName UEmotionalQuestDesigner::DesignPersonalizedQuest(const FPlayerEmotionalProfile& PlayerProfile, const FNarrativeContext& Context)
{
    UE_LOG(LogTemp, Warning, TEXT("EmotionalQuestDesigner: Designing personalized quest"));
    
    // Find the emotional state the player needs most
    EEmotionalState NeededEmotion = PredictPlayerEmotionalNeed(Context);
    
    // Find the quest type the player prefers
    EQuestType PreferredType = GetPreferredQuestType(PlayerProfile);
    
    // Map quest type to theme
    ENarrativeTheme Theme = MapQuestTypeToTheme(PreferredType);
    
    // Design personalized emotional arc
    FQuestEmotionalArc PersonalizedArc = DesignEmotionalArc(Theme, NeededEmotion);
    PersonalizedArc.IntensityCurve = PlayerProfile.ChallengePreference;
    
    // Adjust arc based on player preferences
    AdjustArcForPlayer(PersonalizedArc, PlayerProfile);
    
    // Select pattern that matches preferences
    FQuestDesignPattern PersonalizedPattern = SelectPatternForPlayer(PlayerProfile, Theme);
    
    // Create context variables
    TMap<FString, FString> ContextVariables;
    ContextVariables.Add(TEXT("PlayerType"), GetPlayerTypeString(PlayerProfile));
    ContextVariables.Add(TEXT("PreferredEmotion"), UEnum::GetValueAsString(NeededEmotion));
    ContextVariables.Add(TEXT("ChallengeLevel"), FString::SanitizeFloat(PlayerProfile.ChallengePreference));
    
    // Instantiate the quest
    FName QuestID = InstantiatePattern(PersonalizedPattern, ContextVariables);
    
    if (!QuestID.IsNone())
    {
        ActiveEmotionalArcs.Add(QuestID, PersonalizedArc);
        UE_LOG(LogTemp, Warning, TEXT("EmotionalQuestDesigner: Created personalized quest %s"), *QuestID.ToString());
    }
    
    return QuestID;
}

FQuestEmotionalArc UEmotionalQuestDesigner::DesignEmotionalArc(ENarrativeTheme Theme, EEmotionalState DesiredOutcome)
{
    FQuestEmotionalArc Arc;
    Arc.ResolutionEmotion = DesiredOutcome;
    
    // Design opening and middle emotions based on theme
    switch (Theme)
    {
        case ENarrativeTheme::SurvivalAgainstNature:
            Arc.OpeningEmotion = EEmotionalState::Tension;
            Arc.MiddleEmotion = EEmotionalState::Fear;
            Arc.IntensityCurve = 1.2f;
            break;
            
        case ENarrativeTheme::WonderOfDiscovery:
            Arc.OpeningEmotion = EEmotionalState::Curiosity;
            Arc.MiddleEmotion = EEmotionalState::Wonder;
            Arc.IntensityCurve = 0.8f;
            break;
            
        case ENarrativeTheme::ConnectionWithNature:
            Arc.OpeningEmotion = EEmotionalState::Curiosity;
            Arc.MiddleEmotion = EEmotionalState::Empathy;
            Arc.IntensityCurve = 0.9f;
            break;
            
        case ENarrativeTheme::IsolationAndLoneliness:
            Arc.OpeningEmotion = EEmotionalState::Loneliness;
            Arc.MiddleEmotion = EEmotionalState::Determination;
            Arc.IntensityCurve = 1.1f;
            break;
            
        default:
            Arc.OpeningEmotion = EEmotionalState::Curiosity;
            Arc.MiddleEmotion = EEmotionalState::Tension;
            Arc.IntensityCurve = 1.0f;
            break;
    }
    
    // Set emotional payoff
    Arc.EmotionalPayoff = FText::FromString(GenerateEmotionalPayoffText(Theme, DesiredOutcome));
    
    UE_LOG(LogTemp, Warning, TEXT("EmotionalQuestDesigner: Designed emotional arc %s -> %s -> %s"), 
           *UEnum::GetValueAsString(Arc.OpeningEmotion),
           *UEnum::GetValueAsString(Arc.MiddleEmotion),
           *UEnum::GetValueAsString(Arc.ResolutionEmotion));
    
    return Arc;
}

bool UEmotionalQuestDesigner::ValidateEmotionalFlow(const FQuestData& QuestData, const FQuestEmotionalArc& Arc)
{
    // Check if quest structure supports the emotional arc
    if (QuestData.Objectives.Num() < 3)
    {
        UE_LOG(LogTemp, Warning, TEXT("EmotionalQuestDesigner: Quest too short for proper emotional arc"));
        return false;
    }
    
    // Validate emotional progression
    bool bHasOpening = false;
    bool bHasMiddle = false;
    bool bHasResolution = false;
    
    for (const FQuestObjective& Objective : QuestData.Objectives)
    {
        // Check if objectives support emotional beats
        if (Objective.Type == EObjectiveType::Observe && Arc.OpeningEmotion == EEmotionalState::Curiosity)
        {
            bHasOpening = true;
        }
        else if (Objective.Type == EObjectiveType::Survive && Arc.MiddleEmotion == EEmotionalState::Fear)
        {
            bHasMiddle = true;
        }
        else if (Objective.Type == EObjectiveType::Discover && Arc.ResolutionEmotion == EEmotionalState::Achievement)
        {
            bHasResolution = true;
        }
    }
    
    bool bIsValid = bHasOpening && bHasMiddle && bHasResolution;
    
    UE_LOG(LogTemp, Warning, TEXT("EmotionalQuestDesigner: Emotional flow validation %s"), 
           bIsValid ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bIsValid;
}

void UEmotionalQuestDesigner::AdjustQuestPacing(FQuestData& QuestData, const FPlayerEmotionalProfile& PlayerProfile)
{
    UE_LOG(LogTemp, Warning, TEXT("EmotionalQuestDesigner: Adjusting quest pacing for player profile"));
    
    // Adjust objective durations based on player attention span
    float PacingMultiplier = PlayerProfile.AttentionSpan / 600.0f; // 600s is baseline
    
    for (FQuestObjective& Objective : QuestData.Objectives)
    {
        // Adjust time limits
        if (Objective.TimeLimit > 0)
        {
            Objective.TimeLimit *= PacingMultiplier;
        }
        
        // Adjust complexity based on player preferences
        if (PlayerProfile.ChallengePreference > 0.7f)
        {
            // Player likes challenge - add complexity
            Objective.RequiredCount = FMath::Max(Objective.RequiredCount, Objective.RequiredCount + 1);
        }
        else if (PlayerProfile.ChallengePreference < 0.3f)
        {
            // Player prefers easier content
            Objective.RequiredCount = FMath::Max(1, Objective.RequiredCount - 1);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EmotionalQuestDesigner: Quest pacing adjusted with multiplier %f"), PacingMultiplier);
}

FQuestDesignPattern UEmotionalQuestDesigner::SelectOptimalPattern(const FNarrativeContext& Context, ENarrativeTheme DesiredTheme)
{
    UE_LOG(LogTemp, Warning, TEXT("EmotionalQuestDesigner: Selecting optimal pattern for theme %s"), 
           *UEnum::GetValueAsString(DesiredTheme));
    
    // Filter patterns by theme
    TArray<FQuestDesignPattern> MatchingPatterns;
    for (const FQuestDesignPattern& Pattern : DesignPatterns)
    {
        if (Pattern.PrimaryTheme == DesiredTheme)
        {
            MatchingPatterns.Add(Pattern);
        }
    }
    
    if (MatchingPatterns.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("EmotionalQuestDesigner: No patterns found for theme, using default"));
        return CreateDefaultPattern(DesiredTheme);
    }
    
    // Score patterns based on context
    FQuestDesignPattern BestPattern = MatchingPatterns[0];
    float BestScore = 0.0f;
    
    for (const FQuestDesignPattern& Pattern : MatchingPatterns)
    {
        float Score = ScorePatternForContext(Pattern, Context);
        if (Score > BestScore)
        {
            BestScore = Score;
            BestPattern = Pattern;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EmotionalQuestDesigner: Selected pattern %s with score %f"), 
           *BestPattern.PatternName.ToString(), BestScore);
    
    return BestPattern;
}

void UEmotionalQuestDesigner::LoadDefaultDesignPatterns()
{
    UE_LOG(LogTemp, Warning, TEXT("EmotionalQuestDesigner: Loading default design patterns"));
    
    // Wonder of Discovery Pattern
    FQuestDesignPattern WonderPattern;
    WonderPattern.PatternID = FName("WonderDiscovery");
    WonderPattern.PatternName = FText::FromString("Wonder of Discovery");
    WonderPattern.PrimaryTheme = ENarrativeTheme::WonderOfDiscovery;
    WonderPattern.EmotionalArc.OpeningEmotion = EEmotionalState::Curiosity;
    WonderPattern.EmotionalArc.MiddleEmotion = EEmotionalState::Wonder;
    WonderPattern.EmotionalArc.ResolutionEmotion = EEmotionalState::Discovery;
    WonderPattern.ObjectiveSequence = {EObjectiveType::Observe, EObjectiveType::Discover, EObjectiveType::Document};
    WonderPattern.OptimalDuration = 900.0f;
    WonderPattern.EmotionalWeight = 1.2f;
    DesignPatterns.Add(WonderPattern);
    
    // Survival Challenge Pattern
    FQuestDesignPattern SurvivalPattern;
    SurvivalPattern.PatternID = FName("SurvivalChallenge");
    SurvivalPattern.PatternName = FText::FromString("Survival Challenge");
    SurvivalPattern.PrimaryTheme = ENarrativeTheme::SurvivalAgainstNature;
    SurvivalPattern.EmotionalArc.OpeningEmotion = EEmotionalState::Tension;
    SurvivalPattern.EmotionalArc.MiddleEmotion = EEmotionalState::Fear;
    SurvivalPattern.EmotionalArc.ResolutionEmotion = EEmotionalState::Achievement;
    SurvivalPattern.ObjectiveSequence = {EObjectiveType::Survive, EObjectiveType::Escape, EObjectiveType::Overcome};
    SurvivalPattern.OptimalDuration = 1200.0f;
    SurvivalPattern.EmotionalWeight = 1.5f;
    DesignPatterns.Add(SurvivalPattern);
    
    // Connection Pattern
    FQuestDesignPattern ConnectionPattern;
    ConnectionPattern.PatternID = FName("NatureConnection");
    ConnectionPattern.PatternName = FText::FromString("Connection with Nature");
    ConnectionPattern.PrimaryTheme = ENarrativeTheme::ConnectionWithNature;
    ConnectionPattern.EmotionalArc.OpeningEmotion = EEmotionalState::Curiosity;
    ConnectionPattern.EmotionalArc.MiddleEmotion = EEmotionalState::Empathy;
    ConnectionPattern.EmotionalArc.ResolutionEmotion = EEmotionalState::Connection;
    ConnectionPattern.ObjectiveSequence = {EObjectiveType::Observe, EObjectiveType::Domesticate, EObjectiveType::Bond};
    ConnectionPattern.OptimalDuration = 1800.0f;
    ConnectionPattern.EmotionalWeight = 1.3f;
    DesignPatterns.Add(ConnectionPattern);
    
    UE_LOG(LogTemp, Warning, TEXT("EmotionalQuestDesigner: Loaded %d default design patterns"), DesignPatterns.Num());
}

// Helper functions implementation
EEmotionalState UEmotionalQuestDesigner::DetermineEmotionalResponseToEvent(const FEcosystemEvent& Event)
{
    // Map ecosystem events to emotional responses
    if (Event.EventType.MatchesTag(FGameplayTag::RequestGameplayTag("Event.Predator.Hunt")))
    {
        return EEmotionalState::Fear;
    }
    else if (Event.EventType.MatchesTag(FGameplayTag::RequestGameplayTag("Event.Discovery.NewSpecies")))
    {
        return EEmotionalState::Wonder;
    }
    else if (Event.EventType.MatchesTag(FGameplayTag::RequestGameplayTag("Event.Social.Bonding")))
    {
        return EEmotionalState::Connection;
    }
    
    return EEmotionalState::Curiosity; // Default
}

ENarrativeTheme UEmotionalQuestDesigner::MapEventToTheme(const FEcosystemEvent& Event)
{
    if (Event.EventType.MatchesTag(FGameplayTag::RequestGameplayTag("Event.Predator")))
    {
        return ENarrativeTheme::SurvivalAgainstNature;
    }
    else if (Event.EventType.MatchesTag(FGameplayTag::RequestGameplayTag("Event.Discovery")))
    {
        return ENarrativeTheme::WonderOfDiscovery;
    }
    else if (Event.EventType.MatchesTag(FGameplayTag::RequestGameplayTag("Event.Social")))
    {
        return ENarrativeTheme::ConnectionWithNature;
    }
    
    return ENarrativeTheme::WonderOfDiscovery; // Default
}

EQuestType UEmotionalQuestDesigner::GetPreferredQuestType(const FPlayerEmotionalProfile& PlayerProfile)
{
    EQuestType PreferredType = EQuestType::Discovery;
    float HighestPreference = 0.0f;
    
    for (const auto& Preference : PlayerProfile.QuestTypePreferences)
    {
        if (Preference.Value > HighestPreference)
        {
            HighestPreference = Preference.Value;
            PreferredType = Preference.Key;
        }
    }
    
    return PreferredType;
}

FString UEmotionalQuestDesigner::GenerateEmotionalPayoffText(ENarrativeTheme Theme, EEmotionalState Emotion)
{
    switch (Theme)
    {
        case ENarrativeTheme::WonderOfDiscovery:
            return "The thrill of uncovering nature's secrets fills you with wonder";
        case ENarrativeTheme::SurvivalAgainstNature:
            return "Against all odds, you have proven your resilience";
        case ENarrativeTheme::ConnectionWithNature:
            return "A profound bond forms between you and the ancient world";
        default:
            return "You feel a deep sense of accomplishment";
    }
}

float UEmotionalQuestDesigner::ScorePatternForContext(const FQuestDesignPattern& Pattern, const FNarrativeContext& Context)
{
    float Score = 1.0f;
    
    // Score based on current act appropriateness
    if (Context.CurrentAct == EStoryAct::Act1_Arrival && Pattern.OptimalDuration < 600.0f)
    {
        Score += 0.5f; // Prefer shorter quests in Act 1
    }
    
    // Score based on recent quest completion
    if (Context.RecentlyCompletedQuests.Contains(Pattern.PatternID))
    {
        Score -= 0.8f; // Avoid repetition
    }
    
    // Score based on player emotional state
    float EmotionalAlignment = FMath::Abs(Context.PlayerEmotionalState - 0.5f);
    Score += (1.0f - EmotionalAlignment) * 0.3f;
    
    return Score;
}

FQuestDesignPattern UEmotionalQuestDesigner::CreateDefaultPattern(ENarrativeTheme Theme)
{
    FQuestDesignPattern DefaultPattern;
    DefaultPattern.PatternID = FName("Default");
    DefaultPattern.PatternName = FText::FromString("Default Pattern");
    DefaultPattern.PrimaryTheme = Theme;
    DefaultPattern.EmotionalArc.OpeningEmotion = EEmotionalState::Curiosity;
    DefaultPattern.EmotionalArc.MiddleEmotion = EEmotionalState::Tension;
    DefaultPattern.EmotionalArc.ResolutionEmotion = EEmotionalState::Achievement;
    DefaultPattern.ObjectiveSequence = {EObjectiveType::Observe, EObjectiveType::Discover};
    DefaultPattern.OptimalDuration = 600.0f;
    DefaultPattern.EmotionalWeight = 1.0f;
    
    return DefaultPattern;
}