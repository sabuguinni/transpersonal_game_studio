#include "NarrativeDialogueSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"

UNarrativeDialogueSystem::UNarrativeDialogueSystem()
{
    DialogueVolume = 0.8f;
    MinDialogueInterval = 15.0f;
    LastDialogueTime = 0.0f;
    CurrentDialogueID = "";
    DialogueAudioComponent = nullptr;
}

void UNarrativeDialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueSystem: Initializing..."));
    
    // Initialize dialogue database
    InitializeDialogueDatabase();
    
    // Set default narrative context
    CurrentContext = FNarr_NarrativeContext();
    CurrentContext.CurrentBiome = EEng_BiomeType::Forest;
    CurrentContext.CurrentTimeOfDay = EEng_TimeOfDay::Morning;
    CurrentContext.CurrentWeather = EEng_WeatherType::Clear;
    CurrentContext.ThreatLevel = EEng_ThreatLevel::Safe;
    CurrentContext.PlayerHealthPercentage = 100.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueSystem: Initialized with %d dialogue entries"), DialogueEntries.Num());
}

void UNarrativeDialogueSystem::Deinitialize()
{
    if (DialogueAudioComponent && DialogueAudioComponent->IsValidLowLevel())
    {
        DialogueAudioComponent->Stop();
        DialogueAudioComponent = nullptr;
    }
    
    RegisteredTriggers.Empty();
    DialogueEntries.Empty();
    
    Super::Deinitialize();
}

void UNarrativeDialogueSystem::TriggerDialogue(const FString& DialogueID, const FNarr_NarrativeContext& Context)
{
    if (!CanPlayDialogue())
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueSystem: Cannot play dialogue - cooldown active"));
        return;
    }
    
    FNarr_DialogueEntry DialogueEntry = GetDialogueEntry(DialogueID);
    if (DialogueEntry.DialogueID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueSystem: Dialogue ID not found: %s"), *DialogueID);
        return;
    }
    
    CurrentDialogueID = DialogueID;
    LastDialogueTime = GetWorld()->GetTimeSeconds();
    
    // Update context
    CurrentContext = Context;
    
    // Play audio if available
    if (!DialogueEntry.AudioURL.IsEmpty())
    {
        PlayDialogueAudio(DialogueEntry.AudioURL, DialogueVolume);
    }
    
    // Log dialogue for debugging
    UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueSystem: Playing dialogue - %s: %s"), 
           *DialogueEntry.SpeakerName, *DialogueEntry.DialogueText);
    
    // Set cooldown timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            DialogueCooldownTimer,
            this,
            &UNarrativeDialogueSystem::OnDialogueFinished,
            DialogueEntry.CooldownTime,
            false
        );
    }
}

void UNarrativeDialogueSystem::TriggerContextualNarration(const FNarr_NarrativeContext& Context)
{
    if (!CanPlayDialogue())
    {
        return;
    }
    
    // Get contextual dialogues
    TArray<FNarr_DialogueEntry> ContextualDialogues = GetContextualDialogues(Context);
    
    if (ContextualDialogues.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueSystem: No contextual dialogues found for current context"));
        return;
    }
    
    // Select best dialogue based on context
    FNarr_DialogueEntry SelectedDialogue = SelectBestDialogue(ContextualDialogues);
    
    if (!SelectedDialogue.DialogueID.IsEmpty())
    {
        TriggerDialogue(SelectedDialogue.DialogueID, Context);
    }
}

void UNarrativeDialogueSystem::TriggerEmergencyAlert(EEng_ThreatLevel ThreatLevel, const FString& ThreatDescription)
{
    // Emergency dialogues override cooldown
    StopCurrentDialogue();
    
    FNarr_NarrativeContext EmergencyContext = CurrentContext;
    EmergencyContext.ThreatLevel = ThreatLevel;
    EmergencyContext.bPlayerInDanger = true;
    
    // Find emergency dialogue
    for (const FNarr_DialogueEntry& Entry : DialogueEntries)
    {
        if (Entry.bIsEmergencyDialogue && Entry.DialogueText.Contains(ThreatDescription))
        {
            TriggerDialogue(Entry.DialogueID, EmergencyContext);
            return;
        }
    }
    
    // Fallback emergency message
    UE_LOG(LogTemp, Error, TEXT("NarrativeDialogueSystem: EMERGENCY - %s (Threat Level: %d)"), 
           *ThreatDescription, (int32)ThreatLevel);
}

