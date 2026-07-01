#include "BiomeManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "GameFramework/Actor.h"

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Update every 0.5s for performance

    InitializeBiomeDataTable();
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();

    // Register default biome zones for the MinPlayableMap
    // These will be overridden by PCGWorldGenerator when it runs
    RegisterBiomeZone(EEng_BiomeType::Jungle,  FVector(0.0f,    0.0f,    0.0f), 5000.0f);
    RegisterBiomeZone(EEng_BiomeType::Savanna,  FVector(8000.0f, 0.0f,    0.0f), 4000.0f);
    RegisterBiomeZone(EEng_BiomeType::Swamp,    FVector(0.0f,    8000.0f, 0.0f), 3500.0f);
    RegisterBiomeZone(EEng_BiomeType::Volcanic, FVector(-6000.0f,4000.0f, 0.0f), 3000.0f);
    RegisterBiomeZone(EEng_BiomeType::Coastal,  FVector(0.0f,   -8000.0f, 0.0f), 4500.0f);

    UE_LOG(LogTemp, Log, TEXT("ABiomeManager: Initialized with %d biome zones"), RegisteredZones.Num());
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bEnableAtmosphereUpdates)
    {
        return;
    }

    AtmosphereUpdateTimer += DeltaTime;
    if (AtmosphereUpdateTimer >= AtmosphereUpdateInterval)
    {
        AtmosphereUpdateTimer = 0.0f;

        // Update atmosphere based on player location
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC && PC->GetPawn())
        {
            UpdateAtmosphereForPlayerLocation(PC->GetPawn()->GetActorLocation());
        }
    }

    // Debug draw
    if (bDrawBiomeZones)
    {
        DebugDrawAllBiomeZones();
    }
}

EEng_BiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float BestWeight = -1.0f;
    EEng_BiomeType BestBiome = DefaultBiome;

    for (const FBiomeZone& Zone : RegisteredZones)
    {
        float Dist = FVector::Dist(WorldLocation, Zone.Center);
        if (Dist <= Zone.Radius)
        {
            // Weight by inverse distance (closer = stronger)
            float Weight = 1.0f - (Dist / Zone.Radius);
            if (Weight > BestWeight)
            {
                BestWeight = Weight;
                BestBiome = Zone.BiomeType;
            }
        }
    }

    return BestBiome;
}

FEng_BiomeData ABiomeManager::GetBiomeDataAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    return GetBiomeDataForType(BiomeType);
}

FEng_BiomeData ABiomeManager::GetBlendedBiomeData(const FVector& WorldLocation) const
{
    // Collect all zones that influence this location
    struct FZoneInfluence
    {
        FEng_BiomeData Data;
        float Weight;
    };

    TArray<FZoneInfluence> Influences;
    float TotalWeight = 0.0f;

    for (const FBiomeZone& Zone : RegisteredZones)
    {
        float Dist = FVector::Dist(WorldLocation, Zone.Center);
        float BlendStart = Zone.Radius - BiomeTransitionBlendRadius;

        if (Dist <= Zone.Radius)
        {
            float Weight = 1.0f;
            if (Dist > BlendStart && BlendStart > 0.0f)
            {
                // Smooth blend at edges
                float BlendT = (Dist - BlendStart) / BiomeTransitionBlendRadius;
                Weight = FMath::SmoothStep(1.0f, 0.0f, BlendT);
            }

            if (Weight > 0.0f)
            {
                FZoneInfluence Inf;
                Inf.Data = GetBiomeDataForType(Zone.BiomeType);
                Inf.Weight = Weight;
                Influences.Add(Inf);
                TotalWeight += Weight;
            }
        }
    }

    if (Influences.Num() == 0 || TotalWeight <= 0.0f)
    {
        return GetBiomeDataForType(DefaultBiome);
    }

    // Weighted blend of biome properties
    FEng_BiomeData Blended;
    Blended.Temperature = 0.0f;
    Blended.Humidity = 0.0f;
    Blended.VegetationDensity = 0.0f;
    Blended.DinosaurSpawnWeight = 0.0f;
    Blended.FogDensity = 0.0f;
    Blended.FogColor = FLinearColor::Black;

    for (const FZoneInfluence& Inf : Influences)
    {
        float NormalizedWeight = Inf.Weight / TotalWeight;
        Blended.Temperature        += Inf.Data.Temperature        * NormalizedWeight;
        Blended.Humidity           += Inf.Data.Humidity           * NormalizedWeight;
        Blended.VegetationDensity  += Inf.Data.VegetationDensity  * NormalizedWeight;
        Blended.DinosaurSpawnWeight+= Inf.Data.DinosaurSpawnWeight * NormalizedWeight;
        Blended.FogDensity         += Inf.Data.FogDensity         * NormalizedWeight;
        Blended.FogColor.R         += Inf.Data.FogColor.R         * NormalizedWeight;
        Blended.FogColor.G         += Inf.Data.FogColor.G         * NormalizedWeight;
        Blended.FogColor.B         += Inf.Data.FogColor.B         * NormalizedWeight;
    }

    return Blended;
}

