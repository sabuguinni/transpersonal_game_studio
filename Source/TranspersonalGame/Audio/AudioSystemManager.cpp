#include "AudioSystemManager.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "AudioParameterControllerInterface.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "../Characters/DinosaurCharacter.h"
#include "../Characters/PlayerCharacter.h"

// MetaSound parameter names
const FName UAudioSystemManager::PARAM_ThreatLevel = "ThreatLevel";
const FName UAudioSystemManager::PARAM_EnvironmentType = "EnvironmentType";
const FName UAudioSystemManager::PARAM_TimeOfDay = "TimeOfDay";
const FName UAudioSystemManager::PARAM_StealthLevel = "StealthLevel";
const FName UAudioSystemManager::PARAM_WeatherIntensity = "WeatherIntensity";
const FName UAudioSystemManager::PARAM_DinosaurProximity = "DinosaurProximity";

UAudioSystemManager::UAudioSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Create audio components
    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    ThreatAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ThreatAudioComponent"));

    // Set default audio state
    CurrentAudioState.EnvironmentType = EAudioEnvironmentType::DenseForest;
    CurrentAudioState.ThreatLevel = EThreatLevel::Safe;
    CurrentAudioState.TimeOfDay = ETimeOfDay::Morning;
    CurrentAudioState.PlayerStealthLevel = 0.0f;
    CurrentAudioState.WeatherIntensity = 0.0f;
    CurrentAudioState.bIsInPlayerBase = false;
    CurrentAudioState.NearbyDinosaurCount = 0;
    CurrentAudioState.DistanceToNearestPredator = 10000.0f;
}

void UAudioSystemManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize audio components
    if (MusicAudioComponent && AdaptiveMusicMetaSound)
    {
        MusicAudioComponent->SetSound(AdaptiveMusicMetaSound);
        MusicAudioComponent->bAutoActivate = true;
        MusicAudioComponent->Play();
    }

    if (AmbientAudioComponent && AmbientEnvironmentMetaSound)
    {
        AmbientAudioComponent->SetSound(AmbientEnvironmentMetaSound);
        AmbientAudioComponent->bAutoActivate = true;
        AmbientAudioComponent->Play();
    }

    if (ThreatAudioComponent && ThreatMusicMetaSound)
    {
        ThreatAudioComponent->SetSound(ThreatMusicMetaSound);
        ThreatAudioComponent->bAutoActivate = false; // Only play when threatened
    }

    // Initialize parameters
    UpdateMusicParameters();
    UpdateAmbientParameters();
    UpdateThreatParameters();
}

void UAudioSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Detect nearby threats periodically
    LastThreatUpdate += DeltaTime;
    if (LastThreatUpdate >= ThreatUpdateInterval)
    {
        DetectNearbyThreats();
        CalculateStealthLevel();
        LastThreatUpdate = 0.0f;
    }

    // Handle state transitions
    if (bIsTransitioning)
    {
        float TransitionProgress = (GetWorld()->GetTimeSeconds() - TransitionStartTime) / MusicTransitionTime;
        if (TransitionProgress >= 1.0f)
        {
            bIsTransitioning = false;
            PreviousAudioState = CurrentAudioState;
        }
    }

    // Update audio parameters continuously
    UpdateMusicParameters();
    UpdateAmbientParameters();
    UpdateThreatParameters();
}

void UAudioSystemManager::UpdateAudioState(const FAudioStateData& NewState)
{
    if (CurrentAudioState.ThreatLevel != NewState.ThreatLevel ||
        CurrentAudioState.EnvironmentType != NewState.EnvironmentType ||
        CurrentAudioState.TimeOfDay != NewState.TimeOfDay)
    {
        PreviousAudioState = CurrentAudioState;
        CurrentAudioState = NewState;
        bIsTransitioning = true;
        TransitionStartTime = GetWorld()->GetTimeSeconds();

        UE_LOG(LogTemp, Log, TEXT("Audio state updated: Threat=%d, Environment=%d, Time=%d"), 
               (int32)NewState.ThreatLevel, (int32)NewState.EnvironmentType, (int32)NewState.TimeOfDay);
    }
    else
    {
        CurrentAudioState = NewState;
    }
}

