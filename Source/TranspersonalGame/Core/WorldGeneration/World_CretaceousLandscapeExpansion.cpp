#include "World_CretaceousLandscapeExpansion.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "LandscapeEditorModule.h"
#include "EditorLevelLibrary.h"
#include "TranspersonalGame/SharedTypes.h"

AWorld_CretaceousLandscapeExpansion::AWorld_CretaceousLandscapeExpansion()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 5.0f;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create landscape expansion component
    LandscapeExpansionComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LandscapeExpansionComponent"));
    LandscapeExpansionComponent->SetupAttachment(RootComponent);

    // Initialize expansion parameters
    TargetLandscapeSize = FVector2D(10000.0f, 10000.0f); // 10km x 10km
    BiomeZoneRadius = 2000.0f; // 2km radius per biome
    ExpansionProgress = 0.0f;
    bIsExpanding = false;
    bAutoExpand = true;

    // Initialize biome zone data
    InitializeBiomeZones();
}

void AWorld_CretaceousLandscapeExpansion::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousLandscapeExpansion: BeginPlay - Starting landscape expansion system"));
    
    if (bAutoExpand)
    {
        StartLandscapeExpansion();
    }
}

void AWorld_CretaceousLandscapeExpansion::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsExpanding)
    {
        UpdateExpansionProgress(DeltaTime);
    }
}

void AWorld_CretaceousLandscapeExpansion::InitializeBiomeZones()
{
    BiomeZones.Empty();
    
    // Define the 5 Cretaceous biome zones as per memory requirements
    FWorld_BiomeZoneData SwampBiome;
    SwampBiome.BiomeName = TEXT("Pantano_SW");
    SwampBiome.BiomeType = EWorld_BiomeType::Swamp;
    SwampBiome.CenterLocation = FVector(-5000.0f, -5000.0f, 100.0f);
    SwampBiome.Radius = BiomeZoneRadius;
    SwampBiome.Elevation = 100.0f;
    SwampBiome.WaterLevel = 80.0f;
    BiomeZones.Add(SwampBiome);
    
    FWorld_BiomeZoneData ForestBiome;
    ForestBiome.BiomeName = TEXT("Floresta_NW");
    ForestBiome.BiomeType = EWorld_BiomeType::TropicalForest;
    ForestBiome.CenterLocation = FVector(-5000.0f, 5000.0f, 200.0f);
    ForestBiome.Radius = BiomeZoneRadius;
    ForestBiome.Elevation = 200.0f;
    ForestBiome.WaterLevel = 150.0f;
    BiomeZones.Add(ForestBiome);
    
    FWorld_BiomeZoneData SavannaBiome;
    SavannaBiome.BiomeName = TEXT("Savana_Centro");
    SavannaBiome.BiomeType = EWorld_BiomeType::Grassland;
    SavannaBiome.CenterLocation = FVector(0.0f, 0.0f, 150.0f);
    SavannaBiome.Radius = BiomeZoneRadius;
    SavannaBiome.Elevation = 150.0f;
    SavannaBiome.WaterLevel = 120.0f;
    BiomeZones.Add(SavannaBiome);
    
    FWorld_BiomeZoneData DesertBiome;
    DesertBiome.BiomeName = TEXT("Deserto_E");
    DesertBiome.BiomeType = EWorld_BiomeType::Desert;
    DesertBiome.CenterLocation = FVector(5000.0f, 0.0f, 100.0f);
    DesertBiome.Radius = BiomeZoneRadius;
    DesertBiome.Elevation = 100.0f;
    DesertBiome.WaterLevel = 50.0f;
    BiomeZones.Add(DesertBiome);
    
    FWorld_BiomeZoneData MountainBiome;
    MountainBiome.BiomeName = TEXT("Montanha_NE");
    MountainBiome.BiomeType = EWorld_BiomeType::Mountain;
    MountainBiome.CenterLocation = FVector(5000.0f, 5000.0f, 500.0f);
    MountainBiome.Radius = BiomeZoneRadius;
    MountainBiome.Elevation = 500.0f;
    MountainBiome.WaterLevel = 480.0f;
    BiomeZones.Add(MountainBiome);
    
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousLandscapeExpansion: Initialized %d biome zones"), BiomeZones.Num());
}

void AWorld_CretaceousLandscapeExpansion::StartLandscapeExpansion()
{
    if (bIsExpanding)
    {
        UE_LOG(LogTemp, Warning, TEXT("World_CretaceousLandscapeExpansion: Expansion already in progress"));
        return;
    }
    
    bIsExpanding = true;
    ExpansionProgress = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousLandscapeExpansion: Starting landscape expansion to %fx%f"), 
           TargetLandscapeSize.X, TargetLandscapeSize.Y);
    
    // Create biome zone markers
    CreateBiomeZoneMarkers();
    
    // Create water bodies
    CreateWaterBodies();
    
    // Trigger expansion complete event
    OnExpansionComplete();
}

