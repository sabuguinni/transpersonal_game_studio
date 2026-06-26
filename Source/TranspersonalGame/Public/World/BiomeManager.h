#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    DenseJungle     UMETA(DisplayName = "Dense Jungle"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    RockyCliffs     UMETA(DisplayName = "Rocky Cliffs"),
    Wetlands        UMETA(DisplayName = "Wetlands"),
    Volcanic        UMETA(DisplayName = "Volcanic")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CentreLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 4000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurSpawnWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeFogColor = FLinearColor(0.5f, 0.7f, 0.5f, 1.0f);
};

USTRUCT(BlueprintType)
struct FWorld_WaterBody
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    bool bIsDrinkable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FName WaterBodyTag = "WaterSource";
};

/**
 * ABiomeManager — manages biome zones, water bodies, and terrain variation
 * for the Cretaceous world. Spawns in MinPlayableMap as a singleton actor.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === Biome Query API ===

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsLocationInBiome(const FVector& WorldLocation, EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetVegetationDensityAt(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetDinosaurSpawnWeightAt(const FVector& WorldLocation) const;

    // === Water Query API ===

    UFUNCTION(BlueprintCallable, Category = "Water")
    bool FindNearestWaterSource(const FVector& FromLocation, FVector& OutWaterLocation, float SearchRadius = 15000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Water")
    bool IsLocationNearWater(const FVector& WorldLocation, float Threshold = 500.0f) const;

    // === Biome Registration ===

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiomeZone(const FWorld_BiomeZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Water")
    void RegisterWaterBody(const FWorld_WaterBody& WaterBody);

    // === Singleton Access ===

    UFUNCTION(BlueprintCallable, Category = "Biome", meta = (WorldContext = "WorldContextObject"))
    static ABiomeManager* GetInstance(UObject* WorldContextObject);

    // === Debug ===

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DrawBiomeDebugOverlay();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Config")
    TArray<FWorld_WaterBody> WaterBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    bool bShowDebugOverlay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float DebugDrawDuration = 0.1f;

private:
    void InitializeDefaultBiomes();
    void InitializeDefaultWaterBodies();

    static ABiomeManager* Instance;
};
