#include "AtmosphericSystem.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AAtmosphericSystem::AAtmosphericSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for atmospheric updates

    // Create atmospheric components
    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    RootComponent = SkyAtmosphere;
    
    VolumetricClouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricClouds"));
    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));

    // Initialize default parameters
    AtmosphereSettings = FAtmosphericParameters();
    CloudSettings = FCloudParameters();
    FogSettings = FFogParameters();

    // Set prehistoric environment defaults
    Humidity = 0.7f; // Higher humidity in prehistoric times
    AirPurity = 0.95f; // Cleaner air
    AtmosphericPressure = 1.3f; // Higher atmospheric pressure
    Temperature = 28.0f; // Warmer climate

    // Initialize time tracking
    TimeAccumulator = 0.0f;
    LastUpdateTime = 0.0f;
}

void AAtmosphericSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("AtmosphericSystem: Initializing prehistoric atmosphere"));
    
    // Apply initial settings
    ApplyAtmosphericSettings();
    ApplyCloudSettings();
    ApplyFogSettings();
    
    // Set up automatic variation timers
    if (GetWorld())
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AAtmosphericSystem::UpdateEnvironmentalConditions, 30.0f, true);
    }
}

void AAtmosphericSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    TimeAccumulator += DeltaTime;
    
    // Update dynamic effects
    UpdateDynamicEffects(DeltaTime);
    
    // Periodic full updates
    if (TimeAccumulator - LastUpdateTime >= 1.0f)
    {
        UpdateAtmosphericConditions();
        LastUpdateTime = TimeAccumulator;
    }
}

void AAtmosphericSystem::ApplyAtmosphericSettings()
{
    if (!SkyAtmosphere)
    {
        UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: SkyAtmosphere component not found"));
        return;
    }

    // Apply Rayleigh scattering (molecules - blue sky)
    SkyAtmosphere->SetRayleighScattering(AtmosphereSettings.RayleighScattering);
    SkyAtmosphere->SetRayleighScatteringScale(AtmosphereSettings.RayleighScatteringScale);
    SkyAtmosphere->SetRayleighExponentialDistribution(AtmosphereSettings.RayleighExponentialDistribution);

    // Apply Mie scattering (aerosols - haze)
    SkyAtmosphere->SetMieScattering(AtmosphereSettings.MieScattering);
    SkyAtmosphere->SetMieScatteringScale(AtmosphereSettings.MieScatteringScale);
    SkyAtmosphere->SetMieAbsorption(AtmosphereSettings.MieAbsorption);
    SkyAtmosphere->SetMieAbsorptionScale(AtmosphereSettings.MieAbsorptionScale);
    SkyAtmosphere->SetMieAnisotropy(AtmosphereSettings.MieAnisotropy);
    SkyAtmosphere->SetMieExponentialDistribution(AtmosphereSettings.MieExponentialDistribution);

    // Apply absorption (ozone layer)
    SkyAtmosphere->SetOtherAbsorption(AtmosphereSettings.AbsorptionExtinction);
    SkyAtmosphere->SetOtherAbsorptionScale(AtmosphereSettings.AbsorptionScale);

    // Apply planet parameters
    SkyAtmosphere->SetBottomRadius(AtmosphereSettings.GroundRadius);
    SkyAtmosphere->SetAtmosphereHeight(AtmosphereSettings.AtmosphereHeight);
    SkyAtmosphere->SetGroundAlbedo(AtmosphereSettings.GroundAlbedo);

    // Apply quality settings
    SkyAtmosphere->SetTraceSampleCountScale(AtmosphereSettings.TraceSampleCountScale);
    SkyAtmosphere->SetMultiScatteringLUTEnabled(AtmosphereSettings.bMultiScattering);
    SkyAtmosphere->SetAerialPespectiveViewDistanceScale(AtmosphereSettings.AerialPerspectiveDistanceScale);

    UE_LOG(LogTemp, Log, TEXT("AtmosphericSystem: Applied atmospheric settings"));
}

