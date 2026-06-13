#include "Audio_MetaSoundManager.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "MetasoundSource.h"

AAudio_MetaSoundManager::AAudio_MetaSoundManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    // Create audio components
    BiomeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BiomeAudioComponent"));
    BiomeAudioComponent->SetupAttachment(RootComponent);
    BiomeAudioComponent->bAutoActivate = false;

    WeatherAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WeatherAudioComponent"));
    WeatherAudioComponent->SetupAttachment(RootComponent);
    WeatherAudioComponent->bAutoActivate = false;

    ThreatAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ThreatAudioComponent"));
    ThreatAudioComponent->SetupAttachment(RootComponent);
    ThreatAudioComponent->bAutoActivate = false;

    // Initialize state
    CurrentBiome = EAudio_BiomeType::Forest;
    CurrentTimeOfDay = EAudio_TimeOfDay::Day;
    CurrentWeather = EAudio_WeatherState::Clear;
    bThreatActive = false;
    ThreatLevel = 0.0f;

    // Initialize biome audio data
    FAudio_BiomeAudioData ForestData;
    ForestData.BaseVolume = 0.4f;
    ForestData.FadeInTime = 3.0f;
    ForestData.FadeOutTime = 2.0f;
    BiomeAudioMap.Add(EAudio_BiomeType::Forest, ForestData);

    FAudio_BiomeAudioData PlainsData;
    PlainsData.BaseVolume = 0.3f;
    PlainsData.FadeInTime = 2.5f;
    PlainsData.FadeOutTime = 1.8f;
    BiomeAudioMap.Add(EAudio_BiomeType::Plains, PlainsData);

    FAudio_BiomeAudioData RiverData;
    RiverData.BaseVolume = 0.6f;
    RiverData.FadeInTime = 2.0f;
    RiverData.FadeOutTime = 1.5f;
    BiomeAudioMap.Add(EAudio_BiomeType::River, RiverData);

    // Initialize threat audio data
    TRexThreatAudio.TriggerDistance = 2000.0f;
    TRexThreatAudio.MaxVolume = 0.9f;
    TRexThreatAudio.FadeSpeed = 4.0f;
}

void AAudio_MetaSoundManager::BeginPlay()
{
    Super::BeginPlay();

    // Start with forest biome audio
    UpdateBiomeAudio();
    UpdateWeatherAudio();
}

void AAudio_MetaSoundManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update threat audio based on nearby threats
    UpdateThreatAudio(DeltaTime);
}

void AAudio_MetaSoundManager::SetBiome(EAudio_BiomeType NewBiome)
{
    if (CurrentBiome != NewBiome)
    {
        CurrentBiome = NewBiome;
        UpdateBiomeAudio();
    }
}

void AAudio_MetaSoundManager::SetTimeOfDay(EAudio_TimeOfDay NewTimeOfDay)
{
    if (CurrentTimeOfDay != NewTimeOfDay)
    {
        CurrentTimeOfDay = NewTimeOfDay;
        UpdateBiomeAudio(); // Biome audio changes with time of day
    }
}

void AAudio_MetaSoundManager::SetWeather(EAudio_WeatherState NewWeather)
{
    if (CurrentWeather != NewWeather)
    {
        CurrentWeather = NewWeather;
        UpdateWeatherAudio();
    }
}

void AAudio_MetaSoundManager::TriggerThreatAudio(float ThreatDistance)
{
    bThreatActive = true;
    
    // Calculate threat level based on distance
    float NormalizedDistance = FMath::Clamp(ThreatDistance / TRexThreatAudio.TriggerDistance, 0.0f, 1.0f);
    ThreatLevel = 1.0f - NormalizedDistance; // Closer = higher threat level

    if (ThreatAudioComponent && TRexThreatAudio.ThreatMetaSound)
    {
        if (!ThreatAudioComponent->IsPlaying())
        {
            ThreatAudioComponent->SetSound(TRexThreatAudio.ThreatMetaSound);
            ThreatAudioComponent->Play();
        }
        
        // Set volume based on threat level
        float TargetVolume = ThreatLevel * TRexThreatAudio.MaxVolume;
        ThreatAudioComponent->SetVolumeMultiplier(TargetVolume);
    }
}