void UAudioSystemManager::SetThreatLevel(EThreatLevel NewThreatLevel)
{
    if (CurrentAudioState.ThreatLevel != NewThreatLevel)
    {
        CurrentAudioState.ThreatLevel = NewThreatLevel;
        
        // Handle threat audio component activation
        if (NewThreatLevel >= EThreatLevel::Danger && ThreatAudioComponent && !ThreatAudioComponent->IsPlaying())
        {
            ThreatAudioComponent->Play();
        }
        else if (NewThreatLevel < EThreatLevel::Danger && ThreatAudioComponent && ThreatAudioComponent->IsPlaying())
        {
            ThreatAudioComponent->FadeOut(MusicTransitionTime, 0.0f);
        }

        UE_LOG(LogTemp, Log, TEXT("Threat level changed to: %d"), (int32)NewThreatLevel);
    }
}

void UAudioSystemManager::SetEnvironmentType(EAudioEnvironmentType NewEnvironment)
{
    if (CurrentAudioState.EnvironmentType != NewEnvironment)
    {
        CurrentAudioState.EnvironmentType = NewEnvironment;
        UE_LOG(LogTemp, Log, TEXT("Environment changed to: %d"), (int32)NewEnvironment);
    }
}

void UAudioSystemManager::OnDinosaurSpotted(ADinosaurCharacter* Dinosaur, float Distance)
{
    if (!Dinosaur) return;

    // Increase threat level based on dinosaur type and distance
    EThreatLevel NewThreatLevel = CurrentAudioState.ThreatLevel;

    if (Distance < 500.0f) // Very close
    {
        if (Dinosaur->GetDinosaurType() == EDinosaurType::LargeCarnivore)
        {
            NewThreatLevel = EThreatLevel::Terror;
        }
        else if (Dinosaur->GetDinosaurType() == EDinosaurType::MediumCarnivore)
        {
            NewThreatLevel = EThreatLevel::Danger;
        }
        else
        {
            NewThreatLevel = EThreatLevel::Tense;
        }
    }
    else if (Distance < 1500.0f) // Medium distance
    {
        if (Dinosaur->GetDinosaurType() == EDinosaurType::LargeCarnivore)
        {
            NewThreatLevel = EThreatLevel::Danger;
        }
        else if (Dinosaur->GetDinosaurType() == EDinosaurType::MediumCarnivore)
        {
            NewThreatLevel = EThreatLevel::Tense;
        }
        else
        {
            NewThreatLevel = EThreatLevel::Cautious;
        }
    }
    else // Far distance
    {
        NewThreatLevel = EThreatLevel::Cautious;
    }

    SetThreatLevel(NewThreatLevel);
    CurrentAudioState.DistanceToNearestPredator = FMath::Min(CurrentAudioState.DistanceToNearestPredator, Distance);
}

void UAudioSystemManager::OnPlayerEnterStealth()
{
    CurrentAudioState.PlayerStealthLevel = 1.0f;
    UE_LOG(LogTemp, Log, TEXT("Player entered stealth - audio adjusted"));
}

void UAudioSystemManager::OnPlayerExitStealth()
{
    CurrentAudioState.PlayerStealthLevel = 0.0f;
    UE_LOG(LogTemp, Log, TEXT("Player exited stealth - audio restored"));
}

void UAudioSystemManager::PlayDinosaurVocalization(ADinosaurCharacter* Dinosaur, const FString& VocalizationType)
{
    if (!Dinosaur) return;

    // This will be implemented to trigger specific dinosaur sound effects
    // For now, just log the event
    UE_LOG(LogTemp, Log, TEXT("Dinosaur vocalization: %s - %s"), 
           *Dinosaur->GetName(), *VocalizationType);
}

void UAudioSystemManager::UpdateMusicParameters()
{
    if (!MusicAudioComponent) return;

    // Set MetaSound parameters
    MusicAudioComponent->SetFloatParameter(PARAM_ThreatLevel, (float)CurrentAudioState.ThreatLevel);
    MusicAudioComponent->SetFloatParameter(PARAM_EnvironmentType, (float)CurrentAudioState.EnvironmentType);
    MusicAudioComponent->SetFloatParameter(PARAM_TimeOfDay, (float)CurrentAudioState.TimeOfDay);
    MusicAudioComponent->SetFloatParameter(PARAM_StealthLevel, CurrentAudioState.PlayerStealthLevel);
    MusicAudioComponent->SetFloatParameter(PARAM_WeatherIntensity, CurrentAudioState.WeatherIntensity);
    
    // Calculate proximity factor (0.0 = far, 1.0 = very close)
    float ProximityFactor = 1.0f - FMath::Clamp(CurrentAudioState.DistanceToNearestPredator / ThreatDetectionRadius, 0.0f, 1.0f);
    MusicAudioComponent->SetFloatParameter(PARAM_DinosaurProximity, ProximityFactor);
}

