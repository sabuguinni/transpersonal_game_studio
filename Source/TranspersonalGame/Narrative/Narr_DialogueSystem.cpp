#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    bIsDialoguePlaying = false;
    DialogueTimer = 0.0f;
}

void UNarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultDialogue();
    
    UE_LOG(LogTemp, Log, TEXT("Narrative Dialogue System initialized with %d dialogue lines"), DialogueDatabase.Num());
}

void UNarr_DialogueSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsDialoguePlaying)
    {
        UpdateDialogueTimer(DeltaTime);
    }
}

void UNarr_DialogueSystem::PlayDialogue(const FNarr_DialogueLine& DialogueLine)
{
    if (bIsDialoguePlaying)
    {
        StopDialogue();
    }
    
    CurrentDialogue = DialogueLine;
    bIsDialoguePlaying = true;
    DialogueTimer = DialogueLine.Duration;
    
    // Broadcast dialogue started event
    OnDialogueStarted.Broadcast(DialogueLine);
    
    // Log dialogue for debugging
    UE_LOG(LogTemp, Log, TEXT("Playing dialogue: %s - %s"), *DialogueLine.SpeakerName, *DialogueLine.DialogueText);
    
    // Display on screen for testing
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("[%s]: %s"), *DialogueLine.SpeakerName, *DialogueLine.DialogueText);
        GEngine->AddOnScreenDebugMessage(-1, DialogueLine.Duration, FColor::Green, DisplayText);
    }
}

void UNarr_DialogueSystem::StopDialogue()
{
    if (bIsDialoguePlaying)
    {
        bIsDialoguePlaying = false;
        DialogueTimer = 0.0f;
        OnDialogueEnded.Broadcast();
        
        UE_LOG(LogTemp, Log, TEXT("Dialogue stopped"));
    }
}

void UNarr_DialogueSystem::TriggerNarrativeEvent(const FString& EventID)
{
    for (FNarr_NarrativeEvent& Event : NarrativeEvents)
    {
        if (Event.EventID == EventID && !Event.bHasBeenTriggered)
        {
            Event.bHasBeenTriggered = true;
            
            // Play first dialogue line from the event
            if (Event.DialogueLines.Num() > 0)
            {
                PlayDialogue(Event.DialogueLines[0]);
            }
            
            OnNarrativeEventTriggered.Broadcast(EventID);
            UE_LOG(LogTemp, Log, TEXT("Triggered narrative event: %s"), *EventID);
            break;
        }
    }
}

void UNarr_DialogueSystem::TriggerBiomeDialogue(EEng_BiomeType BiomeType)
{
    FNarr_DialogueLine BiomeDialogue = GetRandomDialogueByType(BiomeType);
    if (!BiomeDialogue.DialogueText.IsEmpty())
    {
        PlayDialogue(BiomeDialogue);
    }
}

void UNarr_DialogueSystem::TriggerSurvivalDialogue(EEng_SurvivalStat StatType, float StatValue)
{
    FNarr_DialogueLine SurvivalDialogue;
    SurvivalDialogue.SpeakerName = "Survival Guide";
    SurvivalDialogue.Duration = 8.0f;
    SurvivalDialogue.bIsContextual = true;
    
    switch (StatType)
    {
        case EEng_SurvivalStat::Health:
            if (StatValue < 30.0f)
            {
                SurvivalDialogue.DialogueText = "Warning! Your health is critically low. Seek shelter and rest immediately.";
            }
            break;
            
        case EEng_SurvivalStat::Hunger:
            if (StatValue < 25.0f)
            {
                SurvivalDialogue.DialogueText = "You're starving. Find food sources quickly - berries, small game, or fish.";
            }
            break;
            
        case EEng_SurvivalStat::Thirst:
            if (StatValue < 20.0f)
            {
                SurvivalDialogue.DialogueText = "Severe dehydration detected. Locate fresh water immediately or risk collapse.";
            }
            break;
            
        case EEng_SurvivalStat::Fear:
            if (StatValue > 80.0f)
            {
                SurvivalDialogue.DialogueText = "Your fear levels are dangerously high. Find a safe location and calm yourself.";
            }
            break;
            
        default:
            return;
    }
    
    if (!SurvivalDialogue.DialogueText.IsEmpty())
    {
        PlayDialogue(SurvivalDialogue);
    }
}

void UNarr_DialogueSystem::TriggerDinosaurEncounterDialogue(EEng_DinosaurSpecies Species, float Distance)
{
    FNarr_DialogueLine EncounterDialogue;
    EncounterDialogue.SpeakerName = "Field Researcher";
    EncounterDialogue.Duration = 10.0f;
    EncounterDialogue.bIsContextual = true;
    
    FString SpeciesName;
    switch (Species)
    {
        case EEng_DinosaurSpecies::TRex:
            SpeciesName = "Tyrannosaurus Rex";
            if (Distance < 5000.0f)
            {
                EncounterDialogue.DialogueText = "Massive T-Rex detected nearby! Remain absolutely still and avoid eye contact.";
            }
            else
            {
                EncounterDialogue.DialogueText = "T-Rex spotted in the distance. Magnificent apex predator - observe from safe range.";
            }
            break;
            
        case EEng_DinosaurSpecies::Raptor:
            SpeciesName = "Velociraptor";
            if (Distance < 3000.0f)
            {
                EncounterDialogue.DialogueText = "Raptor pack detected! They hunt in coordinated groups. Seek high ground immediately.";
            }
            else
            {
                EncounterDialogue.DialogueText = "Velociraptors in the area. Intelligent hunters with pack tactics. Stay alert.";
            }
            break;
            
        case EEng_DinosaurSpecies::Brachiosaurus:
            SpeciesName = "Brachiosaurus";
            EncounterDialogue.DialogueText = "Gentle giant Brachiosaurus ahead. Peaceful herbivore, but mind the massive feet.";
            break;
            
        case EEng_DinosaurSpecies::Triceratops:
            SpeciesName = "Triceratops";
            EncounterDialogue.DialogueText = "Triceratops herd spotted. Defensive herbivores with dangerous horns when threatened.";
            break;
            
        default:
            EncounterDialogue.DialogueText = "Unknown dinosaur species detected. Proceed with extreme caution.";
            break;
    }
    
    PlayDialogue(EncounterDialogue);
}

