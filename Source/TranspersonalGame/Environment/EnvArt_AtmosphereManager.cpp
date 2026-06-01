#include "EnvArt_AtmosphereManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AEnvArt_AtmosphereManager::AEnvArt_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize time of day presets
    DawnSettings.SunAngle = 15.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.SkyColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.FogColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    NoonSettings.SunAngle = 80.0f;
    NoonSettings.SunColor = FLinearColor(1.0f, 1.0f, 0.9f, 1.0f);
    NoonSettings.SunIntensity = 5.0f;
    NoonSettings.SkyColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);
    NoonSettings.FogDensity = 0.01f;
    NoonSettings.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    DuskSettings.SunAngle = 10.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    DuskSettings.SunIntensity = 1.5f;
    DuskSettings.SkyColor = FLinearColor(1.0f, 0.4f, 0.2f, 1.0f);
    DuskSettings.FogDensity = 0.08f;
    DuskSettings.FogColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);

    NightSettings.SunAngle = -20.0f;
    NightSettings.SunColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SkyColor = FLinearColor(0.1f, 0.1f, 0.3f, 1.0f);
    NightSettings.FogDensity = 0.03f;
    NightSettings.FogColor = FLinearColor(0.2f, 0.2f, 0.4f, 1.0f);

    SunLight = nullptr;
    SkyLightActor = nullptr;
    HeightFogActor = nullptr;
}

void AEnvArt_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindAndConfigureLights();
    FindAndConfigureFog();
    FindAndConfigureSkyAtmosphere();
    
    // Set initial golden hour lighting
    SetGoldenHourLighting();
}

void AEnvArt_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDynamicTimeOfDay)
    {
        TimeAccumulator += DeltaTime * TimeOfDaySpeed;
        if (TimeAccumulator >= 1.0f)
        {
            CurrentTimeOfDay += TimeAccumulator;
            if (CurrentTimeOfDay >= 24.0f)
            {
                CurrentTimeOfDay -= 24.0f;
            }
            TimeAccumulator = 0.0f;
            UpdateAtmosphere();
        }
    }
}

void AEnvArt_AtmosphereManager::SetTimeOfDay(float NewTimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
    UpdateAtmosphere();
}

void AEnvArt_AtmosphereManager::ApplyTimeOfDayPreset(const FEnvArt_TimeOfDaySettings& Settings)
{
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetLightColor(Settings.SunColor);
        LightComp->SetIntensity(Settings.SunIntensity);
        
        FRotator SunRotation = FRotator(-Settings.SunAngle, 0.0f, 0.0f);
        SunLight->SetActorRotation(SunRotation);
    }

    if (SkyLightActor && SkyLightActor->GetLightComponent())
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetLightComponent();
        SkyComp->SetLightColor(Settings.SkyColor);
        SkyComp->RecaptureSky();
    }

    if (HeightFogActor && HeightFogActor->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = HeightFogActor->GetComponent();
        FogComp->SetFogDensity(Settings.FogDensity);
        FogComp->SetFogInscatteringColor(Settings.FogColor);
    }
}

void AEnvArt_AtmosphereManager::UpdateAtmosphere()
{
    FEnvArt_TimeOfDaySettings CurrentSettings = GetCurrentTimeOfDaySettings();
    ApplyTimeOfDayPreset(CurrentSettings);
}

void AEnvArt_AtmosphereManager::SetGoldenHourLighting()
{
    CurrentTimeOfDay = 17.0f; // 5 PM
    
    FEnvArt_TimeOfDaySettings GoldenHour;
    GoldenHour.SunAngle = 25.0f;
    GoldenHour.SunColor = FLinearColor(1.0f, 0.8f, 0.5f, 1.0f);
    GoldenHour.SunIntensity = 3.5f;
    GoldenHour.SkyColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    GoldenHour.FogDensity = 0.04f;
    GoldenHour.FogColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    
    ApplyTimeOfDayPreset(GoldenHour);
}

void AEnvArt_AtmosphereManager::SetMidDayLighting()
{
    CurrentTimeOfDay = 12.0f; // Noon
    ApplyTimeOfDayPreset(NoonSettings);
}

void AEnvArt_AtmosphereManager::SetDuskLighting()
{
    CurrentTimeOfDay = 19.0f; // 7 PM
    ApplyTimeOfDayPreset(DuskSettings);
}

void AEnvArt_AtmosphereManager::EnableVolumetricFog(bool bEnable)
{
    bEnableVolumetricFog = bEnable;
    
    if (HeightFogActor && HeightFogActor->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = HeightFogActor->GetComponent();
        FogComp->SetVolumetricFog(bEnable);
        FogComp->SetVolumetricFogExtinctionScale(VolumetricFogExtinctionScale);
        FogComp->SetVolumetricFogScatteringDistribution(VolumetricFogScatteringDistribution);
        FogComp->SetVolumetricFogAlbedo(VolumetricFogAlbedo);
    }
}

