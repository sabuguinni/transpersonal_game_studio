#include "Quest_NarrativeQuestController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UQuest_NarrativeQuestController::UQuest_NarrativeQuestController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f;

    // Initialize narrative settings
    NarrativeUpdateInterval = 2.0f;
    EmotionalImpactThreshold = 0.7f;
    bEnableDynamicNarrative = true;
    MaxConcurrentArcs = 5;
    LastNarrativeUpdate = 0.0f;

    // Initialize default narrative data
    CurrentNarrativeData = FQuest_NarrativeData();
    CurrentNarrativeData.QuestTitle = TEXT("The First Steps");
    CurrentNarrativeData.QuestDescription = TEXT("Begin your journey in the prehistoric world");
    CurrentNarrativeData.NarrativeType = EQuest_NarrativeType::Discovery;
    CurrentNarrativeData.EmotionalTone = EQuest_EmotionalTone::Hopeful;
}

void UQuest_NarrativeQuestController::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_NarrativeQuestController: Narrative quest system initialized"));
    
    // Initialize narrative event tracking
    NarrativeEventHistory.Empty();
    CharacterProgressionMap.Empty();
    
    // Start narrative processing
    ProcessNarrativeEvents();
}

void UQuest_NarrativeQuestController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bEnableDynamicNarrative) return;

    LastNarrativeUpdate += DeltaTime;
    
    if (LastNarrativeUpdate >= NarrativeUpdateInterval)
    {
        UpdateCharacterArcs(DeltaTime);
        ValidateNarrativeConsistency();
        EvaluateEmotionalImpact();
        
        LastNarrativeUpdate = 0.0f;
    }
}

void UQuest_NarrativeQuestController::InitializeNarrativeQuest(const FQuest_NarrativeData& QuestData)
{
    CurrentNarrativeData = QuestData;
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_NarrativeQuestController: Initialized narrative quest '%s'"), 
           *QuestData.QuestTitle);
    
    // Log narrative event
    LogNarrativeEvent(FString::Printf(TEXT("Quest Initialized: %s"), *QuestData.QuestTitle));
    
    // Reset story progression
    CurrentNarrativeData.CurrentStoryBeat = EQuest_StoryBeat::Introduction;
    
    // Broadcast quest initialization if needed
    ProcessNarrativeEvents();
}

void UQuest_NarrativeQuestController::AdvanceStoryBeat()
{
    // Advance to next story beat
    switch (CurrentNarrativeData.CurrentStoryBeat)
    {
        case EQuest_StoryBeat::Introduction:
            CurrentNarrativeData.CurrentStoryBeat = EQuest_StoryBeat::RisingAction;
            break;
        case EQuest_StoryBeat::RisingAction:
            CurrentNarrativeData.CurrentStoryBeat = EQuest_StoryBeat::Climax;
            break;
        case EQuest_StoryBeat::Climax:
            CurrentNarrativeData.CurrentStoryBeat = EQuest_StoryBeat::FallingAction;
            break;
        case EQuest_StoryBeat::FallingAction:
            CurrentNarrativeData.CurrentStoryBeat = EQuest_StoryBeat::Resolution;
            break;
        case EQuest_StoryBeat::Resolution:
            CurrentNarrativeData.CurrentStoryBeat = EQuest_StoryBeat::Epilogue;
            break;
        case EQuest_StoryBeat::Epilogue:
            // Quest complete - could trigger new quest
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_NarrativeQuestController: Advanced to story beat %d"), 
           (int32)CurrentNarrativeData.CurrentStoryBeat);
    
    LogNarrativeEvent(TEXT("Story Beat Advanced"));
}

void UQuest_NarrativeQuestController::UpdateEmotionalTone(EQuest_EmotionalTone NewTone)
{
    EQuest_EmotionalTone PreviousTone = CurrentNarrativeData.EmotionalTone;
    CurrentNarrativeData.EmotionalTone = NewTone;
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_NarrativeQuestController: Emotional tone changed from %d to %d"), 
           (int32)PreviousTone, (int32)NewTone);
    
    // Adjust emotional impact based on tone change
    switch (NewTone)
    {
        case EQuest_EmotionalTone::Triumphant:
        case EQuest_EmotionalTone::Hopeful:
            CurrentNarrativeData.EmotionalImpact = FMath::Min(1.0f, CurrentNarrativeData.EmotionalImpact + 0.2f);
            break;
        case EQuest_EmotionalTone::Tense:
        case EQuest_EmotionalTone::Urgent:
            CurrentNarrativeData.EmotionalImpact = FMath::Min(1.0f, CurrentNarrativeData.EmotionalImpact + 0.15f);
            break;
        case EQuest_EmotionalTone::Melancholic:
            CurrentNarrativeData.EmotionalImpact = FMath::Max(0.0f, CurrentNarrativeData.EmotionalImpact - 0.1f);
            break;
        default:
            break;
    }
    
    LogNarrativeEvent(FString::Printf(TEXT("Emotional Tone Updated: %d"), (int32)NewTone));
}

