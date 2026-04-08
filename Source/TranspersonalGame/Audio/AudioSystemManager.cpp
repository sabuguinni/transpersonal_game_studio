#include "AudioSystemManager.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "MetasoundSource.h"

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Audio System Manager: Initializing..."));
    
    // Inicializar estado padrão
    CurrentAudioState.CurrentState = EAudioState::Calm;
    CurrentAudioState.StateIntensity = 0.0f;
    CurrentAudioState.Environment = EEnvironmentType::Forest;
    CurrentAudioState.ThreatLevel = EThreatLevel::None;
    
    InitializeAudioComponents();
    
    UE_LOG(LogTemp, Log, TEXT("Audio System Manager: Initialized successfully"));
}

void UAudioSystemManager::Deinitialize()
{
    if (MusicAudioComponent && MusicAudioComponent->IsValidLowLevel())
    {
        MusicAudioComponent->Stop();
    }
    
    if (AmbienceAudioComponent && AmbienceAudioComponent->IsValidLowLevel())
    {
        AmbienceAudioComponent->Stop();
    }
    
    Super::Deinitialize();
}

void UAudioSystemManager::InitializeAudioComponents()
{
    if (UWorld* World = GetWorld())
    {
        // Criar componente de música
        MusicAudioComponent = NewObject<UAudioComponent>(this);
        if (MusicAudioComponent)
        {
            MusicAudioComponent->SetVolumeMultiplier(MusicVolume);
            MusicAudioComponent->bAutoActivate = false;
        }
        
        // Criar componente de ambiente
        AmbienceAudioComponent = NewObject<UAudioComponent>(this);
        if (AmbienceAudioComponent)
        {
            AmbienceAudioComponent->SetVolumeMultiplier(AmbienceVolume);
            AmbienceAudioComponent->bAutoActivate = false;
        }
    }
}

void UAudioSystemManager::SetAudioState(EAudioState NewState, float Intensity)
{
    if (CurrentAudioState.CurrentState != NewState)
    {
        UE_LOG(LogTemp, Log, TEXT("Audio System: Transitioning from %d to %d with intensity %.2f"), 
               (int32)CurrentAudioState.CurrentState, (int32)NewState, Intensity);
        
        CurrentAudioState.CurrentState = NewState;
        CurrentAudioState.StateIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
        
        UpdateMusicParameters();
    }
}

void UAudioSystemManager::SetEnvironmentType(EEnvironmentType NewEnvironment)
{
    if (CurrentAudioState.Environment != NewEnvironment)
    {
        UE_LOG(LogTemp, Log, TEXT("Audio System: Environment changed to %d"), (int32)NewEnvironment);
        
        CurrentAudioState.Environment = NewEnvironment;
        UpdateAmbienceParameters();
    }
}

void UAudioSystemManager::SetThreatLevel(EThreatLevel NewThreatLevel)
{
    if (CurrentAudioState.ThreatLevel != NewThreatLevel)
    {
        UE_LOG(LogTemp, Log, TEXT("Audio System: Threat level changed to %d"), (int32)NewThreatLevel);
        
        CurrentAudioState.ThreatLevel = NewThreatLevel;
        
        // Ajustar estado baseado no nível de ameaça
        switch (NewThreatLevel)
        {
            case EThreatLevel::None:
                SetAudioState(EAudioState::Calm, 0.2f);
                break;
            case EThreatLevel::Low:
                SetAudioState(EAudioState::Tension, 0.4f);
                break;
            case EThreatLevel::Medium:
                SetAudioState(EAudioState::Danger, 0.6f);
                break;
            case EThreatLevel::High:
                SetAudioState(EAudioState::Panic, 0.8f);
                break;
            case EThreatLevel::Extreme:
                SetAudioState(EAudioState::Panic, 1.0f);
                break;
        }
    }
}

void UAudioSystemManager::UpdateAudioSystem(float DeltaTime)
{
    // Atualizar transições suaves entre estados
    UpdateMusicParameters();
    UpdateAmbienceParameters();
}

