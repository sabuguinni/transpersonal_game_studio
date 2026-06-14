#include "Audio_ScreenEffectsManager.h"
#include "Engine/World.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

UAudio_ScreenEffectsManager::UAudio_ScreenEffectsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS
    
    // Initialize default values
    CurrentShakeIntensity = 0.0f;
    ShakeDecayRate = 2.0f;
    MaxShakeDistance = 5000.0f;
    
    DamageFlashDuration = 0.3f;
    CurrentFlashIntensity = 0.0f;
    
    CurrentTimeOfDay = 0.5f; // Noon
    bNightVisionActive = false;
    
    CurrentWeatherIntensity = 0.0f;
    CurrentWeatherType = EWeatherType::Clear;
    CurrentFearLevel = 0.0f;
    
    // Initialize timers
    ShakeTimer = 0.0f;
    ShakeDuration = 0.0f;
    ShakeFrequency = 10.0f;
    FlashTimer = 0.0f;
    FlashColor = FLinearColor::Red;
}

void UAudio_ScreenEffectsManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find post process volume in the level
    TArray<AActor*> PostProcessVolumes;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APostProcessVolume::StaticClass(), PostProcessVolumes);
    
    if (PostProcessVolumes.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(PostProcessVolumes[0]);
    }
    
    // Load global effects material parameter collection
    GlobalEffectsCollection = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Game/TranspersonalGame/Materials/MPC_GlobalEffects"));
}

void UAudio_ScreenEffectsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateScreenShake(DeltaTime);
    UpdateDamageFlash(DeltaTime);
    ApplyPostProcessEffects();
}

void UAudio_ScreenEffectsManager::TriggerScreenShake(float Intensity, float Duration, float Frequency)
{
    CurrentShakeIntensity = FMath::Max(CurrentShakeIntensity, Intensity);
    ShakeDuration = Duration;
    ShakeTimer = 0.0f;
    ShakeFrequency = Frequency;
    
    // Apply camera shake to player controller
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->PlayerCameraManager)
    {
        PC->PlayerCameraManager->StartCameraShake(nullptr, Intensity);
    }
}

void UAudio_ScreenEffectsManager::TriggerDinosaurProximityShake(float Distance, float DinosaurSize)
{
    if (Distance > MaxShakeDistance)
        return;
    
    // Calculate shake intensity based on distance and dinosaur size
    float DistanceRatio = 1.0f - (Distance / MaxShakeDistance);
    float ShakeIntensity = DistanceRatio * DinosaurSize * 0.5f;
    float ShakeDuration = 0.2f + (DinosaurSize * 0.1f);
    
    TriggerScreenShake(ShakeIntensity, ShakeDuration, 15.0f);
}

void UAudio_ScreenEffectsManager::TriggerDamageFlash(float Intensity, FLinearColor FlashColor)
{
    CurrentFlashIntensity = Intensity;
    FlashTimer = 0.0f;
    this->FlashColor = FlashColor;
}

void UAudio_ScreenEffectsManager::UpdateTimeOfDayEffects(float TimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(TimeOfDay, 0.0f, 1.0f);
    
    // Update global material parameters for time of day
    if (GlobalEffectsCollection)
    {
        UMaterialParameterCollectionInstance* CollectionInstance = GetWorld()->GetParameterCollectionInstance(GlobalEffectsCollection);
        if (CollectionInstance)
        {
            CollectionInstance->SetScalarParameterValue(TEXT("TimeOfDay"), CurrentTimeOfDay);
            
            // Calculate ambient lighting based on time
            float AmbientIntensity = FMath::Clamp(FMath::Sin(CurrentTimeOfDay * PI), 0.1f, 1.0f);
            CollectionInstance->SetScalarParameterValue(TEXT("AmbientIntensity"), AmbientIntensity);
        }
    }
}

void UAudio_ScreenEffectsManager::SetNightVisionMode(bool bEnabled)
{
    bNightVisionActive = bEnabled;
    
    if (GlobalEffectsCollection)
    {
        UMaterialParameterCollectionInstance* CollectionInstance = GetWorld()->GetParameterCollectionInstance(GlobalEffectsCollection);
        if (CollectionInstance)
        {
            CollectionInstance->SetScalarParameterValue(TEXT("NightVision"), bEnabled ? 1.0f : 0.0f);
        }
    }
}

