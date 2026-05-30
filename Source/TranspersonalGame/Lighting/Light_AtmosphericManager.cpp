#include "Light_AtmosphericManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/PointLight.h"
#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ALight_AtmosphericManager::ALight_AtmosphericManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // Initialize default settings
    TimeOfDay = 0.0f;
    bIsInitialized = false;

    // Set up default Cretaceous lighting settings
    CretaceousSettings.SunIntensity = 8.5f;
    CretaceousSettings.ColorTemperature = 3200.0f;
    CretaceousSettings.SunColor = FLinearColor(1.0f, 0.85f, 0.65f, 1.0f);
    CretaceousSettings.SunAngle = FRotator(-25.0f, 45.0f, 0.0f);
    CretaceousSettings.AtmosphericScattering = 0.8f;
    CretaceousSettings.VolumetricFogDensity = 0.02f;

    SetupDefaultBiomeConfigs();
}

void ALight_AtmosphericManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphericManager: BeginPlay started"));
    
    // Initialize the atmospheric lighting system
    InitializeAtmosphericSystem();
}

void ALight_AtmosphericManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsInitialized)
    {
        // Update time of day for dynamic lighting
        TimeOfDay += DeltaTime * 0.1f; // Slow time progression
        if (TimeOfDay > 24.0f)
        {
            TimeOfDay = 0.0f;
        }

        // Update atmospheric perspective based on time
        UpdateAtmosphericPerspective();
    }
}

void ALight_AtmosphericManager::InitializeAtmosphericSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphericManager: Initializing atmospheric system"));

    // Find existing lighting components in the world
    FindLightingComponents();

    // Apply Cretaceous period lighting
    ApplyCretaceousLighting();

    // Configure biome-specific lighting
    ConfigureBiomeLighting();

    // Enhance Lumen Global Illumination
    EnhanceLumenGlobalIllumination();

    // Setup volumetric fog
    SetupVolumetricFog();

    // Create dinosaur rim lighting
    CreateDinosaurRimLighting();

    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphericManager: Atmospheric system initialized"));
}

void ALight_AtmosphericManager::FindLightingComponents()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Light_AtmosphericManager: World not found"));
        return;
    }

    // Find directional light (sun)
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        SunLight = *ActorItr;
        break;
    }

    // Find sky atmosphere
    for (TActorIterator<ASkyAtmosphere> ActorItr(World); ActorItr; ++ActorItr)
    {
        SkyAtmosphere = *ActorItr;
        break;
    }

    // Find height fog
    for (TActorIterator<AExponentialHeightFog> ActorItr(World); ActorItr; ++ActorItr)
    {
        HeightFog = *ActorItr;
        break;
    }

    // Find post process volume
    for (TActorIterator<APostProcessVolume> ActorItr(World); ActorItr; ++ActorItr)
    {
        PostProcessVolume = *ActorItr;
        break;
    }

    UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphericManager: Found components - Sun: %s, Sky: %s, Fog: %s, PPV: %s"),
        SunLight ? TEXT("Yes") : TEXT("No"),
        SkyAtmosphere ? TEXT("Yes") : TEXT("No"),
        HeightFog ? TEXT("Yes") : TEXT("No"),
        PostProcessVolume ? TEXT("Yes") : TEXT("No"));
}

void ALight_AtmosphericManager::ApplyCretaceousLighting()
{
    if (!SunLight)
    {
        UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphericManager: No directional light found"));
        return;
    }

    UDirectionalLightComponent* SunComponent = SunLight->GetLightComponent();
    if (SunComponent)
    {
        // Apply Cretaceous period lighting settings
        SunComponent->SetIntensity(CretaceousSettings.SunIntensity);
        SunComponent->SetLightColor(CretaceousSettings.SunColor);
        SunComponent->SetTemperature(CretaceousSettings.ColorTemperature);
        SunComponent->SetUseTemperature(true);
        
        // Set sun angle for late afternoon dramatic lighting
        SunLight->SetActorRotation(CretaceousSettings.SunAngle);

        UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphericManager: Applied Cretaceous sun lighting"));
    }
}