void UQuest_NarrativeQuestController::TriggerNarrativeEvent(const FString& EventName)
{
    UE_LOG(LogTemp, Warning, TEXT("Quest_NarrativeQuestController: Narrative event triggered: %s"), *EventName);
    
    LogNarrativeEvent(EventName);
    
    // Process event-specific logic
    if (EventName.Contains(TEXT("Discovery")))
    {
        UpdateEmotionalTone(EQuest_EmotionalTone::Mysterious);
    }
    else if (EventName.Contains(TEXT("Combat")))
    {
        UpdateEmotionalTone(EQuest_EmotionalTone::Tense);
    }
    else if (EventName.Contains(TEXT("Success")))
    {
        UpdateEmotionalTone(EQuest_EmotionalTone::Triumphant);
        AdvanceStoryBeat();
    }
    else if (EventName.Contains(TEXT("Failure")))
    {
        UpdateEmotionalTone(EQuest_EmotionalTone::Melancholic);
    }
}

void UQuest_NarrativeQuestController::RegisterCharacterArc(const FQuest_CharacterArc& CharacterArc)
{
    if (ActiveCharacterArcs.Num() >= MaxConcurrentArcs)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_NarrativeQuestController: Maximum character arcs reached"));
        return;
    }
    
    // Check if character already has an arc
    for (int32 i = 0; i < ActiveCharacterArcs.Num(); i++)
    {
        if (ActiveCharacterArcs[i].CharacterName == CharacterArc.CharacterName)
        {
            // Update existing arc
            ActiveCharacterArcs[i] = CharacterArc;
            UE_LOG(LogTemp, Warning, TEXT("Quest_NarrativeQuestController: Updated character arc for %s"), 
                   *CharacterArc.CharacterName);
            return;
        }
    }
    
    // Add new character arc
    ActiveCharacterArcs.Add(CharacterArc);
    CharacterProgressionMap.Add(CharacterArc.CharacterName, CharacterArc.ProgressionLevel);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_NarrativeQuestController: Registered new character arc for %s"), 
           *CharacterArc.CharacterName);
    
    LogNarrativeEvent(FString::Printf(TEXT("Character Arc Registered: %s"), *CharacterArc.CharacterName));
}

void UQuest_NarrativeQuestController::UpdateCharacterProgression(const FString& CharacterName, float ProgressDelta)
{
    if (float* CurrentProgress = CharacterProgressionMap.Find(CharacterName))
    {
        *CurrentProgress = FMath::Clamp(*CurrentProgress + ProgressDelta, 0.0f, 1.0f);
        
        // Update the character arc
        for (FQuest_CharacterArc& Arc : ActiveCharacterArcs)
        {
            if (Arc.CharacterName == CharacterName)
            {
                Arc.ProgressionLevel = *CurrentProgress;
                
                // Check if arc is complete
                if (Arc.ProgressionLevel >= 1.0f && !Arc.bArcCompleted)
                {
                    Arc.bArcCompleted = true;
                    TriggerNarrativeEvent(FString::Printf(TEXT("Character Arc Complete: %s"), *CharacterName));
                }
                break;
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Quest_NarrativeQuestController: Updated progression for %s: %.2f"), 
               *CharacterName, *CurrentProgress);
    }
}

bool UQuest_NarrativeQuestController::IsCharacterArcComplete(const FString& CharacterName) const
{
    for (const FQuest_CharacterArc& Arc : ActiveCharacterArcs)
    {
        if (Arc.CharacterName == CharacterName)
        {
            return Arc.bArcCompleted;
        }
    }
    return false;
}

FQuest_NarrativeData UQuest_NarrativeQuestController::GenerateContextualQuest(EQuest_NarrativeType PreferredType)
{
    FQuest_NarrativeData GeneratedQuest;
    GeneratedQuest.NarrativeType = PreferredType;
    
    // Generate quest based on type
    switch (PreferredType)
    {
        case EQuest_NarrativeType::Discovery:
            GeneratedQuest.QuestTitle = TEXT("Uncharted Territory");
            GeneratedQuest.QuestDescription = TEXT("Explore the unknown regions and discover what lies beyond");
            GeneratedQuest.EmotionalTone = EQuest_EmotionalTone::Mysterious;
            break;
            
        case EQuest_NarrativeType::Investigation:
            GeneratedQuest.QuestTitle = TEXT("Traces of the Past");
            GeneratedQuest.QuestDescription = TEXT("Investigate the strange markings and uncover their meaning");
            GeneratedQuest.EmotionalTone = EQuest_EmotionalTone::Tense;
            break;
            
        case EQuest_NarrativeType::Social:
            GeneratedQuest.QuestTitle = TEXT("Bonds of Survival");
            GeneratedQuest.QuestDescription = TEXT("Build relationships with fellow survivors");
            GeneratedQuest.EmotionalTone = EQuest_EmotionalTone::Hopeful;
            break;
            
        case EQuest_NarrativeType::Survival:
            GeneratedQuest.QuestTitle = TEXT("Against All Odds");
            GeneratedQuest.QuestDescription = TEXT("Survive the harsh conditions and deadly predators");
            GeneratedQuest.EmotionalTone = EQuest_EmotionalTone::Urgent;
            break;
            
        default:
            GeneratedQuest.QuestTitle = TEXT("The Journey Continues");
            GeneratedQuest.QuestDescription = TEXT("Face the challenges ahead");
            GeneratedQuest.EmotionalTone = EQuest_EmotionalTone::Neutral;
            break;
    }
    
    GeneratedQuest.EmotionalImpact = FMath::RandRange(0.3f, 0.8f);
    GeneratedQuest.StoryChapter = CurrentNarrativeData.StoryChapter;
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_NarrativeQuestController: Generated contextual quest: %s"), 
           *GeneratedQuest.QuestTitle);
    
    return GeneratedQuest;
}

