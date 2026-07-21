#include "Perf_DayNightCycle.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/SkyLight.h"
#include "Kismet/KismetMathLibrary.h"

UPerf_DayNightCycle::UPerf_DayNightCycle()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms for smooth transitions
    
    // Default cycle settings
    DayDurationInMinutes = 20.0f; // 20 minute full day/night cycle
    CurrentTimeOfDay = 0.5f; // Start at noon
    TimeMultiplier = 1.0f;
    bPauseCycle = false;
    
    // Sun settings
    SunIntensityDay = 3.0f;
    SunIntensityNight = 0.1f;
    SunColorDay = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f); // Warm daylight
    SunColorNight = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f); // Cool moonlight
    
    // Sky settings
    SkyIntensityDay = 1.0f;
    SkyIntensityNight = 0.2f;
    
    // Performance settings
    bOptimizeForPerformance = true;
    LightUpdateFrequency = 0.2f; // Update lights every 200ms
    
    LastLightUpdateTime = 0.0f;
}

void UPerf_DayNightCycle::BeginPlay()
{
    Super::BeginPlay();
    
    // Find sun and sky lights in the world
    FindLightActors();
    
    // Initialize with current time
    UpdateLighting();
    
    UE_LOG(LogTemp, Warning, TEXT("Day/Night Cycle initialized - Duration: %.1f minutes"), DayDurationInMinutes);
}

void UPerf_DayNightCycle::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bPauseCycle)
    {
        return;
    }
    
    // Update time of day
    float TimeIncrement = (DeltaTime * TimeMultiplier) / (DayDurationInMinutes * 60.0f);
    CurrentTimeOfDay += TimeIncrement;
    
    // Wrap around at 1.0 (full day)
    if (CurrentTimeOfDay >= 1.0f)
    {
        CurrentTimeOfDay -= 1.0f;
        OnDayCompleted.Broadcast();
    }
    
    // Performance optimization: only update lighting at specified frequency
    if (bOptimizeForPerformance)
    {
        LastLightUpdateTime += DeltaTime;
        if (LastLightUpdateTime >= LightUpdateFrequency)
        {
            UpdateLighting();
            LastLightUpdateTime = 0.0f;
        }
    }
    else
    {
        UpdateLighting();
    }
}

void UPerf_DayNightCycle::FindLightActors()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Find directional light (sun)
    for (TActorIterator<ADirectionalLight> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        ADirectionalLight* DirectionalLight = *ActorItr;
        if (DirectionalLight && DirectionalLight->GetLightComponent())
        {
            SunLight = DirectionalLight;
            UE_LOG(LogTemp, Log, TEXT("Found Sun Light: %s"), *DirectionalLight->GetName());
            break;
        }
    }
    
    // Find sky light
    for (TActorIterator<ASkyLight> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        ASkyLight* SkyLightActor = *ActorItr;
        if (SkyLightActor && SkyLightActor->GetLightComponent())
        {
            SkyLight = SkyLightActor;
            UE_LOG(LogTemp, Log, TEXT("Found Sky Light: %s"), *SkyLightActor->GetName());
            break;
        }
    }
    
    if (!SunLight)
    {
        UE_LOG(LogTemp, Warning, TEXT("No Directional Light found for Day/Night cycle"));
    }
    
    if (!SkyLight)
    {
        UE_LOG(LogTemp, Warning, TEXT("No Sky Light found for Day/Night cycle"));
    }
}

