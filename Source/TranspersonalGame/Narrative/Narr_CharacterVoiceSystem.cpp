#include "Narr_CharacterVoiceSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UNarr_CharacterVoiceSystem::UNarr_CharacterVoiceSystem()
{
    MasterVolume = 1.0f;
    bIsPlaying = false;
    VoiceTimer = 0.0f;
    VoiceAudioComponent = nullptr;
}

void UNarr_CharacterVoiceSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Narr_CharacterVoiceSystem: Initializing voice system"));
    
    // Initialize default character profiles
    InitializeDefaultCharacters();
    
    // Create audio component for voice playback
    if (UWorld* World = GetWorld())
    {
        VoiceAudioComponent = NewObject<UAudioComponent>(this);
        if (VoiceAudioComponent)
        {
            VoiceAudioComponent->SetVolumeMultiplier(MasterVolume);
            VoiceAudioComponent->bAutoActivate = false;
        }
    }
}

void UNarr_CharacterVoiceSystem::Deinitialize()
{
    StopCurrentVoice();
    
    if (VoiceAudioComponent)
    {
        VoiceAudioComponent->Stop();
        VoiceAudioComponent = nullptr;
    }
    
    CharacterProfiles.Empty();
    QueuedVoiceLines.Empty();
    
    Super::Deinitialize();
}

void UNarr_CharacterVoiceSystem::RegisterCharacterProfile(const FNarr_CharacterProfile& Profile)
{
    if (Profile.CharacterName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Narr_CharacterVoiceSystem: Cannot register character with empty name"));
        return;
    }
    
    CharacterProfiles.Add(Profile.CharacterName, Profile);
    UE_LOG(LogTemp, Log, TEXT("Narr_CharacterVoiceSystem: Registered character '%s' with %d voice lines"), 
           *Profile.CharacterName, Profile.VoiceLines.Num());
}

void UNarr_CharacterVoiceSystem::PlayVoiceLine(const FString& CharacterName, const FString& DialogueText, ESurvivalContext Context)
{
    if (CharacterName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Narr_CharacterVoiceSystem: Cannot play voice line for empty character name"));
        return;
    }
    
    // Find character profile
    if (!CharacterProfiles.Contains(CharacterName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Narr_CharacterVoiceSystem: Character '%s' not found"), *CharacterName);
        return;
    }
    
    // Find best matching voice line
    FNarr_VoiceLine VoiceLine = FindBestVoiceLine(CharacterName, Context);
    
    if (!VoiceLine.DialogueText.IsEmpty())
    {
        // Override with specific dialogue if provided
        if (!DialogueText.IsEmpty())
        {
            VoiceLine.DialogueText = DialogueText;
        }
        
        // Add to queue or play immediately
        if (bIsPlaying)
        {
            QueuedVoiceLines.Add(VoiceLine);
            UE_LOG(LogTemp, Log, TEXT("Narr_CharacterVoiceSystem: Queued voice line for '%s'"), *CharacterName);
        }
        else
        {
            CurrentVoiceLine = VoiceLine;
            bIsPlaying = true;
            VoiceTimer = VoiceLine.Duration;
            
            // Start timer for voice line completion
            if (UWorld* World = GetWorld())
            {
                FTimerHandle TimerHandle;
                World->GetTimerManager().SetTimer(TimerHandle, this, &UNarr_CharacterVoiceSystem::OnVoiceLineComplete, VoiceLine.Duration, false);
            }
            
            UE_LOG(LogTemp, Log, TEXT("Narr_CharacterVoiceSystem: Playing voice line for '%s': %s"), 
                   *CharacterName, *VoiceLine.DialogueText);
        }
    }
}

void UNarr_CharacterVoiceSystem::PlayContextualVoice(ESurvivalContext Context, int32 Priority)
{
    // Find all characters with voice lines for this context
    TArray<FNarr_VoiceLine> ContextualLines;
    
    for (const auto& ProfilePair : CharacterProfiles)
    {
        const FNarr_CharacterProfile& Profile = ProfilePair.Value;
        
        for (const FNarr_VoiceLine& VoiceLine : Profile.VoiceLines)
        {
            if (VoiceLine.Context == Context && VoiceLine.Priority >= Priority)
            {
                ContextualLines.Add(VoiceLine);
            }
        }
    }
    
    if (ContextualLines.Num() > 0)
    {
        // Sort by priority and pick the best one
        ContextualLines.Sort([](const FNarr_VoiceLine& A, const FNarr_VoiceLine& B) {
            return A.Priority > B.Priority;
        });
        
        FNarr_VoiceLine SelectedLine = ContextualLines[0];
        PlayVoiceLine(SelectedLine.CharacterName, SelectedLine.DialogueText, Context);
    }
}

void UNarr_CharacterVoiceSystem::StopCurrentVoice()
{
    if (VoiceAudioComponent && VoiceAudioComponent->IsPlaying())
    {
        VoiceAudioComponent->Stop();
    }
    
    bIsPlaying = false;
    VoiceTimer = 0.0f;
    CurrentVoiceLine = FNarr_VoiceLine();
    
    // Process next voice in queue
    ProcessVoiceQueue();
}

bool UNarr_CharacterVoiceSystem::IsVoicePlaying() const
{
    return bIsPlaying;
}

FNarr_CharacterProfile UNarr_CharacterVoiceSystem::GetCharacterProfile(const FString& CharacterName) const
{
    if (CharacterProfiles.Contains(CharacterName))
    {
        return CharacterProfiles[CharacterName];
    }
    
    return FNarr_CharacterProfile();
}

TArray<FString> UNarr_CharacterVoiceSystem::GetRegisteredCharacters() const
{
    TArray<FString> Characters;
    CharacterProfiles.GetKeys(Characters);
    return Characters;
}

void UNarr_CharacterVoiceSystem::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (VoiceAudioComponent)
    {
        VoiceAudioComponent->SetVolumeMultiplier(MasterVolume);
    }
}

