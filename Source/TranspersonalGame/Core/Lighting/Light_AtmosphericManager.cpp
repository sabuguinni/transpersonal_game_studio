#include "Light_AtmosphericManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ALight_AtmosphericManager::ALight_AtmosphericManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default atmospheric settings
    DawnSettings.SunIntensity = 4.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnSettings.SunTemperature = 4500.0f;
    DawnSettings.FogDensity = 0.04f;

    MiddaySettings.SunIntensity = 8.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
    MiddaySettings.SunTemperature = 5800.0f;
    MiddaySettings.FogDensity = 0.02f;

    DuskSettings.SunIntensity = 3.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    DuskSettings.SunTemperature = 3500.0f;
    DuskSettings.FogDensity = 0.05f;

    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightSettings.SunTemperature = 8000.0f;
    NightSettings.FogDensity = 0.03f;
}

void ALight_AtmosphericManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeLightingReferences();
    ApplyAtmosphericSettings(MiddaySettings);
}

void ALight_AtmosphericManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        UpdateDayNightCycle(DeltaTime);
    }
}

void ALight_AtmosphericManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Fmod(NewTime, 24.0f);
    if (CurrentTimeOfDay < 0.0f)
    {
        CurrentTimeOfDay += 24.0f;
    }

    UpdateSunPosition();
    FLight_AtmosphericSettings InterpolatedSettings = GetInterpolatedSettings();
    ApplyAtmosphericSettings(InterpolatedSettings);
}

ELight_TimeOfDay ALight_AtmosphericManager::GetCurrentTimePhase() const
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

void ALight_AtmosphericManager::ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings)
{
    // Apply sun light settings
    if (MainSunLight && MainSunLight->GetLightComponent())
    {
        UDirectionalLightComponent* SunComp = MainSunLight->GetLightComponent();
        SunComp->SetIntensity(Settings.SunIntensity);
        SunComp->SetLightColor(Settings.SunColor);
        SunComp->SetTemperature(Settings.SunTemperature);
        SunComp->SetUseTemperature(true);
        SunComp->SetCastVolumetricShadow(true);
        SunComp->SetVolumetricScatteringIntensity(Settings.VolumetricScatteringIntensity);
    }

    // Apply atmospheric fog settings
    if (AtmosphericFog && AtmosphericFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = AtmosphericFog->GetComponent();
        FogComp->SetFogDensity(Settings.FogDensity);
        FogComp->SetFogHeightFalloff(Settings.FogHeightFalloff);
        FogComp->SetFogInscatteringColor(Settings.FogInscatteringColor);
        FogComp->SetVolumetricFog(true);
        FogComp->SetVolumetricFogScatteringDistribution(0.8f);
        FogComp->SetVolumetricFogAlbedo(Settings.VolumetricAlbedo);
        FogComp->SetVolumetricFogExtinctionScale(3.0f);
    }

    // Update sky light
    if (SkyLightActor && SkyLightActor->GetLightComponent())
    {
        SkyLightActor->GetLightComponent()->RecaptureSky();
    }
}

void ALight_AtmosphericManager::InitializeLightingReferences()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find main directional light
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        ADirectionalLight* DirLight = Cast<ADirectionalLight>(Actor);
        if (DirLight && DirLight->GetName().Contains(TEXT("Sun")) || DirLight->GetActorLabel().Contains(TEXT("Sun")))
        {
            MainSunLight = DirLight;
            break;
        }
    }

    if (!MainSunLight && FoundActors.Num() > 0)
    {
        MainSunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }

    // Find rim light
    for (AActor* Actor : FoundActors)
    {
        ADirectionalLight* DirLight = Cast<ADirectionalLight>(Actor);
        if (DirLight && (DirLight->GetName().Contains(TEXT("Rim")) || DirLight->GetActorLabel().Contains(TEXT("Rim"))))
        {
            RimLight = DirLight;
            break;
        }
    }

    // Find sky light
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
    }

    // Find atmospheric fog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        AtmosphericFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }
}

void ALight_AtmosphericManager::UpdateSunPosition()
{
    if (MainSunLight)
    {
        FRotator SunRotation = CalculateSunRotation();
        MainSunLight->SetActorRotation(SunRotation);
    }

    if (RimLight)
    {
        FRotator RimRotation = CalculateSunRotation();
        RimRotation.Yaw += 120.0f; // Offset for rim lighting
        RimRotation.Pitch -= 30.0f;
        RimLight->SetActorRotation(RimRotation);
    }
}

void ALight_AtmosphericManager::UpdateAtmosphericFog()
{
    FLight_AtmosphericSettings CurrentSettings = GetInterpolatedSettings();
    ApplyAtmosphericSettings(CurrentSettings);
}

