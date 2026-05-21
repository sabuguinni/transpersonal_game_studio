#include "NarrativeDialogueManager.h"
#include "Quest/QuestManager.h"
#include "Crowd/CrowdSimulationManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

UNarrativeDialogueManager::UNarrativeDialogueManager()
{
    QuestManagerRef = nullptr;
    CrowdManagerRef = nullptr;
}

void UNarrativeDialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Initializing dialogue system"));
    
    InitializeDialogueSystem();
}

void UNarrativeDialogueManager::Deinitialize()
{
    DialogueDatabase.Empty();
    QuestDialogues.Empty();
    UnlockedDialogues.Empty();
    
    QuestManagerRef = nullptr;
    CrowdManagerRef = nullptr;
    
    Super::Deinitialize();
}

void UNarrativeDialogueManager::InitializeDialogueSystem()
{
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Loading dialogue database"));
    
    LoadDialogueDatabase();
    SetupBiomeDialogues();
    SetupMigrationDialogues();
    SetupPredatorDialogues();
    
    // Initialize narrative context
    CurrentContext = FNarr_NarrativeContext();
    CurrentContext.CurrentBiome = ENarr_BiomeType::Savana;
    CurrentContext.SurvivalTime = 0.0f;
    
    // Unlock initial dialogues
    UnlockDialogue("intro_valley");
    UnlockDialogue("survival_basics");
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Dialogue system initialized with %d entries"), DialogueDatabase.Num());
}

void UNarrativeDialogueManager::LoadDialogueDatabase()
{
    // Load core narrative dialogues
    DialogueDatabase.Add("intro_valley", CreateDialogueEntry(
        "intro_valley", 
        "Valley Narrator", 
        "The ancient valley holds many secrets, survivor. Listen carefully to the wind - it carries the scent of predators and the distant calls of the great herds.",
        "/Game/Audio/Narrative/valley_intro.wav"
    ));
    
    DialogueDatabase.Add("survival_basics", CreateDialogueEntry(
        "survival_basics",
        "Survival Guide",
        "Your survival depends on understanding the patterns of this prehistoric world. Watch for signs, listen for warnings, and always have an escape route.",
        "/Game/Audio/Narrative/survival_guide.wav"
    ));
    
    DialogueDatabase.Add("first_dinosaur", CreateDialogueEntry(
        "first_dinosaur",
        "Valley Narrator",
        "You are not alone in this valley. The giants that ruled this world still walk among us. Observe them, learn their ways, but keep your distance.",
        "/Game/Audio/Narrative/first_dinosaur.wav"
    ));
}

void UNarrativeDialogueManager::SetupBiomeDialogues()
{
    // Savana biome dialogues
    DialogueDatabase.Add("savana_entry", CreateDialogueEntry(
        "savana_entry",
        "Territory Scout",
        "The open plains offer visibility but little shelter. Predators can spot you from great distances here. Move quickly between cover points.",
        "/Game/Audio/Narrative/savana_warning.wav"
    ));
    
    // Forest biome dialogues
    DialogueDatabase.Add("forest_entry", CreateDialogueEntry(
        "forest_entry",
        "Forest Guide",
        "The dense canopy provides shelter but conceals many dangers. Listen for the snap of branches - it may be your only warning of an approaching predator.",
        "/Game/Audio/Narrative/forest_warning.wav"
    ));
    
    // Desert biome dialogues
    DialogueDatabase.Add("desert_entry", CreateDialogueEntry(
        "desert_entry",
        "Desert Survivor",
        "Water is life in these barren lands. Follow the ancient riverbeds and watch for signs of underground springs. The desert keeps its secrets well.",
        "/Game/Audio/Narrative/desert_warning.wav"
    ));
}

