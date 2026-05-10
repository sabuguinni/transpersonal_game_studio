#include "EnvArt_LightingController.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UEnvArt_LightingController::UEnvArt_LightingController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default Cretaceous lighting settings
    InitializeTimeOfDayPresets();
}

void UEnvArt_LightingController::BeginPlay()
{
    Super::BeginPlay();
    
    // Find lighting actors in the scene
    FindLightingActors();
    
    // Setup initial Cretaceous lighting
    SetupCretaceousLighting();
    
    // Apply current time of day
    SetTimeOfDay(CurrentTimeOfDay);
}

void UEnvArt_LightingController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableDayNightCycle)
    {
        UpdateDayNightCycle(DeltaTime);
    }
    
    if (bTransitioning && bSmoothTransitions)
    {
        InterpolateLightingSettings(DeltaTime);
    }
}

void UEnvArt_LightingController::InitializeTimeOfDayPresets()
{
    // Dawn - soft golden light
    FEnvArt_LightingSettings DawnSettings;
    DawnSettings.SunRotation = FRotator(-15.0f, 75.0f, 0.0f);
    DawnSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnSettings.SunIntensity = 5.0f;
    DawnSettings.SkyColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    DawnSettings.SkyIntensity = 0.5f;
    DawnSettings.FogColor = FLinearColor(0.9f, 0.8f, 0.7f, 1.0f);
    DawnSettings.FogDensity = 0.04f;
    TimeOfDayPresets.Add(EEnvArt_TimeOfDay::Dawn, DawnSettings);
    
    // Morning - bright tropical light
    FEnvArt_LightingSettings MorningSettings;
    MorningSettings.SunRotation = FRotator(-30.0f, 45.0f, 0.0f);
    MorningSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
    MorningSettings.SunIntensity = 8.0f;
    MorningSettings.SkyColor = FLinearColor(0.4f, 0.7f, 1.0f, 1.0f);
    MorningSettings.SkyIntensity = 0.8f;
    MorningSettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    MorningSettings.FogDensity = 0.025f;
    TimeOfDayPresets.Add(EEnvArt_TimeOfDay::Morning, MorningSettings);
    
    // Midday - intense tropical sun
    FEnvArt_LightingSettings MiddaySettings;
    MiddaySettings.SunRotation = FRotator(-75.0f, 0.0f, 0.0f);
    MiddaySettings.SunColor = FLinearColor(1.0f, 1.0f, 0.95f, 1.0f);
    MiddaySettings.SunIntensity = 12.0f;
    MiddaySettings.SkyColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);
    MiddaySettings.SkyIntensity = 1.0f;
    MiddaySettings.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    MiddaySettings.FogDensity = 0.015f;
    TimeOfDayPresets.Add(EEnvArt_TimeOfDay::Midday, MiddaySettings);
    
    // Afternoon - warm golden hour
    FEnvArt_LightingSettings AfternoonSettings;
    AfternoonSettings.SunRotation = FRotator(-45.0f, -30.0f, 0.0f);
    AfternoonSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    AfternoonSettings.SunIntensity = 10.0f;
    AfternoonSettings.SkyColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
    AfternoonSettings.SkyIntensity = 0.9f;
    AfternoonSettings.FogColor = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f);
    AfternoonSettings.FogDensity = 0.03f;
    TimeOfDayPresets.Add(EEnvArt_TimeOfDay::Afternoon, AfternoonSettings);
    
    // Dusk - dramatic sunset
    FEnvArt_LightingSettings DuskSettings;
    DuskSettings.SunRotation = FRotator(-10.0f, -75.0f, 0.0f);
    DuskSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    DuskSettings.SunIntensity = 6.0f;
    DuskSettings.SkyColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
    DuskSettings.SkyIntensity = 0.6f;
    DuskSettings.FogColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DuskSettings.FogDensity = 0.05f;
    TimeOfDayPresets.Add(EEnvArt_TimeOfDay::Dusk, DuskSettings);
    
    // Night - moonlit atmosphere
    FEnvArt_LightingSettings NightSettings;
    NightSettings.SunRotation = FRotator(15.0f, -120.0f, 0.0f);
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
    NightSettings.SunIntensity = 0.5f;
    NightSettings.SkyColor = FLinearColor(0.1f, 0.2f, 0.4f, 1.0f);
    NightSettings.SkyIntensity = 0.2f;
    NightSettings.FogColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
    NightSettings.FogDensity = 0.06f;
    TimeOfDayPresets.Add(EEnvArt_TimeOfDay::Night, NightSettings);
}

