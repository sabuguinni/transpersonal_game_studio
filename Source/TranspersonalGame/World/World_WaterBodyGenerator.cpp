#include "World_WaterBodyGenerator.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UWorld_WaterBodyGenerator::UWorld_WaterBodyGenerator()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize water body settings
    WaterBodySettings.RiverWidth = 2000.0f;
    WaterBodySettings.RiverDepth = 200.0f;
    WaterBodySettings.LakeRadius = 5000.0f;
    WaterBodySettings.LakeDepth = 500.0f;
    WaterBodySettings.WaterFlowSpeed = 100.0f;
    WaterBodySettings.WaterMaterialPath = TEXT("/Engine/BasicShapes/BasicShapeMaterial");
    
    // Initialize biome water configurations
    InitializeBiomeWaterConfigs();
}

void UWorld_WaterBodyGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoGenerateOnBeginPlay)
    {
        GenerateWaterBodiesForAllBiomes();
    }
}

void UWorld_WaterBodyGenerator::InitializeBiomeWaterConfigs()
{
    // Savana biome - sparse water sources
    FWorld_BiomeWaterConfig SavanaConfig;
    SavanaConfig.BiomeType = EWorld_BiomeType::Savana;
    SavanaConfig.RiverDensity = 0.3f;
    SavanaConfig.LakeDensity = 0.2f;
    SavanaConfig.WaterBodyScale = 0.8f;
    SavanaConfig.PreferredWaterTypes = {EWorld_WaterBodyType::SmallLake, EWorld_WaterBodyType::Stream};
    BiomeWaterConfigs.Add(EWorld_BiomeType::Savana, SavanaConfig);
    
    // Pantano biome - abundant water
    FWorld_BiomeWaterConfig PantanoConfig;
    PantanoConfig.BiomeType = EWorld_BiomeType::Pantano;
    PantanoConfig.RiverDensity = 0.8f;
    PantanoConfig.LakeDensity = 0.6f;
    PantanoConfig.WaterBodyScale = 1.2f;
    PantanoConfig.PreferredWaterTypes = {EWorld_WaterBodyType::River, EWorld_WaterBodyType::Lake, EWorld_WaterBodyType::Swamp};
    BiomeWaterConfigs.Add(EWorld_BiomeType::Pantano, PantanoConfig);
    
    // Floresta biome - moderate water with streams
    FWorld_BiomeWaterConfig FlorestaConfig;
    FlorestaConfig.BiomeType = EWorld_BiomeType::Floresta;
    FlorestaConfig.RiverDensity = 0.5f;
    FlorestaConfig.LakeDensity = 0.4f;
    FlorestaConfig.WaterBodyScale = 1.0f;
    FlorestaConfig.PreferredWaterTypes = {EWorld_WaterBodyType::Stream, EWorld_WaterBodyType::SmallLake};
    BiomeWaterConfigs.Add(EWorld_BiomeType::Floresta, FlorestaConfig);
    
    // Deserto biome - very sparse water
    FWorld_BiomeWaterConfig DesertoConfig;
    DesertoConfig.BiomeType = EWorld_BiomeType::Deserto;
    DesertoConfig.RiverDensity = 0.1f;
    DesertoConfig.LakeDensity = 0.05f;
    DesertoConfig.WaterBodyScale = 0.6f;
    DesertoConfig.PreferredWaterTypes = {EWorld_WaterBodyType::Oasis};
    BiomeWaterConfigs.Add(EWorld_BiomeType::Deserto, DesertoConfig);
    
    // Montanha biome - mountain lakes and streams
    FWorld_BiomeWaterConfig MontanhaConfig;
    MontanhaConfig.BiomeType = EWorld_BiomeType::Montanha;
    MontanhaConfig.RiverDensity = 0.4f;
    MontanhaConfig.LakeDensity = 0.3f;
    MontanhaConfig.WaterBodyScale = 0.9f;
    MontanhaConfig.PreferredWaterTypes = {EWorld_WaterBodyType::MountainLake, EWorld_WaterBodyType::Stream};
    BiomeWaterConfigs.Add(EWorld_BiomeType::Montanha, MontanhaConfig);
}

void UWorld_WaterBodyGenerator::GenerateWaterBodiesForAllBiomes()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("World_WaterBodyGenerator: No valid world found"));
        return;
    }
    
    // Generate water bodies for each biome
    for (const auto& BiomeConfig : BiomeWaterConfigs)
    {
        GenerateWaterBodiesForBiome(BiomeConfig.Value);
    }
    
    UE_LOG(LogTemp, Log, TEXT("World_WaterBodyGenerator: Generated water bodies for all biomes"));
}

