#include "Audio_MetaSoundsManager.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

AAudio_MetaSoundsManager::AAudio_MetaSoundsManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    GlobalVolumeMultiplier = 1.0f;
    UpdateFrequency = 0.1f; // Update 10 times per second
    CurrentBiome = EBiomeType::Temperate_Forest;
    CurrentWeather = EWeatherType::Clear;
    DistanceToPlayer = 0.0f;
    LastUpdateTime = 0.0f;
    CachedPlayerPawn = nullptr;

    // Initialize default biome configs
    BiomeConfigs.SetNum(4);
    
    // Forest biome config
    BiomeConfigs[0].BiomeType = EBiomeType::Temperate_Forest;
    BiomeConfigs[0].MaxAudibleDistance = 2000.0f;
    BiomeConfigs[0].CrossfadeDistance = 500.0f;
    
    // Plains biome config
    BiomeConfigs[1].BiomeType = EBiomeType::Grassland;
    BiomeConfigs[1].MaxAudibleDistance = 3000.0f;
    BiomeConfigs[1].CrossfadeDistance = 800.0f;
    
    // Mountain biome config
    BiomeConfigs[2].BiomeType = EBiomeType::Mountain;
    BiomeConfigs[2].MaxAudibleDistance = 4000.0f;
    BiomeConfigs[2].CrossfadeDistance = 1000.0f;
    
    // River biome config
    BiomeConfigs[3].BiomeType = EBiomeType::River;
    BiomeConfigs[3].MaxAudibleDistance = 1500.0f;
    BiomeConfigs[3].CrossfadeDistance = 300.0f;
}

void AAudio_MetaSoundsManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioLayers();
    
    // Cache player pawn
    CachedPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    
    // Start with current biome audio
    SetBiomeAudio(CurrentBiome);
}

void AAudio_MetaSoundsManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update at specified frequency
    if (GetWorld()->GetTimeSeconds() - LastUpdateTime >= UpdateFrequency)
    {
        UpdatePlayerDistance();
        UpdateAudioLayers();
        LastUpdateTime = GetWorld()->GetTimeSeconds();
    }
}

void AAudio_MetaSoundsManager::InitializeAudioLayers()
{
    // Create 6 audio layers for complex soundscapes
    for (int32 i = 0; i < 6; i++)
    {
        FString ComponentName = FString::Printf(TEXT("AudioLayer_%d"), i);
        UAudioComponent* AudioLayer = CreateDefaultSubobject<UAudioComponent>(*ComponentName);
        
        if (AudioLayer)
        {
            AudioLayer->SetupAttachment(RootComponent);
            AudioLayer->bAutoActivate = false;
            AudioLayer->SetVolumeMultiplier(0.0f);
            AudioLayers.Add(AudioLayer);
        }
    }
}

void AAudio_MetaSoundsManager::SetBiomeAudio(EBiomeType NewBiome)
{
    if (CurrentBiome == NewBiome)
    {
        return;
    }
    
    CurrentBiome = NewBiome;
    
    FAudio_BiomeAudioConfig* Config = GetBiomeConfig(NewBiome);
    if (!Config)
    {
        return;
    }
    
    // Fade out current layers
    for (int32 i = 0; i < AudioLayers.Num(); i++)
    {
        if (AudioLayers[i] && AudioLayers[i]->IsPlaying())
        {
            FadeOutLayer(i, 2.0f);
        }
    }
    
    // Set up new biome layers
    for (int32 i = 0; i < Config->AmbientLayers.Num() && i < AudioLayers.Num(); i++)
    {
        if (AudioLayers[i] && Config->AmbientLayers[i].SoundAsset)
        {
            AudioLayers[i]->SetSound(Config->AmbientLayers[i].SoundAsset);
            AudioLayers[i]->SetVolumeMultiplier(Config->AmbientLayers[i].VolumeMultiplier * GlobalVolumeMultiplier);
            AudioLayers[i]->SetPitchMultiplier(Config->AmbientLayers[i].PitchMultiplier);
            
            if (IsPlayerInRange())
            {
                FadeInLayer(i, Config->AmbientLayers[i].FadeInTime);
            }
        }
    }
}