void ABiomeManager::RegisterBiomeZone(EEng_BiomeType BiomeType, const FVector& Center, float Radius)
{
    FBiomeZone NewZone;
    NewZone.BiomeType = BiomeType;
    NewZone.Center = Center;
    NewZone.Radius = Radius;
    RegisteredZones.Add(NewZone);

    UE_LOG(LogTemp, Log, TEXT("ABiomeManager: Registered biome zone %d at (%.0f, %.0f, %.0f) R=%.0f"),
        (int32)BiomeType, Center.X, Center.Y, Center.Z, Radius);
}

void ABiomeManager::ClearAllBiomeZones()
{
    RegisteredZones.Empty();
    UE_LOG(LogTemp, Log, TEXT("ABiomeManager: All biome zones cleared"));
}

FEng_BiomeData ABiomeManager::GetBiomeDataForType(EEng_BiomeType BiomeType) const
{
    const FEng_BiomeData* Found = BiomeDataTable.Find(BiomeType);
    if (Found)
    {
        return *Found;
    }

    // Return default jungle data if not found
    FEng_BiomeData Default;
    Default.BiomeType = EEng_BiomeType::Jungle;
    Default.BiomeName = TEXT("Jungle");
    Default.Temperature = 30.0f;
    Default.Humidity = 0.8f;
    Default.VegetationDensity = 0.9f;
    Default.DinosaurSpawnWeight = 1.2f;
    Default.FogColor = FLinearColor(0.3f, 0.5f, 0.3f, 1.0f);
    Default.FogDensity = 0.025f;
    return Default;
}

int32 ABiomeManager::GetBiomeZoneCount() const
{
    return RegisteredZones.Num();
}

void ABiomeManager::ApplyBiomeAtmosphere(EEng_BiomeType BiomeType)
{
    FEng_BiomeData Data = GetBiomeDataForType(BiomeType);

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find ExponentialHeightFog and apply biome fog settings
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);

    for (AActor* FogActor : FogActors)
    {
        AExponentialHeightFog* Fog = Cast<AExponentialHeightFog>(FogActor);
        if (Fog)
        {
            UExponentialHeightFogComponent* FogComp = Fog->GetComponent();
            if (FogComp)
            {
                FogComp->SetFogDensity(Data.FogDensity);
                FogComp->SetFogInscatteringColor(Data.FogColor);
                UE_LOG(LogTemp, Log, TEXT("ABiomeManager: Applied atmosphere for biome %s"), *Data.BiomeName);
            }
            break; // Only apply to first fog actor
        }
    }
}

void ABiomeManager::UpdateAtmosphereForPlayerLocation(const FVector& PlayerLocation)
{
    FEng_BiomeData BlendedData = GetBlendedBiomeData(PlayerLocation);
    EEng_BiomeType CurrentBiome = GetBiomeAtLocation(PlayerLocation);

    // Only apply if biome changed significantly (optimization)
    static EEng_BiomeType LastAppliedBiome = EEng_BiomeType::None;
    if (CurrentBiome != LastAppliedBiome)
    {
        ApplyBiomeAtmosphere(CurrentBiome);
        LastAppliedBiome = CurrentBiome;
    }
}

void ABiomeManager::DebugDrawAllBiomeZones()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    static const TMap<EEng_BiomeType, FColor> BiomeColors = {
        { EEng_BiomeType::Jungle,   FColor::Green   },
        { EEng_BiomeType::Savanna,  FColor::Yellow  },
        { EEng_BiomeType::Swamp,    FColor(0, 128, 64) },
        { EEng_BiomeType::Volcanic, FColor::Red     },
        { EEng_BiomeType::Coastal,  FColor::Cyan    },
        { EEng_BiomeType::Forest,   FColor(0, 200, 100) },
        { EEng_BiomeType::Desert,   FColor::Orange  },
    };

    for (const FBiomeZone& Zone : RegisteredZones)
    {
        const FColor* ColorPtr = BiomeColors.Find(Zone.BiomeType);
        FColor DrawColor = ColorPtr ? *ColorPtr : FColor::White;

        DrawDebugCircle(
            World,
            Zone.Center,
            Zone.Radius,
            64,
            DrawColor,
            false,
            AtmosphereUpdateInterval + 0.1f,
            0,
            20.0f,
            FVector(1, 0, 0),
            FVector(0, 1, 0)
        );
    }
}