void UNarrativeDialogueManager::SetupMigrationDialogues()
{
    DialogueDatabase.Add("migration_detected", CreateDialogueEntry(
        "migration_detected",
        "Migration Guide",
        "The great herds are on the move! Their ancient instincts guide them to safety. Follow their trails to discover water sources and safe passages.",
        "/Game/Audio/Narrative/migration_start.wav"
    ));
    
    DialogueDatabase.Add("migration_complete", CreateDialogueEntry(
        "migration_complete",
        "Migration Guide",
        "The migration is complete. You have witnessed one of nature's greatest spectacles. The knowledge of their routes will serve you well.",
        "/Game/Audio/Narrative/migration_complete.wav"
    ));
}

void UNarrativeDialogueManager::SetupPredatorDialogues()
{
    DialogueDatabase.Add("pack_hunters", CreateDialogueEntry(
        "pack_hunters",
        "Survival Guide",
        "Warning! Pack hunters detected in the eastern ravines. Their coordinated movements suggest they are tracking prey. Stay low and move carefully.",
        "/Game/Audio/Narrative/pack_warning.wav"
    ));
    
    DialogueDatabase.Add("apex_territory", CreateDialogueEntry(
        "apex_territory",
        "Territory Scout",
        "Territory marked by the apex predator. Fresh claw marks indicate a T-Rex has claimed this hunting ground. Find an alternate route immediately.",
        "/Game/Audio/Narrative/apex_warning.wav"
    ));
}

FNarr_DialogueEntry UNarrativeDialogueManager::GetDialogueForContext(const FNarr_NarrativeContext& Context)
{
    // Priority-based dialogue selection
    if (Context.bInDanger && IsDialogueUnlocked("pack_hunters"))
    {
        return DialogueDatabase["pack_hunters"];
    }
    
    // Biome-specific dialogues
    FString BiomeDialogueID;
    switch (Context.CurrentBiome)
    {
        case ENarr_BiomeType::Savana:
            BiomeDialogueID = "savana_entry";
            break;
        case ENarr_BiomeType::Floresta:
            BiomeDialogueID = "forest_entry";
            break;
        case ENarr_BiomeType::Deserto:
            BiomeDialogueID = "desert_entry";
            break;
        default:
            BiomeDialogueID = "intro_valley";
            break;
    }
    
    if (IsDialogueUnlocked(BiomeDialogueID))
    {
        return DialogueDatabase[BiomeDialogueID];
    }
    
    // Default to intro dialogue
    return DialogueDatabase["intro_valley"];
}

void UNarrativeDialogueManager::TriggerContextualDialogue(ENarr_BiomeType Biome, const FString& EventType)
{
    CurrentContext.CurrentBiome = Biome;
    
    if (EventType == "Migration")
    {
        UnlockDialogue("migration_detected");
        FNarr_DialogueEntry DialogueEntry = DialogueDatabase["migration_detected"];
        PlayDialogueAudio(DialogueEntry.AudioPath);
    }
    else if (EventType == "PredatorPack")
    {
        CurrentContext.bInDanger = true;
        UnlockDialogue("pack_hunters");
        FNarr_DialogueEntry DialogueEntry = DialogueDatabase["pack_hunters"];
        PlayDialogueAudio(DialogueEntry.AudioPath);
    }
    else if (EventType == "ApexTerritory")
    {
        CurrentContext.bInDanger = true;
        UnlockDialogue("apex_territory");
        FNarr_DialogueEntry DialogueEntry = DialogueDatabase["apex_territory"];
        PlayDialogueAudio(DialogueEntry.AudioPath);
    }
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Triggered contextual dialogue for %s in biome %d"), *EventType, (int32)Biome);
}

void UNarrativeDialogueManager::UpdateNarrativeContext(const FNarr_NarrativeContext& NewContext)
{
    CurrentContext = NewContext;
    
    // Unlock progression-based dialogues
    if (CurrentContext.DinosaursSeen >= 1 && !IsDialogueUnlocked("first_dinosaur"))
    {
        UnlockDialogue("first_dinosaur");
    }
    
    if (CurrentContext.QuestsCompleted >= 1 && !IsDialogueUnlocked("migration_complete"))
    {
        UnlockDialogue("migration_complete");
    }
}