void UEnvArt_LightingController::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find directional light (sun)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }
    
    // Find sky light
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);
    for (AActor* Actor : FoundActors)
    {
        if (Actor->FindComponentByClass<USkyLightComponent>())
        {
            SkyLightActor = Actor;
            break;
        }
    }
    
    // Find sky atmosphere
    for (AActor* Actor : FoundActors)
    {
        if (Actor->FindComponentByClass<USkyAtmosphereComponent>())
        {
            SkyAtmosphereActor = Actor;
            break;
        }
    }
    
    // Find fog actor
    for (AActor* Actor : FoundActors)
    {
        if (Actor->FindComponentByClass<UExponentialHeightFogComponent>())
        {
            FogActor = Actor;
            break;
        }
    }
    
    // Find post process volume
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(FoundActors[0]);
    }
}

void UEnvArt_LightingController::SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    
    if (TimeOfDayPresets.Contains(NewTimeOfDay))
    {
        TargetSettings = TimeOfDayPresets[NewTimeOfDay];
        
        if (bSmoothTransitions)
        {
            bTransitioning = true;
        }
        else
        {
            CurrentSettings = TargetSettings;
            ApplyLightingSettings(CurrentSettings);
        }
    }
}

void UEnvArt_LightingController::ApplyLightingSettings(const FEnvArt_LightingSettings& Settings)
{
    UpdateSunLight(Settings);
    UpdateSkyLight(Settings);
    UpdateFog(Settings);
    UpdatePostProcess(Settings);
}

void UEnvArt_LightingController::SetupCretaceousLighting()
{
    // Set default morning lighting for Cretaceous period
    CreateGoldenHourAtmosphere();
    SetupVolumetricFog();
}

void UEnvArt_LightingController::CreateGoldenHourAtmosphere()
{
    // Apply golden hour lighting preset
    FEnvArt_LightingSettings GoldenHour;
    GoldenHour.SunRotation = FRotator(-25.0f, 35.0f, 0.0f);
    GoldenHour.SunColor = FLinearColor(1.0f, 0.9f, 0.75f, 1.0f);
    GoldenHour.SunIntensity = 9.0f;
    GoldenHour.SkyColor = FLinearColor(0.6f, 0.8f, 1.0f, 1.0f);
    GoldenHour.SkyIntensity = 0.85f;
    GoldenHour.FogColor = FLinearColor(0.95f, 0.85f, 0.7f, 1.0f);
    GoldenHour.FogDensity = 0.028f;
    
    ApplyLightingSettings(GoldenHour);
}

void UEnvArt_LightingController::SetupVolumetricFog()
{
    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComponent = FogActor->FindComponentByClass<UExponentialHeightFogComponent>();
        if (FogComponent)
        {
            FogComponent->bEnableVolumetricFog = true;
            FogComponent->VolumetricFogScatteringDistribution = 0.2f;
            FogComponent->VolumetricFogAlbedo = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);
            FogComponent->VolumetricFogEmissive = FLinearColor(0.05f, 0.05f, 0.1f, 1.0f);
            FogComponent->VolumetricFogExtinctionScale = 1.0f;
        }
    }
}

void UEnvArt_LightingController::UpdateDayNightCycle(float DeltaTime)
{
    CurrentDayTime += DeltaTime / (DayDurationMinutes * 60.0f);
    if (CurrentDayTime >= 1.0f)
    {
        CurrentDayTime -= 1.0f;
    }
    
    EEnvArt_TimeOfDay NewTimeOfDay = GetTimeOfDayFromDayTime(CurrentDayTime);
    if (NewTimeOfDay != CurrentTimeOfDay)
    {
        SetTimeOfDay(NewTimeOfDay);
    }
}

