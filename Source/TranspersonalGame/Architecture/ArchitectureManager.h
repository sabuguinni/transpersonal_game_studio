#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "ArchitectureTypes.h"
#include "ArchitectureManager.generated.h"

class AArch_StructureActor;
class UArch_StructureComponent;

/**
 * World subsystem that manages all architectural structures in the game world
 * Handles procedural placement, settlement generation, and structural integrity
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UArchitectureManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // World subsystem interface
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    /**
     * Generate a prehistoric settlement at the specified location
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool GenerateSettlement(const FVector& CenterLocation, int32 PopulationSize = 20, float Radius = 1000.0f);

    /**
     * Place a single structure at the specified location
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    AArch_StructureActor* PlaceStructure(const FArch_StructureData& StructureData);

    /**
     * Find suitable locations for structure placement based on terrain
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FVector> FindBuildableLocations(const FVector& SearchCenter, float SearchRadius, int32 MaxLocations = 10);

    /**
     * Generate interior furnishings for a structure
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FArch_InteriorFurnishing> GenerateInteriorFurnishings(EArch_StructureType StructureType, EArch_InteriorLayout LayoutType);

    /**
     * Update structural integrity over time (weathering, damage)
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateStructuralIntegrity(float DeltaTime);

    /**
     * Get all structures within a radius
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AArch_StructureActor*> GetStructuresInRadius(const FVector& Location, float Radius);

    /**
     * Generate ruins from existing structures
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ConvertToRuins(AArch_StructureActor* Structure, float RuinLevel = 0.7f);

    /**
     * Create a ritual stone circle
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    AArch_StructureActor* CreateRitualCircle(const FVector& Location, float Radius = 500.0f, int32 StoneCount = 12);

    /**
     * Generate cave dwelling interior
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetupCaveDwelling(AArch_StructureActor* CaveStructure);

protected:
    /**
     * All active structures in the world
     */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<AArch_StructureActor*> ActiveStructures;

    /**
     * Settlement layouts for tracking communities
     */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FArch_SettlementLayout> Settlements;

    /**
     * Timer for structural integrity updates
     */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    float IntegrityUpdateTimer = 0.0f;

    /**
     * How often to update structural integrity (in seconds)
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Settings")
    float IntegrityUpdateInterval = 60.0f;

    /**
     * Base weathering rate per minute
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Settings")
    float WeatheringRate = 0.001f;

    /**
     * Maximum number of structures allowed in world
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Settings")
    int32 MaxStructures = 500;

private:
    /**
     * Validate location for structure placement
     */
    bool IsValidBuildLocation(const FVector& Location, EArch_StructureType StructureType);

    /**
     * Calculate structure spacing based on type
     */
    float GetMinimumStructureSpacing(EArch_StructureType StructureType);

    /**
     * Generate structure data for settlement
     */
    FArch_StructureData GenerateStructureForSettlement(const FVector& SettlementCenter, float SettlementRadius, int32 PopulationSize);

    /**
     * Apply weathering effects to structure
     */
    void ApplyWeathering(AArch_StructureActor* Structure, float WeatheringAmount);

    /**
     * Clean up destroyed or invalid structures
     */
    void CleanupInvalidStructures();
};