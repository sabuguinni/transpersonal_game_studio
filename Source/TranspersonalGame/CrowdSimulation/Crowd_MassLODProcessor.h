#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassLODTypes.h"
#include "Crowd_SharedTypes.h"
#include "Crowd_MassLODProcessor.generated.h"

/**
 * Mass processor that manages Level of Detail (LOD) for crowd simulation entities
 * Dynamically adjusts LOD based on distance to player and entity importance
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassLODProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassLODProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

    /**
     * Calculate appropriate LOD level based on distance and species
     */
    EMassLOD::Type CalculateLODLevel(float Distance, ECrowd_DinosaurSpecies Species) const;

public:
    /**
     * Set custom LOD distances
     */
    UFUNCTION(BlueprintCallable, Category = "Crowd LOD")
    void SetLODDistances(const TArray<float>& NewDistances);

    /**
     * Get LOD distance for specific level
     */
    UFUNCTION(BlueprintPure, Category = "Crowd LOD")
    float GetLODDistance(int32 LODLevel) const;

protected:
    /**
     * Distance thresholds for LOD transitions
     * [0] = LOD High to Medium
     * [1] = LOD Medium to Low  
     * [2] = LOD Low to Off
     * [3] = LOD Off to Culled
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    TArray<float> LODDistances;

    /**
     * Query for entities with LOD and transform components
     */
    FMassEntityQuery EntityQuery;
};