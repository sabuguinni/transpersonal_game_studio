#include "Light_DynamicWeatherSystem.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/PostProcessVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ULight_DynamicWeatherSystem::ULight_DynamicWeatherSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    CurrentTimeOfDay = 12.0f; // Start at noon
    DayNightCycleSpeed = 1.0f;
    bRandomWeatherEnabled = false;
    MinWeatherDuration = 300.0f;
    MaxWeatherDuration = 900.0f;
    NextWeatherChangeTime = 0.0f;
}

void ULight_DynamicWeatherSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeWeatherPresets();
    FindLightingComponents();
    SetCretaceousAtmosphere();
    UpdateLighting();
}

void ULight_DynamicWeatherSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update time of day
    CurrentTimeOfDay += (DeltaTime * DayNightCycleSpeed) / 3600.0f; // Convert seconds to hours
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }
    
    // Update weather transition
    if (bIsTransitioning)
    {
        TickWeatherTransition(DeltaTime);
    }
    
    // Update random weather
    if (bRandomWeatherEnabled)
    {
        TickRandomWeather(DeltaTime);
    }
    
    // Update lighting based on current state
    UpdateLighting();
}

void ULight_DynamicWeatherSystem::SetWeatherType(ELight_WeatherType NewWeatherType)
{
    if (CurrentWeatherType != NewWeatherType)
    {
        CurrentWeatherType = NewWeatherType;
        bIsTransitioning = false;
        UpdateLighting();
    }
}

void ULight_DynamicWeatherSystem::TransitionToWeather(ELight_WeatherType TargetWeather, float TransitionDuration)
{
    if (CurrentWeatherType != TargetWeather)
    {
        TargetWeatherType = TargetWeather;
        this->TransitionDuration = FMath::Max(0.1f, TransitionDuration);
        TransitionProgress = 0.0f;
        bIsTransitioning = true;
    }
}

void ULight_DynamicWeatherSystem::StartRandomWeatherCycle()
{
    bRandomWeatherEnabled = true;
    NextWeatherChangeTime = GetWorld()->GetTimeSeconds() + FMath::RandRange(MinWeatherDuration, MaxWeatherDuration);
}

void ULight_DynamicWeatherSystem::StopWeatherCycle()
{
    bRandomWeatherEnabled = false;
}

void ULight_DynamicWeatherSystem::SetTimeOfDay(float Hours)
{
    CurrentTimeOfDay = FMath::Clamp(Hours, 0.0f, 24.0f);
    UpdateLighting();
}

void ULight_DynamicWeatherSystem::SetDayNightCycleSpeed(float SpeedMultiplier)
{
    DayNightCycleSpeed = FMath::Max(0.0f, SpeedMultiplier);
}

void ULight_DynamicWeatherSystem::UpdateLighting()
{
    if (!SunLight || !SkyAtmosphere)
    {
        FindLightingComponents();
        if (!SunLight || !SkyAtmosphere) return;
    }
    
    FLight_WeatherPreset CurrentPreset = GetCurrentWeatherPreset();
    
    // Update sun position based on time of day
    UpdateSunPosition();
    
    // Update sun light properties
    UDirectionalLightComponent* LightComponent = SunLight->GetComponent();
    if (LightComponent)
    {
        // Calculate sun intensity based on time of day (dimmer at dawn/dusk)
        float TimeBasedIntensity = 1.0f;
        if (CurrentTimeOfDay < 6.0f || CurrentTimeOfDay > 18.0f)
        {
            // Night time - very dim
            TimeBasedIntensity = 0.1f;
        }
        else if (CurrentTimeOfDay < 8.0f || CurrentTimeOfDay > 16.0f)
        {
            // Dawn/Dusk - reduced intensity
            float DawnDuskFactor = FMath::Min(
                FMath::Abs(CurrentTimeOfDay - 6.0f) / 2.0f,
                FMath::Abs(18.0f - CurrentTimeOfDay) / 2.0f
            );
            TimeBasedIntensity = FMath::Lerp(0.1f, 1.0f, DawnDuskFactor);
        }
        
        float FinalIntensity = CurrentPreset.SunIntensity * TimeBasedIntensity;
        LightComponent->SetIntensity(FinalIntensity);
        
        // Set sun color with warmer tones during dawn/dusk
        FLinearColor SunColor = CurrentPreset.SunColor;
        if (CurrentTimeOfDay < 8.0f || CurrentTimeOfDay > 16.0f)
        {
            // Add orange/red tint during dawn/dusk
            SunColor.R = FMath::Min(1.0f, SunColor.R * 1.2f);
            SunColor.G = FMath::Min(1.0f, SunColor.G * 0.9f);
            SunColor.B = FMath::Min(1.0f, SunColor.B * 0.7f);
        }
        
        LightComponent->SetLightColor(SunColor);
    }
    
    UpdateAtmosphere();
    UpdatePostProcess();
}

