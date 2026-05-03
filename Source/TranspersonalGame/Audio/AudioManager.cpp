#include "AudioManager.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "AudioDevice.h"

UAudioManager::UAudioManager()
{
    CurrentBiome = EAudio_BiomeAmbience::Savanna;
    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    ListenerLocation = FVector::ZeroVector;
    MasterVolume = 1.0f;
    AmbienceVolume = 0.7f;
    SFXVolume = 0.8f;
}

void UAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Inicialização do sistema de áudio"));
    
    // Inicializar configurações de biomas
    InitializeBiomeSettings();
    
    // Inicializar configurações de threat levels
    InitializeThreatSettings();
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Sistema de áudio inicializado com sucesso"));
}

void UAudioManager::Deinitialize()
{
    // Limpar componentes de áudio
    if (AmbienceAudioComponent && IsValid(AmbienceAudioComponent))
    {
        AmbienceAudioComponent->Stop();
        AmbienceAudioComponent = nullptr;
    }
    
    if (MusicAudioComponent && IsValid(MusicAudioComponent))
    {
        MusicAudioComponent->Stop();
        MusicAudioComponent = nullptr;
    }
    
    if (HeartbeatAudioComponent && IsValid(HeartbeatAudioComponent))
    {
        HeartbeatAudioComponent->Stop();
        HeartbeatAudioComponent = nullptr;
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Sistema de áudio desligado"));
    
    Super::Deinitialize();
}

// ═══════════════════════════════════════════════════════════════
// BIOME AUDIO MANAGEMENT
// ═══════════════════════════════════════════════════════════════

void UAudioManager::SetCurrentBiome(EAudio_BiomeAmbience NewBiome)
{
    if (CurrentBiome != NewBiome)
    {
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Transição de bioma de %d para %d"), 
               (int32)CurrentBiome, (int32)NewBiome);
        
        TransitionToNewBiome(NewBiome);
        CurrentBiome = NewBiome;
    }
}

void UAudioManager::UpdateBiomeAmbience(const FVector& PlayerLocation)
{
    // Determinar bioma baseado na localização
    EAudio_BiomeAmbience DetectedBiome = DetermineBiomeFromLocation(PlayerLocation);
    
    // Actualizar se mudou
    if (DetectedBiome != CurrentBiome)
    {
        SetCurrentBiome(DetectedBiome);
    }
    
    // Actualizar posição do listener
    SetListenerLocation(PlayerLocation);
}

// ═══════════════════════════════════════════════════════════════
// THREAT LEVEL AUDIO
// ═══════════════════════════════════════════════════════════════

void UAudioManager::SetThreatLevel(EAudio_ThreatLevel NewThreatLevel)
{
    if (CurrentThreatLevel != NewThreatLevel)
    {
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Threat level mudou para %d"), (int32)NewThreatLevel);
        
        CurrentThreatLevel = NewThreatLevel;
        
        // Aplicar configurações de áudio para o novo threat level
        if (ThreatAudioSettings.IsValidIndex((int32)NewThreatLevel))
        {
            const FAudio_ThreatAudio& ThreatSettings = ThreatAudioSettings[(int32)NewThreatLevel];
            
            // Actualizar música de tensão
            if (MusicAudioComponent && ThreatSettings.ThreatMusic.IsValid())
            {
                MusicAudioComponent->SetVolumeMultiplier(ThreatSettings.MusicVolume * MasterVolume);
            }
            
            // Actualizar heartbeat
            if (HeartbeatAudioComponent)
            {
                HeartbeatAudioComponent->SetVolumeMultiplier(ThreatSettings.HeartbeatIntensity * MasterVolume);
            }
        }
    }
}

void UAudioManager::PlayDinosaurFootsteps(const FVector& DinosaurLocation, float DinosaurMass)
{
    // Calcular volume baseado na massa e distância
    float Distance = FVector::Dist(ListenerLocation, DinosaurLocation);
    float VolumeMultiplier = FMath::Clamp(DinosaurMass / 5000.0f, 0.1f, 2.0f); // Normalizar massa
    float DistanceAttenuation = FMath::Clamp(1.0f - (Distance / 10000.0f), 0.0f, 1.0f);
    
    float FinalVolume = VolumeMultiplier * DistanceAttenuation * SFXVolume * MasterVolume;
    
    if (FinalVolume > 0.05f) // Só tocar se audível
    {
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Footsteps de dinossauro (massa=%.1f) a %.1f metros, volume=%.2f"), 
               DinosaurMass, Distance, FinalVolume);
        
        // TODO: Carregar e tocar som de footsteps baseado no tipo de terreno
        // PlaySpatialSound(FootstepsSound, DinosaurLocation, FinalVolume);
    }
}