void UNarr_DialogueSystem::LoadDialogueFromDataTable(UDataTable* DialogueTable)
{
    if (!DialogueTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue table is null"));
        return;
    }
    
    // Clear existing dialogue
    DialogueDatabase.Empty();
    
    // Load from data table (implementation would depend on table structure)
    UE_LOG(LogTemp, Log, TEXT("Loading dialogue from data table"));
}

void UNarr_DialogueSystem::AddDialogueToDatabase(const FNarr_DialogueLine& NewDialogue)
{
    DialogueDatabase.Add(NewDialogue);
    UE_LOG(LogTemp, Log, TEXT("Added dialogue to database: %s"), *NewDialogue.SpeakerName);
}

TArray<FNarr_DialogueLine> UNarr_DialogueSystem::GetDialogueByBiome(EEng_BiomeType BiomeType)
{
    TArray<FNarr_DialogueLine> BiomeDialogues;
    
    for (const FNarr_DialogueLine& Dialogue : DialogueDatabase)
    {
        if (Dialogue.TriggerBiome == BiomeType)
        {
            BiomeDialogues.Add(Dialogue);
        }
    }
    
    return BiomeDialogues;
}

void UNarr_DialogueSystem::InitializeDefaultDialogue()
{
    // Initialize default biome-specific dialogues
    FNarr_DialogueLine SwampDialogue;
    SwampDialogue.SpeakerName = "Field Narrator";
    SwampDialogue.DialogueText = "Welcome to the prehistoric swamplands. Watch for crocodilians and toxic plants.";
    SwampDialogue.TriggerBiome = EEng_BiomeType::Swamp;
    SwampDialogue.Duration = 8.0f;
    DialogueDatabase.Add(SwampDialogue);
    
    FNarr_DialogueLine ForestDialogue;
    ForestDialogue.SpeakerName = "Field Narrator";
    ForestDialogue.DialogueText = "Dense prehistoric forest detected. Visibility is limited - predators may be lurking.";
    ForestDialogue.TriggerBiome = EEng_BiomeType::Forest;
    ForestDialogue.Duration = 8.0f;
    DialogueDatabase.Add(ForestDialogue);
    
    FNarr_DialogueLine SavannaDialogue;
    SavannaDialogue.SpeakerName = "Field Narrator";
    SavannaDialogue.DialogueText = "Open savanna plains ahead. Good visibility but little cover from predators.";
    SavannaDialogue.TriggerBiome = EEng_BiomeType::Savanna;
    SavannaDialogue.Duration = 8.0f;
    DialogueDatabase.Add(SavannaDialogue);
    
    FNarr_DialogueLine DesertDialogue;
    DesertDialogue.SpeakerName = "Field Narrator";
    DesertDialogue.DialogueText = "Arid desert environment. Monitor temperature and water levels carefully.";
    DesertDialogue.TriggerBiome = EEng_BiomeType::Desert;
    DesertDialogue.Duration = 8.0f;
    DialogueDatabase.Add(DesertDialogue);
    
    FNarr_DialogueLine MountainDialogue;
    MountainDialogue.SpeakerName = "Field Narrator";
    MountainDialogue.DialogueText = "High altitude snowy peaks. Cold temperatures and treacherous terrain ahead.";
    MountainDialogue.TriggerBiome = EEng_BiomeType::SnowyMountain;
    MountainDialogue.Duration = 8.0f;
    DialogueDatabase.Add(MountainDialogue);
    
    // Create default narrative events
    FNarr_NarrativeEvent FirstExploration;
    FirstExploration.EventID = "FirstExploration";
    FirstExploration.EventDescription = "Player's first exploration of the prehistoric world";
    FirstExploration.RelatedQuestType = EEng_QuestType::Exploration;
    
    FNarr_DialogueLine IntroDialogue;
    IntroDialogue.SpeakerName = "Research Lead";
    IntroDialogue.DialogueText = "Welcome to the Cretaceous period. Your survival depends on understanding this ancient ecosystem.";
    IntroDialogue.Duration = 10.0f;
    FirstExploration.DialogueLines.Add(IntroDialogue);
    
    NarrativeEvents.Add(FirstExploration);
}

void UNarr_DialogueSystem::UpdateDialogueTimer(float DeltaTime)
{
    if (DialogueTimer > 0.0f)
    {
        DialogueTimer -= DeltaTime;
        
        if (DialogueTimer <= 0.0f)
        {
            StopDialogue();
        }
    }
}

FNarr_DialogueLine UNarr_DialogueSystem::GetRandomDialogueByType(EEng_BiomeType BiomeType)
{
    TArray<FNarr_DialogueLine> BiomeDialogues = GetDialogueByBiome(BiomeType);
    
    if (BiomeDialogues.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, BiomeDialogues.Num() - 1);
        return BiomeDialogues[RandomIndex];
    }
    
    return FNarr_DialogueLine();
}