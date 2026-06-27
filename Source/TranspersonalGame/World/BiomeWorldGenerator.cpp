#include "World/BiomeWorldGenerator.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

ABiomeWorldGenerator::ABiomeWorldGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    InitDefaultBiomes();
}

void ABiomeWorldGenerator::BeginPlay()
{
    Super::BeginPlay();
    if (!bWorldGenerated)
    {
        GenerateWorld();
    }
}

void ABiomeWorldGenerator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABiomeWorldGenerator::InitDefaultBiomes()
{
    BiomeZones.Empty();

    // Jungle biome — left side
    FWorld_BiomeZone Jungle;
    Jungle.BiomeType = EWorld_BiomeType::Jungle;
    Jungle.Center = FVector(-3000.0f, 0.0f, 0.0f);
    Jungle.Radius = 1500.0f;
    Jungle.VegetationDensity = 2.0f;
    Jungle.Temperature = 30.0f;
    Jungle.Humidity = 0.9f;
    BiomeZones.Add(Jungle);

    // Savanna biome — center
    FWorld_BiomeZone Savanna;
    Savanna.BiomeType = EWorld_BiomeType::Savanna;
    Savanna.Center = FVector(0.0f, 0.0f, 0.0f);
    Savanna.Radius = 1500.0f;
    Savanna.VegetationDensity = 0.5f;
    Savanna.Temperature = 35.0f;
    Savanna.Humidity = 0.3f;
    BiomeZones.Add(Savanna);

    // Volcanic biome — right side
    FWorld_BiomeZone Volcanic;
    Volcanic.BiomeType = EWorld_BiomeType::Volcanic;
    Volcanic.Center = FVector(3000.0f, 0.0f, 0.0f);
    Volcanic.Radius = 1500.0f;
    Volcanic.VegetationDensity = 0.1f;
    Volcanic.Temperature = 55.0f;
    Volcanic.Humidity = 0.05f;
    BiomeZones.Add(Volcanic);

    // River zone — cutting through center
    FWorld_BiomeZone River;
    River.BiomeType = EWorld_BiomeType::River;
    River.Center = FVector(0.0f, -500.0f, -30.0f);
    River.Radius = 300.0f;
    River.VegetationDensity = 1.5f;
    River.Temperature = 22.0f;
    River.Humidity = 1.0f;
    BiomeZones.Add(River);
}

EWorld_BiomeType ABiomeWorldGenerator::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float ClosestDist = MAX_FLT;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::Unknown;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.Center);
        if (Dist < Zone.Radius && Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = Zone.BiomeType;
        }
    }
    return ClosestBiome;
}

FWorld_BiomeZone ABiomeWorldGenerator::GetBiomeZoneAtLocation(const FVector& WorldLocation) const
{
    float ClosestDist = MAX_FLT;
    int32 ClosestIdx = -1;

    for (int32 i = 0; i < BiomeZones.Num(); ++i)
    {
        float Dist = FVector::Dist2D(WorldLocation, BiomeZones[i].Center);
        if (Dist < BiomeZones[i].Radius && Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestIdx = i;
        }
    }

    if (ClosestIdx >= 0)
    {
        return BiomeZones[ClosestIdx];
    }

    // Return default zone if no biome found
    FWorld_BiomeZone Default;
    Default.BiomeType = EWorld_BiomeType::Unknown;
    return Default;
}

float ABiomeWorldGenerator::GetVegetationDensityAt(const FVector& WorldLocation) const
{
    FWorld_BiomeZone Zone = GetBiomeZoneAtLocation(WorldLocation);
    return Zone.VegetationDensity;
}

float ABiomeWorldGenerator::GetTemperatureAt(const FVector& WorldLocation) const
{
    FWorld_BiomeZone Zone = GetBiomeZoneAtLocation(WorldLocation);
    return Zone.Temperature;
}

float ABiomeWorldGenerator::GetHumidityAt(const FVector& WorldLocation) const
{
    FWorld_BiomeZone Zone = GetBiomeZoneAtLocation(WorldLocation);
    return Zone.Humidity;
}

void ABiomeWorldGenerator::GenerateWorld()
{
    if (bWorldGenerated)
    {
        UE_LOG(LogTemp, Warning, TEXT("BiomeWorldGenerator: World already generated. Call ClearGeneratedActors() first."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeWorldGenerator: Starting world generation with %d biomes"), BiomeZones.Num());

    FMath::RandInit(RandomSeed);

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        if (Zone.BiomeType == EWorld_BiomeType::River)
        {
            SpawnRiverForZone(Zone);
        }
        else
        {
            SpawnVegetationForBiome(Zone);
        }
    }

    bWorldGenerated = true;
    UE_LOG(LogTemp, Log, TEXT("BiomeWorldGenerator: Generation complete. %d actors spawned."), GeneratedActors.Num());
}

void ABiomeWorldGenerator::ClearGeneratedActors()
{
    for (AActor* Actor : GeneratedActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    GeneratedActors.Empty();
    bWorldGenerated = false;
    UE_LOG(LogTemp, Log, TEXT("BiomeWorldGenerator: All generated actors cleared."));
}

int32 ABiomeWorldGenerator::GetGeneratedActorCount() const
{
    return GeneratedActors.Num();
}

void ABiomeWorldGenerator::SpawnVegetationForBiome(const FWorld_BiomeZone& Zone)
{
    UWorld* World = GetWorld();
    if (!World) return;

    int32 VegCount = FMath::RoundToInt(Zone.VegetationDensity * 20.0f);
    VegCount = FMath::Clamp(VegCount, 1, 50);

    for (int32 i = 0; i < VegCount; ++i)
    {
        float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
        float Radius = FMath::FRandRange(100.0f, Zone.Radius * 0.9f);
        float X = Zone.Center.X + Radius * FMath::Cos(Angle);
        float Y = Zone.Center.Y + Radius * FMath::Sin(Angle);
        float Z = Zone.Center.Z;

        float HeightScale = 1.0f + FMath::FRandRange(0.0f, 1.5f);
        float WidthScale = 0.5f + FMath::FRandRange(0.0f, 0.5f);

        FString Label = FString::Printf(TEXT("%s_Veg_%03d"),
            *UEnum::GetValueAsString(Zone.BiomeType), i);

        AActor* Veg = SpawnPrimitiveMesh(
            FVector(X, Y, Z),
            FVector(WidthScale, WidthScale, HeightScale),
            Label
        );

        if (Veg)
        {
            GeneratedActors.Add(Veg);
        }
    }
}

void ABiomeWorldGenerator::SpawnRiverForZone(const FWorld_BiomeZone& Zone)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Spawn a long flat plane for the river
    AActor* River = SpawnPrimitiveMesh(
        Zone.Center,
        FVector(30.0f, 5.0f, 0.1f),
        TEXT("PCG_River_Main")
    );

    if (River)
    {
        GeneratedActors.Add(River);
        UE_LOG(LogTemp, Log, TEXT("BiomeWorldGenerator: River spawned at %s"), *Zone.Center.ToString());
    }
}

AActor* ABiomeWorldGenerator::SpawnPrimitiveMesh(const FVector& Location, const FVector& Scale, const FString& Label)
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        Location,
        FRotator::ZeroRotator,
        Params
    );

    if (MeshActor)
    {
        MeshActor->SetActorLabel(Label);
        MeshActor->SetActorScale3D(Scale);

        UStaticMeshComponent* SMC = MeshActor->GetStaticMeshComponent();
        if (SMC)
        {
            SMC->SetMobility(EComponentMobility::Static);
        }
    }

    return MeshActor;
}
