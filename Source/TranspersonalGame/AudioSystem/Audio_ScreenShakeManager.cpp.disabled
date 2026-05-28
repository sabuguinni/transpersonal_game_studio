#include "Audio_ScreenShakeManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

void UAudio_ScreenShakeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeShakePresets();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeManager: Initialized with shake system ready"));
}

void UAudio_ScreenShakeManager::Deinitialize()
{
    StopAllShakes();
    
    // Clear damage flash timer
    if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(DamageFlashTimerHandle))
    {
        GetWorld()->GetTimerManager().ClearTimer(DamageFlashTimerHandle);
    }
    
    Super::Deinitialize();
}

void UAudio_ScreenShakeManager::InitializeShakePresets()
{
    // Light shake preset
    FAudio_ShakeSettings LightShake;
    LightShake.Duration = 0.3f;
    LightShake.Amplitude = 0.5f;
    LightShake.Frequency = 15.0f;
    LightShake.FadeInTime = 0.05f;
    LightShake.FadeOutTime = 0.2f;
    ShakePresets.Add(EAudio_ShakeIntensity::Light, LightShake);

    // Medium shake preset
    FAudio_ShakeSettings MediumShake;
    MediumShake.Duration = 0.8f;
    MediumShake.Amplitude = 1.2f;
    MediumShake.Frequency = 12.0f;
    MediumShake.FadeInTime = 0.1f;
    MediumShake.FadeOutTime = 0.4f;
    ShakePresets.Add(EAudio_ShakeIntensity::Medium, MediumShake);

    // Heavy shake preset
    FAudio_ShakeSettings HeavyShake;
    HeavyShake.Duration = 1.5f;
    HeavyShake.Amplitude = 2.0f;
    HeavyShake.Frequency = 8.0f;
    HeavyShake.FadeInTime = 0.15f;
    HeavyShake.FadeOutTime = 0.8f;
    ShakePresets.Add(EAudio_ShakeIntensity::Heavy, HeavyShake);

    // Massive shake preset (for T-Rex footsteps)
    FAudio_ShakeSettings MassiveShake;
    MassiveShake.Duration = 2.0f;
    MassiveShake.Amplitude = 3.5f;
    MassiveShake.Frequency = 5.0f;
    MassiveShake.FadeInTime = 0.2f;
    MassiveShake.FadeOutTime = 1.2f;
    ShakePresets.Add(EAudio_ShakeIntensity::Massive, MassiveShake);
}

void UAudio_ScreenShakeManager::TriggerShake(EAudio_ShakeIntensity Intensity, EAudio_ShakeType ShakeType, FVector Location)
{
    if (!bShakeEnabled)
    {
        return;
    }

    FAudio_ShakeSettings Settings = GetShakeSettingsForIntensity(Intensity);
    
    // Modify settings based on shake type
    switch (ShakeType)
    {
        case EAudio_ShakeType::Footstep:
            Settings.Frequency *= 0.7f; // Lower frequency for footsteps
            Settings.Duration *= 0.8f;  // Shorter duration
            break;
        case EAudio_ShakeType::Impact:
            Settings.FadeInTime = 0.0f; // Instant impact
            Settings.Amplitude *= 1.2f; // More intense
            break;
        case EAudio_ShakeType::Explosion:
            Settings.Amplitude *= 1.5f; // Much more intense
            Settings.Duration *= 1.3f;  // Longer lasting
            break;
        case EAudio_ShakeType::Earthquake:
            Settings.Frequency *= 0.5f; // Very low frequency
            Settings.Duration *= 2.0f;  // Much longer
            break;
    }

    ApplyShakeToPlayer(Settings, Location);
}

void UAudio_ScreenShakeManager::TriggerCustomShake(const FAudio_ShakeSettings& Settings, FVector Location)
{
    if (!bShakeEnabled)
    {
        return;
    }

    ApplyShakeToPlayer(Settings, Location);
}