void ALight_AtmosphericManager::ConfigureBiomeLighting()
{
    CreateBiomeAtmosphericLights();
    UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphericManager: Configured biome lighting"));
}

void ALight_AtmosphericManager::EnhanceLumenGlobalIllumination()
{
    if (!PostProcessVolume)
    {
        UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphericManager: No post process volume found"));
        return;
    }

    ApplyLumenEnhancements();
    UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphericManager: Enhanced Lumen GI"));
}

void ALight_AtmosphericManager::SetupVolumetricFog()
{
    if (!HeightFog)
    {
        UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphericManager: No height fog found"));
        return;
    }

    UExponentialHeightFogComponent* FogComponent = HeightFog->GetComponent();
    if (FogComponent)
    {
        FogComponent->SetFogDensity(CretaceousSettings.VolumetricFogDensity);
        FogComponent->SetFogHeightFalloff(0.1f);
        FogComponent->SetInscatteringColor(FLinearColor(0.9f, 0.8f, 0.6f, 1.0f));
        FogComponent->SetVolumetricFog(true);
        FogComponent->SetVolumetricFogScatteringDistribution(0.2f);
        FogComponent->SetVolumetricFogAlbedo(FLinearColor(0.95f, 0.9f, 0.8f, 1.0f));

        UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphericManager: Setup volumetric fog"));
    }
}