void UAudio_ScreenEffectsManager::TriggerWeatherEffect(EWeatherType WeatherType, float Intensity)
{
    CurrentWeatherType = WeatherType;
    CurrentWeatherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    if (GlobalEffectsCollection)
    {
        UMaterialParameterCollectionInstance* CollectionInstance = GetWorld()->GetParameterCollectionInstance(GlobalEffectsCollection);
        if (CollectionInstance)
        {
            CollectionInstance->SetScalarParameterValue(TEXT("WeatherIntensity"), CurrentWeatherIntensity);
            CollectionInstance->SetScalarParameterValue(TEXT("WeatherType"), static_cast<float>(CurrentWeatherType));
        }
    }
}

void UAudio_ScreenEffectsManager::UpdateFearEffect(float FearLevel)
{
    CurrentFearLevel = FMath::Clamp(FearLevel, 0.0f, 1.0f);
    
    if (GlobalEffectsCollection)
    {
        UMaterialParameterCollectionInstance* CollectionInstance = GetWorld()->GetParameterCollectionInstance(GlobalEffectsCollection);
        if (CollectionInstance)
        {
            CollectionInstance->SetScalarParameterValue(TEXT("FearLevel"), CurrentFearLevel);
            
            // Trigger subtle screen distortion at high fear levels
            if (CurrentFearLevel > 0.7f)
            {
                float DistortionIntensity = (CurrentFearLevel - 0.7f) * 0.1f;
                CollectionInstance->SetScalarParameterValue(TEXT("FearDistortion"), DistortionIntensity);
            }
        }
    }
}

void UAudio_ScreenEffectsManager::UpdateScreenShake(float DeltaTime)
{
    if (CurrentShakeIntensity > 0.0f)
    {
        ShakeTimer += DeltaTime;
        
        if (ShakeTimer >= ShakeDuration)
        {
            // Decay shake intensity
            CurrentShakeIntensity = FMath::FInterpTo(CurrentShakeIntensity, 0.0f, DeltaTime, ShakeDecayRate);
            
            if (CurrentShakeIntensity < 0.01f)
            {
                CurrentShakeIntensity = 0.0f;
                ShakeTimer = 0.0f;
            }
        }
    }
}

void UAudio_ScreenEffectsManager::UpdateDamageFlash(float DeltaTime)
{
    if (CurrentFlashIntensity > 0.0f)
    {
        FlashTimer += DeltaTime;
        
        // Flash decay curve
        float FlashProgress = FlashTimer / DamageFlashDuration;
        CurrentFlashIntensity = FMath::Lerp(CurrentFlashIntensity, 0.0f, FlashProgress);
        
        if (FlashTimer >= DamageFlashDuration)
        {
            CurrentFlashIntensity = 0.0f;
            FlashTimer = 0.0f;
        }
    }
}

void UAudio_ScreenEffectsManager::ApplyPostProcessEffects()
{
    if (!PostProcessVolume)
        return;
    
    // Apply damage flash to post process settings
    if (CurrentFlashIntensity > 0.0f)
    {
        PostProcessVolume->Settings.bOverride_ColorSaturation = true;
        PostProcessVolume->Settings.ColorSaturation = FVector4(1.0f - CurrentFlashIntensity, 1.0f, 1.0f, 1.0f);
        
        PostProcessVolume->Settings.bOverride_ColorGain = true;
        PostProcessVolume->Settings.ColorGain = FVector4(1.0f + CurrentFlashIntensity, 1.0f - (CurrentFlashIntensity * 0.5f), 1.0f - (CurrentFlashIntensity * 0.5f), 1.0f);
    }
    else
    {
        // Reset to normal
        PostProcessVolume->Settings.bOverride_ColorSaturation = false;
        PostProcessVolume->Settings.bOverride_ColorGain = false;
    }
    
    // Apply time of day effects
    if (CurrentTimeOfDay < 0.2f || CurrentTimeOfDay > 0.8f) // Night time
    {
        PostProcessVolume->Settings.bOverride_ColorGamma = true;
        float NightGamma = FMath::Lerp(1.2f, 1.0f, FMath::Min(CurrentTimeOfDay / 0.2f, (1.0f - CurrentTimeOfDay) / 0.2f));
        PostProcessVolume->Settings.ColorGamma = FVector4(NightGamma, NightGamma, NightGamma, 1.0f);
    }
    else
    {
        PostProcessVolume->Settings.bOverride_ColorGamma = false;
    }
}