void UAudio_ScreenShakeManager::TriggerDinosaurFootstep(float DinosaurMass, float Distance)
{
    if (!bShakeEnabled || Distance > MaxShakeDistance)
    {
        return;
    }

    // Calculate intensity based on mass
    EAudio_ShakeIntensity Intensity = EAudio_ShakeIntensity::Light;
    
    if (DinosaurMass > 8000.0f) // T-Rex territory (8+ tons)
    {
        Intensity = EAudio_ShakeIntensity::Massive;
    }
    else if (DinosaurMass > 3000.0f) // Large theropods (3-8 tons)
    {
        Intensity = EAudio_ShakeIntensity::Heavy;
    }
    else if (DinosaurMass > 800.0f) // Medium dinosaurs (0.8-3 tons)
    {
        Intensity = EAudio_ShakeIntensity::Medium;
    }
    // Small dinosaurs use Light intensity

    // Get base settings and modify for distance
    FAudio_ShakeSettings Settings = GetShakeSettingsForIntensity(Intensity);
    
    // Apply distance attenuation
    float DistanceAttenuation = FMath::Clamp(1.0f - (Distance / MaxShakeDistance), 0.1f, 1.0f);
    Settings.Amplitude *= DistanceAttenuation;
    
    // Mass scaling
    float MassMultiplier = FMath::Clamp(DinosaurMass / 1000.0f, 0.1f, 15.0f); // Scale by tons
    Settings.Amplitude *= FMath::Sqrt(MassMultiplier); // Square root scaling for realism
    
    ApplyShakeToPlayer(Settings, FVector::ZeroVector);
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur footstep shake: Mass=%.1f kg, Distance=%.1f m, Intensity=%d"), 
           DinosaurMass, Distance, (int32)Intensity);
}

void UAudio_ScreenShakeManager::TriggerDamageFlash(float DamageAmount)
{
    if (DamageAmount <= 0.0f)
    {
        return;
    }

    // Scale flash duration based on damage amount
    float FlashDuration = FMath::Clamp(DamageFlashDuration * (DamageAmount / 100.0f), 0.1f, 1.0f);
    
    // Scale flash color intensity
    FLinearColor FlashColor = DamageFlashColor;
    FlashColor.A = FMath::Clamp(DamageAmount / 50.0f, 0.3f, 0.8f);
    
    StartDamageFlash(FlashDuration, FlashColor);
    
    // Also trigger a light screen shake for impact feedback
    TriggerShake(EAudio_ShakeIntensity::Light, EAudio_ShakeType::Impact);
}

void UAudio_ScreenShakeManager::StopAllShakes()
{
    // Get player controller and stop camera shakes
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APlayerCameraManager* CameraManager = PC->PlayerCameraManager)
            {
                CameraManager->StopAllCameraShakes(false);
            }
        }
    }
    
    EndDamageFlash();
}

void UAudio_ScreenShakeManager::SetShakeEnabled(bool bEnabled)
{
    bShakeEnabled = bEnabled;
    
    if (!bEnabled)
    {
        StopAllShakes();
    }
}

void UAudio_ScreenShakeManager::SetShakeIntensityMultiplier(float Multiplier)
{
    ShakeIntensityMultiplier = FMath::Clamp(Multiplier, 0.0f, 5.0f);
}

