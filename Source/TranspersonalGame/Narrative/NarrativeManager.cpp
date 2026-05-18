#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "TimerManager.h"

UNarrativeManager::UNarrativeManager()
{
    CurrentStoryPhase = ENarr_StoryPhase::Introduction;
    LastNarrationTime = 0.0f;
    NarrationCooldown = 30.0f; // 30 seconds between narrations
}

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initializing narrative system"));
    
    InitializeDefaultEvents();
    LoadNarrativeData();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initialized with %d events"), RegisteredEvents.Num());
}

void UNarrativeManager::Deinitialize()
{
    RegisteredEvents.Empty();
    TriggeredEventIDs.Empty();
    
    Super::Deinitialize();
}

void UNarrativeManager::TriggerNarrativeEvent(const FString& EventID, AActor* TriggeringActor)
{
    if (!CanTriggerNarration())
    {
        return;
    }

    FNarr_NarrativeEvent* Event = FindEventByID(EventID);
    if (!Event)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Event %s not found"), *EventID);
        return;
    }

    if (Event->bHasBeenTriggered && !Event->bIsRepeatable)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Triggering event %s"), *EventID);

    // Mark event as triggered
    Event->bHasBeenTriggered = true;
    if (!TriggeredEventIDs.Contains(EventID))
    {
        TriggeredEventIDs.Add(EventID);
    }

    // Play dialogue lines
    APlayerController* PlayerController = nullptr;
    if (UWorld* World = GetWorld())
    {
        PlayerController = World->GetFirstPlayerController();
    }

    for (const FNarr_DialogueLine& DialogueLine : Event->DialogueLines)
    {
        PlayDialogue(DialogueLine, PlayerController);
    }

    LastNarrationTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

void UNarrativeManager::PlayDialogue(const FNarr_DialogueLine& DialogueLine, APlayerController* PlayerController)
{
    if (DialogueLine.DialogueText.IsEmpty())
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: %s: %s"), *DialogueLine.SpeakerName, *DialogueLine.DialogueText);

    // Play voice clip if available
    if (DialogueLine.VoiceClip && PlayerController)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), DialogueLine.VoiceClip);
    }

    // Display text (in a real game, this would go to UI)
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("%s: %s"), *DialogueLine.SpeakerName, *DialogueLine.DialogueText);
        GEngine->AddOnScreenDebugMessage(-1, DialogueLine.Duration, FColor::Yellow, DisplayText);
    }
}

void UNarrativeManager::AdvanceStoryPhase()
{
    int32 CurrentPhaseInt = static_cast<int32>(CurrentStoryPhase);
    int32 MaxPhaseInt = static_cast<int32>(ENarr_StoryPhase::FinalSurvival);
    
    if (CurrentPhaseInt < MaxPhaseInt)
    {
        CurrentStoryPhase = static_cast<ENarr_StoryPhase>(CurrentPhaseInt + 1);
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Advanced to story phase %d"), CurrentPhaseInt + 1);
        
        // Trigger phase change event
        FString PhaseEventID = FString::Printf(TEXT("StoryPhase_%d"), CurrentPhaseInt + 1);
        TriggerNarrativeEvent(PhaseEventID);
    }
}

void UNarrativeManager::RegisterNarrativeEvent(const FNarr_NarrativeEvent& NewEvent)
{
    if (NewEvent.EventID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Cannot register event with empty ID"));
        return;
    }

    // Check if event already exists
    if (FindEventByID(NewEvent.EventID))
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Event %s already exists"), *NewEvent.EventID);
        return;
    }

    RegisteredEvents.Add(NewEvent);
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Registered event %s"), *NewEvent.EventID);
}

bool UNarrativeManager::IsEventTriggered(const FString& EventID) const
{
    return TriggeredEventIDs.Contains(EventID);
}

void UNarrativeManager::CheckLocationBasedEvents(const FVector& PlayerLocation)
{
    if (!CanTriggerNarration())
    {
        return;
    }

    for (FNarr_NarrativeEvent& Event : RegisteredEvents)
    {
        if (Event.bHasBeenTriggered && !Event.bIsRepeatable)
        {
            continue;
        }

        float Distance = FVector::Dist(PlayerLocation, Event.TriggerLocation);
        if (Distance <= Event.TriggerRadius)
        {
            TriggerNarrativeEvent(Event.EventID);
            break; // Only trigger one event per check
        }
    }
}

void UNarrativeManager::TriggerEnvironmentalNarration(EBiomeType BiomeType, const FVector& Location)
{
    if (!CanTriggerNarration())
    {
        return;
    }

    FString BiomeEventID;
    switch (BiomeType)
    {
        case EBiomeType::Savanna:
            BiomeEventID = TEXT("BiomeEntry_Savanna");
            break;
        case EBiomeType::Forest:
            BiomeEventID = TEXT("BiomeEntry_Forest");
            break;
        case EBiomeType::Desert:
            BiomeEventID = TEXT("BiomeEntry_Desert");
            break;
        case EBiomeType::Swamp:
            BiomeEventID = TEXT("BiomeEntry_Swamp");
            break;
        case EBiomeType::Mountain:
            BiomeEventID = TEXT("BiomeEntry_Mountain");
            break;
        default:
            return;
    }

    TriggerNarrativeEvent(BiomeEventID);
}

