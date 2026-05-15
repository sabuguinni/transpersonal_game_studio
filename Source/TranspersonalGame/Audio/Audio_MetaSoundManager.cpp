#include "Audio_MetaSoundManager.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundWave.h"
#include "MetasoundSource.h"
#include "AudioDevice.h"
#include "Engine/World.h"
#include "TimerManager.h"

AAudio_MetaSoundManager::AAudio_MetaSoundManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create primary audio component for narrative voice lines
    PrimaryAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("PrimaryAudioComponent"));
    PrimaryAudioComponent->SetupAttachment(RootComponent);
    PrimaryAudioComponent->bAutoActivate = false;
    PrimaryAudioComponent->SetVolumeMultiplier(1.0f);
    
    // Create secondary audio component for overlapping sounds
    SecondaryAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SecondaryAudioComponent"));
    SecondaryAudioComponent->SetupAttachment(RootComponent);
    SecondaryAudioComponent->bAutoActivate = false;
    SecondaryAudioComponent->SetVolumeMultiplier(0.8f);
    
    // Create ambient audio component for environmental sounds
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = true;
    AmbientAudioComponent->SetVolumeMultiplier(0.6f);
    
    // Initialize audio component pool
    AudioComponentPool.Add(PrimaryAudioComponent);
    AudioComponentPool.Add(SecondaryAudioComponent);
    AudioComponentPool.Add(AmbientAudioComponent);
    
    // Set default parameters
    DefaultParams.Volume = 1.0f;
    DefaultParams.Pitch = 1.0f;
    DefaultParams.LowPassFilter = 22000.0f;
    DefaultParams.HighPassFilter = 20.0f;
    DefaultParams.ReverbAmount = 0.1f;
    DefaultParams.bUseDistanceAttenuation = true;
    
    // Initialize state
    bIsPlayingNarrative = false;
    CurrentNarrativeType = EAudio_NarrativeType::None;
    CurrentPlaybackTime = 0.0f;
    NarrativeStartTime = 0.0f;
    CurrentNarrativeDuration = 0.0f;
}

void AAudio_MetaSoundManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultVoiceLines();
    LoadMetaSoundAssets();
    
    // Set up 3D audio settings
    SetSpatialMode(EAudio_SpatialMode::ThreeD_Positional);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager initialized with %d voice lines"), NarrativeVoiceLines.Num());
}

void AAudio_MetaSoundManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update narrative playback time
    if (bIsPlayingNarrative)
    {
        CurrentPlaybackTime += DeltaTime;
        
        // Check if narrative finished
        if (CurrentNarrativeDuration > 0.0f && CurrentPlaybackTime >= CurrentNarrativeDuration)
        {
            OnNarrativeFinished();
        }
    }
}

void AAudio_MetaSoundManager::InitializeDefaultVoiceLines()
{
    // Initialize with voice lines from previous narrative generation
    FAudio_VoiceLine AncientWisdom;
    AncientWisdom.NarrativeType = EAudio_NarrativeType::AncientWisdom;
    AncientWisdom.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778842332378_AncientNarrator.mp3";
    AncientWisdom.Duration = 25.0f;
    AncientWisdom.Priority = 3;
    AncientWisdom.bIs3D = true;
    NarrativeVoiceLines.Add(AncientWisdom);
    
    FAudio_VoiceLine DangerWarning;
    DangerWarning.NarrativeType = EAudio_NarrativeType::DangerWarning;
    DangerWarning.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778842338450_ScoutWarning.mp3";
    DangerWarning.Duration = 18.0f;
    DangerWarning.Priority = 5;
    DangerWarning.bIs3D = true;
    NarrativeVoiceLines.Add(DangerWarning);
    
    FAudio_VoiceLine HuntInstruction;
    HuntInstruction.NarrativeType = EAudio_NarrativeType::HuntInstruction;
    HuntInstruction.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778842343769_HuntLeader.mp3";
    HuntInstruction.Duration = 20.0f;
    HuntInstruction.Priority = 4;
    HuntInstruction.bIs3D = true;
    NarrativeVoiceLines.Add(HuntInstruction);
    
    FAudio_VoiceLine SurvivalTip;
    SurvivalTip.NarrativeType = EAudio_NarrativeType::SurvivalTip;
    SurvivalTip.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778842348847_ElderWisdom.mp3";
    SurvivalTip.Duration = 16.0f;
    SurvivalTip.Priority = 2;
    SurvivalTip.bIs3D = false; // 2D UI narration
    NarrativeVoiceLines.Add(SurvivalTip);
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d default voice lines"), NarrativeVoiceLines.Num());
}