void UAudioSystemManager::UpdateAmbientParameters()
{
    if (!AmbientAudioComponent) return;

    AmbientAudioComponent->SetFloatParameter(PARAM_EnvironmentType, (float)CurrentAudioState.EnvironmentType);
    AmbientAudioComponent->SetFloatParameter(PARAM_TimeOfDay, (float)CurrentAudioState.TimeOfDay);
    AmbientAudioComponent->SetFloatParameter(PARAM_WeatherIntensity, CurrentAudioState.WeatherIntensity);
    
    // Reduce ambient volume when in stealth
    float AmbientVolume = 1.0f - (CurrentAudioState.PlayerStealthLevel * StealthAudioReduction);
    AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume);
}

void UAudioSystemManager::UpdateThreatParameters()
{
    if (!ThreatAudioComponent) return;

    ThreatAudioComponent->SetFloatParameter(PARAM_ThreatLevel, (float)CurrentAudioState.ThreatLevel);
    
    float ProximityFactor = 1.0f - FMath::Clamp(CurrentAudioState.DistanceToNearestPredator / ThreatDetectionRadius, 0.0f, 1.0f);
    ThreatAudioComponent->SetFloatParameter(PARAM_DinosaurProximity, ProximityFactor);
    
    // Set threat audio volume based on threat level
    float ThreatVolume = FMath::Clamp((float)CurrentAudioState.ThreatLevel / (float)EThreatLevel::Terror, 0.0f, 1.0f);
    ThreatAudioComponent->SetVolumeMultiplier(ThreatVolume);
}

void UAudioSystemManager::DetectNearbyThreats()
{
    if (!GetWorld()) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    CurrentAudioState.NearbyDinosaurCount = 0;
    CurrentAudioState.DistanceToNearestPredator = 10000.0f;

    // Find all dinosaurs in the world
    TArray<AActor*> FoundDinosaurs;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADinosaurCharacter::StaticClass(), FoundDinosaurs);

    EThreatLevel HighestThreat = EThreatLevel::Safe;

    for (AActor* Actor : FoundDinosaurs)
    {
        ADinosaurCharacter* Dinosaur = Cast<ADinosaurCharacter>(Actor);
        if (!Dinosaur) continue;

        float Distance = FVector::Dist(PlayerLocation, Dinosaur->GetActorLocation());
        
        if (Distance <= ThreatDetectionRadius)
        {
            CurrentAudioState.NearbyDinosaurCount++;
            CurrentAudioState.DistanceToNearestPredator = FMath::Min(CurrentAudioState.DistanceToNearestPredator, Distance);

            // Determine threat level based on dinosaur type and distance
            if (Dinosaur->GetDinosaurType() == EDinosaurType::LargeCarnivore)
            {
                if (Distance < 500.0f) HighestThreat = FMath::Max(HighestThreat, EThreatLevel::Terror);
                else if (Distance < 1000.0f) HighestThreat = FMath::Max(HighestThreat, EThreatLevel::Danger);
                else HighestThreat = FMath::Max(HighestThreat, EThreatLevel::Tense);
            }
            else if (Dinosaur->GetDinosaurType() == EDinosaurType::MediumCarnivore)
            {
                if (Distance < 300.0f) HighestThreat = FMath::Max(HighestThreat, EThreatLevel::Danger);
                else if (Distance < 800.0f) HighestThreat = FMath::Max(HighestThreat, EThreatLevel::Tense);
                else HighestThreat = FMath::Max(HighestThreat, EThreatLevel::Cautious);
            }
        }
    }

    // Only increase threat level, don't decrease it immediately (let it decay naturally)
    if (HighestThreat > CurrentAudioState.ThreatLevel)
    {
        SetThreatLevel(HighestThreat);
    }
}

void UAudioSystemManager::CalculateStealthLevel()
{
    // This would integrate with the stealth system
    // For now, maintain current stealth level
    // In the full implementation, this would check:
    // - Player movement speed
    // - Player visibility to dinosaurs
    // - Environmental cover
    // - Time of day
}