#include "Audio_MetaSoundsManager.h"
#include "Components/SceneComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"

AAudio_MetaSoundsManager::AAudio_MetaSoundsManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootComp;

    // Create audio components
    MasterAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MasterAudioComponent"));
    MasterAudioComponent->SetupAttachment(RootComponent);

    AmbienceComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbienceComponent"));
    AmbienceComponent->SetupAttachment(RootComponent);

    MusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComponent"));
    MusicComponent->SetupAttachment(RootComponent);

    EffectsComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EffectsComponent"));
    EffectsComponent->SetupAttachment(RootComponent);

    // Initialize default values
    MasterVolume = 1.0f;
    AmbienceVolume = 0.7f;
    MusicVolume = 0.5f;
    EffectsVolume = 0.8f;
    
    CurrentBiome = EAudio_BiomeType::Forest;
    CurrentIntensity = EAudio_IntensityLevel::Calm;
    bIsNightTime = false;
    DinosaurProximityFactor = 0.0f;
    
    CurrentFadeTime = 0.0f;
    bIsFading = false;
    PreviousBiome = EAudio_BiomeType::Forest;

    // Initialize biome audio data with defaults
    BiomeAudioData.SetNum(5);
    for (int32 i = 0; i < BiomeAudioData.Num(); i++)
    {
        BiomeAudioData[i].BiomeType = static_cast<EAudio_BiomeType>(i);
        BiomeAudioData[i].BaseVolume = 0.7f;
        BiomeAudioData[i].FadeTime = 2.0f;
    }

    // Initialize dinosaur proximity settings
    DinosaurProximitySettings.MinDistance = 500.0f;
    DinosaurProximitySettings.MaxDistance = 2000.0f;
    DinosaurProximitySettings.VolumeMultiplier = 1.0f;
}

void AAudio_MetaSoundsManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize audio components
    if (MasterAudioComponent)
    {
        MasterAudioComponent->SetVolumeMultiplier(MasterVolume);
    }

    if (AmbienceComponent)
    {
        AmbienceComponent->SetVolumeMultiplier(AmbienceVolume);
        AmbienceComponent->bAutoActivate = true;
    }

    if (MusicComponent)
    {
        MusicComponent->SetVolumeMultiplier(MusicVolume);
        MusicComponent->bAutoActivate = true;
    }

    if (EffectsComponent)
    {
        EffectsComponent->SetVolumeMultiplier(EffectsVolume);
    }

    // Start with default biome audio
    SetCurrentBiome(CurrentBiome, false);
    
    UE_LOG(LogTemp, Warning, TEXT("MetaSounds Audio Manager initialized"));
}

void AAudio_MetaSoundsManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle biome fading
    if (bIsFading)
    {
        CurrentFadeTime += DeltaTime;
        
        // Find current biome data
        FAudio_BiomeAudioData* CurrentBiomeData = nullptr;
        for (FAudio_BiomeAudioData& Data : BiomeAudioData)
        {
            if (Data.BiomeType == CurrentBiome)
            {
                CurrentBiomeData = &Data;
                break;
            }
        }

        if (CurrentBiomeData && CurrentFadeTime >= CurrentBiomeData->FadeTime)
        {
            bIsFading = false;
            CurrentFadeTime = 0.0f;
        }
    }

    // Update audio layers based on current state
    UpdateAudioLayers();
}

void AAudio_MetaSoundsManager::SetCurrentBiome(EAudio_BiomeType NewBiome, bool bFadeTransition)
{
    if (NewBiome == CurrentBiome)
    {
        return;
    }

    PreviousBiome = CurrentBiome;
    CurrentBiome = NewBiome;

    if (bFadeTransition)
    {
        FadeBetweenBiomes(PreviousBiome, CurrentBiome);
    }
    else
    {
        UpdateAudioLayers();
    }

    UE_LOG(LogTemp, Warning, TEXT("Biome changed to: %d"), static_cast<int32>(NewBiome));
}

void AAudio_MetaSoundsManager::SetIntensityLevel(EAudio_IntensityLevel NewIntensity)
{
    if (NewIntensity != CurrentIntensity)
    {
        CurrentIntensity = NewIntensity;
        ApplyIntensityModifiers();
        
        UE_LOG(LogTemp, Warning, TEXT("Intensity level changed to: %d"), static_cast<int32>(NewIntensity));
    }
}

void AAudio_MetaSoundsManager::SetTimeOfDay(bool bIsNight)
{
    if (bIsNight != bIsNightTime)
    {
        bIsNightTime = bIsNight;
        UpdateAudioLayers();
        
        UE_LOG(LogTemp, Warning, TEXT("Time of day changed - Night: %s"), bIsNight ? TEXT("True") : TEXT("False"));
    }
}