void UAudioSystemManager::UpdateMusicParameters()
{
    if (!MusicAudioComponent || !AdaptiveMusicMetaSound)
        return;
    
    // Definir parâmetros do MetaSound baseados no estado atual
    float TensionLevel = 0.0f;
    float RhythmIntensity = 0.0f;
    float HarmonyComplexity = 0.0f;
    
    switch (CurrentAudioState.CurrentState)
    {
        case EAudioState::Calm:
            TensionLevel = 0.1f;
            RhythmIntensity = 0.2f;
            HarmonyComplexity = 0.3f;
            break;
        case EAudioState::Tension:
            TensionLevel = 0.4f;
            RhythmIntensity = 0.5f;
            HarmonyComplexity = 0.6f;
            break;
        case EAudioState::Danger:
            TensionLevel = 0.7f;
            RhythmIntensity = 0.8f;
            HarmonyComplexity = 0.4f;
            break;
        case EAudioState::Panic:
            TensionLevel = 1.0f;
            RhythmIntensity = 1.0f;
            HarmonyComplexity = 0.2f;
            break;
        case EAudioState::Stealth:
            TensionLevel = 0.6f;
            RhythmIntensity = 0.1f;
            HarmonyComplexity = 0.8f;
            break;
    }
    
    // Aplicar intensidade do estado
    TensionLevel *= CurrentAudioState.StateIntensity;
    RhythmIntensity *= CurrentAudioState.StateIntensity;
    
    // Enviar parâmetros para o MetaSound
    MusicAudioComponent->SetFloatParameter(FName("TensionLevel"), TensionLevel);
    MusicAudioComponent->SetFloatParameter(FName("RhythmIntensity"), RhythmIntensity);
    MusicAudioComponent->SetFloatParameter(FName("HarmonyComplexity"), HarmonyComplexity);
}

void UAudioSystemManager::UpdateAmbienceParameters()
{
    if (!AmbienceAudioComponent)
        return;
    
    // Parâmetros baseados no ambiente
    float ForestDensity = 0.0f;
    float WaterPresence = 0.0f;
    float WindIntensity = 0.5f;
    
    switch (CurrentAudioState.Environment)
    {
        case EEnvironmentType::Forest:
            ForestDensity = 1.0f;
            WaterPresence = 0.1f;
            WindIntensity = 0.3f;
            break;
        case EEnvironmentType::Plains:
            ForestDensity = 0.2f;
            WaterPresence = 0.0f;
            WindIntensity = 0.8f;
            break;
        case EEnvironmentType::Swamp:
            ForestDensity = 0.6f;
            WaterPresence = 0.9f;
            WindIntensity = 0.2f;
            break;
        case EEnvironmentType::Cave:
            ForestDensity = 0.0f;
            WaterPresence = 0.3f;
            WindIntensity = 0.1f;
            break;
        case EEnvironmentType::River:
            ForestDensity = 0.4f;
            WaterPresence = 1.0f;
            WindIntensity = 0.4f;
            break;
    }
    
    AmbienceAudioComponent->SetFloatParameter(FName("ForestDensity"), ForestDensity);
    AmbienceAudioComponent->SetFloatParameter(FName("WaterPresence"), WaterPresence);
    AmbienceAudioComponent->SetFloatParameter(FName("WindIntensity"), WindIntensity);
}

void UAudioSystemManager::PlayAdaptiveMusic()
{
    if (MusicAudioComponent && AdaptiveMusicMetaSound)
    {
        MusicAudioComponent->SetSound(AdaptiveMusicMetaSound);
        MusicAudioComponent->Play();
        UE_LOG(LogTemp, Log, TEXT("Audio System: Adaptive music started"));
    }
}

void UAudioSystemManager::StopAdaptiveMusic()
{
    if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
    {
        MusicAudioComponent->FadeOut(2.0f, 0.0f);
        UE_LOG(LogTemp, Log, TEXT("Audio System: Adaptive music stopping"));
    }
}

void UAudioSystemManager::PlayDinosaurSound(const FString& DinosaurType, const FVector& Location, float Distance)
{
    // Calcular volume baseado na distância
    float VolumeMultiplier = FMath::Clamp(1.0f - (Distance / 5000.0f), 0.1f, 1.0f);
    
    // Determinar tipo de som baseado na distância e tipo de dinossauro
    FString SoundVariation = "Distant";
    if (Distance < 1000.0f)
    {
        SoundVariation = "Close";
    }
    else if (Distance < 2500.0f)
    {
        SoundVariation = "Medium";
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio System: Playing %s dinosaur sound at distance %.2f"), 
           *DinosaurType, Distance);
    
    // Aqui seria implementada a lógica para tocar o som específico
    // usando UGameplayStatics::PlaySoundAtLocation com o som apropriado
}

void UAudioSystemManager::PlayEnvironmentSound(const FString& SoundType, const FVector& Location)
{
    UE_LOG(LogTemp, Log, TEXT("Audio System: Playing environment sound %s at location"), *SoundType);
    
    // Implementar reprodução de sons ambientais pontuais
    // como galhos quebrando, folhas rustling, etc.
}

void UAudioSystemManager::PlayPlayerActionSound(const FString& ActionType)
{
    UE_LOG(LogTemp, Log, TEXT("Audio System: Playing player action sound %s"), *ActionType);
    
    // Implementar sons de ações do jogador
    // como passos, respiração, heartbeat durante tensão, etc.
}