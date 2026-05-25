#include "NarrativeEventManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

void UNarrativeEventManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeEventManager: Initializing survival narrative system"));
    
    // Initialize story progress
    CurrentStoryProgress = FNarr_StoryProgress();
    
    // Create audio component for narrative playback
    if (UWorld* World = GetWorld())
    {
        NarrativeAudioComponent = NewObject<UAudioComponent>(this);
        if (NarrativeAudioComponent)
        {
            NarrativeAudioComponent->bAutoActivate = false;
            NarrativeAudioComponent->SetVolumeMultiplier(0.8f);
        }
    }
    
    // Initialize default narrative events
    InitializeDefaultNarrativeEvents();
}

void UNarrativeEventManager::Deinitialize()
{
    if (NarrativeAudioComponent)
    {
        NarrativeAudioComponent->Stop();
        NarrativeAudioComponent = nullptr;
    }
    
    NarrativeEvents.Empty();
    
    Super::Deinitialize();
}

void UNarrativeEventManager::RegisterNarrativeEvent(const FNarr_NarrativeEvent& Event)
{
    // Check if event already exists
    for (const FNarr_NarrativeEvent& ExistingEvent : NarrativeEvents)
    {
        if (ExistingEvent.EventID == Event.EventID)
        {
            UE_LOG(LogTemp, Warning, TEXT("NarrativeEventManager: Event %s already registered"), *Event.EventID);
            return;
        }
    }
    
    NarrativeEvents.Add(Event);
    UE_LOG(LogTemp, Log, TEXT("NarrativeEventManager: Registered event %s"), *Event.EventID);
}

void UNarrativeEventManager::TriggerNarrativeEvent(const FString& EventID, AActor* TriggeringActor)
{
    for (FNarr_NarrativeEvent& Event : NarrativeEvents)
    {
        if (Event.EventID == EventID)
        {
            if (Event.bIsOneTime && Event.bHasBeenTriggered)
            {
                return; // Event already triggered
            }
            
            // Mark as triggered
            Event.bHasBeenTriggered = true;
            
            // Play narrative audio
            PlayNarrativeAudio(Event);
            
            // Update story progress
            CurrentStoryProgress.CompletedEvents.AddUnique(EventID);
            
            UE_LOG(LogTemp, Warning, TEXT("NarrativeEventManager: Triggered event %s - %s"), *Event.EventID, *Event.DialogueText);
            break;
        }
    }
}

bool UNarrativeEventManager::CheckEventTrigger(const FVector& PlayerLocation)
{
    bool bEventTriggered = false;
    
    for (FNarr_NarrativeEvent& Event : NarrativeEvents)
    {
        if (Event.bIsOneTime && Event.bHasBeenTriggered)
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, Event.TriggerLocation);
        if (Distance <= Event.TriggerRadius)
        {
            TriggerNarrativeEvent(Event.EventID, nullptr);
            bEventTriggered = true;
        }
    }
    
    return bEventTriggered;
}

void UNarrativeEventManager::PlayNarrativeAudio(const FNarr_NarrativeEvent& Event)
{
    if (NarrativeAudioComponent && Event.VoiceAudio)
    {
        NarrativeAudioComponent->SetSound(Event.VoiceAudio);
        NarrativeAudioComponent->Play();
        
        UE_LOG(LogTemp, Log, TEXT("NarrativeEventManager: Playing audio for %s"), *Event.CharacterName);
    }
    else
    {
        // Fallback to text display
        if (UWorld* World = GetWorld())
        {
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                FString DisplayText = FString::Printf(TEXT("%s: %s"), *Event.CharacterName, *Event.DialogueText);
                UKismetSystemLibrary::PrintString(World, DisplayText, true, true, FLinearColor::Yellow, 5.0f);
            }
        }
    }
}

void UNarrativeEventManager::UpdateStoryProgress(const FString& EventType, int32 Value)
{
    if (EventType == TEXT("DinosaurEncounter"))
    {
        CurrentStoryProgress.DinosaurEncounters += Value;
    }
    else if (EventType == TEXT("SuccessfulHunt"))
    {
        CurrentStoryProgress.SuccessfulHunts += Value;
    }
    else if (EventType == TEXT("SurvivalDay"))
    {
        CurrentStoryProgress.SurvivalDays += Value;
    }
    
    // Check for chapter progression
    if (CurrentStoryProgress.SurvivalDays >= 7.0f && CurrentStoryProgress.CurrentChapter == 1)
    {
        CurrentStoryProgress.CurrentChapter = 2;
        
        // Trigger chapter 2 narrative event
        FNarr_NarrativeEvent ChapterEvent;
        ChapterEvent.EventID = TEXT("Chapter2_WeekSurvivor");
        ChapterEvent.DialogueText = TEXT("You have survived a full week in this prehistoric world. The valley begins to feel less hostile, but greater challenges await.");
        ChapterEvent.CharacterName = TEXT("Ancient Narrator");
        ChapterEvent.TriggerLocation = FVector::ZeroVector;
        ChapterEvent.bIsOneTime = true;
        
        TriggerNarrativeEvent(ChapterEvent.EventID, nullptr);
    }
}

