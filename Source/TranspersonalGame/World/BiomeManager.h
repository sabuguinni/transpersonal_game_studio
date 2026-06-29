#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Cretaceous Forest"),
    Savanna     UMETA(DisplayName = "Open Savanna"),
    Wetland     UMETA(DisplayName = "River Wetland"),
    Volcanic    UMETA(DisplayName = "Volcanic Badlands"),
    Plains      UMETA(DisplayName = "Open Plains"),
    Coastal     UMETA(DisplayName = "Coastal Shore"),
    None        UMETA(DisplayName = "Undefined")
};

USTRUCT(BlueprintType)
struct FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FoliageDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurSpawnWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeDebugColor = FLinearColor::White;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Query which biome a world position belongs to
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    // Get full biome data for a world position
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeData GetBiomeDataAtLocation(FVector WorldLocation) const;

    // Get biome data by type
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeData GetBiomeDataByType(EEng_BiomeType BiomeType) const;

    // Register a biome zone (called by world generator)
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiomeZone(FVector Center, float Radius, EEng_BiomeType BiomeType);

    // Get all registered biome zones count
    UFUNCTION(BlueprintCallable, Category = "Biome")
    int32 GetBiomeZoneCount() const;

    // Debug: draw biome boundaries in viewport
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Debug")
    void DrawBiomeBoundaries();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome|Config")
    float BiomeBlendRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome|Config")
    bool bShowDebugBiomes = false;

private:
    struct FEng_BiomeZone
    {
        FVector Center;
        float Radius;
        EEng_BiomeType BiomeType;
    };

    TArray<FEng_BiomeZone> BiomeZones;
    TMap<EEng_BiomeType, FEng_BiomeData> BiomeDataMap;

    void InitializeBiomeData();
    EEng_BiomeType ClassifyLocationByNoise(FVector WorldLocation) const;
};
