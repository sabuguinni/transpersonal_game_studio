#include "Light_AtmosphericManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ALight_AtmosphericManager::ALight_AtmosphericManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;
    TimeOfDaySpeed = 1.0f;
    bAutoAdvanceTime = false;
    CurrentTimeHours = 14.0f; // 2 PM
    bUseCretaceousAtmosphere = true;
    AtmosphericDensityMultiplier = 1.2f;
    CO2LevelMultiplier = 4.0f;

    // Initialize time presets
    InitializeTimePresets();
}

void ALight_AtmosphericManager::BeginPlay()
{
    Super::BeginPlay();

    // Find existing lighting actors in the world
    FindLightingActors();

    // Apply Cretaceous atmosphere if enabled
    if (bUseCretaceousAtmosphere)
    {
        InitializeCretaceousAtmosphere();
    }

    // Enable Lumen global illumination
    EnableLumenGlobalIllumination();
}

void ALight_AtmosphericManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bAutoAdvanceTime)
    {
        CurrentTimeHours += (TimeOfDaySpeed * DeltaTime) / 3600.0f; // Convert seconds to hours
        
        if (CurrentTimeHours >= 24.0f)
        {
            CurrentTimeHours = 0.0f;
        }

        // Update time of day enum based on hours
        if (CurrentTimeHours >= 5.0f && CurrentTimeHours < 8.0f)
        {
            CurrentTimeOfDay = ELight_TimeOfDay::Dawn;
        }
        else if (CurrentTimeHours >= 8.0f && CurrentTimeHours < 12.0f)
        {
            CurrentTimeOfDay = ELight_TimeOfDay::Morning;
        }
        else if (CurrentTimeHours >= 12.0f && CurrentTimeHours < 15.0f)
        {
            CurrentTimeOfDay = ELight_TimeOfDay::Noon;
        }
        else if (CurrentTimeHours >= 15.0f && CurrentTimeHours < 18.0f)
        {
            CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;
        }
        else if (CurrentTimeHours >= 18.0f && CurrentTimeHours < 21.0f)
        {
            CurrentTimeOfDay = ELight_TimeOfDay::Dusk;
        }
        else
        {
            CurrentTimeOfDay = ELight_TimeOfDay::Night;
        }

        // Update sun position
        UpdateSunPosition(CurrentTimeHours);

        // Apply current preset with interpolation
        FLight_AtmosphericPreset CurrentPreset = GetCurrentPreset();
        ApplyAtmosphericPreset(CurrentPreset);
    }
}

void ALight_AtmosphericManager::SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;

    // Convert enum to hours
    switch (NewTimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:
            CurrentTimeHours = 6.0f;
            break;
        case ELight_TimeOfDay::Morning:
            CurrentTimeHours = 9.0f;
            break;
        case ELight_TimeOfDay::Noon:
            CurrentTimeHours = 12.0f;
            break;
        case ELight_TimeOfDay::Afternoon:
            CurrentTimeHours = 15.0f;
            break;
        case ELight_TimeOfDay::Dusk:
            CurrentTimeHours = 19.0f;
            break;
        case ELight_TimeOfDay::Night:
            CurrentTimeHours = 23.0f;
            break;
    }

    UpdateSunPosition(CurrentTimeHours);
    
    if (TimePresets.Contains(NewTimeOfDay))
    {
        ApplyAtmosphericPreset(TimePresets[NewTimeOfDay]);
    }
}

void ALight_AtmosphericManager::SetTimeOfDayByHours(float Hours)
{
    CurrentTimeHours = FMath::Clamp(Hours, 0.0f, 24.0f);
    UpdateSunPosition(CurrentTimeHours);
    
    FLight_AtmosphericPreset CurrentPreset = GetCurrentPreset();
    ApplyAtmosphericPreset(CurrentPreset);
}

