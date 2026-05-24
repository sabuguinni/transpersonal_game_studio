#include "World_BiomeGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AWorld_BiomeGenerator::AWorld_BiomeGenerator()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    bAutoGenerateOnPlay = true;
    RandomSeed = 12345;

    // Initialize default biome configurations
    InitializeDefaultBiomes();
}

void AWorld_BiomeGenerator::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoGenerateOnPlay)
    {
        // Delay generation to ensure world is fully loaded
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AWorld_BiomeGenerator::GenerateAllBiomes, 1.0f, false);
    }
}

void AWorld_BiomeGenerator::InitializeDefaultBiomes()
{
    BiomeConfigurations.Empty();

    // Swamp Biome - Southwest
    FWorld_BiomeConfig SwampBiome;
    SwampBiome.BiomeType = EWorld_BiomeType::Swamp;
    SwampBiome.CenterLocation = FVector(-3000.0f, -3000.0f, 0.0f);
    SwampBiome.Radius = 2500.0f;
    SwampBiome.VegetationDensity = 80;
    SwampBiome.GroundColor = FLinearColor(0.2f, 0.4f, 0.1f, 1.0f);
    SwampBiome.TemperatureRange = 28.0f;
    SwampBiome.HumidityLevel = 0.9f;
    BiomeConfigurations.Add(SwampBiome);

    // Forest Biome - Northwest
    FWorld_BiomeConfig ForestBiome;
    ForestBiome.BiomeType = EWorld_BiomeType::Forest;
    ForestBiome.CenterLocation = FVector(-3000.0f, 3000.0f, 0.0f);
    ForestBiome.Radius = 2500.0f;
    ForestBiome.VegetationDensity = 100;
    ForestBiome.GroundColor = FLinearColor(0.1f, 0.6f, 0.1f, 1.0f);
    ForestBiome.TemperatureRange = 22.0f;
    ForestBiome.HumidityLevel = 0.7f;
    BiomeConfigurations.Add(ForestBiome);

    // Savanna Biome - Center
    FWorld_BiomeConfig SavannaBiome;
    SavannaBiome.BiomeType = EWorld_BiomeType::Savanna;
    SavannaBiome.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
    SavannaBiome.Radius = 2000.0f;
    SavannaBiome.VegetationDensity = 30;
    SavannaBiome.GroundColor = FLinearColor(0.8f, 0.6f, 0.2f, 1.0f);
    SavannaBiome.TemperatureRange = 32.0f;
    SavannaBiome.HumidityLevel = 0.4f;
    BiomeConfigurations.Add(SavannaBiome);

    // Desert Biome - Southeast
    FWorld_BiomeConfig DesertBiome;
    DesertBiome.BiomeType = EWorld_BiomeType::Desert;
    DesertBiome.CenterLocation = FVector(3000.0f, -3000.0f, 0.0f);
    DesertBiome.Radius = 2500.0f;
    DesertBiome.VegetationDensity = 10;
    DesertBiome.GroundColor = FLinearColor(0.9f, 0.7f, 0.3f, 1.0f);
    DesertBiome.TemperatureRange = 45.0f;
    DesertBiome.HumidityLevel = 0.1f;
    BiomeConfigurations.Add(DesertBiome);

    // Snow Mountain Biome - Northeast
    FWorld_BiomeConfig SnowBiome;
    SnowBiome.BiomeType = EWorld_BiomeType::SnowMountain;
    SnowBiome.CenterLocation = FVector(3000.0f, 3000.0f, 500.0f);
    SnowBiome.Radius = 2000.0f;
    SnowBiome.VegetationDensity = 20;
    SnowBiome.GroundColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);
    SnowBiome.TemperatureRange = -5.0f;
    SnowBiome.HumidityLevel = 0.3f;
    BiomeConfigurations.Add(SnowBiome);
}

void AWorld_BiomeGenerator::GenerateAllBiomes()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("World_BiomeGenerator: No valid world found"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("World_BiomeGenerator: Starting biome generation with %d biomes"), BiomeConfigurations.Num());

    // Clear previous generation
    ClearAllGeneration();

    // Set random seed for consistent generation
    FMath::RandInit(RandomSeed);

    // Generate each biome
    for (const FWorld_BiomeConfig& BiomeConfig : BiomeConfigurations)
    {
        GenerateBiome(BiomeConfig);
    }

    // Create water features
    CreateWaterFeatures();

    // Setup biome transitions
    SetupBiomeTransitions();

    UE_LOG(LogTemp, Warning, TEXT("World_BiomeGenerator: Biome generation complete"));
}