void ABiomeManager::InitializeBiomeDataTable()
{
    // Jungle
    {
        FEng_BiomeData Data;
        Data.BiomeType = EEng_BiomeType::Jungle;
        Data.BiomeName = TEXT("Cretaceous Jungle");
        Data.Temperature = 32.0f;
        Data.Humidity = 0.85f;
        Data.VegetationDensity = 0.95f;
        Data.DinosaurSpawnWeight = 1.3f;
        Data.FogColor = FLinearColor(0.3f, 0.55f, 0.3f, 1.0f);
        Data.FogDensity = 0.025f;
        BiomeDataTable.Add(EEng_BiomeType::Jungle, Data);
    }

    // Savanna
    {
        FEng_BiomeData Data;
        Data.BiomeType = EEng_BiomeType::Savanna;
        Data.BiomeName = TEXT("Prehistoric Savanna");
        Data.Temperature = 38.0f;
        Data.Humidity = 0.25f;
        Data.VegetationDensity = 0.35f;
        Data.DinosaurSpawnWeight = 1.5f; // High dino density — open ground
        Data.FogColor = FLinearColor(0.7f, 0.6f, 0.4f, 1.0f);
        Data.FogDensity = 0.012f;
        BiomeDataTable.Add(EEng_BiomeType::Savanna, Data);
    }

    // Swamp
    {
        FEng_BiomeData Data;
        Data.BiomeType = EEng_BiomeType::Swamp;
        Data.BiomeName = TEXT("Primordial Swamp");
        Data.Temperature = 28.0f;
        Data.Humidity = 0.95f;
        Data.VegetationDensity = 0.7f;
        Data.DinosaurSpawnWeight = 0.8f;
        Data.FogColor = FLinearColor(0.2f, 0.4f, 0.25f, 1.0f);
        Data.FogDensity = 0.045f; // Dense swamp fog
        BiomeDataTable.Add(EEng_BiomeType::Swamp, Data);
    }

    // Volcanic
    {
        FEng_BiomeData Data;
        Data.BiomeType = EEng_BiomeType::Volcanic;
        Data.BiomeName = TEXT("Volcanic Badlands");
        Data.Temperature = 55.0f;
        Data.Humidity = 0.1f;
        Data.VegetationDensity = 0.05f;
        Data.DinosaurSpawnWeight = 0.5f; // Hostile — few dinos
        Data.FogColor = FLinearColor(0.6f, 0.3f, 0.1f, 1.0f);
        Data.FogDensity = 0.035f; // Ash/smoke fog
        BiomeDataTable.Add(EEng_BiomeType::Volcanic, Data);
    }

    // Coastal
    {
        FEng_BiomeData Data;
        Data.BiomeType = EEng_BiomeType::Coastal;
        Data.BiomeName = TEXT("Cretaceous Coast");
        Data.Temperature = 26.0f;
        Data.Humidity = 0.6f;
        Data.VegetationDensity = 0.4f;
        Data.DinosaurSpawnWeight = 1.0f;
        Data.FogColor = FLinearColor(0.5f, 0.65f, 0.75f, 1.0f);
        Data.FogDensity = 0.018f;
        BiomeDataTable.Add(EEng_BiomeType::Coastal, Data);
    }

    // Forest
    {
        FEng_BiomeData Data;
        Data.BiomeType = EEng_BiomeType::Forest;
        Data.BiomeName = TEXT("Ancient Forest");
        Data.Temperature = 22.0f;
        Data.Humidity = 0.7f;
        Data.VegetationDensity = 0.85f;
        Data.DinosaurSpawnWeight = 1.1f;
        Data.FogColor = FLinearColor(0.35f, 0.5f, 0.35f, 1.0f);
        Data.FogDensity = 0.022f;
        BiomeDataTable.Add(EEng_BiomeType::Forest, Data);
    }

    // Desert
    {
        FEng_BiomeData Data;
        Data.BiomeType = EEng_BiomeType::Desert;
        Data.BiomeName = TEXT("Arid Flats");
        Data.Temperature = 45.0f;
        Data.Humidity = 0.05f;
        Data.VegetationDensity = 0.02f;
        Data.DinosaurSpawnWeight = 0.6f;
        Data.FogColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
        Data.FogDensity = 0.008f;
        BiomeDataTable.Add(EEng_BiomeType::Desert, Data);
    }

    UE_LOG(LogTemp, Log, TEXT("ABiomeManager: Initialized %d biome types in data table"), BiomeDataTable.Num());
}
