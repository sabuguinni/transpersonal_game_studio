#include "World_BiomeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"

UWorld_BiomeManager::UWorld_BiomeManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    RandomSeed = 12345;
    WorldSize = 50000.0f;
    bAutoGenerateOnBeginPlay = true;
    
    SetupDefaultBiomes();
}

void UWorld_BiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoGenerateOnBeginPlay)
    {
        InitializeBiomes();
    }
}

void UWorld_BiomeManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UWorld_BiomeManager::InitializeBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Initializing biomes..."));
    
    // Clear any existing generated content
    ClearAllGeneratedContent();
    
    // Set random seed for consistent generation
    FMath::RandInit(RandomSeed);
    
    // Create the 5 biomes in strategic locations
    CreateBiome(EWorld_BiomeType::Swampland, FVector(-15000, -15000, 0), 8000.0f);
    CreateBiome(EWorld_BiomeType::Forest, FVector(0, 0, 0), 10000.0f);
    CreateBiome(EWorld_BiomeType::Savanna, FVector(15000, -10000, 0), 12000.0f);
    CreateBiome(EWorld_BiomeType::Desert, FVector(20000, 15000, 0), 9000.0f);
    CreateBiome(EWorld_BiomeType::SnowyMountains, FVector(-10000, 20000, 1000), 7000.0f);
    
    // Generate river system connecting biomes
    GenerateRiverSystem();
    
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Biome initialization complete"));
}

void UWorld_BiomeManager::CreateBiome(EWorld_BiomeType BiomeType, FVector Location, float Radius)
{
    UE_LOG(LogTemp, Warning, TEXT("Creating biome %d at location %s with radius %f"), 
           (int32)BiomeType, *Location.ToString(), Radius);
    
    switch (BiomeType)
    {
        case EWorld_BiomeType::Swampland:
            CreateSwamplandBiome(Location, Radius);
            break;
        case EWorld_BiomeType::Forest:
            CreateForestBiome(Location, Radius);
            break;
        case EWorld_BiomeType::Savanna:
            CreateSavannaBiome(Location, Radius);
            break;
        case EWorld_BiomeType::Desert:
            CreateDesertBiome(Location, Radius);
            break;
        case EWorld_BiomeType::SnowyMountains:
            CreateSnowyMountainBiome(Location, Radius);
            break;
    }
    
    // Place vegetation specific to this biome
    PlaceVegetationInBiome(BiomeType, FMath::RandRange(50, 150));
}

EWorld_BiomeType UWorld_BiomeManager::GetBiomeAtLocation(const FVector& Location) const
{
    float ClosestDistance = MAX_FLT;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::Forest;
    
    for (const FWorld_BiomeData& Biome : BiomeConfigurations)
    {
        float Distance = CalculateDistanceToBiomeCenter(Location, Biome);
        if (Distance < ClosestDistance && Distance <= Biome.Radius)
        {
            ClosestDistance = Distance;
            ClosestBiome = Biome.BiomeType;
        }
    }
    
    return ClosestBiome;
}

FWorld_BiomeData UWorld_BiomeManager::GetBiomeData(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeData& Biome : BiomeConfigurations)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome;
        }
    }
    
    return FWorld_BiomeData();
}

void UWorld_BiomeManager::SpawnTerrainFeature(const FWorld_TerrainFeature& Feature)
{
    if (!Feature.FeatureMesh.IsValid())
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Spawn static mesh actor for terrain feature
    AStaticMeshActor* FeatureActor = World->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        Feature.Location,
        Feature.Rotation
    );
    
    if (FeatureActor)
    {
        FeatureActor->GetStaticMeshComponent()->SetStaticMesh(Feature.FeatureMesh.LoadSynchronous());
        FeatureActor->SetActorScale3D(Feature.Scale);
        FeatureActor->SetActorLabel(TEXT("TerrainFeature"));
        
        GeneratedActors.Add(FeatureActor);
    }
}

