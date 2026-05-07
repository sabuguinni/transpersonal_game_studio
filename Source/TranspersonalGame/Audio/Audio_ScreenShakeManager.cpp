#include "Audio_ScreenShakeManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/Engine.h"

UAudio_ScreenShakeManager::UAudio_ScreenShakeManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    PlayerController = nullptr;
    CameraShakeClass = nullptr;
    
    InitializeShakeSettings();
}

void UAudio_ScreenShakeManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Get player controller reference
    if (UWorld* World = GetWorld())
    {
        PlayerController = World->GetFirstPlayerController();
        if (!PlayerController)
        {
            UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeManager: No PlayerController found"));
        }
    }
}

void UAudio_ScreenShakeManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAudio_ScreenShakeManager::InitializeShakeSettings()
{
    // T-Rex Footstep - Heavy, slow shake
    FAudio_ScreenShakeSettings TRexSettings;
    TRexSettings.Intensity = 2.0f;
    TRexSettings.Duration = 0.8f;
    TRexSettings.Frequency = 5.0f;
    TRexSettings.bFadeIn = true;
    TRexSettings.bFadeOut = true;
    ShakeTypeSettings.Add(EAudio_ShakeType::TRexFootstep, TRexSettings);

    // Dinosaur Impact - Medium intensity, quick shake
    FAudio_ScreenShakeSettings ImpactSettings;
    ImpactSettings.Intensity = 1.5f;
    ImpactSettings.Duration = 0.4f;
    ImpactSettings.Frequency = 15.0f;
    ImpactSettings.bFadeIn = false;
    ImpactSettings.bFadeOut = true;
    ShakeTypeSettings.Add(EAudio_ShakeType::DinosaurImpact, ImpactSettings);

    // Ground Pound - Very heavy, sustained shake
    FAudio_ScreenShakeSettings GroundPoundSettings;
    GroundPoundSettings.Intensity = 3.0f;
    GroundPoundSettings.Duration = 1.2f;
    GroundPoundSettings.Frequency = 8.0f;
    GroundPoundSettings.bFadeIn = true;
    GroundPoundSettings.bFadeOut = true;
    ShakeTypeSettings.Add(EAudio_ShakeType::GroundPound, GroundPoundSettings);

    // Earthquake - Long, rolling shake
    FAudio_ScreenShakeSettings EarthquakeSettings;
    EarthquakeSettings.Intensity = 2.5f;
    EarthquakeSettings.Duration = 3.0f;
    EarthquakeSettings.Frequency = 3.0f;
    EarthquakeSettings.bFadeIn = true;
    EarthquakeSettings.bFadeOut = true;
    ShakeTypeSettings.Add(EAudio_ShakeType::Earthquake, EarthquakeSettings);

    // Explosion - Sharp, intense shake
    FAudio_ScreenShakeSettings ExplosionSettings;
    ExplosionSettings.Intensity = 4.0f;
    ExplosionSettings.Duration = 0.3f;
    ExplosionSettings.Frequency = 25.0f;
    ExplosionSettings.bFadeIn = false;
    ExplosionSettings.bFadeOut = true;
    ShakeTypeSettings.Add(EAudio_ShakeType::Explosion, ExplosionSettings);
}

void UAudio_ScreenShakeManager::TriggerScreenShake(EAudio_ShakeType ShakeType, float IntensityMultiplier)
{
    if (const FAudio_ScreenShakeSettings* Settings = ShakeTypeSettings.Find(ShakeType))
    {
        ExecuteScreenShake(*Settings, IntensityMultiplier);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeManager: No settings found for shake type"));
    }
}

void UAudio_ScreenShakeManager::TriggerCustomScreenShake(const FAudio_ScreenShakeSettings& ShakeSettings)
{
    ExecuteScreenShake(ShakeSettings, 1.0f);
}

void UAudio_ScreenShakeManager::TriggerProximityScreenShake(FVector SourceLocation, float MaxDistance, EAudio_ShakeType ShakeType)
{
    if (!IsPlayerInShakeRange(SourceLocation, MaxDistance))
    {
        return;
    }

    float DistanceIntensity = CalculateDistanceBasedIntensity(SourceLocation, MaxDistance);
    TriggerScreenShake(ShakeType, DistanceIntensity);
}

bool UAudio_ScreenShakeManager::IsPlayerInShakeRange(FVector SourceLocation, float MaxDistance) const
{
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return false;
    }

    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, SourceLocation);
    
    return Distance <= MaxDistance;
}

void UAudio_ScreenShakeManager::ExecuteScreenShake(const FAudio_ScreenShakeSettings& Settings, float IntensityMultiplier)
{
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeManager: No PlayerController available for screen shake"));
        return;
    }

    // Create a simple camera shake using the built-in functionality
    // Since we don't have a specific camera shake class, we'll use the player controller's client shake
    float FinalIntensity = Settings.Intensity * IntensityMultiplier;
    
    // Clamp intensity to reasonable values
    FinalIntensity = FMath::Clamp(FinalIntensity, 0.1f, 10.0f);
    
    // Use the player controller's shake functionality
    if (PlayerController)
    {
        // Create a basic shake effect
        PlayerController->ClientStartCameraShake(nullptr, FinalIntensity);
        
        UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeManager: Triggered screen shake - Intensity: %f, Duration: %f"), 
               FinalIntensity, Settings.Duration);
    }
}

float UAudio_ScreenShakeManager::CalculateDistanceBasedIntensity(FVector SourceLocation, float MaxDistance) const
{
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return 0.0f;
    }

    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, SourceLocation);
    
    // Calculate intensity falloff (1.0 at source, 0.0 at max distance)
    float NormalizedDistance = FMath::Clamp(Distance / MaxDistance, 0.0f, 1.0f);
    float Intensity = 1.0f - NormalizedDistance;
    
    // Apply a curve to make the falloff more natural
    Intensity = FMath::Pow(Intensity, 1.5f);
    
    return Intensity;
}