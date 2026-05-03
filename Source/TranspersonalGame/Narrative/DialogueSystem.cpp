#include "DialogueSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalGameState.h"

ANarr_DialogueSystem::ANarr_DialogueSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio component for dialogue playback
    DialogueAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueAudioComponent"));
    RootComponent = DialogueAudioComponent;

    // Initialize dialogue state
    bIsPlayingDialogue = false;
    CurrentDialogueTime = 0.0f;
    LastKnownBiome = EEng_BiomeType::Forest;
    LastKnownWeather = EEng_WeatherType::Clear;
    LastKnownTimeOfDay = EEng_TimeOfDay::Morning;

    // Configure audio component
    DialogueAudioComponent->bAutoActivate = false;
    DialogueAudioComponent->SetVolumeMultiplier(0.8f);
}

void ANarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default dialogues and load database
    InitializeDefaultDialogues();
    LoadDialogueDatabase();
    
    UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Initialized with %d dialogue entries"), DialogueDatabase.Num());
}

void ANarr_DialogueSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update dialogue cooldowns
    UpdateCooldowns(DeltaTime);
    
    // Track current dialogue time
    if (bIsPlayingDialogue)
    {
        CurrentDialogueTime += DeltaTime;
        
        // Check if current dialogue should finish
        if (CurrentDialogueTime >= CurrentDialogue.Duration)
        {
            OnDialogueFinished();
        }
    }
}

void ANarr_DialogueSystem::TriggerDialogue(const FString& DialogueID)
{
    if (!CanPlayDialogue(DialogueID))
    {
        return;
    }

    FNarr_DialogueEntry FoundDialogue = GetDialogueEntry(DialogueID);
    if (FoundDialogue.DialogueID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueSystem: Dialogue ID '%s' not found"), *DialogueID);
        return;
    }

    // Stop current dialogue if playing
    if (bIsPlayingDialogue)
    {
        StopCurrentDialogue();
    }

    // Start new dialogue
    CurrentDialogue = FoundDialogue;
    bIsPlayingDialogue = true;
    CurrentDialogueTime = 0.0f;

    // Play audio if available
    if (!CurrentDialogue.AudioFilePath.IsEmpty())
    {
        PlayDialogueAudio(CurrentDialogue.AudioFilePath);
    }

    // Set cooldown
    DialogueCooldowns.Add(DialogueID, CurrentDialogue.RepeatCooldown);

    UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Playing dialogue '%s' by %s"), 
           *CurrentDialogue.DialogueID, *CurrentDialogue.SpeakerName);
}

void ANarr_DialogueSystem::TriggerDialogueByType(ENarr_DialogueType DialogueType, EEng_BiomeType CurrentBiome)
{
    TArray<FNarr_DialogueEntry> CandidateDialogues;
    
    // Find dialogues matching type and biome
    for (const FNarr_DialogueEntry& Entry : DialogueDatabase)
    {
        if (Entry.DialogueType == DialogueType && 
            (Entry.RequiredBiome == CurrentBiome || Entry.RequiredBiome == EEng_BiomeType::Forest) &&
            CanPlayDialogue(Entry.DialogueID))
        {
            CandidateDialogues.Add(Entry);
        }
    }

    if (CandidateDialogues.Num() > 0)
    {
        FNarr_DialogueEntry SelectedDialogue = SelectBestDialogue(CandidateDialogues);
        TriggerDialogue(SelectedDialogue.DialogueID);
    }
}

void ANarr_DialogueSystem::TriggerDinosaurDialogue(EEng_DinosaurSpecies DinosaurSpecies, float Distance)
{
    TArray<FNarr_DialogueEntry> CandidateDialogues;
    
    // Find dialogues related to this dinosaur species
    for (const FNarr_DialogueEntry& Entry : DialogueDatabase)
    {
        if (Entry.RelatedDinosaur == DinosaurSpecies && 
            Distance <= Entry.TriggerRadius &&
            CanPlayDialogue(Entry.DialogueID))
        {
            CandidateDialogues.Add(Entry);
        }
    }

    if (CandidateDialogues.Num() > 0)
    {
        FNarr_DialogueEntry SelectedDialogue = SelectBestDialogue(CandidateDialogues);
        TriggerDialogue(SelectedDialogue.DialogueID);
    }
}

void ANarr_DialogueSystem::StopCurrentDialogue()
{
    if (bIsPlayingDialogue)
    {
        bIsPlayingDialogue = false;
        CurrentDialogueTime = 0.0f;
        DialogueAudioComponent->Stop();
        
        UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Stopped dialogue '%s'"), *CurrentDialogue.DialogueID);
    }
}

bool ANarr_DialogueSystem::IsDialoguePlaying() const
{
    return bIsPlayingDialogue;
}

void ANarr_DialogueSystem::LoadDialogueDatabase()
{
    // In a full implementation, this would load from a data table or external file
    // For now, we use the initialized default dialogues
    UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Database loaded with %d entries"), DialogueDatabase.Num());
}

