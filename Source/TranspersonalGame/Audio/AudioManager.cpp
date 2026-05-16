#include "AudioManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

UAudioManager::UAudioManager()
{
    AmbientAudioComponent = nullptr;
    WeatherAudioComponent = nullptr;
    NarrativeAudioComponent = nullptr;
    CachedWorld = nullptr;
    LastBiomeTransitionTime = 0.0f;
}

void UAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Initializing prehistoric audio system"));
    
    // Get world reference
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        CachedWorld = GameInstance->GetWorld();
    }
    
    InitializeAudioData();
    CreateAudioComponents();
    
    // Set default audio state
    CurrentAudioState.CurrentBiome = EBiomeType::Savanna;
    CurrentAudioState.DangerLevel = 0.0f;
    CurrentAudioState.bIsNightTime = false;
    CurrentAudioState.WeatherIntensity = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Prehistoric audio system ready"));
}

void UAudioManager::Deinitialize()
{
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        AmbientAudioComponent->Stop();
        AmbientAudioComponent = nullptr;
    }
    
    if (WeatherAudioComponent && IsValid(WeatherAudioComponent))
    {
        WeatherAudioComponent->Stop();
        WeatherAudioComponent = nullptr;
    }
    
    if (NarrativeAudioComponent && IsValid(NarrativeAudioComponent))
    {
        NarrativeAudioComponent->Stop();
        NarrativeAudioComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UAudioManager::InitializeAudioData()
{
    // Initialize biome audio data with placeholder settings
    for (int32 BiomeIndex = 0; BiomeIndex < static_cast<int32>(EBiomeType::Count); ++BiomeIndex)
    {
        EBiomeType BiomeType = static_cast<EBiomeType>(BiomeIndex);
        FAudio_BiomeAudioData& BiomeData = BiomeAudioMap.FindOrAdd(BiomeType);
        
        switch (BiomeType)
        {
            case EBiomeType::Savanna:
                BiomeData.AmbientVolume = 0.6f;
                BiomeData.RandomSoundChance = 0.15f;
                break;
                
            case EBiomeType::Forest:
                BiomeData.AmbientVolume = 0.8f;
                BiomeData.RandomSoundChance = 0.25f;
                break;
                
            case EBiomeType::Swamp:
                BiomeData.AmbientVolume = 0.7f;
                BiomeData.RandomSoundChance = 0.2f;
                break;
                
            case EBiomeType::Desert:
                BiomeData.AmbientVolume = 0.4f;
                BiomeData.RandomSoundChance = 0.05f;
                break;
                
            case EBiomeType::Mountain:
                BiomeData.AmbientVolume = 0.5f;
                BiomeData.RandomSoundChance = 0.1f;
                break;
                
            default:
                BiomeData.AmbientVolume = 0.5f;
                BiomeData.RandomSoundChance = 0.1f;
                break;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Biome audio data initialized for %d biomes"), BiomeAudioMap.Num());
}

void UAudioManager::CreateAudioComponents()
{
    if (!CachedWorld)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: No world reference for audio components"));
        return;
    }
    
    // Create ambient audio component
    AmbientAudioComponent = NewObject<UAudioComponent>(this);
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(0.7f);
        AmbientAudioComponent->bAutoActivate = false;
        AmbientAudioComponent->RegisterComponent();
    }
    
    // Create weather audio component
    WeatherAudioComponent = NewObject<UAudioComponent>(this);
    if (WeatherAudioComponent)
    {
        WeatherAudioComponent->SetVolumeMultiplier(0.5f);
        WeatherAudioComponent->bAutoActivate = false;
        WeatherAudioComponent->RegisterComponent();
    }
    
    // Create narrative audio component
    NarrativeAudioComponent = NewObject<UAudioComponent>(this);
    if (NarrativeAudioComponent)
    {
        NarrativeAudioComponent->SetVolumeMultiplier(0.9f);
        NarrativeAudioComponent->bAutoActivate = false;
        NarrativeAudioComponent->RegisterComponent();
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Audio components created successfully"));
}

void UAudioManager::UpdateBiomeAudio(EBiomeType NewBiome, const FVector& PlayerLocation)
{
    if (CurrentAudioState.CurrentBiome == NewBiome)
    {
        return; // No change needed
    }
    
    EBiomeType PreviousBiome = CurrentAudioState.CurrentBiome;
    CurrentAudioState.CurrentBiome = NewBiome;
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Transitioning from %s to %s biome"), 
           *UEnum::GetValueAsString(PreviousBiome), 
           *UEnum::GetValueAsString(NewBiome));
    
    CrossfadeBiomeAudio(PreviousBiome, NewBiome);
    LastBiomeTransitionTime = CachedWorld ? CachedWorld->GetTimeSeconds() : 0.0f;
}

void UAudioManager::CrossfadeBiomeAudio(EBiomeType FromBiome, EBiomeType ToBiome)
{
    if (!AmbientAudioComponent)
    {
        return;
    }
    
    // Get new biome audio data
    const FAudio_BiomeAudioData* NewBiomeData = BiomeAudioMap.Find(ToBiome);
    if (!NewBiomeData)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: No audio data found for biome %s"), 
               *UEnum::GetValueAsString(ToBiome));
        return;
    }
    
    // Apply new ambient volume
    AmbientAudioComponent->SetVolumeMultiplier(NewBiomeData->AmbientVolume);
    
    // Log biome transition
    FString BiomeNames[] = {TEXT("Savanna"), TEXT("Forest"), TEXT("Swamp"), TEXT("Desert"), TEXT("Mountain")};
    int32 BiomeIndex = static_cast<int32>(ToBiome);
    if (BiomeIndex >= 0 && BiomeIndex < 5)
    {
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Now playing %s ambient audio (Volume: %.2f)"), 
               *BiomeNames[BiomeIndex], NewBiomeData->AmbientVolume);
    }
}