void UWorld_WaterBodyGenerator::GenerateWaterBodiesForBiome(const FWorld_BiomeWaterConfig& BiomeConfig)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get biome center coordinates
    FVector BiomeCenter = GetBiomeCenterLocation(BiomeConfig.BiomeType);
    float BiomeRadius = 15000.0f; // 15km radius per biome
    
    // Generate rivers
    int32 RiverCount = FMath::RoundToInt(BiomeConfig.RiverDensity * MaxRiversPerBiome);
    for (int32 i = 0; i < RiverCount; i++)
    {
        GenerateRiver(BiomeCenter, BiomeRadius, BiomeConfig);
    }
    
    // Generate lakes
    int32 LakeCount = FMath::RoundToInt(BiomeConfig.LakeDensity * MaxLakesPerBiome);
    for (int32 i = 0; i < LakeCount; i++)
    {
        EWorld_WaterBodyType WaterType = GetRandomWaterTypeForBiome(BiomeConfig);
        GenerateLake(BiomeCenter, BiomeRadius, BiomeConfig, WaterType);
    }
}

FVector UWorld_WaterBodyGenerator::GetBiomeCenterLocation(EWorld_BiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EWorld_BiomeType::Savana:
            return FVector(0.0f, 0.0f, 100.0f);
        case EWorld_BiomeType::Pantano:
            return FVector(-50000.0f, -45000.0f, 100.0f);
        case EWorld_BiomeType::Floresta:
            return FVector(-45000.0f, 40000.0f, 100.0f);
        case EWorld_BiomeType::Deserto:
            return FVector(55000.0f, 0.0f, 100.0f);
        case EWorld_BiomeType::Montanha:
            return FVector(40000.0f, 50000.0f, 100.0f);
        default:
            return FVector::ZeroVector;
    }
}

EWorld_WaterBodyType UWorld_WaterBodyGenerator::GetRandomWaterTypeForBiome(const FWorld_BiomeWaterConfig& BiomeConfig) const
{
    if (BiomeConfig.PreferredWaterTypes.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, BiomeConfig.PreferredWaterTypes.Num() - 1);
        return BiomeConfig.PreferredWaterTypes[RandomIndex];
    }
    
    return EWorld_WaterBodyType::Lake;
}

void UWorld_WaterBodyGenerator::GenerateRiver(const FVector& BiomeCenter, float BiomeRadius, const FWorld_BiomeWaterConfig& BiomeConfig)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Generate river path with multiple segments
    TArray<FVector> RiverPoints;
    int32 RiverSegments = FMath::RandRange(3, 6);
    
    // Start point
    FVector StartPoint = BiomeCenter + FVector(
        FMath::RandRange(-BiomeRadius * 0.8f, BiomeRadius * 0.8f),
        FMath::RandRange(-BiomeRadius * 0.8f, BiomeRadius * 0.8f),
        0.0f
    );
    RiverPoints.Add(StartPoint);
    
    // Generate intermediate points
    for (int32 i = 1; i < RiverSegments; i++)
    {
        FVector PrevPoint = RiverPoints[i - 1];
        FVector NextPoint = PrevPoint + FVector(
            FMath::RandRange(-3000.0f, 3000.0f),
            FMath::RandRange(-3000.0f, 3000.0f),
            FMath::RandRange(-50.0f, 50.0f)
        );
        RiverPoints.Add(NextPoint);
    }
    
    // Create river segments
    for (int32 i = 0; i < RiverPoints.Num() - 1; i++)
    {
        CreateWaterBodyActor(RiverPoints[i], EWorld_WaterBodyType::River, BiomeConfig);
    }
}

void UWorld_WaterBodyGenerator::GenerateLake(const FVector& BiomeCenter, float BiomeRadius, const FWorld_BiomeWaterConfig& BiomeConfig, EWorld_WaterBodyType WaterType)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Random location within biome
    FVector LakeLocation = BiomeCenter + FVector(
        FMath::RandRange(-BiomeRadius * 0.6f, BiomeRadius * 0.6f),
        FMath::RandRange(-BiomeRadius * 0.6f, BiomeRadius * 0.6f),
        0.0f
    );
    
    CreateWaterBodyActor(LakeLocation, WaterType, BiomeConfig);
}

