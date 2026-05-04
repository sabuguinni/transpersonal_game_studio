#include "Audio_SoundManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

UAudio_SoundManager::UAudio_SoundManager()
{
    MasterVolume = 1.0f;
    MaxAudioDistance = 50000.0f; // 500 metros em UU
    AmbientAudioComponent = nullptr;
    MusicAudioComponent = nullptr;
}

void UAudio_SoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Sistema de áudio inicializado"));
    
    // Inicializar perfis de bioma
    InitializeBiomeProfiles();
    
    // Configurar perfil inicial (Savana)
    CurrentProfile.BiomeType = EAudio_BiomeType::Savanna;
    CurrentProfile.WeatherType = EAudio_WeatherType::Clear;
    CurrentProfile.TimeOfDay = EAudio_TimeOfDay::Morning;
    CurrentProfile.BaseVolume = 0.7f;
    CurrentProfile.WindIntensity = 0.3f;
    
    // Criar componentes de áudio
    if (UWorld* World = GetWorld())
    {
        if (AActor* AudioActor = World->SpawnActor<AActor>())
        {
            AmbientAudioComponent = AudioActor->CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
            MusicAudioComponent = AudioActor->CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudio"));
            
            if (AmbientAudioComponent)
            {
                AmbientAudioComponent->bAutoActivate = true;
                AmbientAudioComponent->SetVolumeMultiplier(CurrentProfile.BaseVolume * MasterVolume);
            }
            
            if (MusicAudioComponent)
            {
                MusicAudioComponent->bAutoActivate = false;
                MusicAudioComponent->SetVolumeMultiplier(0.4f * MasterVolume);
            }
        }
    }
    
    UpdateAmbientAudio();
}

void UAudio_SoundManager::Deinitialize()
{
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        AmbientAudioComponent->Stop();
    }
    
    if (MusicAudioComponent && IsValid(MusicAudioComponent))
    {
        MusicAudioComponent->Stop();
    }
    
    Super::Deinitialize();
}

void UAudio_SoundManager::UpdateAmbientProfile(EAudio_BiomeType NewBiome, EAudio_WeatherType NewWeather, EAudio_TimeOfDay NewTime)
{
    // Verificar se houve mudança
    bool bProfileChanged = (CurrentProfile.BiomeType != NewBiome) || 
                          (CurrentProfile.WeatherType != NewWeather) || 
                          (CurrentProfile.TimeOfDay != NewTime);
    
    if (!bProfileChanged)
    {
        return;
    }
    
    // Atualizar perfil atual
    CurrentProfile.BiomeType = NewBiome;
    CurrentProfile.WeatherType = NewWeather;
    CurrentProfile.TimeOfDay = NewTime;
    
    // Buscar perfil correspondente no mapa de biomas
    if (BiomeProfiles.Contains(NewBiome))
    {
        FAudio_AmbientProfile* BaseProfile = BiomeProfiles.Find(NewBiome);
        if (BaseProfile)
        {
            CurrentProfile.AmbientSound = BaseProfile->AmbientSound;
            CurrentProfile.MusicTrack = BaseProfile->MusicTrack;
            CurrentProfile.BaseVolume = BaseProfile->BaseVolume;
        }
    }
    
    // Ajustar volume baseado no clima
    switch (NewWeather)
    {
        case EAudio_WeatherType::Rain:
            CurrentProfile.BaseVolume *= 1.2f;
            CurrentProfile.WindIntensity = 0.6f;
            break;
        case EAudio_WeatherType::Storm:
            CurrentProfile.BaseVolume *= 1.5f;
            CurrentProfile.WindIntensity = 0.9f;
            break;
        case EAudio_WeatherType::Fog:
            CurrentProfile.BaseVolume *= 0.7f;
            CurrentProfile.WindIntensity = 0.1f;
            break;
        default:
            CurrentProfile.WindIntensity = 0.3f;
            break;
    }
    
    // Ajustar volume baseado na hora do dia
    switch (NewTime)
    {
        case EAudio_TimeOfDay::Night:
            CurrentProfile.BaseVolume *= 0.6f;
            break;
        case EAudio_TimeOfDay::Dawn:
        case EAudio_TimeOfDay::Dusk:
            CurrentProfile.BaseVolume *= 0.8f;
            break;
        default:
            break;
    }
    
    UpdateAmbientAudio();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Perfil atualizado - Bioma: %d, Clima: %d, Hora: %d"), 
           (int32)NewBiome, (int32)NewWeather, (int32)NewTime);
}