void ALight_AtmosphericManager::ApplyAtmosphericPreset(const FLight_AtmosphericPreset& Preset)
{
    // Apply sun light settings
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* SunComponent = SunLight->GetLightComponent();
        SunComponent->SetIntensity(Preset.SunIntensity);
        SunComponent->SetLightColor(Preset.SunColor);
        SunComponent->SetTemperature(Preset.SunTemperature);
        SunLight->SetActorRotation(Preset.SunRotation);
    }

    // Apply sky light settings
    if (SkyLight && SkyLight->GetLightComponent())
    {
        USkyLightComponent* SkyComponent = SkyLight->GetLightComponent();
        SkyComponent->SetIntensity(Preset.SkyLightIntensity);
        SkyComponent->SetLightColor(Preset.SkyLightColor);
        SkyComponent->RecaptureSky();
    }

    // Apply atmosphere settings
    if (Atmosphere)
    {
        USkyAtmosphereComponent* AtmosphereComponent = Atmosphere->GetComponent();
        if (AtmosphereComponent)
        {
            AtmosphereComponent->SetAerosolDensity(Preset.AerosolDensity * AtmosphericDensityMultiplier);
            AtmosphereComponent->SetRayleighScattering(Preset.RayleighScattering);
        }
    }

    // Apply fog settings
    if (VolumetricFog)
    {
        UExponentialHeightFogComponent* FogComponent = VolumetricFog->GetComponent();
        if (FogComponent)
        {
            FogComponent->SetFogDensity(Preset.FogDensity);
            FogComponent->SetFogInscatteringColor(Preset.FogColor);
        }
    }

    // Apply post process settings
    if (PostProcessVolume)
    {
        UPostProcessComponent* PPComponent = PostProcessVolume->GetComponent();
        if (PPComponent)
        {
            PPComponent->Settings.bOverride_WhiteTemp = true;
            PPComponent->Settings.WhiteTemp = Preset.WhiteTemperature;
            PPComponent->Settings.bOverride_BloomIntensity = true;
            PPComponent->Settings.BloomIntensity = Preset.BloomIntensity;
        }
    }
}

void ALight_AtmosphericManager::InitializeCretaceousAtmosphere()
{
    // Apply Cretaceous period modifications to all presets
    for (auto& PresetPair : TimePresets)
    {
        ApplyCretaceousModifications(PresetPair.Value);
    }

    UE_LOG(LogTemp, Log, TEXT("Cretaceous atmospheric conditions initialized"));
}

void ALight_AtmosphericManager::UpdateSunPosition(float TimeHours)
{
    if (!SunLight)
        return;

    // Calculate sun angle based on time (simplified solar path)
    float SunAngle = (TimeHours - 12.0f) * 15.0f; // 15 degrees per hour from noon
    float SunElevation = FMath::Sin(FMath::DegreesToRadians((TimeHours - 6.0f) * 15.0f)) * 90.0f;
    
    // Clamp elevation to prevent sun going underground
    SunElevation = FMath::Clamp(SunElevation, -10.0f, 90.0f);

    FRotator NewRotation = FRotator(-SunElevation, SunAngle, 0.0f);
    SunLight->SetActorRotation(NewRotation);
}

void ALight_AtmosphericManager::EnableLumenGlobalIllumination()
{
    if (UWorld* World = GetWorld())
    {
        // Enable Lumen via console commands
        if (GEngine)
        {
            GEngine->Exec(World, TEXT("r.GlobalIlluminationPlugin 1"));
            GEngine->Exec(World, TEXT("r.Lumen.GlobalIllumination 1"));
            GEngine->Exec(World, TEXT("r.Lumen.Reflections 1"));
            GEngine->Exec(World, TEXT("r.Lumen.ScreenProbeGather 1"));
            
            UE_LOG(LogTemp, Log, TEXT("Lumen Global Illumination enabled"));
        }
    }
}

