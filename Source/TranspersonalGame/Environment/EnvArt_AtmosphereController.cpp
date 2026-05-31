#include "EnvArt_AtmosphereController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/Engine.h"

AEnvArt_AtmosphereController::AEnvArt_AtmosphereController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    
    // Set default root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AEnvArt_AtmosphereController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTimeOfDaySettings();
    FindSceneLightingActors();
    UpdateLighting();
}

void AEnvArt_AtmosphereController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableAutoCycle && DayCycleDuration > 0.0f)
    {
        CurrentCycleTime += DeltaTime;
        
        // Calculate progress through the day cycle
        float CycleProgress = FMath::Fmod(CurrentCycleTime, DayCycleDuration) / DayCycleDuration;
        
        // Determine current time of day based on cycle progress
        if (CycleProgress < 0.1f)
            CurrentTimeOfDay = EEnvArt_TimeOfDay::Night;
        else if (CycleProgress < 0.2f)
            CurrentTimeOfDay = EEnvArt_TimeOfDay::Dawn;
        else if (CycleProgress < 0.4f)
            CurrentTimeOfDay = EEnvArt_TimeOfDay::Morning;
        else if (CycleProgress < 0.6f)
            CurrentTimeOfDay = EEnvArt_TimeOfDay::Midday;
        else if (CycleProgress < 0.8f)
            CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;
        else if (CycleProgress < 0.9f)
            CurrentTimeOfDay = EEnvArt_TimeOfDay::Dusk;
        else
            CurrentTimeOfDay = EEnvArt_TimeOfDay::Night;
        
        UpdateLighting();
    }
}

void AEnvArt_AtmosphereController::InitializeTimeOfDaySettings()
{
    // Dawn settings
    FEnvArt_LightingSettings DawnSettings;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    DawnSettings.SunIntensity = 1.5f;
    DawnSettings.SunRotation = FRotator(-10.0f, 75.0f, 0.0f);
    DawnSettings.SkyColor = FLinearColor(0.8f, 0.6f, 0.9f, 1.0f);
    DawnSettings.SkyIntensity = 0.5f;
    DawnSettings.FogColor = FLinearColor(0.9f, 0.8f, 0.7f, 1.0f);
    DawnSettings.FogDensity = 0.04f;
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Dawn, DawnSettings);
    
    // Morning settings - Golden Hour
    FEnvArt_LightingSettings MorningSettings;
    MorningSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.6f, 1.0f);
    MorningSettings.SunIntensity = 2.5f;
    MorningSettings.SunRotation = FRotator(-25.0f, 45.0f, 0.0f);
    MorningSettings.SkyColor = FLinearColor(0.6f, 0.8f, 1.0f, 1.0f);
    MorningSettings.SkyIntensity = 0.8f;
    MorningSettings.FogColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    MorningSettings.FogDensity = 0.025f;
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Morning, MorningSettings);
    
    // Midday settings
    FEnvArt_LightingSettings MiddaySettings;
    MiddaySettings.SunColor = FLinearColor::White;
    MiddaySettings.SunIntensity = 4.0f;
    MiddaySettings.SunRotation = FRotator(-80.0f, 0.0f, 0.0f);
    MiddaySettings.SkyColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);
    MiddaySettings.SkyIntensity = 1.2f;
    MiddaySettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    MiddaySettings.FogDensity = 0.015f;
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Midday, MiddaySettings);
    
    // Afternoon settings
    FEnvArt_LightingSettings AfternoonSettings;
    AfternoonSettings.SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
    AfternoonSettings.SunIntensity = 3.0f;
    AfternoonSettings.SunRotation = FRotator(-45.0f, -30.0f, 0.0f);
    AfternoonSettings.SkyColor = FLinearColor(0.4f, 0.7f, 1.0f, 1.0f);
    AfternoonSettings.SkyIntensity = 1.0f;
    AfternoonSettings.FogColor = FLinearColor(0.9f, 0.9f, 0.8f, 1.0f);
    AfternoonSettings.FogDensity = 0.02f;
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Afternoon, AfternoonSettings);
    
    // Dusk settings - Golden Hour
    FEnvArt_LightingSettings DuskSettings;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DuskSettings.SunIntensity = 2.0f;
    DuskSettings.SunRotation = FRotator(-5.0f, -75.0f, 0.0f);
    DuskSettings.SkyColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DuskSettings.SkyIntensity = 0.6f;
    DuskSettings.FogColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DuskSettings.FogDensity = 0.035f;
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Dusk, DuskSettings);
    
    // Night settings
    FEnvArt_LightingSettings NightSettings;
    NightSettings.SunColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunRotation = FRotator(30.0f, 180.0f, 0.0f);
    NightSettings.SkyColor = FLinearColor(0.1f, 0.1f, 0.3f, 1.0f);
    NightSettings.SkyIntensity = 0.2f;
    NightSettings.FogColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
    NightSettings.FogDensity = 0.05f;
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Night, NightSettings);
}