float UNarr_CharacterVoiceSystem::GetMasterVolume() const
{
    return MasterVolume;
}

void UNarr_CharacterVoiceSystem::ProcessVoiceQueue()
{
    if (QueuedVoiceLines.Num() > 0 && !bIsPlaying)
    {
        FNarr_VoiceLine NextLine = QueuedVoiceLines[0];
        QueuedVoiceLines.RemoveAt(0);
        
        PlayVoiceLine(NextLine.CharacterName, NextLine.DialogueText, NextLine.Context);
    }
}

void UNarr_CharacterVoiceSystem::OnVoiceLineComplete()
{
    bIsPlaying = false;
    VoiceTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Narr_CharacterVoiceSystem: Voice line completed for '%s'"), *CurrentVoiceLine.CharacterName);
    
    CurrentVoiceLine = FNarr_VoiceLine();
    
    // Process next voice in queue
    ProcessVoiceQueue();
}

void UNarr_CharacterVoiceSystem::InitializeDefaultCharacters()
{
    // Survival Narrator
    FNarr_CharacterProfile SurvivalNarrator;
    SurvivalNarrator.CharacterName = TEXT("SurvivalNarrator");
    SurvivalNarrator.VoiceID = TEXT("narrator_voice");
    SurvivalNarrator.BaseVolume = 0.8f;
    SurvivalNarrator.BasePitch = 1.0f;
    
    // Add voice lines for Survival Narrator
    FNarr_VoiceLine ExplorationLine;
    ExplorationLine.CharacterName = TEXT("SurvivalNarrator");
    ExplorationLine.DialogueText = TEXT("Day 156 in the Cretaceous wilderness. The morning mist reveals fresh Carnotaurus tracks near the eastern ravine.");
    ExplorationLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778406660126_SurvivalNarrator.mp3");
    ExplorationLine.Context = ESurvivalContext::Exploration;
    ExplorationLine.Duration = 24.0f;
    ExplorationLine.Priority = 7;
    SurvivalNarrator.VoiceLines.Add(ExplorationLine);
    
    RegisterCharacterProfile(SurvivalNarrator);
    
    // Emergency Alert Character
    FNarr_CharacterProfile EmergencyAlert;
    EmergencyAlert.CharacterName = TEXT("EmergencyAlert");
    EmergencyAlert.VoiceID = TEXT("alert_voice");
    EmergencyAlert.BaseVolume = 1.0f;
    EmergencyAlert.BasePitch = 1.1f;
    
    FNarr_VoiceLine DangerLine;
    DangerLine.CharacterName = TEXT("EmergencyAlert");
    DangerLine.DialogueText = TEXT("Critical alert! Massive Triceratops herd approaching from the northwestern forest.");
    DangerLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778406668527_EmergencyAlert.mp3");
    DangerLine.Context = ESurvivalContext::Danger;
    DangerLine.Duration = 23.0f;
    DangerLine.Priority = 10;
    EmergencyAlert.VoiceLines.Add(DangerLine);
    
    RegisterCharacterProfile(EmergencyAlert);
    
    // Marcus the Geologist
    FNarr_CharacterProfile MarcusGeologist;
    MarcusGeologist.CharacterName = TEXT("MarcusGeologist");
    MarcusGeologist.VoiceID = TEXT("marcus_voice");
    MarcusGeologist.BaseVolume = 0.9f;
    MarcusGeologist.BasePitch = 0.95f;
    
    FNarr_VoiceLine KnowledgeLine;
    KnowledgeLine.CharacterName = TEXT("MarcusGeologist");
    KnowledgeLine.DialogueText = TEXT("Listen carefully, newcomer. The desert holds ancient secrets buried in stone and sand.");
    KnowledgeLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778406676652_MarcusGeologist.mp3");
    KnowledgeLine.Context = ESurvivalContext::Discovery;
    KnowledgeLine.Duration = 23.0f;
    KnowledgeLine.Priority = 6;
    MarcusGeologist.VoiceLines.Add(KnowledgeLine);
    
    RegisterCharacterProfile(MarcusGeologist);
    
    // Hazard Alert System
    FNarr_CharacterProfile HazardAlert;
    HazardAlert.CharacterName = TEXT("HazardAlert");
    HazardAlert.VoiceID = TEXT("hazard_voice");
    HazardAlert.BaseVolume = 1.0f;
    HazardAlert.BasePitch = 1.05f;
    
    FNarr_VoiceLine EnvironmentalLine;
    EnvironmentalLine.CharacterName = TEXT("HazardAlert");
    EnvironmentalLine.DialogueText = TEXT("Environmental hazard detected! Volcanic ash cloud approaching from the southern mountains.");
    EnvironmentalLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778406684173_HazardAlert.mp3");
    EnvironmentalLine.Context = ESurvivalContext::Environmental;
    EnvironmentalLine.Duration = 22.0f;
    EnvironmentalLine.Priority = 9;
    HazardAlert.VoiceLines.Add(EnvironmentalLine);
    
    RegisterCharacterProfile(HazardAlert);
    
    UE_LOG(LogTemp, Warning, TEXT("Narr_CharacterVoiceSystem: Initialized %d default characters"), CharacterProfiles.Num());
}

FNarr_VoiceLine UNarr_CharacterVoiceSystem::FindBestVoiceLine(const FString& CharacterName, ESurvivalContext Context) const
{
    if (!CharacterProfiles.Contains(CharacterName))
    {
        return FNarr_VoiceLine();
    }
    
    const FNarr_CharacterProfile& Profile = CharacterProfiles[CharacterName];
    
    // Find exact context match first
    for (const FNarr_VoiceLine& VoiceLine : Profile.VoiceLines)
    {
        if (VoiceLine.Context == Context)
        {
            return VoiceLine;
        }
    }
    
    // If no exact match, return first available voice line
    if (Profile.VoiceLines.Num() > 0)
    {
        return Profile.VoiceLines[0];
    }
    
    return FNarr_VoiceLine();
}