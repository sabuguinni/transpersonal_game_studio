#include "World_BiomeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

AWorld_BiomeManager::AWorld_BiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create visualization mesh component
    BiomeVisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BiomeVisualizationMesh"));
    RootComponent = BiomeVisualizationMesh;

    // Load a basic mesh for visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMesh.Succeeded())
    {
        BiomeVisualizationMesh->SetStaticMesh(SphereMesh.Object);
        BiomeVisualizationMesh->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
    }

    SetupDefaultBiomes();
}

void AWorld_BiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomes();
    CreateBiomeMarkers();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initialized with %d biome zones"), BiomeZones.Num());
}

void AWorld_BiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWorld_BiomeManager::SetupDefaultBiomes()
{
    BiomeZones.Empty();

    // Forest Biome
    FWorld_BiomeData ForestBiome;
    ForestBiome.BiomeType = EWorld_BiomeType::Forest;
    ForestBiome.CenterLocation = FVector(-2000.0f, -2000.0f, 100.0f);
    ForestBiome.Radius = 1500.0f;
    ForestBiome.VegetationDensity = 0.8f;
    ForestBiome.WaterPresence = 0.4f;
    ForestBiome.TerrainRoughness = 0.3f;
    BiomeZones.Add(ForestBiome);

    // Savanna Biome
    FWorld_BiomeData SavannaBiome;
    SavannaBiome.BiomeType = EWorld_BiomeType::Savanna;
    SavannaBiome.CenterLocation = FVector(2000.0f, -2000.0f, 100.0f);
    SavannaBiome.Radius = 1800.0f;
    SavannaBiome.VegetationDensity = 0.3f;
    SavannaBiome.WaterPresence = 0.1f;
    SavannaBiome.TerrainRoughness = 0.2f;
    BiomeZones.Add(SavannaBiome);

    // Mountain Biome
    FWorld_BiomeData MountainBiome;
    MountainBiome.BiomeType = EWorld_BiomeType::Mountain;
    MountainBiome.CenterLocation = FVector(0.0f, 2000.0f, 500.0f);
    MountainBiome.Radius = 1200.0f;
    MountainBiome.VegetationDensity = 0.2f;
    MountainBiome.WaterPresence = 0.6f;
    MountainBiome.TerrainRoughness = 0.9f;
    BiomeZones.Add(MountainBiome);

    // Swamp Biome
    FWorld_BiomeData SwampBiome;
    SwampBiome.BiomeType = EWorld_BiomeType::Swamp;
    SwampBiome.CenterLocation = FVector(-1500.0f, 1500.0f, 50.0f);
    SwampBiome.Radius = 1000.0f;
    SwampBiome.VegetationDensity = 0.6f;
    SwampBiome.WaterPresence = 0.9f;
    SwampBiome.TerrainRoughness = 0.1f;
    BiomeZones.Add(SwampBiome);

    // Desert Biome
    FWorld_BiomeData DesertBiome;
    DesertBiome.BiomeType = EWorld_BiomeType::Desert;
    DesertBiome.CenterLocation = FVector(3000.0f, 1000.0f, 200.0f);
    DesertBiome.Radius = 2000.0f;
    DesertBiome.VegetationDensity = 0.05f;
    DesertBiome.WaterPresence = 0.02f;
    DesertBiome.TerrainRoughness = 0.4f;
    BiomeZones.Add(DesertBiome);
}

EWorld_BiomeType AWorld_BiomeManager::GetBiomeAtLocation(const FVector& Location) const
{
    float ClosestDistance = FLT_MAX;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::Forest;

    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        float Distance = CalculateDistanceToBiome(Location, Biome);
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBiome = Biome.BiomeType;
        }
    }

    return ClosestBiome;
}

void AWorld_BiomeManager::InitializeBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initializing %d biomes"), BiomeZones.Num());
    
    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        FString BiomeName = UEnum::GetValueAsString(Biome.BiomeType);
        UE_LOG(LogTemp, Warning, TEXT("Biome: %s at location %s with radius %.1f"), 
               *BiomeName, *Biome.CenterLocation.ToString(), Biome.Radius);
    }
}