void UWorld_BiomeManager::GenerateRiverSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("Generating river system..."));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create simple river connections between biomes
    TArray<FVector> RiverPoints = {
        FVector(-15000, -15000, -50), // Swampland
        FVector(-5000, -5000, -30),   // Transition
        FVector(0, 0, -20),           // Forest center
        FVector(8000, -2000, -25),    // Toward Savanna
        FVector(15000, -10000, -30)   // Savanna
    };
    
    // Spawn water plane actors to represent rivers
    for (int32 i = 0; i < RiverPoints.Num() - 1; ++i)
    {
        FVector StartPoint = RiverPoints[i];
        FVector EndPoint = RiverPoints[i + 1];
        FVector MidPoint = (StartPoint + EndPoint) * 0.5f;
        
        // Create water plane
        AStaticMeshActor* WaterActor = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            MidPoint,
            FRotator::ZeroRotator
        );
        
        if (WaterActor)
        {
            // Set scale to create river segment
            float Distance = FVector::Dist(StartPoint, EndPoint);
            WaterActor->SetActorScale3D(FVector(Distance / 100.0f, 2.0f, 0.1f));
            WaterActor->SetActorLabel(FString::Printf(TEXT("River_Segment_%d"), i));
            
            GeneratedActors.Add(WaterActor);
        }
    }
}

void UWorld_BiomeManager::PlaceVegetationInBiome(EWorld_BiomeType BiomeType, int32 Count)
{
    FWorld_BiomeData BiomeData = GetBiomeData(BiomeType);
    if (BiomeData.VegetationMeshes.Num() == 0)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    for (int32 i = 0; i < Count; ++i)
    {
        FVector SpawnLocation = GetRandomLocationInRadius(BiomeData.CenterLocation, BiomeData.Radius);
        
        // Random vegetation mesh from biome configuration
        int32 MeshIndex = FMath::RandRange(0, BiomeData.VegetationMeshes.Num() - 1);
        TSoftObjectPtr<UStaticMesh> VegMesh = BiomeData.VegetationMeshes[MeshIndex];
        
        if (VegMesh.IsValid())
        {
            AStaticMeshActor* VegetationActor = World->SpawnActor<AStaticMeshActor>(
                AStaticMeshActor::StaticClass(),
                SpawnLocation,
                FRotator(0, FMath::RandRange(0.0f, 360.0f), 0)
            );
            
            if (VegetationActor)
            {
                VegetationActor->GetStaticMeshComponent()->SetStaticMesh(VegMesh.LoadSynchronous());
                
                // Random scale variation
                float ScaleVariation = FMath::RandRange(0.8f, 1.2f);
                VegetationActor->SetActorScale3D(FVector(ScaleVariation));
                
                VegetationActor->SetActorLabel(FString::Printf(TEXT("Vegetation_%s_%d"), 
                    *UEnum::GetValueAsString(BiomeType), i));
                
                GeneratedActors.Add(VegetationActor);
            }
        }
    }
}

void UWorld_BiomeManager::ApplyBiomeWeather(EWorld_BiomeType BiomeType)
{
    // Weather effects would be implemented here
    // For now, just log the weather application
    UE_LOG(LogTemp, Warning, TEXT("Applying weather for biome: %s"), 
           *UEnum::GetValueAsString(BiomeType));
}

void UWorld_BiomeManager::UpdateDayNightCycle(float TimeOfDay)
{
    // Day/night cycle updates would be implemented here
    // This would affect lighting, temperature, and creature behavior
}

void UWorld_BiomeManager::RegenerateAllBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("Regenerating all biomes..."));
    InitializeBiomes();
}