void UAudio_ScreenShakeManager::ApplyShakeToPlayer(const FAudio_ShakeSettings& Settings, FVector Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->PlayerCameraManager)
    {
        return;
    }

    // Calculate distance attenuation if location is specified
    float DistanceAttenuation = 1.0f;
    if (Location != FVector::ZeroVector)
    {
        FVector PlayerLocation = PlayerController->GetPawn() ? PlayerController->GetPawn()->GetActorLocation() : FVector::ZeroVector;
        DistanceAttenuation = CalculateDistanceAttenuation(Location, PlayerLocation);
    }

    // Apply intensity multiplier and distance attenuation
    FAudio_ShakeSettings ModifiedSettings = Settings;
    ModifiedSettings.Amplitude *= ShakeIntensityMultiplier * DistanceAttenuation;

    // Create and start camera shake using the built-in camera shake system
    // Note: In a full implementation, you would create a custom UCameraShakeBase subclass
    // For now, we'll use the legacy camera shake approach
    if (ModifiedSettings.Amplitude > 0.01f) // Only shake if amplitude is significant
    {
        PlayerController->PlayerCameraManager->StartCameraShake(
            nullptr, // CameraShakeClass - would need custom class
            ModifiedSettings.Amplitude,
            ECameraShakePlaySpace::CameraLocal,
            FRotator::ZeroRotator
        );
        
        UE_LOG(LogTemp, Warning, TEXT("Screen shake applied: Amplitude=%.2f, Duration=%.2f"), 
               ModifiedSettings.Amplitude, ModifiedSettings.Duration);
    }
}

float UAudio_ScreenShakeManager::CalculateDistanceAttenuation(FVector ShakeLocation, FVector PlayerLocation)
{
    float Distance = FVector::Dist(ShakeLocation, PlayerLocation);
    return FMath::Clamp(1.0f - (Distance / MaxShakeDistance), 0.0f, 1.0f);
}

FAudio_ShakeSettings UAudio_ScreenShakeManager::GetShakeSettingsForIntensity(EAudio_ShakeIntensity Intensity)
{
    if (FAudio_ShakeSettings* FoundSettings = ShakePresets.Find(Intensity))
    {
        return *FoundSettings;
    }
    
    // Fallback to medium shake if not found
    return ShakePresets.FindRef(EAudio_ShakeIntensity::Medium);
}

void UAudio_ScreenShakeManager::StartDamageFlash(float Duration, FLinearColor FlashColor)
{
    if (!GetWorld())
    {
        return;
    }

    bDamageFlashActive = true;
    DamageFlashStartTime = GetWorld()->GetTimeSeconds();
    CurrentDamageFlashDuration = Duration;
    
    // Set up timer to update flash effect
    GetWorld()->GetTimerManager().SetTimer(
        DamageFlashTimerHandle,
        this,
        &UAudio_ScreenShakeManager::UpdateDamageFlash,
        0.016f, // ~60 FPS updates
        true
    );
    
    UE_LOG(LogTemp, Warning, TEXT("Damage flash started: Duration=%.2f, Color=(%f,%f,%f,%f)"), 
           Duration, FlashColor.R, FlashColor.G, FlashColor.B, FlashColor.A);
}

void UAudio_ScreenShakeManager::UpdateDamageFlash()
{
    if (!bDamageFlashActive || !GetWorld())
    {
        EndDamageFlash();
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    float ElapsedTime = CurrentTime - DamageFlashStartTime;
    
    if (ElapsedTime >= CurrentDamageFlashDuration)
    {
        EndDamageFlash();
        return;
    }

    // Calculate fade out alpha (flash intensity decreases over time)
    float Alpha = 1.0f - (ElapsedTime / CurrentDamageFlashDuration);
    Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
    
    // In a full implementation, this would apply the flash effect to the screen
    // For now, we just log the progress
    if (FMath::Fmod(ElapsedTime, 0.1f) < 0.016f) // Log every 0.1 seconds
    {
        UE_LOG(LogTemp, Verbose, TEXT("Damage flash update: Alpha=%.2f, Elapsed=%.2f"), Alpha, ElapsedTime);
    }
}

void UAudio_ScreenShakeManager::EndDamageFlash()
{
    bDamageFlashActive = false;
    
    if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(DamageFlashTimerHandle))
    {
        GetWorld()->GetTimerManager().ClearTimer(DamageFlashTimerHandle);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Damage flash ended"));
}