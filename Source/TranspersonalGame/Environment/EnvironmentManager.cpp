#include "EnvironmentManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/StaticMeshActor.h"

AEnvironmentManager::AEnvironmentManager()
{
    PrimaryActorTick.bCanEverTick = false;
    AssetsPerBiome = 500;
    SpawnRadius = 10000.0f;
}

void AEnvironmentManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize biome definitions if empty
    if (BiomeDefinitions.Num() == 0)
    {
        InitializeBiomeDefinitions();
    }
    
    // Discover available assets if empty
    if (EnvironmentAssets.Num() == 0)
    {
        DiscoverEnvironmentAssets();
    }
}

void AEnvironmentManager::InitializeBiomeDefinitions()
{
    BiomeDefinitions.Empty();
    
    // Savana biome
    FEnvArt_BiomeCoordinates Savana;
    Savana.BiomeType = EEnvArt_BiomeType::Savana;
    Savana.CenterLocation = FVector(0.0f, 0.0f, 100.0f);
    Savana.Radius = 15000.0f;
    BiomeDefinitions.Add(Savana);
    
    // Floresta biome
    FEnvArt_BiomeCoordinates Floresta;
    Floresta.BiomeType = EEnvArt_BiomeType::Floresta;
    Floresta.CenterLocation = FVector(-45000.0f, 40000.0f, 100.0f);
    Floresta.Radius = 12000.0f;
    BiomeDefinitions.Add(Floresta);
    
    // Pantano biome
    FEnvArt_BiomeCoordinates Pantano;
    Pantano.BiomeType = EEnvArt_BiomeType::Pantano;
    Pantano.CenterLocation = FVector(-50000.0f, -45000.0f, 50.0f);
    Pantano.Radius = 10000.0f;
    BiomeDefinitions.Add(Pantano);
    
    // Deserto biome
    FEnvArt_BiomeCoordinates Deserto;
    Deserto.BiomeType = EEnvArt_BiomeType::Deserto;
    Deserto.CenterLocation = FVector(55000.0f, 0.0f, 200.0f);
    Deserto.Radius = 18000.0f;
    BiomeDefinitions.Add(Deserto);
    
    // Montanha biome
    FEnvArt_BiomeCoordinates Montanha;
    Montanha.BiomeType = EEnvArt_BiomeType::Montanha;
    Montanha.CenterLocation = FVector(40000.0f, 50000.0f, 500.0f);
    Montanha.Radius = 14000.0f;
    BiomeDefinitions.Add(Montanha);
    
    UE_LOG(LogTemp, Warning, TEXT("EnvironmentManager: Initialized %d biome definitions"), BiomeDefinitions.Num());
}

void AEnvironmentManager::DiscoverEnvironmentAssets()
{
    EnvironmentAssets.Empty();
    
    // Add placeholder assets - these would be discovered from Content Browser in a real implementation
    FEnvArt_EnvironmentAsset TreeAsset;
    TreeAsset.AssetName = TEXT("Forest_Tree");
    TreeAsset.PreferredBiome = EEnvArt_BiomeType::Floresta;
    TreeAsset.SpawnDensity = 2.0f;
    TreeAsset.ScaleRange = FVector(0.8f, 1.5f, 1.0f);
    EnvironmentAssets.Add(TreeAsset);
    
    FEnvArt_EnvironmentAsset RockAsset;
    RockAsset.AssetName = TEXT("Savana_Rock");
    RockAsset.PreferredBiome = EEnvArt_BiomeType::Savana;
    RockAsset.SpawnDensity = 1.5f;
    RockAsset.ScaleRange = FVector(0.5f, 2.0f, 1.0f);
    EnvironmentAssets.Add(RockAsset);
    
    FEnvArt_EnvironmentAsset LogAsset;
    LogAsset.AssetName = TEXT("Fallen_Log");
    LogAsset.PreferredBiome = EEnvArt_BiomeType::Floresta;
    LogAsset.SpawnDensity = 0.8f;
    LogAsset.ScaleRange = FVector(0.7f, 1.3f, 1.0f);
    EnvironmentAssets.Add(LogAsset);
    
    UE_LOG(LogTemp, Warning, TEXT("EnvironmentManager: Discovered %d environment assets"), EnvironmentAssets.Num());
}