// ═══════════════════════════════════════════════════════════════
// SPATIAL AUDIO
// ═══════════════════════════════════════════════════════════════

void UAudioManager::PlaySpatialSound(USoundCue* Sound, const FVector& Location, float VolumeMultiplier)
{
    if (!Sound)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Tentativa de tocar som nulo"));
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Mundo não encontrado"));
        return;
    }
    
    // Tocar som espacial
    UGameplayStatics::PlaySoundAtLocation(
        World,
        Sound,
        Location,
        VolumeMultiplier * SFXVolume * MasterVolume,
        1.0f, // Pitch
        0.0f, // Start time
        nullptr, // Attenuation settings (usar default)
        nullptr, // Concurrency settings
        nullptr  // Owner
    );
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Som espacial tocado em (%s) com volume %.2f"), 
           *Location.ToString(), VolumeMultiplier);
}

void UAudioManager::SetListenerLocation(const FVector& Location)
{
    ListenerLocation = Location;
    
    // Actualizar posição do listener no audio device
    UWorld* World = GetWorld();
    if (World && World->GetAudioDeviceRaw())
    {
        // O UE5 actualiza automaticamente a posição do listener baseado na câmara
        // Mas podemos usar esta informação para cálculos de distância
    }
}

// ═══════════════════════════════════════════════════════════════
// MASTER VOLUME CONTROLS
// ═══════════════════════════════════════════════════════════════

void UAudioManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Actualizar todos os componentes de áudio
    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->SetVolumeMultiplier(AmbienceVolume * MasterVolume);
    }
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MasterVolume);
    }
    
    if (HeartbeatAudioComponent)
    {
        HeartbeatAudioComponent->SetVolumeMultiplier(MasterVolume);
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Master volume definido para %.2f"), MasterVolume);
}

void UAudioManager::SetAmbienceVolume(float Volume)
{
    AmbienceVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->SetVolumeMultiplier(AmbienceVolume * MasterVolume);
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Ambience volume definido para %.2f"), AmbienceVolume);
}

void UAudioManager::SetSFXVolume(float Volume)
{
    SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioManager: SFX volume definido para %.2f"), SFXVolume);
}

// ═══════════════════════════════════════════════════════════════
// INTERNAL METHODS
// ═══════════════════════════════════════════════════════════════

void UAudioManager::InitializeBiomeSettings()
{
    BiomeSettings.Empty();
    BiomeSettings.SetNum(5); // 5 biomas
    
    // Pantano
    BiomeSettings[0].BiomeType = EAudio_BiomeAmbience::Swamp;
    BiomeSettings[0].ReverbWetness = 0.8f;
    BiomeSettings[0].ReverbDecayTime = 4.0f;
    BiomeSettings[0].WindIntensity = 0.2f;
    
    // Floresta
    BiomeSettings[1].BiomeType = EAudio_BiomeAmbience::Forest;
    BiomeSettings[1].ReverbWetness = 0.6f;
    BiomeSettings[1].ReverbDecayTime = 3.0f;
    BiomeSettings[1].WindIntensity = 0.4f;
    
    // Savana
    BiomeSettings[2].BiomeType = EAudio_BiomeAmbience::Savanna;
    BiomeSettings[2].ReverbWetness = 0.2f;
    BiomeSettings[2].ReverbDecayTime = 1.5f;
    BiomeSettings[2].WindIntensity = 0.7f;
    
    // Deserto
    BiomeSettings[3].BiomeType = EAudio_BiomeAmbience::Desert;
    BiomeSettings[3].ReverbWetness = 0.1f;
    BiomeSettings[3].ReverbDecayTime = 0.8f;
    BiomeSettings[3].WindIntensity = 0.9f;
    
    // Montanha
    BiomeSettings[4].BiomeType = EAudio_BiomeAmbience::Mountain;
    BiomeSettings[4].ReverbWetness = 0.3f;
    BiomeSettings[4].ReverbDecayTime = 6.0f;
    BiomeSettings[4].WindIntensity = 1.0f;
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Configurações de bioma inicializadas"));
}

