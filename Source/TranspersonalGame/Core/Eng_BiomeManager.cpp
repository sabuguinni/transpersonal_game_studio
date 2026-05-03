#include "Eng_BiomeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"

AEng_BiomeManager::AEng_BiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create visualization component
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    RootComponent = VisualizationMesh;

    // Set up visualization mesh (cube for now)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        VisualizationMesh->SetStaticMesh(CubeMeshAsset.Object);
        VisualizationMesh->SetWorldScale3D(FVector(10.0f, 10.0f, 10.0f));
    }

    // Initialize biome regions with default values
    SetupDefaultBiomeRegions();
}

void AEng_BiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Ensure biome regions are properly initialized
    InitializeBiomeRegions();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initialized with %d biome regions"), BiomeRegions.Num());
}

void AEng_BiomeManager::SetupDefaultBiomeRegions()
{
    BiomeRegions.Empty();

    // PANTANO (Swamp) - Sudoeste
    FEng_BiomeRegion PantanoRegion;
    PantanoRegion.BiomeType = EEng_BiomeType::Pantano;
    PantanoRegion.Center = FVector(-50000.0f, -45000.0f, 0.0f);
    PantanoRegion.MinBounds = FVector(-77500.0f, -76500.0f, -100.0f);
    PantanoRegion.MaxBounds = FVector(-25000.0f, -15000.0f, 200.0f);
    PantanoRegion.Temperature = 28.0f;
    PantanoRegion.Humidity = 85.0f;
    PantanoRegion.Elevation = -10.0f;
    BiomeRegions.Add(PantanoRegion);

    // FLORESTA (Forest) - Noroeste
    FEng_BiomeRegion FlorestaRegion;
    FlorestaRegion.BiomeType = EEng_BiomeType::Floresta;
    FlorestaRegion.Center = FVector(-45000.0f, 40000.0f, 0.0f);
    FlorestaRegion.MinBounds = FVector(-77500.0f, 15000.0f, -50.0f);
    FlorestaRegion.MaxBounds = FVector(-15000.0f, 76500.0f, 300.0f);
    FlorestaRegion.Temperature = 22.0f;
    FlorestaRegion.Humidity = 75.0f;
    FlorestaRegion.Elevation = 50.0f;
    BiomeRegions.Add(FlorestaRegion);

    // SAVANA (Savanna) - Centro
    FEng_BiomeRegion SavanaRegion;
    SavanaRegion.BiomeType = EEng_BiomeType::Savana;
    SavanaRegion.Center = FVector(0.0f, 0.0f, 0.0f);
    SavanaRegion.MinBounds = FVector(-20000.0f, -20000.0f, -50.0f);
    SavanaRegion.MaxBounds = FVector(20000.0f, 20000.0f, 150.0f);
    SavanaRegion.Temperature = 25.0f;
    SavanaRegion.Humidity = 45.0f;
    SavanaRegion.Elevation = 0.0f;
    BiomeRegions.Add(SavanaRegion);

    // DESERTO (Desert) - Leste
    FEng_BiomeRegion DesertoRegion;
    DesertoRegion.BiomeType = EEng_BiomeType::Deserto;
    DesertoRegion.Center = FVector(55000.0f, 0.0f, 0.0f);
    DesertoRegion.MinBounds = FVector(25000.0f, -30000.0f, -20.0f);
    DesertoRegion.MaxBounds = FVector(79500.0f, 30000.0f, 100.0f);
    DesertoRegion.Temperature = 35.0f;
    DesertoRegion.Humidity = 15.0f;
    DesertoRegion.Elevation = 20.0f;
    BiomeRegions.Add(DesertoRegion);

    // MONTANHA (Mountain) - Nordeste
    FEng_BiomeRegion MontanhaRegion;
    MontanhaRegion.BiomeType = EEng_BiomeType::Montanha;
    MontanhaRegion.Center = FVector(40000.0f, 50000.0f, 500.0f);
    MontanhaRegion.MinBounds = FVector(15000.0f, 20000.0f, 200.0f);
    MontanhaRegion.MaxBounds = FVector(79500.0f, 76500.0f, 1000.0f);
    MontanhaRegion.Temperature = 10.0f;
    MontanhaRegion.Humidity = 60.0f;
    MontanhaRegion.Elevation = 500.0f;
    BiomeRegions.Add(MontanhaRegion);
}

