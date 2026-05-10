#include "Audio_ScreenShakeController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UAudio_ScreenShakeController::UAudio_ScreenShakeController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    bIsShaking = false;
    CurrentShakeTime = 0.0f;
    CachedPlayerController = nullptr;
    
    InitializeShakeSettings();
}

void UAudio_ScreenShakeController::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache player controller
    CachedPlayerController = GetPlayerController();
    
    if (CachedPlayerController)
    {
        UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: Player controller cached successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeController: Could not cache player controller"));
    }
}

void UAudio_ScreenShakeController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsShaking)
    {
        CurrentShakeTime += DeltaTime;
    }
}

void UAudio_ScreenShakeController::TriggerTRexProximityShake(float Distance)
{
    FAudio_ShakeSettings ShakeToUse;
    
    if (Distance <= CloseDistance)
    {
        ShakeToUse = TRexCloseShake;
        UE_LOG(LogTemp, Log, TEXT("T-Rex Close Proximity Shake triggered - Distance: %f"), Distance);
    }
    else if (Distance <= MediumDistance)
    {
        ShakeToUse = TRexMediumShake;
        UE_LOG(LogTemp, Log, TEXT("T-Rex Medium Proximity Shake triggered - Distance: %f"), Distance);
    }
    else if (Distance <= FarDistance)
    {
        ShakeToUse = TRexFarShake;
        UE_LOG(LogTemp, Log, TEXT("T-Rex Far Proximity Shake triggered - Distance: %f"), Distance);
    }
    else
    {
        // Too far, no shake
        return;
    }
    
    ExecuteShake(ShakeToUse);
}

void UAudio_ScreenShakeController::TriggerFootstepShake(EAudio_ShakeIntensity Intensity)
{
    FAudio_ShakeSettings ShakeToUse;
    
    switch (Intensity)
    {
        case EAudio_ShakeIntensity::Light:
        case EAudio_ShakeIntensity::Medium:
            ShakeToUse = FootstepLightShake;
            UE_LOG(LogTemp, Log, TEXT("Light Footstep Shake triggered"));
            break;
            
        case EAudio_ShakeIntensity::Heavy:
        case EAudio_ShakeIntensity::Extreme:
            ShakeToUse = FootstepHeavyShake;
            UE_LOG(LogTemp, Log, TEXT("Heavy Footstep Shake triggered"));
            break;
    }
    
    ExecuteShake(ShakeToUse);
}

void UAudio_ScreenShakeController::TriggerCustomShake(const FAudio_ShakeSettings& Settings)
{
    UE_LOG(LogTemp, Log, TEXT("Custom Shake triggered - Duration: %f, Amplitude: %f"), Settings.Duration, Settings.Amplitude);
    ExecuteShake(Settings);
}

void UAudio_ScreenShakeController::StopAllShakes()
{
    if (CachedPlayerController)
    {
        CachedPlayerController->ClientStopCameraShake(UCameraShakeBase::StaticClass());
        bIsShaking = false;
        CurrentShakeTime = 0.0f;
        UE_LOG(LogTemp, Log, TEXT("All camera shakes stopped"));
    }
}

void UAudio_ScreenShakeController::InitializeShakeSettings()
{
    // T-Rex proximity shakes
    TRexCloseShake.Duration = 2.0f;
    TRexCloseShake.Amplitude = 1.5f;
    TRexCloseShake.Frequency = 15.0f;
    TRexCloseShake.bFadeOut = true;
    
    TRexMediumShake.Duration = 1.5f;
    TRexMediumShake.Amplitude = 1.0f;
    TRexMediumShake.Frequency = 12.0f;
    TRexMediumShake.bFadeOut = true;
    
    TRexFarShake.Duration = 1.0f;
    TRexFarShake.Amplitude = 0.5f;
    TRexFarShake.Frequency = 8.0f;
    TRexFarShake.bFadeOut = true;
    
    // Footstep shakes
    FootstepLightShake.Duration = 0.3f;
    FootstepLightShake.Amplitude = 0.2f;
    FootstepLightShake.Frequency = 20.0f;
    FootstepLightShake.bFadeOut = true;
    
    FootstepHeavyShake.Duration = 0.8f;
    FootstepHeavyShake.Amplitude = 0.8f;
    FootstepHeavyShake.Frequency = 15.0f;
    FootstepHeavyShake.bFadeOut = true;
}

void UAudio_ScreenShakeController::ExecuteShake(const FAudio_ShakeSettings& Settings)
{
    APlayerController* PC = GetPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot execute shake - no player controller"));
        return;
    }
    
    // For now, we'll use a simple camera shake approach
    // In a full implementation, we'd create custom camera shake classes
    bIsShaking = true;
    CurrentShakeTime = 0.0f;
    
    // Log the shake execution for debugging
    UE_LOG(LogTemp, Log, TEXT("Executing screen shake - Duration: %f, Amplitude: %f, Frequency: %f"), 
           Settings.Duration, Settings.Amplitude, Settings.Frequency);
    
    // This is a placeholder - in production we'd use actual camera shake classes
    // PC->ClientStartCameraShake(CustomShakeClass, Settings.Amplitude);
}

APlayerController* UAudio_ScreenShakeController::GetPlayerController()
{
    if (CachedPlayerController && IsValid(CachedPlayerController))
    {
        return CachedPlayerController;
    }
    
    UWorld* World = GetWorld();
    if (World)
    {
        CachedPlayerController = World->GetFirstPlayerController();
        return CachedPlayerController;
    }
    
    return nullptr;
}