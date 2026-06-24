#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCGBiomeZoneSystem.generated.h"

// ============================================================
// PCGBiomeZoneSystem.h — Agent #05 Procedural World Generator
// Biome zone definitions for the Cretaceous world.
// Three zones: Savanna (open), Jungle (dense), Rocky (outcrop)
// ============================================================

UENUM(BlueprintType)
enum class EWorld_BiomeZoneType : uint8
{
    Savanna     UMETA(DisplayName = "Open Savanna"),
    Jungle      UMETA(DisplayName = "Dense Jungle"),
    Rocky       UMETA(DisplayName = "Rocky Outcrop"),
    Riverbank   UMETA(DisplayName = "Riverbank"),
    Volcanic    UMETA(DisplayName = "Volcanic Field"),
    Unknown     UMETA(DisplayName = "Unknown")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeZoneType ZoneType = EWorld_BiomeZoneType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CentreLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RadiusUnits = 4000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;   // 0.0 = bare, 1.0 = dense

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TerrainRoughness = 0.3f;    // 0.0 = flat, 1.0 = very rough

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float AmbientTemperature = 28.0f; // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasWaterSource = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor DebugColour = FLinearColor(0.5f, 0.8f, 0.2f, 1.0f);
};

USTRUCT(BlueprintType)
struct FWorld_TerrainHeightSample
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Terrain")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain")
    float HeightMetres = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain")
    EWorld_BiomeZoneType BiomeAtLocation = EWorld_BiomeZoneType::Unknown;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain")
    float SlopeAngleDegrees = 0.0f;
};

/**
 * UWorld_BiomeZoneManager
 * Manages biome zone definitions and provides query functions
 * for other systems (foliage placement, AI navigation, audio).
 */
UCLASS(ClassGroup = "WorldGen", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeZoneManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeZoneManager();

    // Returns the biome type at a given world location
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeZoneType GetBiomeAtLocation(const FVector& WorldLocation) const;

    // Returns full biome data for the zone containing the location
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeZoneData GetBiomeDataAtLocation(const FVector& WorldLocation) const;

    // Returns all zones of a given type
    UFUNCTION(BlueprintCallable, Category = "Biome")
    TArray<FWorld_BiomeZoneData> GetZonesOfType(EWorld_BiomeZoneType ZoneType) const;

    // Returns true if the location is within any registered zone
    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsLocationInZone(const FVector& WorldLocation, EWorld_BiomeZoneType ZoneType) const;

    // Registers a new biome zone at runtime
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiomeZone(const FWorld_BiomeZoneData& ZoneData);

    // Returns vegetation density at a location (0.0 - 1.0)
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetVegetationDensityAtLocation(const FVector& WorldLocation) const;

    // Initialise default Cretaceous zones
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome")
    void InitialiseDefaultCretaceousZones();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FWorld_BiomeZoneData> RegisteredZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bDrawDebugZones = false;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Returns squared distance from location to zone centre
    float GetSquaredDistanceToZone(const FVector& Location, const FWorld_BiomeZoneData& Zone) const;
};

/**
 * AWorld_BiomeZoneActor
 * Placeable actor that defines a biome zone in the level.
 * Can be placed in the editor to mark zone boundaries.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeZoneActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    FWorld_BiomeZoneData ZoneDefinition;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    UWorld_BiomeZoneManager* BiomeManager;

    // Called when placed in editor — registers this zone with the world manager
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome Zone")
    void RegisterZoneWithWorld();

    virtual void BeginPlay() override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