void AEng_BiomeManager::InitializeBiomeRegions()
{
    if (BiomeRegions.Num() == 0)
    {
        SetupDefaultBiomeRegions();
    }

    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Biome regions initialized:"));
    for (const FEng_BiomeRegion& Region : BiomeRegions)
    {
        FString BiomeName;
        switch (Region.BiomeType)
        {
            case EEng_BiomeType::Pantano: BiomeName = TEXT("PANTANO"); break;
            case EEng_BiomeType::Floresta: BiomeName = TEXT("FLORESTA"); break;
            case EEng_BiomeType::Savana: BiomeName = TEXT("SAVANA"); break;
            case EEng_BiomeType::Deserto: BiomeName = TEXT("DESERTO"); break;
            case EEng_BiomeType::Montanha: BiomeName = TEXT("MONTANHA"); break;
            default: BiomeName = TEXT("UNKNOWN"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("  %s: Center(%s) Temp:%.1f Humidity:%.1f"), 
            *BiomeName, *Region.Center.ToString(), Region.Temperature, Region.Humidity);
    }
}

EEng_BiomeType AEng_BiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float MinDistance = FLT_MAX;
    EEng_BiomeType ClosestBiome = EEng_BiomeType::Savana;

    for (const FEng_BiomeRegion& Region : BiomeRegions)
    {
        // Check if location is within biome bounds
        if (IsLocationInBiome(WorldLocation, Region.BiomeType))
        {
            return Region.BiomeType;
        }

        // If not within bounds, find closest biome
        float Distance = FVector::Dist(WorldLocation, Region.Center);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = Region.BiomeType;
        }
    }

    return ClosestBiome;
}

FVector AEng_BiomeManager::GetValidSpawnLocation(EEng_BiomeType BiomeType, float RandomRadius) const
{
    const FEng_BiomeRegion* TargetRegion = nullptr;
    
    for (const FEng_BiomeRegion& Region : BiomeRegions)
    {
        if (Region.BiomeType == BiomeType)
        {
            TargetRegion = &Region;
            break;
        }
    }

    if (!TargetRegion)
    {
        UE_LOG(LogTemp, Error, TEXT("BiomeManager: Invalid biome type requested"));
        return FVector::ZeroVector;
    }

    // Generate random offset within the specified radius
    FVector RandomOffset = FVector(
        FMath::RandRange(-RandomRadius, RandomRadius),
        FMath::RandRange(-RandomRadius, RandomRadius),
        0.0f
    );

    FVector ProposedLocation = TargetRegion->Center + RandomOffset;

    // Clamp to biome bounds
    ProposedLocation.X = FMath::Clamp(ProposedLocation.X, TargetRegion->MinBounds.X, TargetRegion->MaxBounds.X);
    ProposedLocation.Y = FMath::Clamp(ProposedLocation.Y, TargetRegion->MinBounds.Y, TargetRegion->MaxBounds.Y);
    ProposedLocation.Z = TargetRegion->Elevation;

    return ProposedLocation;
}

bool AEng_BiomeManager::IsLocationInBiome(const FVector& Location, EEng_BiomeType BiomeType) const
{
    const FEng_BiomeRegion* TargetRegion = nullptr;
    
    for (const FEng_BiomeRegion& Region : BiomeRegions)
    {
        if (Region.BiomeType == BiomeType)
        {
            TargetRegion = &Region;
            break;
        }
    }

    if (!TargetRegion)
    {
        return false;
    }

    return (Location.X >= TargetRegion->MinBounds.X && Location.X <= TargetRegion->MaxBounds.X &&
            Location.Y >= TargetRegion->MinBounds.Y && Location.Y <= TargetRegion->MaxBounds.Y);
}

