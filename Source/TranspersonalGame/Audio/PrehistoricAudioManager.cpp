#include "PrehistoricAudioManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/Engine.h"

APrehistoricAudioManager::APrehistoricAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Update twice per second

    // Create root scene component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create main audio component
    MainAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MainAudioComponent"));
    MainAudioComponent->SetupAttachment(RootComponent);
    MainAudioComponent->bAutoActivate = false;
    MainAudioComponent->SetVolumeMultiplier(0.7f);

    // Create audio trigger sphere
    AudioTriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AudioTriggerSphere"));
    AudioTriggerSphere->SetupAttachment(RootComponent);
    AudioTriggerSphere->SetSphereRadius(5000.0f); // 50m radius
    AudioTriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    AudioTriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    AudioTriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize default values
    CurrentBiome = EBiomeType::Savana;
    CurrentDangerLevel = 0.0f;
    bIsPlayingDangerAudio = false;
    AudioUpdateInterval = 1.0f;
    BiomeTransitionTime = 3.0f;
    LastAudioUpdateTime = 0.0f;
    CachedPlayerPawn = nullptr;
    bIsFading = false;
    FadeStartTime = 0.0f;
    FadeDuration = 0.0f;
    FadeTargetSound = nullptr;
}

void APrehistoricAudioManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeAudioData();
    
    // Cache player pawn reference
    CachedPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    
    // Start with current biome audio
    UpdateBiomeAudio(CurrentBiome);
    
    UE_LOG(LogTemp, Log, TEXT("PrehistoricAudioManager initialized"));
}

void APrehistoricAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update audio based on player state at regular intervals
    if (CurrentTime - LastAudioUpdateTime >= AudioUpdateInterval)
    {
        UpdateAudioBasedOnPlayerState();
        LastAudioUpdateTime = CurrentTime;
    }
    
    // Handle audio fading
    if (bIsFading && FadeTargetSound)
    {
        float FadeProgress = (CurrentTime - FadeStartTime) / FadeDuration;
        if (FadeProgress >= 1.0f)
        {
            // Fade complete
            MainAudioComponent->SetSound(FadeTargetSound);
            MainAudioComponent->SetVolumeMultiplier(0.7f);
            bIsFading = false;
            FadeTargetSound = nullptr;
        }
        else
        {
            // Interpolate volume during fade
            float FadeVolume = FMath::Lerp(0.0f, 0.7f, FadeProgress);
            MainAudioComponent->SetVolumeMultiplier(FadeVolume);
        }
    }
}

void APrehistoricAudioManager::InitializeBiomeAudioData()
{
    // Initialize biome audio data with default values
    // In a real implementation, these would be loaded from data assets
    
    FAudio_BiomeAudioData SavanaData;
    SavanaData.BaseVolume = 0.6f;
    SavanaData.FadeDistance = 6000.0f;
    BiomeAudioMap.Add(EBiomeType::Savana, SavanaData);
    
    FAudio_BiomeAudioData FlorestaData;
    FlorestaData.BaseVolume = 0.8f;
    FlorestaData.FadeDistance = 4000.0f;
    BiomeAudioMap.Add(EBiomeType::Floresta, FlorestaData);
    
    FAudio_BiomeAudioData DesertoData;
    DesertoData.BaseVolume = 0.5f;
    DesertoData.FadeDistance = 8000.0f;
    BiomeAudioMap.Add(EBiomeType::Deserto, DesertoData);
    
    FAudio_BiomeAudioData PantanoData;
    PantanoData.BaseVolume = 0.7f;
    PantanoData.FadeDistance = 3000.0f;
    BiomeAudioMap.Add(EBiomeType::Pantano, PantanoData);
    
    FAudio_BiomeAudioData MontanhaData;
    MontanhaData.BaseVolume = 0.4f;
    MontanhaData.FadeDistance = 10000.0f;
    BiomeAudioMap.Add(EBiomeType::Montanha, MontanhaData);
}

