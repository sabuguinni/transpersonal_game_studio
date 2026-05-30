#include "Eng_BiomeManager.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Math/UnrealMathUtility.h"

AEng_BiomeManager::AEng_BiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    bAutoInitializeBiomes = true;
    bDebugVisualization = false;
}

void AEng_BiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoInitializeBiomes)
    {
        InitializeDefaultBiomes();
    }
    
    if (bDebugVisualization)
    {
        CreateBiomeVisualizationActors();
    }
}

void AEng_BiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bDebugVisualization && GetWorld())
    {
        for (const FEng_BiomeData& Biome : BiomeDefinitions)
        {
            DrawDebugSphere(GetWorld(), Biome.BiomeCenter, Biome.BiomeRadius, 32, FColor::Green, false, -1.0f, 0, 100.0f);
        }
    }
}

void AEng_BiomeManager::InitializeDefaultBiomes()
{
    BiomeDefinitions.Empty();
    SetupDefaultBiomeData();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initialized %d default biomes"), BiomeDefinitions.Num());
}

void AEng_BiomeManager::SetupDefaultBiomeData()
{
    // Savanna Biome (0, 0)
    FEng_BiomeData Savanna;
    Savanna.BiomeName = TEXT("Savanna");
    Savanna.BiomeCenter = FVector(0.0f, 0.0f, 100.0f);
    Savanna.BiomeRadius = 15000.0f;
    Savanna.BiomeType = EBiomeType::Savanna;
    Savanna.Temperature = 28.0f;
    Savanna.Humidity = 40.0f;
    Savanna.AllowedDinosaurTypes.Add(TEXT("TRex"));
    Savanna.AllowedDinosaurTypes.Add(TEXT("Triceratops"));
    Savanna.AllowedDinosaurTypes.Add(TEXT("Velociraptor"));
    BiomeDefinitions.Add(Savanna);
    
    // Swamp Biome (-50000, -45000)
    FEng_BiomeData Swamp;
    Swamp.BiomeName = TEXT("Swamp");
    Swamp.BiomeCenter = FVector(-50000.0f, -45000.0f, 100.0f);
    Swamp.BiomeRadius = 15000.0f;
    Swamp.BiomeType = EBiomeType::Swamp;
    Swamp.Temperature = 26.0f;
    Swamp.Humidity = 85.0f;
    Swamp.AllowedDinosaurTypes.Add(TEXT("Brachiosaurus"));
    Swamp.AllowedDinosaurTypes.Add(TEXT("Parasaurolophus"));
    Swamp.AllowedDinosaurTypes.Add(TEXT("Triceratops"));
    BiomeDefinitions.Add(Swamp);
    
    // Forest Biome (-45000, 40000)
    FEng_BiomeData Forest;
    Forest.BiomeName = TEXT("Forest");
    Forest.BiomeCenter = FVector(-45000.0f, 40000.0f, 100.0f);
    Forest.BiomeRadius = 15000.0f;
    Forest.BiomeType = EBiomeType::Forest;
    Forest.Temperature = 22.0f;
    Forest.Humidity = 70.0f;
    Forest.AllowedDinosaurTypes.Add(TEXT("Brachiosaurus"));
    Forest.AllowedDinosaurTypes.Add(TEXT("Velociraptor"));
    Forest.AllowedDinosaurTypes.Add(TEXT("Ankylosaurus"));
    BiomeDefinitions.Add(Forest);
    
    // Desert Biome (55000, 0)
    FEng_BiomeData Desert;
    Desert.BiomeName = TEXT("Desert");
    Desert.BiomeCenter = FVector(55000.0f, 0.0f, 100.0f);
    Desert.BiomeRadius = 15000.0f;
    Desert.BiomeType = EBiomeType::Desert;
    Desert.Temperature = 35.0f;
    Desert.Humidity = 15.0f;
    Desert.AllowedDinosaurTypes.Add(TEXT("TRex"));
    Desert.AllowedDinosaurTypes.Add(TEXT("Protoceratops"));
    Desert.AllowedDinosaurTypes.Add(TEXT("Pachycephalo"));
    BiomeDefinitions.Add(Desert);
    
    // Mountain Biome (40000, 50000)
    FEng_BiomeData Mountain;
    Mountain.BiomeName = TEXT("Mountain");
    Mountain.BiomeCenter = FVector(40000.0f, 50000.0f, 100.0f);
    Mountain.BiomeRadius = 15000.0f;
    Mountain.BiomeType = EBiomeType::Mountain;
    Mountain.Temperature = 18.0f;
    Mountain.Humidity = 60.0f;
    Mountain.AllowedDinosaurTypes.Add(TEXT("Ankylosaurus"));
    Mountain.AllowedDinosaurTypes.Add(TEXT("Tsintaosaurus"));
    Mountain.AllowedDinosaurTypes.Add(TEXT("Velociraptor"));
    BiomeDefinitions.Add(Mountain);
}

EBiomeType AEng_BiomeManager::GetBiomeTypeAtLocation(const FVector& Location) const
{
    float MinDistance = MAX_FLT;
    EBiomeType ClosestBiome = EBiomeType::Savanna;
    
    for (const FEng_BiomeData& Biome : BiomeDefinitions)
    {
        float Distance = FVector::Dist(Location, Biome.BiomeCenter);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = Biome.BiomeType;
        }
    }
    
    return ClosestBiome;
}