void UNarrativeDialogueManager::RegisterQuestDialogue(const FString& QuestID, const TArray<FNarr_DialogueEntry>& DialogueEntries)
{
    QuestDialogues.Add(QuestID, DialogueEntries);
    
    // Add to main database for easy access
    for (const FNarr_DialogueEntry& Entry : DialogueEntries)
    {
        DialogueDatabase.Add(Entry.DialogueID, Entry);
    }
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Registered %d dialogue entries for quest %s"), DialogueEntries.Num(), *QuestID);
}

TArray<FNarr_DialogueEntry> UNarrativeDialogueManager::GetQuestDialogue(const FString& QuestID)
{
    if (QuestDialogues.Contains(QuestID))
    {
        return QuestDialogues[QuestID];
    }
    
    return TArray<FNarr_DialogueEntry>();
}

void UNarrativeDialogueManager::OnHerdMigrationDetected(ENarr_BiomeType FromBiome, ENarr_BiomeType ToBiome, int32 HerdSize)
{
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Herd migration detected - %d animals from biome %d to %d"), HerdSize, (int32)FromBiome, (int32)ToBiome);
    
    TriggerContextualDialogue(FromBiome, "Migration");
    
    // Update context
    CurrentContext.CurrentBiome = FromBiome;
    CurrentContext.bInDanger = false; // Migrations are generally safe events
}

void UNarrativeDialogueManager::OnPredatorEncounter(const FString& PredatorType, ENarr_BiomeType Biome)
{
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Predator encounter - %s in biome %d"), *PredatorType, (int32)Biome);
    
    if (PredatorType.Contains("Pack") || PredatorType.Contains("Raptor"))
    {
        TriggerContextualDialogue(Biome, "PredatorPack");
    }
    else if (PredatorType.Contains("TRex") || PredatorType.Contains("Apex"))
    {
        TriggerContextualDialogue(Biome, "ApexTerritory");
    }
}

void UNarrativeDialogueManager::PlayDialogueAudio(const FString& AudioPath)
{
    if (AudioPath.IsEmpty())
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Load and play audio
    USoundBase* DialogueSound = LoadObject<USoundBase>(nullptr, *AudioPath);
    if (DialogueSound)
    {
        UGameplayStatics::PlaySound2D(World, DialogueSound);
        UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Playing dialogue audio: %s"), *AudioPath);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueManager: Failed to load audio: %s"), *AudioPath);
    }
}

bool UNarrativeDialogueManager::IsDialogueAudioPlaying() const
{
    // Simple implementation - in full system would track active audio components
    return false;
}

void UNarrativeDialogueManager::UnlockDialogue(const FString& DialogueID)
{
    if (!UnlockedDialogues.Contains(DialogueID))
    {
        UnlockedDialogues.Add(DialogueID);
        UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Unlocked dialogue: %s"), *DialogueID);
    }
}

bool UNarrativeDialogueManager::IsDialogueUnlocked(const FString& DialogueID) const
{
    return UnlockedDialogues.Contains(DialogueID);
}

float UNarrativeDialogueManager::GetNarrativeProgress() const
{
    float MaxProgress = 10.0f; // Total expected unlocked dialogues
    float CurrentProgress = FMath::Min((float)UnlockedDialogues.Num(), MaxProgress);
    return CurrentProgress / MaxProgress;
}

FNarr_DialogueEntry UNarrativeDialogueManager::CreateDialogueEntry(const FString& ID, const FString& Speaker, const FString& Text, const FString& Audio)
{
    FNarr_DialogueEntry Entry;
    Entry.DialogueID = ID;
    Entry.SpeakerName = Speaker;
    Entry.DialogueText = Text;
    Entry.AudioPath = Audio;
    Entry.RequiredBiome = ENarr_BiomeType::Savana; // Default
    
    return Entry;
}