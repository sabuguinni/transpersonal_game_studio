#include "Light_VolcanicAtmosphereSystem.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

ALight_VolcanicAtmosphereSystem::ALight_VolcanicAtmosphereSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create main volcanic glow component
    MainVolcanicGlow = CreateDefaultSubobject<UPointLightComponent>(TEXT("MainVolcanicGlow"));
    RootComponent = MainVolcanicGlow;
    MainVolcanicGlow->SetIntensity(2000.0f);
    MainVolcanicGlow->SetLightColor(FLinearColor(1.0f, 0.3f, 0.1f, 1.0f));
    MainVolcanicGlow->SetAttenuationRadius(3000.0f);
    MainVolcanicGlow->SetCastShadows(true);

    // Create lava pit light component
    LavaPitLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("LavaPitLight"));
    LavaPitLight->SetupAttachment(RootComponent);
    LavaPitLight->SetRelativeLocation(FVector(-200.0f, -200.0f, -100.0f));
    LavaPitLight->SetIntensity(1800.0f);
    LavaPitLight->SetLightColor(FLinearColor(1.0f, 0.2f, 0.05f, 1.0f));
    LavaPitLight->SetAttenuationRadius(2500.0f);
    LavaPitLight->SetCastShadows(false);

    // Create cave entrance spotlight
    CaveEntranceLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("CaveEntranceLight"));
    CaveEntranceLight->SetupAttachment(RootComponent);
    CaveEntranceLight->SetRelativeLocation(FVector(200.0f, 200.0f, 200.0f));
    CaveEntranceLight->SetRelativeRotation(FRotator(-45.0f, 180.0f, 0.0f));
    CaveEntranceLight->SetIntensity(1500.0f);
    CaveEntranceLight->SetLightColor(FLinearColor(1.0f, 0.4f, 0.15f, 1.0f));
    CaveEntranceLight->SetAttenuationRadius(2000.0f);
    CaveEntranceLight->SetOuterConeAngle(60.0f);
    CaveEntranceLight->SetInnerConeAngle(30.0f);

    // Create volcanic fog component
    VolcanicFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("VolcanicFog"));
    VolcanicFog->SetupAttachment(RootComponent);
    VolcanicFog->SetFogDensity(0.08f);
    VolcanicFog->SetFogInscatteringColor(FLinearColor(0.8f, 0.3f, 0.1f, 1.0f));
    VolcanicFog->SetFogHeightFalloff(0.2f);
    VolcanicFog->SetFogMaxOpacity(1.0f);

    // Initialize lighting config
    LightingConfig.LavaGlowIntensity = 2000.0f;
    LightingConfig.LavaColor = FLinearColor(1.0f, 0.3f, 0.1f, 1.0f);
    LightingConfig.AttenuationRadius = 3000.0f;
    LightingConfig.FogDensity = 0.08f;
    LightingConfig.FogColor = FLinearColor(0.8f, 0.3f, 0.1f, 1.0f);

    BaseIntensity = 2000.0f;
    FlickerTimer = 0.0f;
}

void ALight_VolcanicAtmosphereSystem::BeginPlay()
{
    Super::BeginPlay();
    SetupVolcanicLighting();
}

void ALight_VolcanicAtmosphereSystem::SetupVolcanicLighting()
{
    if (MainVolcanicGlow)
    {
        MainVolcanicGlow->SetIntensity(LightingConfig.LavaGlowIntensity);
        MainVolcanicGlow->SetLightColor(LightingConfig.LavaColor);
        MainVolcanicGlow->SetAttenuationRadius(LightingConfig.AttenuationRadius);
    }

    if (LavaPitLight)
    {
        LavaPitLight->SetIntensity(LightingConfig.LavaGlowIntensity * 0.9f);
        LavaPitLight->SetLightColor(FLinearColor(1.0f, 0.2f, 0.05f, 1.0f));
        LavaPitLight->SetAttenuationRadius(LightingConfig.AttenuationRadius * 0.8f);
    }

    if (CaveEntranceLight)
    {
        CaveEntranceLight->SetIntensity(LightingConfig.LavaGlowIntensity * 0.75f);
        CaveEntranceLight->SetLightColor(FLinearColor(1.0f, 0.4f, 0.15f, 1.0f));
        CaveEntranceLight->SetAttenuationRadius(LightingConfig.AttenuationRadius * 0.67f);
    }

    if (VolcanicFog)
    {
        VolcanicFog->SetFogDensity(LightingConfig.FogDensity);
        VolcanicFog->SetFogInscatteringColor(LightingConfig.FogColor);
    }

    BaseIntensity = LightingConfig.LavaGlowIntensity;
}

void ALight_VolcanicAtmosphereSystem::UpdateLavaIntensity(float NewIntensity)
{
    LightingConfig.LavaGlowIntensity = NewIntensity;
    BaseIntensity = NewIntensity;
    
    if (MainVolcanicGlow)
    {
        MainVolcanicGlow->SetIntensity(NewIntensity);
    }
    
    if (LavaPitLight)
    {
        LavaPitLight->SetIntensity(NewIntensity * 0.9f);
    }
}

void ALight_VolcanicAtmosphereSystem::SetVolcanicFogDensity(float NewDensity)
{
    LightingConfig.FogDensity = NewDensity;
    
    if (VolcanicFog)
    {
        VolcanicFog->SetFogDensity(NewDensity);
    }
}

void ALight_VolcanicAtmosphereSystem::EnableVolcanicAtmosphere(bool bEnable)
{
    if (MainVolcanicGlow)
    {
        MainVolcanicGlow->SetVisibility(bEnable);
    }
    
    if (LavaPitLight)
    {
        LavaPitLight->SetVisibility(bEnable);
    }
    
    if (CaveEntranceLight)
    {
        CaveEntranceLight->SetVisibility(bEnable);
    }
    
    if (VolcanicFog)
    {
        VolcanicFog->SetVisibility(bEnable);
    }
}

void ALight_VolcanicAtmosphereSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Create flickering lava effect
    FlickerTimer += DeltaTime;
    
    float FlickerIntensity = BaseIntensity + FMath::Sin(FlickerTimer * 3.0f) * 200.0f + 
                            FMath::Sin(FlickerTimer * 7.0f) * 100.0f;
    
    if (MainVolcanicGlow)
    {
        MainVolcanicGlow->SetIntensity(FlickerIntensity);
    }
    
    if (LavaPitLight)
    {
        float LavaFlicker = BaseIntensity * 0.9f + FMath::Sin(FlickerTimer * 5.0f) * 150.0f;
        LavaPitLight->SetIntensity(LavaFlicker);
    }
}