FLight_AtmosphericSettings ALight_AtmosphericManager::GetInterpolatedSettings() const
{
    FLight_AtmosphericSettings Result;

    // Determine which settings to interpolate between
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 12.0f)
    {
        // Dawn to Midday
        float Alpha = (CurrentTimeOfDay - 5.0f) / 7.0f;
        Result.SunIntensity = FMath::Lerp(DawnSettings.SunIntensity, MiddaySettings.SunIntensity, Alpha);
        Result.SunColor = FMath::Lerp(DawnSettings.SunColor, MiddaySettings.SunColor, Alpha);
        Result.SunTemperature = FMath::Lerp(DawnSettings.SunTemperature, MiddaySettings.SunTemperature, Alpha);
        Result.FogDensity = FMath::Lerp(DawnSettings.FogDensity, MiddaySettings.FogDensity, Alpha);
    }
    else if (CurrentTimeOfDay >= 12.0f && CurrentTimeOfDay < 18.0f)
    {
        // Midday to Dusk
        float Alpha = (CurrentTimeOfDay - 12.0f) / 6.0f;
        Result.SunIntensity = FMath::Lerp(MiddaySettings.SunIntensity, DuskSettings.SunIntensity, Alpha);
        Result.SunColor = FMath::Lerp(MiddaySettings.SunColor, DuskSettings.SunColor, Alpha);
        Result.SunTemperature = FMath::Lerp(MiddaySettings.SunTemperature, DuskSettings.SunTemperature, Alpha);
        Result.FogDensity = FMath::Lerp(MiddaySettings.FogDensity, DuskSettings.FogDensity, Alpha);
    }
    else if (CurrentTimeOfDay >= 18.0f || CurrentTimeOfDay < 5.0f)
    {
        // Dusk to Night to Dawn
        float NightTime = (CurrentTimeOfDay >= 18.0f) ? CurrentTimeOfDay - 18.0f : CurrentTimeOfDay + 6.0f;
        float Alpha = NightTime / 11.0f;
        
        if (Alpha < 0.5f)
        {
            // Dusk to Night
            float LocalAlpha = Alpha * 2.0f;
            Result.SunIntensity = FMath::Lerp(DuskSettings.SunIntensity, NightSettings.SunIntensity, LocalAlpha);
            Result.SunColor = FMath::Lerp(DuskSettings.SunColor, NightSettings.SunColor, LocalAlpha);
            Result.SunTemperature = FMath::Lerp(DuskSettings.SunTemperature, NightSettings.SunTemperature, LocalAlpha);
            Result.FogDensity = FMath::Lerp(DuskSettings.FogDensity, NightSettings.FogDensity, LocalAlpha);
        }
        else
        {
            // Night to Dawn
            float LocalAlpha = (Alpha - 0.5f) * 2.0f;
            Result.SunIntensity = FMath::Lerp(NightSettings.SunIntensity, DawnSettings.SunIntensity, LocalAlpha);
            Result.SunColor = FMath::Lerp(NightSettings.SunColor, DawnSettings.SunColor, LocalAlpha);
            Result.SunTemperature = FMath::Lerp(NightSettings.SunTemperature, DawnSettings.SunTemperature, LocalAlpha);
            Result.FogDensity = FMath::Lerp(NightSettings.FogDensity, DawnSettings.FogDensity, LocalAlpha);
        }
    }

    return Result;
}

void ALight_AtmosphericManager::UpdateDayNightCycle(float DeltaTime)
{
    CurrentTimeOfDay += (DeltaTime / 3600.0f) * TimeSpeed; // Convert to hours
    CurrentTimeOfDay = FMath::Fmod(CurrentTimeOfDay, 24.0f);

    UpdateSunPosition();
    UpdateAtmosphericFog();
}

FRotator ALight_AtmosphericManager::CalculateSunRotation() const
{
    // Calculate sun position based on time of day
    float SunAngle = (CurrentTimeOfDay / 24.0f) * 360.0f - 90.0f; // -90 to start at dawn
    float ElevationAngle = GetSunElevationAngle();
    
    return FRotator(-ElevationAngle, SunAngle, 0.0f);
}

float ALight_AtmosphericManager::GetSunElevationAngle() const
{
    // Calculate sun elevation based on time of day
    float NormalizedTime = CurrentTimeOfDay / 24.0f;
    float SunCycle = FMath::Sin(NormalizedTime * PI * 2.0f - PI * 0.5f);
    
    // Map to reasonable elevation angles (-20 to 80 degrees)
    return FMath::Lerp(-20.0f, 80.0f, (SunCycle + 1.0f) * 0.5f);
}