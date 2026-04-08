#include "AudioManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"

UAudioManager::UAudioManager()
{
    CurrentEmotionalState = EEmotionalState::Calm;
    CurrentEnvironment = EEnvironmentType::Forest;
    CurrentTimeOfDay = 0.5f; // Start at noon
    CurrentWeatherIntensity = 0.0f; // Start clear
    
    // Initialize tension parameters
    CurrentTensionParams.ThreatProximity = 0.0f;
    CurrentTensionParams.ThreatLevel = 0.0f;
    CurrentTensionParams.PlayerHeartRate = 60.0f;
    CurrentTensionParams.bIsHiding = false;
    CurrentTensionParams.bIsBeingHunted = false;
}

void UAudioManager::Initialize(UWorld* World)
{
    GameWorld = World;
    
    if (!GameWorld)
    {
        UE_LOG(LogTemp, Error, TEXT("AudioManager: Failed to initialize - World is null"));
        return;
    }

    // Create audio components
    AActor* AudioManagerActor = UGameplayStatics::GetActorOfClass(GameWorld, AActor::StaticClass());
    if (!AudioManagerActor)
    {
        // Create a dummy actor to hold our audio components
        AudioManagerActor = GameWorld->SpawnActor<AActor>();
        AudioManagerActor->SetActorLabel(TEXT("AudioManagerActor"));
    }

    MusicComponent = AudioManagerActor->CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComponent"));
    AmbienceComponent = AudioManagerActor->CreateDefaultSubobject<UAudioComponent>(TEXT("AmbienceComponent"));
    TensionComponent = AudioManagerActor->CreateDefaultSubobject<UAudioComponent>(TEXT("TensionComponent"));

    // Set up audio components
    if (MusicComponent)
    {
        MusicComponent->bAutoActivate = true;
        MusicComponent->SetVolumeMultiplier(0.7f);
    }

    if (AmbienceComponent)
    {
        AmbienceComponent->bAutoActivate = true;
        AmbienceComponent->SetVolumeMultiplier(0.8f);
    }

    if (TensionComponent)
    {
        TensionComponent->bAutoActivate = false;
        TensionComponent->SetVolumeMultiplier(0.6f);
    }

    UE_LOG(LogTemp, Log, TEXT("AudioManager: Initialized successfully"));
}

void UAudioManager::UpdateEmotionalState(EEmotionalState NewState, float TransitionTime)
{
    if (CurrentEmotionalState == NewState)
        return;

    UE_LOG(LogTemp, Log, TEXT("AudioManager: Transitioning from %d to %d over %.2f seconds"), 
           (int32)CurrentEmotionalState, (int32)NewState, TransitionTime);

    CurrentEmotionalState = NewState;
    UpdateMusicParameters();
    UpdateTensionParameters();
}

void UAudioManager::UpdateEnvironmentType(EEnvironmentType NewEnvironment)
{
    if (CurrentEnvironment == NewEnvironment)
        return;

    UE_LOG(LogTemp, Log, TEXT("AudioManager: Environment changed to %d"), (int32)NewEnvironment);

    CurrentEnvironment = NewEnvironment;
    UpdateAmbienceParameters();
}

void UAudioManager::UpdateTensionParameters(const FTensionParameters& NewParameters)
{
    CurrentTensionParams = NewParameters;
    
    // Calculate dynamic tension level
    float TensionLevel = CalculateTensionLevel();
    
    // Update MetaSound parameters
    if (TensionComponent && TensionSystemMetaSound)
    {
        TensionComponent->SetFloatParameter(FName("TensionLevel"), TensionLevel);
        TensionComponent->SetFloatParameter(FName("ThreatProximity"), NewParameters.ThreatProximity);
        TensionComponent->SetFloatParameter(FName("HeartRate"), NewParameters.PlayerHeartRate);
        TensionComponent->SetBoolParameter(FName("IsHiding"), NewParameters.bIsHiding);
        TensionComponent->SetBoolParameter(FName("IsBeingHunted"), NewParameters.bIsBeingHunted);
    }

    // Auto-transition emotional state based on tension
    if (TensionLevel > 0.8f)
    {
        UpdateEmotionalState(EEmotionalState::Terror, 0.5f);
    }
    else if (TensionLevel > 0.6f)
    {
        UpdateEmotionalState(EEmotionalState::Danger, 1.0f);
    }
    else if (TensionLevel > 0.3f)
    {
        UpdateEmotionalState(EEmotionalState::Tension, 1.5f);
    }
    else if (TensionLevel < 0.1f && CurrentEmotionalState != EEmotionalState::Calm)
    {
        UpdateEmotionalState(EEmotionalState::Relief, 3.0f);
    }
}

