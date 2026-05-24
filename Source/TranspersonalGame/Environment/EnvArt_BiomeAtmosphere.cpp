#include "EnvArt_BiomeAtmosphere.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AEnvArt_BiomeAtmosphere::AEnvArt_BiomeAtmosphere()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create particle components
    DustParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("DustParticleComponent"));
    DustParticleComponent->SetupAttachment(RootComponent);
    DustParticleComponent->bAutoActivate = false;

    PollenParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("PollenParticleComponent"));
    PollenParticleComponent->SetupAttachment(RootComponent);
    PollenParticleComponent->bAutoActivate = false;

    // Create ambient audio component
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;

    // Initialize default values
    BiomeType = EBiomeType::Forest;
    TimeOfDay = 0.5f; // Noon
    DayDuration = 1200.0f; // 20 minutes real time = 24 hours game time

    // Initialize cached references
    CachedDirectionalLight = nullptr;
    CachedSkyLight = nullptr;
    CachedHeightFog = nullptr;

    InitializeBiomeSettings();
}

void AEnvArt_BiomeAtmosphere::BeginPlay()
{
    Super::BeginPlay();

    // Find existing lighting actors in the world
    TArray<AActor*> FoundActors;
    
    // Find DirectionalLight
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        CachedDirectionalLight = Cast<ADirectionalLight>(FoundActors[0]);
    }

    // Find SkyLight
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        CachedSkyLight = Cast<ASkyLight>(FoundActors[0]);
    }

    // Find ExponentialHeightFog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        CachedHeightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }

    // Apply initial biome atmosphere
    ApplyBiomeAtmosphere(BiomeType);
    SetGoldenHourLighting();
}

void AEnvArt_BiomeAtmosphere::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update time of day
    if (DayDuration > 0.0f)
    {
        TimeOfDay += DeltaTime / DayDuration;
        if (TimeOfDay > 1.0f)
        {
            TimeOfDay -= 1.0f;
        }
        UpdateLightingForTimeOfDay();
    }
}