void UNarrativeDialogueSystem::TriggerDiscoveryNarration(EEng_BiomeType Biome, const FString& DiscoveryType)
{
    FNarr_NarrativeContext DiscoveryContext = CurrentContext;
    DiscoveryContext.CurrentBiome = Biome;
    DiscoveryContext.bRecentDiscovery = true;
    
    // Find discovery dialogue for this biome
    for (const FNarr_DialogueEntry& Entry : DialogueEntries)
    {
        if (Entry.bIsDiscoveryDialogue && 
            Entry.TriggerBiome == Biome && 
            Entry.DialogueText.Contains(DiscoveryType))
        {
            TriggerDialogue(Entry.DialogueID, DiscoveryContext);
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueSystem: Discovery in %s - %s"), 
           *UEnum::GetValueAsString(Biome), *DiscoveryType);
}

void UNarrativeDialogueSystem::RegisterNarrativeTrigger(ATriggerBox* TriggerBox, const FString& DialogueID)
{
    if (TriggerBox && !DialogueID.IsEmpty())
    {
        RegisteredTriggers.Add(TriggerBox, DialogueID);
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueSystem: Registered trigger %s with dialogue %s"), 
               *TriggerBox->GetName(), *DialogueID);
    }
}

void UNarrativeDialogueSystem::UpdateNarrativeContext(const FNarr_NarrativeContext& NewContext)
{
    CurrentContext = NewContext;
    
    // Check if context change should trigger narration
    if (NewContext.bPlayerInDanger && NewContext.ThreatLevel >= EEng_ThreatLevel::Dangerous)
    {
        TriggerEmergencyAlert(NewContext.ThreatLevel, "High threat detected");
    }
    else if (NewContext.bRecentDiscovery)
    {
        TriggerDiscoveryNarration(NewContext.CurrentBiome, "Interesting discovery");
    }
}

void UNarrativeDialogueSystem::PlayDialogueAudio(const FString& AudioURL, float Volume)
{
    if (AudioURL.IsEmpty())
    {
        return;
    }
    
    // For now, log the audio URL - in production this would load and play the audio
    UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueSystem: Playing audio - %s (Volume: %.2f)"), 
           *AudioURL, Volume);
    
    // TODO: Implement actual audio loading and playback from URL
    // This would involve downloading the audio file and creating a USoundWave
}

void UNarrativeDialogueSystem::StopCurrentDialogue()
{
    if (DialogueAudioComponent && DialogueAudioComponent->IsValidLowLevel())
    {
        DialogueAudioComponent->Stop();
    }
    
    CurrentDialogueID = "";
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(DialogueCooldownTimer);
    }
}

bool UNarrativeDialogueSystem::IsDialoguePlaying() const
{
    return !CurrentDialogueID.IsEmpty() && 
           DialogueAudioComponent && 
           DialogueAudioComponent->IsValidLowLevel() && 
           DialogueAudioComponent->IsPlaying();
}

void UNarrativeDialogueSystem::LoadDialogueDatabase()
{
    // Load dialogue entries from data asset or config file
    // For now, create default entries
    CreateDefaultDialogues();
}

void UNarrativeDialogueSystem::AddDialogueEntry(const FNarr_DialogueEntry& Entry)
{
    DialogueEntries.Add(Entry);
    UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueSystem: Added dialogue entry - %s"), *Entry.DialogueID);
}

FNarr_DialogueEntry UNarrativeDialogueSystem::GetDialogueEntry(const FString& DialogueID) const
{
    for (const FNarr_DialogueEntry& Entry : DialogueEntries)
    {
        if (Entry.DialogueID == DialogueID)
        {
            return Entry;
        }
    }
    
    return FNarr_DialogueEntry(); // Return empty entry if not found
}

TArray<FNarr_DialogueEntry> UNarrativeDialogueSystem::GetContextualDialogues(const FNarr_NarrativeContext& Context) const
{
    TArray<FNarr_DialogueEntry> ContextualEntries;
    
    for (const FNarr_DialogueEntry& Entry : DialogueEntries)
    {
        // Check biome match
        if (Entry.TriggerBiome == Context.CurrentBiome)
        {
            ContextualEntries.Add(Entry);
            continue;
        }
        
        // Check emergency context
        if (Entry.bIsEmergencyDialogue && Context.bPlayerInDanger)
        {
            ContextualEntries.Add(Entry);
            continue;
        }
        
        // Check discovery context
        if (Entry.bIsDiscoveryDialogue && Context.bRecentDiscovery)
        {
            ContextualEntries.Add(Entry);
            continue;
        }
    }
    
    return ContextualEntries;
}

void UNarrativeDialogueSystem::InitializeDialogueDatabase()
{
    DialogueEntries.Empty();
    CreateDefaultDialogues();
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueSystem: Dialogue database initialized with %d entries"), 
           DialogueEntries.Num());
}