void AWorld_CretaceousLandscapeExpansion::CreateBiomeZoneMarkers()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("World_CretaceousLandscapeExpansion: No valid world context"));
        return;
    }
    
    for (const FWorld_BiomeZoneData& BiomeZone : BiomeZones)
    {
        // Spawn biome zone marker actor
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = FName(*FString::Printf(TEXT("BiomeZone_%s"), *BiomeZone.BiomeName));
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        AStaticMeshActor* ZoneMarker = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            BiomeZone.CenterLocation,
            FRotator::ZeroRotator,
            SpawnParams
        );
        
        if (ZoneMarker)
        {
            ZoneMarker->SetActorLabel(FString::Printf(TEXT("BiomeZone_%s"), *BiomeZone.BiomeName));
            ZoneMarker->SetActorScale3D(FVector(20.0f, 20.0f, 1.0f)); // 2km radius representation
            
            BiomeZoneActors.Add(ZoneMarker);
            
            UE_LOG(LogTemp, Warning, TEXT("World_CretaceousLandscapeExpansion: Created biome zone marker for %s at %s"), 
                   *BiomeZone.BiomeName, *BiomeZone.CenterLocation.ToString());
        }
    }
}

void AWorld_CretaceousLandscapeExpansion::CreateWaterBodies()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Define water body locations
    TArray<FWorld_WaterBodyData> WaterBodies;
    
    FWorld_WaterBodyData SwampWater;
    SwampWater.Name = TEXT("SwampWater_Main");
    SwampWater.Location = FVector(-4500.0f, -4500.0f, 50.0f);
    SwampWater.Scale = FVector(10.0f, 10.0f, 0.1f);
    WaterBodies.Add(SwampWater);
    
    FWorld_WaterBodyData CentralLake;
    CentralLake.Name = TEXT("CentralLake");
    CentralLake.Location = FVector(500.0f, 500.0f, 120.0f);
    CentralLake.Scale = FVector(8.0f, 8.0f, 0.1f);
    WaterBodies.Add(CentralLake);
    
    FWorld_WaterBodyData MountainLake;
    MountainLake.Name = TEXT("MountainLake");
    MountainLake.Location = FVector(4800.0f, 4800.0f, 480.0f);
    MountainLake.Scale = FVector(6.0f, 6.0f, 0.1f);
    WaterBodies.Add(MountainLake);
    
    for (const FWorld_WaterBodyData& WaterBody : WaterBodies)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = FName(*WaterBody.Name);
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        AStaticMeshActor* WaterActor = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            WaterBody.Location,
            FRotator::ZeroRotator,
            SpawnParams
        );
        
        if (WaterActor)
        {
            WaterActor->SetActorLabel(WaterBody.Name);
            WaterActor->SetActorScale3D(WaterBody.Scale);
            
            WaterBodyActors.Add(WaterActor);
            
            UE_LOG(LogTemp, Warning, TEXT("World_CretaceousLandscapeExpansion: Created water body %s at %s"), 
                   *WaterBody.Name, *WaterBody.Location.ToString());
        }
    }
}

void AWorld_CretaceousLandscapeExpansion::UpdateExpansionProgress(float DeltaTime)
{
    ExpansionProgress += DeltaTime * 0.1f; // 10 second expansion time
    
    if (ExpansionProgress >= 1.0f)
    {
        ExpansionProgress = 1.0f;
        bIsExpanding = false;
        
        UE_LOG(LogTemp, Warning, TEXT("World_CretaceousLandscapeExpansion: Landscape expansion completed"));
        OnExpansionComplete();
    }
}

void AWorld_CretaceousLandscapeExpansion::OnExpansionComplete()
{
    UE_LOG(LogTemp, Warning, TEXT("World_CretaceousLandscapeExpansion: Expansion complete - %d biome zones, %d water bodies created"), 
           BiomeZoneActors.Num(), WaterBodyActors.Num());
    
    // Broadcast expansion complete event
    if (OnLandscapeExpansionComplete.IsBound())
    {
        OnLandscapeExpansionComplete.Broadcast();
    }
}

FVector AWorld_CretaceousLandscapeExpansion::GetBiomeZoneCenter(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeZoneData& BiomeZone : BiomeZones)
    {
        if (BiomeZone.BiomeType == BiomeType)
        {
            return BiomeZone.CenterLocation;
        }
    }
    
    return FVector::ZeroVector;
}

TArray<FWorld_BiomeZoneData> AWorld_CretaceousLandscapeExpansion::GetAllBiomeZones() const
{
    return BiomeZones;
}

bool AWorld_CretaceousLandscapeExpansion::IsExpansionComplete() const
{
    return !bIsExpanding && ExpansionProgress >= 1.0f;
}

float AWorld_CretaceousLandscapeExpansion::GetExpansionProgress() const
{
    return ExpansionProgress;
}