void UAudioManager::SetDangerLevel(float DangerAmount)
{
    CurrentAudioState.DangerLevel = FMath::Clamp(DangerAmount, 0.0f, 1.0f);
    
    if (CurrentAudioState.DangerLevel > 0.7f)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: HIGH DANGER - Predator nearby! (Danger: %.2f)"), 
               CurrentAudioState.DangerLevel);
    }
    else if (CurrentAudioState.DangerLevel > 0.3f)
    {
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Moderate danger detected (Danger: %.2f)"), 
               CurrentAudioState.DangerLevel);
    }
    
    UpdateAmbientAudio();
}

void UAudioManager::UpdateAmbientAudio()
{
    if (!AmbientAudioComponent)
    {
        return;
    }
    
    // Adjust volume based on danger level
    const FAudio_BiomeAudioData* BiomeData = BiomeAudioMap.Find(CurrentAudioState.CurrentBiome);
    if (BiomeData)
    {
        float DangerVolumeModifier = 1.0f + (CurrentAudioState.DangerLevel * 0.5f);
        float FinalVolume = BiomeData->AmbientVolume * DangerVolumeModifier;
        AmbientAudioComponent->SetVolumeMultiplier(FinalVolume);
    }
}

void UAudioManager::PlayNarrativeClip(const FString& ClipName, const FVector& Location)
{
    if (!NarrativeAudioComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: No narrative audio component available"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Playing narrative clip '%s' at location %s"), 
           *ClipName, *Location.ToString());
    
    // For now, log the narrative event (actual audio files will be loaded later)
    if (ClipName.Contains(TEXT("Danger")))
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: NARRATIVE - Danger warning played"));
    }
    else if (ClipName.Contains(TEXT("Discovery")))
    {
        UE_LOG(LogTemp, Log, TEXT("AudioManager: NARRATIVE - Discovery audio played"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("AudioManager: NARRATIVE - General clip played"));
    }
}

void UAudioManager::UpdateDynamicAudioState(const FAudio_DynamicAudioState& NewState)
{
    CurrentAudioState = NewState;
    UpdateAmbientAudio();
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Dynamic state updated - Biome: %s, Danger: %.2f, Night: %s"), 
           *UEnum::GetValueAsString(CurrentAudioState.CurrentBiome),
           CurrentAudioState.DangerLevel,
           CurrentAudioState.bIsNightTime ? TEXT("Yes") : TEXT("No"));
}

void UAudioManager::PlayFootstepSound(EBiomeType BiomeType, const FVector& Location, bool bIsHeavy)
{
    FString FootstepType = bIsHeavy ? TEXT("Heavy") : TEXT("Light");
    FString BiomeName = UEnum::GetValueAsString(BiomeType);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("AudioManager: %s footstep in %s at %s"), 
           *FootstepType, *BiomeName, *Location.ToString());
}

void UAudioManager::PlayDinosaurRoar(const FString& DinosaurType, const FVector& Location, float Intensity)
{
    UE_LOG(LogTemp, Log, TEXT("AudioManager: %s roar at %s (Intensity: %.2f)"), 
           *DinosaurType, *Location.ToString(), Intensity);
    
    // Increase danger level when dinosaur roars
    SetDangerLevel(CurrentAudioState.DangerLevel + (Intensity * 0.3f));
}

void UAudioManager::StartWeatherAudio(EWeatherType WeatherType, float Intensity)
{
    CurrentAudioState.WeatherIntensity = Intensity;
    
    if (WeatherAudioComponent)
    {
        WeatherAudioComponent->SetVolumeMultiplier(Intensity);
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Weather audio started - Type: %s, Intensity: %.2f"), 
           *UEnum::GetValueAsString(WeatherType), Intensity);
}

void UAudioManager::StopWeatherAudio()
{
    CurrentAudioState.WeatherIntensity = 0.0f;
    
    if (WeatherAudioComponent && WeatherAudioComponent->IsPlaying())
    {
        WeatherAudioComponent->Stop();
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Weather audio stopped"));
}

void UAudioManager::PlaySurvivalTip(const FString& TipCategory)
{
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Survival tip played - Category: %s"), *TipCategory);
    
    // Trigger narrative audio component for survival tips
    PlayNarrativeClip(FString::Printf(TEXT("SurvivalTip_%s"), *TipCategory), FVector::ZeroVector);
}

void UAudioManager::PlayDangerWarning(const FString& ThreatType, const FVector& ThreatLocation)
{
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: DANGER WARNING - %s threat at %s"), 
           *ThreatType, *ThreatLocation.ToString());
    
    // Play urgent warning audio
    PlayNarrativeClip(FString::Printf(TEXT("DangerWarning_%s"), *ThreatType), ThreatLocation);
    
    // Increase danger level significantly
    SetDangerLevel(0.9f);
}