void AAudio_MetaSoundsManager::SetWeatherAudio(EWeatherType NewWeather)
{
    if (CurrentWeather == NewWeather)
    {
        return;
    }
    
    CurrentWeather = NewWeather;
    
    FAudio_BiomeAudioConfig* Config = GetBiomeConfig(CurrentBiome);
    if (!Config)
    {
        return;
    }
    
    // Handle weather-specific audio layers
    int32 WeatherLayerStart = Config->AmbientLayers.Num();
    
    for (int32 i = 0; i < Config->WeatherLayers.Num() && (WeatherLayerStart + i) < AudioLayers.Num(); i++)
    {
        int32 LayerIndex = WeatherLayerStart + i;
        
        if (AudioLayers[LayerIndex])
        {
            if (NewWeather != EWeatherType::Clear && Config->WeatherLayers[i].SoundAsset)
            {
                AudioLayers[LayerIndex]->SetSound(Config->WeatherLayers[i].SoundAsset);
                AudioLayers[LayerIndex]->SetVolumeMultiplier(Config->WeatherLayers[i].VolumeMultiplier * GlobalVolumeMultiplier);
                FadeInLayer(LayerIndex, Config->WeatherLayers[i].FadeInTime);
            }
            else
            {
                FadeOutLayer(LayerIndex, 1.0f);
            }
        }
    }
}

void AAudio_MetaSoundsManager::UpdateAudioLayers()
{
    if (!IsPlayerInRange())
    {
        // Fade out all layers if player is too far
        for (int32 i = 0; i < AudioLayers.Num(); i++)
        {
            if (AudioLayers[i] && AudioLayers[i]->IsPlaying())
            {
                FadeOutLayer(i, 1.0f);
            }
        }
        return;
    }
    
    FAudio_BiomeAudioConfig* Config = GetBiomeConfig(CurrentBiome);
    if (!Config)
    {
        return;
    }
    
    // Calculate distance-based volume multiplier
    float DistanceMultiplier = 1.0f;
    if (DistanceToPlayer > Config->CrossfadeDistance)
    {
        float FadeRange = Config->MaxAudibleDistance - Config->CrossfadeDistance;
        float FadeDistance = DistanceToPlayer - Config->CrossfadeDistance;
        DistanceMultiplier = FMath::Clamp(1.0f - (FadeDistance / FadeRange), 0.0f, 1.0f);
    }
    
    // Update volume for all active layers
    for (int32 i = 0; i < AudioLayers.Num(); i++)
    {
        if (AudioLayers[i] && AudioLayers[i]->IsPlaying())
        {
            float BaseVolume = 1.0f;
            
            if (i < Config->AmbientLayers.Num())
            {
                BaseVolume = Config->AmbientLayers[i].VolumeMultiplier;
            }
            else if ((i - Config->AmbientLayers.Num()) < Config->WeatherLayers.Num())
            {
                BaseVolume = Config->WeatherLayers[i - Config->AmbientLayers.Num()].VolumeMultiplier;
            }
            
            float FinalVolume = BaseVolume * GlobalVolumeMultiplier * DistanceMultiplier;
            AudioLayers[i]->SetVolumeMultiplier(FinalVolume);
        }
    }
}

void AAudio_MetaSoundsManager::FadeInLayer(int32 LayerIndex, float FadeTime)
{
    if (LayerIndex >= 0 && LayerIndex < AudioLayers.Num() && AudioLayers[LayerIndex])
    {
        AudioLayers[LayerIndex]->FadeIn(FadeTime);
    }
}

void AAudio_MetaSoundsManager::FadeOutLayer(int32 LayerIndex, float FadeTime)
{
    if (LayerIndex >= 0 && LayerIndex < AudioLayers.Num() && AudioLayers[LayerIndex])
    {
        AudioLayers[LayerIndex]->FadeOut(FadeTime, 0.0f);
    }
}

void AAudio_MetaSoundsManager::SetGlobalVolume(float NewVolume)
{
    GlobalVolumeMultiplier = FMath::Clamp(NewVolume, 0.0f, 2.0f);
    UpdateAudioLayers();
}

float AAudio_MetaSoundsManager::GetDistanceToPlayer() const
{
    return DistanceToPlayer;
}

bool AAudio_MetaSoundsManager::IsPlayerInRange() const
{
    FAudio_BiomeAudioConfig* Config = GetBiomeConfig(CurrentBiome);
    if (!Config)
    {
        return false;
    }
    
    return DistanceToPlayer <= Config->MaxAudibleDistance;
}

void AAudio_MetaSoundsManager::UpdatePlayerDistance()
{
    if (!CachedPlayerPawn)
    {
        CachedPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    }
    
    if (CachedPlayerPawn)
    {
        DistanceToPlayer = FVector::Dist(GetActorLocation(), CachedPlayerPawn->GetActorLocation());
    }
}

FAudio_BiomeAudioConfig* AAudio_MetaSoundsManager::GetBiomeConfig(EBiomeType BiomeType)
{
    for (FAudio_BiomeAudioConfig& Config : BiomeConfigs)
    {
        if (Config.BiomeType == BiomeType)
        {
            return &Config;
        }
    }
    
    return nullptr;
}