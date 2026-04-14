#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "WorldGenerationTypes.h"
#include "WorldGenerationManager.generated.h"

class UWorld_TerrainGenerator;
class UWorld_BiomeManager;
class UWorld_RiverGenerator;
class UWorld_SettlementPlacer;

/**
 * Main manager for procedural world generation
 * Coordinates terrain, biomes, rivers, and settlements
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorldGenerationManager : public AActor
{
    GENERATED_BODY()

public:
    AWorldGenerationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core generation components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Generation")
    UWorld_TerrainGenerator* TerrainGenerator;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Generation")
    UWorld_BiomeManager* BiomeManager;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Generation")
    UWorld_RiverGenerator* RiverGenerator;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Generation")
    UWorld_SettlementPlacer* SettlementPlacer;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    UDataTable* BiomeConfigTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FWorld_TerrainParams TerrainParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FWorld_RiverParams RiverParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 WorldSeed = 12345;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FVector2D WorldSize = FVector2D(50000.0f, 50000.0f);

    // Generation control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bAutoGenerate = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bGenerateTerrain = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bGenerateBiomes = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bGenerateRivers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bGenerateSettlements = true;

    // Generation functions
    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor)
    void GenerateWorld();

    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor)
    void GenerateTerrain();

    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor)
    void GenerateBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor)
    void GenerateRivers();

    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor)
    void GenerateSettlements();

    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor)
    void ClearWorld();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    float GetElevationAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    bool IsLocationNearWater(const FVector& Location, float SearchRadius = 1000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    TArray<FWorld_SettlementData> GetNearbySettlements(const FVector& Location, float SearchRadius = 5000.0f) const;

private:
    // Internal state
    bool bWorldGenerated = false;
    TArray<FWorld_SettlementData> GeneratedSettlements;

    // Helper functions
    void InitializeComponents();
    void SetupRandomSeed();
    bool ValidateConfiguration() const;
};