void ULight_DynamicWeatherSystem::SetCretaceousAtmosphere()
{
    // Set default Cretaceous period atmospheric conditions
    SetWeatherType(ELight_WeatherType::Clear);
    
    if (SunLight)
    {
        UDirectionalLightComponent* LightComponent = SunLight->GetComponent();
        if (LightComponent)
        {
            // Cretaceous period had higher CO2, creating warmer, more humid conditions
            LightComponent->SetIntensity(5.0f);
            LightComponent->SetLightColor(FLinearColor(1.0f, 0.94f, 0.78f, 1.0f)); // Warm golden light
        }
    }
}

void ULight_DynamicWeatherSystem::InitializeWeatherPresets()
{
    WeatherPresets.Empty();
    
    // Clear Skies - Typical Cretaceous
    FLight_WeatherPreset ClearPreset;
    ClearPreset.WeatherType = ELight_WeatherType::Clear;
    ClearPreset.SunIntensity = 5.0f;
    ClearPreset.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
    ClearPreset.CloudCoverage = 0.2f;
    ClearPreset.FogDensity = 0.05f;
    ClearPreset.RainIntensity = 0.0f;
    ClearPreset.WindStrength = 1.0f;
    WeatherPresets.Add(ELight_WeatherType::Clear, ClearPreset);
    
    // Overcast - Dense Cretaceous atmosphere
    FLight_WeatherPreset OvercastPreset;
    OvercastPreset.WeatherType = ELight_WeatherType::Overcast;
    OvercastPreset.SunIntensity = 2.5f;
    OvercastPreset.SunColor = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f);
    OvercastPreset.CloudCoverage = 0.8f;
    OvercastPreset.FogDensity = 0.15f;
    OvercastPreset.RainIntensity = 0.0f;
    OvercastPreset.WindStrength = 1.5f;
    WeatherPresets.Add(ELight_WeatherType::Overcast, OvercastPreset);
    
    // Light Rain - Frequent Cretaceous precipitation
    FLight_WeatherPreset LightRainPreset;
    LightRainPreset.WeatherType = ELight_WeatherType::LightRain;
    LightRainPreset.SunIntensity = 1.5f;
    LightRainPreset.SunColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    LightRainPreset.CloudCoverage = 0.9f;
    LightRainPreset.FogDensity = 0.25f;
    LightRainPreset.RainIntensity = 0.3f;
    LightRainPreset.WindStrength = 2.0f;
    WeatherPresets.Add(ELight_WeatherType::LightRain, LightRainPreset);
    
    // Heavy Rain - Monsoon-like conditions
    FLight_WeatherPreset HeavyRainPreset;
    HeavyRainPreset.WeatherType = ELight_WeatherType::HeavyRain;
    HeavyRainPreset.SunIntensity = 0.8f;
    HeavyRainPreset.SunColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);
    HeavyRainPreset.CloudCoverage = 1.0f;
    HeavyRainPreset.FogDensity = 0.4f;
    HeavyRainPreset.RainIntensity = 0.8f;
    HeavyRainPreset.WindStrength = 3.0f;
    WeatherPresets.Add(ELight_WeatherType::HeavyRain, HeavyRainPreset);
    
    // Storm - Dramatic weather events
    FLight_WeatherPreset StormPreset;
    StormPreset.WeatherType = ELight_WeatherType::Storm;
    StormPreset.SunIntensity = 0.3f;
    StormPreset.SunColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
    StormPreset.CloudCoverage = 1.0f;
    StormPreset.FogDensity = 0.6f;
    StormPreset.RainIntensity = 1.0f;
    StormPreset.WindStrength = 5.0f;
    WeatherPresets.Add(ELight_WeatherType::Storm, StormPreset);
    
    // Fog - Dense prehistoric atmosphere
    FLight_WeatherPreset FogPreset;
    FogPreset.WeatherType = ELight_WeatherType::Fog;
    FogPreset.SunIntensity = 1.0f;
    FogPreset.SunColor = FLinearColor(0.9f, 0.9f, 0.8f, 1.0f);
    FogPreset.CloudCoverage = 0.6f;
    FogPreset.FogDensity = 0.8f;
    FogPreset.RainIntensity = 0.1f;
    FogPreset.WindStrength = 0.5f;
    WeatherPresets.Add(ELight_WeatherType::Fog, FogPreset);
}

void ULight_DynamicWeatherSystem::FindLightingComponents()
{
    if (UWorld* World = GetWorld())
    {
        // Find directional light (sun)
        if (!SunLight)
        {
            TArray<AActor*> FoundLights;
            UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundLights);
            if (FoundLights.Num() > 0)
            {
                SunLight = Cast<ADirectionalLight>(FoundLights[0]);
            }
        }
        
        // Find sky atmosphere
        if (!SkyAtmosphere)
        {
            TArray<AActor*> FoundAtmosphere;
            UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundAtmosphere);
            if (FoundAtmosphere.Num() > 0)
            {
                SkyAtmosphere = Cast<ASkyAtmosphere>(FoundAtmosphere[0]);
            }
        }
        
        // Find post process volume
        if (!PostProcessVolume)
        {
            TArray<AActor*> FoundPostProcess;
            UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), FoundPostProcess);
            if (FoundPostProcess.Num() > 0)
            {
                PostProcessVolume = Cast<APostProcessVolume>(FoundPostProcess[0]);
            }
        }
    }
}