void APrehistoricAudioManager::UpdateBiomeAudio(EBiomeType NewBiome)
{
    if (NewBiome == CurrentBiome)
    {
        return; // No change needed
    }
    
    CurrentBiome = NewBiome;
    
    // Get biome audio data
    if (FAudio_BiomeAudioData* BiomeData = BiomeAudioMap.Find(NewBiome))
    {
        if (BiomeData->AmbientSound.IsValid())
        {
            USoundBase* NewAmbientSound = BiomeData->AmbientSound.LoadSynchronous();
            if (NewAmbientSound)
            {
                CrossfadeToNewAudio(NewAmbientSound, BiomeTransitionTime);
                UE_LOG(LogTemp, Log, TEXT("Transitioning to %s biome audio"), 
                    *UEnum::GetValueAsString(NewBiome));
            }
        }
    }
}

void APrehistoricAudioManager::PlaySurvivalAudioCue(ESurvivalStat StatType, float StatValue)
{
    USoundBase* CueSound = nullptr;
    
    // Determine which survival audio cue to play based on stat type and value
    switch (StatType)
    {
        case ESurvivalStat::Health:
            if (StatValue < 25.0f && SurvivalAudioCues.LowHealthSound.IsValid())
            {
                CueSound = SurvivalAudioCues.LowHealthSound.LoadSynchronous();
            }
            break;
            
        case ESurvivalStat::Fear:
            if (StatValue > 75.0f && SurvivalAudioCues.HighFearSound.IsValid())
            {
                CueSound = SurvivalAudioCues.HighFearSound.LoadSynchronous();
            }
            break;
            
        case ESurvivalStat::Hunger:
            if (StatValue > 80.0f && SurvivalAudioCues.HungerSound.IsValid())
            {
                CueSound = SurvivalAudioCues.HungerSound.LoadSynchronous();
            }
            break;
            
        case ESurvivalStat::Thirst:
            if (StatValue > 80.0f && SurvivalAudioCues.ThirstSound.IsValid())
            {
                CueSound = SurvivalAudioCues.ThirstSound.LoadSynchronous();
            }
            break;
    }
    
    if (CueSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this, 
            CueSound, 
            GetActorLocation(), 
            SurvivalAudioCues.SurvivalAudioVolume
        );
        
        UE_LOG(LogTemp, Log, TEXT("Playing survival audio cue for %s"), 
            *UEnum::GetValueAsString(StatType));
    }
}

void APrehistoricAudioManager::SetDangerLevel(float DangerLevel)
{
    CurrentDangerLevel = FMath::Clamp(DangerLevel, 0.0f, 100.0f);
    
    // Adjust audio based on danger level
    if (CurrentDangerLevel > 70.0f && !bIsPlayingDangerAudio)
    {
        // High danger - play danger audio
        if (FAudio_BiomeAudioData* BiomeData = BiomeAudioMap.Find(CurrentBiome))
        {
            if (BiomeData->DangerSound.IsValid())
            {
                USoundBase* DangerSound = BiomeData->DangerSound.LoadSynchronous();
                if (DangerSound)
                {
                    UGameplayStatics::PlaySoundAtLocation(
                        this, 
                        DangerSound, 
                        GetActorLocation(), 
                        BiomeData->BaseVolume * 0.8f
                    );
                    bIsPlayingDangerAudio = true;
                }
            }
        }
    }
    else if (CurrentDangerLevel < 30.0f && bIsPlayingDangerAudio)
    {
        // Danger subsided - return to ambient
        bIsPlayingDangerAudio = false;
        UpdateBiomeAudio(CurrentBiome);
    }
}

void APrehistoricAudioManager::PlayDinosaurProximityAudio(EDinosaurSpecies Species, float Distance)
{
    // Calculate volume based on distance
    float ProximityVolume = CalculateAudioVolume(Distance, 2000.0f); // 20m max distance
    
    if (ProximityVolume > 0.1f)
    {
        // Play species-specific proximity sound
        // This would load species-specific audio assets
        UE_LOG(LogTemp, Log, TEXT("Dinosaur proximity audio: %s at distance %.1f"), 
            *UEnum::GetValueAsString(Species), Distance);
        
        // Increase danger level based on proximity
        float ProximityDanger = FMath::Lerp(100.0f, 0.0f, Distance / 2000.0f);
        SetDangerLevel(FMath::Max(CurrentDangerLevel, ProximityDanger));
    }
}

void APrehistoricAudioManager::StopAllAudio()
{
    if (MainAudioComponent)
    {
        MainAudioComponent->Stop();
    }
    
    bIsPlayingDangerAudio = false;
    bIsFading = false;
    CurrentDangerLevel = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("All audio stopped"));
}

