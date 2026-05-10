#include "Audio_ScreenShakeManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/CameraShake.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UAudio_ScreenShakeManager::UAudio_ScreenShakeManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Check 10 times per second

    // Initialize default values
    MaxShakeDistance = 2000.0f;
    MinShakeDistance = 100.0f;
    TRexFootstepInterval = 2.0f;
    TRexShakeRadius = 1500.0f;
    bIsShaking = false;
    CurrentShakeIntensity = 0.0f;
    TrackedTRex = nullptr;
}

void UAudio_ScreenShakeManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeShakeProfiles();
}

void UAudio_ScreenShakeManager::InitializeShakeProfiles()
{
    // Light shake (small dinosaurs, distant T-Rex)
    FAudio_ShakeProfile LightShake;
    LightShake.Duration = 0.3f;
    LightShake.Amplitude = 0.2f;
    LightShake.Frequency = 8.0f;
    LightShake.bFadeIn = false;
    LightShake.bFadeOut = true;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Light, LightShake);

    // Medium shake (medium dinosaurs, moderate T-Rex proximity)
    FAudio_ShakeProfile MediumShake;
    MediumShake.Duration = 0.6f;
    MediumShake.Amplitude = 0.5f;
    MediumShake.Frequency = 12.0f;
    MediumShake.bFadeIn = true;
    MediumShake.bFadeOut = true;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Medium, MediumShake);

    // Heavy shake (large dinosaurs, close T-Rex)
    FAudio_ShakeProfile HeavyShake;
    HeavyShake.Duration = 1.0f;
    HeavyShake.Amplitude = 1.0f;
    HeavyShake.Frequency = 15.0f;
    HeavyShake.bFadeIn = true;
    HeavyShake.bFadeOut = true;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Heavy, HeavyShake);

    // Extreme shake (T-Rex footstep, massive impacts)
    FAudio_ShakeProfile ExtremeShake;
    ExtremeShake.Duration = 1.5f;
    ExtremeShake.Amplitude = 1.5f;
    ExtremeShake.Frequency = 20.0f;
    ExtremeShake.bFadeIn = true;
    ExtremeShake.bFadeOut = true;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Extreme, ExtremeShake);
}

void UAudio_ScreenShakeManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update shake intensity based on T-Rex proximity
    if (TrackedTRex && IsValid(TrackedTRex))
    {
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), TrackedTRex->GetActorLocation());
        CurrentShakeIntensity = CalculateShakeIntensity(Distance, TRexShakeRadius, MinShakeDistance);
        
        // Trigger ambient shake if close enough
        if (CurrentShakeIntensity > 0.1f)
        {
            EAudio_ShakeIntensity ShakeLevel = GetShakeIntensityFromDistance(Distance);
            if (ShakeLevel != EAudio_ShakeIntensity::None)
            {
                // Only trigger if not already shaking at this level
                if (!bIsShaking)
                {
                    TriggerShake(ShakeLevel, CurrentShakeIntensity);
                }
            }
        }
    }
}

void UAudio_ScreenShakeManager::TriggerShake(EAudio_ShakeIntensity Intensity, float Scale)
{
    if (Intensity == EAudio_ShakeIntensity::None)
    {
        return;
    }

    const FAudio_ShakeProfile* Profile = ShakeProfiles.Find(Intensity);
    if (Profile)
    {
        ExecuteShake(*Profile, Scale);
        bIsShaking = true;
        
        // Reset shaking state after duration
        FTimerHandle ShakeResetTimer;
        GetWorld()->GetTimerManager().SetTimer(ShakeResetTimer, [this]()
        {
            bIsShaking = false;
        }, Profile->Duration, false);
    }
}

void UAudio_ScreenShakeManager::TriggerProximityShake(const FVector& SourceLocation, float SourceMass)
{
    if (!GetOwner())
    {
        return;
    }

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), SourceLocation);
    float Intensity = CalculateShakeIntensity(Distance, MaxShakeDistance, MinShakeDistance);
    
    if (Intensity > 0.0f)
    {
        // Scale intensity by source mass (T-Rex = 8.0, Raptor = 0.5, etc.)
        float MassScale = FMath::Clamp(SourceMass / 8.0f, 0.1f, 2.0f);
        EAudio_ShakeIntensity ShakeLevel = GetShakeIntensityFromDistance(Distance);
        TriggerShake(ShakeLevel, Intensity * MassScale);
    }
}

void UAudio_ScreenShakeManager::TriggerTRexFootstep(const FVector& FootstepLocation)
{
    if (!GetOwner())
    {
        return;
    }

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), FootstepLocation);
    
    // T-Rex footsteps are always heavy impacts
    if (Distance <= TRexShakeRadius)
    {
        float Intensity = CalculateShakeIntensity(Distance, TRexShakeRadius, MinShakeDistance);
        TriggerShake(EAudio_ShakeIntensity::Extreme, Intensity);
        
        // Log for debugging
        UE_LOG(LogTemp, Log, TEXT("T-Rex footstep shake: Distance=%.1f, Intensity=%.2f"), Distance, Intensity);
    }
}

