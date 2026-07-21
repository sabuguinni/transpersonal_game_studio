#include "Light_InteriorSystem.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/RectLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

ALight_InteriorSystem::ALight_InteriorSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Create main fire light component
    MainFireLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("MainFireLight"));
    MainFireLight->SetupAttachment(RootComponent);
    MainFireLight->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
    MainFireLight->SetIntensity(1500.0f);
    MainFireLight->SetLightColor(FLinearColor(1.0f, 0.4f, 0.1f, 1.0f));
    MainFireLight->SetTemperature(2200.0f);
    MainFireLight->SetAttenuationRadius(800.0f);
    MainFireLight->SetCastShadows(true);
    MainFireLight->SetCastVolumetricShadow(true);

    // Create rim fire light component
    RimFireLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("RimFireLight"));
    RimFireLight->SetupAttachment(RootComponent);
    RimFireLight->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
    RimFireLight->SetIntensity(600.0f);
    RimFireLight->SetLightColor(FLinearColor(1.0f, 0.6f, 0.2f, 1.0f));
    RimFireLight->SetTemperature(2800.0f);
    RimFireLight->SetAttenuationRadius(400.0f);
    RimFireLight->SetCastShadows(false);

    // Create entrance spot light component
    EntranceLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("EntranceLight"));
    EntranceLight->SetupAttachment(RootComponent);
    EntranceLight->SetRelativeLocation(FVector(-300.0f, 0.0f, 200.0f));
    EntranceLight->SetRelativeRotation(FRotator(-30.0f, 90.0f, 0.0f));
    EntranceLight->SetIntensity(1000.0f);
    EntranceLight->SetLightColor(FLinearColor(0.7f, 0.8f, 1.0f, 1.0f));
    EntranceLight->SetTemperature(6500.0f);
    EntranceLight->SetAttenuationRadius(1200.0f);
    EntranceLight->SetInnerConeAngle(25.0f);
    EntranceLight->SetOuterConeAngle(45.0f);
    EntranceLight->SetCastShadows(true);

    // Create depth fill rect light component
    DepthFillLight = CreateDefaultSubobject<URectLightComponent>(TEXT("DepthFillLight"));
    DepthFillLight->SetupAttachment(RootComponent);
    DepthFillLight->SetRelativeLocation(FVector(200.0f, 0.0f, 150.0f));
    DepthFillLight->SetRelativeRotation(FRotator(-45.0f, 180.0f, 0.0f));
    DepthFillLight->SetIntensity(300.0f);
    DepthFillLight->SetLightColor(FLinearColor(0.3f, 0.4f, 0.6f, 1.0f));
    DepthFillLight->SetTemperature(7000.0f);
    DepthFillLight->SetAttenuationRadius(600.0f);
    DepthFillLight->SetBarnDoorAngle(30.0f);
    DepthFillLight->SetCastShadows(false);

    // Create fire pit mesh component (placeholder)
    FirePitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirePitMesh"));
    FirePitMesh->SetupAttachment(RootComponent);
    FirePitMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

    // Initialize configuration
    BaseFireIntensity = FireConfig.MainIntensity;
    BaseRimIntensity = FireConfig.RimIntensity;
}

void ALight_InteriorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    ConfigureLightingForType();
}

void ALight_InteriorSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (FireConfig.bEnableFlickering)
    {
        UpdateFireIntensity(DeltaTime);
    }
}

void ALight_InteriorSystem::SetInteriorType(ELight_InteriorType NewType)
{
    InteriorType = NewType;
    
    if (bAutoConfigureForInteriorType)
    {
        ConfigureLightingForType();
    }
}

void ALight_InteriorSystem::ConfigureLightingForType()
{
    switch (InteriorType)
    {
        case ELight_InteriorType::CaveDwelling:
            FireConfig.MainIntensity = 1500.0f;
            FireConfig.FireColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);
            FireConfig.FireTemperature = 2200.0f;
            AtmosphereConfig.EntranceLightIntensity = 1000.0f;
            AtmosphereConfig.DepthFillIntensity = 300.0f;
            break;
            
        case ELight_InteriorType::RockShelter:
            FireConfig.MainIntensity = 1200.0f;
            FireConfig.FireColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
            FireConfig.FireTemperature = 2400.0f;
            AtmosphereConfig.EntranceLightIntensity = 1200.0f;
            AtmosphereConfig.DepthFillIntensity = 400.0f;
            break;
            
        case ELight_InteriorType::TribalHut:
            FireConfig.MainIntensity = 1000.0f;
            FireConfig.FireColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
            FireConfig.FireTemperature = 2600.0f;
            AtmosphereConfig.EntranceLightIntensity = 800.0f;
            AtmosphereConfig.DepthFillIntensity = 250.0f;
            break;
            
        case ELight_InteriorType::SacredSpace:
            FireConfig.MainIntensity = 1800.0f;
            FireConfig.FireColor = FLinearColor(1.0f, 0.3f, 0.05f, 1.0f);
            FireConfig.FireTemperature = 2000.0f;
            AtmosphereConfig.EntranceLightIntensity = 600.0f;
            AtmosphereConfig.DepthFillIntensity = 500.0f;
            break;
    }
    
    // Apply configurations
    SetFireConfiguration(FireConfig);
    SetAtmosphereConfiguration(AtmosphereConfig);
}