void AAudio_MetaSoundManager::StopThreatAudio()
{
    bThreatActive = false;
    ThreatLevel = 0.0f;
    
    if (ThreatAudioComponent && ThreatAudioComponent->IsPlaying())
    {
        ThreatAudioComponent->FadeOut(1.0f, 0.0f);
    }
}

void AAudio_MetaSoundManager::UpdatePlayerFootsteps(bool bIsWalking, bool bIsRunning)
{
    // This would be called by the character movement component
    // to trigger appropriate footstep sounds based on movement state
    // Implementation depends on MetaSound setup for footsteps
}

void AAudio_MetaSoundManager::UpdateBiomeAudio()
{
    if (!BiomeAudioComponent)
        return;

    FAudio_BiomeAudioData* BiomeData = BiomeAudioMap.Find(CurrentBiome);
    if (BiomeData && BiomeData->AmbientMetaSound)
    {
        // Stop current audio if playing
        if (BiomeAudioComponent->IsPlaying())
        {
            BiomeAudioComponent->FadeOut(BiomeData->FadeOutTime, 0.0f);
        }

        // Set new MetaSound and play
        BiomeAudioComponent->SetSound(BiomeData->AmbientMetaSound);
        
        // Adjust volume based on time of day
        float TimeModifier = 1.0f;
        switch (CurrentTimeOfDay)
        {
            case EAudio_TimeOfDay::Dawn:
                TimeModifier = 0.7f;
                break;
            case EAudio_TimeOfDay::Day:
                TimeModifier = 1.0f;
                break;
            case EAudio_TimeOfDay::Dusk:
                TimeModifier = 0.8f;
                break;
            case EAudio_TimeOfDay::Night:
                TimeModifier = 0.5f;
                break;
        }

        float FinalVolume = BiomeData->BaseVolume * TimeModifier;
        BiomeAudioComponent->SetVolumeMultiplier(0.0f);
        BiomeAudioComponent->Play();
        BiomeAudioComponent->FadeIn(BiomeData->FadeInTime, FinalVolume);
    }
}

void AAudio_MetaSoundManager::UpdateWeatherAudio()
{
    if (!WeatherAudioComponent)
        return;

    TObjectPtr<UMetaSoundSource>* WeatherSound = WeatherAudioMap.Find(CurrentWeather);
    
    if (WeatherSound && *WeatherSound)
    {
        if (WeatherAudioComponent->IsPlaying())
        {
            WeatherAudioComponent->FadeOut(1.0f, 0.0f);
        }

        WeatherAudioComponent->SetSound(*WeatherSound);
        WeatherAudioComponent->SetVolumeMultiplier(0.0f);
        WeatherAudioComponent->Play();
        WeatherAudioComponent->FadeIn(2.0f, 0.6f);
    }
    else if (CurrentWeather == EAudio_WeatherState::Clear)
    {
        // Stop weather audio for clear weather
        if (WeatherAudioComponent->IsPlaying())
        {
            WeatherAudioComponent->FadeOut(2.0f, 0.0f);
        }
    }
}

void AAudio_MetaSoundManager::UpdateThreatAudio(float DeltaTime)
{
    if (!bThreatActive)
        return;

    // Check for nearby threats (T-Rex actors)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
    
    bool bThreatNearby = false;
    float ClosestThreatDistance = TRexThreatAudio.TriggerDistance;

    // Get player location
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
        return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Check for T-Rex actors
    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("TRex")))
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            if (Distance <= TRexThreatAudio.TriggerDistance)
            {
                bThreatNearby = true;
                ClosestThreatDistance = FMath::Min(ClosestThreatDistance, Distance);
            }
        }
    }

    if (bThreatNearby)
    {
        TriggerThreatAudio(ClosestThreatDistance);
    }
    else
    {
        StopThreatAudio();
    }
}

void AAudio_MetaSoundManager::CrossfadeAudio(UAudioComponent* FromComponent, UAudioComponent* ToComponent, float FadeTime)
{
    if (FromComponent && FromComponent->IsPlaying())
    {
        FromComponent->FadeOut(FadeTime, 0.0f);
    }

    if (ToComponent)
    {
        ToComponent->SetVolumeMultiplier(0.0f);
        ToComponent->Play();
        ToComponent->FadeIn(FadeTime, 1.0f);
    }
}