void AAtmosphericSystem::ApplyCloudSettings()
{
    if (!VolumetricClouds)
    {
        UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: VolumetricClouds component not found"));
        return;
    }

    // Apply cloud coverage and density
    // Note: These would typically be set through a cloud material
    // For now, we'll set component-level properties
    
    VolumetricClouds->SetLayerBottomAltitude(CloudSettings.LayerBottomAltitude);
    VolumetricClouds->SetLayerHeight(CloudSettings.LayerHeight);
    
    // Apply tracing quality
    VolumetricClouds->SetViewSampleCountScale(CloudSettings.TraceSampleCountScale);
    VolumetricClouds->SetShadowViewSampleCountScale(CloudSettings.ShadowSampleCountScale);
    VolumetricClouds->SetReflectionSampleCountScale(CloudSettings.ReflectionSampleCountScale);

    UE_LOG(LogTemp, Log, TEXT("AtmosphericSystem: Applied cloud settings"));
}

void AAtmosphericSystem::ApplyFogSettings()
{
    if (!HeightFog)
    {
        UE_LOG(LogTemp, Warning, TEXT("AtmosphericSystem: HeightFog component not found"));
        return;
    }

    // Apply primary fog settings
    HeightFog->SetFogDensity(FogSettings.FogDensity);
    HeightFog->SetFogHeightFalloff(FogSettings.FogHeightFalloff);
    HeightFog->SetFogMaxOpacity(FogSettings.FogMaxOpacity);
    HeightFog->SetFogInscatteringColor(FogSettings.FogInscatteringColor);

    // Apply secondary fog layer
    HeightFog->SetSecondFogData(FogSettings.SecondFogDensity, FogSettings.SecondFogHeightFalloff, FogSettings.SecondFogHeightOffset);

    // Apply directional inscattering
    HeightFog->SetDirectionalInscatteringColor(FogSettings.DirectionalInscatteringColor);
    HeightFog->SetDirectionalInscatteringExponent(FogSettings.DirectionalInscatteringExponent);
    HeightFog->SetDirectionalInscatteringStartDistance(FogSettings.DirectionalInscatteringStartDistance);

    // Apply distance controls
    HeightFog->SetStartDistance(FogSettings.StartDistance);
    HeightFog->SetFogCutoffDistance(FogSettings.FogCutoffDistance);

    // Apply volumetric fog settings
    HeightFog->SetVolumetricFog(FogSettings.bVolumetricFog);
    if (FogSettings.bVolumetricFog)
    {
        HeightFog->SetVolumetricFogScatteringDistribution(FogSettings.VolumetricFogScatteringDistribution);
        HeightFog->SetVolumetricFogAlbedo(FogSettings.VolumetricFogAlbedo);
        HeightFog->SetVolumetricFogEmissive(FogSettings.VolumetricFogEmissive);
        HeightFog->SetVolumetricFogExtinctionScale(FogSettings.VolumetricFogExtinctionScale);
    }

    UE_LOG(LogTemp, Log, TEXT("AtmosphericSystem: Applied fog settings"));
}

void AAtmosphericSystem::UpdateDynamicEffects(float DeltaTime)
{
    // Update cloud movement
    if (VolumetricClouds)
    {
        // Animate cloud offset based on wind speed
        FVector2D CloudOffset = CloudSettings.CloudSpeed * TimeAccumulator * 0.01f; // Scale for reasonable movement
        // Note: Cloud offset would typically be applied through material parameters
    }

    // Update atmospheric variations based on curves
    if (HumidityCurve)
    {
        float CurveTime = FMath::Fmod(TimeAccumulator, 3600.0f) / 3600.0f; // Hourly cycle
        float CurveValue = HumidityCurve->GetFloatValue(CurveTime);
        Humidity = FMath::Lerp(0.4f, 0.9f, CurveValue);
    }

    if (CloudCoverageCurve)
    {
        float CurveTime = FMath::Fmod(TimeAccumulator, 7200.0f) / 7200.0f; // 2-hour cycle
        float CurveValue = CloudCoverageCurve->GetFloatValue(CurveTime);
        CloudSettings.CloudCoverage = FMath::Lerp(0.2f, 0.8f, CurveValue);
    }

    if (FogDensityCurve)
    {
        float CurveTime = FMath::Fmod(TimeAccumulator, 1800.0f) / 1800.0f; // 30-minute cycle
        float CurveValue = FogDensityCurve->GetFloatValue(CurveTime);
        FogSettings.FogDensity = BaseFogDensity * FMath::Lerp(0.5f, 2.0f, CurveValue);
    }
}

