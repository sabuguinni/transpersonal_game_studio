#include "AtmosphericLightingManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/VolumetricCloud.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AAtmosphericLightingManager::AAtmosphericLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default settings for each time of day
    FLight_AtmosphericSettings DawnSettings;
    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    DawnSettings.SunAngle = 10.0f;
    DawnSettings.RayleighScattering = 0.0331f;
    DawnSettings.bEnableVolumetricClouds = false;
    DawnSettings.FogDensity = 0.05f;
    TimeOfDaySettings.Add(ELight_TimeOfDay::Dawn, DawnSettings);

    FLight_AtmosphericSettings MorningSettings;
    MorningSettings.SunIntensity = 4.0f;
    MorningSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
    MorningSettings.SunAngle = 30.0f;
    MorningSettings.RayleighScattering = 0.0331f;
    MorningSettings.bEnableVolumetricClouds = false;
    MorningSettings.FogDensity = 0.03f;
    TimeOfDaySettings.Add(ELight_TimeOfDay::Morning, MorningSettings);

    FLight_AtmosphericSettings MiddaySettings;
    MiddaySettings.SunIntensity = 5.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
    MiddaySettings.SunAngle = 80.0f;
    MiddaySettings.RayleighScattering = 0.0331f;
    MiddaySettings.bEnableVolumetricClouds = false;
    MiddaySettings.FogDensity = 0.02f;
    TimeOfDaySettings.Add(ELight_TimeOfDay::Midday, MiddaySettings);

    FLight_AtmosphericSettings AfternoonSettings;
    AfternoonSettings.SunIntensity = 4.5f;
    AfternoonSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
    AfternoonSettings.SunAngle = 45.0f;
    AfternoonSettings.RayleighScattering = 0.0331f;
    AfternoonSettings.bEnableVolumetricClouds = false;
    AfternoonSettings.FogDensity = 0.025f;
    TimeOfDaySettings.Add(ELight_TimeOfDay::Afternoon, AfternoonSettings);

    FLight_AtmosphericSettings DuskSettings;
    DuskSettings.SunIntensity = 2.5f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DuskSettings.SunAngle = 5.0f;
    DuskSettings.RayleighScattering = 0.0331f;
    DuskSettings.bEnableVolumetricClouds = false;
    DuskSettings.FogDensity = 0.04f;
    TimeOfDaySettings.Add(ELight_TimeOfDay::Dusk, DuskSettings);

    FLight_AtmosphericSettings NightSettings;
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.2f, 0.3f, 0.8f, 1.0f);
    NightSettings.SunAngle = -30.0f;
    NightSettings.RayleighScattering = 0.0331f;
    NightSettings.bEnableVolumetricClouds = false;
    NightSettings.FogDensity = 0.06f;
    TimeOfDaySettings.Add(ELight_TimeOfDay::Night, NightSettings);

    CurrentSettings = MiddaySettings;
}

void AAtmosphericLightingManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindLightingActors();
    ApplyCretaceousAtmosphere();
}

void AAtmosphericLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        TimeAccumulator += DeltaTime;
        
        // Convert real time to game time
        float GameTimeIncrement = (DeltaTime / 60.0f) / DayDurationMinutes * 24.0f;
        CurrentTimeOfDay += GameTimeIncrement;
        
        if (CurrentTimeOfDay >= 24.0f)
        {
            CurrentTimeOfDay -= 24.0f;
        }
        
        UpdateSunPosition();
        UpdateAtmosphericSettings();
    }
}

void AAtmosphericLightingManager::FindLightingActors()
{
    if (UWorld* World = GetWorld())
    {
        // Find DirectionalLight
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        }

        // Find SkyAtmosphere
        FoundActors.Empty();
        UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SkyAtmosphere = Cast<ASkyAtmosphere>(FoundActors[0]);
        }

        // Find VolumetricCloud
        FoundActors.Empty();
        UGameplayStatics::GetAllActorsOfClass(World, AVolumetricCloud::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            VolumetricClouds = Cast<AVolumetricCloud>(FoundActors[0]);
        }
    }
}

void AAtmosphericLightingManager::ApplyCretaceousAtmosphere()
{
    // Apply warm tropical Cretaceous atmosphere settings
    FLight_AtmosphericSettings CretaceousSettings;
    CretaceousSettings.SunIntensity = 5.0f;
    CretaceousSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
    CretaceousSettings.SunAngle = 60.0f;
    CretaceousSettings.RayleighScattering = 0.0331f;
    CretaceousSettings.MieScattering = 0.003996f;
    CretaceousSettings.bEnableVolumetricClouds = false;
    CretaceousSettings.FogDensity = 0.02f;

    SetAtmosphericSettings(CretaceousSettings);
    CurrentSettings = CretaceousSettings;
}