void AWorld_BiomeGenerator::GenerateBiome(const FWorld_BiomeConfig& BiomeConfig)
{
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeGenerator: Generating biome type %d at location %s"), 
           (int32)BiomeConfig.BiomeType, *BiomeConfig.CenterLocation.ToString());

    // Spawn vegetation for this biome
    SpawnVegetationInBiome(BiomeConfig);

    // Create biome marker (visual indicator)
    if (UWorld* World = GetWorld())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AStaticMeshActor* BiomeMarker = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            BiomeConfig.CenterLocation,
            FRotator::ZeroRotator,
            SpawnParams
        );

        if (BiomeMarker)
        {
            // Set a basic cube mesh as marker
            UStaticMeshComponent* MeshComp = BiomeMarker->GetStaticMeshComponent();
            if (MeshComp)
            {
                // Use engine default cube
                UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
                if (CubeMesh)
                {
                    MeshComp->SetStaticMesh(CubeMesh);
                    MeshComp->SetWorldScale3D(FVector(0.5f, 0.5f, 0.1f)); // Flat marker
                }
            }

            // Set actor label for easy identification
            FString BiomeName = UEnum::GetValueAsString(BiomeConfig.BiomeType);
            BiomeMarker->SetActorLabel(FString::Printf(TEXT("BiomeMarker_%s"), *BiomeName));
        }
    }
}

void AWorld_BiomeGenerator::SpawnVegetationInBiome(const FWorld_BiomeConfig& BiomeConfig)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    int32 VegetationCount = BiomeConfig.VegetationDensity;
    
    for (int32 i = 0; i < VegetationCount; i++)
    {
        FVector SpawnLocation = GenerateRandomLocationInRadius(BiomeConfig.CenterLocation, BiomeConfig.Radius);
        
        if (IsLocationValidForVegetation(SpawnLocation))
        {
            FWorld_VegetationSpawn VegSpawn;
            VegSpawn.Location = SpawnLocation;
            VegSpawn.Rotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
            VegSpawn.Scale = FVector(FMath::RandRange(0.8f, 1.5f));
            VegSpawn.SourceBiome = BiomeConfig.BiomeType;

            SpawnVegetationActor(VegSpawn);
            GeneratedVegetation.Add(VegSpawn);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("World_BiomeGenerator: Spawned %d vegetation actors for biome %d"), 
           VegetationCount, (int32)BiomeConfig.BiomeType);
}

void AWorld_BiomeGenerator::SpawnVegetationActor(const FWorld_VegetationSpawn& SpawnData)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

    AStaticMeshActor* VegetationActor = World->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        SpawnData.Location,
        SpawnData.Rotation,
        SpawnParams
    );

    if (VegetationActor)
    {
        UStaticMeshComponent* MeshComp = VegetationActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            // Use different shapes based on biome type
            UStaticMesh* VegMesh = nullptr;
            FString ActorLabel;

            switch (SpawnData.SourceBiome)
            {
                case EWorld_BiomeType::Forest:
                    VegMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
                    ActorLabel = TEXT("ForestTree");
                    break;
                case EWorld_BiomeType::Swamp:
                    VegMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone"));
                    ActorLabel = TEXT("SwampTree");
                    break;
                case EWorld_BiomeType::Desert:
                    VegMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
                    ActorLabel = TEXT("DesertCactus");
                    break;
                case EWorld_BiomeType::Savanna:
                    VegMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
                    ActorLabel = TEXT("SavannaGrass");
                    break;
                case EWorld_BiomeType::SnowMountain:
                    VegMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone"));
                    ActorLabel = TEXT("SnowPine");
                    break;
            }

            if (VegMesh)
            {
                MeshComp->SetStaticMesh(VegMesh);
                MeshComp->SetWorldScale3D(SpawnData.Scale);
            }

            VegetationActor->SetActorLabel(FString::Printf(TEXT("%s_%d"), *ActorLabel, GeneratedVegetation.Num()));
        }
    }
}

void AWorld_BiomeGenerator::CreateWaterFeatures()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Create a river between biomes
    TArray<FVector> RiverPoints = {
        FVector(-2000.0f, 0.0f, -50.0f),
        FVector(-1000.0f, 500.0f, -50.0f),
        FVector(0.0f, 1000.0f, -50.0f),
        FVector(1000.0f, 500.0f, -50.0f),
        FVector(2000.0f, 0.0f, -50.0f)
    };

    for (int32 i = 0; i < RiverPoints.Num(); i++)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AStaticMeshActor* WaterSegment = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            RiverPoints[i],
            FRotator::ZeroRotator,
            SpawnParams
        );

        if (WaterSegment)
        {
            UStaticMeshComponent* MeshComp = WaterSegment->GetStaticMeshComponent();
            if (MeshComp)
            {
                UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane"));
                if (PlaneMesh)
                {
                    MeshComp->SetStaticMesh(PlaneMesh);
                    MeshComp->SetWorldScale3D(FVector(5.0f, 5.0f, 1.0f));
                }
            }

            WaterSegment->SetActorLabel(FString::Printf(TEXT("RiverSegment_%d"), i));
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("World_BiomeGenerator: Created river with %d segments"), RiverPoints.Num());
}