void AAtmosphericSystem::UpdateAtmosphericConditions()
{
    // Adjust atmospheric parameters based on environmental conditions
    
    // Humidity affects Mie scattering (more particles in humid air)
    float HumidityEffect = FMath::Lerp(0.8f, 1.4f, Humidity);
    AtmosphereSettings.MieScatteringScale = BaseMieScattering * HumidityEffect;
    
    // Air purity affects both scattering types
    float PurityEffect = FMath::Lerp(1.5f, 0.8f, AirPurity); // Less pure = more scattering
    AtmosphereSettings.RayleighScatteringScale = BaseRayleighScattering * PurityEffect;
    
    // Atmospheric pressure affects density distribution
    AtmosphereSettings.RayleighExponentialDistribution = BaseRayleighDistribution * (2.0f - AtmosphericPressure);
    AtmosphereSettings.MieExponentialDistribution = BaseMieDistribution * (2.0f - AtmosphericPressure);
    
    // Temperature affects fog formation
    float TemperatureEffect = FMath::GetMappedRangeValueClamped(FVector2D(15.0f, 35.0f), FVector2D(1.5f, 0.7f), Temperature);
    FogSettings.FogDensity = BaseFogDensity * TemperatureEffect * Humidity;
    
    // Apply updated settings
    ApplyAtmosphericSettings();
    ApplyFogSettings();
}

void AAtmosphericSystem::UpdateEnvironmentalConditions()
{
    // Simulate natural variations in environmental conditions
    
    // Add subtle random variations to humidity
    float HumidityVariation = FMath::RandRange(-0.05f, 0.05f);
    Humidity = FMath::Clamp(Humidity + HumidityVariation, 0.3f, 0.95f);
    
    // Add subtle temperature variations
    float TemperatureVariation = FMath::RandRange(-2.0f, 2.0f);
    Temperature = FMath::Clamp(Temperature + TemperatureVariation, 20.0f, 35.0f);
    
    // Occasionally trigger weather events
    if (FMath::RandRange(0.0f, 1.0f) < 0.1f) // 10% chance every 30 seconds
    {
        TriggerWeatherEvent();
    }
    
    UE_LOG(LogTemp, Verbose, TEXT("AtmosphericSystem: Updated environmental conditions - Humidity: %.2f, Temperature: %.1f°C"), 
           Humidity, Temperature);
}

void AAtmosphericSystem::TriggerWeatherEvent()
{
    // Randomly select a weather event type
    int32 EventType = FMath::RandRange(0, 2);
    
    switch (EventType)
    {
        case 0: // Fog bank rolling in
            UE_LOG(LogTemp, Log, TEXT("AtmosphericSystem: Fog bank event triggered"));
            FogSettings.FogDensity *= FMath::RandRange(1.5f, 3.0f);
            Humidity = FMath::Min(Humidity + 0.2f, 0.95f);
            break;
            
        case 1: // Atmospheric clearing
            UE_LOG(LogTemp, Log, TEXT("AtmosphericSystem: Atmospheric clearing event triggered"));
            AirPurity = FMath::Min(AirPurity + 0.1f, 1.0f);
            FogSettings.FogDensity *= FMath::RandRange(0.3f, 0.7f);
            break;
            
        case 2: // Pressure change
            UE_LOG(LogTemp, Log, TEXT("AtmosphericSystem: Pressure change event triggered"));
            AtmosphericPressure += FMath::RandRange(-0.1f, 0.1f);
            AtmosphericPressure = FMath::Clamp(AtmosphericPressure, 1.0f, 1.5f);
            break;
    }
    
    // Schedule return to normal conditions
    if (GetWorld())
    {
        FTimerHandle ResetTimerHandle;
        GetWorld()->GetTimerManager().SetTimer(ResetTimerHandle, this, &AAtmosphericSystem::ResetToNormalConditions, 
                                              FMath::RandRange(300.0f, 900.0f), false); // 5-15 minutes
    }
}