FEng_BiomeData AEng_BiomeManager::GetBiomeDataAtLocation(const FVector& Location) const
{
    EBiomeType BiomeType = GetBiomeTypeAtLocation(Location);
    
    for (const FEng_BiomeData& Biome : BiomeDefinitions)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome;
        }
    }
    
    return FEng_BiomeData();
}

FVector AEng_BiomeManager::GetRandomLocationInBiome(EBiomeType BiomeType) const
{
    for (const FEng_BiomeData& Biome : BiomeDefinitions)
    {
        if (Biome.BiomeType == BiomeType)
        {
            float RandomX = FMath::RandRange(-Biome.BiomeRadius, Biome.BiomeRadius);
            float RandomY = FMath::RandRange(-Biome.BiomeRadius, Biome.BiomeRadius);
            return Biome.BiomeCenter + FVector(RandomX, RandomY, 0.0f);
        }
    }
    
    return FVector::ZeroVector;
}

TArray<FVector> AEng_BiomeManager::GetDistributedSpawnLocations(int32 TotalCount) const
{
    TArray<FVector> SpawnLocations;
    
    if (BiomeDefinitions.Num() == 0)
    {
        return SpawnLocations;
    }
    
    int32 CountPerBiome = TotalCount / BiomeDefinitions.Num();
    int32 Remainder = TotalCount % BiomeDefinitions.Num();
    
    for (int32 BiomeIndex = 0; BiomeIndex < BiomeDefinitions.Num(); BiomeIndex++)
    {
        const FEng_BiomeData& Biome = BiomeDefinitions[BiomeIndex];
        int32 SpawnsForThisBiome = CountPerBiome + (BiomeIndex < Remainder ? 1 : 0);
        
        for (int32 SpawnIndex = 0; SpawnIndex < SpawnsForThisBiome; SpawnIndex++)
        {
            FVector SpawnLocation = GetRandomLocationInBiome(Biome.BiomeType);
            SpawnLocations.Add(SpawnLocation);
        }
    }
    
    return SpawnLocations;
}

bool AEng_BiomeManager::IsDinosaurAllowedInBiome(const FString& DinosaurType, EBiomeType BiomeType) const
{
    for (const FEng_BiomeData& Biome : BiomeDefinitions)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome.AllowedDinosaurTypes.Contains(DinosaurType);
        }
    }
    
    return false;
}

void AEng_BiomeManager::AddBiome(const FEng_BiomeData& NewBiome)
{
    BiomeDefinitions.Add(NewBiome);
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Added biome %s"), *NewBiome.BiomeName);
}

void AEng_BiomeManager::RemoveBiome(EBiomeType BiomeType)
{
    int32 Index = FindBiomeIndex(BiomeType);
    if (Index != INDEX_NONE)
    {
        BiomeDefinitions.RemoveAt(Index);
        UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Removed biome type %d"), (int32)BiomeType);
    }
}

void AEng_BiomeManager::UpdateBiomeData(EBiomeType BiomeType, const FEng_BiomeData& UpdatedData)
{
    int32 Index = FindBiomeIndex(BiomeType);
    if (Index != INDEX_NONE)
    {
        BiomeDefinitions[Index] = UpdatedData;
        UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Updated biome %s"), *UpdatedData.BiomeName);
    }
}

void AEng_BiomeManager::DebugPrintBiomeInfo()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME MANAGER DEBUG INFO ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Biomes: %d"), BiomeDefinitions.Num());
    
    for (int32 i = 0; i < BiomeDefinitions.Num(); i++)
    {
        const FEng_BiomeData& Biome = BiomeDefinitions[i];
        UE_LOG(LogTemp, Warning, TEXT("Biome %d: %s at (%f, %f, %f) - Radius: %f"), 
               i, *Biome.BiomeName, Biome.BiomeCenter.X, Biome.BiomeCenter.Y, Biome.BiomeCenter.Z, Biome.BiomeRadius);
        UE_LOG(LogTemp, Warning, TEXT("  Temperature: %f, Humidity: %f"), Biome.Temperature, Biome.Humidity);
        UE_LOG(LogTemp, Warning, TEXT("  Allowed Dinosaurs: %d"), Biome.AllowedDinosaurTypes.Num());
    }
}

void AEng_BiomeManager::CreateBiomeVisualizationActors()
{
    if (!GetWorld())
    {
        return;
    }
    
    for (const FEng_BiomeData& Biome : BiomeDefinitions)
    {
        AActor* VisualizationActor = GetWorld()->SpawnActor<AActor>();
        if (VisualizationActor)
        {
            VisualizationActor->SetActorLocation(Biome.BiomeCenter);
            VisualizationActor->SetActorLabel(FString::Printf(TEXT("BiomeViz_%s"), *Biome.BiomeName));
            
            UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(VisualizationActor);
            VisualizationActor->SetRootComponent(MeshComp);
            
            UE_LOG(LogTemp, Warning, TEXT("Created visualization actor for biome: %s"), *Biome.BiomeName);
        }
    }
}

float AEng_BiomeManager::CalculateDistanceToNearestBiome(const FVector& Location) const
{
    float MinDistance = MAX_FLT;
    
    for (const FEng_BiomeData& Biome : BiomeDefinitions)
    {
        float Distance = FVector::Dist(Location, Biome.BiomeCenter);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
        }
    }
    
    return MinDistance;
}

int32 AEng_BiomeManager::FindBiomeIndex(EBiomeType BiomeType) const
{
    for (int32 i = 0; i < BiomeDefinitions.Num(); i++)
    {
        if (BiomeDefinitions[i].BiomeType == BiomeType)
        {
            return i;
        }
    }
    
    return INDEX_NONE;
}