void ALight_InteriorSystem::SetFireConfiguration(const FLight_FireConfiguration& NewConfig)
{
    FireConfig = NewConfig;
    BaseFireIntensity = FireConfig.MainIntensity;
    BaseRimIntensity = FireConfig.RimIntensity;
    
    if (MainFireLight)
    {
        MainFireLight->SetIntensity(FireConfig.MainIntensity);
        MainFireLight->SetLightColor(FireConfig.FireColor);
        MainFireLight->SetTemperature(FireConfig.FireTemperature);
        MainFireLight->SetAttenuationRadius(FireConfig.AttenuationRadius);
    }
    
    if (RimFireLight)
    {
        RimFireLight->SetIntensity(FireConfig.RimIntensity);
        RimFireLight->SetLightColor(FLinearColor(
            FireConfig.FireColor.R,
            FMath::Min(FireConfig.FireColor.G + 0.2f, 1.0f),
            FMath::Min(FireConfig.FireColor.B + 0.1f, 1.0f),
            1.0f
        ));
        RimFireLight->SetTemperature(FireConfig.FireTemperature + 600.0f);
        RimFireLight->SetAttenuationRadius(FireConfig.AttenuationRadius * 0.5f);
    }
}

void ALight_InteriorSystem::SetAtmosphereConfiguration(const FLight_AtmosphereConfiguration& NewConfig)
{
    AtmosphereConfig = NewConfig;
    
    if (EntranceLight)
    {
        EntranceLight->SetIntensity(AtmosphereConfig.EntranceLightIntensity);
        EntranceLight->SetLightColor(AtmosphereConfig.EntranceLightColor);
        EntranceLight->SetTemperature(AtmosphereConfig.EntranceTemperature);
    }
    
    if (DepthFillLight)
    {
        DepthFillLight->SetIntensity(AtmosphereConfig.DepthFillIntensity);
        DepthFillLight->SetLightColor(AtmosphereConfig.DepthFillColor);
    }
}

void ALight_InteriorSystem::EnableFlickering(bool bEnable)
{
    FireConfig.bEnableFlickering = bEnable;
}

void ALight_InteriorSystem::SetFlickerParameters(float Speed, float Intensity)
{
    FireConfig.FlickerSpeed = Speed;
    FireConfig.FlickerIntensity = Intensity;
}

void ALight_InteriorSystem::UpdateFireIntensity(float DeltaTime)
{
    FlickerTimer += DeltaTime * FireConfig.FlickerSpeed;
    
    // Generate flickering using sine waves with noise
    float FlickerNoise = FMath::Sin(FlickerTimer) * 0.5f + 
                        FMath::Sin(FlickerTimer * 1.7f) * 0.3f + 
                        FMath::Sin(FlickerTimer * 2.3f) * 0.2f;
    
    float FlickerMultiplier = 1.0f + (FlickerNoise * FireConfig.FlickerIntensity);
    
    if (MainFireLight)
    {
        MainFireLight->SetIntensity(BaseFireIntensity * FlickerMultiplier);
    }
    
    if (RimFireLight)
    {
        RimFireLight->SetIntensity(BaseRimIntensity * FlickerMultiplier * 0.8f);
    }
}

void ALight_InteriorSystem::SetDayNightCycle(float TimeOfDay)
{
    // Adjust entrance light based on time of day
    float DayIntensity = FMath::Sin(TimeOfDay * PI); // 0 at night, 1 at noon
    float NightIntensity = 1.0f - DayIntensity;
    
    if (EntranceLight)
    {
        float AdjustedIntensity = AtmosphereConfig.EntranceLightIntensity * DayIntensity;
        EntranceLight->SetIntensity(FMath::Max(AdjustedIntensity, 100.0f)); // Minimum ambient
        
        // Adjust color temperature (warmer at dawn/dusk)
        float ColorTemp = 6500.0f;
        if (DayIntensity < 0.3f) // Dawn/dusk
        {
            ColorTemp = FMath::Lerp(3200.0f, 6500.0f, DayIntensity / 0.3f);
        }
        EntranceLight->SetTemperature(ColorTemp);
    }
    
    // Fire becomes more prominent at night
    float FireMultiplier = 1.0f + (NightIntensity * 0.3f);
    BaseFireIntensity = FireConfig.MainIntensity * FireMultiplier;
    BaseRimIntensity = FireConfig.RimIntensity * FireMultiplier;
}

void ALight_InteriorSystem::SetWeatherInfluence(float RainIntensity, float WindStrength)
{
    // Rain affects entrance light (dimmer, cooler)
    if (EntranceLight)
    {
        float RainDimming = 1.0f - (RainIntensity * 0.4f);
        float AdjustedIntensity = AtmosphereConfig.EntranceLightIntensity * RainDimming;
        EntranceLight->SetIntensity(AdjustedIntensity);
        
        // Cooler color during rain
        FLinearColor RainColor = FLinearColor::LerpUsingHSV(
            AtmosphereConfig.EntranceLightColor,
            FLinearColor(0.6f, 0.7f, 1.0f, 1.0f),
            RainIntensity * 0.5f
        );
        EntranceLight->SetLightColor(RainColor);
    }
    
    // Wind affects fire flickering
    if (WindStrength > 0.1f)
    {
        FireConfig.FlickerSpeed = FMath::Lerp(2.0f, 5.0f, WindStrength);
        FireConfig.FlickerIntensity = FMath::Lerp(0.3f, 0.7f, WindStrength);
    }
}