void AEnvArt_BiomeAtmosphere::InitializeBiomeSettings()
{
    // Swamp settings - dark, misty atmosphere
    SwampSettings.SunColor = FLinearColor(0.8f, 0.9f, 0.6f);
    SwampSettings.SunIntensity = 2.0f;
    SwampSettings.SunRotation = FRotator(-30.0f, 45.0f, 0.0f);
    SwampSettings.FogColor = FLinearColor(0.6f, 0.7f, 0.5f);
    SwampSettings.FogDensity = 0.08f;
    SwampSettings.FogHeightFalloff = 0.1f;
    SwampSettings.SkyLightColor = FLinearColor(0.4f, 0.5f, 0.6f);
    SwampSettings.SkyLightIntensity = 0.5f;
    SwampSettings.bEnableDustParticles = false;
    SwampSettings.bEnablePollenParticles = true;
    SwampSettings.bEnableAmbientSounds = true;

    // Forest settings - dappled sunlight through canopy
    ForestSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.7f);
    ForestSettings.SunIntensity = 3.5f;
    ForestSettings.SunRotation = FRotator(-45.0f, 30.0f, 0.0f);
    ForestSettings.FogColor = FLinearColor(0.7f, 0.8f, 0.9f);
    ForestSettings.FogDensity = 0.03f;
    ForestSettings.FogHeightFalloff = 0.15f;
    ForestSettings.SkyLightColor = FLinearColor(0.5f, 0.7f, 0.9f);
    ForestSettings.SkyLightIntensity = 0.8f;
    ForestSettings.bEnableDustParticles = true;
    ForestSettings.bEnablePollenParticles = true;
    ForestSettings.bEnableAmbientSounds = true;

    // Savanna settings - bright, open sky
    SavannaSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.8f);
    SavannaSettings.SunIntensity = 5.0f;
    SavannaSettings.SunRotation = FRotator(-60.0f, 45.0f, 0.0f);
    SavannaSettings.FogColor = FLinearColor(0.9f, 0.85f, 0.7f);
    SavannaSettings.FogDensity = 0.01f;
    SavannaSettings.FogHeightFalloff = 0.3f;
    SavannaSettings.SkyLightColor = FLinearColor(0.6f, 0.8f, 1.0f);
    SavannaSettings.SkyLightIntensity = 1.2f;
    SavannaSettings.bEnableDustParticles = true;
    SavannaSettings.bEnablePollenParticles = false;
    SavannaSettings.bEnableAmbientSounds = true;

    // Desert settings - harsh, bright light
    DesertSettings.SunColor = FLinearColor(1.0f, 0.85f, 0.6f);
    DesertSettings.SunIntensity = 6.0f;
    DesertSettings.SunRotation = FRotator(-70.0f, 60.0f, 0.0f);
    DesertSettings.FogColor = FLinearColor(1.0f, 0.9f, 0.7f);
    DesertSettings.FogDensity = 0.005f;
    DesertSettings.FogHeightFalloff = 0.5f;
    DesertSettings.SkyLightColor = FLinearColor(0.8f, 0.9f, 1.0f);
    DesertSettings.SkyLightIntensity = 1.5f;
    DesertSettings.bEnableDustParticles = true;
    DesertSettings.bEnablePollenParticles = false;
    DesertSettings.bEnableAmbientSounds = true;

    // Mountain settings - cool, clear air
    MountainSettings.SunColor = FLinearColor(0.9f, 0.95f, 1.0f);
    MountainSettings.SunIntensity = 4.0f;
    MountainSettings.SunRotation = FRotator(-50.0f, 20.0f, 0.0f);
    MountainSettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f);
    MountainSettings.FogDensity = 0.02f;
    MountainSettings.FogHeightFalloff = 0.25f;
    MountainSettings.SkyLightColor = FLinearColor(0.7f, 0.8f, 1.0f);
    MountainSettings.SkyLightIntensity = 1.0f;
    MountainSettings.bEnableDustParticles = false;
    MountainSettings.bEnablePollenParticles = false;
    MountainSettings.bEnableAmbientSounds = true;
}

void AEnvArt_BiomeAtmosphere::ApplyBiomeAtmosphere(EBiomeType InBiomeType)
{
    BiomeType = InBiomeType;
    FEnvArt_BiomeAtmosphereSettings Settings = GetCurrentBiomeSettings();
    ApplyAtmosphereSettings(Settings);
}

void AEnvArt_BiomeAtmosphere::UpdateTimeOfDay(float NewTimeOfDay)
{
    TimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 1.0f);
    UpdateLightingForTimeOfDay();
}

void AEnvArt_BiomeAtmosphere::SetGoldenHourLighting()
{
    TimeOfDay = 0.75f; // Golden hour (6 PM in 24-hour cycle)
    UpdateLightingForTimeOfDay();

    if (CachedDirectionalLight && CachedDirectionalLight->GetLightComponent())
    {
        // Golden hour specific adjustments
        FLinearColor GoldenColor = FLinearColor(1.0f, 0.8f, 0.5f);
        CachedDirectionalLight->GetLightComponent()->SetLightColor(GoldenColor);
        
        // Lower angle for golden hour
        FRotator GoldenRotation = FRotator(-15.0f, 45.0f, 0.0f);
        CachedDirectionalLight->SetActorRotation(GoldenRotation);
    }
}

void AEnvArt_BiomeAtmosphere::EnableVolumetricFog(bool bEnable)
{
    if (CachedHeightFog && CachedHeightFog->GetComponent())
    {
        CachedHeightFog->GetComponent()->SetVolumetricFog(bEnable);
        if (bEnable)
        {
            CachedHeightFog->GetComponent()->SetVolumetricFogScatteringDistribution(0.2f);
            CachedHeightFog->GetComponent()->SetVolumetricFogAlbedo(FLinearColor(0.9f, 0.9f, 0.9f));
        }
    }
}