void AAtmosphericLightingManager::FixAtmosphericLighting()
{
    FindLightingActors();
    
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(5.0f);
        LightComp->SetLightColor(FLinearColor(1.0f, 0.95f, 0.9f, 1.0f));
        
        FRotator SunRotation = FRotator(-45.0f, 0.0f, 0.0f);
        SunLight->SetActorRotation(SunRotation);
        
        UE_LOG(LogTemp, Warning, TEXT("Fixed DirectionalLight settings"));
    }

    if (SkyAtmosphere)
    {
        if (USkyAtmosphereComponent* SkyComp = SkyAtmosphere->GetComponent())
        {
            SkyComp->RayleighScattering = FLinearColor(0.0331f, 0.0331f, 0.0331f, 1.0f);
            SkyComp->MarkRenderStateDirty();
        }
        UE_LOG(LogTemp, Warning, TEXT("Fixed SkyAtmosphere settings"));
    }

    if (VolumetricClouds)
    {
        VolumetricClouds->SetActorHiddenInGame(true);
        UE_LOG(LogTemp, Warning, TEXT("Disabled VolumetricClouds"));
    }
}

void AAtmosphericLightingManager::SetTimeOfDay(float TimeInHours)
{
    CurrentTimeOfDay = FMath::Clamp(TimeInHours, 0.0f, 24.0f);
    UpdateSunPosition();
    UpdateAtmosphericSettings();
}

void AAtmosphericLightingManager::SetAtmosphericSettings(const FLight_AtmosphericSettings& Settings)
{
    CurrentSettings = Settings;

    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(Settings.SunIntensity);
        LightComp->SetLightColor(Settings.SunColor);
    }

    if (SkyAtmosphere)
    {
        if (USkyAtmosphereComponent* SkyComp = SkyAtmosphere->GetComponent())
        {
            SkyComp->RayleighScattering = FLinearColor(Settings.RayleighScattering, Settings.RayleighScattering, Settings.RayleighScattering, 1.0f);
            SkyComp->MieScattering = FLinearColor(Settings.MieScattering, Settings.MieScattering, Settings.MieScattering, 1.0f);
            SkyComp->MarkRenderStateDirty();
        }
    }

    if (VolumetricClouds)
    {
        VolumetricClouds->SetActorHiddenInGame(!Settings.bEnableVolumetricClouds);
    }
}

ELight_TimeOfDay AAtmosphericLightingManager::GetCurrentTimeOfDayEnum() const
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

void AAtmosphericLightingManager::UpdateSunPosition()
{
    if (SunLight)
    {
        // Convert time of day to sun angle
        float SunAngle = (CurrentTimeOfDay - 6.0f) * 15.0f - 90.0f; // 6 AM = horizon, 12 PM = zenith
        FRotator SunRotation = FRotator(SunAngle, 0.0f, 0.0f);
        SunLight->SetActorRotation(SunRotation);
    }
}

void AAtmosphericLightingManager::UpdateAtmosphericSettings()
{
    ELight_TimeOfDay CurrentTimeEnum = GetCurrentTimeOfDayEnum();
    
    if (TimeOfDaySettings.Contains(CurrentTimeEnum))
    {
        FLight_AtmosphericSettings TargetSettings = TimeOfDaySettings[CurrentTimeEnum];
        
        // Smooth interpolation between time periods
        float TimeInPeriod = 0.0f;
        switch (CurrentTimeEnum)
        {
            case ELight_TimeOfDay::Dawn:
                TimeInPeriod = (CurrentTimeOfDay - 5.0f) / 2.0f;
                break;
            case ELight_TimeOfDay::Morning:
                TimeInPeriod = (CurrentTimeOfDay - 7.0f) / 4.0f;
                break;
            case ELight_TimeOfDay::Midday:
                TimeInPeriod = (CurrentTimeOfDay - 11.0f) / 4.0f;
                break;
            case ELight_TimeOfDay::Afternoon:
                TimeInPeriod = (CurrentTimeOfDay - 15.0f) / 3.0f;
                break;
            case ELight_TimeOfDay::Dusk:
                TimeInPeriod = (CurrentTimeOfDay - 18.0f) / 2.0f;
                break;
            case ELight_TimeOfDay::Night:
                TimeInPeriod = 1.0f;
                break;
        }
        
        InterpolateSettings(CurrentSettings, TargetSettings, FMath::Clamp(TimeInPeriod, 0.0f, 1.0f));
    }
}

void AAtmosphericLightingManager::InterpolateSettings(const FLight_AtmosphericSettings& From, const FLight_AtmosphericSettings& To, float Alpha)
{
    FLight_AtmosphericSettings InterpolatedSettings;
    
    InterpolatedSettings.SunIntensity = FMath::Lerp(From.SunIntensity, To.SunIntensity, Alpha);
    InterpolatedSettings.SunColor = FMath::Lerp(From.SunColor, To.SunColor, Alpha);
    InterpolatedSettings.SunAngle = FMath::Lerp(From.SunAngle, To.SunAngle, Alpha);
    InterpolatedSettings.RayleighScattering = FMath::Lerp(From.RayleighScattering, To.RayleighScattering, Alpha);
    InterpolatedSettings.MieScattering = FMath::Lerp(From.MieScattering, To.MieScattering, Alpha);
    InterpolatedSettings.FogDensity = FMath::Lerp(From.FogDensity, To.FogDensity, Alpha);
    InterpolatedSettings.bEnableVolumetricClouds = Alpha > 0.5f ? To.bEnableVolumetricClouds : From.bEnableVolumetricClouds;
    
    SetAtmosphericSettings(InterpolatedSettings);
    CurrentSettings = InterpolatedSettings;
}