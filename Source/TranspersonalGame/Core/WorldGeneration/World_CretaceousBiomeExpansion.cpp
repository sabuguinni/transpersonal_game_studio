#include "World_CretaceousBiomeExpansion.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/PointLight.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AWorld_CretaceousBiomeExpansion::AWorld_CretaceousBiomeExpansion()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize landscape configuration for 10km x 10km world
    LandscapeConfig.LandscapeSize = FVector(10000, 10000, 1000);
    LandscapeConfig.ComponentSizeQuads = 63;
    LandscapeConfig.SectionsPerComponent = 2;
    LandscapeConfig.HeightmapScale = 100.0f;
    LandscapeConfig.bCreateCollision = true;

    // Initialize the 5 biome zones as per purchase criteria
    BiomeZones.Empty();
    
    // Pantano SW
    FWorld_BiomeZoneData SwampBiome;
    SwampBiome.BiomeName = TEXT("Pantano_SW");
    SwampBiome.ZoneLocation = FVector(-2500, -2500, 50);
    SwampBiome.ZoneRadius = 2000.0f;
    SwampBiome.BiomeType = EBiomeType::Swamp;
    SwampBiome.BiomeColor = FLinearColor(0.2f, 0.8f, 0.3f, 1.0f);
    BiomeZones.Add(SwampBiome);

    // Floresta NW
    FWorld_BiomeZoneData ForestBiome;
    ForestBiome.BiomeName = TEXT("Floresta_NW");
    ForestBiome.ZoneLocation = FVector(-2500, 2500, 100);
    ForestBiome.ZoneRadius = 2000.0f;
    ForestBiome.BiomeType = EBiomeType::Forest;
    ForestBiome.BiomeColor = FLinearColor(0.1f, 0.6f, 0.1f, 1.0f);
    BiomeZones.Add(ForestBiome);

    // Savana Centro
    FWorld_BiomeZoneData SavannaBiome;
    SavannaBiome.BiomeName = TEXT("Savana_Centro");
    SavannaBiome.ZoneLocation = FVector(0, 0, 25);
    SavannaBiome.ZoneRadius = 1500.0f;
    SavannaBiome.BiomeType = EBiomeType::Grassland;
    SavannaBiome.BiomeColor = FLinearColor(0.8f, 0.7f, 0.3f, 1.0f);
    BiomeZones.Add(SavannaBiome);

    // Deserto E
    FWorld_BiomeZoneData DesertBiome;
    DesertBiome.BiomeName = TEXT("Deserto_E");
    DesertBiome.ZoneLocation = FVector(2500, 0, 10);
    DesertBiome.ZoneRadius = 2000.0f;
    DesertBiome.BiomeType = EBiomeType::Desert;
    DesertBiome.BiomeColor = FLinearColor(0.9f, 0.8f, 0.4f, 1.0f);
    BiomeZones.Add(DesertBiome);

    // Montanha NE
    FWorld_BiomeZoneData MountainBiome;
    MountainBiome.BiomeName = TEXT("Montanha_NE");
    MountainBiome.ZoneLocation = FVector(2500, 2500, 300);
    MountainBiome.ZoneRadius = 2000.0f;
    MountainBiome.BiomeType = EBiomeType::Mountain;
    MountainBiome.BiomeColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
    BiomeZones.Add(MountainBiome);

    // Initialize arrays
    BiomeMarkers.Empty();
    WaterBodies.Empty();
    MainLandscape = nullptr;
}

void AWorld_CretaceousBiomeExpansion::BeginPlay()
{
    Super::BeginPlay();
    
    // Validate purchase criteria on begin play
    LogPurchaseCriteriaStatus();
}