void ANarr_DialogueSystem::AddDialogueEntry(const FNarr_DialogueEntry& NewEntry)
{
    DialogueDatabase.Add(NewEntry);
    UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Added dialogue entry '%s'"), *NewEntry.DialogueID);
}

FNarr_DialogueEntry ANarr_DialogueSystem::GetDialogueEntry(const FString& DialogueID)
{
    for (const FNarr_DialogueEntry& Entry : DialogueDatabase)
    {
        if (Entry.DialogueID == DialogueID)
        {
            return Entry;
        }
    }
    
    // Return empty entry if not found
    return FNarr_DialogueEntry();
}

void ANarr_DialogueSystem::CheckProximityTriggers(const FVector& PlayerLocation)
{
    // This would be called by the game state or player controller
    // to check for proximity-based dialogue triggers
    
    // Implementation would check distance to known threat locations,
    // dinosaur positions, resource nodes, etc.
}

void ANarr_DialogueSystem::OnBiomeChanged(EEng_BiomeType NewBiome)
{
    if (NewBiome != LastKnownBiome)
    {
        LastKnownBiome = NewBiome;
        TriggerDialogueByType(ENarr_DialogueType::EnvironmentalAlert, NewBiome);
    }
}

void ANarr_DialogueSystem::OnDinosaurDetected(EEng_DinosaurSpecies Species, float Distance, const FVector& DinosaurLocation)
{
    // Trigger appropriate warning or observation dialogue
    TriggerDinosaurDialogue(Species, Distance);
}

void ANarr_DialogueSystem::OnWeatherChanged(EEng_WeatherType NewWeather)
{
    if (NewWeather != LastKnownWeather)
    {
        LastKnownWeather = NewWeather;
        TriggerDialogueByType(ENarr_DialogueType::EnvironmentalAlert, LastKnownBiome);
    }
}

void ANarr_DialogueSystem::OnTimeOfDayChanged(EEng_TimeOfDay NewTimeOfDay)
{
    if (NewTimeOfDay != LastKnownTimeOfDay)
    {
        LastKnownTimeOfDay = NewTimeOfDay;
        
        // Trigger time-specific dialogues (dawn warnings, night safety tips, etc.)
        if (NewTimeOfDay == EEng_TimeOfDay::Dusk || NewTimeOfDay == EEng_TimeOfDay::Night)
        {
            TriggerDialogueByType(ENarr_DialogueType::SafetyProtocol, LastKnownBiome);
        }
    }
}

void ANarr_DialogueSystem::OnHealthCritical(float HealthPercentage)
{
    if (HealthPercentage <= 25.0f)
    {
        TriggerDialogueByType(ENarr_DialogueType::SurvivalTip, LastKnownBiome);
    }
}

void ANarr_DialogueSystem::PlayDialogueAudio(const FString& AudioFilePath)
{
    // In a full implementation, this would load and play the audio file
    // For now, we just simulate audio playback
    DialogueAudioComponent->Play();
    
    UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Playing audio '%s'"), *AudioFilePath);
}

void ANarr_DialogueSystem::OnDialogueFinished()
{
    bIsPlayingDialogue = false;
    CurrentDialogueTime = 0.0f;
    DialogueAudioComponent->Stop();
    
    UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Finished dialogue '%s'"), *CurrentDialogue.DialogueID);
}

bool ANarr_DialogueSystem::CanPlayDialogue(const FString& DialogueID)
{
    // Check if dialogue is on cooldown
    if (DialogueCooldowns.Contains(DialogueID))
    {
        float* CooldownTime = DialogueCooldowns.Find(DialogueID);
        if (CooldownTime && *CooldownTime > 0.0f)
        {
            return false;
        }
    }
    
    // Don't interrupt high-priority dialogues
    if (bIsPlayingDialogue && CurrentDialogue.Priority >= 5)
    {
        return false;
    }
    
    return true;
}

void ANarr_DialogueSystem::UpdateCooldowns(float DeltaTime)
{
    TArray<FString> KeysToRemove;
    
    for (auto& CooldownPair : DialogueCooldowns)
    {
        CooldownPair.Value -= DeltaTime;
        if (CooldownPair.Value <= 0.0f)
        {
            KeysToRemove.Add(CooldownPair.Key);
        }
    }
    
    // Remove expired cooldowns
    for (const FString& Key : KeysToRemove)
    {
        DialogueCooldowns.Remove(Key);
    }
}

FNarr_DialogueEntry ANarr_DialogueSystem::SelectBestDialogue(const TArray<FNarr_DialogueEntry>& CandidateDialogues)
{
    if (CandidateDialogues.Num() == 0)
    {
        return FNarr_DialogueEntry();
    }
    
    // Sort by priority (higher priority first)
    TArray<FNarr_DialogueEntry> SortedDialogues = CandidateDialogues;
    SortedDialogues.Sort([](const FNarr_DialogueEntry& A, const FNarr_DialogueEntry& B) {
        return A.Priority > B.Priority;
    });
    
    return SortedDialogues[0];
}