void AAudio_MetaSoundsManager::UpdateDinosaurProximity(float ProximityFactor)
{
    DinosaurProximityFactor = FMath::Clamp(ProximityFactor, 0.0f, 1.0f);
    
    // Adjust music and ambience based on proximity
    if (MusicComponent)
    {
        float ProximityVolume = MusicVolume * (1.0f - DinosaurProximityFactor * 0.5f);
        MusicComponent->SetVolumeMultiplier(ProximityVolume);
    }
}

void AAudio_MetaSoundsManager::PlayFootstepEffect(FVector Location, float Volume)
{
    if (EffectsComponent)
    {
        EffectsComponent->SetWorldLocation(Location);
        EffectsComponent->SetVolumeMultiplier(EffectsVolume * Volume);
        EffectsComponent->Play();
        
        UE_LOG(LogTemp, Log, TEXT("Footstep effect played at location: %s"), *Location.ToString());
    }
}

void AAudio_MetaSoundsManager::PlayDamageEffect()
{
    if (EffectsComponent)
    {
        EffectsComponent->SetVolumeMultiplier(EffectsVolume * 1.2f);
        EffectsComponent->Play();
        
        UE_LOG(LogTemp, Warning, TEXT("Damage effect played"));
    }
}

void AAudio_MetaSoundsManager::PlayDinosaurRoar(FVector Location, float Intensity)
{
    if (EffectsComponent)
    {
        EffectsComponent->SetWorldLocation(Location);
        EffectsComponent->SetVolumeMultiplier(EffectsVolume * Intensity);
        EffectsComponent->Play();
        
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur roar played at intensity: %f"), Intensity);
    }
}

void AAudio_MetaSoundsManager::TestAudioSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("Testing Audio System..."));
    
    // Cycle through biomes
    int32 NextBiome = (static_cast<int32>(CurrentBiome) + 1) % 5;
    SetCurrentBiome(static_cast<EAudio_BiomeType>(NextBiome), true);
    
    // Test effects
    PlayFootstepEffect(GetActorLocation(), 1.0f);
}

void AAudio_MetaSoundsManager::UpdateAudioLayers()
{
    // Find current biome data
    FAudio_BiomeAudioData* CurrentBiomeData = nullptr;
    for (FAudio_BiomeAudioData& Data : BiomeAudioData)
    {
        if (Data.BiomeType == CurrentBiome)
        {
            CurrentBiomeData = &Data;
            break;
        }
    }

    if (!CurrentBiomeData)
    {
        return;
    }

    // Update ambience component
    if (AmbienceComponent && CurrentBiomeData->AmbientSound)
    {
        if (AmbienceComponent->GetSound() != CurrentBiomeData->AmbientSound)
        {
            AmbienceComponent->SetSound(CurrentBiomeData->AmbientSound);
        }
        
        float AdjustedVolume = CurrentBiomeData->BaseVolume * AmbienceVolume;
        if (bIsNightTime)
        {
            AdjustedVolume *= 0.7f; // Quieter at night
        }
        
        AmbienceComponent->SetVolumeMultiplier(AdjustedVolume);
    }

    // Update music component
    if (MusicComponent && CurrentBiomeData->MusicLayer)
    {
        if (MusicComponent->GetSound() != CurrentBiomeData->MusicLayer)
        {
            MusicComponent->SetSound(CurrentBiomeData->MusicLayer);
        }
        
        float AdjustedVolume = MusicVolume;
        if (bIsNightTime)
        {
            AdjustedVolume *= 1.2f; // More prominent at night
        }
        
        MusicComponent->SetVolumeMultiplier(AdjustedVolume);
    }
}

void AAudio_MetaSoundsManager::FadeBetweenBiomes(EAudio_BiomeType FromBiome, EAudio_BiomeType ToBiome)
{
    bIsFading = true;
    CurrentFadeTime = 0.0f;
    
    // Start fade transition
    UpdateAudioLayers();
    
    UE_LOG(LogTemp, Log, TEXT("Fading from biome %d to biome %d"), static_cast<int32>(FromBiome), static_cast<int32>(ToBiome));
}

void AAudio_MetaSoundsManager::ApplyIntensityModifiers()
{
    float IntensityMultiplier = 1.0f;
    
    switch (CurrentIntensity)
    {
        case EAudio_IntensityLevel::Calm:
            IntensityMultiplier = 0.8f;
            break;
        case EAudio_IntensityLevel::Tense:
            IntensityMultiplier = 1.0f;
            break;
        case EAudio_IntensityLevel::Danger:
            IntensityMultiplier = 1.3f;
            break;
        case EAudio_IntensityLevel::Combat:
            IntensityMultiplier = 1.5f;
            break;
    }

    if (MusicComponent)
    {
        MusicComponent->SetVolumeMultiplier(MusicVolume * IntensityMultiplier);
    }

    if (EffectsComponent)
    {
        EffectsComponent->SetVolumeMultiplier(EffectsVolume * IntensityMultiplier);
    }
}