void UNarrativeManager::OnSurvivalStateChanged(float Health, float Hunger, float Thirst, float Fear)
{
    if (!CanTriggerNarration())
    {
        return;
    }

    // Trigger survival-based narrative events
    if (Health < 0.3f && !IsEventTriggered(TEXT("LowHealth_Warning")))
    {
        TriggerNarrativeEvent(TEXT("LowHealth_Warning"));
    }
    else if (Hunger < 0.2f && !IsEventTriggered(TEXT("Starving_Warning")))
    {
        TriggerNarrativeEvent(TEXT("Starving_Warning"));
    }
    else if (Thirst < 0.2f && !IsEventTriggered(TEXT("Dehydrated_Warning")))
    {
        TriggerNarrativeEvent(TEXT("Dehydrated_Warning"));
    }
    else if (Fear > 0.8f && !IsEventTriggered(TEXT("HighFear_Narration")))
    {
        TriggerNarrativeEvent(TEXT("HighFear_Narration"));
    }
}

void UNarrativeManager::OnDinosaurEncounter(const FString& DinosaurType, bool bIsHostile)
{
    if (!CanTriggerNarration())
    {
        return;
    }

    FString EncounterEventID = FString::Printf(TEXT("DinosaurEncounter_%s"), *DinosaurType);
    
    if (bIsHostile)
    {
        EncounterEventID += TEXT("_Hostile");
    }
    else
    {
        EncounterEventID += TEXT("_Peaceful");
    }

    TriggerNarrativeEvent(EncounterEventID);
}

void UNarrativeManager::InitializeDefaultEvents()
{
    // Introduction phase events
    FNarr_NarrativeEvent IntroEvent;
    IntroEvent.EventID = TEXT("GameStart_Introduction");
    IntroEvent.EventDescription = TEXT("Player starts the game");
    IntroEvent.TriggerLocation = FVector(0, 0, 0);
    IntroEvent.TriggerRadius = 5000.0f;
    IntroEvent.bIsRepeatable = false;

    FNarr_DialogueLine IntroLine;
    IntroLine.SpeakerName = TEXT("Valley Narrator");
    IntroLine.DialogueText = TEXT("The ancient valley holds many secrets, survivor. Your journey in this primordial world begins now.");
    IntroLine.Duration = 5.0f;
    IntroEvent.DialogueLines.Add(IntroLine);

    RegisteredEvents.Add(IntroEvent);

    // Biome entry events
    FNarr_NarrativeEvent SavannaEvent;
    SavannaEvent.EventID = TEXT("BiomeEntry_Savanna");
    SavannaEvent.EventDescription = TEXT("Player enters savanna biome");
    SavannaEvent.TriggerLocation = FVector(0, 0, 0);
    SavannaEvent.TriggerRadius = 10000.0f;
    SavannaEvent.bIsRepeatable = true;

    FNarr_DialogueLine SavannaLine;
    SavannaLine.SpeakerName = TEXT("Scout");
    SavannaLine.DialogueText = TEXT("The open savanna stretches before you. Watch for predators in the tall grass.");
    SavannaLine.Duration = 4.0f;
    SavannaEvent.DialogueLines.Add(SavannaLine);

    RegisteredEvents.Add(SavannaEvent);

    // Survival warning events
    FNarr_NarrativeEvent LowHealthEvent;
    LowHealthEvent.EventID = TEXT("LowHealth_Warning");
    LowHealthEvent.EventDescription = TEXT("Player health is critically low");
    LowHealthEvent.bIsRepeatable = true;

    FNarr_DialogueLine HealthLine;
    HealthLine.SpeakerName = TEXT("Inner Voice");
    HealthLine.DialogueText = TEXT("Your strength fades. Find shelter and tend to your wounds before it's too late.");
    HealthLine.Duration = 4.0f;
    LowHealthEvent.DialogueLines.Add(HealthLine);

    RegisteredEvents.Add(LowHealthEvent);
}

void UNarrativeManager::LoadNarrativeData()
{
    // In a full implementation, this would load from data tables or JSON files
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Loading narrative data from default setup"));
}

FNarr_NarrativeEvent* UNarrativeManager::FindEventByID(const FString& EventID)
{
    for (FNarr_NarrativeEvent& Event : RegisteredEvents)
    {
        if (Event.EventID == EventID)
        {
            return &Event;
        }
    }
    return nullptr;
}

bool UNarrativeManager::CanTriggerNarration() const
{
    if (!GetWorld())
    {
        return false;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastNarrationTime) >= NarrationCooldown;
}