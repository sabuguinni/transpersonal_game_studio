#include "Light_DayNightSystem.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ULight_DayNightSystem::ULight_DayNightSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
    
    InitializeTimeSettings();
}

void ULight_DayNightSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeLightReferences();
    SetTimeOfDay(CurrentTimeOfDay);
}

void ULight_DayNightSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bEnableDayNightCycle) return;

    // Update cycle time
    CurrentCycleTime += DeltaTime * CycleSpeed;
    if (CurrentCycleTime >= TotalCycleTime)
    {
        CurrentCycleTime = 0.0f;
    }

    // Calculate current time period based on cycle progress
    float CycleProgress = CurrentCycleTime / TotalCycleTime;
    int32 TimeIndex = FMath::FloorToInt(CycleProgress * 6.0f);
    ELight_TimeOfDay NewTimeOfDay = static_cast<ELight_TimeOfDay>(FMath::Clamp(TimeIndex, 0, 5));

    // Transition between time periods
    if (NewTimeOfDay != CurrentTimeOfDay && !bIsTransitioning)
    {
        TransitionToTime(NewTimeOfDay, 2.0f);
    }

    // Handle transitions
    if (bIsTransitioning)
    {
        TransitionTimer += DeltaTime;
        float Alpha = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);

        if (Alpha >= 1.0f)
        {
            // Transition complete
            bIsTransitioning = false;
            CurrentTimeOfDay = TransitionTarget;
            SetTimeOfDay(CurrentTimeOfDay);
        }
        else
        {
            // Interpolate between current and target settings
            const FLight_TimeSettings* TargetSettings = TimeSettings.Find(TransitionTarget);
            if (TargetSettings)
            {
                FLight_TimeSettings InterpolatedSettings;
                InterpolatedSettings.SunColor = FMath::Lerp(TransitionStartSettings.SunColor, TargetSettings->SunColor, Alpha);
                InterpolatedSettings.SunIntensity = FMath::Lerp(TransitionStartSettings.SunIntensity, TargetSettings->SunIntensity, Alpha);
                InterpolatedSettings.SunAngle = FMath::Lerp(TransitionStartSettings.SunAngle, TargetSettings->SunAngle, Alpha);
                InterpolatedSettings.SkyColor = FMath::Lerp(TransitionStartSettings.SkyColor, TargetSettings->SkyColor, Alpha);
                InterpolatedSettings.FogDensity = FMath::Lerp(TransitionStartSettings.FogDensity, TargetSettings->FogDensity, Alpha);
                InterpolatedSettings.FogColor = FMath::Lerp(TransitionStartSettings.FogColor, TargetSettings->FogColor, Alpha);
                
                UpdateLighting(InterpolatedSettings);
            }
        }
    }
}

void ULight_DayNightSystem::SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    
    const FLight_TimeSettings* Settings = TimeSettings.Find(CurrentTimeOfDay);
    if (Settings)
    {
        UpdateLighting(*Settings);
    }
}

void ULight_DayNightSystem::TransitionToTime(ELight_TimeOfDay TargetTime, float Duration)
{
    if (bIsTransitioning) return;

    TransitionTarget = TargetTime;
    TransitionDuration = Duration;
    TransitionTimer = 0.0f;
    bIsTransitioning = true;

    // Store current settings as transition start
    const FLight_TimeSettings* CurrentSettings = TimeSettings.Find(CurrentTimeOfDay);
    if (CurrentSettings)
    {
        TransitionStartSettings = *CurrentSettings;
    }
}

float ULight_DayNightSystem::GetCurrentCycleProgress() const
{
    return CurrentCycleTime / TotalCycleTime;
}

void ULight_DayNightSystem::InitializeLightReferences()
{
    FindLightActors();
}

