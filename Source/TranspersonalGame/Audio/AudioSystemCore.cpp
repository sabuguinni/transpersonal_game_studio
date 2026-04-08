#include "AudioSystemCore.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"

void UAudioSystemCore::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CurrentAudioState = EAudioState::Exploration;
    CurrentEnvironment = EEnvironmentType::Forest;
    CurrentTensionLevel = 0.0f;

    // Configurações padrão para cada estado
    FAudioStateConfig ExplorationConfig;
    ExplorationConfig.MusicVolume = 0.3f;
    ExplorationConfig.AmbienceVolume = 0.8f;
    ExplorationConfig.SFXVolume = 1.0f;
    ExplorationConfig.LowPassCutoff = 20000.0f;
    ExplorationConfig.ReverbWetness = 0.1f;
    AudioStateConfigs.Add(EAudioState::Exploration, ExplorationConfig);

    FAudioStateConfig TensionConfig;
    TensionConfig.MusicVolume = 0.5f;
    TensionConfig.AmbienceVolume = 0.6f;
    TensionConfig.SFXVolume = 1.0f;
    TensionConfig.LowPassCutoff = 15000.0f;
    TensionConfig.ReverbWetness = 0.2f;
    AudioStateConfigs.Add(EAudioState::Tension, TensionConfig);

    FAudioStateConfig DangerConfig;
    DangerConfig.MusicVolume = 0.7f;
    DangerConfig.AmbienceVolume = 0.4f;
    DangerConfig.SFXVolume = 1.0f;
    DangerConfig.LowPassCutoff = 12000.0f;
    DangerConfig.ReverbWetness = 0.3f;
    AudioStateConfigs.Add(EAudioState::Danger, DangerConfig);

    FAudioStateConfig CombatConfig;
    CombatConfig.MusicVolume = 0.8f;
    CombatConfig.AmbienceVolume = 0.3f;
    CombatConfig.SFXVolume = 1.0f;
    CombatConfig.LowPassCutoff = 18000.0f;
    CombatConfig.ReverbWetness = 0.1f;
    AudioStateConfigs.Add(EAudioState::Combat, CombatConfig);

    FAudioStateConfig SafetyConfig;
    SafetyConfig.MusicVolume = 0.2f;
    SafetyConfig.AmbienceVolume = 0.9f;
    SafetyConfig.SFXVolume = 0.8f;
    SafetyConfig.LowPassCutoff = 20000.0f;
    SafetyConfig.ReverbWetness = 0.05f;
    AudioStateConfigs.Add(EAudioState::Safety, SafetyConfig);

    UE_LOG(LogTemp, Log, TEXT("Audio System Core initialized"));
}

void UAudioSystemCore::Deinitialize()
{
    if (MusicAudioComponent && IsValid(MusicAudioComponent))
    {
        MusicAudioComponent->Stop();
    }
    
    if (AmbienceAudioComponent && IsValid(AmbienceAudioComponent))
    {
        AmbienceAudioComponent->Stop();
    }

    Super::Deinitialize();
}

void UAudioSystemCore::SetAudioState(EAudioState NewState)
{
    if (CurrentAudioState == NewState) return;

    UE_LOG(LogTemp, Log, TEXT("Audio State changed from %d to %d"), (int32)CurrentAudioState, (int32)NewState);
    
    CurrentAudioState = NewState;
    
    if (AudioStateConfigs.Contains(NewState))
    {
        ApplyAudioStateConfig(AudioStateConfigs[NewState]);
    }
    
    UpdateMusicParameters();
    UpdateAmbienceParameters();
}

void UAudioSystemCore::SetEnvironmentType(EEnvironmentType NewEnvironment)
{
    if (CurrentEnvironment == NewEnvironment) return;

    UE_LOG(LogTemp, Log, TEXT("Environment changed from %d to %d"), (int32)CurrentEnvironment, (int32)NewEnvironment);
    
    CurrentEnvironment = NewEnvironment;
    UpdateAmbienceParameters();
}

