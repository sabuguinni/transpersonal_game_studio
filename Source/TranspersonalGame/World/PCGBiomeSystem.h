// PCGBiomeSystem.h
// Agent #05 — Procedural World Generator
// Cycle: PROD_CYCLE_AUTO_20260626_001
// Biome classification and PCG-driven world population system

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "PCGBiomeSystem.generated.h"

// ============================================================
// ENUMS — Global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    CretaceousJungle    UMETA(DisplayName = "Cretaceous Jungle"),
    OpenSavanna         UMETA(DisplayName = "Open Savanna"),
    RockyBadlands       UMETA(DisplayName = "Rocky Badlands"),
    RiverDelta          UMETA(DisplayName = "River Delta"),
    VolcanicPlains      UMETA(DisplayName = "Volcanic Plains"),
    CoastalShallows     UMETA(DisplayName = "Coastal Shallows")
};

UENUM(BlueprintType)
enum class EWorld_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Dense Fog")
};

// ============================================================
// STRUCTS — Global scope (RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct FWorld_BiomeData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::CretaceousJungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Cretaceous Jungle");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TreeDensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RockDensity = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterCoverage = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float AmbientTemperature = 28.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityLevel = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor(0.6f, 0.8f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> VegetationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FName> DinosaurSpecies;
};

USTRUCT(BlueprintType)
struct FWorld_TerrainCell
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector2D GridPosition = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EWorld_BiomeType AssignedBiome = EWorld_BiomeType::CretaceousJungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float ElevationMeters = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float MoistureLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bHasWater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bIsPopulated = false;
};

USTRUCT(BlueprintType)
struct FWorld_PCGSpawnRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    EWorld_BiomeType TargetBiome = EWorld_BiomeType::CretaceousJungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    int32 MaxActorsToSpawn = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    int32 RandomSeed = 42;
};

// ============================================================
// MAIN CLASS
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API APCGBiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    APCGBiomeSystem();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Biome Query ----

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    // ---- PCG Population ----

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|PCG")
    void PopulateBiomeRegion(const FWorld_PCGSpawnRequest& Request);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|PCG")
    void ClearAllProceduralActors();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|PCG")
    void RegenerateWorld();

    // ---- Weather ----

    UFUNCTION(BlueprintCallable, Category = "World|Weather")
    void SetWeatherState(EWorld_WeatherState NewWeather);

    UFUNCTION(BlueprintPure, Category = "World|Weather")
    EWorld_WeatherState GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(BlueprintCallable, Category = "World|Weather")
    void TickWeatherSystem(float DeltaSeconds);

    // ---- Terrain Grid ----

    UFUNCTION(BlueprintCallable, Category = "World|Terrain")
    void InitializeTerrainGrid(int32 GridWidth, int32 GridHeight, float CellSize);

    UFUNCTION(BlueprintPure, Category = "World|Terrain")
    int32 GetTotalTerrainCells() const { return TerrainGrid.Num(); }

protected:
    // ---- Biome Configuration ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    TMap<EWorld_BiomeType, FWorld_BiomeData> BiomeDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float BiomeBlendRadius = 500.0f;

    // ---- Terrain Grid ----

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
    TArray<FWorld_TerrainCell> TerrainGrid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 GridWidth = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 GridHeight = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float CellSizeUnits = 1000.0f;

    // ---- Weather ----

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather")
    EWorld_WeatherState CurrentWeather = EWorld_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionTime = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherChangeChancePerMinute = 0.15f;

    // ---- PCG State ----

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PCG")
    TArray<AActor*> ProceduralActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    int32 GlobalSeed = 12345;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    bool bAutoPopulateOnBeginPlay = false;

private:
    float WeatherTimer = 0.0f;

    void InitializeBiomeDatabase();
    FVector2D WorldToGrid(const FVector& WorldLocation) const;
    float SampleNoise(float X, float Y, int32 Seed) const;
    EWorld_BiomeType ClassifyBiomeFromNoise(float Temperature, float Moisture) const;
};
