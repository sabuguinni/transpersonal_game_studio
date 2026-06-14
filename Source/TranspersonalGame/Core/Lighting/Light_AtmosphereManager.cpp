#include "Light_AtmosphereManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Math/UnrealMathUtility.h"

ALight_AtmosphereManager::ALight_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize default time settings
    CurrentTime.Hours = 12.0f;
    CurrentTime.Minutes = 0.0f;
    CurrentTime.SunIntensity = 8.0f;
    CurrentTime.SunTemperature = 5800.0f;
    CurrentTime.FogDensity = 0.02f;
    CurrentTime.FogColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);

    // Setup lighting presets for Cretaceous period
    DawnSettings.Hours = 6.0f;
    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SunTemperature = 4500.0f;
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.FogColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);

    NoonSettings.Hours = 12.0f;
    NoonSettings.SunIntensity = 10.0f;
    NoonSettings.SunTemperature = 6000.0f;
    NoonSettings.FogDensity = 0.01f;
    NoonSettings.FogColor = FLinearColor(0.9f, 0.9f, 0.8f, 1.0f);

    DuskSettings.Hours = 18.0f;
    DuskSettings.SunIntensity = 4.0f;
    DuskSettings.SunTemperature = 3500.0f;
    DuskSettings.FogDensity = 0.04f;
    DuskSettings.FogColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);

    NightSettings.Hours = 0.0f;
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunTemperature = 4000.0f;
    NightSettings.FogDensity = 0.03f;
    NightSettings.FogColor = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);

    SunLight = nullptr;
    SkyAtmosphere = nullptr;
    HeightFog = nullptr;
}

void ALight_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindOrCreateLightingActors();
    SetupCretaceousAtmosphere();
    EnableLumenGlobalIllumination();
    ApplyLightingSettings(CurrentTime);
}

void ALight_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        // Advance time based on day duration
        float TimeIncrement = (24.0f * 60.0f) / (DayDurationMinutes * 60.0f) * DeltaTime;
        CurrentTime.Minutes += TimeIncrement;
        
        if (CurrentTime.Minutes >= 60.0f)
        {
            CurrentTime.Hours += 1.0f;
            CurrentTime.Minutes = 0.0f;
        }
        
        if (CurrentTime.Hours >= 24.0f)
        {
            CurrentTime.Hours = 0.0f;
        }

        // Interpolate lighting settings based on time
        FLight_TimeOfDay InterpolatedSettings = InterpolateLightingSettings(GetNormalizedTimeOfDay());
        ApplyLightingSettings(InterpolatedSettings);
        UpdateSunPosition();
    }
}

void ALight_AtmosphereManager::SetTimeOfDay(float Hours, float Minutes)
{
    CurrentTime.Hours = FMath::Clamp(Hours, 0.0f, 23.0f);
    CurrentTime.Minutes = FMath::Clamp(Minutes, 0.0f, 59.0f);
    
    FLight_TimeOfDay InterpolatedSettings = InterpolateLightingSettings(GetNormalizedTimeOfDay());
    ApplyLightingSettings(InterpolatedSettings);
    UpdateSunPosition();
}

void ALight_AtmosphereManager::ApplyLightingSettings(const FLight_TimeOfDay& Settings)
{
    if (SunLight && SunLight->GetLightComponent())
    {
        SunLight->GetLightComponent()->SetIntensity(Settings.SunIntensity);
        SunLight->GetLightComponent()->SetTemperature(Settings.SunTemperature);
    }

    if (HeightFog && HeightFog->GetComponent())
    {
        HeightFog->GetComponent()->SetFogDensity(Settings.FogDensity);
        HeightFog->GetComponent()->SetFogInscatteringColor(Settings.FogColor);
    }
}

void ALight_AtmosphereManager::SetupCretaceousAtmosphere()
{
    if (SkyAtmosphere && SkyAtmosphere->GetAtmosphereComponent())
    {
        // Configure atmosphere for Cretaceous period - warmer, more humid
        SkyAtmosphere->GetAtmosphereComponent()->SetRayleighScatteringScale(0.8f);
        SkyAtmosphere->GetAtmosphereComponent()->SetMieScatteringScale(0.6f);
        SkyAtmosphere->GetAtmosphereComponent()->SetMieAbsorptionScale(0.4f);
    }

    if (HeightFog && HeightFog->GetComponent())
    {
        // Setup volumetric fog for prehistoric atmosphere
        HeightFog->GetComponent()->SetVolumetricFog(true);
        HeightFog->GetComponent()->SetFogHeightFalloff(0.2f);
        HeightFog->GetComponent()->SetFogMaxOpacity(0.8f);
        HeightFog->GetComponent()->SetStartDistance(1000.0f);
    }
}