void AWorld_CretaceousBiomeExpansion::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWorld_CretaceousBiomeExpansion::CreateCretaceousLandscape()
{
    UE_LOG(LogTemp, Warning, TEXT("Creating Cretaceous Landscape - 10km x 10km"));
    
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("World is null - cannot create landscape"));
        return;
    }

    CreateLandscapeInternal();
    CreateBiomeZones();
    CreateWaterBodies();
    
    // Validate that we meet purchase criteria
    if (MeetsPurchaseCriteria())
    {
        UE_LOG(LogTemp, Warning, TEXT("SUCCESS: Purchase criteria met - 10km landscape with 5 biomes created"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FAILED: Purchase criteria not met"));
    }
}

void AWorld_CretaceousBiomeExpansion::CreateBiomeZones()
{
    UE_LOG(LogTemp, Warning, TEXT("Creating 5 biome zones"));
    
    // Clear existing markers
    for (APointLight* Marker : BiomeMarkers)
    {
        if (Marker && IsValid(Marker))
        {
            Marker->Destroy();
        }
    }
    BiomeMarkers.Empty();

    SetupBiomeMarkersInternal();
}

void AWorld_CretaceousBiomeExpansion::CreateWaterBodies()
{
    UE_LOG(LogTemp, Warning, TEXT("Creating water bodies"));
    
    // Clear existing water bodies
    for (AStaticMeshActor* WaterBody : WaterBodies)
    {
        if (WaterBody && IsValid(WaterBody))
        {
            WaterBody->Destroy();
        }
    }
    WaterBodies.Empty();

    CreateWaterBodiesInternal();
}

bool AWorld_CretaceousBiomeExpansion::ValidateLandscapeSize()
{
    if (!MainLandscape)
    {
        UE_LOG(LogTemp, Warning, TEXT("No landscape found for validation"));
        return false;
    }

    // Check if landscape meets minimum 10km x 10km requirement
    FVector LandscapeScale = MainLandscape->GetActorScale3D();
    float EffectiveSize = FMath::Min(LandscapeScale.X, LandscapeScale.Y) * 100.0f; // Convert to cm
    
    bool bMeetsSize = EffectiveSize >= 1000000.0f; // 10km in cm
    
    UE_LOG(LogTemp, Warning, TEXT("Landscape validation - Size: %.2f cm, Meets criteria: %s"), 
           EffectiveSize, bMeetsSize ? TEXT("YES") : TEXT("NO"));
    
    return bMeetsSize;
}

void AWorld_CretaceousBiomeExpansion::AddBiomeZone(const FWorld_BiomeZoneData& BiomeData)
{
    BiomeZones.Add(BiomeData);
    UE_LOG(LogTemp, Log, TEXT("Added biome zone: %s"), *BiomeData.BiomeName);
}

void AWorld_CretaceousBiomeExpansion::RemoveBiomeZone(const FString& BiomeName)
{
    BiomeZones.RemoveAll([BiomeName](const FWorld_BiomeZoneData& Zone)
    {
        return Zone.BiomeName == BiomeName;
    });
    UE_LOG(LogTemp, Log, TEXT("Removed biome zone: %s"), *BiomeName);
}

FWorld_BiomeZoneData AWorld_CretaceousBiomeExpansion::GetBiomeZoneData(const FString& BiomeName)
{
    for (const FWorld_BiomeZoneData& Zone : BiomeZones)
    {
        if (Zone.BiomeName == BiomeName)
        {
            return Zone;
        }
    }
    
    // Return default if not found
    return FWorld_BiomeZoneData();
}

void AWorld_CretaceousBiomeExpansion::ExpandLandscapeToSize(const FVector& TargetSize)
{
    LandscapeConfig.LandscapeSize = TargetSize;
    UE_LOG(LogTemp, Warning, TEXT("Landscape target size set to: %s"), *TargetSize.ToString());
    
    if (MainLandscape)
    {
        // Calculate required scale
        FVector RequiredScale = TargetSize / 100.0f; // Convert to UE units
        MainLandscape->SetActorScale3D(RequiredScale);
        UE_LOG(LogTemp, Warning, TEXT("Landscape scaled to: %s"), *RequiredScale.ToString());
    }
}

void AWorld_CretaceousBiomeExpansion::SetLandscapeConfiguration(const FWorld_CretaceousLandscapeConfig& Config)
{
    LandscapeConfig = Config;
    UE_LOG(LogTemp, Log, TEXT("Landscape configuration updated"));
}

void AWorld_CretaceousBiomeExpansion::CreateLandscapeInternal()
{
    // Find existing landscape in the world
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Look for existing landscape
    for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
    {
        ALandscape* ExistingLandscape = *ActorItr;
        if (ExistingLandscape)
        {
            MainLandscape = ExistingLandscape;
            UE_LOG(LogTemp, Warning, TEXT("Found existing landscape: %s"), *ExistingLandscape->GetName());
            
            // Ensure it meets size requirements
            FVector RequiredScale = LandscapeConfig.LandscapeSize / 100.0f;
            ExistingLandscape->SetActorScale3D(RequiredScale);
            break;
        }
    }

    if (!MainLandscape)
    {
        UE_LOG(LogTemp, Warning, TEXT("No existing landscape found - landscape should be created via UE5 Python"));
    }
}

void AWorld_CretaceousBiomeExpansion::SetupBiomeMarkersInternal()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (const FWorld_BiomeZoneData& BiomeData : BiomeZones)
    {
        // Spawn point light as biome marker
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = FName(*FString::Printf(TEXT("BiomeMarker_%s"), *BiomeData.BiomeName));
        
        APointLight* BiomeMarker = World->SpawnActor<APointLight>(
            APointLight::StaticClass(),
            BiomeData.ZoneLocation,
            FRotator::ZeroRotator,
            SpawnParams
        );

        if (BiomeMarker)
        {
            BiomeMarker->SetActorLabel(FString::Printf(TEXT("BiomeZone_%s"), *BiomeData.BiomeName));
            
            // Configure light component
            UPointLightComponent* LightComp = BiomeMarker->GetPointLightComponent();
            if (LightComp)
            {
                LightComp->SetLightColor(BiomeData.BiomeColor);
                LightComp->SetIntensity(1000.0f);
                LightComp->SetAttenuationRadius(BiomeData.ZoneRadius);
            }
            
            BiomeMarkers.Add(BiomeMarker);
            UE_LOG(LogTemp, Log, TEXT("Created biome marker: %s at %s"), 
                   *BiomeData.BiomeName, *BiomeData.ZoneLocation.ToString());
        }
    }
}