void UQuest_NarrativeQuestController::AdaptQuestToPlayerActions(const TArray<FString>& PlayerActions)
{
    for (const FString& Action : PlayerActions)
    {
        if (Action.Contains(TEXT("Aggressive")))
        {
            UpdateEmotionalTone(EQuest_EmotionalTone::Tense);
        }
        else if (Action.Contains(TEXT("Peaceful")))
        {
            UpdateEmotionalTone(EQuest_EmotionalTone::Peaceful);
        }
        else if (Action.Contains(TEXT("Explore")))
        {
            UpdateEmotionalTone(EQuest_EmotionalTone::Mysterious);
        }
        else if (Action.Contains(TEXT("Help")))
        {
            UpdateEmotionalTone(EQuest_EmotionalTone::Hopeful);
        }
    }
    
    LogNarrativeEvent(TEXT("Quest Adapted to Player Actions"));
}

void UQuest_NarrativeQuestController::EvaluateEmotionalImpact()
{
    // Calculate emotional impact based on recent events
    float ImpactSum = 0.0f;
    int32 RecentEvents = FMath::Min(NarrativeEventHistory.Num(), 5);
    
    for (int32 i = NarrativeEventHistory.Num() - RecentEvents; i < NarrativeEventHistory.Num(); i++)
    {
        const FString& Event = NarrativeEventHistory[i];
        if (Event.Contains(TEXT("Success")) || Event.Contains(TEXT("Complete")))
        {
            ImpactSum += 0.2f;
        }
        else if (Event.Contains(TEXT("Failure")) || Event.Contains(TEXT("Death")))
        {
            ImpactSum -= 0.15f;
        }
    }
    
    CurrentNarrativeData.EmotionalImpact = FMath::Clamp(CurrentNarrativeData.EmotionalImpact + ImpactSum, 0.0f, 1.0f);
    
    // Trigger high impact events
    if (CurrentNarrativeData.EmotionalImpact >= EmotionalImpactThreshold)
    {
        TriggerNarrativeEvent(TEXT("High Emotional Impact Reached"));
    }
}

float UQuest_NarrativeQuestController::CalculateNarrativeCoherence() const
{
    float Coherence = 0.5f; // Base coherence
    
    // Factor in story beat progression
    if (CurrentNarrativeData.CurrentStoryBeat != EQuest_StoryBeat::Introduction)
    {
        Coherence += 0.2f;
    }
    
    // Factor in character arc completion
    int32 CompletedArcs = 0;
    for (const FQuest_CharacterArc& Arc : ActiveCharacterArcs)
    {
        if (Arc.bArcCompleted)
        {
            CompletedArcs++;
        }
    }
    
    if (ActiveCharacterArcs.Num() > 0)
    {
        Coherence += (float(CompletedArcs) / float(ActiveCharacterArcs.Num())) * 0.3f;
    }
    
    return FMath::Clamp(Coherence, 0.0f, 1.0f);
}