void AAudio_MetaSoundManager::LoadMetaSoundAssets()
{
    // Load MetaSound assets for narrative and ambient audio
    // These would be created in the UE5 editor as MetaSound sources
    
    // For now, log that we're ready to load MetaSounds
    UE_LOG(LogTemp, Warning, TEXT("MetaSound assets ready for loading - create MS_Narrative and MS_Ambient in editor"));
}

bool AAudio_MetaSoundManager::PlayNarrativeLine(EAudio_NarrativeType NarrativeType, const FVector& WorldLocation)
{
    // Find the voice line
    FAudio_VoiceLine* VoiceLine = nullptr;
    for (FAudio_VoiceLine& Line : NarrativeVoiceLines)
    {
        if (Line.NarrativeType == NarrativeType)
        {
            VoiceLine = &Line;
            break;
        }
    }
    
    if (!VoiceLine)
    {
        UE_LOG(LogTemp, Warning, TEXT("Voice line not found for narrative type: %d"), (int32)NarrativeType);
        return false;
    }
    
    // Stop current narrative if playing
    if (bIsPlayingNarrative)
    {
        StopNarrativeLine();
    }
    
    // Get available audio component
    UAudioComponent* AudioComp = GetAvailableAudioComponent();
    if (!AudioComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("No available audio component for narrative playback"));
        return false;
    }
    
    // Set 3D position if needed
    if (VoiceLine->bIs3D && WorldLocation != FVector::ZeroVector)
    {
        AudioComp->SetWorldLocation(WorldLocation);
    }
    
    // Apply audio parameters
    AudioComp->SetVolumeMultiplier(DefaultParams.Volume);
    AudioComp->SetPitchMultiplier(DefaultParams.Pitch);
    
    // Start playback (would load SoundWave from URL in full implementation)
    // For now, simulate playback
    bIsPlayingNarrative = true;
    CurrentNarrativeType = NarrativeType;
    CurrentPlaybackTime = 0.0f;
    CurrentNarrativeDuration = VoiceLine->Duration;
    NarrativeStartTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Playing narrative line: %s (Duration: %.1fs)"), 
           *UEnum::GetValueAsString(NarrativeType), VoiceLine->Duration);
    
    return true;
}

void AAudio_MetaSoundManager::StopNarrativeLine()
{
    if (bIsPlayingNarrative)
    {
        // Stop all audio components
        for (UAudioComponent* AudioComp : AudioComponentPool)
        {
            if (AudioComp && AudioComp->IsPlaying())
            {
                AudioComp->Stop();
            }
        }
        
        OnNarrativeFinished();
    }
}

void AAudio_MetaSoundManager::SetMetaSoundParameter(const FString& ParameterName, float Value)
{
    // Set MetaSound parameter on active audio components
    for (UAudioComponent* AudioComp : AudioComponentPool)
    {
        if (AudioComp && AudioComp->IsPlaying())
        {
            // In full implementation, would use MetaSound parameter interface
            UE_LOG(LogTemp, Log, TEXT("Setting MetaSound parameter %s to %.2f"), *ParameterName, Value);
        }
    }
}

