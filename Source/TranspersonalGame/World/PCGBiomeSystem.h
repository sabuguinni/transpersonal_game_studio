#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCGBiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    RiverDelta     UMETA(DisplayName = "River Delta"),
    DenseForest    UMETA(DisplayName = "Dense Forest"),
    OpenPlains     UMETA(DisplayName = "Open Plains"),
    RockyHighland  UMETA(DisplayName = "Rocky Highland"),
    Volcanic       UMETA(DisplayName = "Volcanic"),
    Swamp          UMETA(DisplayName = "Swamp")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterCoverage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeTint = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FWorld_RiverSegment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FVector StartPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FVector EndPoint = FVector(0, 1000, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float Width = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float Depth = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float FlowSpeed = 100.0f;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API APCGBiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    APCGBiomeSystem();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "PCG|Biome")
    void GenerateBiomeZones();

    UFUNCTION(BlueprintCallable, Category = "PCG|Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "PCG|Biome")
    float GetBiomeBlendWeight(const FVector& WorldLocation, EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "PCG|River")
    void GenerateRiverNetwork();

    UFUNCTION(BlueprintCallable, Category = "PCG|River")
    bool IsLocationInRiver(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "PCG|Vegetation")
    void ScatterVegetationForBiome(EWorld_BiomeType BiomeType, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "PCG|Biome")
    const TArray<FWorld_BiomeZone>& GetAllBiomeZones() const { return BiomeZones; }

    UFUNCTION(BlueprintCallable, Category = "PCG|River")
    const TArray<FWorld_RiverSegment>& GetRiverSegments() const { return RiverSegments; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Biome")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|River")
    TArray<FWorld_RiverSegment> RiverSegments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Settings")
    int32 RandomSeed = 42;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Settings")
    float WorldExtent = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Settings")
    bool bAutoGenerateOnBeginPlay = true;

private:
    void InitializeDefaultBiomes();
    void InitializeDefaultRivers();
    float CalculateDistanceToBiomeEdge(const FVector& Location, const FWorld_BiomeZone& Zone) const;
};