void UAudioManager::PlayDinosaurSound(FVector Location, FString DinosaurSpecies, FString SoundType)
{
    // This will be expanded to handle procedural dinosaur audio
    // For now, log the event for debugging
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Dinosaur sound - Species: %s, Type: %s, Location: %s"), 
           *DinosaurSpecies, *SoundType, *Location.ToString());

    // Calculate distance-based parameters
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GameWorld, 0);
    if (PlayerPawn)
    {
        float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Location);
        
        // Update tension based on proximity to dinosaur sounds
        if (SoundType.Contains("Roar") || SoundType.Contains("Growl"))
        {
            float ThreatProximity = FMath::Clamp(1.0f - (Distance / 5000.0f), 0.0f, 1.0f);
            
            FTensionParameters UpdatedParams = CurrentTensionParams;
            UpdatedParams.ThreatProximity = FMath::Max(UpdatedParams.ThreatProximity, ThreatProximity);
            UpdatedParams.ThreatLevel = FMath::Max(UpdatedParams.ThreatLevel, ThreatProximity * 0.8f);
            
            UpdateTensionParameters(UpdatedParams);
        }
    }
}

void UAudioManager::SetTimeOfDay(float TimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(TimeOfDay, 0.0f, 1.0f);
    
    // Update ambience for time of day
    if (AmbienceComponent && EnvironmentAmbienceMetaSound)
    {
        AmbienceComponent->SetFloatParameter(FName("TimeOfDay"), CurrentTimeOfDay);
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Time of day updated to %.2f"), CurrentTimeOfDay);
}

void UAudioManager::SetWeatherIntensity(float Intensity)
{
    CurrentWeatherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    // Update ambience for weather
    if (AmbienceComponent && EnvironmentAmbienceMetaSound)
    {
        AmbienceComponent->SetFloatParameter(FName("WeatherIntensity"), CurrentWeatherIntensity);
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Weather intensity updated to %.2f"), CurrentWeatherIntensity);
}

void UAudioManager::UpdateMusicParameters()
{
    if (!MusicComponent || !AdaptiveMusicMetaSound)
        return;

    float MusicalIntensity = CalculateMusicalIntensity();
    
    // Map emotional states to musical parameters
    switch (CurrentEmotionalState)
    {
        case EEmotionalState::Calm:
            MusicComponent->SetFloatParameter(FName("Intensity"), 0.2f);
            MusicComponent->SetFloatParameter(FName("Tension"), 0.1f);
            MusicComponent->SetFloatParameter(FName("Tempo"), 60.0f);
            break;
            
        case EEmotionalState::Tension:
            MusicComponent->SetFloatParameter(FName("Intensity"), 0.4f);
            MusicComponent->SetFloatParameter(FName("Tension"), 0.5f);
            MusicComponent->SetFloatParameter(FName("Tempo"), 80.0f);
            break;
            
        case EEmotionalState::Danger:
            MusicComponent->SetFloatParameter(FName("Intensity"), 0.7f);
            MusicComponent->SetFloatParameter(FName("Tension"), 0.8f);
            MusicComponent->SetFloatParameter(FName("Tempo"), 120.0f);
            break;
            
        case EEmotionalState::Terror:
            MusicComponent->SetFloatParameter(FName("Intensity"), 1.0f);
            MusicComponent->SetFloatParameter(FName("Tension"), 1.0f);
            MusicComponent->SetFloatParameter(FName("Tempo"), 140.0f);
            break;
            
        case EEmotionalState::Relief:
            MusicComponent->SetFloatParameter(FName("Intensity"), 0.3f);
            MusicComponent->SetFloatParameter(FName("Tension"), 0.0f);
            MusicComponent->SetFloatParameter(FName("Tempo"), 70.0f);
            break;
    }
}

void UAudioManager::UpdateAmbienceParameters()
{
    if (!AmbienceComponent || !EnvironmentAmbienceMetaSound)
        return;

    // Map environment types to ambience parameters
    switch (CurrentEnvironment)
    {
        case EEnvironmentType::Forest:
            AmbienceComponent->SetFloatParameter(FName("ForestDensity"), 1.0f);
            AmbienceComponent->SetFloatParameter(FName("OpenSpace"), 0.2f);
            AmbienceComponent->SetFloatParameter(FName("WaterProximity"), 0.0f);
            break;
            
        case EEnvironmentType::Clearing:
            AmbienceComponent->SetFloatParameter(FName("ForestDensity"), 0.3f);
            AmbienceComponent->SetFloatParameter(FName("OpenSpace"), 1.0f);
            AmbienceComponent->SetFloatParameter(FName("WaterProximity"), 0.0f);
            break;
            
        case EEnvironmentType::Water:
            AmbienceComponent->SetFloatParameter(FName("ForestDensity"), 0.5f);
            AmbienceComponent->SetFloatParameter(FName("OpenSpace"), 0.7f);
            AmbienceComponent->SetFloatParameter(FName("WaterProximity"), 1.0f);
            break;
            
        case EEnvironmentType::Cave:
            AmbienceComponent->SetFloatParameter(FName("ForestDensity"), 0.0f);
            AmbienceComponent->SetFloatParameter(FName("OpenSpace"), 0.1f);
            AmbienceComponent->SetFloatParameter(FName("WaterProximity"), 0.3f);
            AmbienceComponent->SetFloatParameter(FName("CaveReverb"), 1.0f);
            break;
            
        case EEnvironmentType::Cliff:
            AmbienceComponent->SetFloatParameter(FName("ForestDensity"), 0.2f);
            AmbienceComponent->SetFloatParameter(FName("OpenSpace"), 0.9f);
            AmbienceComponent->SetFloatParameter(FName("WaterProximity"), 0.0f);
            AmbienceComponent->SetFloatParameter(FName("WindIntensity"), 0.8f);
            break;
    }
}

void UAudioManager::UpdateTensionParameters()
{
    // This method handles the tension-specific audio updates
    // Called when emotional state changes
}

float UAudioManager::CalculateTensionLevel()
{
    float BaseTension = CurrentTensionParams.ThreatLevel;
    float ProximityMultiplier = FMath::Pow(CurrentTensionParams.ThreatProximity, 0.5f);
    float HeartRateInfluence = (CurrentTensionParams.PlayerHeartRate - 60.0f) / 80.0f; // Normalize 60-140 BPM
    
    float TensionLevel = BaseTension * ProximityMultiplier;
    
    // Modifiers
    if (CurrentTensionParams.bIsBeingHunted)
        TensionLevel += 0.3f;
        
    if (CurrentTensionParams.bIsHiding)
        TensionLevel *= 0.7f; // Reduce tension when hiding
        
    TensionLevel += HeartRateInfluence * 0.2f;
    
    return FMath::Clamp(TensionLevel, 0.0f, 1.0f);
}

float UAudioManager::CalculateMusicalIntensity()
{
    float BaseIntensity = 0.0f;
    
    switch (CurrentEmotionalState)
    {
        case EEmotionalState::Calm: BaseIntensity = 0.2f; break;
        case EEmotionalState::Tension: BaseIntensity = 0.4f; break;
        case EEmotionalState::Danger: BaseIntensity = 0.7f; break;
        case EEmotionalState::Terror: BaseIntensity = 1.0f; break;
        case EEmotionalState::Relief: BaseIntensity = 0.3f; break;
    }
    
    // Modify based on environment and time
    float EnvironmentModifier = 1.0f;
    if (CurrentEnvironment == EEnvironmentType::Cave)
        EnvironmentModifier = 1.2f; // Caves are more tense
    else if (CurrentEnvironment == EEnvironmentType::Clearing)
        EnvironmentModifier = 0.8f; // Open areas feel safer
        
    float TimeModifier = 1.0f;
    if (CurrentTimeOfDay < 0.2f || CurrentTimeOfDay > 0.8f) // Night time
        TimeModifier = 1.3f;
        
    return FMath::Clamp(BaseIntensity * EnvironmentModifier * TimeModifier, 0.0f, 1.0f);
}