void AAudio_MetaSoundManager::SetAudioParams(const FAudio_MetaSoundParams& Params)
{
    DefaultParams = Params;
    
    // Apply to all audio components
    for (UAudioComponent* AudioComp : AudioComponentPool)
    {
        if (AudioComp)
        {
            AudioComp->SetVolumeMultiplier(Params.Volume);
            AudioComp->SetPitchMultiplier(Params.Pitch);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Updated audio parameters - Volume: %.2f, Pitch: %.2f"), 
           Params.Volume, Params.Pitch);
}

void AAudio_MetaSoundManager::RegisterVoiceLine(const FAudio_VoiceLine& VoiceLine)
{
    // Check if voice line already exists
    for (int32 i = 0; i < NarrativeVoiceLines.Num(); i++)
    {
        if (NarrativeVoiceLines[i].NarrativeType == VoiceLine.NarrativeType)
        {
            // Update existing voice line
            NarrativeVoiceLines[i] = VoiceLine;
            UE_LOG(LogTemp, Warning, TEXT("Updated existing voice line for type: %d"), (int32)VoiceLine.NarrativeType);
            return;
        }
    }
    
    // Add new voice line
    NarrativeVoiceLines.Add(VoiceLine);
    UE_LOG(LogTemp, Warning, TEXT("Registered new voice line for type: %d"), (int32)VoiceLine.NarrativeType);
}

FAudio_VoiceLine AAudio_MetaSoundManager::GetVoiceLineByType(EAudio_NarrativeType NarrativeType) const
{
    for (const FAudio_VoiceLine& Line : NarrativeVoiceLines)
    {
        if (Line.NarrativeType == NarrativeType)
        {
            return Line;
        }
    }
    
    return FAudio_VoiceLine(); // Return default if not found
}

void AAudio_MetaSoundManager::SetSpatialMode(EAudio_SpatialMode SpatialMode)
{
    for (UAudioComponent* AudioComp : AudioComponentPool)
    {
        if (AudioComp)
        {
            switch (SpatialMode)
            {
                case EAudio_SpatialMode::TwoD:
                    AudioComp->bAllowSpatialization = false;
                    break;
                case EAudio_SpatialMode::ThreeD_Positional:
                    AudioComp->bAllowSpatialization = true;
                    AudioComp->bOverrideAttenuation = false;
                    break;
                case EAudio_SpatialMode::ThreeD_Ambient:
                    AudioComp->bAllowSpatialization = true;
                    AudioComp->bIsUISound = false;
                    break;
                case EAudio_SpatialMode::ThreeD_Directional:
                    AudioComp->bAllowSpatialization = true;
                    AudioComp->bOverrideAttenuation = true;
                    break;
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Set spatial mode to: %d"), (int32)SpatialMode);
}

void AAudio_MetaSoundManager::UpdateListenerPosition(const FVector& ListenerLocation, const FVector& ListenerForward)
{
    // Update 3D audio listener position for spatial audio
    if (GetWorld() && GetWorld()->GetAudioDevice())
    {
        // In full implementation, would update audio device listener transform
        UE_LOG(LogTemp, VeryVerbose, TEXT("Updated audio listener position: %s"), *ListenerLocation.ToString());
    }
}

UAudioComponent* AAudio_MetaSoundManager::GetAvailableAudioComponent()
{
    // Find first available (not playing) audio component
    for (UAudioComponent* AudioComp : AudioComponentPool)
    {
        if (AudioComp && !AudioComp->IsPlaying())
        {
            return AudioComp;
        }
    }
    
    // If all busy, return primary component (will interrupt)
    return PrimaryAudioComponent;
}

void AAudio_MetaSoundManager::OnNarrativeFinished()
{
    bIsPlayingNarrative = false;
    CurrentNarrativeType = EAudio_NarrativeType::None;
    CurrentPlaybackTime = 0.0f;
    CurrentNarrativeDuration = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Narrative playback finished"));
}