void AWorld_BiomeGenerator::SetupBiomeTransitions()
{
    // Create transition zones between biomes
    for (int32 i = 0; i < BiomeConfigurations.Num(); i++)
    {
        for (int32 j = i + 1; j < BiomeConfigurations.Num(); j++)
        {
            const FWorld_BiomeConfig& BiomeA = BiomeConfigurations[i];
            const FWorld_BiomeConfig& BiomeB = BiomeConfigurations[j];

            float Distance = FVector::Dist(BiomeA.CenterLocation, BiomeB.CenterLocation);
            float CombinedRadius = BiomeA.Radius + BiomeB.Radius;

            // If biomes are close enough, create transition zone
            if (Distance < CombinedRadius * 0.8f)
            {
                FVector TransitionCenter = (BiomeA.CenterLocation + BiomeB.CenterLocation) * 0.5f;
                
                // Spawn transition marker
                if (UWorld* World = GetWorld())
                {
                    FActorSpawnParameters SpawnParams;
                    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

                    AStaticMeshActor* TransitionMarker = World->SpawnActor<AStaticMeshActor>(
                        AStaticMeshActor::StaticClass(),
                        TransitionCenter,
                        FRotator::ZeroRotator,
                        SpawnParams
                    );

                    if (TransitionMarker)
                    {
                        UStaticMeshComponent* MeshComp = TransitionMarker->GetStaticMeshComponent();
                        if (MeshComp)
                        {
                            UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
                            if (SphereMesh)
                            {
                                MeshComp->SetStaticMesh(SphereMesh);
                                MeshComp->SetWorldScale3D(FVector(0.3f));
                            }
                        }

                        FString BiomeAName = UEnum::GetValueAsString(BiomeA.BiomeType);
                        FString BiomeBName = UEnum::GetValueAsString(BiomeB.BiomeType);
                        TransitionMarker->SetActorLabel(FString::Printf(TEXT("Transition_%s_%s"), *BiomeAName, *BiomeBName));
                    }
                }
            }
        }
    }
}

void AWorld_BiomeGenerator::ClearAllGeneration()
{
    GeneratedVegetation.Empty();

    // Clear existing generated actors
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> ActorsToDestroy;
        
        // Find all actors with our naming convention
        for (TActorIterator<AStaticMeshActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AStaticMeshActor* Actor = *ActorIterator;
            if (Actor && Actor->GetOwner() == this)
            {
                ActorsToDestroy.Add(Actor);
            }
        }

        // Destroy the actors
        for (AActor* Actor : ActorsToDestroy)
        {
            Actor->Destroy();
        }

        UE_LOG(LogTemp, Warning, TEXT("World_BiomeGenerator: Cleared %d generated actors"), ActorsToDestroy.Num());
    }
}

FVector AWorld_BiomeGenerator::GenerateRandomLocationInRadius(const FVector& Center, float Radius) const
{
    float Angle = FMath::RandRange(0.0f, 2.0f * PI);
    float Distance = FMath::RandRange(0.0f, Radius);
    
    FVector RandomOffset = FVector(
        FMath::Cos(Angle) * Distance,
        FMath::Sin(Angle) * Distance,
        FMath::RandRange(-50.0f, 50.0f) // Some height variation
    );

    return Center + RandomOffset;
}

bool AWorld_BiomeGenerator::IsLocationValidForVegetation(const FVector& Location) const
{
    // Simple validation - avoid spawning too close to water level
    return Location.Z > -100.0f;
}

FVector AWorld_BiomeGenerator::GetBiomeCenter(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeConfig& Config : BiomeConfigurations)
    {
        if (Config.BiomeType == BiomeType)
        {
            return Config.CenterLocation;
        }
    }
    return FVector::ZeroVector;
}

EWorld_BiomeType AWorld_BiomeGenerator::GetBiomeAtLocation(const FVector& Location) const
{
    float ClosestDistance = MAX_FLT;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::Forest;

    for (const FWorld_BiomeConfig& Config : BiomeConfigurations)
    {
        float Distance = FVector::Dist(Location, Config.CenterLocation);
        if (Distance < Config.Radius && Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBiome = Config.BiomeType;
        }
    }

    return ClosestBiome;
}