#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Jungle       UMETA(DisplayName = "Jungle"),
    Savanna      UMETA(DisplayName = "Savanna"),
    Swamp        UMETA(DisplayName = "Swamp"),
    Volcanic     UMETA(DisplayName = "Volcanic"),
    Coastal      UMETA(DisplayName = "Coastal"),
    Forest       UMETA(DisplayName = "Forest"),
    Count        UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Jungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector WorldOrigin = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 28.0f;   // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.8f;       // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FoliageDensity = 1.0f; // 0-1 multiplier

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor(0.4f, 0.6f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FName> AllowedDinosaurSpecies;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // --- Biome Registry ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FEng_BiomeData> RegisteredBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float BiomeTransitionWidth = 500.0f;

    // --- Query API ---
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    bool GetBiomeData(EEng_BiomeType BiomeType, FEng_BiomeData& OutData) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetBlendWeightAtLocation(const FVector& WorldLocation, EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    TArray<FName> GetAllowedDinosaursAtLocation(const FVector& WorldLocation) const;

    // --- Runtime ---
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void RegisterBiome(const FEng_BiomeData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void InitializeDefaultBiomes();

    // --- Debug ---
    UFUNCTION(CallInEditor, Category = "Debug")
    void DrawBiomeBoundaries();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TMap<EEng_BiomeType, FEng_BiomeData> BiomeMap;
};
