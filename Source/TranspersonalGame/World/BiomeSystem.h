#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeSystem.generated.h"

// ============================================================
// EWorld_BiomeType — 6 prehistoric biome types
// ============================================================
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest          UMETA(DisplayName = "Dense Forest"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    RockyHighlands  UMETA(DisplayName = "Rocky Highlands"),
    RiverDelta      UMETA(DisplayName = "River Delta"),
    VolcanicZone    UMETA(DisplayName = "Volcanic Zone"),
    Swampland       UMETA(DisplayName = "Swampland"),
    None            UMETA(DisplayName = "Unclassified")
};

// ============================================================
// FWorld_BiomeData — per-biome configuration struct
// ============================================================
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FoliageDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;  // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;      // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurSpawnWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown Biome");
};

// ============================================================
// ABiomeSystem — manages all biome zones in the world
// ============================================================
UCLASS(ClassGroup = (World), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ABiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    ABiomeSystem();

    // ---- Biome Registry ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeData> RegisteredBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float BiomeBlendRadius = 500.0f;

    // ---- Runtime State ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biomes")
    int32 ActiveBiomeCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biomes")
    bool bBiomesInitialized = false;

    // ---- Core Methods ----
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetBiomeBlendFactor(const FVector& WorldLocation, EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    TArray<EWorld_BiomeType> GetNeighboringBiomes(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FLinearColor GetFogColorAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biomes")
    void RegenerateDefaultBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    int32 GetBiomeCount() const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    bool IsLocationInBiome(const FVector& WorldLocation, EWorld_BiomeType BiomeType) const;

    // ---- AActor overrides ----
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

protected:
    void BuildDefaultBiomeRegistry();
    FWorld_BiomeData* FindBiomeDataMutable(EWorld_BiomeType BiomeType);
};