void AEnvArt_AtmosphereManager::FindAndConfigureLights()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find directional light (sun)
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    
    if (DirectionalLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(DirectionalLights[0]);
        if (SunLight && SunLight->GetLightComponent())
        {
            SunLight->GetLightComponent()->SetCastShadows(true);
            SunLight->GetLightComponent()->SetCastVolumetricShadow(true);
        }
    }

    // Find sky light
    TArray<AActor*> SkyLights;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
    
    if (SkyLights.Num() > 0)
    {
        SkyLightActor = Cast<ASkyLight>(SkyLights[0]);
        if (SkyLightActor && SkyLightActor->GetLightComponent())
        {
            SkyLightActor->GetLightComponent()->SetIntensity(1.0f);
            SkyLightActor->GetLightComponent()->SetSourceType(SLS_CapturedScene);
        }
    }
}

void AEnvArt_AtmosphereManager::FindAndConfigureFog()
{
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
    
    if (FogActors.Num() > 0)
    {
        HeightFogActor = Cast<AExponentialHeightFog>(FogActors[0]);
        if (HeightFogActor && HeightFogActor->GetComponent())
        {
            UExponentialHeightFogComponent* FogComp = HeightFogActor->GetComponent();
            FogComp->SetFogHeightFalloff(0.2f);
            FogComp->SetFogMaxOpacity(1.0f);
            FogComp->SetStartDistance(5000.0f);
            EnableVolumetricFog(bEnableVolumetricFog);
        }
    }
}

void AEnvArt_AtmosphereManager::FindAndConfigureSkyAtmosphere()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Sky atmosphere is typically a component on the world, not a separate actor
    // We'll configure it if we find one
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (USkyAtmosphereComponent* SkyAtmosphere = Actor->FindComponentByClass<USkyAtmosphereComponent>())
        {
            SkyAtmosphere->SetRayleighScatteringScale(1.0f);
            SkyAtmosphere->SetRayleighExponentialDistribution(8.0f);
            SkyAtmosphere->SetMieScatteringScale(1.0f);
            SkyAtmosphere->SetMieAnisotropy(0.8f);
            break;
        }
    }
}

FEnvArt_TimeOfDaySettings AEnvArt_AtmosphereManager::InterpolateSettings(const FEnvArt_TimeOfDaySettings& A, const FEnvArt_TimeOfDaySettings& B, float Alpha)
{
    FEnvArt_TimeOfDaySettings Result;
    
    Result.SunAngle = FMath::Lerp(A.SunAngle, B.SunAngle, Alpha);
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SkyColor = FMath::Lerp(A.SkyColor, B.SkyColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    
    return Result;
}

FEnvArt_TimeOfDaySettings AEnvArt_AtmosphereManager::GetCurrentTimeOfDaySettings()
{
    float TimeNormalized = CurrentTimeOfDay / 24.0f;
    
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 10.0f)
    {
        // Dawn to Morning
        float Alpha = (CurrentTimeOfDay - 5.0f) / 5.0f;
        return InterpolateSettings(DawnSettings, NoonSettings, Alpha);
    }
    else if (CurrentTimeOfDay >= 10.0f && CurrentTimeOfDay < 16.0f)
    {
        // Morning to Afternoon (stay at noon settings)
        return NoonSettings;
    }
    else if (CurrentTimeOfDay >= 16.0f && CurrentTimeOfDay < 20.0f)
    {
        // Afternoon to Dusk
        float Alpha = (CurrentTimeOfDay - 16.0f) / 4.0f;
        return InterpolateSettings(NoonSettings, DuskSettings, Alpha);
    }
    else if (CurrentTimeOfDay >= 20.0f && CurrentTimeOfDay < 22.0f)
    {
        // Dusk to Night
        float Alpha = (CurrentTimeOfDay - 20.0f) / 2.0f;
        return InterpolateSettings(DuskSettings, NightSettings, Alpha);
    }
    else
    {
        // Night to Dawn
        if (CurrentTimeOfDay >= 22.0f)
        {
            float Alpha = (CurrentTimeOfDay - 22.0f) / 7.0f; // 22:00 to 05:00 = 7 hours
            return InterpolateSettings(NightSettings, DawnSettings, Alpha);
        }
        else
        {
            float Alpha = (CurrentTimeOfDay + 2.0f) / 7.0f; // Continue from 00:00 to 05:00
            return InterpolateSettings(NightSettings, DawnSettings, Alpha);
        }
    }
}