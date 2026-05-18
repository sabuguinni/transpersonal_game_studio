#include "EnvironmentManager.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AEnvironmentManager::AEnvironmentManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    CurrentTimeOfDay = 12.0f; // Noon
    bInitialized = false;

    // Initialize default biome configurations
    BiomeConfigurations.SetNum(5);
    
    // Savana biome
    BiomeConfigurations[0].BiomeType = EBiomeType::Savana;
    BiomeConfigurations[0].CenterLocation = FVector(0.0f, 0.0f, 100.0f);
    BiomeConfigurations[0].Radius = 15000.0f;
    BiomeConfigurations[0].AmbientColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    BiomeConfigurations[0].FogDensity = 0.01f;

    // Floresta biome
    BiomeConfigurations[1].BiomeType = EBiomeType::Floresta;
    BiomeConfigurations[1].CenterLocation = FVector(-45000.0f, 40000.0f, 100.0f);
    BiomeConfigurations[1].Radius = 12000.0f;
    BiomeConfigurations[1].AmbientColor = FLinearColor(0.7f, 1.0f, 0.7f, 1.0f);
    BiomeConfigurations[1].FogDensity = 0.05f;

    // Deserto biome
    BiomeConfigurations[2].BiomeType = EBiomeType::Deserto;
    BiomeConfigurations[2].CenterLocation = FVector(55000.0f, 0.0f, 100.0f);
    BiomeConfigurations[2].Radius = 18000.0f;
    BiomeConfigurations[2].AmbientColor = FLinearColor(1.0f, 0.9f, 0.6f, 1.0f);
    BiomeConfigurations[2].FogDensity = 0.008f;

    // Pantano biome
    BiomeConfigurations[3].BiomeType = EBiomeType::Pantano;
    BiomeConfigurations[3].CenterLocation = FVector(-50000.0f, -45000.0f, 50.0f);
    BiomeConfigurations[3].Radius = 10000.0f;
    BiomeConfigurations[3].AmbientColor = FLinearColor(0.8f, 1.0f, 0.9f, 1.0f);
    BiomeConfigurations[3].FogDensity = 0.08f;

    // Montanha biome
    BiomeConfigurations[4].BiomeType = EBiomeType::Montanha;
    BiomeConfigurations[4].CenterLocation = FVector(40000.0f, 50000.0f, 500.0f);
    BiomeConfigurations[4].Radius = 14000.0f;
    BiomeConfigurations[4].AmbientColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);
    BiomeConfigurations[4].FogDensity = 0.03f;
}

void AEnvironmentManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomes();
    UpdateLighting();
}

void AEnvironmentManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update time of day slowly
    CurrentTimeOfDay += DeltaTime * 0.01f; // Very slow day/night cycle
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay = 0.0f;
    }
    
    UpdateSunPosition(CurrentTimeOfDay);
}

void AEnvironmentManager::InitializeBiomes()
{
    if (bInitialized)
    {
        return;
    }

    // Find existing lighting actors
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }

    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }

    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyAtmosphere::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyAtmosphere = Cast<ASkyAtmosphere>(FoundActors[0]);
    }

    SpawnEnvironmentProps();
    bInitialized = true;
}

void AEnvironmentManager::SpawnEnvironmentProps()
{
    for (const FEnvArt_BiomeData& BiomeData : BiomeConfigurations)
    {
        SpawnPropsForBiome(BiomeData);
    }
}

