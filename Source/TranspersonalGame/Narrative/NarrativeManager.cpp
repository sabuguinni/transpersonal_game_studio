#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Initializing narrative system"));
    
    InitializeStoryBeats();
    LoadDialogueData();
    
    CurrentNarrativeContext = TEXT("Valley_Intro");
}

void UNarrativeManager::Deinitialize()
{
    Super::Deinitialize();
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Shutting down narrative system"));
}

void UNarrativeManager::TriggerDialogue(const FString& DialogueID, AActor* Speaker)
{
    FNarr_DialogueEntry* DialogueEntry = FindDialogueEntry(DialogueID);
    if (!DialogueEntry)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Dialogue entry not found: %s"), *DialogueID);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Triggering dialogue: %s - %s"), 
           *DialogueEntry->SpeakerName, *DialogueEntry->DialogueText.ToString());

    // Play audio if available
    if (!DialogueEntry->AudioPath.IsEmpty())
    {
        USoundCue* DialogueSound = LoadObject<USoundCue>(nullptr, *DialogueEntry->AudioPath);
        if (DialogueSound && Speaker)
        {
            UGameplayStatics::PlaySoundAtLocation(this, DialogueSound, Speaker->GetActorLocation());
        }
    }

    // TODO: Integrate with UI system to display dialogue text
    // For now, log to console
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("%s: %s"), 
                                            *DialogueEntry->SpeakerName, 
                                            *DialogueEntry->DialogueText.ToString());
        GEngine->AddOnScreenDebugMessage(-1, DialogueEntry->Duration, FColor::Yellow, DisplayText);
    }
}

void UNarrativeManager::CompleteStoryBeat(const FString& BeatID)
{
    for (FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (Beat.BeatID == BeatID && !Beat.bIsCompleted)
        {
            Beat.bIsCompleted = true;
            CompletedStoryBeats.AddUnique(BeatID);
            
            UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Story beat completed: %s"), *BeatID);
            
            // Unlock dependent story beats
            for (const FString& UnlockedBeat : Beat.UnlockedBeats)
            {
                UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Unlocked story beat: %s"), *UnlockedBeat);
            }
            
            break;
        }
    }
}

bool UNarrativeManager::IsStoryBeatCompleted(const FString& BeatID) const
{
    return CompletedStoryBeats.Contains(BeatID);
}

TArray<FNarr_StoryBeat> UNarrativeManager::GetAvailableStoryBeats() const
{
    TArray<FNarr_StoryBeat> AvailableBeats;
    
    for (const FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (Beat.bIsCompleted)
        {
            continue;
        }
        
        // Check prerequisites
        bool bAllPrerequisitesMet = true;
        for (const FString& Prerequisite : Beat.Prerequisites)
        {
            if (!IsStoryBeatCompleted(Prerequisite))
            {
                bAllPrerequisitesMet = false;
                break;
            }
        }
        
        if (bAllPrerequisitesMet)
        {
            AvailableBeats.Add(Beat);
        }
    }
    
    return AvailableBeats;
}

void UNarrativeManager::PlayNarration(const FString& NarrationID, const FVector& Location)
{
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Playing narration: %s at location: %s"), 
           *NarrationID, *Location.ToString());
    
    // Find narration audio file
    FString AudioPath = FString::Printf(TEXT("/Game/Audio/Narration/%s"), *NarrationID);
    USoundCue* NarrationSound = LoadObject<USoundCue>(nullptr, *AudioPath);
    
    if (NarrationSound)
    {
        if (Location != FVector::ZeroVector)
        {
            UGameplayStatics::PlaySoundAtLocation(this, NarrationSound, Location);
        }
        else
        {
            UGameplayStatics::PlaySound2D(this, NarrationSound);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Narration audio not found: %s"), *AudioPath);
    }
}

void UNarrativeManager::SetNarrativeContext(const FString& Context)
{
    CurrentNarrativeContext = Context;
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Narrative context changed to: %s"), *Context);
}

FString UNarrativeManager::GetCurrentNarrativeContext() const
{
    return CurrentNarrativeContext;
}