void APrehistoricAudioManager::UpdateWeatherAudio(EWeatherType WeatherType)
{
    // Get current biome data
    if (FAudio_BiomeAudioData* BiomeData = BiomeAudioMap.Find(CurrentBiome))
    {
        if (BiomeData->WeatherSound.IsValid())
        {
            USoundBase* WeatherSound = BiomeData->WeatherSound.LoadSynchronous();
            if (WeatherSound)
            {
                float WeatherVolume = BiomeData->BaseVolume * 0.6f;
                
                // Adjust volume based on weather intensity
                switch (WeatherType)
                {
                    case EWeatherType::Storm:
                        WeatherVolume *= 1.5f;
                        break;
                    case EWeatherType::Rain:
                        WeatherVolume *= 1.2f;
                        break;
                    case EWeatherType::Clear:
                        WeatherVolume *= 0.5f;
                        break;
                }
                
                UGameplayStatics::PlaySoundAtLocation(
                    this, 
                    WeatherSound, 
                    GetActorLocation(), 
                    WeatherVolume
                );
                
                UE_LOG(LogTemp, Log, TEXT("Playing weather audio for %s"), 
                    *UEnum::GetValueAsString(WeatherType));
            }
        }
    }
}

void APrehistoricAudioManager::UpdateTimeOfDayAudio(float TimeOfDay)
{
    // Adjust ambient audio volume based on time of day
    float TimeMultiplier = 1.0f;
    
    // Dawn/Dusk (5-7 AM, 6-8 PM) - increased bird activity
    if ((TimeOfDay >= 5.0f && TimeOfDay <= 7.0f) || (TimeOfDay >= 18.0f && TimeOfDay <= 20.0f))
    {
        TimeMultiplier = 1.3f;
    }
    // Night (10 PM - 5 AM) - reduced ambient, increased danger audio sensitivity
    else if (TimeOfDay >= 22.0f || TimeOfDay <= 5.0f)
    {
        TimeMultiplier = 0.6f;
        // Night increases danger sensitivity
        SetDangerLevel(CurrentDangerLevel * 1.2f);
    }
    
    if (MainAudioComponent)
    {
        float CurrentVolume = MainAudioComponent->VolumeMultiplier;
        float TargetVolume = 0.7f * TimeMultiplier;
        MainAudioComponent->SetVolumeMultiplier(FMath::Lerp(CurrentVolume, TargetVolume, 0.1f));
    }
}

void APrehistoricAudioManager::UpdateAudioBasedOnPlayerState()
{
    if (!CachedPlayerPawn)
    {
        CachedPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
        if (!CachedPlayerPawn)
        {
            return;
        }
    }
    
    // Calculate distance to player
    float DistanceToPlayer = FVector::Dist(GetActorLocation(), CachedPlayerPawn->GetActorLocation());
    
    // Update audio volume based on distance
    if (FAudio_BiomeAudioData* BiomeData = BiomeAudioMap.Find(CurrentBiome))
    {
        float VolumeMultiplier = CalculateAudioVolume(DistanceToPlayer, BiomeData->FadeDistance);
        
        if (MainAudioComponent)
        {
            MainAudioComponent->SetVolumeMultiplier(BiomeData->BaseVolume * VolumeMultiplier);
        }
    }
}

float APrehistoricAudioManager::CalculateAudioVolume(float Distance, float MaxDistance)
{
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }
    
    // Linear falloff with smooth curve
    float DistanceRatio = Distance / MaxDistance;
    return FMath::Pow(1.0f - DistanceRatio, 2.0f);
}

void APrehistoricAudioManager::CrossfadeToNewAudio(USoundBase* NewSound, float FadeTime)
{
    if (!NewSound || !MainAudioComponent)
    {
        return;
    }
    
    // Start fade to new sound
    FadeTargetSound = NewSound;
    FadeStartTime = GetWorld()->GetTimeSeconds();
    FadeDuration = FadeTime;
    bIsFading = true;
    
    // Start with low volume
    MainAudioComponent->SetVolumeMultiplier(0.0f);
    MainAudioComponent->SetSound(NewSound);
    MainAudioComponent->Play();
}