AStaticMeshActor* UWorld_WaterBodyGenerator::CreateWaterBodyActor(const FVector& Location, EWorld_WaterBodyType WaterType, const FWorld_BiomeWaterConfig& BiomeConfig)
{
    if (!GetWorld())
    {
        return nullptr;
    }
    
    // Spawn static mesh actor for water body
    AStaticMeshActor* WaterActor = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    if (!WaterActor)
    {
        return nullptr;
    }
    
    // Configure water body based on type
    FVector Scale = GetWaterBodyScale(WaterType, BiomeConfig);
    WaterActor->SetActorScale3D(Scale);
    
    // Set water body properties
    FString WaterBodyName = GetWaterBodyName(WaterType, BiomeConfig.BiomeType);
    WaterActor->SetActorLabel(WaterBodyName);
    
    // Configure static mesh component
    UStaticMeshComponent* MeshComp = WaterActor->GetStaticMeshComponent();
    if (MeshComp)
    {
        // Use plane mesh for water surface
        UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane"));
        if (PlaneMesh)
        {
            MeshComp->SetStaticMesh(PlaneMesh);
        }
        
        // Apply water material if available
        UMaterialInterface* WaterMaterial = LoadObject<UMaterialInterface>(nullptr, *WaterBodySettings.WaterMaterialPath);
        if (WaterMaterial)
        {
            MeshComp->SetMaterial(0, WaterMaterial);
        }
    }
    
    // Store reference
    GeneratedWaterBodies.Add(WaterActor);
    
    return WaterActor;
}

FVector UWorld_WaterBodyGenerator::GetWaterBodyScale(EWorld_WaterBodyType WaterType, const FWorld_BiomeWaterConfig& BiomeConfig) const
{
    float BaseScale = BiomeConfig.WaterBodyScale;
    
    switch (WaterType)
    {
        case EWorld_WaterBodyType::River:
            return FVector(20.0f * BaseScale, 2.0f * BaseScale, 0.1f);
        case EWorld_WaterBodyType::Lake:
            return FVector(10.0f * BaseScale, 10.0f * BaseScale, 0.1f);
        case EWorld_WaterBodyType::SmallLake:
            return FVector(5.0f * BaseScale, 5.0f * BaseScale, 0.1f);
        case EWorld_WaterBodyType::Stream:
            return FVector(15.0f * BaseScale, 1.0f * BaseScale, 0.1f);
        case EWorld_WaterBodyType::Swamp:
            return FVector(8.0f * BaseScale, 8.0f * BaseScale, 0.05f);
        case EWorld_WaterBodyType::Oasis:
            return FVector(3.0f * BaseScale, 3.0f * BaseScale, 0.1f);
        case EWorld_WaterBodyType::MountainLake:
            return FVector(6.0f * BaseScale, 6.0f * BaseScale, 0.2f);
        default:
            return FVector(5.0f * BaseScale, 5.0f * BaseScale, 0.1f);
    }
}

FString UWorld_WaterBodyGenerator::GetWaterBodyName(EWorld_WaterBodyType WaterType, EWorld_BiomeType BiomeType) const
{
    FString BiomeName = GetBiomeNameString(BiomeType);
    FString WaterTypeName = GetWaterTypeNameString(WaterType);
    
    return FString::Printf(TEXT("%s_%s_%d"), *BiomeName, *WaterTypeName, FMath::RandRange(1, 999));
}

FString UWorld_WaterBodyGenerator::GetBiomeNameString(EWorld_BiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EWorld_BiomeType::Savana: return TEXT("Savana");
        case EWorld_BiomeType::Pantano: return TEXT("Pantano");
        case EWorld_BiomeType::Floresta: return TEXT("Floresta");
        case EWorld_BiomeType::Deserto: return TEXT("Deserto");
        case EWorld_BiomeType::Montanha: return TEXT("Montanha");
        default: return TEXT("Unknown");
    }
}

FString UWorld_WaterBodyGenerator::GetWaterTypeNameString(EWorld_WaterBodyType WaterType) const
{
    switch (WaterType)
    {
        case EWorld_WaterBodyType::River: return TEXT("River");
        case EWorld_WaterBodyType::Lake: return TEXT("Lake");
        case EWorld_WaterBodyType::SmallLake: return TEXT("SmallLake");
        case EWorld_WaterBodyType::Stream: return TEXT("Stream");
        case EWorld_WaterBodyType::Swamp: return TEXT("Swamp");
        case EWorld_WaterBodyType::Oasis: return TEXT("Oasis");
        case EWorld_WaterBodyType::MountainLake: return TEXT("MountainLake");
        default: return TEXT("Water");
    }
}

void UWorld_WaterBodyGenerator::ClearAllWaterBodies()
{
    for (AStaticMeshActor* WaterActor : GeneratedWaterBodies)
    {
        if (IsValid(WaterActor))
        {
            WaterActor->Destroy();
        }
    }
    
    GeneratedWaterBodies.Empty();
    UE_LOG(LogTemp, Log, TEXT("World_WaterBodyGenerator: Cleared all generated water bodies"));
}

void UWorld_WaterBodyGenerator::RegenerateWaterBodies()
{
    ClearAllWaterBodies();
    GenerateWaterBodiesForAllBiomes();
}