FLight_AtmosphericPreset ALight_AtmosphericManager::GetCurrentPreset() const
{
    if (TimePresets.Contains(CurrentTimeOfDay))
    {
        return TimePresets[CurrentTimeOfDay];
    }
    
    // Return default afternoon preset
    FLight_AtmosphericPreset DefaultPreset;
    return DefaultPreset;
}

void ALight_AtmosphericManager::SetupCretaceousLighting()
{
    FindLightingActors();
    InitializeCretaceousAtmosphere();
    SetTimeOfDay(ELight_TimeOfDay::Afternoon);
    EnableLumenGlobalIllumination();
    
    UE_LOG(LogTemp, Log, TEXT("Cretaceous lighting setup complete"));
}

void ALight_AtmosphericManager::FindLightingActors()
{
    if (UWorld* World = GetWorld())
    {
        // Find directional light (sun)
        for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
        {
            SunLight = *ActorItr;
            break; // Use first found
        }

        // Find sky light
        for (TActorIterator<ASkyLight> ActorItr(World); ActorItr; ++ActorItr)
        {
            SkyLight = *ActorItr;
            break;
        }

        // Find volumetric fog
        for (TActorIterator<AExponentialHeightFog> ActorItr(World); ActorItr; ++ActorItr)
        {
            VolumetricFog = *ActorItr;
            break;
        }

        // Find sky atmosphere
        for (TActorIterator<ASkyAtmosphere> ActorItr(World); ActorItr; ++ActorItr)
        {
            Atmosphere = *ActorItr;
            break;
        }

        // Find post process volume
        for (TActorIterator<APostProcessVolume> ActorItr(World); ActorItr; ++ActorItr)
        {
            PostProcessVolume = *ActorItr;
            break;
        }

        bLightingActorsFound = true;
        UE_LOG(LogTemp, Log, TEXT("Lighting actors found and linked"));
    }
}

void ALight_AtmosphericManager::InitializeTimePresets()
{
    // Dawn preset (5-8 AM)
    FLight_AtmosphericPreset DawnPreset;
    DawnPreset.SunIntensity = 3.0f;
    DawnPreset.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    DawnPreset.SunTemperature = 3200.0f;
    DawnPreset.SunRotation = FRotator(-10.0f, 90.0f, 0.0f);
    DawnPreset.SkyLightIntensity = 0.5f;
    DawnPreset.SkyLightColor = FLinearColor(0.6f, 0.7f, 1.0f, 1.0f);
    DawnPreset.FogDensity = 0.05f;
    DawnPreset.FogColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnPreset.WhiteTemperature = 3200.0f;
    TimePresets.Add(ELight_TimeOfDay::Dawn, DawnPreset);

    // Morning preset (8-12 AM)
    FLight_AtmosphericPreset MorningPreset;
    MorningPreset.SunIntensity = 6.0f;
    MorningPreset.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    MorningPreset.SunTemperature = 4500.0f;
    MorningPreset.SunRotation = FRotator(-30.0f, 110.0f, 0.0f);
    MorningPreset.SkyLightIntensity = 0.8f;
    MorningPreset.FogDensity = 0.03f;
    TimePresets.Add(ELight_TimeOfDay::Morning, MorningPreset);

    // Noon preset (12-3 PM)
    FLight_AtmosphericPreset NoonPreset;
    NoonPreset.SunIntensity = 10.0f;
    NoonPreset.SunColor = FLinearColor(1.0f, 1.0f, 0.9f, 1.0f);
    NoonPreset.SunTemperature = 6500.0f;
    NoonPreset.SunRotation = FRotator(-80.0f, 180.0f, 0.0f);
    NoonPreset.SkyLightIntensity = 1.5f;
    NoonPreset.FogDensity = 0.01f;
    TimePresets.Add(ELight_TimeOfDay::Noon, NoonPreset);

    // Afternoon preset (3-6 PM) - Default Cretaceous
    FLight_AtmosphericPreset AfternoonPreset;
    AfternoonPreset.SunIntensity = 8.5f;
    AfternoonPreset.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    AfternoonPreset.SunTemperature = 5600.0f;
    AfternoonPreset.SunRotation = FRotator(-45.0f, 135.0f, 0.0f);
    AfternoonPreset.SkyLightIntensity = 1.2f;
    AfternoonPreset.FogDensity = 0.02f;
    TimePresets.Add(ELight_TimeOfDay::Afternoon, AfternoonPreset);

    // Dusk preset (6-9 PM)
    FLight_AtmosphericPreset DuskPreset;
    DuskPreset.SunIntensity = 4.0f;
    DuskPreset.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DuskPreset.SunTemperature = 2800.0f;
    DuskPreset.SunRotation = FRotator(-5.0f, 270.0f, 0.0f);
    DuskPreset.SkyLightIntensity = 0.6f;
    DuskPreset.FogDensity = 0.04f;
    DuskPreset.FogColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DuskPreset.WhiteTemperature = 2800.0f;
    TimePresets.Add(ELight_TimeOfDay::Dusk, DuskPreset);

    // Night preset (9 PM - 5 AM)
    FLight_AtmosphericPreset NightPreset;
    NightPreset.SunIntensity = 0.1f;
    NightPreset.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightPreset.SunTemperature = 8000.0f;
    NightPreset.SkyLightIntensity = 0.2f;
    NightPreset.SkyLightColor = FLinearColor(0.4f, 0.5f, 1.0f, 1.0f);
    NightPreset.FogDensity = 0.03f;
    NightPreset.FogColor = FLinearColor(0.4f, 0.5f, 0.8f, 1.0f);
    NightPreset.WhiteTemperature = 8000.0f;
    TimePresets.Add(ELight_TimeOfDay::Night, NightPreset);
}