void AEnvironmentManager::PopulateBiome(EEnvArt_BiomeType BiomeType, int32 AssetCount)
{
    const FEnvArt_BiomeCoordinates* TargetBiome = nullptr;
    for (const auto& Biome : BiomeDefinitions)
    {
        if (Biome.BiomeType == BiomeType)
        {
            TargetBiome = &Biome;
            break;
        }
    }
    
    if (!TargetBiome)
    {
        UE_LOG(LogTemp, Error, TEXT("EnvironmentManager: Biome type not found"));
        return;
    }
    
    // Filter assets for this biome
    TArray<FEnvArt_EnvironmentAsset> BiomeAssets;
    for (const auto& Asset : EnvironmentAssets)
    {
        if (Asset.PreferredBiome == BiomeType)
        {
            BiomeAssets.Add(Asset);
        }
    }
    
    if (BiomeAssets.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnvironmentManager: No assets found for biome"));
        return;
    }
    
    // Spawn assets
    int32 SpawnedCount = 0;
    for (int32 i = 0; i < AssetCount; ++i)
    {
        const FEnvArt_EnvironmentAsset& Asset = BiomeAssets[FMath::RandRange(0, BiomeAssets.Num() - 1)];
        FVector SpawnLocation = GetRandomLocationInBiome(*TargetBiome);
        FRotator SpawnRotation = GetRandomRotation();
        
        SpawnAssetAtLocation(Asset, SpawnLocation, SpawnRotation);
        SpawnedCount++;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EnvironmentManager: Spawned %d assets in biome"), SpawnedCount);
}

void AEnvironmentManager::PopulateAllBiomes()
{
    for (const auto& Biome : BiomeDefinitions)
    {
        PopulateBiome(Biome.BiomeType, AssetsPerBiome);
    }
}

FVector AEnvironmentManager::GetBiomeCenter(EEnvArt_BiomeType BiomeType) const
{
    for (const auto& Biome : BiomeDefinitions)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome.CenterLocation;
        }
    }
    return FVector::ZeroVector;
}

void AEnvironmentManager::SetGoldenHourLighting()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find directional light
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    
    for (AActor* LightActor : DirectionalLights)
    {
        ADirectionalLight* DirLight = Cast<ADirectionalLight>(LightActor);
        if (DirLight && DirLight->GetLightComponent())
        {
            // Set golden hour rotation (low sun angle)
            DirLight->SetActorRotation(FRotator(-15.0f, 45.0f, 0.0f));
            
            // Set warm golden color
            DirLight->GetLightComponent()->SetLightColor(FLinearColor(1.0f, 0.9f, 0.7f, 1.0f));
            DirLight->GetLightComponent()->SetIntensity(3.5f);
            
            UE_LOG(LogTemp, Warning, TEXT("EnvironmentManager: Applied golden hour lighting"));
            break;
        }
    }
}

void AEnvironmentManager::AddVolumetricFog()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find exponential height fog
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
    
    for (AActor* FogActor : FogActors)
    {
        AExponentialHeightFog* HeightFog = Cast<AExponentialHeightFog>(FogActor);
        if (HeightFog && HeightFog->GetComponent())
        {
            UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
            
            // Set atmospheric fog parameters
            FogComp->SetFogDensity(0.02f);
            FogComp->SetFogHeightFalloff(0.2f);
            FogComp->SetFogMaxOpacity(0.8f);
            FogComp->SetStartDistance(1000.0f);
            FogComp->SetFogCutoffDistance(50000.0f);
            
            // Set warm fog color
            FogComp->SetFogInscatteringColor(FLinearColor(0.9f, 0.8f, 0.6f, 1.0f));
            
            UE_LOG(LogTemp, Warning, TEXT("EnvironmentManager: Applied volumetric fog"));
            break;
        }
    }
}

void AEnvironmentManager::SpawnAssetAtLocation(const FEnvArt_EnvironmentAsset& Asset, const FVector& Location, const FRotator& Rotation)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // In a real implementation, this would load the actual mesh asset
    // For now, we'll create a placeholder static mesh actor
    AStaticMeshActor* SpawnedActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (SpawnedActor)
    {
        SpawnedActor->SetActorLabel(FString::Printf(TEXT("%s_%d"), *Asset.AssetName, FMath::RandRange(1000, 9999)));
        
        // Apply random scale within range
        float RandomScale = FMath::RandRange(Asset.ScaleRange.X, Asset.ScaleRange.Y);
        SpawnedActor->SetActorScale3D(FVector(RandomScale));
    }
}

FVector AEnvironmentManager::GetRandomLocationInBiome(const FEnvArt_BiomeCoordinates& Biome) const
{
    // Generate random point within biome radius
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomRadius = FMath::RandRange(0.0f, Biome.Radius);
    
    FVector Offset;
    Offset.X = FMath::Cos(RandomAngle) * RandomRadius;
    Offset.Y = FMath::Sin(RandomAngle) * RandomRadius;
    Offset.Z = FMath::RandRange(-100.0f, 100.0f); // Small height variation
    
    return Biome.CenterLocation + Offset;
}

FRotator AEnvironmentManager::GetRandomRotation() const
{
    return FRotator(
        FMath::RandRange(-10.0f, 10.0f), // Small pitch variation
        FMath::RandRange(0.0f, 360.0f),  // Random yaw
        FMath::RandRange(-5.0f, 5.0f)    // Small roll variation
    );
}