void UNarrativeEventManager::InitializeDefaultNarrativeEvents()
{
    CreateSurvivalNarrativeEvents();
    CreatePredatorWarningEvents();
    CreateDiscoveryEvents();
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeEventManager: Initialized %d default narrative events"), NarrativeEvents.Num());
}

void UNarrativeEventManager::CreateSurvivalNarrativeEvents()
{
    // Valley entrance event
    FNarr_NarrativeEvent ValleyEvent;
    ValleyEvent.EventID = TEXT("ValleyEntrance_FirstTime");
    ValleyEvent.DialogueText = TEXT("The ancient valley stretches before you, filled with both wonder and terror. Every shadow could hide death, every sound could be your last warning.");
    ValleyEvent.CharacterName = TEXT("Ancient Narrator");
    ValleyEvent.TriggerLocation = FVector(-10000, 5000, 100);
    ValleyEvent.TriggerRadius = 2000.0f;
    ValleyEvent.bIsOneTime = true;
    RegisterNarrativeEvent(ValleyEvent);
    
    // First night survival
    FNarr_NarrativeEvent NightEvent;
    NightEvent.EventID = TEXT("FirstNight_Survival");
    NightEvent.DialogueText = TEXT("As darkness falls, the valley transforms. The hunters emerge, and you must find shelter or become prey.");
    NightEvent.CharacterName = TEXT("Survival Instinct");
    NightEvent.TriggerLocation = FVector::ZeroVector;
    NightEvent.TriggerRadius = 5000.0f;
    NightEvent.bIsOneTime = true;
    RegisterNarrativeEvent(NightEvent);
}

void UNarrativeEventManager::CreatePredatorWarningEvents()
{
    // T-Rex territory warning
    FNarr_NarrativeEvent TRexWarning;
    TRexWarning.EventID = TEXT("TRex_Territory_Warning");
    TRexWarning.DialogueText = TEXT("The ground trembles beneath massive footsteps. The apex predator rules these lands - retreat or face certain death.");
    TRexWarning.CharacterName = TEXT("Tribal Elder");
    TRexWarning.TriggerLocation = FVector(15000, -8000, 100);
    TRexWarning.TriggerRadius = 3000.0f;
    TRexWarning.bIsOneTime = false; // Can trigger multiple times
    RegisterNarrativeEvent(TRexWarning);
    
    // Raptor pack warning
    FNarr_NarrativeEvent RaptorWarning;
    RaptorWarning.EventID = TEXT("Raptor_Pack_Warning");
    RaptorWarning.DialogueText = TEXT("Clever hunters move through the undergrowth. They hunt in packs, they learn, they remember. Do not underestimate their cunning.");
    RaptorWarning.CharacterName = TEXT("Tribal Scout");
    RaptorWarning.TriggerLocation = FVector(-5000, 12000, 100);
    RaptorWarning.TriggerRadius = 2500.0f;
    RaptorWarning.bIsOneTime = false;
    RegisterNarrativeEvent(RaptorWarning);
}

void UNarrativeEventManager::CreateDiscoveryEvents()
{
    // Ancient ruins discovery
    FNarr_NarrativeEvent RuinsEvent;
    RuinsEvent.EventID = TEXT("AncientRuins_Discovery");
    RuinsEvent.DialogueText = TEXT("These stones predate the great beasts. What civilization could have built here, and what fate befell them?");
    RuinsEvent.CharacterName = TEXT("Ancient Narrator");
    RuinsEvent.TriggerLocation = FVector(-20000, -15000, 200);
    RuinsEvent.TriggerRadius = 1500.0f;
    RuinsEvent.bIsOneTime = true;
    RegisterNarrativeEvent(RuinsEvent);
    
    // Water source discovery
    FNarr_NarrativeEvent WaterEvent;
    WaterEvent.EventID = TEXT("WaterSource_Discovery");
    WaterEvent.DialogueText = TEXT("Fresh water - the essence of survival. But beware, all creatures need water, including those that hunt you.");
    WaterEvent.CharacterName = TEXT("Survival Instinct");
    WaterEvent.TriggerLocation = FVector(8000, 15000, 50);
    WaterEvent.TriggerRadius = 1000.0f;
    WaterEvent.bIsOneTime = true;
    RegisterNarrativeEvent(WaterEvent);
}

TArray<FNarr_NarrativeEvent> UNarrativeEventManager::GetActiveEvents() const
{
    TArray<FNarr_NarrativeEvent> ActiveEvents;
    
    for (const FNarr_NarrativeEvent& Event : NarrativeEvents)
    {
        if (!Event.bIsOneTime || !Event.bHasBeenTriggered)
        {
            ActiveEvents.Add(Event);
        }
    }
    
    return ActiveEvents;
}