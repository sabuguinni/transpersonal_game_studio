#include "NarrativeDialogueManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

UNarrativeDialogueManager::UNarrativeDialogueManager()
{
    NarrationVolume = 0.8f;
    bIsCurrentlyPlaying = false;
    CurrentDialogueID = TEXT("");
}

void UNarrativeDialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueManager: Initializing dialogue system"));
    
    // Create audio component for dialogue playback
    if (UWorld* World = GetWorld())
    {
        if (AActor* DummyActor = World->SpawnActor<AActor>())
        {
            DialogueAudioComponent = DummyActor->CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueAudio"));
            if (DialogueAudioComponent)
            {
                DialogueAudioComponent->SetVolumeMultiplier(NarrationVolume);
                DialogueAudioComponent->bAutoActivate = false;
                DialogueAudioComponent->OnAudioFinished.AddDynamic(this, &UNarrativeDialogueManager::HandleAudioFinished);
            }
        }
    }
    
    InitializeDialogueDatabase();
    LoadEnvironmentalNarration();
}

void UNarrativeDialogueManager::Deinitialize()
{
    StopCurrentDialogue();
    
    if (DialogueAudioComponent && IsValid(DialogueAudioComponent))
    {
        DialogueAudioComponent->Stop();
        DialogueAudioComponent = nullptr;
    }
    
    DialogueDatabase.Empty();
    QuestDialogues.Empty();
    
    Super::Deinitialize();
}

void UNarrativeDialogueManager::PlayDialogue(const FString& DialogueID)
{
    if (bIsCurrentlyPlaying)
    {
        StopCurrentDialogue();
    }
    
    if (FNarr_DialogueEntry* DialogueEntry = DialogueDatabase.Find(DialogueID))
    {
        CurrentDialogueID = DialogueID;
        bIsCurrentlyPlaying = true;
        
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueManager: Playing dialogue - %s: %s"), 
               *DialogueEntry->SpeakerName, *DialogueEntry->DialogueText.ToString());
        
        // Play audio if available
        if (DialogueAudioComponent && DialogueEntry->VoiceAudio.IsValid())
        {
            DialogueAudioComponent->SetSound(DialogueEntry->VoiceAudio.LoadSynchronous());
            DialogueAudioComponent->Play();
        }
        else
        {
            // Use timer for text-only dialogue
            if (UWorld* World = GetWorld())
            {
                World->GetTimerManager().SetTimer(DialogueTimerHandle, 
                    this, &UNarrativeDialogueManager::OnDialogueFinished, 
                    DialogueEntry->Duration, false);
            }
        }
        
        // Broadcast dialogue started (could add delegate here)
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueManager: Dialogue ID not found: %s"), *DialogueID);
    }
}

void UNarrativeDialogueManager::PlayQuestDialogue(const FString& QuestID, int32 EntryIndex)
{
    if (FNarr_QuestDialogue* QuestDialogue = QuestDialogues.Find(QuestID))
    {
        if (QuestDialogue->DialogueEntries.IsValidIndex(EntryIndex))
        {
            const FNarr_DialogueEntry& Entry = QuestDialogue->DialogueEntries[EntryIndex];
            
            // Create temporary dialogue ID for quest entry
            FString TempDialogueID = FString::Printf(TEXT("%s_%d"), *QuestID, EntryIndex);
            DialogueDatabase.Add(TempDialogueID, Entry);
            
            PlayDialogue(TempDialogueID);
        }
    }
}

void UNarrativeDialogueManager::StopCurrentDialogue()
{
    if (bIsCurrentlyPlaying)
    {
        bIsCurrentlyPlaying = false;
        CurrentDialogueID = TEXT("");
        
        if (DialogueAudioComponent && DialogueAudioComponent->IsPlaying())
        {
            DialogueAudioComponent->Stop();
        }
        
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(DialogueTimerHandle);
        }
    }
}

bool UNarrativeDialogueManager::IsDialoguePlaying() const
{
    return bIsCurrentlyPlaying;
}

void UNarrativeDialogueManager::RegisterQuestDialogue(const FString& QuestID, const TArray<FNarr_DialogueEntry>& DialogueEntries)
{
    FNarr_QuestDialogue NewQuestDialogue;
    NewQuestDialogue.QuestID = QuestID;
    NewQuestDialogue.DialogueEntries = DialogueEntries;
    NewQuestDialogue.bIsActive = true;
    
    QuestDialogues.Add(QuestID, NewQuestDialogue);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueManager: Registered quest dialogue for %s with %d entries"), 
           *QuestID, DialogueEntries.Num());
}