void UAudio_SoundManager::PlayDinosaurSound(const FString& DinosaurType, const FVector& Location, float Volume)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Calcular volume baseado na distância
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    float DistanceVolume = CalculateVolumeByDistance(Location, PlayerPawn->GetActorLocation());
    float FinalVolume = Volume * DistanceVolume * MasterVolume;
    
    // Log para debug
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Som de dinossauro %s na posição %s, volume final: %f"), 
           *DinosaurType, *Location.ToString(), FinalVolume);
    
    // Por agora, usar som genérico até termos assets específicos
    if (FinalVolume > 0.1f)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), nullptr, Location, FinalVolume);
    }
}

void UAudio_SoundManager::PlayFootstepSound(const FVector& Location, float Intensity)
{
    if (!GetWorld())
    {
        return;
    }
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    float DistanceVolume = CalculateVolumeByDistance(Location, PlayerPawn->GetActorLocation());
    float FinalVolume = Intensity * DistanceVolume * MasterVolume * 0.5f; // Footsteps mais baixos
    
    if (FinalVolume > 0.05f)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), nullptr, Location, FinalVolume);
    }
}

void UAudio_SoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        AmbientAudioComponent->SetVolumeMultiplier(CurrentProfile.BaseVolume * MasterVolume);
    }
    
    if (MusicAudioComponent && IsValid(MusicAudioComponent))
    {
        MusicAudioComponent->SetVolumeMultiplier(0.4f * MasterVolume);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Volume principal definido para %f"), MasterVolume);
}

void UAudio_SoundManager::FadeToNewAmbient(const FAudio_AmbientProfile& NewProfile, float FadeTime)
{
    // Por agora implementação simples - fade instantâneo
    // TODO: Implementar fade gradual com timer
    CurrentProfile = NewProfile;
    UpdateAmbientAudio();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Fade para novo ambiente em %f segundos"), FadeTime);
}

void UAudio_SoundManager::InitializeBiomeProfiles()
{
    // Perfil Savana
    FAudio_AmbientProfile SavannaProfile;
    SavannaProfile.BiomeType = EAudio_BiomeType::Savanna;
    SavannaProfile.BaseVolume = 0.7f;
    SavannaProfile.WindIntensity = 0.3f;
    BiomeProfiles.Add(EAudio_BiomeType::Savanna, SavannaProfile);
    
    // Perfil Floresta
    FAudio_AmbientProfile ForestProfile;
    ForestProfile.BiomeType = EAudio_BiomeType::Forest;
    ForestProfile.BaseVolume = 0.8f;
    ForestProfile.WindIntensity = 0.5f;
    BiomeProfiles.Add(EAudio_BiomeType::Forest, ForestProfile);
    
    // Perfil Pântano
    FAudio_AmbientProfile SwampProfile;
    SwampProfile.BiomeType = EAudio_BiomeType::Swamp;
    SwampProfile.BaseVolume = 0.6f;
    SwampProfile.WindIntensity = 0.2f;
    BiomeProfiles.Add(EAudio_BiomeType::Swamp, SwampProfile);
    
    // Perfil Deserto
    FAudio_AmbientProfile DesertProfile;
    DesertProfile.BiomeType = EAudio_BiomeType::Desert;
    DesertProfile.BaseVolume = 0.5f;
    DesertProfile.WindIntensity = 0.7f;
    BiomeProfiles.Add(EAudio_BiomeType::Desert, DesertProfile);
    
    // Perfil Montanha
    FAudio_AmbientProfile MountainProfile;
    MountainProfile.BiomeType = EAudio_BiomeType::Mountain;
    MountainProfile.BaseVolume = 0.4f;
    MountainProfile.WindIntensity = 0.8f;
    BiomeProfiles.Add(EAudio_BiomeType::Mountain, MountainProfile);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: %d perfis de bioma inicializados"), BiomeProfiles.Num());
}

void UAudio_SoundManager::UpdateAmbientAudio()
{
    if (!AmbientAudioComponent || !IsValid(AmbientAudioComponent))
    {
        return;
    }
    
    // Atualizar volume
    AmbientAudioComponent->SetVolumeMultiplier(CurrentProfile.BaseVolume * MasterVolume);
    
    // TODO: Carregar e reproduzir som ambiente específico do bioma
    // Por agora apenas ajustar volume
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Áudio ambiente atualizado - Volume: %f"), 
           CurrentProfile.BaseVolume * MasterVolume);
}

float UAudio_SoundManager::CalculateVolumeByDistance(const FVector& SoundLocation, const FVector& ListenerLocation) const
{
    float Distance = FVector::Dist(SoundLocation, ListenerLocation);
    
    if (Distance >= MaxAudioDistance)
    {
        return 0.0f;
    }
    
    // Atenuação linear
    float VolumeMultiplier = 1.0f - (Distance / MaxAudioDistance);
    return FMath::Clamp(VolumeMultiplier, 0.0f, 1.0f);
}