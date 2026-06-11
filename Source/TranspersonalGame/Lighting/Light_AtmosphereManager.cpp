#include "Light_AtmosphereManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

ALight_AtmosphereManager::ALight_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize Cretaceous atmosphere settings
    CretaceousSettings.SunIntensity = 5.0f;
    CretaceousSettings.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f); // Warm golden
    CretaceousSettings.FogDensity = 0.02f;
    CretaceousSettings.FogHeightFalloff = 0.2f;
    CretaceousSettings.FogInscatteringColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
}

void ALight_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindLightingActors();
    ApplyCretaceousAtmosphere();
}

void ALight_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableDayNightCycle)
    {
        // Advance time - 24 hours in DayDurationMinutes real minutes
        float TimeAdvancement = (24.0f / (DayDurationMinutes * 60.0f)) * DeltaTime;
        CurrentTimeOfDay += TimeAdvancement;
        
        if (CurrentTimeOfDay >= 24.0f)
        {
            CurrentTimeOfDay -= 24.0f;
        }
        
        UpdateLightingForTime(CurrentTimeOfDay);
    }
}

void ALight_AtmosphereManager::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find DirectionalLight
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    if (DirectionalLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(DirectionalLights[0]);
    }
    
    // Find SkyAtmosphere
    TArray<AActor*> SkyAtmospheres;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), SkyAtmospheres);
    if (SkyAtmospheres.Num() > 0)
    {
        SkyAtmosphere = Cast<ASkyAtmosphere>(SkyAtmospheres[0]);
    }
    
    // Find ExponentialHeightFog
    TArray<AActor*> HeightFogs;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), HeightFogs);
    if (HeightFogs.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(HeightFogs[0]);
    }
}

void ALight_AtmosphereManager::SetTimeOfDay(float Hours)
{
    CurrentTimeOfDay = FMath::Clamp(Hours, 0.0f, 24.0f);
    UpdateLightingForTime(CurrentTimeOfDay);
}

void ALight_AtmosphereManager::ApplyCretaceousAtmosphere()
{
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(CretaceousSettings.SunIntensity);
        LightComp->SetLightColor(CretaceousSettings.SunColor);
    }
    
    if (HeightFog && HeightFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
        FogComp->SetFogDensity(CretaceousSettings.FogDensity);
        FogComp->SetFogHeightFalloff(CretaceousSettings.FogHeightFalloff);
        FogComp->SetFogInscatteringColor(CretaceousSettings.FogInscatteringColor);
    }
}

void ALight_AtmosphereManager::UpdateSunPosition()
{
    if (!SunLight) return;
    
    // Calculate sun rotation based on time of day
    float SunAngle = (CurrentTimeOfDay / 24.0f) * 360.0f - 90.0f; // -90 to start at sunrise
    FRotator SunRotation = FRotator(SunAngle, 0.0f, 0.0f);
    SunLight->SetActorRotation(SunRotation);
}

void ALight_AtmosphereManager::UpdateLightingForTime(float TimeHours)
{
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        
        // Update sun position
        float SunAngle = (TimeHours / 24.0f) * 360.0f - 90.0f;
        SunLight->SetActorRotation(FRotator(SunAngle, 0.0f, 0.0f));
        
        // Update intensity and color based on time
        float Intensity = GetSunIntensityForTime(TimeHours);
        FLinearColor Color = GetSunColorForTime(TimeHours);
        
        LightComp->SetIntensity(Intensity);
        LightComp->SetLightColor(Color);
    }
}

ELight_TimeOfDay ALight_AtmosphereManager::GetCurrentTimeOfDay() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)
        return ELight_TimeOfDay::Dawn;
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 11.0f)
        return ELight_TimeOfDay::Morning;
    else if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 15.0f)
        return ELight_TimeOfDay::Midday;
    else if (CurrentTimeOfDay >= 15.0f && CurrentTimeOfDay < 18.0f)
        return ELight_TimeOfDay::Afternoon;
    else if (CurrentTimeOfDay >= 18.0f && CurrentTimeOfDay < 20.0f)
        return ELight_TimeOfDay::Dusk;
    else
        return ELight_TimeOfDay::Night;
}

FLinearColor ALight_AtmosphereManager::GetSunColorForTime(float TimeHours) const
{
    // Cretaceous period warm lighting throughout day
    if (TimeHours >= 6.0f && TimeHours <= 18.0f) // Daytime
    {
        if (TimeHours <= 8.0f || TimeHours >= 16.0f) // Golden hours
        {
            return FLinearColor(1.0f, 0.8f, 0.6f, 1.0f); // Warm orange-gold
        }
        else // Midday
        {
            return FLinearColor(1.0f, 0.94f, 0.78f, 1.0f); // Warm white
        }
    }
    else // Night
    {
        return FLinearColor(0.3f, 0.4f, 0.6f, 1.0f); // Cool moonlight
    }
}

float ALight_AtmosphereManager::GetSunIntensityForTime(float TimeHours) const
{
    if (TimeHours >= 6.0f && TimeHours <= 18.0f) // Daytime
    {
        // Smooth curve for sun intensity
        float NormalizedTime = (TimeHours - 6.0f) / 12.0f; // 0-1 over day
        float SineCurve = FMath::Sin(NormalizedTime * PI);
        return FMath::Lerp(1.0f, 8.0f, SineCurve); // 1-8 intensity range
    }
    else // Night
    {
        return 0.1f; // Very dim moonlight
    }
}