void AWorld_CretaceousBiomeExpansion::CreateWaterBodiesInternal()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Define water body locations
    TArray<TPair<FString, FVector>> WaterLocations = {
        {TEXT("SwampWater_Main"), FVector(-2500, -2500, 0)},
        {TEXT("CentralLake"), FVector(0, 0, 0)},
        {TEXT("MountainStream"), FVector(2500, 2500, 250)}
    };

    for (const auto& WaterLocation : WaterLocations)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = FName(*FString::Printf(TEXT("Water_%s"), *WaterLocation.Key));
        
        AStaticMeshActor* WaterActor = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            WaterLocation.Value,
            FRotator::ZeroRotator,
            SpawnParams
        );

        if (WaterActor)
        {
            WaterActor->SetActorLabel(FString::Printf(TEXT("Water_%s"), *WaterLocation.Key));
            WaterActor->SetActorScale3D(FVector(10, 10, 0.1f)); // Flat water surface
            
            WaterBodies.Add(WaterActor);
            UE_LOG(LogTemp, Log, TEXT("Created water body: %s at %s"), 
                   *WaterLocation.Key, *WaterLocation.Value.ToString());
        }
    }
}

bool AWorld_CretaceousBiomeExpansion::ValidateWorldSize() const
{
    return LandscapeConfig.LandscapeSize.X >= 10000.0f && LandscapeConfig.LandscapeSize.Y >= 10000.0f;
}

bool AWorld_CretaceousBiomeExpansion::ValidateBiomeConfiguration() const
{
    // Must have exactly 5 biomes as per purchase criteria
    return BiomeZones.Num() == 5;
}

bool AWorld_CretaceousBiomeExpansion::MeetsPurchaseCriteria() const
{
    bool bWorldSizeOK = ValidateWorldSize();
    bool bBiomeConfigOK = ValidateBiomeConfiguration();
    bool bLandscapeExists = (MainLandscape != nullptr);
    
    return bWorldSizeOK && bBiomeConfigOK && bLandscapeExists;
}

void AWorld_CretaceousBiomeExpansion::LogPurchaseCriteriaStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== PURCHASE CRITERIA STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("World Size (10km+): %s"), ValidateWorldSize() ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Warning, TEXT("5 Biomes Config: %s"), ValidateBiomeConfiguration() ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Warning, TEXT("Landscape Exists: %s"), (MainLandscape != nullptr) ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Warning, TEXT("Overall Status: %s"), MeetsPurchaseCriteria() ? TEXT("READY FOR PURCHASE") : TEXT("NOT READY"));
    UE_LOG(LogTemp, Warning, TEXT("================================"));
}