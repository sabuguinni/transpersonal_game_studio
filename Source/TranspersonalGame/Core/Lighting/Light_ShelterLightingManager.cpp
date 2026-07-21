#include "Light_ShelterLightingManager.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/RectLightComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

ALight_ShelterLightingManager::ALight_ShelterLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20 FPS for smooth fire flicker

    // Create root scene component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Initialize main fire light component
    MainFireLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("MainFireLight"));
    MainFireLight->SetupAttachment(RootComponent);
    MainFireLight->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
    MainFireLight->SetIntensity(800.0f);
    MainFireLight->SetLightColor(FLinearColor(1.0f, 0.6f, 0.3f, 1.0f));
    MainFireLight->SetAttenuationRadius(1200.0f);
    MainFireLight->SetSourceRadius(25.0f);
    MainFireLight->SetCastShadows(true);
    MainFireLight->SetVolumetricScatteringIntensity(2.0f);

    // Initialize secondary fire light component
    SecondaryFireLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("SecondaryFireLight"));
    SecondaryFireLight->SetupAttachment(RootComponent);
    SecondaryFireLight->SetRelativeLocation(FVector(200.0f, 150.0f, 30.0f));
    SecondaryFireLight->SetIntensity(600.0f);
    SecondaryFireLight->SetLightColor(FLinearColor(1.0f, 0.65f, 0.35f, 1.0f));
    SecondaryFireLight->SetAttenuationRadius(900.0f);
    SecondaryFireLight->SetSourceRadius(20.0f);
    SecondaryFireLight->SetCastShadows(true);
    SecondaryFireLight->SetVolumetricScatteringIntensity(1.5f);

    // Initialize entrance transition light
    EntranceLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("EntranceLight"));
    EntranceLight->SetupAttachment(RootComponent);
    EntranceLight->SetRelativeLocation(FVector(-300.0f, 0.0f, 200.0f));
    EntranceLight->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));
    EntranceLight->SetIntensity(1200.0f);
    EntranceLight->SetLightColor(FLinearColor(0.8f, 0.9f, 1.0f, 1.0f));
    EntranceLight->SetInnerConeAngle(25.0f);
    EntranceLight->SetOuterConeAngle(45.0f);
    EntranceLight->SetAttenuationRadius(2000.0f);
    EntranceLight->SetCastShadows(true);

    // Initialize ambient forest light
    AmbientLight = CreateDefaultSubobject<URectLightComponent>(TEXT("AmbientLight"));
    AmbientLight->SetupAttachment(RootComponent);
    AmbientLight->SetRelativeLocation(FVector(0.0f, 0.0f, 400.0f));
    AmbientLight->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f));
    AmbientLight->SetIntensity(400.0f);
    AmbientLight->SetLightColor(FLinearColor(0.9f, 1.0f, 0.8f, 1.0f));
    AmbientLight->SetBarnDoorAngle(35.0f);
    AmbientLight->SetBarnDoorLength(0.3f);

    // Initialize default shelter lighting data
    ShelterLightingData.ShelterType = ELight_ShelterType::Cave_MainFire;
    ShelterLightingData.bEnableFireFlicker = true;
    ShelterLightingData.bEnableVolumetricFog = true;

    // Fire flicker settings
    bEnableFireFlicker = true;
    FlickerTimeAccumulator = 0.0f;
    CurrentFlickerOffset = 0.0f;

    // Integration settings
    bIntegrateWithWeatherSystem = true;
    WeatherInfluenceStrength = 0.4f;
    bAdjustForTimeOfDay = true;
    DaytimeIntensityMultiplier = 0.6f;
    NighttimeIntensityMultiplier = 1.2f;
}

void ALight_ShelterLightingManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial shelter type configuration
    ApplyShelterTypePreset();
    ConfigureLightingComponents();
    UpdateAtmosphericEffects();

    UE_LOG(LogTemp, Log, TEXT("ShelterLightingManager: Initialized with shelter type %d"), 
           static_cast<int32>(ShelterLightingData.ShelterType));
}

void ALight_ShelterLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update fire flicker effect
    if (bEnableFireFlicker && ShelterLightingData.bEnableFireFlicker)
    {
        UpdateFireFlicker(DeltaTime);
    }
}

