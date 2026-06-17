#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TerrainPhysicsManager.generated.h"

/**
 * Terrain Physics Manager - Manages physics materials and properties for different biome terrain types
 * Integrates with PCGWorldGenerator biome system to apply appropriate friction/restitution values
 * 
 * Biome Physics Properties:
 * - Savanna: Dry grass, moderate friction (0.7), low restitution (0.1)
 * - Forest: Soft forest floor, high friction (0.8), medium restitution (0.15)
 * - Swamp: Muddy terrain, low friction (0.3), very low restitution (0.05)
 * - Volcanic: Rocky surface, medium friction (0.6), medium restitution (0.2)
 * - Coastal: Sandy beach, medium-low friction (0.5), low restitution (0.1)
 */
UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ATerrainPhysicsManager : public AActor
{
    GENERATED_BODY()

public:
    ATerrainPhysicsManager();

    /** Map of biome name to physics material asset path */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TMap<FString, FString> BiomePhysicsMaterials;

    /** Apply physics material to landscape based on biome type */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyBiomePhysics(const FString& BiomeName, class ALandscape* Landscape);

    /** Get physics material for specific biome */
    UFUNCTION(BlueprintPure, Category = "Terrain Physics")
    class UPhysicalMaterial* GetBiomePhysicsMaterial(const FString& BiomeName) const;

    /** Initialize all biome physics materials */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain Physics")
    void InitializeBiomePhysics();

protected:
    virtual void BeginPlay() override;

private:
    /** Cache of loaded physics materials */
    UPROPERTY()
    TMap<FString, class UPhysicalMaterial*> LoadedPhysicsMaterials;
};