FEng_BiomeRegion AEng_BiomeManager::GetBiomeRegion(EEng_BiomeType BiomeType) const
{
    for (const FEng_BiomeRegion& Region : BiomeRegions)
    {
        if (Region.BiomeType == BiomeType)
        {
            return Region;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("BiomeManager: Biome region not found, returning default"));
    return FEng_BiomeRegion();
}

FEng_SpawnParameters AEng_BiomeManager::CalculateSpawnParameters(EEng_BiomeType BiomeType, const FString& ActorType) const
{
    FEng_SpawnParameters SpawnParams;
    SpawnParams.TargetBiome = BiomeType;

    const FEng_BiomeRegion Region = GetBiomeRegion(BiomeType);

    // Adjust spawn parameters based on biome type and actor type
    if (ActorType.Contains(TEXT("Tree")) || ActorType.Contains(TEXT("Vegetation")))
    {
        switch (BiomeType)
        {
            case EEng_BiomeType::Floresta:
                SpawnParams.DensityMultiplier = 2.0f;
                SpawnParams.RandomRadius = 3000.0f;
                break;
            case EEng_BiomeType::Pantano:
                SpawnParams.DensityMultiplier = 1.5f;
                SpawnParams.RandomRadius = 4000.0f;
                break;
            case EEng_BiomeType::Deserto:
                SpawnParams.DensityMultiplier = 0.2f;
                SpawnParams.RandomRadius = 8000.0f;
                break;
            default:
                SpawnParams.DensityMultiplier = 1.0f;
                SpawnParams.RandomRadius = 5000.0f;
                break;
        }
    }
    else if (ActorType.Contains(TEXT("Dinosaur")) || ActorType.Contains(TEXT("Animal")))
    {
        switch (BiomeType)
        {
            case EEng_BiomeType::Savana:
                SpawnParams.DensityMultiplier = 1.5f;
                SpawnParams.RandomRadius = 6000.0f;
                break;
            case EEng_BiomeType::Floresta:
                SpawnParams.DensityMultiplier = 1.2f;
                SpawnParams.RandomRadius = 4000.0f;
                break;
            default:
                SpawnParams.DensityMultiplier = 0.8f;
                SpawnParams.RandomRadius = 5000.0f;
                break;
        }
    }

    SpawnParams.ElevationOffset = Region.Elevation;
    return SpawnParams;
}

bool AEng_BiomeManager::ValidateSpawnLocation(const FVector& Location, const FString& ActorType) const
{
    // Check if location is within overall map bounds
    if (!IsWithinMapBounds(Location))
    {
        return false;
    }

    // Get biome at location
    EEng_BiomeType BiomeAtLocation = GetBiomeAtLocation(Location);
    
    // Validate actor type compatibility with biome
    if (ActorType.Contains(TEXT("Snow")) && BiomeAtLocation != EEng_BiomeType::Montanha)
    {
        return false;
    }
    
    if (ActorType.Contains(TEXT("Cactus")) && BiomeAtLocation != EEng_BiomeType::Deserto)
    {
        return false;
    }
    
    if (ActorType.Contains(TEXT("Swamp")) && BiomeAtLocation != EEng_BiomeType::Pantano)
    {
        return false;
    }

    return true;
}

void AEng_BiomeManager::ValidateWorldArchitecture()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("BiomeManager: No world available for validation"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Starting world architecture validation..."));

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    int32 ValidActors = 0;
    int32 InvalidActors = 0;

    for (AActor* Actor : AllActors)
    {
        if (!Actor || Actor == this)
        {
            continue;
        }

        FVector ActorLocation = Actor->GetActorLocation();
        FString ActorType = Actor->GetClass()->GetName();

        if (ValidateSpawnLocation(ActorLocation, ActorType))
        {
            ValidActors++;
        }
        else
        {
            InvalidActors++;
            UE_LOG(LogTemp, Warning, TEXT("Invalid actor placement: %s at %s"), 
                *ActorType, *ActorLocation.ToString());
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Architecture validation complete: %d valid, %d invalid actors"), 
        ValidActors, InvalidActors);
}

void AEng_BiomeManager::GenerateArchitecturalReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME ARCHITECTURE REPORT ==="));
    
    for (const FEng_BiomeRegion& Region : BiomeRegions)
    {
        FString BiomeName;
        switch (Region.BiomeType)
        {
            case EEng_BiomeType::Pantano: BiomeName = TEXT("PANTANO"); break;
            case EEng_BiomeType::Floresta: BiomeName = TEXT("FLORESTA"); break;
            case EEng_BiomeType::Savana: BiomeName = TEXT("SAVANA"); break;
            case EEng_BiomeType::Deserto: BiomeName = TEXT("DESERTO"); break;
            case EEng_BiomeType::Montanha: BiomeName = TEXT("MONTANHA"); break;
        }

        UE_LOG(LogTemp, Warning, TEXT("%s:"), *BiomeName);
        UE_LOG(LogTemp, Warning, TEXT("  Center: %s"), *Region.Center.ToString());
        UE_LOG(LogTemp, Warning, TEXT("  Bounds: %s to %s"), 
            *Region.MinBounds.ToString(), *Region.MaxBounds.ToString());
        UE_LOG(LogTemp, Warning, TEXT("  Environment: Temp=%.1f°C, Humidity=%.1f%%, Elevation=%.1fm"), 
            Region.Temperature, Region.Humidity, Region.Elevation);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

float AEng_BiomeManager::CalculateDistanceToNearestBiome(const FVector& Location) const
{
    float MinDistance = FLT_MAX;
    
    for (const FEng_BiomeRegion& Region : BiomeRegions)
    {
        float Distance = FVector::Dist(Location, Region.Center);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
        }
    }
    
    return MinDistance;
}

bool AEng_BiomeManager::IsWithinMapBounds(const FVector& Location) const
{
    // Map bounds: 157,000 x 153,000 UU, centered at (0,0,0)
    // X range: -77,500 to +79,500 | Y range: -76,500 to +76,500
    return (Location.X >= -77500.0f && Location.X <= 79500.0f &&
            Location.Y >= -76500.0f && Location.Y <= 76500.0f);
}