void ULight_DayNightSystem::InitializeTimeSettings()
{
    // Dawn (5:00-7:00)
    FLight_TimeSettings DawnSettings;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f); // Warm orange-pink
    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.SunAngle = 15.0f;
    DawnSettings.SkyColor = FLinearColor(0.8f, 0.6f, 0.9f, 1.0f); // Pink-purple sky
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.FogColor = FLinearColor(0.9f, 0.7f, 0.8f, 1.0f);
    TimeSettings.Add(ELight_TimeOfDay::Dawn, DawnSettings);

    // Morning (7:00-11:00)
    FLight_TimeSettings MorningSettings;
    MorningSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f); // Golden
    MorningSettings.SunIntensity = 4.0f;
    MorningSettings.SunAngle = 35.0f;
    MorningSettings.SkyColor = FLinearColor(0.6f, 0.8f, 1.0f, 1.0f); // Light blue
    MorningSettings.FogDensity = 0.03f;
    MorningSettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    TimeSettings.Add(ELight_TimeOfDay::Morning, MorningSettings);

    // Midday (11:00-15:00)
    FLight_TimeSettings MiddaySettings;
    MiddaySettings.SunColor = FLinearColor(1.0f, 1.0f, 0.95f, 1.0f); // Bright white-yellow
    MiddaySettings.SunIntensity = 6.0f;
    MiddaySettings.SunAngle = 75.0f;
    MiddaySettings.SkyColor = FLinearColor(0.4f, 0.7f, 1.0f, 1.0f); // Clear blue
    MiddaySettings.FogDensity = 0.01f;
    MiddaySettings.FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    TimeSettings.Add(ELight_TimeOfDay::Midday, MiddaySettings);

    // Afternoon (15:00-18:00)
    FLight_TimeSettings AfternoonSettings;
    AfternoonSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f); // Warm amber
    AfternoonSettings.SunIntensity = 4.5f;
    AfternoonSettings.SunAngle = 45.0f;
    AfternoonSettings.SkyColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f); // Warm blue
    AfternoonSettings.FogDensity = 0.02f;
    AfternoonSettings.FogColor = FLinearColor(0.9f, 0.8f, 0.7f, 1.0f);
    TimeSettings.Add(ELight_TimeOfDay::Afternoon, AfternoonSettings);

    // Dusk (18:00-20:00)
    FLight_TimeSettings DuskSettings;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f); // Deep orange-red
    DuskSettings.SunIntensity = 2.5f;
    DuskSettings.SunAngle = 10.0f;
    DuskSettings.SkyColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f); // Orange sky
    DuskSettings.FogDensity = 0.04f;
    DuskSettings.FogColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    TimeSettings.Add(ELight_TimeOfDay::Dusk, DuskSettings);

    // Night (20:00-5:00)
    FLight_TimeSettings NightSettings;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f); // Cool blue moonlight
    NightSettings.SunIntensity = 0.5f;
    NightSettings.SunAngle = -30.0f; // Below horizon
    NightSettings.SkyColor = FLinearColor(0.1f, 0.1f, 0.3f, 1.0f); // Dark blue
    NightSettings.FogDensity = 0.06f;
    NightSettings.FogColor = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);
    TimeSettings.Add(ELight_TimeOfDay::Night, NightSettings);
}

void ULight_DayNightSystem::UpdateLighting(const FLight_TimeSettings& Settings)
{
    // Update Directional Light (Sun)
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetLightColor(Settings.SunColor);
        LightComp->SetIntensity(Settings.SunIntensity);
        
        // Set sun rotation based on angle
        FRotator SunRotation = FRotator(-Settings.SunAngle, 0.0f, 0.0f);
        SunLight->SetActorRotation(SunRotation);
        
        // Enable/disable volumetric scattering based on intensity
        LightComp->SetCastVolumetricShadow(Settings.SunIntensity > 1.0f);
        LightComp->SetVolumetricScatteringIntensity(FMath::Clamp(Settings.SunIntensity * 0.2f, 0.0f, 1.0f));
    }

    // Update Sky Light
    if (SkyLight && SkyLight->GetLightComponent())
    {
        USkyLightComponent* SkyComp = SkyLight->GetLightComponent();
        SkyComp->SetLightColor(Settings.SkyColor);
        SkyComp->SetIntensity(FMath::Clamp(Settings.SunIntensity * 0.3f, 0.1f, 2.0f));
        SkyComp->RecaptureSky();
    }

    // Update Height Fog
    if (HeightFog && HeightFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
        FogComp->SetFogDensity(Settings.FogDensity);
        FogComp->SetFogInscatteringColor(Settings.FogColor);
        FogComp->SetFogMaxOpacity(FMath::Clamp(Settings.FogDensity * 20.0f, 0.3f, 1.0f));
    }
}

void ULight_DayNightSystem::FindLightActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find Directional Light
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }

    // Find Sky Light
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyLight = Cast<ASkyLight>(FoundActors[0]);
    }

    // Find Height Fog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }
}