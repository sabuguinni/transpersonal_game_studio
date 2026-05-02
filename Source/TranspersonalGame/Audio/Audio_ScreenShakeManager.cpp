#include "Audio_ScreenShakeManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UAudio_ScreenShakeManager::UAudio_ScreenShakeManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    CachedPlayerController = nullptr;
}

void UAudio_ScreenShakeManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache player controller
    CachedPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    
    // Initialize shake presets
    InitializeShakePresets();
}

void UAudio_ScreenShakeManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update cached player controller if needed
    if (!CachedPlayerController)
    {
        CachedPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    }
}

void UAudio_ScreenShakeManager::TriggerScreenShake(FVector SourceLocation, EAudio_ShakeIntensity Intensity)
{
    if (!CachedPlayerController)
    {
        return;
    }
    
    // Get shake settings for this intensity
    FAudio_ShakeSettings* ShakeSettings = ShakePresets.Find(Intensity);
    if (!ShakeSettings)
    {
        return;
    }
    
    // Check if player is in range
    if (!IsPlayerInRange(SourceLocation, ShakeSettings->MaxDistance))
    {
        return;
    }
    
    // Calculate distance attenuation
    float Attenuation = CalculateDistanceAttenuation(SourceLocation, ShakeSettings->MaxDistance);
    
    // Apply shake with distance attenuation
    FAudio_ShakeSettings AttenuatedShake = *ShakeSettings;
    AttenuatedShake.Amplitude *= Attenuation;
    
    TriggerCustomShake(SourceLocation, AttenuatedShake);
}

void UAudio_ScreenShakeManager::TriggerCustomShake(FVector SourceLocation, FAudio_ShakeSettings ShakeSettings)
{
    if (!CachedPlayerController)
    {
        return;
    }
    
    // Use UE5's built-in camera shake system
    // For now, we'll use a simple client-side shake
    CachedPlayerController->ClientStartCameraShake(
        nullptr, // Camera shake class (using default)
        ShakeSettings.Amplitude,
        ECameraShakePlaySpace::CameraLocal,
        FRotator::ZeroRotator
    );
    
    // Log shake for debugging
    UE_LOG(LogTemp, Log, TEXT("Audio Screen Shake: Amplitude=%.2f, Duration=%.2f"), 
           ShakeSettings.Amplitude, ShakeSettings.Duration);
}

bool UAudio_ScreenShakeManager::IsPlayerInRange(FVector SourceLocation, float MaxDistance)
{
    if (!CachedPlayerController || !CachedPlayerController->GetPawn())
    {
        return false;
    }
    
    FVector PlayerLocation = CachedPlayerController->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(SourceLocation, PlayerLocation);
    
    return Distance <= MaxDistance;
}

void UAudio_ScreenShakeManager::InitializeShakePresets()
{
    // Light shake (small creatures, distant impacts)
    FAudio_ShakeSettings LightShake;
    LightShake.Duration = 0.3f;
    LightShake.Amplitude = 0.2f;
    LightShake.Frequency = 15.0f;
    LightShake.MaxDistance = 500.0f;
    ShakePresets.Add(EAudio_ShakeIntensity::Light, LightShake);
    
    // Medium shake (medium dinosaurs, nearby impacts)
    FAudio_ShakeSettings MediumShake;
    MediumShake.Duration = 0.6f;
    MediumShake.Amplitude = 0.5f;
    MediumShake.Frequency = 12.0f;
    MediumShake.MaxDistance = 1000.0f;
    ShakePresets.Add(EAudio_ShakeIntensity::Medium, MediumShake);
    
    // Heavy shake (large dinosaurs, T-Rex footsteps)
    FAudio_ShakeSettings HeavyShake;
    HeavyShake.Duration = 1.0f;
    HeavyShake.Amplitude = 1.0f;
    HeavyShake.Frequency = 8.0f;
    HeavyShake.MaxDistance = 1500.0f;
    ShakePresets.Add(EAudio_ShakeIntensity::Heavy, HeavyShake);
    
    // Massive shake (T-Rex roar, massive impacts)
    FAudio_ShakeSettings MassiveShake;
    MassiveShake.Duration = 1.5f;
    MassiveShake.Amplitude = 1.5f;
    MassiveShake.Frequency = 5.0f;
    MassiveShake.MaxDistance = 2000.0f;
    ShakePresets.Add(EAudio_ShakeIntensity::Massive, MassiveShake);
}

float UAudio_ScreenShakeManager::CalculateDistanceAttenuation(FVector SourceLocation, float MaxDistance)
{
    if (!CachedPlayerController || !CachedPlayerController->GetPawn())
    {
        return 0.0f;
    }
    
    FVector PlayerLocation = CachedPlayerController->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(SourceLocation, PlayerLocation);
    
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }
    
    // Linear falloff
    return 1.0f - (Distance / MaxDistance);
}