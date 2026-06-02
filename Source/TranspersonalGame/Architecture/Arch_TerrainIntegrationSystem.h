#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Arch_TerrainIntegrationSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTerrainIntegration, Log, All);

/**
 * Terrain adaptation data for architectural structures
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_TerrainAdaptation
{
    GENERATED_BODY()

    /** Terrain slope at structure location */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float TerrainSlope = 0.0f;

    /** Terrain height variation within structure footprint */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightVariation = 0.0f;

    /** Soil composition type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FString SoilType = TEXT("Clay");

    /** Drainage quality (0.0 = poor, 1.0 = excellent) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float DrainageQuality = 0.5f;

    /** Foundation stability rating */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float FoundationStability = 0.8f;

    /** Erosion risk level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float ErosionRisk = 0.3f;

    /** Vegetation density affecting foundation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float VegetationDensity = 0.4f;

    /** Water table depth in meters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float WaterTableDepth = 5.0f;

    FArch_TerrainAdaptation()
    {
        TerrainSlope = 0.0f;
        HeightVariation = 0.0f;
        SoilType = TEXT("Clay");
        DrainageQuality = 0.5f;
        FoundationStability = 0.8f;
        ErosionRisk = 0.3f;
        VegetationDensity = 0.4f;
        WaterTableDepth = 5.0f;
    }
};

/**
 * Foundation requirements for different structure types
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_FoundationRequirements
{
    GENERATED_BODY()

    /** Structure type this applies to */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    FString StructureType = TEXT("Basic");

    /** Minimum foundation depth required */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    float MinFoundationDepth = 0.5f;

    /** Maximum acceptable terrain slope */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    float MaxTerrainSlope = 15.0f;

    /** Minimum drainage quality required */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    float MinDrainageQuality = 0.3f;

    /** Required soil stability */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    float RequiredStability = 0.6f;

    /** Foundation material type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    FString FoundationMaterial = TEXT("Stone");

    /** Reinforcement requirements */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    bool bRequiresReinforcement = false;

    FArch_FoundationRequirements()
    {
        StructureType = TEXT("Basic");
        MinFoundationDepth = 0.5f;
        MaxTerrainSlope = 15.0f;
        MinDrainageQuality = 0.3f;
        RequiredStability = 0.6f;
        FoundationMaterial = TEXT("Stone");
        bRequiresReinforcement = false;
    }
};

/**
 * Terrain modification data for structure placement
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_TerrainModification
{
    GENERATED_BODY()

    /** Location of terrain modification */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modification")
    FVector ModificationLocation = FVector::ZeroVector;

    /** Radius of terrain modification */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modification")
    float ModificationRadius = 5.0f;

    /** Type of modification (Level, Raise, Lower, Smooth) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modification")
    FString ModificationType = TEXT("Level");

    /** Intensity of modification (0.0 to 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modification")
    float ModificationIntensity = 0.5f;

    /** Target height for leveling operations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modification")
    float TargetHeight = 0.0f;

    /** Feathering distance for smooth transitions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modification")
    float FeatheringDistance = 2.0f;

    /** Whether modification affects vegetation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modification")
    bool bAffectsVegetation = true;

    FArch_TerrainModification()
    {
        ModificationLocation = FVector::ZeroVector;
        ModificationRadius = 5.0f;
        ModificationType = TEXT("Level");
        ModificationIntensity = 0.5f;
        TargetHeight = 0.0f;
        FeatheringDistance = 2.0f;
        bAffectsVegetation = true;
    }
};

/**
 * Terrain Integration System for Architecture
 * Manages how architectural structures adapt to and modify terrain
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_TerrainIntegrationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_TerrainIntegrationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Analyze terrain at specified location for structure placement */
    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    FArch_TerrainAdaptation AnalyzeTerrainAtLocation(const FVector& Location, float AnalysisRadius = 10.0f);

    /** Check if location is suitable for structure type */
    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    bool IsLocationSuitableForStructure(const FVector& Location, const FString& StructureType);

    /** Calculate required terrain modifications for structure placement */
    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    TArray<FArch_TerrainModification> CalculateRequiredModifications(const FVector& Location, const FString& StructureType);

    /** Apply terrain modifications for structure foundation */
    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    bool ApplyTerrainModifications(const TArray<FArch_TerrainModification>& Modifications);

    /** Get foundation requirements for structure type */
    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    FArch_FoundationRequirements GetFoundationRequirements(const FString& StructureType);

    /** Register new foundation requirements */
    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    void RegisterFoundationRequirements(const FString& StructureType, const FArch_FoundationRequirements& Requirements);

    /** Calculate optimal structure orientation based on terrain */
    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    FRotator CalculateOptimalOrientation(const FVector& Location, const FString& StructureType);

    /** Assess terrain stability for long-term structure placement */
    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    float AssessLongTermStability(const FVector& Location, float AssessmentRadius = 15.0f);

    /** Find best nearby location for structure if current location unsuitable */
    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    FVector FindAlternativeLocation(const FVector& PreferredLocation, const FString& StructureType, float SearchRadius = 50.0f);

    /** Update terrain integration for existing structures */
    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    void UpdateExistingStructureIntegration();

    /** Get terrain slope at location */
    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    float GetTerrainSlope(const FVector& Location, float SampleRadius = 5.0f);

    /** Get soil composition at location */
    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    FString GetSoilComposition(const FVector& Location);

    /** Calculate drainage quality at location */
    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    float CalculateDrainageQuality(const FVector& Location);

    /** Assess erosion risk at location */
    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    float AssessErosionRisk(const FVector& Location);

    /** Set terrain modification intensity */
    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    void SetModificationIntensity(float NewIntensity);

    /** Enable or disable automatic terrain adaptation */
    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    void SetAutoAdaptationEnabled(bool bEnabled);

protected:
    /** Foundation requirements for different structure types */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation Requirements")
    TArray<FArch_FoundationRequirements> FoundationRequirements;

    /** Active terrain modifications */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Modifications")
    TArray<FArch_TerrainModification> ActiveModifications;

    /** Terrain modification intensity (0.0 to 2.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float ModificationIntensity = 1.0f;

    /** Maximum terrain slope for automatic placement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxAutoPlacementSlope = 20.0f;

    /** Minimum foundation stability required */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MinFoundationStability = 0.6f;

    /** Whether to automatically adapt structures to terrain */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoAdaptationEnabled = true;

    /** Whether to modify terrain for better structure placement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAllowTerrainModification = true;

    /** Update interval for terrain integration checks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float UpdateInterval = 5.0f;

private:
    /** Initialize foundation requirements for standard structure types */
    void InitializeFoundationRequirements();

    /** Sample terrain height at multiple points around location */
    TArray<float> SampleTerrainHeights(const FVector& Center, float Radius, int32 SampleCount = 8);

    /** Calculate terrain normal at location */
    FVector CalculateTerrainNormal(const FVector& Location, float SampleRadius = 2.0f);

    /** Assess soil stability based on composition and moisture */
    float AssessSoilStability(const FString& SoilType, float Moisture);

    /** Timer handle for periodic updates */
    FTimerHandle UpdateTimerHandle;

    /** Last update time */
    float LastUpdateTime = 0.0f;
};