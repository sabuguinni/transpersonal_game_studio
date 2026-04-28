#pragma once

#include "CoreMinimal.h"
#include "PCGElement.h"
#include "PCGSettings.h"
#include "PCGContext.h"
#include "Data/PCGPointData.h"
#include "Data/PCGSpatialData.h"
#include "PCGBuildingGenerator.generated.h"

UENUM(BlueprintType)
enum class EPCGBuildingDensity : uint8
{
    Sparse               UMETA(DisplayName = "Sparse (1-2 per km²)"),
    Low                  UMETA(DisplayName = "Low (3-5 per km²)"),
    Medium               UMETA(DisplayName = "Medium (6-10 per km²)"),
    High                 UMETA(DisplayName = "High (11-15 per km²)"),
    Dense                UMETA(DisplayName = "Dense (16+ per km²)")
};

UENUM(BlueprintType)
enum class EPCGBuildingDistribution : uint8
{
    Random               UMETA(DisplayName = "Random"),
    Clustered            UMETA(DisplayName = "Clustered"),
    Linear               UMETA(DisplayName = "Linear (along paths)"),
    Defensive            UMETA(DisplayName = "Defensive (high ground)"),
    Resource             UMETA(DisplayName = "Resource-based"),
    Ritual               UMETA(DisplayName = "Ritual sites")
};

/**
 * PCG Building Generator Settings
 * Configures procedural placement of prehistoric buildings using PCG
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPCGBuildingGeneratorSettings : public UPCGSettings
{
    GENERATED_BODY()

public:
    UPCGBuildingGeneratorSettings();

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Building Generation")
    EPCGBuildingDensity BuildingDensity;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Building Generation")
    EPCGBuildingDistribution DistributionPattern;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Building Generation", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
    float MinDistanceBetweenBuildings;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Building Generation", meta = (ClampMin = "0.0", ClampMax = "45.0"))
    float MaxSlopeAngle;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Building Generation", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float MinDistanceFromWater;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Building Generation")
    bool bAvoidSteepTerrain;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Building Generation")
    bool bPreferHighGround;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Building Generation")
    bool bClusterAroundResources;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Building Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AbandonmentProbability;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Building Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DeteriorationVariation;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Building Styles")
    TMap<FString, float> StyleProbabilities;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Building Purposes")
    TMap<FString, float> PurposeProbabilities;

protected:
#if WITH_EDITOR
    virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCGBuildingGeneratorSettings", "NodeTitle", "Generate Prehistoric Buildings"); }
#endif

public:
    virtual TArray<FPCGPinProperties> InputPinProperties() const override;
    virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
    virtual UPCGNode* CreateNode() const override;

protected:
    virtual FPCGElementPtr CreateElement() const override;
};

/**
 * PCG Building Generator Element
 * Executes the building placement logic
 */
class TRANSPERSONALGAME_API FPCGBuildingGeneratorElement : public IPCGElement
{
public:
    virtual FPCGContext* Initialize(const FPCGDataCollection& InputData, TWeakObjectPtr<UPCGComponent> SourceComponent, const UPCGNode* Node) override;
    virtual bool ExecuteInternal(FPCGContext* Context) const override;

protected:
    bool GenerateBuildingPoints(FPCGContext* Context, const UPCGBuildingGeneratorSettings* Settings, const UPCGSpatialData* SpatialData, UPCGData* OutputData) const;
    bool IsValidBuildingLocation(const FVector& Location, const UPCGBuildingGeneratorSettings* Settings, const UPCGSpatialData* SpatialData) const;
    float CalculateLocationScore(const FVector& Location, const UPCGBuildingGeneratorSettings* Settings, const UPCGSpatialData* SpatialData) const;
    FString SelectBuildingStyle(const UPCGBuildingGeneratorSettings* Settings) const;
    FString SelectBuildingPurpose(const UPCGBuildingGeneratorSettings* Settings) const;
    void ApplyDistributionPattern(TArray<FVector>& Locations, EPCGBuildingDistribution Pattern, const UPCGSpatialData* SpatialData) const;
    void CreateClusteredDistribution(TArray<FVector>& Locations, const UPCGSpatialData* SpatialData) const;
    void CreateLinearDistribution(TArray<FVector>& Locations, const UPCGSpatialData* SpatialData) const;
    void CreateDefensiveDistribution(TArray<FVector>& Locations, const UPCGSpatialData* SpatialData) const;
};