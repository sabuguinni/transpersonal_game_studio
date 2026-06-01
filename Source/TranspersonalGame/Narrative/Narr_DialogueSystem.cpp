#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    CurrentPlayingDialogue = "";
    DialogueStartTime = 0.0f;
}

void UNarr_DialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Narrative Dialogue System Initialized"));
    
    InitializeDefaultDialogues();
    InitializeStoryProgression();
}

void UNarr_DialogueSystem::Deinitialize()
{
    StopCurrentDialogue();
    DialogueDatabase.Empty();
    StoryProgressMap.Empty();
    
    Super::Deinitialize();
}

void UNarr_DialogueSystem::TriggerDialogue(const FString& DialogueID, AActor* Speaker, AActor* Listener)
{
    if (!IsDialogueAvailable(DialogueID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue %s not available or not found"), *DialogueID);
        return;
    }

    FNarr_DialogueEntry DialogueEntry = GetDialogue(DialogueID);
    
    if (!CheckDialoguePrerequisites(DialogueEntry))
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue %s prerequisites not met"), *DialogueID);
        return;
    }

    // Stop current dialogue if playing
    StopCurrentDialogue();

    // Set current dialogue
    CurrentPlayingDialogue = DialogueID;
    DialogueStartTime = GetWorld()->GetTimeSeconds();

    // Display dialogue text (in a real implementation, this would trigger UI)
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("[%s]: %s"), 
            *UEnum::GetValueAsString(DialogueEntry.SpeakerType), 
            *DialogueEntry.DialogueText.ToString());
        
        GEngine->AddOnScreenDebugMessage(-1, DialogueEntry.Duration, FColor::Yellow, DisplayText);
    }

    // Play audio if available
    if (!DialogueEntry.AudioFilePath.IsEmpty())
    {
        PlayDialogueAudio(DialogueID);
    }

    // Log dialogue trigger
    UE_LOG(LogTemp, Log, TEXT("Triggered dialogue: %s by %s"), 
        *DialogueID, 
        *UEnum::GetValueAsString(DialogueEntry.SpeakerType));

    // Schedule dialogue completion
    FTimerHandle DialogueTimer;
    GetWorld()->GetTimerManager().SetTimer(DialogueTimer, 
        FTimerDelegate::CreateLambda([this, DialogueID]()
        {
            OnDialogueCompleted(DialogueID);
        }), 
        DialogueEntry.Duration, false);
}

bool UNarr_DialogueSystem::IsDialogueAvailable(const FString& DialogueID) const
{
    return DialogueDatabase.Contains(DialogueID);
}

void UNarr_DialogueSystem::RegisterDialogue(const FNarr_DialogueEntry& DialogueEntry)
{
    DialogueDatabase.Add(DialogueEntry.DialogueID, DialogueEntry);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue: %s"), *DialogueEntry.DialogueID);
}

FNarr_DialogueEntry UNarr_DialogueSystem::GetDialogue(const FString& DialogueID) const
{
    if (const FNarr_DialogueEntry* FoundDialogue = DialogueDatabase.Find(DialogueID))
    {
        return *FoundDialogue;
    }
    
    return FNarr_DialogueEntry();
}

void UNarr_DialogueSystem::AdvanceStory(const FString& StoryID)
{
    if (FNarr_StoryProgress* StoryProgress = StoryProgressMap.Find(StoryID))
    {
        if (!StoryProgress->bIsCompleted && StoryProgress->CurrentStep < StoryProgress->TotalSteps)
        {
            StoryProgress->CurrentStep++;
            
            if (StoryProgress->CurrentStep >= StoryProgress->TotalSteps)
            {
                CompleteStory(StoryID);
            }
            
            UE_LOG(LogTemp, Log, TEXT("Advanced story %s to step %d/%d"), 
                *StoryID, StoryProgress->CurrentStep, StoryProgress->TotalSteps);
        }
    }
}

void UNarr_DialogueSystem::SetStoryProgress(const FString& StoryID, int32 NewStep)
{
    if (FNarr_StoryProgress* StoryProgress = StoryProgressMap.Find(StoryID))
    {
        StoryProgress->CurrentStep = FMath::Clamp(NewStep, 0, StoryProgress->TotalSteps);
        
        if (StoryProgress->CurrentStep >= StoryProgress->TotalSteps)
        {
            CompleteStory(StoryID);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Set story %s progress to %d/%d"), 
            *StoryID, StoryProgress->CurrentStep, StoryProgress->TotalSteps);
    }
}