void UAudioSystemCore::SetTensionLevel(float TensionLevel)
{
    CurrentTensionLevel = FMath::Clamp(TensionLevel, 0.0f, 1.0f);
    
    // Transição automática de estados baseada na tensão
    if (CurrentTensionLevel < 0.2f)
    {
        SetAudioState(EAudioState::Exploration);
    }
    else if (CurrentTensionLevel < 0.5f)
    {
        SetAudioState(EAudioState::Tension);
    }
    else if (CurrentTensionLevel < 0.8f)
    {
        SetAudioState(EAudioState::Danger);
    }
    else
    {
        SetAudioState(EAudioState::Combat);
    }
    
    UpdateMusicParameters();
}

void UAudioSystemCore::PlayAdaptiveMusic(UMetaSoundSource* MusicMetaSound)
{
    if (!MusicMetaSound) return;

    UWorld* World = GetWorld();
    if (!World) return;

    if (MusicAudioComponent && IsValid(MusicAudioComponent))
    {
        MusicAudioComponent->Stop();
    }

    MusicAudioComponent = UGameplayStatics::SpawnSound2D(World, MusicMetaSound);
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetSoundClass(MusicSoundClass);
        UpdateMusicParameters();
    }
}

void UAudioSystemCore::StopAdaptiveMusic(float FadeTime)
{
    if (MusicAudioComponent && IsValid(MusicAudioComponent))
    {
        MusicAudioComponent->FadeOut(FadeTime, 0.0f);
    }
}

void UAudioSystemCore::UpdateAmbienceForLocation(FVector Location)
{
    // Esta função será expandida para analisar o ambiente ao redor da localização
    // e ajustar a ambiência sonora dinamicamente
    UpdateAmbienceParameters();
}

void UAudioSystemCore::PlayDinosaurSound(class ADinosaurBase* Dinosaur, FName SoundType)
{
    // Implementação será expandida quando o sistema de dinossauros estiver disponível
    UE_LOG(LogTemp, Log, TEXT("Dinosaur sound requested: %s"), *SoundType.ToString());
}

void UAudioSystemCore::UpdateAcousticEnvironment(FVector ListenerLocation)
{
    // Sistema de reverb e oclusão dinâmico baseado na geometria do ambiente
    // Será implementado com ray tracing para detectar espaços fechados vs abertos
}

void UAudioSystemCore::ApplyAudioStateConfig(const FAudioStateConfig& Config)
{
    if (!DynamicSoundMix) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Aplicar configurações de volume por classe de som
    UGameplayStatics::SetSoundClassVolume(World, MusicSoundClass, Config.MusicVolume);
    UGameplayStatics::SetSoundClassVolume(World, AmbienceSoundClass, Config.AmbienceVolume);
    UGameplayStatics::SetSoundClassVolume(World, SFXSoundClass, Config.SFXVolume);

    // Aplicar mix de som dinâmico
    UGameplayStatics::PushSoundMixModifier(World, DynamicSoundMix);
}

void UAudioSystemCore::UpdateMusicParameters()
{
    if (!MusicAudioComponent || !IsValid(MusicAudioComponent)) return;

    // Atualizar parâmetros do MetaSound baseado no estado atual
    MusicAudioComponent->SetFloatParameter(FName("TensionLevel"), CurrentTensionLevel);
    MusicAudioComponent->SetIntParameter(FName("AudioState"), (int32)CurrentAudioState);
    MusicAudioComponent->SetIntParameter(FName("Environment"), (int32)CurrentEnvironment);
}

void UAudioSystemCore::UpdateAmbienceParameters()
{
    if (!AmbienceAudioComponent || !IsValid(AmbienceAudioComponent)) return;

    // Atualizar parâmetros de ambiência
    AmbienceAudioComponent->SetFloatParameter(FName("TensionLevel"), CurrentTensionLevel);
    AmbienceAudioComponent->SetIntParameter(FName("Environment"), (int32)CurrentEnvironment);
}