void ANarr_DialogueSystem::InitializeDefaultDialogues()
{
    // Initialize with practical survival-focused dialogues
    
    // T-Rex warnings
    FNarr_DialogueEntry TRexWarning;
    TRexWarning.DialogueID = "TREX_PROXIMITY_WARNING";
    TRexWarning.DialogueType = ENarr_DialogueType::TacticalWarning;
    TRexWarning.SpeakerName = "Tactical Narrator";
    TRexWarning.DialogueText = "Attention, survivor. You've entered a critical zone. The large predator ahead is a T-Rex - apex hunter of this territory. Stay low, move slowly, and avoid direct eye contact.";
    TRexWarning.Duration = 15.0f;
    TRexWarning.TriggerType = ENarr_DialogueTrigger::DinosaurSighting;
    TRexWarning.RelatedDinosaur = EEng_DinosaurSpecies::TRex;
    TRexWarning.TriggerRadius = 2000.0f;
    TRexWarning.Priority = 10;
    TRexWarning.bCanRepeat = true;
    TRexWarning.RepeatCooldown = 600.0f; // 10 minutes
    DialogueDatabase.Add(TRexWarning);
    
    // Raptor pack warning
    FNarr_DialogueEntry RaptorWarning;
    RaptorWarning.DialogueID = "RAPTOR_PACK_WARNING";
    RaptorWarning.DialogueType = ENarr_DialogueType::TacticalWarning;
    RaptorWarning.SpeakerName = "Field Researcher";
    RaptorWarning.DialogueText = "Field research log, day 127. The pack dynamics I'm observing are extremely sophisticated. Velociraptors use coordinated flanking maneuvers. If you see one, assume there are others nearby.";
    RaptorWarning.Duration = 16.0f;
    RaptorWarning.TriggerType = ENarr_DialogueTrigger::DinosaurSighting;
    RaptorWarning.RelatedDinosaur = EEng_DinosaurSpecies::Raptor;
    RaptorWarning.TriggerRadius = 1500.0f;
    RaptorWarning.Priority = 8;
    RaptorWarning.bCanRepeat = true;
    RaptorWarning.RepeatCooldown = 450.0f;
    DialogueDatabase.Add(RaptorWarning);
    
    // Herbivore migration alert
    FNarr_DialogueEntry HerbivoreAlert;
    HerbivoreAlert.DialogueID = "HERBIVORE_MIGRATION_ALERT";
    HerbivoreAlert.DialogueType = ENarr_DialogueType::SafetyProtocol;
    HerbivoreAlert.SpeakerName = "Safety Protocol";
    HerbivoreAlert.DialogueText = "Critical alert! Detected movement of herbivore herd heading towards your position. Brachiosaurus in seasonal migration. Despite being peaceful, their size poses trampling danger. Seek elevated shelter immediately.";
    HerbivoreAlert.Duration = 18.0f;
    HerbivoreAlert.TriggerType = ENarr_DialogueTrigger::DinosaurSighting;
    HerbivoreAlert.RelatedDinosaur = EEng_DinosaurSpecies::Brachiosaurus;
    HerbivoreAlert.TriggerRadius = 3000.0f;
    HerbivoreAlert.Priority = 7;
    HerbivoreAlert.bCanRepeat = true;
    HerbivoreAlert.RepeatCooldown = 800.0f;
    DialogueDatabase.Add(HerbivoreAlert);
    
    // Biome entry - Swamp
    FNarr_DialogueEntry SwampEntry;
    SwampEntry.DialogueID = "SWAMP_BIOME_ENTRY";
    SwampEntry.DialogueType = ENarr_DialogueType::EnvironmentalAlert;
    SwampEntry.SpeakerName = "Environmental Monitor";
    SwampEntry.DialogueText = "Environmental alert: Entering swampland territory. High humidity detected. Watch for unstable ground, stagnant water, and increased insect activity. Visibility is reduced - predators use this to their advantage.";
    SwampEntry.Duration = 14.0f;
    SwampEntry.TriggerType = ENarr_DialogueTrigger::BiomeEntry;
    SwampEntry.RequiredBiome = EEng_BiomeType::Swamp;
    SwampEntry.Priority = 5;
    SwampEntry.bCanRepeat = false;
    DialogueDatabase.Add(SwampEntry);
    
    // Night safety protocol
    FNarr_DialogueEntry NightSafety;
    NightSafety.DialogueID = "NIGHT_SAFETY_PROTOCOL";
    NightSafety.DialogueType = ENarr_DialogueType::SafetyProtocol;
    NightSafety.SpeakerName = "Survival Instructor";
    NightSafety.DialogueText = "Night protocol activated. Nocturnal predators are now active. Find secure shelter, maintain fire if possible, and avoid unnecessary movement. Your survival depends on staying hidden until dawn.";
    NightSafety.Duration = 13.0f;
    NightSafety.TriggerType = ENarr_DialogueTrigger::TimeOfDay;
    NightSafety.Priority = 6;
    NightSafety.bCanRepeat = true;
    NightSafety.RepeatCooldown = 1200.0f; // 20 minutes
    DialogueDatabase.Add(NightSafety);
    
    UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Initialized %d default dialogues"), DialogueDatabase.Num());
}