void UNarrativeDialogueManager::TriggerEnvironmentalNarration(ENarr_BiomeType BiomeType, ENarr_ThreatLevel ThreatLevel)
{
    FString NarrationKey = FString::Printf(TEXT("ENV_%s_%s"), 
        *UEnum::GetValueAsString(BiomeType),
        *UEnum::GetValueAsString(ThreatLevel));
    
    PlayDialogue(NarrationKey);
}

void UNarrativeDialogueManager::TriggerSurvivalWarning(ENarr_SurvivalWarningType WarningType)
{
    FString WarningKey = FString::Printf(TEXT("WARNING_%s"), 
        *UEnum::GetValueAsString(WarningType));
    
    PlayDialogue(WarningKey);
}

void UNarrativeDialogueManager::SetNarrationVolume(float Volume)
{
    NarrationVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (DialogueAudioComponent)
    {
        DialogueAudioComponent->SetVolumeMultiplier(NarrationVolume);
    }
}

float UNarrativeDialogueManager::GetNarrationVolume() const
{
    return NarrationVolume;
}

void UNarrativeDialogueManager::InitializeDialogueDatabase()
{
    // Initialize core survival narration
    FNarr_DialogueEntry IntroNarration;
    IntroNarration.DialogueID = TEXT("INTRO_VALLEY");
    IntroNarration.SpeakerName = TEXT("Narrator");
    IntroNarration.DialogueText = FText::FromString(TEXT("The ancient valley holds many secrets, survivor. Listen carefully to the wind - it carries the scent of predators on the hunt."));
    IntroNarration.Duration = 6.0f;
    IntroNarration.DialogueType = ENarr_DialogueType::Narration;
    DialogueDatabase.Add(IntroNarration.DialogueID, IntroNarration);
    
    // Pack hunter warning
    FNarr_DialogueEntry PackWarning;
    PackWarning.DialogueID = TEXT("WARNING_PACK_HUNTERS");
    PackWarning.SpeakerName = TEXT("Survival Guide");
    PackWarning.DialogueText = FText::FromString(TEXT("Warning! Pack hunters detected in the eastern ravines. Their coordinated movements suggest they are tracking prey."));
    PackWarning.Duration = 5.0f;
    PackWarning.DialogueType = ENarr_DialogueType::Warning;
    DialogueDatabase.Add(PackWarning.DialogueID, PackWarning);
    
    // Environmental discoveries
    FNarr_DialogueEntry CaveDiscovery;
    CaveDiscovery.DialogueID = TEXT("DISCOVERY_CAVE");
    CaveDiscovery.SpeakerName = TEXT("Explorer");
    CaveDiscovery.DialogueText = FText::FromString(TEXT("These cave paintings... they show hunting patterns. The ancients knew these creatures well."));
    CaveDiscovery.Duration = 4.5f;
    CaveDiscovery.DialogueType = ENarr_DialogueType::Discovery;
    DialogueDatabase.Add(CaveDiscovery.DialogueID, CaveDiscovery);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueManager: Initialized %d dialogue entries"), DialogueDatabase.Num());
}

void UNarrativeDialogueManager::LoadEnvironmentalNarration()
{
    // Biome-specific narration
    TArray<FString> BiomeNarrations = {
        TEXT("ENV_SAVANA_LOW|The grasslands stretch endlessly. Perfect hunting grounds for both predator and prey."),
        TEXT("ENV_FOREST_MEDIUM|Ancient trees whisper warnings. Something large moves through these shadows."),
        TEXT("ENV_DESERT_HIGH|The sun beats down mercilessly. Water is life here, and death follows those who forget."),
        TEXT("ENV_SWAMP_HIGH|The fetid waters hide countless dangers. Every step could be your last."),
        TEXT("ENV_MOUNTAIN_MEDIUM|The peaks offer safety from ground predators, but the cold is equally deadly.")
    };
    
    for (const FString& NarrationData : BiomeNarrations)
    {
        FString DialogueID, DialogueText;
        if (NarrationData.Split(TEXT("|"), &DialogueID, &DialogueText))
        {
            FNarr_DialogueEntry Entry;
            Entry.DialogueID = DialogueID;
            Entry.SpeakerName = TEXT("Environment Guide");
            Entry.DialogueText = FText::FromString(DialogueText);
            Entry.Duration = 4.0f;
            Entry.DialogueType = ENarr_DialogueType::Environmental;
            DialogueDatabase.Add(Entry.DialogueID, Entry);
        }
    }
}

void UNarrativeDialogueManager::OnDialogueFinished()
{
    bIsCurrentlyPlaying = false;
    CurrentDialogueID = TEXT("");
    
    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }
}

void UNarrativeDialogueManager::HandleAudioFinished()
{
    OnDialogueFinished();
}