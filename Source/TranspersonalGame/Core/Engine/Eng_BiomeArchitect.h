#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../../SharedTypes.h"
#include "Eng_BiomeArchitect.generated.h"

/**
 * Engine Architect's Biome System Architecture
 * Defines the core biome structure and environmental rules
 * for the prehistoric survival world
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UEng_BiomeArchitect : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_BiomeArchitect();

    // Core biome definitions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Architecture")
    TArray<FBiomeDefinition> BiomeDefinitions;

    // Environmental constraints per biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Rules")
    TMap<EBiomeType, FEnvironmentalConstraints> BiomeConstraints;

    // Dinosaur distribution rules
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Distribution")
    TMap<EBiomeType, TArray<EDinosaurSpecies>> AllowedSpeciesPerBiome;

    // Performance limits per biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Limits")
    int32 MaxActorsPerBiome = 4000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Limits")
    int32 MaxDinosaursPerBiome = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Limits")
    int32 MaxPropsPerBiome = 1000;

    // Architecture validation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateBiomeConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FBiomeDefinition GetBiomeDefinition(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CanSpawnSpeciesInBiome(EDinosaurSpecies Species, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FVector GetBiomeCenter(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    EBiomeType GetBiomeAtLocation(FVector WorldLocation);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActorCountInBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsWithinPerformanceLimits(EBiomeType BiomeType);

protected:
    virtual void BeginPlay() override;

    // Initialize default biome configurations
    void InitializeDefaultBiomes();

    // Setup environmental constraints
    void SetupEnvironmentalConstraints();

    // Configure species distribution
    void ConfigureSpeciesDistribution();

private:
    // Biome center coordinates (from memory)
    static const FVector SavannaCenter;
    static const FVector SwampCenter;
    static const FVector ForestCenter;
    static const FVector DesertCenter;
    static const FVector MountainCenter;

    // Biome radius for location detection
    static const float BiomeRadius;
};