void AEnvArt_BiomeAtmosphere::SpawnAtmosphericParticles()
{
    FEnvArt_BiomeAtmosphereSettings Settings = GetCurrentBiomeSettings();

    if (Settings.bEnableDustParticles && DustParticleComponent)
    {
        DustParticleComponent->Activate();
    }

    if (Settings.bEnablePollenParticles && PollenParticleComponent)
    {
        PollenParticleComponent->Activate();
    }

    if (Settings.bEnableAmbientSounds && AmbientAudioComponent)
    {
        AmbientAudioComponent->Activate();
    }
}

void AEnvArt_BiomeAtmosphere::UpdateLightingForTimeOfDay()
{
    if (!CachedDirectionalLight || !CachedDirectionalLight->GetLightComponent())
    {
        return;
    }

    // Calculate sun angle based on time of day
    float SunAngle = (TimeOfDay - 0.5f) * 180.0f; // -90 to +90 degrees
    FRotator SunRotation = FRotator(SunAngle, 45.0f, 0.0f);
    CachedDirectionalLight->SetActorRotation(SunRotation);

    // Adjust intensity based on sun angle
    float IntensityMultiplier = FMath::Clamp(FMath::Cos(FMath::DegreesToRadians(FMath::Abs(SunAngle))), 0.1f, 1.0f);
    FEnvArt_BiomeAtmosphereSettings Settings = GetCurrentBiomeSettings();
    CachedDirectionalLight->GetLightComponent()->SetIntensity(Settings.SunIntensity * IntensityMultiplier);

    // Adjust color temperature based on time of day
    FLinearColor TimeColor = Settings.SunColor;
    if (TimeOfDay < 0.25f || TimeOfDay > 0.75f) // Dawn/Dusk
    {
        TimeColor = FLinearColor(1.0f, 0.7f, 0.4f); // Warmer colors
    }
    else if (TimeOfDay < 0.1f || TimeOfDay > 0.9f) // Night
    {
        TimeColor = FLinearColor(0.3f, 0.4f, 0.8f); // Cooler moonlight
        IntensityMultiplier *= 0.1f; // Much dimmer
    }
    
    CachedDirectionalLight->GetLightComponent()->SetLightColor(TimeColor);
}

FEnvArt_BiomeAtmosphereSettings AEnvArt_BiomeAtmosphere::GetCurrentBiomeSettings() const
{
    switch (BiomeType)
    {
        case EBiomeType::Swamp:
            return SwampSettings;
        case EBiomeType::Forest:
            return ForestSettings;
        case EBiomeType::Savanna:
            return SavannaSettings;
        case EBiomeType::Desert:
            return DesertSettings;
        case EBiomeType::Mountain:
            return MountainSettings;
        default:
            return ForestSettings;
    }
}

void AEnvArt_BiomeAtmosphere::ApplyAtmosphereSettings(const FEnvArt_BiomeAtmosphereSettings& Settings)
{
    // Apply directional light settings
    if (CachedDirectionalLight && CachedDirectionalLight->GetLightComponent())
    {
        CachedDirectionalLight->GetLightComponent()->SetLightColor(Settings.SunColor);
        CachedDirectionalLight->GetLightComponent()->SetIntensity(Settings.SunIntensity);
        CachedDirectionalLight->SetActorRotation(Settings.SunRotation);
    }

    // Apply sky light settings
    if (CachedSkyLight && CachedSkyLight->GetLightComponent())
    {
        CachedSkyLight->GetLightComponent()->SetLightColor(Settings.SkyLightColor);
        CachedSkyLight->GetLightComponent()->SetIntensity(Settings.SkyLightIntensity);
    }

    // Apply fog settings
    if (CachedHeightFog && CachedHeightFog->GetComponent())
    {
        CachedHeightFog->GetComponent()->SetFogInscatteringColor(Settings.FogColor);
        CachedHeightFog->GetComponent()->SetFogDensity(Settings.FogDensity);
        CachedHeightFog->GetComponent()->SetFogHeightFalloff(Settings.FogHeightFalloff);
    }

    // Enable volumetric fog for enhanced atmosphere
    EnableVolumetricFog(true);

    // Spawn atmospheric particles
    SpawnAtmosphericParticles();
}