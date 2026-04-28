#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ANarrativeManager::ANarrativeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    CurrentStoryChapter = 1;
    TotalStoryChapters = 12;
    CurrentDialogueState = ENarr_DialogueState::Inactive;
    DialogueInteractionRange = 500.0f;
    DialogueTimer = 0.0f;
    CurrentDialogueIndex = 0;
    
    // Set up root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void ANarrativeManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeStoryChapters();
    InitializeDialogues();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager initialized with %d story chapters"), TotalStoryChapters);
}

void ANarrativeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateDialogueState(DeltaTime);
}

void ANarrativeManager::InitializeStoryChapters()
{
    StoryChapters.Empty();
    
    // Chapter 1: First Contact
    FNarr_StoryChapter Chapter1;
    Chapter1.ChapterName = TEXT("The Awakening");
    Chapter1.ChapterDescription = FText::FromString(TEXT("You awaken in a world ruled by giants. Learn to survive among the apex predators."));
    Chapter1.RequiredObjectives.Add(TEXT("FindShelter"));
    Chapter1.RequiredObjectives.Add(TEXT("CraftBasicTools"));
    Chapter1.RequiredObjectives.Add(TEXT("AvoidPredators"));
    StoryChapters.Add(Chapter1);
    
    // Chapter 2: The Hunt Begins
    FNarr_StoryChapter Chapter2;
    Chapter2.ChapterName = TEXT("Predator Territory");
    Chapter2.ChapterDescription = FText::FromString(TEXT("The pack hunters have noticed you. Learn their patterns or become their prey."));
    Chapter2.RequiredObjectives.Add(TEXT("StudyRaptorBehavior"));
    Chapter2.RequiredObjectives.Add(TEXT("FindTribalCamp"));
    Chapter2.RequiredObjectives.Add(TEXT("SurviveFirstHunt"));
    StoryChapters.Add(Chapter2);
    
    // Chapter 3: Ancient Wisdom
    FNarr_StoryChapter Chapter3;
    Chapter3.ChapterName = TEXT("The Elder's Warning");
    Chapter3.ChapterDescription = FText::FromString(TEXT("An ancient storyteller shares knowledge passed down through generations."));
    Chapter3.RequiredObjectives.Add(TEXT("MeetTribalElder"));
    Chapter3.RequiredObjectives.Add(TEXT("LearnAncientTactics"));
    Chapter3.RequiredObjectives.Add(TEXT("UnderstandTerritories"));
    StoryChapters.Add(Chapter3);
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d story chapters"), StoryChapters.Num());
}

void ANarrativeManager::InitializeDialogues()
{
    // Tribal Elder Dialogues
    TribalElderDialogues.Empty();
    
    FNarr_DialogueEntry ElderIntro;
    ElderIntro.SpeakerName = TEXT("Ancient Storyteller");
    ElderIntro.DialogueText = FText::FromString(TEXT("Long ago, when the earth trembled beneath the feet of giants, our ancestors learned the first law of survival - respect the territory of the great hunters."));
    ElderIntro.VoiceAssetPath = TEXT("/Game/Audio/Dialogue/Elder_Intro");
    ElderIntro.DisplayDuration = 8.0f;
    TribalElderDialogues.Add(ElderIntro);
    
    FNarr_DialogueEntry ElderWarning;
    ElderWarning.SpeakerName = TEXT("Ancient Storyteller");
    ElderWarning.DialogueText = FText::FromString(TEXT("The pack hunters move in patterns older than memory. Watch the shadows, young one. They are always watching you."));
    ElderWarning.VoiceAssetPath = TEXT("/Game/Audio/Dialogue/Elder_Warning");
    ElderWarning.DisplayDuration = 6.0f;
    TribalElderDialogues.Add(ElderWarning);
    
    // Hunter Dialogues
    HunterDialogues.Empty();
    
    FNarr_DialogueEntry HunterPanic;
    HunterPanic.SpeakerName = TEXT("Desperate Hunter");
    HunterPanic.DialogueText = FText::FromString(TEXT("The pack... they've learned to use the canyon walls. Three of our best trackers are trapped in the eastern gorge. We need a plan!"));
    HunterPanic.VoiceAssetPath = TEXT("/Game/Audio/Dialogue/Hunter_Panic");
    HunterPanic.DisplayDuration = 7.0f;
    HunterDialogues.Add(HunterPanic);
    
    // Scout Dialogues
    ScoutDialogues.Empty();
    
    FNarr_DialogueEntry ScoutReport;
    ScoutReport.SpeakerName = TEXT("Young Scout");
    ScoutReport.DialogueText = FText::FromString(TEXT("I've seen something unnatural. The great predators are moving in patterns, coordinating their hunts across vast distances."));
    ScoutReport.VoiceAssetPath = TEXT("/Game/Audio/Dialogue/Scout_Report");
    ScoutReport.DisplayDuration = 6.0f;
    ScoutDialogues.Add(ScoutReport);
    
    UE_LOG(LogTemp, Log, TEXT("Initialized dialogue sets: Elder(%d), Hunter(%d), Scout(%d)"), 
           TribalElderDialogues.Num(), HunterDialogues.Num(), ScoutDialogues.Num());
}