void ALight_AtmosphereManager::EnableLumenGlobalIllumination()
{
    if (UWorld* World = GetWorld())
    {
        // Enable Lumen for dynamic global illumination
        if (GEngine)
        {
            GEngine->Exec(World, TEXT("r.Lumen.GlobalIllumination 1"));
            GEngine->Exec(World, TEXT("r.Lumen.Reflections 1"));
            GEngine->Exec(World, TEXT("r.Lumen.ScreenProbeGather 1"));
        }
    }
}

void ALight_AtmosphereManager::FindOrCreateLightingActors()
{
    if (UWorld* World = GetWorld())
    {
        // Find existing directional light
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
        
        if (FoundActors.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        }

        // Find sky atmosphere
        FoundActors.Empty();
        UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
        
        if (FoundActors.Num() > 0)
        {
            SkyAtmosphere = Cast<ASkyAtmosphere>(FoundActors[0]);
        }

        // Find height fog
        FoundActors.Empty();
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
        
        if (FoundActors.Num() > 0)
        {
            HeightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
        }
    }
}

void ALight_AtmosphereManager::UpdateSunPosition()
{
    if (SunLight)
    {
        // Calculate sun angle based on time of day
        float TimeOfDay = GetNormalizedTimeOfDay();
        float SunAngle = (TimeOfDay - 0.5f) * 180.0f; // -90 to +90 degrees
        
        FRotator SunRotation = FRotator(SunAngle, 0.0f, 0.0f);
        SunLight->SetActorRotation(SunRotation);
    }
}

FLight_TimeOfDay ALight_AtmosphereManager::InterpolateLightingSettings(float TimeOfDay)
{
    FLight_TimeOfDay Result;
    
    if (TimeOfDay < 0.25f) // Night to Dawn
    {
        float Alpha = TimeOfDay / 0.25f;
        Result.SunIntensity = FMath::Lerp(NightSettings.SunIntensity, DawnSettings.SunIntensity, Alpha);
        Result.SunTemperature = FMath::Lerp(NightSettings.SunTemperature, DawnSettings.SunTemperature, Alpha);
        Result.FogDensity = FMath::Lerp(NightSettings.FogDensity, DawnSettings.FogDensity, Alpha);
        Result.FogColor = FMath::Lerp(NightSettings.FogColor, DawnSettings.FogColor, Alpha);
    }
    else if (TimeOfDay < 0.5f) // Dawn to Noon
    {
        float Alpha = (TimeOfDay - 0.25f) / 0.25f;
        Result.SunIntensity = FMath::Lerp(DawnSettings.SunIntensity, NoonSettings.SunIntensity, Alpha);
        Result.SunTemperature = FMath::Lerp(DawnSettings.SunTemperature, NoonSettings.SunTemperature, Alpha);
        Result.FogDensity = FMath::Lerp(DawnSettings.FogDensity, NoonSettings.FogDensity, Alpha);
        Result.FogColor = FMath::Lerp(DawnSettings.FogColor, NoonSettings.FogColor, Alpha);
    }
    else if (TimeOfDay < 0.75f) // Noon to Dusk
    {
        float Alpha = (TimeOfDay - 0.5f) / 0.25f;
        Result.SunIntensity = FMath::Lerp(NoonSettings.SunIntensity, DuskSettings.SunIntensity, Alpha);
        Result.SunTemperature = FMath::Lerp(NoonSettings.SunTemperature, DuskSettings.SunTemperature, Alpha);
        Result.FogDensity = FMath::Lerp(NoonSettings.FogDensity, DuskSettings.FogDensity, Alpha);
        Result.FogColor = FMath::Lerp(NoonSettings.FogColor, DuskSettings.FogColor, Alpha);
    }
    else // Dusk to Night
    {
        float Alpha = (TimeOfDay - 0.75f) / 0.25f;
        Result.SunIntensity = FMath::Lerp(DuskSettings.SunIntensity, NightSettings.SunIntensity, Alpha);
        Result.SunTemperature = FMath::Lerp(DuskSettings.SunTemperature, NightSettings.SunTemperature, Alpha);
        Result.FogDensity = FMath::Lerp(DuskSettings.FogDensity, NightSettings.FogDensity, Alpha);
        Result.FogColor = FMath::Lerp(DuskSettings.FogColor, NightSettings.FogColor, Alpha);
    }
    
    return Result;
}

float ALight_AtmosphereManager::GetNormalizedTimeOfDay() const
{
    float TotalMinutes = CurrentTime.Hours * 60.0f + CurrentTime.Minutes;
    return TotalMinutes / (24.0f * 60.0f);
}