void UNarrativeManager::InitializeStoryBeats()
{
    // Initialize core story beats for prehistoric survival
    FNarr_StoryBeat IntroductionBeat;
    IntroductionBeat.BeatID = TEXT("Valley_Awakening");
    IntroductionBeat.Title = FText::FromString(TEXT("Awakening in the Valley"));
    IntroductionBeat.Description = FText::FromString(TEXT("You awaken in a primordial valley, surrounded by the sounds of ancient creatures."));
    IntroductionBeat.bIsCompleted = false;
    IntroductionBeat.UnlockedBeats.Add(TEXT("First_Encounter"));
    StoryBeats.Add(IntroductionBeat);

    FNarr_StoryBeat FirstEncounterBeat;
    FirstEncounterBeat.BeatID = TEXT("First_Encounter");
    FirstEncounterBeat.Title = FText::FromString(TEXT("First Dinosaur Encounter"));
    FirstEncounterBeat.Description = FText::FromString(TEXT("Your first glimpse of the magnificent and terrifying creatures that rule this world."));
    FirstEncounterBeat.bIsCompleted = false;
    FirstEncounterBeat.Prerequisites.Add(TEXT("Valley_Awakening"));
    FirstEncounterBeat.UnlockedBeats.Add(TEXT("Survival_Basics"));
    StoryBeats.Add(FirstEncounterBeat);

    FNarr_StoryBeat SurvivalBasicsBeat;
    SurvivalBasicsBeat.BeatID = TEXT("Survival_Basics");
    SurvivalBasicsBeat.Title = FText::FromString(TEXT("Learning to Survive"));
    SurvivalBasicsBeat.Description = FText::FromString(TEXT("Master the basics of survival: finding water, shelter, and avoiding predators."));
    SurvivalBasicsBeat.bIsCompleted = false;
    SurvivalBasicsBeat.Prerequisites.Add(TEXT("First_Encounter"));
    SurvivalBasicsBeat.UnlockedBeats.Add(TEXT("Territory_Exploration"));
    StoryBeats.Add(SurvivalBasicsBeat);

    FNarr_StoryBeat TerritoryExplorationBeat;
    TerritoryExplorationBeat.BeatID = TEXT("Territory_Exploration");
    TerritoryExplorationBeat.Title = FText::FromString(TEXT("Exploring the Territory"));
    TerritoryExplorationBeat.Description = FText::FromString(TEXT("Venture beyond the safety of your initial shelter to explore the vast prehistoric landscape."));
    TerritoryExplorationBeat.bIsCompleted = false;
    TerritoryExplorationBeat.Prerequisites.Add(TEXT("Survival_Basics"));
    TerritoryExplorationBeat.UnlockedBeats.Add(TEXT("Predator_Encounter"));
    StoryBeats.Add(TerritoryExplorationBeat);

    FNarr_StoryBeat PredatorEncounterBeat;
    PredatorEncounterBeat.BeatID = TEXT("Predator_Encounter");
    PredatorEncounterBeat.Title = FText::FromString(TEXT("Face of the Apex Predator"));
    PredatorEncounterBeat.Description = FText::FromString(TEXT("A terrifying encounter with a Tyrannosaurus Rex tests your survival instincts."));
    PredatorEncounterBeat.bIsCompleted = false;
    PredatorEncounterBeat.Prerequisites.Add(TEXT("Territory_Exploration"));
    PredatorEncounterBeat.UnlockedBeats.Add(TEXT("Tribal_Discovery"));
    StoryBeats.Add(PredatorEncounterBeat);

    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initialized %d story beats"), StoryBeats.Num());
}

void UNarrativeManager::LoadDialogueData()
{
    // TODO: Load from data table when available
    // For now, create some basic dialogue entries programmatically
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Dialogue data loading complete"));
}

FNarr_DialogueEntry* UNarrativeManager::FindDialogueEntry(const FString& DialogueID)
{
    if (!DialogueDataTable)
    {
        return nullptr;
    }
    
    return DialogueDataTable->FindRow<FNarr_DialogueEntry>(FName(*DialogueID), TEXT(""));
}