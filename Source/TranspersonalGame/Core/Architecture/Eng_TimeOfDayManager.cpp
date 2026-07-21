#include "Eng_TimeOfDayManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UEng_TimeOfDayManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CurrentTimeOfDay = 12.0f; // Start at noon
    TimeScale = 1.0f;
    bTimePaused = false;
    DayDurationMinutes = 24.0f; // 24 real minutes = 1 game day
    
    SunriseTime = 6.0f;
    SunsetTime = 18.0f;
    MaxSunIntensity = 5.0f;
    MinSunIntensity = 0.1f;
    
    DayLightColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f); // Warm daylight
    NightLightColor = FLinearColor(0.2f, 0.3f, 0.8f, 1.0f); // Cool moonlight
    SunriseColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f); // Orange sunrise
    SunsetColor = FLinearColor(1.0f, 0.4f, 0.2f, 1.0f); // Red sunset
    
    LastLightingUpdate = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("TimeOfDayManager initialized at %.1f hours"), CurrentTimeOfDay);
}

void UEng_TimeOfDayManager::Deinitialize()
{
    SunLight.Reset();
    SkyLight.Reset();
    
    Super::Deinitialize();
}

void UEng_TimeOfDayManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bTimePaused)
    {
        UpdateTimeOfDay(DeltaTime);
    }
    
    LastLightingUpdate += DeltaTime;
    if (LastLightingUpdate >= LightingUpdateInterval)
    {
        UpdateLighting();
        LastLightingUpdate = 0.0f;
    }
}

void UEng_TimeOfDayManager::SetTimeOfDay(float Hours)
{
    CurrentTimeOfDay = FMath::Fmod(Hours, 24.0f);
    if (CurrentTimeOfDay < 0.0f)
    {
        CurrentTimeOfDay += 24.0f;
    }
    
    UpdateLighting();
    UE_LOG(LogTemp, Warning, TEXT("Time set to %.2f hours"), CurrentTimeOfDay);
}

void UEng_TimeOfDayManager::SetTimeScale(float Scale)
{
    TimeScale = FMath::Max(0.0f, Scale);
    UE_LOG(LogTemp, Warning, TEXT("Time scale set to %.2fx"), TimeScale);
}

void UEng_TimeOfDayManager::PauseTime(bool bPause)
{
    bTimePaused = bPause;
    UE_LOG(LogTemp, Warning, TEXT("Time %s"), bPause ? TEXT("paused") : TEXT("resumed"));
}

void UEng_TimeOfDayManager::SetDayDuration(float Minutes)
{
    DayDurationMinutes = FMath::Max(1.0f, Minutes);
    UE_LOG(LogTemp, Warning, TEXT("Day duration set to %.1f minutes"), DayDurationMinutes);
}

EEng_TimeOfDay UEng_TimeOfDayManager::GetTimePhase() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)
    {
        return EEng_TimeOfDay::Dawn;
    }
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 17.0f)
    {
        return EEng_TimeOfDay::Day;
    }
    else if (CurrentTimeOfDay >= 17.0f && CurrentTimeOfDay < 19.0f)
    {
        return EEng_TimeOfDay::Dusk;
    }
    else
    {
        return EEng_TimeOfDay::Night;
    }
}

float UEng_TimeOfDayManager::GetSunAngle() const
{
    // Convert time to angle (0-360 degrees)
    // 6 AM = 0 degrees (horizon), 12 PM = 90 degrees (zenith), 6 PM = 180 degrees (horizon)
    float NormalizedTime = (CurrentTimeOfDay - SunriseTime) / (SunsetTime - SunriseTime);
    NormalizedTime = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    
    return NormalizedTime * 180.0f; // 0 to 180 degrees
}

bool UEng_TimeOfDayManager::IsNight() const
{
    return CurrentTimeOfDay < SunriseTime || CurrentTimeOfDay > SunsetTime;
}

bool UEng_TimeOfDayManager::IsDay() const
{
    return !IsNight();
}

void UEng_TimeOfDayManager::UpdateSunPosition()
{
    if (!SunLight.IsValid())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
        
        if (FoundActors.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        }
    }
    
    if (SunLight.IsValid())
    {
        float SunAngle = GetSunAngle();
        
        // Calculate sun rotation
        FRotator SunRotation;
        SunRotation.Pitch = -(SunAngle - 90.0f); // Negative because UE4 pitch is inverted
        SunRotation.Yaw = 0.0f; // Sun moves east to west
        SunRotation.Roll = 0.0f;
        
        SunLight->SetActorRotation(SunRotation);
    }
}

void UEng_TimeOfDayManager::UpdateSkyLighting()
{
    // TODO: Update sky lighting based on time of day
    UE_LOG(LogTemp, Log, TEXT("Sky lighting updated for time %.2f"), CurrentTimeOfDay);
}

void UEng_TimeOfDayManager::UpdateTimeOfDay(float DeltaTime)
{
    // Convert real time to game time
    float GameTimeIncrement = (DeltaTime * TimeScale * 24.0f) / (DayDurationMinutes * 60.0f);
    
    CurrentTimeOfDay += GameTimeIncrement;
    
    // Wrap around 24 hours
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
        UE_LOG(LogTemp, Warning, TEXT("New day started"));
    }
}

void UEng_TimeOfDayManager::UpdateLighting()
{
    UpdateSunPosition();
    
    if (SunLight.IsValid())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        if (LightComp)
        {
            // Update light intensity
            float Intensity = GetCurrentLightIntensity();
            LightComp->SetIntensity(Intensity);
            
            // Update light color
            FLinearColor Color = GetCurrentLightColor();
            LightComp->SetLightColor(Color);
        }
    }
}

FLinearColor UEng_TimeOfDayManager::GetCurrentLightColor() const
{
    EEng_TimeOfDay Phase = GetTimePhase();
    
    switch (Phase)
    {
        case EEng_TimeOfDay::Dawn:
            return SunriseColor;
        case EEng_TimeOfDay::Day:
            return DayLightColor;
        case EEng_TimeOfDay::Dusk:
            return SunsetColor;
        case EEng_TimeOfDay::Night:
        default:
            return NightLightColor;
    }
}

float UEng_TimeOfDayManager::GetCurrentLightIntensity() const
{
    if (IsNight())
    {
        return MinSunIntensity;
    }
    
    float SunAngle = GetSunAngle();
    
    // Intensity based on sun height
    if (SunAngle < 0.0f || SunAngle > 180.0f)
    {
        return MinSunIntensity;
    }
    
    // Peak intensity at noon (90 degrees)
    float NormalizedAngle = FMath::Abs(SunAngle - 90.0f) / 90.0f; // 0 at noon, 1 at horizon
    float Intensity = FMath::Lerp(MaxSunIntensity, MinSunIntensity, NormalizedAngle);
    
    return FMath::Max(Intensity, MinSunIntensity);
}