void ALight_AtmosphericManager::InterpolatePresets(const FLight_AtmosphericPreset& PresetA, const FLight_AtmosphericPreset& PresetB, float Alpha, FLight_AtmosphericPreset& OutPreset)
{
    OutPreset.SunIntensity = FMath::Lerp(PresetA.SunIntensity, PresetB.SunIntensity, Alpha);
    OutPreset.SunColor = FMath::Lerp(PresetA.SunColor, PresetB.SunColor, Alpha);
    OutPreset.SunTemperature = FMath::Lerp(PresetA.SunTemperature, PresetB.SunTemperature, Alpha);
    OutPreset.SkyLightIntensity = FMath::Lerp(PresetA.SkyLightIntensity, PresetB.SkyLightIntensity, Alpha);
    OutPreset.SkyLightColor = FMath::Lerp(PresetA.SkyLightColor, PresetB.SkyLightColor, Alpha);
    OutPreset.FogDensity = FMath::Lerp(PresetA.FogDensity, PresetB.FogDensity, Alpha);
    OutPreset.FogColor = FMath::Lerp(PresetA.FogColor, PresetB.FogColor, Alpha);
    OutPreset.WhiteTemperature = FMath::Lerp(PresetA.WhiteTemperature, PresetB.WhiteTemperature, Alpha);
    OutPreset.BloomIntensity = FMath::Lerp(PresetA.BloomIntensity, PresetB.BloomIntensity, Alpha);
}

void ALight_AtmosphericManager::ApplyCretaceousModifications(FLight_AtmosphericPreset& Preset)
{
    // Enhance atmospheric density for Cretaceous period
    Preset.AerosolDensity *= AtmosphericDensityMultiplier;
    
    // Adjust scattering for higher CO2 levels
    Preset.RayleighScattering *= (1.0f + (CO2LevelMultiplier - 1.0f) * 0.1f);
    
    // Slightly warmer color temperature due to greenhouse effect
    Preset.SunTemperature -= 200.0f;
    Preset.WhiteTemperature -= 200.0f;
    
    // Enhanced volumetric effects
    Preset.FogDensity *= 1.1f;
}