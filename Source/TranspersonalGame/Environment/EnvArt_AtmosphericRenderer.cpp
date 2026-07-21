#include "EnvArt_AtmosphericRenderer.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/AtmosphericFogComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/AtmosphericFog.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AEnvArt_AtmosphericRenderer::AEnvArt_AtmosphericRenderer()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    CurrentTimeOfDay = 12.0f; // Noon
    DayDuration = 1200.0f; // 20 minutes real time = 24 hours game time
    SunIntensityMultiplier = 1.0f;
    FogDensityMultiplier = 1.0f;
    AtmosphericPerspectiveMultiplier = 1.0f;
    
    // Golden hour settings
    GoldenHourStart = 6.0f;
    GoldenHourEnd = 8.0f;
    GoldenHourIntensity = 1.5f;
    
    // Atmospheric colors
    DawnColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    NoonColor = FLinearColor(1.0f, 1.0f, 0.9f, 1.0f);
    DuskColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    NightColor = FLinearColor(0.1f, 0.2f, 0.4f, 1.0f);
    
    bEnableDynamicWeather = true;
    WeatherIntensity = 0.5f;
    
    // Initialize component references
    DirectionalLight = nullptr;
    SkyLight = nullptr;
    AtmosphericFog = nullptr;
    HeightFog = nullptr;
}

void AEnvArt_AtmosphericRenderer::BeginPlay()
{
    Super::BeginPlay();
    
    // Find atmospheric components in the world
    FindAtmosphericComponents();
    
    // Start atmospheric update timer
    GetWorldTimerManager().SetTimer(
        AtmosphericUpdateTimer,
        this,
        &AEnvArt_AtmosphericRenderer::UpdateAtmosphericEffects,
        0.1f, // Update every 0.1 seconds
        true
    );
    
    // Initialize atmospheric state
    UpdateAtmosphericEffects();
}

void AEnvArt_AtmosphericRenderer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update time of day
    CurrentTimeOfDay += (DeltaTime / DayDuration) * 24.0f;
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }
}

void AEnvArt_AtmosphericRenderer::FindAtmosphericComponents()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find directional light
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    if (DirectionalLights.Num() > 0)
    {
        DirectionalLight = Cast<ADirectionalLight>(DirectionalLights[0]);
    }
    
    // Find sky light
    TArray<AActor*> SkyLights;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
    if (SkyLights.Num() > 0)
    {
        SkyLight = Cast<ASkyLight>(SkyLights[0]);
    }
    
    // Find atmospheric fog
    TArray<AActor*> AtmosphericFogs;
    UGameplayStatics::GetAllActorsOfClass(World, AAtmosphericFog::StaticClass(), AtmosphericFogs);
    if (AtmosphericFogs.Num() > 0)
    {
        AtmosphericFog = Cast<AAtmosphericFog>(AtmosphericFogs[0]);
    }
    
    // Find height fog
    TArray<AActor*> HeightFogs;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), HeightFogs);
    if (HeightFogs.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(HeightFogs[0]);
    }
}

void AEnvArt_AtmosphericRenderer::UpdateAtmosphericEffects()
{
    UpdateSunLighting();
    UpdateFogEffects();
    UpdateAtmosphericPerspective();
}

void AEnvArt_AtmosphericRenderer::UpdateSunLighting()
{
    if (!DirectionalLight) return;
    
    UDirectionalLightComponent* LightComponent = DirectionalLight->GetLightComponent();
    if (!LightComponent) return;
    
    // Calculate sun angle based on time of day
    float SunAngle = (CurrentTimeOfDay / 24.0f) * 360.0f - 90.0f; // -90 to start at horizon
    float SunElevation = FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 90.0f;
    float SunAzimuth = (CurrentTimeOfDay / 24.0f) * 360.0f;
    
    // Set sun rotation
    FRotator SunRotation = FRotator(SunElevation, SunAzimuth, 0.0f);
    DirectionalLight->SetActorRotation(SunRotation);
    
    // Calculate sun intensity
    float SunIntensity = FMath::Max(0.0f, FMath::Sin(FMath::DegreesToRadians(SunAngle)));
    
    // Apply golden hour boost
    if (IsGoldenHour())
    {
        SunIntensity *= GoldenHourIntensity;
    }
    
    LightComponent->SetIntensity(SunIntensity * SunIntensityMultiplier);
    
    // Update sun color based on time of day
    FLinearColor SunColor = CalculateSunColor();
    LightComponent->SetLightColor(SunColor);
}

void AEnvArt_AtmosphericRenderer::UpdateFogEffects()
{
    // Update atmospheric fog
    if (AtmosphericFog)
    {
        UAtmosphericFogComponent* FogComponent = AtmosphericFog->GetAtmosphericFogComponent();
        if (FogComponent)
        {
            float FogMultiplier = CalculateFogDensity();
            FogComponent->FogMultiplier = FogMultiplier * FogDensityMultiplier;
        }
    }
    
    // Update height fog
    if (HeightFog)
    {
        UExponentialHeightFogComponent* HeightFogComponent = HeightFog->GetComponent();
        if (HeightFogComponent)
        {
            float FogDensity = CalculateFogDensity() * 0.02f;
            HeightFogComponent->FogDensity = FogDensity * FogDensityMultiplier;
            
            // Update fog color
            FLinearColor FogColor = CalculateFogColor();
            HeightFogComponent->FogInscatteringColor = FogColor;
        }
    }
}