void ALight_AtmosphericManager::CreateDinosaurRimLighting()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find dinosaur actors and add rim lighting
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);

    int32 RimLightsCreated = 0;
    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Dinosaur")))
        {
            FVector DinosaurLocation = Actor->GetActorLocation();
            FVector RimLightLocation = DinosaurLocation + FVector(-500.0f, -500.0f, 300.0f);

            APointLight* RimLight = World->SpawnActor<APointLight>(APointLight::StaticClass(), RimLightLocation, FRotator::ZeroRotator);
            if (RimLight)
            {
                UPointLightComponent* RimComponent = RimLight->GetPointLightComponent();
                if (RimComponent)
                {
                    RimComponent->SetIntensity(3.0f);
                    RimComponent->SetLightColor(FLinearColor(1.0f, 0.8f, 0.5f, 1.0f));
                    RimComponent->SetAttenuationRadius(1000.0f);
                }

                RimLight->SetActorLabel(FString::Printf(TEXT("RimLight_%s"), *Actor->GetName()));
                DinosaurRimLights.Add(RimLight);
                RimLightsCreated++;
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphericManager: Created %d dinosaur rim lights"), RimLightsCreated);
}

void ALight_AtmosphericManager::UpdateAtmosphericPerspective()
{
    if (!SkyAtmosphere)
    {
        return;
    }

    USkyAtmosphereComponent* SkyComponent = SkyAtmosphere->GetSkyAtmosphereComponent();
    if (SkyComponent)
    {
        // Subtle atmospheric changes based on time of day
        float TimeNormalized = TimeOfDay / 24.0f;
        float AtmosphericVariation = 0.8f + 0.2f * FMath::Sin(TimeNormalized * PI * 2.0f);
        
        SkyComponent->SetRayleighScatteringScale(CretaceousSettings.AtmosphericScattering * AtmosphericVariation);
    }
}

void ALight_AtmosphericManager::SetupDefaultBiomeConfigs()
{
    BiomeLightingConfigs.Empty();

    // Savana
    FLight_BiomeLightingConfig Savana;
    Savana.BiomeName = TEXT("Savana");
    Savana.BiomeCenter = FVector(0.0f, 0.0f, 0.0f);
    Savana.AmbientTint = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    Savana.AtmosphericDensity = 1.0f;
    Savana.FogIntensity = 0.8f;
    BiomeLightingConfigs.Add(Savana);

    // Pantano
    FLight_BiomeLightingConfig Pantano;
    Pantano.BiomeName = TEXT("Pantano");
    Pantano.BiomeCenter = FVector(-50000.0f, -45000.0f, 0.0f);
    Pantano.AmbientTint = FLinearColor(0.6f, 0.8f, 0.7f, 1.0f);
    Pantano.AtmosphericDensity = 1.5f;
    Pantano.FogIntensity = 1.2f;
    BiomeLightingConfigs.Add(Pantano);

    // Floresta
    FLight_BiomeLightingConfig Floresta;
    Floresta.BiomeName = TEXT("Floresta");
    Floresta.BiomeCenter = FVector(-45000.0f, 40000.0f, 0.0f);
    Floresta.AmbientTint = FLinearColor(0.4f, 0.7f, 0.4f, 1.0f);
    Floresta.AtmosphericDensity = 1.3f;
    Floresta.FogIntensity = 1.0f;
    BiomeLightingConfigs.Add(Floresta);

    // Deserto
    FLight_BiomeLightingConfig Deserto;
    Deserto.BiomeName = TEXT("Deserto");
    Deserto.BiomeCenter = FVector(55000.0f, 0.0f, 0.0f);
    Deserto.AmbientTint = FLinearColor(1.0f, 0.8f, 0.5f, 1.0f);
    Deserto.AtmosphericDensity = 0.7f;
    Deserto.FogIntensity = 0.5f;
    BiomeLightingConfigs.Add(Deserto);

    // Montanha
    FLight_BiomeLightingConfig Montanha;
    Montanha.BiomeName = TEXT("Montanha");
    Montanha.BiomeCenter = FVector(40000.0f, 50000.0f, 0.0f);
    Montanha.AmbientTint = FLinearColor(0.8f, 0.8f, 1.0f, 1.0f);
    Montanha.AtmosphericDensity = 0.9f;
    Montanha.FogIntensity = 1.1f;
    BiomeLightingConfigs.Add(Montanha);
}

void ALight_AtmosphericManager::CreateBiomeAtmosphericLights()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Clear existing biome lights
    for (AActor* Light : BiomeAtmosphericLights)
    {
        if (Light)
        {
            Light->Destroy();
        }
    }
    BiomeAtmosphericLights.Empty();

    // Create atmospheric lights for each biome
    for (const FLight_BiomeLightingConfig& Config : BiomeLightingConfigs)
    {
        FVector LightLocation = Config.BiomeCenter + FVector(0.0f, 0.0f, 800.0f);
        
        APointLight* AtmosphericLight = World->SpawnActor<APointLight>(APointLight::StaticClass(), LightLocation, FRotator::ZeroRotator);
        if (AtmosphericLight)
        {
            UPointLightComponent* LightComponent = AtmosphericLight->GetPointLightComponent();
            if (LightComponent)
            {
                LightComponent->SetIntensity(2.0f * Config.AtmosphericDensity);
                LightComponent->SetLightColor(Config.AmbientTint);
                LightComponent->SetAttenuationRadius(15000.0f);
                LightComponent->SetUseInverseSquaredFalloff(false);
                LightComponent->SetLightFalloffExponent(0.5f);
            }

            AtmosphericLight->SetActorLabel(FString::Printf(TEXT("AtmosphericLight_%s"), *Config.BiomeName));
            BiomeAtmosphericLights.Add(AtmosphericLight);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphericManager: Created %d biome atmospheric lights"), BiomeAtmosphericLights.Num());
}

void ALight_AtmosphericManager::ApplyLumenEnhancements()
{
    if (!PostProcessVolume)
    {
        return;
    }

    FPostProcessSettings& Settings = PostProcessVolume->Settings;
    
    // Enhanced Lumen settings for prehistoric realism
    Settings.bOverride_LumenSceneLightingQuality = true;
    Settings.LumenSceneLightingQuality = 2.0f;
    
    Settings.bOverride_LumenSceneDetail = true;
    Settings.LumenSceneDetail = 2.0f;
    
    Settings.bOverride_LumenSurfaceCacheResolution = true;
    Settings.LumenSurfaceCacheResolution = 2.0f;
    
    Settings.bOverride_LumenFinalGatherQuality = true;
    Settings.LumenFinalGatherQuality = 2.0f;

    UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphericManager: Applied Lumen enhancements"));
}