void ULight_DynamicWeatherSystem::UpdateSunPosition()
{
    if (!SunLight) return;
    
    // Calculate sun angle based on time of day
    // 6 AM = sunrise (90 degrees), 12 PM = noon (0 degrees), 6 PM = sunset (-90 degrees)
    float SunAngle = (CurrentTimeOfDay - 12.0f) * 15.0f; // 15 degrees per hour
    
    // Create rotation for sun position
    FRotator SunRotation = FRotator(SunAngle, 0.0f, 0.0f);
    SunLight->SetActorRotation(SunRotation);
}

void ULight_DynamicWeatherSystem::UpdateAtmosphere()
{
    if (!SkyAtmosphere) return;
    
    USkyAtmosphereComponent* AtmosphereComponent = SkyAtmosphere->GetComponent();
    if (!AtmosphereComponent) return;
    
    FLight_WeatherPreset CurrentPreset = GetCurrentWeatherPreset();
    
    // Update atmospheric properties based on weather
    // These would be set via the component's properties in a real implementation
    // For now, we'll focus on the lighting aspects
}

void ULight_DynamicWeatherSystem::UpdatePostProcess()
{
    if (!PostProcessVolume) return;
    
    FLight_WeatherPreset CurrentPreset = GetCurrentWeatherPreset();
    
    // Update post-process effects based on weather
    // This would involve modifying bloom, exposure, color grading, etc.
    // Implementation would depend on the specific post-process setup
}

void ULight_DynamicWeatherSystem::TickWeatherTransition(float DeltaTime)
{
    TransitionProgress += DeltaTime / TransitionDuration;
    
    if (TransitionProgress >= 1.0f)
    {
        // Transition complete
        CurrentWeatherType = TargetWeatherType;
        bIsTransitioning = false;
        TransitionProgress = 0.0f;
    }
}

void ULight_DynamicWeatherSystem::TickRandomWeather(float DeltaTime)
{
    if (GetWorld()->GetTimeSeconds() >= NextWeatherChangeTime)
    {
        // Choose random weather type
        TArray<ELight_WeatherType> WeatherTypes = {
            ELight_WeatherType::Clear,
            ELight_WeatherType::Overcast,
            ELight_WeatherType::LightRain,
            ELight_WeatherType::Fog
        };
        
        ELight_WeatherType NewWeather = WeatherTypes[FMath::RandRange(0, WeatherTypes.Num() - 1)];
        
        // Avoid immediate repetition
        if (NewWeather == CurrentWeatherType)
        {
            NewWeather = WeatherTypes[(WeatherTypes.Find(NewWeather) + 1) % WeatherTypes.Num()];
        }
        
        TransitionToWeather(NewWeather, 10.0f); // 10 second transition
        
        // Schedule next weather change
        NextWeatherChangeTime = GetWorld()->GetTimeSeconds() + FMath::RandRange(MinWeatherDuration, MaxWeatherDuration);
    }
}

FLight_WeatherPreset ULight_DynamicWeatherSystem::GetCurrentWeatherPreset() const
{
    if (bIsTransitioning)
    {
        // Lerp between current and target weather
        const FLight_WeatherPreset* CurrentPtr = WeatherPresets.Find(CurrentWeatherType);
        const FLight_WeatherPreset* TargetPtr = WeatherPresets.Find(TargetWeatherType);
        
        if (CurrentPtr && TargetPtr)
        {
            return LerpWeatherPresets(*CurrentPtr, *TargetPtr, TransitionProgress);
        }
    }
    
    const FLight_WeatherPreset* PresetPtr = WeatherPresets.Find(CurrentWeatherType);
    return PresetPtr ? *PresetPtr : FLight_WeatherPreset();
}

FLight_WeatherPreset ULight_DynamicWeatherSystem::LerpWeatherPresets(const FLight_WeatherPreset& A, const FLight_WeatherPreset& B, float Alpha) const
{
    FLight_WeatherPreset Result;
    
    Result.WeatherType = Alpha < 0.5f ? A.WeatherType : B.WeatherType;
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.CloudCoverage = FMath::Lerp(A.CloudCoverage, B.CloudCoverage, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.RainIntensity = FMath::Lerp(A.RainIntensity, B.RainIntensity, Alpha);
    Result.WindStrength = FMath::Lerp(A.WindStrength, B.WindStrength, Alpha);
    
    return Result;
}