void AEnvArt_AtmosphericRenderer::UpdateAtmosphericPerspective()
{
    // Update sky light for atmospheric perspective
    if (SkyLight)
    {
        USkyLightComponent* SkyLightComponent = SkyLight->GetLightComponent();
        if (SkyLightComponent)
        {
            float SkyIntensity = CalculateSkyLightIntensity();
            SkyLightComponent->SetIntensity(SkyIntensity * AtmosphericPerspectiveMultiplier);
            
            // Update sky light color
            FLinearColor SkyColor = CalculateSkyColor();
            SkyLightComponent->SetLightColor(SkyColor);
        }
    }
}

bool AEnvArt_AtmosphericRenderer::IsGoldenHour() const
{
    return (CurrentTimeOfDay >= GoldenHourStart && CurrentTimeOfDay <= GoldenHourEnd) ||
           (CurrentTimeOfDay >= (18.0f + GoldenHourStart) && CurrentTimeOfDay <= (18.0f + GoldenHourEnd));
}

FLinearColor AEnvArt_AtmosphericRenderer::CalculateSunColor() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay <= 7.0f)
    {
        // Dawn
        float Alpha = (CurrentTimeOfDay - 5.0f) / 2.0f;
        return FMath::Lerp(NightColor, DawnColor, Alpha);
    }
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay <= 11.0f)
    {
        // Morning to noon
        float Alpha = (CurrentTimeOfDay - 7.0f) / 4.0f;
        return FMath::Lerp(DawnColor, NoonColor, Alpha);
    }
    else if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay <= 17.0f)
    {
        // Noon
        return NoonColor;
    }
    else if (CurrentTimeOfDay >= 17.0f && CurrentTimeOfDay <= 19.0f)
    {
        // Dusk
        float Alpha = (CurrentTimeOfDay - 17.0f) / 2.0f;
        return FMath::Lerp(NoonColor, DuskColor, Alpha);
    }
    else
    {
        // Night
        return NightColor;
    }
}

FLinearColor AEnvArt_AtmosphericRenderer::CalculateFogColor() const
{
    // Fog color follows sun color but is more muted
    FLinearColor SunColor = CalculateSunColor();
    return FLinearColor(
        SunColor.R * 0.8f,
        SunColor.G * 0.85f,
        SunColor.B * 0.9f,
        1.0f
    );
}

FLinearColor AEnvArt_AtmosphericRenderer::CalculateSkyColor() const
{
    // Sky color is brighter version of sun color
    FLinearColor SunColor = CalculateSunColor();
    return FLinearColor(
        FMath::Min(1.0f, SunColor.R * 1.2f),
        FMath::Min(1.0f, SunColor.G * 1.1f),
        FMath::Min(1.0f, SunColor.B * 1.0f),
        1.0f
    );
}

float AEnvArt_AtmosphericRenderer::CalculateFogDensity() const
{
    // Base fog density varies by time of day
    float BaseDensity = 0.5f;
    
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay <= 9.0f)
    {
        // Morning fog
        BaseDensity = 1.0f;
    }
    else if (CurrentTimeOfDay >= 18.0f && CurrentTimeOfDay <= 22.0f)
    {
        // Evening fog
        BaseDensity = 0.8f;
    }
    
    // Apply weather influence
    if (bEnableDynamicWeather)
    {
        BaseDensity *= (1.0f + WeatherIntensity);
    }
    
    return BaseDensity;
}

float AEnvArt_AtmosphericRenderer::CalculateSkyLightIntensity() const
{
    // Sky light intensity follows sun but is more stable
    float SunAngle = (CurrentTimeOfDay / 24.0f) * 360.0f - 90.0f;
    float BaseIntensity = FMath::Max(0.1f, FMath::Sin(FMath::DegreesToRadians(SunAngle)));
    
    return BaseIntensity * 0.8f; // Slightly dimmer than direct sun
}

void AEnvArt_AtmosphericRenderer::SetTimeOfDay(float NewTimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
    UpdateAtmosphericEffects();
}

void AEnvArt_AtmosphericRenderer::SetWeatherIntensity(float NewWeatherIntensity)
{
    WeatherIntensity = FMath::Clamp(NewWeatherIntensity, 0.0f, 2.0f);
    UpdateAtmosphericEffects();
}

void AEnvArt_AtmosphericRenderer::EnableGoldenHourLighting()
{
    SetTimeOfDay(7.0f); // Set to golden hour
    GoldenHourIntensity = 2.0f;
    UpdateAtmosphericEffects();
}

void AEnvArt_AtmosphericRenderer::ResetAtmosphericSettings()
{
    CurrentTimeOfDay = 12.0f;
    SunIntensityMultiplier = 1.0f;
    FogDensityMultiplier = 1.0f;
    AtmosphericPerspectiveMultiplier = 1.0f;
    WeatherIntensity = 0.5f;
    GoldenHourIntensity = 1.5f;
    
    UpdateAtmosphericEffects();
}