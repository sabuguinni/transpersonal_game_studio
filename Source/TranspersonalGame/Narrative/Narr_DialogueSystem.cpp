#include "Narr_DialogueSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    bIsPlaying = false;
    bIsPaused = false;
    CurrentDialogueID = "";
    CurrentPlaybackTime = 0.0f;
    TotalDialogueDuration = 0.0f;
    DialogueVolume = 1.0f;
    bSubtitlesEnabled = true;
    bAutoPlayContextualDialogue = true;
    
    CurrentQueueIndex = 0;
    SequenceTimer = 0.0f;
    DelayTimer = 0.0f;
    bProcessingSequence = false;
    
    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueAudioComponent"));
    if (AudioComponent)
    {
        AudioComponent->bAutoActivate = false;
        AudioComponent->SetVolumeMultiplier(DialogueVolume);
    }
}

void UNarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default dialogue entries for survival scenarios
    InitializeDefaultDialogues();
    
    UE_LOG(LogTemp, Log, TEXT("Narr_DialogueSystem: Sistema de diálogo inicializado com %d entradas"), DialogueDatabase.Num());
}

void UNarr_DialogueSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bProcessingSequence)
    {
        ProcessDialogueQueue();
    }
    
    if (bIsPlaying && !bIsPaused)
    {
        CurrentPlaybackTime += DeltaTime;
        
        // Check if current dialogue has finished
        if (CurrentPlaybackTime >= TotalDialogueDuration)
        {
            OnDialogueFinished();
        }
    }
}

void UNarr_DialogueSystem::PlayDialogue(const FString& DialogueID)
{
    if (DialogueDatabase.Contains(DialogueID))
    {
        const FNarr_DialogueEntry& DialogueEntry = DialogueDatabase[DialogueID];
        
        // Stop current dialogue if playing
        if (bIsPlaying)
        {
            StopCurrentDialogue();
        }
        
        // Set current dialogue state
        CurrentDialogueID = DialogueID;
        CurrentPlaybackTime = 0.0f;
        TotalDialogueDuration = DialogueEntry.Duration;
        bIsPlaying = true;
        bIsPaused = false;
        
        // Load and play audio if available
        if (!DialogueEntry.AudioFilePath.IsEmpty())
        {
            LoadDialogueAudio(DialogueEntry.AudioFilePath);
        }
        
        // Display subtitles if enabled
        if (bSubtitlesEnabled)
        {
            if (GEngine)
            {
                FString SubtitleText = FString::Printf(TEXT("[%s]: %s"), 
                    *UEnum::GetValueAsString(DialogueEntry.Speaker), 
                    *DialogueEntry.DialogueText);
                GEngine->AddOnScreenDebugMessage(-1, TotalDialogueDuration, FColor::White, SubtitleText);
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("Narr_DialogueSystem: A reproduzir diálogo '%s'"), *DialogueID);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Narr_DialogueSystem: Diálogo '%s' não encontrado na base de dados"), *DialogueID);
    }
}

void UNarr_DialogueSystem::PlayDialogueSequence(const FString& SequenceID)
{
    if (SequenceDatabase.Contains(SequenceID))
    {
        const FNarr_DialogueSequence& Sequence = SequenceDatabase[SequenceID];
        
        // Stop current playback
        StopCurrentDialogue();
        
        // Setup sequence playback
        DialogueQueue = Sequence.DialogueEntries;
        CurrentQueueIndex = 0;
        DelayTimer = 0.0f;
        bProcessingSequence = true;
        
        // Start first dialogue if auto-play is enabled
        if (Sequence.bAutoPlay && DialogueQueue.Num() > 0)
        {
            PlayDialogue(DialogueQueue[0].DialogueID);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Narr_DialogueSystem: A iniciar sequência '%s' com %d diálogos"), *SequenceID, DialogueQueue.Num());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Narr_DialogueSystem: Sequência '%s' não encontrada"), *SequenceID);
    }
}

void UNarr_DialogueSystem::StopCurrentDialogue()
{
    if (bIsPlaying)
    {
        bIsPlaying = false;
        bIsPaused = false;
        bProcessingSequence = false;
        CurrentDialogueID = "";
        CurrentPlaybackTime = 0.0f;
        TotalDialogueDuration = 0.0f;
        
        if (AudioComponent && AudioComponent->IsPlaying())
        {
            AudioComponent->Stop();
        }
        
        DialogueQueue.Empty();
        CurrentQueueIndex = 0;
        
        UE_LOG(LogTemp, Log, TEXT("Narr_DialogueSystem: Diálogo parado"));
    }
}