FNarr_StoryProgress UNarr_DialogueSystem::GetStoryProgress(const FString& StoryID) const
{
    if (const FNarr_StoryProgress* FoundProgress = StoryProgressMap.Find(StoryID))
    {
        return *FoundProgress;
    }
    
    return FNarr_StoryProgress();
}

void UNarr_DialogueSystem::CompleteStory(const FString& StoryID)
{
    if (FNarr_StoryProgress* StoryProgress = StoryProgressMap.Find(StoryID))
    {
        StoryProgress->bIsCompleted = true;
        StoryProgress->CurrentStep = StoryProgress->TotalSteps;
        
        UE_LOG(LogTemp, Warning, TEXT("Story completed: %s"), *StoryID);
        
        // Unlock related dialogues
        for (const FString& UnlockedDialogue : StoryProgress->UnlockedDialogues)
        {
            // Mark dialogue as available (implementation depends on dialogue availability system)
            UE_LOG(LogTemp, Log, TEXT("Unlocked dialogue: %s"), *UnlockedDialogue);
        }
    }
}

void UNarr_DialogueSystem::TriggerContextDialogue(ENarr_DialogueType DialogueType, const FVector& Location)
{
    // Find appropriate dialogue based on context
    for (const auto& DialoguePair : DialogueDatabase)
    {
        const FNarr_DialogueEntry& Dialogue = DialoguePair.Value;
        
        if (Dialogue.DialogueType == DialogueType && CheckDialoguePrerequisites(Dialogue))
        {
            TriggerDialogue(Dialogue.DialogueID);
            break;
        }
    }
}

void UNarr_DialogueSystem::TriggerNPCDialogue(ENarr_NPCType NPCType, const FString& Context)
{
    // Find dialogue for specific NPC type
    for (const auto& DialoguePair : DialogueDatabase)
    {
        const FNarr_DialogueEntry& Dialogue = DialoguePair.Value;
        
        if (Dialogue.SpeakerType == NPCType && CheckDialoguePrerequisites(Dialogue))
        {
            TriggerDialogue(Dialogue.DialogueID);
            break;
        }
    }
}

void UNarr_DialogueSystem::PlayDialogueAudio(const FString& DialogueID)
{
    if (const FNarr_DialogueEntry* DialogueEntry = DialogueDatabase.Find(DialogueID))
    {
        // In a full implementation, this would load and play the audio file
        UE_LOG(LogTemp, Log, TEXT("Playing audio for dialogue: %s (File: %s)"), 
            *DialogueID, *DialogueEntry->AudioFilePath);
    }
}

void UNarr_DialogueSystem::StopCurrentDialogue()
{
    if (!CurrentPlayingDialogue.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("Stopping dialogue: %s"), *CurrentPlayingDialogue);
        CurrentPlayingDialogue = "";
        DialogueStartTime = 0.0f;
    }
}

void UNarr_DialogueSystem::InitializeDefaultDialogues()
{
    // Tutorial dialogues
    FNarr_DialogueEntry TutorialStart;
    TutorialStart.DialogueID = "TUTORIAL_START";
    TutorialStart.DialogueText = FText::FromString("Welcome, survivor. This harsh world will test every instinct you possess. Learn quickly, or perish.");
    TutorialStart.DialogueType = ENarr_DialogueType::Tutorial;
    TutorialStart.SpeakerType = ENarr_NPCType::Survivor;
    TutorialStart.Duration = 5.0f;
    TutorialStart.bIsRepeatable = false;
    RegisterDialogue(TutorialStart);

    // Warning dialogues
    FNarr_DialogueEntry DangerWarning;
    DangerWarning.DialogueID = "DANGER_TREX";
    DangerWarning.DialogueText = FText::FromString("Massive predator detected. T-Rex territory ahead. Proceed with extreme caution.");
    DangerWarning.DialogueType = ENarr_DialogueType::Warning;
    DangerWarning.SpeakerType = ENarr_NPCType::Scout;
    DangerWarning.Duration = 4.0f;
    DangerWarning.bIsRepeatable = true;
    RegisterDialogue(DangerWarning);

    // Discovery dialogues
    FNarr_DialogueEntry ForestDiscovery;
    ForestDiscovery.DialogueID = "DISCOVERY_FOREST";
    ForestDiscovery.DialogueText = FText::FromString("Ancient forest discovered. Rich resources await, but dangerous creatures lurk in the shadows.");
    ForestDiscovery.DialogueType = ENarr_DialogueType::Discovery;
    ForestDiscovery.SpeakerType = ENarr_NPCType::Scout;
    ForestDiscovery.Duration = 4.5f;
    ForestDiscovery.bIsRepeatable = false;
    RegisterDialogue(ForestDiscovery);

    // NPC dialogues
    FNarr_DialogueEntry ElderWisdom;
    ElderWisdom.DialogueID = "ELDER_WISDOM";
    ElderWisdom.DialogueText = FText::FromString("The old ways teach us: respect the land, fear the apex predators, and never hunt alone.");
    ElderWisdom.DialogueType = ENarr_DialogueType::Story;
    ElderWisdom.SpeakerType = ENarr_NPCType::TribalElder;
    ElderWisdom.Duration = 6.0f;
    ElderWisdom.bIsRepeatable = true;
    RegisterDialogue(ElderWisdom);

    // Combat dialogues
    FNarr_DialogueEntry CombatAdvice;
    CombatAdvice.DialogueID = "COMBAT_ADVICE";
    CombatAdvice.DialogueText = FText::FromString("Raptors hunt in packs. Use terrain to your advantage. Strike fast, retreat faster.");
    CombatAdvice.DialogueType = ENarr_DialogueType::Combat;
    CombatAdvice.SpeakerType = ENarr_NPCType::Hunter;
    CombatAdvice.Duration = 5.0f;
    CombatAdvice.bIsRepeatable = true;
    RegisterDialogue(CombatAdvice);

    UE_LOG(LogTemp, Warning, TEXT("Initialized %d default dialogues"), DialogueDatabase.Num());
}