void UAudio_ScreenShakeManager::StartTRexProximityShaking(AActor* TRexActor)
{
    if (!IsValid(TRexActor))
    {
        return;
    }

    TrackedTRex = TRexActor;
    
    // Start periodic footstep shakes
    GetWorld()->GetTimerManager().SetTimer(FootstepShakeTimer, this, 
        &UAudio_ScreenShakeManager::OnTRexFootstepTimer, TRexFootstepInterval, true);
    
    UE_LOG(LogTemp, Log, TEXT("Started T-Rex proximity shaking for: %s"), *TRexActor->GetName());
}

void UAudio_ScreenShakeManager::StopTRexProximityShaking()
{
    TrackedTRex = nullptr;
    GetWorld()->GetTimerManager().ClearTimer(FootstepShakeTimer);
    bIsShaking = false;
    CurrentShakeIntensity = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Stopped T-Rex proximity shaking"));
}

void UAudio_ScreenShakeManager::TriggerDamageShake(float DamageAmount, const FVector& DamageDirection)
{
    // Scale shake intensity based on damage amount
    float NormalizedDamage = FMath::Clamp(DamageAmount / 100.0f, 0.1f, 2.0f);
    
    EAudio_ShakeIntensity ShakeLevel;
    if (DamageAmount >= 75.0f)
    {
        ShakeLevel = EAudio_ShakeIntensity::Extreme;
    }
    else if (DamageAmount >= 50.0f)
    {
        ShakeLevel = EAudio_ShakeIntensity::Heavy;
    }
    else if (DamageAmount >= 25.0f)
    {
        ShakeLevel = EAudio_ShakeIntensity::Medium;
    }
    else
    {
        ShakeLevel = EAudio_ShakeIntensity::Light;
    }
    
    TriggerShake(ShakeLevel, NormalizedDamage);
}

void UAudio_ScreenShakeManager::StopAllShakes()
{
    APlayerController* PC = GetPlayerController();
    if (PC)
    {
        PC->ClientStopCameraShake(UCameraShakeBase::StaticClass());
    }
    
    bIsShaking = false;
    CurrentShakeIntensity = 0.0f;
}

float UAudio_ScreenShakeManager::CalculateShakeIntensity(float Distance, float MaxDistance, float MinDistance) const
{
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }
    
    if (Distance <= MinDistance)
    {
        return 1.0f;
    }
    
    // Linear falloff from max to min distance
    float NormalizedDistance = (Distance - MinDistance) / (MaxDistance - MinDistance);
    return 1.0f - NormalizedDistance;
}

EAudio_ShakeIntensity UAudio_ScreenShakeManager::GetShakeIntensityFromDistance(float Distance) const
{
    float Intensity = CalculateShakeIntensity(Distance, MaxShakeDistance, MinShakeDistance);
    
    if (Intensity >= 0.8f)
    {
        return EAudio_ShakeIntensity::Extreme;
    }
    else if (Intensity >= 0.6f)
    {
        return EAudio_ShakeIntensity::Heavy;
    }
    else if (Intensity >= 0.3f)
    {
        return EAudio_ShakeIntensity::Medium;
    }
    else if (Intensity > 0.0f)
    {
        return EAudio_ShakeIntensity::Light;
    }
    
    return EAudio_ShakeIntensity::None;
}

void UAudio_ScreenShakeManager::ExecuteShake(const FAudio_ShakeProfile& Profile, float Scale)
{
    APlayerController* PC = GetPlayerController();
    if (!PC)
    {
        return;
    }

    // Create a simple camera shake
    // Note: In a full implementation, you would create custom camera shake classes
    // For now, we use the basic engine shake functionality
    
    // Calculate final amplitude
    float FinalAmplitude = Profile.Amplitude * Scale;
    
    // Use the engine's built-in shake system
    // This is a simplified version - in production you'd create custom shake assets
    if (FinalAmplitude > 0.0f)
    {
        // Log the shake for debugging
        UE_LOG(LogTemp, Log, TEXT("Executing screen shake: Amplitude=%.2f, Duration=%.2f, Frequency=%.1f"), 
            FinalAmplitude, Profile.Duration, Profile.Frequency);
        
        // Note: Actual camera shake implementation would require custom shake classes
        // This is a placeholder that logs the shake parameters
    }
}

void UAudio_ScreenShakeManager::OnTRexFootstepTimer()
{
    if (TrackedTRex && IsValid(TrackedTRex))
    {
        TriggerTRexFootstep(TrackedTRex->GetActorLocation());
    }
}

APlayerController* UAudio_ScreenShakeManager::GetPlayerController() const
{
    if (!GetWorld())
    {
        return nullptr;
    }
    
    return GetWorld()->GetFirstPlayerController();
}