void ALight_ShelterLightingManager::SetShelterType(ELight_ShelterType NewShelterType)
{
    if (ShelterLightingData.ShelterType != NewShelterType)
    {
        ShelterLightingData.ShelterType = NewShelterType;
        ApplyShelterTypePreset();
        ConfigureLightingComponents();
        
        UE_LOG(LogTemp, Log, TEXT("ShelterLightingManager: Changed to shelter type %d"), 
               static_cast<int32>(NewShelterType));
    }
}

void ALight_ShelterLightingManager::UpdateFireLighting(const FLight_FireLightConfig& NewFireConfig)
{
    ShelterLightingData.FireConfig = NewFireConfig;
    
    if (MainFireLight)
    {
        MainFireLight->SetIntensity(NewFireConfig.BaseIntensity);
        MainFireLight->SetLightColor(NewFireConfig.FireColor);
        MainFireLight->SetAttenuationRadius(NewFireConfig.AttenuationRadius);
        MainFireLight->SetSourceRadius(NewFireConfig.SourceRadius);
    }

    if (SecondaryFireLight)
    {
        SecondaryFireLight->SetIntensity(NewFireConfig.BaseIntensity * 0.75f);
        SecondaryFireLight->SetLightColor(NewFireConfig.FireColor);
        SecondaryFireLight->SetAttenuationRadius(NewFireConfig.AttenuationRadius * 0.75f);
        SecondaryFireLight->SetSourceRadius(NewFireConfig.SourceRadius * 0.8f);
    }
}

void ALight_ShelterLightingManager::SetFireFlickerEnabled(bool bEnabled)
{
    bEnableFireFlicker = bEnabled;
    ShelterLightingData.bEnableFireFlicker = bEnabled;
    
    if (!bEnabled)
    {
        // Reset lights to base intensity
        if (MainFireLight)
        {
            MainFireLight->SetIntensity(ShelterLightingData.FireConfig.BaseIntensity);
        }
        if (SecondaryFireLight)
        {
            SecondaryFireLight->SetIntensity(ShelterLightingData.FireConfig.BaseIntensity * 0.75f);
        }
    }
}

void ALight_ShelterLightingManager::AdjustLightingForTimeOfDay(float TimeOfDayNormalized)
{
    if (!bAdjustForTimeOfDay) return;

    float IntensityMultiplier = 1.0f;
    
    // Calculate time-based intensity (0.0 = midnight, 0.5 = noon)
    if (TimeOfDayNormalized < 0.25f || TimeOfDayNormalized > 0.75f)
    {
        // Night time - increase fire light intensity
        IntensityMultiplier = NighttimeIntensityMultiplier;
    }
    else
    {
        // Day time - reduce fire light intensity
        IntensityMultiplier = DaytimeIntensityMultiplier;
    }

    // Apply multiplier to fire lights
    if (MainFireLight)
    {
        MainFireLight->SetIntensity(ShelterLightingData.FireConfig.BaseIntensity * IntensityMultiplier);
    }
    if (SecondaryFireLight)
    {
        SecondaryFireLight->SetIntensity(ShelterLightingData.FireConfig.BaseIntensity * 0.75f * IntensityMultiplier);
    }

    // Adjust entrance light based on external lighting
    if (EntranceLight)
    {
        float EntranceIntensity = FMath::Lerp(200.0f, 1200.0f, TimeOfDayNormalized);
        EntranceLight->SetIntensity(EntranceIntensity);
    }
}

void ALight_ShelterLightingManager::AdjustLightingForWeather(float WeatherIntensity, const FLinearColor& WeatherTint)
{
    if (!bIntegrateWithWeatherSystem) return;

    // Blend fire color with weather tint
    FLinearColor BlendedFireColor = BlendLightColors(
        ShelterLightingData.FireConfig.FireColor, 
        WeatherTint, 
        WeatherInfluenceStrength * WeatherIntensity
    );

    if (MainFireLight)
    {
        MainFireLight->SetLightColor(BlendedFireColor);
    }
    if (SecondaryFireLight)
    {
        SecondaryFireLight->SetLightColor(BlendedFireColor);
    }

    // Adjust entrance light for weather conditions
    if (EntranceLight)
    {
        float WeatherDimming = FMath::Lerp(1.0f, 0.3f, WeatherIntensity);
        EntranceLight->SetIntensity(1200.0f * WeatherDimming);
        EntranceLight->SetLightColor(BlendLightColors(
            FLinearColor(0.8f, 0.9f, 1.0f, 1.0f), 
            WeatherTint, 
            WeatherIntensity * 0.6f
        ));
    }
}