void UAudioManager::InitializeThreatSettings()
{
    ThreatAudioSettings.Empty();
    ThreatAudioSettings.SetNum(4); // 4 threat levels
    
    // Safe
    ThreatAudioSettings[0].ThreatLevel = EAudio_ThreatLevel::Safe;
    ThreatAudioSettings[0].HeartbeatIntensity = 0.0f;
    ThreatAudioSettings[0].MusicVolume = 0.3f;
    
    // Caution
    ThreatAudioSettings[1].ThreatLevel = EAudio_ThreatLevel::Caution;
    ThreatAudioSettings[1].HeartbeatIntensity = 0.2f;
    ThreatAudioSettings[1].MusicVolume = 0.5f;
    
    // Danger
    ThreatAudioSettings[2].ThreatLevel = EAudio_ThreatLevel::Danger;
    ThreatAudioSettings[2].HeartbeatIntensity = 0.6f;
    ThreatAudioSettings[2].MusicVolume = 0.8f;
    
    // Critical
    ThreatAudioSettings[3].ThreatLevel = EAudio_ThreatLevel::Critical;
    ThreatAudioSettings[3].HeartbeatIntensity = 1.0f;
    ThreatAudioSettings[3].MusicVolume = 1.0f;
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Configurações de threat level inicializadas"));
}

void UAudioManager::TransitionToNewBiome(EAudio_BiomeAmbience NewBiome)
{
    if (!BiomeSettings.IsValidIndex((int32)NewBiome))
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Bioma inválido %d"), (int32)NewBiome);
        return;
    }
    
    const FAudio_BiomeSettings& Settings = BiomeSettings[(int32)NewBiome];
    
    // Actualizar configurações de reverb
    UpdateReverbSettings(Settings);
    
    // TODO: Carregar e tocar nova música de ambiente
    // TODO: Fade out da música anterior e fade in da nova
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Transição para bioma %d concluída"), (int32)NewBiome);
}

void UAudioManager::UpdateReverbSettings(const FAudio_BiomeSettings& Settings)
{
    // TODO: Actualizar configurações de reverb do audio engine
    // Isto requer acesso ao audio device e configuração de reverb zones
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Reverb actualizado - Wetness=%.2f, Decay=%.2f"), 
           Settings.ReverbWetness, Settings.ReverbDecayTime);
}

EAudio_BiomeAmbience UAudioManager::DetermineBiomeFromLocation(const FVector& Location)
{
    // Coordenadas dos biomas (do brain memory)
    // Pantano: Centro(-50000, -45000, 0), Zona X(-77500 a -25000), Y(-76500 a -15000)
    // Floresta: Centro(-45000, 40000, 0), Zona X(-77500 a -15000), Y(15000 a 76500)
    // Savana: Centro(0, 0, 0), Zona X(-20000 a 20000), Y(-20000 a 20000)
    // Deserto: Centro(55000, 0, 0), Zona X(25000 a 79500), Y(-30000 a 30000)
    // Montanha: Centro(40000, 50000, 500), Zona X(15000 a 79500), Y(20000 a 76500)
    
    float X = Location.X;
    float Y = Location.Y;
    
    // Verificar Pantano
    if (X >= -77500 && X <= -25000 && Y >= -76500 && Y <= -15000)
    {
        return EAudio_BiomeAmbience::Swamp;
    }
    
    // Verificar Floresta
    if (X >= -77500 && X <= -15000 && Y >= 15000 && Y <= 76500)
    {
        return EAudio_BiomeAmbience::Forest;
    }
    
    // Verificar Savana (centro)
    if (X >= -20000 && X <= 20000 && Y >= -20000 && Y <= 20000)
    {
        return EAudio_BiomeAmbience::Savanna;
    }
    
    // Verificar Deserto
    if (X >= 25000 && X <= 79500 && Y >= -30000 && Y <= 30000)
    {
        return EAudio_BiomeAmbience::Desert;
    }
    
    // Verificar Montanha
    if (X >= 15000 && X <= 79500 && Y >= 20000 && Y <= 76500)
    {
        return EAudio_BiomeAmbience::Mountain;
    }
    
    // Default para Savana se não estiver em nenhuma zona específica
    return EAudio_BiomeAmbience::Savanna;
}