void UNarr_DialogueSystem::PauseDialogue()
{
    if (bIsPlaying && !bIsPaused)
    {
        bIsPaused = true;
        
        if (AudioComponent && AudioComponent->IsPlaying())
        {
            AudioComponent->SetPaused(true);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Narr_DialogueSystem: Diálogo pausado"));
    }
}

void UNarr_DialogueSystem::ResumeDialogue()
{
    if (bIsPlaying && bIsPaused)
    {
        bIsPaused = false;
        
        if (AudioComponent)
        {
            AudioComponent->SetPaused(false);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Narr_DialogueSystem: Diálogo retomado"));
    }
}

void UNarr_DialogueSystem::TriggerBiomeDialogue(EEng_BiomeType BiomeType)
{
    if (!bAutoPlayContextualDialogue) return;
    
    // Find appropriate biome dialogue
    for (const auto& DialoguePair : DialogueDatabase)
    {
        const FNarr_DialogueEntry& Entry = DialoguePair.Value;
        if (Entry.DialogueType == ENarr_DialogueType::BiomeDescription && 
            Entry.RelevantBiome == BiomeType)
        {
            PlayDialogue(Entry.DialogueID);
            break;
        }
    }
}

void UNarr_DialogueSystem::TriggerThreatDialogue(EEng_ThreatLevel ThreatLevel, EEng_DinosaurSpecies DinosaurSpecies)
{
    if (!bAutoPlayContextualDialogue) return;
    
    // Find appropriate threat dialogue based on urgency
    for (const auto& DialoguePair : DialogueDatabase)
    {
        const FNarr_DialogueEntry& Entry = DialoguePair.Value;
        if (Entry.DialogueType == ENarr_DialogueType::ThreatAlert && 
            Entry.UrgencyLevel == ThreatLevel)
        {
            PlayDialogue(Entry.DialogueID);
            break;
        }
    }
}

void UNarr_DialogueSystem::TriggerWeatherDialogue(EEng_WeatherType WeatherType)
{
    if (!bAutoPlayContextualDialogue) return;
    
    // Find weather-related dialogue
    for (const auto& DialoguePair : DialogueDatabase)
    {
        const FNarr_DialogueEntry& Entry = DialoguePair.Value;
        if (Entry.DialogueType == ENarr_DialogueType::WeatherWarning)
        {
            // Check if this weather dialogue is relevant
            bool bRelevant = false;
            for (const FString& Condition : Entry.TriggerConditions)
            {
                if (Condition.Contains(UEnum::GetValueAsString(WeatherType)))
                {
                    bRelevant = true;
                    break;
                }
            }
            
            if (bRelevant)
            {
                PlayDialogue(Entry.DialogueID);
                break;
            }
        }
    }
}

void UNarr_DialogueSystem::TriggerDiscoveryDialogue(const FString& DiscoveryType)
{
    if (!bAutoPlayContextualDialogue) return;
    
    // Find discovery dialogue
    for (const auto& DialoguePair : DialogueDatabase)
    {
        const FNarr_DialogueEntry& Entry = DialoguePair.Value;
        if (Entry.DialogueType == ENarr_DialogueType::Discovery)
        {
            for (const FString& Condition : Entry.TriggerConditions)
            {
                if (Condition.Contains(DiscoveryType))
                {
                    PlayDialogue(Entry.DialogueID);
                    return;
                }
            }
        }
    }
}

void UNarr_DialogueSystem::RegisterDialogueEntry(const FNarr_DialogueEntry& DialogueEntry)
{
    DialogueDatabase.Add(DialogueEntry.DialogueID, DialogueEntry);
    UE_LOG(LogTemp, Log, TEXT("Narr_DialogueSystem: Diálogo '%s' registado"), *DialogueEntry.DialogueID);
}

void UNarr_DialogueSystem::RegisterDialogueSequence(const FNarr_DialogueSequence& DialogueSequence)
{
    SequenceDatabase.Add(DialogueSequence.SequenceID, DialogueSequence);
    UE_LOG(LogTemp, Log, TEXT("Narr_DialogueSystem: Sequência '%s' registada"), *DialogueSequence.SequenceID);
}

bool UNarr_DialogueSystem::IsDialoguePlaying() const
{
    return bIsPlaying;
}

FString UNarr_DialogueSystem::GetCurrentDialogueID() const
{
    return CurrentDialogueID;
}

void UNarr_DialogueSystem::SetAudioVolume(float Volume)
{
    DialogueVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (AudioComponent)
    {
        AudioComponent->SetVolumeMultiplier(DialogueVolume);
    }
}

void UNarr_DialogueSystem::SetSubtitlesEnabled(bool bEnabled)
{
    bSubtitlesEnabled = bEnabled;
}

void UNarr_DialogueSystem::ProcessDialogueQueue()
{
    if (DialogueQueue.Num() == 0 || CurrentQueueIndex >= DialogueQueue.Num())
    {
        bProcessingSequence = false;
        return;
    }
    
    // Wait for current dialogue to finish before starting next
    if (bIsPlaying)
    {
        return;
    }
    
    // Handle delay between dialogues
    if (DelayTimer > 0.0f)
    {
        DelayTimer -= GetWorld()->GetDeltaSeconds();
        return;
    }
    
    // Play next dialogue in queue
    const FNarr_DialogueEntry& NextDialogue = DialogueQueue[CurrentQueueIndex];
    PlayDialogue(NextDialogue.DialogueID);
    
    CurrentQueueIndex++;
    
    // Set delay for next dialogue if there are more in queue
    if (CurrentQueueIndex < DialogueQueue.Num())
    {
        DelayTimer = 2.0f; // Default delay between dialogues
    }
    else
    {
        bProcessingSequence = false;
    }
}

void UNarr_DialogueSystem::OnDialogueFinished()
{
    bIsPlaying = false;
    CurrentDialogueID = "";
    CurrentPlaybackTime = 0.0f;
    TotalDialogueDuration = 0.0f;
    
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->Stop();
    }
}

bool UNarr_DialogueSystem::CheckDialogueTriggerConditions(const FNarr_DialogueEntry& DialogueEntry)
{
    // Simple condition checking - can be expanded for more complex logic
    return true;
}

void UNarr_DialogueSystem::LoadDialogueAudio(const FString& AudioFilePath)
{
    if (AudioComponent && !AudioFilePath.IsEmpty())
    {
        // Try to load sound wave from path
        USoundWave* SoundWave = LoadObject<USoundWave>(nullptr, *AudioFilePath);
        if (SoundWave)
        {
            AudioComponent->SetSound(SoundWave);
            AudioComponent->Play();
            UE_LOG(LogTemp, Log, TEXT("Narr_DialogueSystem: Áudio carregado: %s"), *AudioFilePath);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Narr_DialogueSystem: Falha ao carregar áudio: %s"), *AudioFilePath);
        }
    }
}

void UNarr_DialogueSystem::InitializeDefaultDialogues()
{
    // Create default survival-focused dialogue entries
    
    // Field Notes
    FNarr_DialogueEntry FieldNote1;
    FieldNote1.DialogueID = "FieldNote_RaptorBehavior";
    FieldNote1.DialogueType = ENarr_DialogueType::FieldNotes;
    FieldNote1.Speaker = ENarr_SpeakerType::Paleontologist;
    FieldNote1.DialogueText = "Registo de campo: Os Raptors desta região demonstram comportamento de caça coordenada excepcional.";
    FieldNote1.Duration = 8.0f;
    FieldNote1.UrgencyLevel = EEng_ThreatLevel::Safe;
    RegisterDialogueEntry(FieldNote1);
    
    // Safety Alerts
    FNarr_DialogueEntry SafetyAlert1;
    SafetyAlert1.DialogueID = "Safety_TRexProximity";
    SafetyAlert1.DialogueType = ENarr_DialogueType::SafetyAlert;
    SafetyAlert1.Speaker = ENarr_SpeakerType::SafetyOfficer;
    SafetyAlert1.DialogueText = "Alerta de segurança: Tyrannosaurus Rex detectado na área. Manter distância segura.";
    SafetyAlert1.Duration = 6.0f;
    SafetyAlert1.UrgencyLevel = EEng_ThreatLevel::Deadly;
    RegisterDialogueEntry(SafetyAlert1);
    
    // Weather Warnings
    FNarr_DialogueEntry WeatherWarning1;
    WeatherWarning1.DialogueID = "Weather_StormApproaching";
    WeatherWarning1.DialogueType = ENarr_DialogueType::WeatherWarning;
    WeatherWarning1.Speaker = ENarr_SpeakerType::WeatherStation;
    WeatherWarning1.DialogueText = "Aviso meteorológico: Tempestade tropical aproxima-se. Procurar abrigo imediatamente.";
    WeatherWarning1.Duration = 7.0f;
    WeatherWarning1.UrgencyLevel = EEng_ThreatLevel::Dangerous;
    WeatherWarning1.TriggerConditions.Add("Storm");
    WeatherWarning1.TriggerConditions.Add("Rain");
    RegisterDialogueEntry(WeatherWarning1);
    
    // Discovery Dialogues
    FNarr_DialogueEntry Discovery1;
    Discovery1.DialogueID = "Discovery_DinosaurNest";
    Discovery1.DialogueType = ENarr_DialogueType::Discovery;
    Discovery1.Speaker = ENarr_SpeakerType::Paleontologist;
    Discovery1.DialogueText = "Descoberta extraordinária: Ninho de dinossauro intacto com evidências de comportamento parental.";
    Discovery1.Duration = 9.0f;
    Discovery1.UrgencyLevel = EEng_ThreatLevel::Safe;
    Discovery1.TriggerConditions.Add("Nest");
    Discovery1.TriggerConditions.Add("Eggs");
    RegisterDialogueEntry(Discovery1);
    
    UE_LOG(LogTemp, Log, TEXT("Narr_DialogueSystem: %d diálogos padrão inicializados"), DialogueDatabase.Num());
}