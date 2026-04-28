// Architecture & Interior Agent — Transpersonal Game Studio
// PROD_JURASSIC_001 — PCG Architecture Generator
// Agent #07 — Architecture & Interior Agent

#pragma once

#include "CoreMinimal.h"
#include "PCGElement.h"
#include "PCGSettings.h"
#include "PCGContext.h"
#include "ArchitectureTypes.h"
#include "PCG_ArchitectureGenerator.generated.h"

/**
 * PCG Node that generates prehistoric architecture based on environmental conditions
 * Integrates with Environment Artist output to place contextually appropriate structures
 */
UCLASS(BlueprintType, ClassGroup=(Procedural))
class TRANSPERSONALGAME_API UPCGArchitectureSettings : public UPCGSettings
{
    GENERATED_BODY()

public:
    UPCGArchitectureSettings();

    // Architecture Generation Parameters
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Architecture Generation")
    float StructureDensity = 0.1f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Architecture Generation")
    float MinDistanceBetweenStructures = 500.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Architecture Generation")
    float MaxDistanceBetweenStructures = 2000.0f;

    // Environmental Influence
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Environmental Influence")
    float WaterProximityInfluence = 1.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Environmental Influence")
    float ElevationInfluence = 0.8f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Environmental Influence")
    float VegetationDensityInfluence = 0.6f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Environmental Influence")
    float SlopeInfluence = 0.4f;

    // Architecture Types Weights
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Structure Types")
    float ShelterWeight = 0.4f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Structure Types")
    float DefensiveWeight = 0.2f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Structure Types")
    float UtilityWeight = 0.3f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Structure Types")
    float AbandonedWeight = 0.1f;

    // Storytelling Parameters
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Narrative")
    float AbandonmentChance = 0.3f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Narrative")
    float PartialDestructionChance = 0.2f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Narrative")
    float OvergrowthChance = 0.4f;

    // Data Asset Reference
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
    TSoftObjectPtr<UArchitectureDataAsset> ArchitectureData;

protected:
    virtual TArray<FPCGPinProperties> InputPinProperties() const override;
    virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
    virtual UPCGNode* CreateNode() const override;

#if WITH_EDITOR
    virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCGArchitectureSettings", "NodeTitle", "Generate Architecture"); }
    virtual FText GetNodeTooltipText() const override;
#endif

protected:
    virtual FPCGElementPtr CreateElement() const override;
};

/**
 * PCG Element that executes architecture generation
 */
class TRANSPERSONALGAME_API FPCGArchitectureElement : public IPCGElement
{
public:
    virtual FPCGContext* Initialize(const FPCGDataCollection& InputData, TWeakObjectPtr<UPCGComponent> SourceComponent, const UPCGNode* Node) override;
    virtual bool ExecuteInternal(FPCGContext* Context) const override;

protected:
    // Core generation functions
    bool GenerateArchitecturePoints(FPCGContext* Context, const UPCGArchitectureSettings* Settings) const;
    
    // Environmental analysis
    float AnalyzeEnvironmentalSuitability(const FVector& Location, const UPCGArchitectureSettings* Settings, FPCGContext* Context) const;
    EArchitectureType DetermineArchitectureType(const FVector& Location, const UPCGArchitectureSettings* Settings, FPCGContext* Context) const;
    EArch_ConstructionMaterial DeterminePrimaryMaterial(const FVector& Location, FPCGContext* Context) const;
    
    // Narrative generation
    void ApplyNarrativeElements(FPCGPoint& Point, const UPCGArchitectureSettings* Settings, FPCGContext* Context) const;
    void GenerateInteriorLayout(FPCGPoint& Point, EArchitectureType ArchType, const UPCGArchitectureSettings* Settings) const;
    
    // Utility functions
    bool IsValidPlacementLocation(const FVector& Location, const TArray<FPCGPoint>& ExistingPoints, float MinDistance) const;
    FRotator CalculateOptimalOrientation(const FVector& Location, FPCGContext* Context) const;
};