void ALight_ShelterLightingManager::OnFireLit()
{
    if (MainFireLight)
    {
        MainFireLight->SetVisibility(true);
        MainFireLight->SetIntensity(ShelterLightingData.FireConfig.BaseIntensity);
    }
    if (SecondaryFireLight)
    {
        SecondaryFireLight->SetVisibility(true);
        SecondaryFireLight->SetIntensity(ShelterLightingData.FireConfig.BaseIntensity * 0.75f);
    }
    
    SetFireFlickerEnabled(true);
    UE_LOG(LogTemp, Log, TEXT("ShelterLightingManager: Fire lit - lighting activated"));
}

void ALight_ShelterLightingManager::OnFireExtinguished()
{
    if (MainFireLight)
    {
        MainFireLight->SetVisibility(false);
    }
    if (SecondaryFireLight)
    {
        SecondaryFireLight->SetVisibility(false);
    }
    
    SetFireFlickerEnabled(false);
    UE_LOG(LogTemp, Log, TEXT("ShelterLightingManager: Fire extinguished - lighting deactivated"));
}

void ALight_ShelterLightingManager::OnFireIntensityChanged(float NewIntensity)
{
    float ClampedIntensity = FMath::Clamp(NewIntensity, 0.1f, 2.0f);
    
    if (MainFireLight)
    {
        MainFireLight->SetIntensity(ShelterLightingData.FireConfig.BaseIntensity * ClampedIntensity);
    }
    if (SecondaryFireLight)
    {
        SecondaryFireLight->SetIntensity(ShelterLightingData.FireConfig.BaseIntensity * 0.75f * ClampedIntensity);
    }
}

void ALight_ShelterLightingManager::EnableVolumetricFog(bool bEnabled)
{
    ShelterLightingData.bEnableVolumetricFog = bEnabled;
    
    if (MainFireLight)
    {
        MainFireLight->SetVolumetricScatteringIntensity(bEnabled ? 2.0f : 0.0f);
    }
    if (SecondaryFireLight)
    {
        SecondaryFireLight->SetVolumetricScatteringIntensity(bEnabled ? 1.5f : 0.0f);
    }
}

void ALight_ShelterLightingManager::SetVolumetricFogProperties(float Density, const FLinearColor& Color)
{
    ShelterLightingData.VolumetricFogDensity = Density;
    ShelterLightingData.VolumetricFogColor = Color;
    
    // Apply to light components
    if (MainFireLight)
    {
        MainFireLight->SetVolumetricScatteringIntensity(Density * 2.0f);
    }
    if (SecondaryFireLight)
    {
        SecondaryFireLight->SetVolumetricScatteringIntensity(Density * 1.5f);
    }
}

void ALight_ShelterLightingManager::UpdateFireFlicker(float DeltaTime)
{
    FlickerTimeAccumulator += DeltaTime;
    
    // Calculate flicker values for main and secondary fires
    float MainFlicker = CalculateFlickerValue(
        FlickerTimeAccumulator, 
        ShelterLightingData.FireConfig.FlickerSpeed, 
        ShelterLightingData.FireConfig.FlickerIntensity
    );
    
    float SecondaryFlicker = CalculateFlickerValue(
        FlickerTimeAccumulator * 1.3f, // Slightly different phase
        ShelterLightingData.FireConfig.FlickerSpeed * 0.8f, 
        ShelterLightingData.FireConfig.FlickerIntensity * 0.7f
    );

    // Apply flicker to main fire light
    if (MainFireLight)
    {
        float FlickeredIntensity = ShelterLightingData.FireConfig.BaseIntensity * (1.0f + MainFlicker);
        MainFireLight->SetIntensity(FlickeredIntensity);
    }

    // Apply flicker to secondary fire light
    if (SecondaryFireLight)
    {
        float FlickeredIntensity = ShelterLightingData.FireConfig.BaseIntensity * 0.75f * (1.0f + SecondaryFlicker);
        SecondaryFireLight->SetIntensity(FlickeredIntensity);
    }
}