void UWorld_BiomeManager::ClearAllGeneratedContent()
{
    UE_LOG(LogTemp, Warning, TEXT("Clearing all generated content..."));
    
    for (AActor* Actor : GeneratedActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    
    GeneratedActors.Empty();
}

void UWorld_BiomeManager::SetupDefaultBiomes()
{
    BiomeConfigurations.Empty();
    
    // Swampland configuration
    FWorld_BiomeData SwampBiome;
    SwampBiome.BiomeType = EWorld_BiomeType::Swampland;
    SwampBiome.Temperature = 28.0f;
    SwampBiome.Humidity = 95.0f;
    SwampBiome.Elevation = -50.0f;
    BiomeConfigurations.Add(SwampBiome);
    
    // Forest configuration
    FWorld_BiomeData ForestBiome;
    ForestBiome.BiomeType = EWorld_BiomeType::Forest;
    ForestBiome.Temperature = 22.0f;
    ForestBiome.Humidity = 70.0f;
    ForestBiome.Elevation = 0.0f;
    BiomeConfigurations.Add(ForestBiome);
    
    // Savanna configuration
    FWorld_BiomeData SavannaBiome;
    SavannaBiome.BiomeType = EWorld_BiomeType::Savanna;
    SavannaBiome.Temperature = 30.0f;
    SavannaBiome.Humidity = 40.0f;
    SavannaBiome.Elevation = 100.0f;
    BiomeConfigurations.Add(SavannaBiome);
    
    // Desert configuration
    FWorld_BiomeData DesertBiome;
    DesertBiome.BiomeType = EWorld_BiomeType::Desert;
    DesertBiome.Temperature = 40.0f;
    DesertBiome.Humidity = 15.0f;
    DesertBiome.Elevation = 200.0f;
    BiomeConfigurations.Add(DesertBiome);
    
    // Snowy Mountains configuration
    FWorld_BiomeData MountainBiome;
    MountainBiome.BiomeType = EWorld_BiomeType::SnowyMountains;
    MountainBiome.Temperature = -5.0f;
    MountainBiome.Humidity = 60.0f;
    MountainBiome.Elevation = 1000.0f;
    BiomeConfigurations.Add(MountainBiome);
}

void UWorld_BiomeManager::CreateSwamplandBiome(const FVector& Location, float Radius)
{
    UE_LOG(LogTemp, Warning, TEXT("Creating Swampland biome at %s"), *Location.ToString());
    // Swampland-specific terrain features would be created here
}

void UWorld_BiomeManager::CreateForestBiome(const FVector& Location, float Radius)
{
    UE_LOG(LogTemp, Warning, TEXT("Creating Forest biome at %s"), *Location.ToString());
    // Forest-specific terrain features would be created here
}

void UWorld_BiomeManager::CreateSavannaBiome(const FVector& Location, float Radius)
{
    UE_LOG(LogTemp, Warning, TEXT("Creating Savanna biome at %s"), *Location.ToString());
    // Savanna-specific terrain features would be created here
}

void UWorld_BiomeManager::CreateDesertBiome(const FVector& Location, float Radius)
{
    UE_LOG(LogTemp, Warning, TEXT("Creating Desert biome at %s"), *Location.ToString());
    // Desert-specific terrain features would be created here
}

void UWorld_BiomeManager::CreateSnowyMountainBiome(const FVector& Location, float Radius)
{
    UE_LOG(LogTemp, Warning, TEXT("Creating Snowy Mountain biome at %s"), *Location.ToString());
    // Mountain-specific terrain features would be created here
}

FVector UWorld_BiomeManager::GetRandomLocationInRadius(const FVector& Center, float Radius) const
{
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(0.0f, Radius);
    
    FVector Offset = FVector(
        FMath::Cos(RandomAngle) * RandomDistance,
        FMath::Sin(RandomAngle) * RandomDistance,
        0.0f
    );
    
    return Center + Offset;
}

float UWorld_BiomeManager::CalculateDistanceToBiomeCenter(const FVector& Location, const FWorld_BiomeData& Biome) const
{
    return FVector::Dist2D(Location, Biome.CenterLocation);
}