void AWorld_BiomeManager::CreateBiomeMarkers()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        // Create a marker actor for each biome
        AStaticMeshActor* MarkerActor = World->SpawnActor<AStaticMeshActor>();
        if (MarkerActor)
        {
            MarkerActor->SetActorLocation(Biome.CenterLocation);
            
            FString BiomeName = UEnum::GetValueAsString(Biome.BiomeType);
            MarkerActor->SetActorLabel(FString::Printf(TEXT("BiomeMarker_%s"), *BiomeName));
            
            UStaticMeshComponent* MeshComp = MarkerActor->GetStaticMeshComponent();
            if (MeshComp)
            {
                UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
                if (CubeMesh)
                {
                    MeshComp->SetStaticMesh(CubeMesh);
                    
                    // Scale based on biome radius
                    float ScaleFactor = Biome.Radius / 1000.0f;
                    MeshComp->SetWorldScale3D(FVector(ScaleFactor, ScaleFactor, 2.0f));
                }
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Created biome marker for %s"), *BiomeName);
        }
    }
}

float AWorld_BiomeManager::GetVegetationDensityAtLocation(const FVector& Location) const
{
    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        float Distance = CalculateDistanceToBiome(Location, Biome);
        if (Distance <= Biome.Radius)
        {
            // Interpolate density based on distance to center
            float DistanceRatio = Distance / Biome.Radius;
            return Biome.VegetationDensity * (1.0f - DistanceRatio * 0.5f);
        }
    }
    
    return 0.1f; // Default low density
}

float AWorld_BiomeManager::GetWaterPresenceAtLocation(const FVector& Location) const
{
    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        float Distance = CalculateDistanceToBiome(Location, Biome);
        if (Distance <= Biome.Radius)
        {
            return Biome.WaterPresence;
        }
    }
    
    return 0.0f; // No water by default
}

void AWorld_BiomeManager::DebugShowBiomeInfo()
{
    if (GEngine)
    {
        for (int32 i = 0; i < BiomeZones.Num(); ++i)
        {
            const FWorld_BiomeData& Biome = BiomeZones[i];
            FString BiomeName = UEnum::GetValueAsString(Biome.BiomeType);
            FString DebugMessage = FString::Printf(
                TEXT("Biome %d: %s | Center: %s | Radius: %.1f | Vegetation: %.2f | Water: %.2f"),
                i, *BiomeName, *Biome.CenterLocation.ToString(), 
                Biome.Radius, Biome.VegetationDensity, Biome.WaterPresence
            );
            
            GEngine->AddOnScreenDebugMessage(i, 10.0f, FColor::Green, DebugMessage);
        }
    }
}

float AWorld_BiomeManager::CalculateDistanceToBiome(const FVector& Location, const FWorld_BiomeData& Biome) const
{
    return FVector::Dist(Location, Biome.CenterLocation);
}

// Biome Subsystem Implementation
void UWorld_BiomeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeSubsystem: Initialized"));
}

void UWorld_BiomeSubsystem::Deinitialize()
{
    BiomeManagerInstance = nullptr;
    Super::Deinitialize();
}

AWorld_BiomeManager* UWorld_BiomeSubsystem::GetBiomeManager() const
{
    if (!BiomeManagerInstance)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            // Find existing biome manager or create one
            for (TActorIterator<AWorld_BiomeManager> ActorItr(World); ActorItr; ++ActorItr)
            {
                const_cast<UWorld_BiomeSubsystem*>(this)->BiomeManagerInstance = *ActorItr;
                break;
            }
            
            if (!BiomeManagerInstance)
            {
                const_cast<UWorld_BiomeSubsystem*>(this)->BiomeManagerInstance = World->SpawnActor<AWorld_BiomeManager>();
            }
        }
    }
    
    return BiomeManagerInstance;
}

EWorld_BiomeType UWorld_BiomeSubsystem::GetCurrentPlayerBiome() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return EWorld_BiomeType::Forest;
    }
    
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return EWorld_BiomeType::Forest;
    }
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    AWorld_BiomeManager* BiomeManager = GetBiomeManager();
    
    if (BiomeManager)
    {
        return BiomeManager->GetBiomeAtLocation(PlayerLocation);
    }
    
    return EWorld_BiomeType::Forest;
}