bool UQuest_NarrativeQuestController::ValidateQuestProgression() const
{
    // Check if quest progression makes narrative sense
    bool bValidProgression = true;
    
    // Validate story beat sequence
    if (CurrentNarrativeData.CurrentStoryBeat == EQuest_StoryBeat::Epilogue && 
        CurrentNarrativeData.EmotionalImpact < 0.3f)
    {
        bValidProgression = false;
    }
    
    // Validate character arcs
    for (const FQuest_CharacterArc& Arc : ActiveCharacterArcs)
    {
        if (Arc.bArcCompleted && Arc.ProgressionLevel < 0.8f)
        {
            bValidProgression = false;
        }
    }
    
    return bValidProgression;
}

void UQuest_NarrativeQuestController::OptimizeNarrativeFlow()
{
    // Optimize narrative pacing and flow
    if (NarrativeEventHistory.Num() > 20)
    {
        // Remove oldest events to maintain performance
        NarrativeEventHistory.RemoveAt(0, 10);
    }
    
    // Balance emotional impact
    if (CurrentNarrativeData.EmotionalImpact > 0.9f)
    {
        CurrentNarrativeData.EmotionalImpact *= 0.9f; // Slight reduction
        UpdateEmotionalTone(EQuest_EmotionalTone::Peaceful);
    }
    else if (CurrentNarrativeData.EmotionalImpact < 0.1f)
    {
        CurrentNarrativeData.EmotionalImpact += 0.1f; // Slight increase
        UpdateEmotionalTone(EQuest_EmotionalTone::Hopeful);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_NarrativeQuestController: Narrative flow optimized"));
}

void UQuest_NarrativeQuestController::ProcessNarrativeEvents()
{
    // Process any pending narrative events
    UE_LOG(LogTemp, Warning, TEXT("Quest_NarrativeQuestController: Processing narrative events"));
    
    // Update quest state based on current conditions
    if (CurrentNarrativeData.CurrentStoryBeat == EQuest_StoryBeat::Introduction)
    {
        // Initialize quest objectives
        CurrentNarrativeData.ObjectiveTexts.Empty();
        CurrentNarrativeData.ObjectiveTexts.Add(TEXT("Begin your survival journey"));
        CurrentNarrativeData.ObjectiveTexts.Add(TEXT("Learn the basics of survival"));
    }
}

void UQuest_NarrativeQuestController::UpdateCharacterArcs(float DeltaTime)
{
    // Update character arc progression over time
    for (FQuest_CharacterArc& Arc : ActiveCharacterArcs)
    {
        if (!Arc.bArcCompleted && Arc.ProgressionLevel < 1.0f)
        {
            // Slow natural progression
            Arc.ProgressionLevel += DeltaTime * 0.01f; // 1% per second
            Arc.ProgressionLevel = FMath::Clamp(Arc.ProgressionLevel, 0.0f, 1.0f);
            
            // Update progression map
            if (float* MapProgress = CharacterProgressionMap.Find(Arc.CharacterName))
            {
                *MapProgress = Arc.ProgressionLevel;
            }
        }
    }
}

void UQuest_NarrativeQuestController::ValidateNarrativeConsistency()
{
    // Validate narrative consistency
    if (!ValidateQuestProgression())
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_NarrativeQuestController: Narrative inconsistency detected - optimizing"));
        OptimizeNarrativeFlow();
    }
}

FString UQuest_NarrativeQuestController::GenerateContextualDialogue(EQuest_EmotionalTone Tone) const
{
    // Generate dialogue based on emotional tone
    switch (Tone)
    {
        case EQuest_EmotionalTone::Hopeful:
            return TEXT("There's still hope. We can overcome this together.");
        case EQuest_EmotionalTone::Tense:
            return TEXT("Stay alert. Danger lurks in every shadow.");
        case EQuest_EmotionalTone::Mysterious:
            return TEXT("Something strange is happening here. We must investigate.");
        case EQuest_EmotionalTone::Triumphant:
            return TEXT("Victory is ours! We have proven ourselves.");
        case EQuest_EmotionalTone::Urgent:
            return TEXT("Time is running out. We must act now!");
        default:
            return TEXT("The path ahead is uncertain, but we must continue.");
    }
}

void UQuest_NarrativeQuestController::LogNarrativeEvent(const FString& Event)
{
    FString TimestampedEvent = FString::Printf(TEXT("[%.2f] %s"), 
                                              GetWorld()->GetTimeSeconds(), *Event);
    NarrativeEventHistory.Add(TimestampedEvent);
    
    UE_LOG(LogTemp, Log, TEXT("Quest_NarrativeQuestController: %s"), *TimestampedEvent);
}