void ALight_ShelterLightingManager::ApplyShelterTypePreset()
{
    switch (ShelterLightingData.ShelterType)
    {
        case ELight_ShelterType::Cave_MainFire:
            ShelterLightingData.FireConfig.BaseIntensity = 800.0f;
            ShelterLightingData.FireConfig.FireColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
            ShelterLightingData.FireConfig.AttenuationRadius = 1200.0f;
            break;
            
        case ELight_ShelterType::Cave_SecondaryFire:
            ShelterLightingData.FireConfig.BaseIntensity = 600.0f;
            ShelterLightingData.FireConfig.FireColor = FLinearColor(1.0f, 0.65f, 0.35f, 1.0f);
            ShelterLightingData.FireConfig.AttenuationRadius = 900.0f;
            break;
            
        case ELight_ShelterType::Cave_Entrance:
            ShelterLightingData.FireConfig.BaseIntensity = 1200.0f;
            ShelterLightingData.FireConfig.FireColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
            ShelterLightingData.FireConfig.AttenuationRadius = 2000.0f;
            break;
            
        case ELight_ShelterType::Forest_Ambient:
            ShelterLightingData.FireConfig.BaseIntensity = 400.0f;
            ShelterLightingData.FireConfig.FireColor = FLinearColor(0.9f, 1.0f, 0.8f, 1.0f);
            ShelterLightingData.FireConfig.AttenuationRadius = 1500.0f;
            break;
            
        case ELight_ShelterType::Rock_Overhang:
            ShelterLightingData.FireConfig.BaseIntensity = 500.0f;
            ShelterLightingData.FireConfig.FireColor = FLinearColor(0.95f, 0.8f, 0.6f, 1.0f);
            ShelterLightingData.FireConfig.AttenuationRadius = 1000.0f;
            break;
    }
}

void ALight_ShelterLightingManager::ConfigureLightingComponents()
{
    // Apply current configuration to all lighting components
    UpdateFireLighting(ShelterLightingData.FireConfig);
    EnableVolumetricFog(ShelterLightingData.bEnableVolumetricFog);
    SetVolumetricFogProperties(ShelterLightingData.VolumetricFogDensity, ShelterLightingData.VolumetricFogColor);
}

void ALight_ShelterLightingManager::UpdateAtmosphericEffects()
{
    // Configure volumetric scattering based on shelter type
    float ScatteringIntensity = 2.0f;
    
    switch (ShelterLightingData.ShelterType)
    {
        case ELight_ShelterType::Cave_MainFire:
        case ELight_ShelterType::Cave_SecondaryFire:
            ScatteringIntensity = 2.5f; // More atmospheric in caves
            break;
        case ELight_ShelterType::Forest_Ambient:
            ScatteringIntensity = 1.5f; // Less scattering in open forest
            break;
        default:
            ScatteringIntensity = 2.0f;
            break;
    }

    if (MainFireLight)
    {
        MainFireLight->SetVolumetricScatteringIntensity(ScatteringIntensity);
    }
    if (SecondaryFireLight)
    {
        SecondaryFireLight->SetVolumetricScatteringIntensity(ScatteringIntensity * 0.75f);
    }
}

float ALight_ShelterLightingManager::CalculateFlickerValue(float Time, float Speed, float Intensity)
{
    // Combine multiple sine waves for realistic fire flicker
    float PrimaryFlicker = FMath::Sin(Time * Speed) * Intensity;
    float SecondaryFlicker = FMath::Sin(Time * Speed * 1.7f) * Intensity * 0.5f;
    float TertiaryFlicker = FMath::Sin(Time * Speed * 2.3f) * Intensity * 0.3f;
    
    return (PrimaryFlicker + SecondaryFlicker + TertiaryFlicker) / 3.0f;
}

FLinearColor ALight_ShelterLightingManager::BlendLightColors(const FLinearColor& BaseColor, const FLinearColor& InfluenceColor, float BlendStrength)
{
    return FLinearColor(
        FMath::Lerp(BaseColor.R, InfluenceColor.R, BlendStrength),
        FMath::Lerp(BaseColor.G, InfluenceColor.G, BlendStrength),
        FMath::Lerp(BaseColor.B, InfluenceColor.B, BlendStrength),
        BaseColor.A
    );
}