void ANarrativeManager::StartDialogue(const FString& NPCName, const FString& DialogueID)
{
    if (CurrentDialogueState != ENarr_DialogueState::Inactive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start dialogue - already in dialogue state"));
        return;
    }
    
    TArray<FNarr_DialogueEntry>* DialogueSet = nullptr;
    
    if (NPCName.Contains(TEXT("Elder")) || NPCName.Contains(TEXT("Storyteller")))
    {
        DialogueSet = &TribalElderDialogues;
    }
    else if (NPCName.Contains(TEXT("Hunter")))
    {
        DialogueSet = &HunterDialogues;
    }
    else if (NPCName.Contains(TEXT("Scout")))
    {
        DialogueSet = &ScoutDialogues;
    }
    
    if (DialogueSet && DialogueSet->Num() > 0)
    {
        CurrentDialogueIndex = 0;
        ActiveDialogue = (*DialogueSet)[CurrentDialogueIndex];
        CurrentDialogueState = ENarr_DialogueState::DisplayingDialogue;
        DialogueTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Started dialogue with %s: %s"), *NPCName, *ActiveDialogue.DialogueText.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No dialogue found for NPC: %s"), *NPCName);
    }
}

void ANarrativeManager::EndDialogue()
{
    CurrentDialogueState = ENarr_DialogueState::Inactive;
    ActiveDialogue = FNarr_DialogueEntry();
    DialogueTimer = 0.0f;
    CurrentDialogueIndex = 0;
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue ended"));
}

void ANarrativeManager::UpdateDialogueState(float DeltaTime)
{
    if (CurrentDialogueState == ENarr_DialogueState::DisplayingDialogue)
    {
        DialogueTimer += DeltaTime;
        
        if (DialogueTimer >= ActiveDialogue.DisplayDuration)
        {
            if (ActiveDialogue.ResponseOptions.Num() > 0)
            {
                CurrentDialogueState = ENarr_DialogueState::WaitingForResponse;
            }
            else
            {
                EndDialogue();
            }
        }
    }
}

void ANarrativeManager::AdvanceStoryChapter()
{
    if (CurrentStoryChapter < TotalStoryChapters)
    {
        // Check if current chapter objectives are completed
        bool bAllObjectivesCompleted = true;
        if (StoryChapters.IsValidIndex(CurrentStoryChapter - 1))
        {
            const FNarr_StoryChapter& CurrentChapter = StoryChapters[CurrentStoryChapter - 1];
            for (const FString& Objective : CurrentChapter.RequiredObjectives)
            {
                if (!IsObjectiveCompleted(Objective))
                {
                    bAllObjectivesCompleted = false;
                    break;
                }
            }
        }
        
        if (bAllObjectivesCompleted)
        {
            CurrentStoryChapter++;
            UE_LOG(LogTemp, Log, TEXT("Advanced to story chapter %d"), CurrentStoryChapter);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Cannot advance chapter - objectives not completed"));
        }
    }
}

bool ANarrativeManager::IsObjectiveCompleted(const FString& ObjectiveID) const
{
    return CompletedObjectives.Contains(ObjectiveID);
}

void ANarrativeManager::CompleteObjective(const FString& ObjectiveID)
{
    if (!CompletedObjectives.Contains(ObjectiveID))
    {
        CompletedObjectives.Add(ObjectiveID);
        UE_LOG(LogTemp, Log, TEXT("Completed objective: %s"), *ObjectiveID);
        
        // Check if this completes the current chapter
        if (StoryChapters.IsValidIndex(CurrentStoryChapter - 1))
        {
            FNarr_StoryChapter& CurrentChapter = StoryChapters[CurrentStoryChapter - 1];
            bool bAllCompleted = true;
            for (const FString& RequiredObjective : CurrentChapter.RequiredObjectives)
            {
                if (!IsObjectiveCompleted(RequiredObjective))
                {
                    bAllCompleted = false;
                    break;
                }
            }
            
            if (bAllCompleted && !CurrentChapter.bIsCompleted)
            {
                CurrentChapter.bIsCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("Chapter %d completed!"), CurrentStoryChapter);
            }
        }
    }
}

void ANarrativeManager::SetNarrativeFlag(const FString& FlagName, bool bValue)
{
    if (bValue)
    {
        if (!ActiveNarrativeFlags.Contains(FlagName))
        {
            ActiveNarrativeFlags.Add(FlagName);
        }
    }
    else
    {
        ActiveNarrativeFlags.Remove(FlagName);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Set narrative flag %s to %s"), *FlagName, bValue ? TEXT("true") : TEXT("false"));
}

bool ANarrativeManager::GetNarrativeFlag(const FString& FlagName) const
{
    return ActiveNarrativeFlags.Contains(FlagName);
}

FText ANarrativeManager::GetCurrentChapterDescription() const
{
    if (StoryChapters.IsValidIndex(CurrentStoryChapter - 1))
    {
        return StoryChapters[CurrentStoryChapter - 1].ChapterDescription;
    }
    
    return FText::FromString(TEXT("No chapter description available"));
}

TArray<FString> ANarrativeManager::GetAvailableDialogueOptions() const
{
    if (CurrentDialogueState == ENarr_DialogueState::WaitingForResponse)
    {
        return ActiveDialogue.ResponseOptions;
    }
    
    return TArray<FString>();
}

void ANarrativeManager::SelectDialogueResponse(int32 ResponseIndex)
{
    if (CurrentDialogueState == ENarr_DialogueState::WaitingForResponse && 
        ActiveDialogue.ResponseOptions.IsValidIndex(ResponseIndex))
    {
        const FString& SelectedResponse = ActiveDialogue.ResponseOptions[ResponseIndex];
        UE_LOG(LogTemp, Log, TEXT("Player selected response: %s"), *SelectedResponse);
        
        // Process response and potentially trigger new dialogue or end conversation
        EndDialogue();
    }
}