void AAtmosphericSystem::ResetToNormalConditions()
{
    UE_LOG(LogTemp, Log, TEXT("AtmosphericSystem: Resetting to normal atmospheric conditions"));
    
    // Gradually return to baseline values
    Humidity = FMath::Lerp(Humidity, 0.7f, 0.1f);
    AirPurity = FMath::Lerp(AirPurity, 0.95f, 0.1f);
    AtmosphericPressure = FMath::Lerp(AtmosphericPressure, 1.3f, 0.1f);
    FogSettings.FogDensity = FMath::Lerp(FogSettings.FogDensity, BaseFogDensity, 0.1f);
}

void AAtmosphericSystem::SetPrehistoricPreset()
{
    UE_LOG(LogTemp, Log, TEXT("AtmosphericSystem: Applying prehistoric atmosphere preset"));
    
    // Higher atmospheric pressure and humidity
    AtmosphericPressure = 1.4f;
    Humidity = 0.8f;
    AirPurity = 0.98f; // Very clean air
    Temperature = 30.0f; // Warmer climate
    
    // Adjust atmospheric parameters for prehistoric conditions
    AtmosphereSettings.RayleighScattering = FLinearColor(0.004f, 0.010f, 0.025f, 1.0f); // Slightly different scattering
    AtmosphereSettings.MieScattering = FLinearColor(0.002f, 0.002f, 0.002f, 1.0f); // Less pollution
    AtmosphereSettings.GroundAlbedo = FLinearColor(0.25f, 0.35f, 0.15f, 1.0f); // Lush vegetation
    
    // More dramatic cloud formations
    CloudSettings.CloudCoverage = 0.6f;
    CloudSettings.CloudDensity = 0.9f;
    CloudSettings.LayerHeight = 5.0f; // Taller clouds
    
    // Denser, more humid fog
    FogSettings.FogDensity = 0.03f;
    FogSettings.FogInscatteringColor = FLinearColor(0.5f, 0.7f, 0.9f, 1.0f); // Slightly greenish tint
    
    ApplyAtmosphericSettings();
    ApplyCloudSettings();
    ApplyFogSettings();
}

void AAtmosphericSystem::SetDangerousWeatherPreset()
{
    UE_LOG(LogTemp, Log, TEXT("AtmosphericSystem: Applying dangerous weather preset"));
    
    // Storm-like conditions
    Humidity = 0.95f;
    AtmosphericPressure = 1.1f; // Lower pressure = storms
    
    // Dramatic cloud coverage
    CloudSettings.CloudCoverage = 0.9f;
    CloudSettings.CloudDensity = 1.0f;
    CloudSettings.MultiScatteringOctaves = 2; // More dramatic lighting
    
    // Heavy fog for reduced visibility
    FogSettings.FogDensity = 0.08f;
    FogSettings.FogMaxOpacity = 0.9f;
    FogSettings.FogInscatteringColor = FLinearColor(0.4f, 0.5f, 0.6f, 1.0f); // Darker, more ominous
    
    ApplyCloudSettings();
    ApplyFogSettings();
}

float AAtmosphericSystem::GetVisibilityDistance() const
{
    // Calculate visibility based on fog density and atmospheric conditions
    float BaseVisibility = 50000.0f; // 500m base visibility
    float FogReduction = 1.0f - (FogSettings.FogDensity * 10.0f);
    float HumidityReduction = 1.0f - (Humidity * 0.3f);
    
    return BaseVisibility * FogReduction * HumidityReduction;
}

float AAtmosphericSystem::GetAtmosphericMood() const
{
    // Calculate overall atmospheric mood (0.0 = clear/peaceful, 1.0 = ominous/threatening)
    float FogContribution = FMath::Clamp(FogSettings.FogDensity * 12.5f, 0.0f, 0.4f);
    float HumidityContribution = FMath::Clamp((Humidity - 0.5f) * 0.6f, 0.0f, 0.3f);
    float CloudContribution = FMath::Clamp(CloudSettings.CloudCoverage * 0.3f, 0.0f, 0.3f);
    
    return FMath::Clamp(FogContribution + HumidityContribution + CloudContribution, 0.0f, 1.0f);
}