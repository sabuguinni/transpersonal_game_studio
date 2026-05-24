#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "World_CretaceousLandscapeCreator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeTerrainConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float ElevationBase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float TerrainRoughness;

    FWorld_BiomeTerrainConfig()
    {
        BiomeName = TEXT("DefaultBiome");
        Location = FVector::ZeroVector;
        Scale = FVector(1000.0f, 1000.0f, 100.0f);
        BiomeType = EBiomeType::Temperate;
        ElevationBase = 0.0f;
        TerrainRoughness = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WaterBodyConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Config")
    FString WaterBodyName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Config")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Config")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Config")
    EWaterType WaterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Config")
    float WaterDepth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Config")
    float FlowRate;

    FWorld_WaterBodyConfig()
    {
        WaterBodyName = TEXT("DefaultWater");
        Location = FVector::ZeroVector;
        Scale = FVector(1000.0f, 1000.0f, 1.0f);
        WaterType = EWaterType::River;
        WaterDepth = 100.0f;
        FlowRate = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_CretaceousLandscapeCreator : public AActor
{
    GENERATED_BODY()

public:
    AWorld_CretaceousLandscapeCreator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Biome terrain configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Creation")
    TArray<FWorld_BiomeTerrainConfig> BiomeConfigurations;

    // Water body configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Creation")
    TArray<FWorld_WaterBodyConfig> WaterBodyConfigurations;

    // Main landscape settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Creation")
    FVector LandscapeSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Creation")
    int32 LandscapeResolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Creation")
    float GlobalElevationScale;

    // Created actors tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> CreatedBiomeActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> CreatedWaterActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    class ALandscape* MainLandscape;

public:
    // Core landscape creation functions
    UFUNCTION(BlueprintCallable, Category = "Landscape Creation")
    bool CreateCretaceousLandscape();

    UFUNCTION(BlueprintCallable, Category = "Landscape Creation")
    bool CreateBiomeTerrainActors();

    UFUNCTION(BlueprintCallable, Category = "Landscape Creation")
    bool CreateWaterBodies();

    UFUNCTION(BlueprintCallable, Category = "Landscape Creation")
    void SetupDefaultBiomeConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Landscape Creation")
    void SetupDefaultWaterConfiguration();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Landscape Creation")
    FVector GetBiomeCenter(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Landscape Creation")
    float GetBiomeElevation(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Landscape Creation")
    bool ValidateLandscapeCreation() const;

    // Editor-only functions
    UFUNCTION(CallInEditor, Category = "Landscape Creation")
    void CreateLandscapeInEditor();

    UFUNCTION(CallInEditor, Category = "Landscape Creation")
    void ClearAllCreatedActors();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetTotalCreatedActors() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FString GetLandscapeCreationReport() const;

private:
    // Internal creation helpers
    AActor* CreateBiomeTerrainActor(const FWorld_BiomeTerrainConfig& Config);
    AActor* CreateWaterBodyActor(const FWorld_WaterBodyConfig& Config);
    bool SetupTerrainMesh(AActor* TerrainActor, const FWorld_BiomeTerrainConfig& Config);
    bool SetupWaterMesh(AActor* WaterActor, const FWorld_WaterBodyConfig& Config);
    
    // Validation helpers
    bool IsValidBiomeConfiguration(const FWorld_BiomeTerrainConfig& Config) const;
    bool IsValidWaterConfiguration(const FWorld_WaterBodyConfig& Config) const;
};