void UEnvArt_LightingController::InterpolateLightingSettings(float DeltaTime)
{
    float InterpSpeed = TransitionSpeed * DeltaTime;
    
    // Interpolate settings
    CurrentSettings.SunRotation = FMath::RInterpTo(CurrentSettings.SunRotation, TargetSettings.SunRotation, DeltaTime, TransitionSpeed);
    CurrentSettings.SunColor = FMath::CInterpTo(CurrentSettings.SunColor, TargetSettings.SunColor, DeltaTime, TransitionSpeed);
    CurrentSettings.SunIntensity = FMath::FInterpTo(CurrentSettings.SunIntensity, TargetSettings.SunIntensity, DeltaTime, TransitionSpeed);
    CurrentSettings.SkyColor = FMath::CInterpTo(CurrentSettings.SkyColor, TargetSettings.SkyColor, DeltaTime, TransitionSpeed);
    CurrentSettings.SkyIntensity = FMath::FInterpTo(CurrentSettings.SkyIntensity, TargetSettings.SkyIntensity, DeltaTime, TransitionSpeed);
    CurrentSettings.FogColor = FMath::CInterpTo(CurrentSettings.FogColor, TargetSettings.FogColor, DeltaTime, TransitionSpeed);
    CurrentSettings.FogDensity = FMath::FInterpTo(CurrentSettings.FogDensity, TargetSettings.FogDensity, DeltaTime, TransitionSpeed);
    
    // Check if transition is complete
    bool bTransitionComplete = 
        FMath::IsNearlyEqual(CurrentSettings.SunIntensity, TargetSettings.SunIntensity, 0.1f) &&
        CurrentSettings.SunColor.Equals(TargetSettings.SunColor, 0.01f) &&
        FMath::IsNearlyEqual(CurrentSettings.FogDensity, TargetSettings.FogDensity, 0.001f);
    
    if (bTransitionComplete)
    {
        CurrentSettings = TargetSettings;
        bTransitioning = false;
    }
    
    ApplyLightingSettings(CurrentSettings);
}

EEnvArt_TimeOfDay UEnvArt_LightingController::GetTimeOfDayFromDayTime(float DayTime)
{
    if (DayTime < 0.125f) return EEnvArt_TimeOfDay::Night;
    if (DayTime < 0.25f) return EEnvArt_TimeOfDay::Dawn;
    if (DayTime < 0.5f) return EEnvArt_TimeOfDay::Morning;
    if (DayTime < 0.625f) return EEnvArt_TimeOfDay::Midday;
    if (DayTime < 0.75f) return EEnvArt_TimeOfDay::Afternoon;
    if (DayTime < 0.875f) return EEnvArt_TimeOfDay::Dusk;
    return EEnvArt_TimeOfDay::Night;
}

void UEnvArt_LightingController::UpdateSunLight(const FEnvArt_LightingSettings& Settings)
{
    if (SunLight)
    {
        SunLight->SetActorRotation(Settings.SunRotation);
        
        UDirectionalLightComponent* LightComponent = SunLight->GetLightComponent();
        if (LightComponent)
        {
            LightComponent->SetLightColor(Settings.SunColor);
            LightComponent->SetIntensity(Settings.SunIntensity);
            LightComponent->SetCastShadows(true);
            LightComponent->SetCastVolumetricShadow(true);
        }
    }
}

void UEnvArt_LightingController::UpdateSkyLight(const FEnvArt_LightingSettings& Settings)
{
    if (SkyLightActor)
    {
        USkyLightComponent* SkyLightComponent = SkyLightActor->FindComponentByClass<USkyLightComponent>();
        if (SkyLightComponent)
        {
            SkyLightComponent->SetLightColor(Settings.SkyColor);
            SkyLightComponent->SetIntensity(Settings.SkyIntensity);
            SkyLightComponent->SetSourceType(SLS_CapturedScene);
            SkyLightComponent->RecaptureSky();
        }
    }
}

void UEnvArt_LightingController::UpdateFog(const FEnvArt_LightingSettings& Settings)
{
    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComponent = FogActor->FindComponentByClass<UExponentialHeightFogComponent>();
        if (FogComponent)
        {
            FogComponent->SetFogInscatteringColor(Settings.FogColor);
            FogComponent->SetFogDensity(Settings.FogDensity);
            FogComponent->SetFogHeightFalloff(Settings.FogHeightFalloff);
            FogComponent->SetStartDistance(Settings.FogStartDistance);
        }
    }
}

void UEnvArt_LightingController::UpdatePostProcess(const FEnvArt_LightingSettings& Settings)
{
    if (PostProcessVolume)
    {
        // Apply post-process settings based on time of day
        // This would typically involve setting exposure, color grading, etc.
        PostProcessVolume->Settings.bOverride_AutoExposureBias = true;
        
        // Adjust exposure based on sun intensity
        float ExposureBias = FMath::Lerp(-1.0f, 1.0f, Settings.SunIntensity / 12.0f);
        PostProcessVolume->Settings.AutoExposureBias = ExposureBias;
        
        // Color temperature adjustment
        PostProcessVolume->Settings.bOverride_WhiteTemp = true;
        float ColorTemp = FMath::Lerp(5000.0f, 7000.0f, Settings.SunIntensity / 12.0f);
        PostProcessVolume->Settings.WhiteTemp = ColorTemp;
    }
}