void UNarr_DialogueSystem::InitializeStoryProgression()
{
    // Main survival story
    FNarr_StoryProgress SurvivalStory;
    SurvivalStory.StoryID = "MAIN_SURVIVAL";
    SurvivalStory.StoryTitle = FText::FromString("Prehistoric Survival");
    SurvivalStory.StoryDescription = FText::FromString("Survive in the dangerous prehistoric world and establish your place in the ecosystem.");
    SurvivalStory.TotalSteps = 10;
    SurvivalStory.UnlockedDialogues.Add("ELDER_WISDOM");
    SurvivalStory.UnlockedDialogues.Add("COMBAT_ADVICE");
    StoryProgressMap.Add(SurvivalStory.StoryID, SurvivalStory);

    // Tribal contact story
    FNarr_StoryProgress TribalStory;
    TribalStory.StoryID = "TRIBAL_CONTACT";
    TribalStory.StoryTitle = FText::FromString("First Contact");
    TribalStory.StoryDescription = FText::FromString("Establish contact with other survivors and learn their ways.");
    TribalStory.TotalSteps = 5;
    TribalStory.UnlockedDialogues.Add("ELDER_WISDOM");
    StoryProgressMap.Add(TribalStory.StoryID, TribalStory);

    // Territory exploration story
    FNarr_StoryProgress ExplorationStory;
    ExplorationStory.StoryID = "TERRITORY_EXPLORATION";
    ExplorationStory.StoryTitle = FText::FromString("Uncharted Lands");
    ExplorationStory.StoryDescription = FText::FromString("Explore the five great biomes and discover their secrets.");
    ExplorationStory.TotalSteps = 5;
    ExplorationStory.UnlockedDialogues.Add("DISCOVERY_FOREST");
    StoryProgressMap.Add(ExplorationStory.StoryID, ExplorationStory);

    UE_LOG(LogTemp, Warning, TEXT("Initialized %d story progressions"), StoryProgressMap.Num());
}

bool UNarr_DialogueSystem::CheckDialoguePrerequisites(const FNarr_DialogueEntry& DialogueEntry) const
{
    // Check if all prerequisites are met
    for (const FString& Prerequisite : DialogueEntry.Prerequisites)
    {
        // In a full implementation, this would check various game state conditions
        // For now, we'll assume all prerequisites are met
        UE_LOG(LogTemp, Log, TEXT("Checking prerequisite: %s"), *Prerequisite);
    }
    
    return true;
}

void UNarr_DialogueSystem::OnDialogueCompleted(const FString& DialogueID)
{
    if (CurrentPlayingDialogue == DialogueID)
    {
        CurrentPlayingDialogue = "";
        DialogueStartTime = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Dialogue completed: %s"), *DialogueID);
        
        // Trigger any follow-up events based on dialogue completion
        if (DialogueID == "TUTORIAL_START")
        {
            AdvanceStory("MAIN_SURVIVAL");
        }
        else if (DialogueID == "ELDER_WISDOM")
        {
            AdvanceStory("TRIBAL_CONTACT");
        }
        else if (DialogueID == "DISCOVERY_FOREST")
        {
            AdvanceStory("TERRITORY_EXPLORATION");
        }
    }
}