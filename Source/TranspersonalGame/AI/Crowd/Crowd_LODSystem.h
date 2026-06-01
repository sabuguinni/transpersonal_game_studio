#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MassLODTypes.h"
#include "MassEntityTypes.h"
#include "SharedTypes.h"
#include "Crowd_LODSystem.generated.h"

/**
 * LOD (Level of Detail) system for crowd simulation
 * Manages performance by reducing detail for distant crowd entities
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowd_LODSystem : public UObject
{
    GENERATED_BODY()

public:
    UCrowd_LODSystem();

    // Initialize the LOD system
    UFUNCTION(BlueprintCallable, Category = "Crowd LOD")
    void Initialize();

    // Update LOD for all crowd entities based on distance from viewer
    UFUNCTION(BlueprintCallable, Category = "Crowd LOD")
    void UpdateLOD(const FVector& ViewerLocation);

    // Get current LOD level for an entity
    UFUNCTION(BlueprintCallable, Category = "Crowd LOD")
    ECrowd_LODLevel GetEntityLOD(FMassEntityHandle Entity) const;

    // Set LOD level for an entity
    UFUNCTION(BlueprintCallable, Category = "Crowd LOD")
    void SetEntityLOD(FMassEntityHandle Entity, ECrowd_LODLevel NewLOD);

    // Get total number of entities per LOD level
    UFUNCTION(BlueprintCallable, Category = "Crowd LOD")
    void GetLODCounts(int32& LOD0Count, int32& LOD1Count, int32& LOD2Count, int32& LOD3Count) const;

protected:
    // Calculate LOD level based on distance
    ECrowd_LODLevel CalculateLODFromDistance(float Distance) const;

    // Apply LOD settings to entity
    void ApplyLODToEntity(FMassEntityHandle Entity, ECrowd_LODLevel LODLevel);

    // Update entity visibility based on LOD
    void UpdateEntityVisibility(FMassEntityHandle Entity, ECrowd_LODLevel LODLevel);

    // Update entity update frequency based on LOD
    void UpdateEntityUpdateFrequency(FMassEntityHandle Entity, ECrowd_LODLevel LODLevel);

public:
    // LOD distance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD0Distance = 500.0f; // High detail - full simulation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD1Distance = 1500.0f; // Medium detail - reduced update rate

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD2Distance = 3000.0f; // Low detail - basic movement only

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD3Distance = 5000.0f; // Culled - no simulation

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxHighDetailEntities = 50; // Maximum entities at LOD0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxMediumDetailEntities = 200; // Maximum entities at LOD1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxLowDetailEntities = 1000; // Maximum entities at LOD2

    // Update frequencies per LOD level (in seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Update Frequency")
    float LOD0UpdateFrequency = 0.016f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Update Frequency")
    float LOD1UpdateFrequency = 0.1f; // 10 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Update Frequency")
    float LOD2UpdateFrequency = 0.5f; // 2 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Update Frequency")
    float LOD3UpdateFrequency = 2.0f; // 0.5 FPS

private:
    // Track entities by LOD level
    TMap<FMassEntityHandle, ECrowd_LODLevel> EntityLODMap;
    
    // Count entities per LOD level
    TArray<int32> LODCounts;
    
    // Last update time
    float LastUpdateTime = 0.0f;
    
    // Update interval for LOD calculations
    float LODUpdateInterval = 0.2f; // Update LOD 5 times per second

    // Helper to get current world time
    float GetWorldTime() const;
};