void AEnvironmentManager::SpawnPropsForBiome(const FEnvArt_BiomeData& BiomeData)
{
    // Spawn props in a circle around the biome center
    int32 PropCount = 20; // Number of props per biome
    float AngleStep = 360.0f / PropCount;
    
    for (int32 i = 0; i < PropCount; ++i)
    {
        float Angle = i * AngleStep;
        float RadiusVariation = FMath::RandRange(0.3f, 0.8f) * BiomeData.Radius;
        
        FVector SpawnLocation = BiomeData.CenterLocation;
        SpawnLocation.X += FMath::Cos(FMath::DegreesToRadians(Angle)) * RadiusVariation;
        SpawnLocation.Y += FMath::Sin(FMath::DegreesToRadians(Angle)) * RadiusVariation;
        SpawnLocation.Z += FMath::RandRange(-50.0f, 100.0f);

        // Create a basic static mesh actor as placeholder
        AStaticMeshActor* PropActor = GetWorld()->SpawnActor<AStaticMeshActor>(SpawnLocation, FRotator::ZeroRotator);
        if (PropActor)
        {
            FString BiomeName = UEnum::GetValueAsString(BiomeData.BiomeType);
            PropActor->SetActorLabel(FString::Printf(TEXT("EnvProp_%s_%03d"), *BiomeName, i));
            SpawnedProps.Add(PropActor);
        }
    }
}

void AEnvironmentManager::UpdateLighting()
{
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(LightingSettings.SunIntensity);
        LightComp->SetLightColor(LightingSettings.SunColor);
        SunLight->SetActorRotation(LightingSettings.SunRotation);
    }

    if (SkyAtmosphere && SkyAtmosphere->GetComponent())
    {
        USkyAtmosphereComponent* SkyComp = SkyAtmosphere->GetComponent();
        SkyComp->SetAtmosphereHeight(60.0f);
    }
}

void AEnvironmentManager::SetTimeOfDay(float TimeHours)
{
    CurrentTimeOfDay = FMath::Clamp(TimeHours, 0.0f, 24.0f);
    UpdateSunPosition(CurrentTimeOfDay);
}

void AEnvironmentManager::UpdateSunPosition(float TimeHours)
{
    if (!SunLight)
    {
        return;
    }

    // Calculate sun position based on time
    float SunAngle = (TimeHours - 6.0f) * 15.0f; // 15 degrees per hour, sunrise at 6 AM
    float Elevation = FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 90.0f;
    float Azimuth = (TimeHours / 24.0f) * 360.0f;

    FRotator SunRotation = FRotator(-Elevation, Azimuth, 0.0f);
    SunLight->SetActorRotation(SunRotation);

    // Adjust sun intensity based on elevation
    if (SunLight->GetLightComponent())
    {
        float IntensityMultiplier = FMath::Max(0.1f, FMath::Sin(FMath::DegreesToRadians(SunAngle)));
        SunLight->GetLightComponent()->SetIntensity(LightingSettings.SunIntensity * IntensityMultiplier);
    }
}

EBiomeType AEnvironmentManager::GetBiomeAtLocation(const FVector& Location) const
{
    float ClosestDistance = FLT_MAX;
    EBiomeType ClosestBiome = EBiomeType::Savana;

    for (const FEnvArt_BiomeData& BiomeData : BiomeConfigurations)
    {
        float Distance = FVector::Dist2D(Location, BiomeData.CenterLocation);
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBiome = BiomeData.BiomeType;
        }
    }

    return ClosestBiome;
}

void AEnvironmentManager::ApplyBiomeAtmosphere(EBiomeType BiomeType)
{
    for (const FEnvArt_BiomeData& BiomeData : BiomeConfigurations)
    {
        if (BiomeData.BiomeType == BiomeType)
        {
            UpdateFogSettings(BiomeType);
            break;
        }
    }
}

void AEnvironmentManager::UpdateFogSettings(EBiomeType BiomeType)
{
    if (!HeightFog || !HeightFog->GetComponent())
    {
        return;
    }

    UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
    
    for (const FEnvArt_BiomeData& BiomeData : BiomeConfigurations)
    {
        if (BiomeData.BiomeType == BiomeType)
        {
            FogComp->SetFogDensity(BiomeData.FogDensity);
            FogComp->SetFogInscatteringColor(BiomeData.AmbientColor);
            break;
        }
    }
}

void AEnvironmentManager::EditorSpawnTestProps()
{
    SpawnEnvironmentProps();
}