void AEnvArt_AtmosphereController::UpdateLighting()
{
    FEnvArt_LightingSettings CurrentSettings = GetCurrentLightingSettings();
    
    // Update sun light
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* DirLight = SunLight->GetLightComponent();
        DirLight->SetLightColor(CurrentSettings.SunColor);
        DirLight->SetIntensity(CurrentSettings.SunIntensity * AtmosphereIntensity);
        SunLight->SetActorRotation(CurrentSettings.SunRotation);
        
        // Enable/disable volumetric scattering for god rays
        DirLight->SetVolumetricScatteringIntensity(bEnableGodRays ? 1.0f : 0.0f);
    }
    
    // Update sky light
    if (SkyLight && SkyLight->GetLightComponent())
    {
        USkyLightComponent* SkyComp = SkyLight->GetLightComponent();
        SkyComp->SetLightColor(CurrentSettings.SkyColor);
        SkyComp->SetIntensity(CurrentSettings.SkyIntensity * AtmosphereIntensity);
        SkyComp->RecaptureSky();
    }
    
    // Update fog
    if (HeightFog && HeightFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
        FogComp->SetFogInscatteringColor(CurrentSettings.FogColor);
        FogComp->SetFogDensity(CurrentSettings.FogDensity * (bEnableVolumetricFog ? 1.0f : 0.5f));
        FogComp->SetFogHeightFalloff(CurrentSettings.FogHeightFalloff);
        FogComp->SetVolumetricFog(bEnableVolumetricFog);
    }
}

void AEnvArt_AtmosphereController::FindSceneLightingActors()
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
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyLight = Cast<ASkyLight>(FoundActors[0]);
    }
    
    // Find exponential height fog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }
}

FEnvArt_LightingSettings AEnvArt_AtmosphereController::GetCurrentLightingSettings()
{
    if (TimeOfDaySettings.Contains(CurrentTimeOfDay))
    {
        return TimeOfDaySettings[CurrentTimeOfDay];
    }
    
    // Return default morning settings if not found
    FEnvArt_LightingSettings DefaultSettings;
    return DefaultSettings;
}

FEnvArt_LightingSettings AEnvArt_AtmosphereController::InterpolateLightingSettings(const FEnvArt_LightingSettings& A, const FEnvArt_LightingSettings& B, float Alpha)
{
    FEnvArt_LightingSettings Result;
    Result.SunColor = FLinearColor::LerpUsingHSV(A.SunColor, B.SunColor, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunRotation = FMath::Lerp(A.SunRotation, B.SunRotation, Alpha);
    Result.SkyColor = FLinearColor::LerpUsingHSV(A.SkyColor, B.SkyColor, Alpha);
    Result.SkyIntensity = FMath::Lerp(A.SkyIntensity, B.SkyIntensity, Alpha);
    Result.FogColor = FLinearColor::LerpUsingHSV(A.FogColor, B.FogColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    return Result;
}

void AEnvArt_AtmosphereController::SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    UpdateLighting();
}

void AEnvArt_AtmosphereController::SetCycleSpeed(float NewDuration)
{
    DayCycleDuration = FMath::Max(NewDuration, 60.0f); // Minimum 1 minute cycle
}

void AEnvArt_AtmosphereController::EnableGoldenHourLighting()
{
    SetTimeOfDay(EEnvArt_TimeOfDay::Morning);
    bEnableGodRays = true;
    bEnableVolumetricFog = true;
    AtmosphereIntensity = 1.2f;
    UpdateLighting();
}

void AEnvArt_AtmosphereController::CreateAtmosphericParticles()
{
    // This would spawn particle systems for dust, pollen, etc.
    // Implementation would require Niagara particle systems
    UE_LOG(LogTemp, Warning, TEXT("AtmosphereController: CreateAtmosphericParticles called - requires Niagara implementation"));
}

void AEnvArt_AtmosphereController::RefreshLightingReferences()
{
    FindSceneLightingActors();
    UpdateLighting();
}