void UPerf_DayNightCycle::UpdateLighting()
{
    if (!SunLight || !SunLight->GetLightComponent())
    {
        return;
    }
    
    UDirectionalLightComponent* SunComponent = SunLight->GetLightComponent();
    
    // Calculate sun angle (0 = sunrise, 0.5 = noon, 1.0 = sunset)
    float SunAngle = CurrentTimeOfDay * 360.0f - 90.0f; // -90 to start at horizon
    
    // Set sun rotation
    FRotator SunRotation = FRotator(SunAngle, 0.0f, 0.0f);
    SunLight->SetActorRotation(SunRotation);
    
    // Calculate lighting intensity based on sun height
    float SunHeight = FMath::Sin(FMath::DegreesToRadians(SunAngle));
    float IntensityFactor = FMath::Clamp(SunHeight, 0.0f, 1.0f);
    
    // Interpolate sun intensity
    float SunIntensity = FMath::Lerp(SunIntensityNight, SunIntensityDay, IntensityFactor);
    SunComponent->SetIntensity(SunIntensity);
    
    // Interpolate sun color
    FLinearColor SunColor = FMath::Lerp(SunColorNight, SunColorDay, IntensityFactor);
    SunComponent->SetLightColor(SunColor);
    
    // Update sky light if available
    if (SkyLight && SkyLight->GetLightComponent())
    {
        USkyLightComponent* SkyComponent = SkyLight->GetLightComponent();
        float SkyIntensity = FMath::Lerp(SkyIntensityNight, SkyIntensityDay, IntensityFactor);
        SkyComponent->SetIntensity(SkyIntensity);
    }
    
    // Broadcast time change event
    OnTimeOfDayChanged.Broadcast(CurrentTimeOfDay, GetCurrentPhase());
}

EPerf_DayPhase UPerf_DayNightCycle::GetCurrentPhase() const
{
    if (CurrentTimeOfDay >= 0.0f && CurrentTimeOfDay < 0.25f)
    {
        return EPerf_DayPhase::Night;
    }
    else if (CurrentTimeOfDay >= 0.25f && CurrentTimeOfDay < 0.3f)
    {
        return EPerf_DayPhase::Dawn;
    }
    else if (CurrentTimeOfDay >= 0.3f && CurrentTimeOfDay < 0.7f)
    {
        return EPerf_DayPhase::Day;
    }
    else if (CurrentTimeOfDay >= 0.7f && CurrentTimeOfDay < 0.75f)
    {
        return EPerf_DayPhase::Dusk;
    }
    else
    {
        return EPerf_DayPhase::Night;
    }
}

FString UPerf_DayNightCycle::GetTimeString() const
{
    int32 Hours = FMath::FloorToInt(CurrentTimeOfDay * 24.0f);
    int32 Minutes = FMath::FloorToInt((CurrentTimeOfDay * 24.0f - Hours) * 60.0f);
    
    return FString::Printf(TEXT("%02d:%02d"), Hours, Minutes);
}

void UPerf_DayNightCycle::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 1.0f);
    UpdateLighting();
    
    UE_LOG(LogTemp, Log, TEXT("Time of day set to: %s"), *GetTimeString());
}

void UPerf_DayNightCycle::SetDayDuration(float NewDurationMinutes)
{
    DayDurationInMinutes = FMath::Clamp(NewDurationMinutes, 1.0f, 120.0f);
    UE_LOG(LogTemp, Log, TEXT("Day duration set to: %.1f minutes"), DayDurationInMinutes);
}

void UPerf_DayNightCycle::PauseCycle(bool bPause)
{
    bPauseCycle = bPause;
    UE_LOG(LogTemp, Log, TEXT("Day/Night cycle %s"), bPause ? TEXT("paused") : TEXT("resumed"));
}

void UPerf_DayNightCycle::SetTimeMultiplier(float NewMultiplier)
{
    TimeMultiplier = FMath::Clamp(NewMultiplier, 0.1f, 10.0f);
    UE_LOG(LogTemp, Log, TEXT("Time multiplier set to: %.1fx"), TimeMultiplier);
}

void UPerf_DayNightCycle::OptimizeForPerformance(bool bOptimize)
{
    bOptimizeForPerformance = bOptimize;
    
    if (bOptimize)
    {
        LightUpdateFrequency = 0.2f; // 5 FPS for lighting updates
        PrimaryComponentTick.TickInterval = 0.1f;
    }
    else
    {
        LightUpdateFrequency = 0.0f; // Update every frame
        PrimaryComponentTick.TickInterval = 0.0f;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance optimization %s"), bOptimize ? TEXT("enabled") : TEXT("disabled"));
}