void UNarrativeDialogueSystem::CreateDefaultDialogues()
{
    // Forest exploration dialogues
    FNarr_DialogueEntry ForestEntry;
    ForestEntry.DialogueID = "forest_exploration_01";
    ForestEntry.SpeakerName = "Field Explorer";
    ForestEntry.DialogueText = "Atenção, explorador! Descobri vestígios de uma antiga migração de Brachiosaurus nesta região.";
    ForestEntry.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777885154222_FieldExplorer.mp3";
    ForestEntry.TriggerBiome = EEng_BiomeType::Forest;
    ForestEntry.bIsDiscoveryDialogue = true;
    ForestEntry.CooldownTime = 45.0f;
    AddDialogueEntry(ForestEntry);
    
    // Research observation dialogue
    FNarr_DialogueEntry ResearchEntry;
    ResearchEntry.DialogueID = "research_observation_01";
    ResearchEntry.SpeakerName = "Research Naturalist";
    ResearchEntry.DialogueText = "Registo de campo, dia 127. Observei comportamento territorial fascinante entre dois machos de Triceratops.";
    ResearchEntry.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777885161419_ResearchNaturalist.mp3";
    ResearchEntry.TriggerBiome = EEng_BiomeType::Savanna;
    ResearchEntry.bIsDiscoveryDialogue = true;
    ResearchEntry.CooldownTime = 60.0f;
    AddDialogueEntry(ResearchEntry);
    
    // Emergency survival dialogue
    FNarr_DialogueEntry EmergencyEntry;
    EmergencyEntry.DialogueID = "emergency_storm_01";
    EmergencyEntry.SpeakerName = "Survival Advisor";
    EmergencyEntry.DialogueText = "Alerta de sobrevivência! Detectei sinais de aproximação de uma tempestade severa.";
    EmergencyEntry.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777885168772_SurvivalAdvisor.mp3";
    EmergencyEntry.TriggerBiome = EEng_BiomeType::Desert;
    EmergencyEntry.bIsEmergencyDialogue = true;
    EmergencyEntry.CooldownTime = 30.0f;
    AddDialogueEntry(EmergencyEntry);
    
    // Paleontology discovery dialogue
    FNarr_DialogueEntry PaleoEntry;
    PaleoEntry.DialogueID = "paleontology_discovery_01";
    PaleoEntry.SpeakerName = "Paleontology Expert";
    PaleoEntry.DialogueText = "Descoberta extraordinária! Encontrei um ninho abandonado de Pteranodon no topo desta falésia.";
    PaleoEntry.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777885175756_PaleontologyExpert.mp3";
    PaleoEntry.TriggerBiome = EEng_BiomeType::SnowyMountain;
    PaleoEntry.bIsDiscoveryDialogue = true;
    PaleoEntry.CooldownTime = 90.0f;
    AddDialogueEntry(PaleoEntry);
    
    // Swamp danger dialogue
    FNarr_DialogueEntry SwampEntry;
    SwampEntry.DialogueID = "swamp_danger_01";
    SwampEntry.SpeakerName = "Survival Advisor";
    SwampEntry.DialogueText = "Cuidado nesta área pantanosa. A lama pode ser traiçoeira e os predadores aquáticos são uma ameaça constante.";
    SwampEntry.TriggerBiome = EEng_BiomeType::Swamp;
    SwampEntry.bIsEmergencyDialogue = false;
    SwampEntry.CooldownTime = 40.0f;
    AddDialogueEntry(SwampEntry);
}

FNarr_DialogueEntry UNarrativeDialogueSystem::SelectBestDialogue(const TArray<FNarr_DialogueEntry>& Candidates) const
{
    if (Candidates.Num() == 0)
    {
        return FNarr_DialogueEntry();
    }
    
    // Priority system: Emergency > Discovery > General
    for (const FNarr_DialogueEntry& Entry : Candidates)
    {
        if (Entry.bIsEmergencyDialogue && CurrentContext.bPlayerInDanger)
        {
            return Entry;
        }
    }
    
    for (const FNarr_DialogueEntry& Entry : Candidates)
    {
        if (Entry.bIsDiscoveryDialogue && CurrentContext.bRecentDiscovery)
        {
            return Entry;
        }
    }
    
    // Return first general dialogue
    return Candidates[0];
}

bool UNarrativeDialogueSystem::CanPlayDialogue() const
{
    if (!GetWorld())
    {
        return false;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastDialogueTime) >= MinDialogueInterval;
}

void UNarrativeDialogueSystem::OnDialogueFinished()
{
    CurrentDialogueID = "";
    UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueSystem: Dialogue finished, cooldown complete"));
}