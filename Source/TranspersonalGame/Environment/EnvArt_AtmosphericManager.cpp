#include "EnvArt_AtmosphericManager.h"
#include "Engine/World.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AEnvArt_AtmosphericManager::AEnvArt_AtmosphericManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Initialize default atmospheric settings
    AtmosphericSettings.FogDensity = 0.02f;
    AtmosphericSettings.FogHeightFalloff = 0.2f;
    AtmosphericSettings.FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    AtmosphericSettings.SunIntensity = 3.5f;
    AtmosphericSettings.SunAngle = FRotator(-15.0f, 45.0f, 0.0f);
}

void AEnvArt_AtmosphericManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeAtmosphere();
}

void AEnvArt_AtmosphericManager::InitializeAtmosphere()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find existing directional light
    TArray<AActor*> FoundLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundLights);
    
    if (FoundLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundLights[0]);
        if (SunLight)
        {
            ApplyAtmosphericSettings();
        }
    }

    // Find existing fog actors
    TArray<AActor*> FoundFogs;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundFogs);
    
    for (AActor* FogActor : FoundFogs)
    {
        if (AExponentialHeightFog* Fog = Cast<AExponentialHeightFog>(FogActor))
        {
            FogActors.Add(Fog);
        }
    }
}

void AEnvArt_AtmosphericManager::ApplyAtmosphericSettings()
{
    // Configure sun light
    if (SunLight)
    {
        SunLight->SetActorRotation(AtmosphericSettings.SunAngle);
        
        if (UDirectionalLightComponent* LightComp = SunLight->GetComponent())
        {
            LightComp->SetLightColor(AtmosphericSettings.SunColor);
            LightComp->SetIntensity(AtmosphericSettings.SunIntensity);
        }
    }

    // Configure fog actors
    for (AExponentialHeightFog* FogActor : FogActors)
    {
        if (FogActor && FogActor->GetComponent())
        {
            UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
            FogComp->SetFogDensity(AtmosphericSettings.FogDensity);
            FogComp->SetFogHeightFalloff(AtmosphericSettings.FogHeightFalloff);
            FogComp->SetFogInscatteringColor(AtmosphericSettings.FogColor);
        }
    }
}

void AEnvArt_AtmosphericManager::CreateBiomeFog(const FVector& Location, EBiomeType BiomeType)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Spawn fog actor at specified location
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AExponentialHeightFog* NewFog = World->SpawnActor<AExponentialHeightFog>(
        AExponentialHeightFog::StaticClass(),
        Location,
        FRotator::ZeroRotator,
        SpawnParams
    );

    if (NewFog)
    {
        FogActors.Add(NewFog);
        ConfigureFogForBiome(NewFog, BiomeType);
        
        // Set appropriate label
        FString BiomeName = UEnum::GetValueAsString(BiomeType);
        NewFog->SetActorLabel(FString::Printf(TEXT("EnvArt_Fog_%s"), *BiomeName));
    }
}

void AEnvArt_AtmosphericManager::ConfigureFogForBiome(AExponentialHeightFog* FogActor, EBiomeType BiomeType)
{
    if (!FogActor || !FogActor->GetComponent())
    {
        return;
    }

    FEnvArt_AtmosphericSettings BiomeSettings = GetBiomeAtmosphericSettings(BiomeType);
    UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();

    FogComp->SetFogDensity(BiomeSettings.FogDensity);
    FogComp->SetFogHeightFalloff(BiomeSettings.FogHeightFalloff);
    FogComp->SetFogInscatteringColor(BiomeSettings.FogColor);
}

FEnvArt_AtmosphericSettings AEnvArt_AtmosphericManager::GetBiomeAtmosphericSettings(EBiomeType BiomeType)
{
    FEnvArt_AtmosphericSettings Settings = AtmosphericSettings;

    switch (BiomeType)
    {
        case EBiomeType::Forest:
            Settings.FogDensity = 0.035f;
            Settings.FogColor = FLinearColor(0.6f, 0.8f, 0.7f, 1.0f); // Green tint
            break;
            
        case EBiomeType::Desert:
            Settings.FogDensity = 0.015f;
            Settings.FogColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f); // Warm tint
            break;
            
        case EBiomeType::Wetlands:
            Settings.FogDensity = 0.05f;
            Settings.FogHeightFalloff = 0.1f;
            Settings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f); // Cool blue tint
            break;
            
        case EBiomeType::Mountains:
            Settings.FogDensity = 0.025f;
            Settings.FogHeightFalloff = 0.3f;
            Settings.FogColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f); // Clear mountain air
            break;
            
        default: // Savanna
            Settings.FogDensity = 0.02f;
            Settings.FogColor = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f); // Warm grassland
            break;
    }

    return Settings;
}

void AEnvArt_AtmosphericManager::SetTimeOfDay(float TimeHours)
{
    // Convert time to sun angle (0-24 hours to rotation)
    float SunAngleY = (TimeHours / 24.0f) * 360.0f - 90.0f; // Start at east
    float SunAngleX = FMath::Sin(FMath::DegreesToRadians(TimeHours * 15.0f)) * -90.0f; // Arc across sky

    AtmosphericSettings.SunAngle = FRotator(SunAngleX, SunAngleY, 0.0f);

    // Adjust sun color and intensity based on time
    if (TimeHours >= 6.0f && TimeHours <= 8.0f) // Dawn
    {
        AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
        AtmosphericSettings.SunIntensity = 2.0f;
    }
    else if (TimeHours >= 17.0f && TimeHours <= 19.0f) // Dusk
    {
        AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
        AtmosphericSettings.SunIntensity = 1.5f;
    }
    else if (TimeHours >= 10.0f && TimeHours <= 16.0f) // Midday
    {
        AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
        AtmosphericSettings.SunIntensity = 4.0f;
    }
    else // Night
    {
        AtmosphericSettings.SunColor = FLinearColor(0.1f, 0.1f, 0.3f, 1.0f);
        AtmosphericSettings.SunIntensity = 0.1f;
    }

    ApplyAtmosphericSettings();
}

void AEnvArt_AtmosphericManager::UpdateEnvironmentalEffects()
{
    ApplyAtmosphericSettings();
    
    // Update all biome-specific fog settings
    for (int32 i = 0; i < FogActors.Num(); i++)
    {
        if (FogActors[i])
        {
            // Determine biome type based on fog actor location
            FVector FogLocation = FogActors[i]->GetActorLocation();
            EBiomeType BiomeType = EBiomeType::Savanna; // Default
            
            // Simple biome detection based on coordinates
            if (FogLocation.X < -20000.0f && FogLocation.Y > 20000.0f)
            {
                BiomeType = EBiomeType::Forest;
            }
            else if (FogLocation.X > 30000.0f && FogLocation.Y < -20000.0f)
            {
                BiomeType = EBiomeType::Desert;
            }
            else if (FMath::Abs(FogLocation.Y) > 60000.0f)
            {
                BiomeType = EBiomeType::Wetlands;
            }
            
            ConfigureFogForBiome(FogActors[i], BiomeType);
        }
    }
}