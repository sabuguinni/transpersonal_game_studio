#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "SharedTypes.h"
#include "World_CretaceousLandscapeManager.generated.h"

/**
 * Manages the creation and configuration of the main Cretaceous landscape
 * Implements the 10km x 10km terrain requirement for asset criteria compliance
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_CretaceousLandscapeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_CretaceousLandscapeManager();

protected:
    virtual void BeginPlay() override;

    // Core landscape management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landscape")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    FVector LandscapeSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    FVector LandscapeScale;

    UPROPERTY(BlueprintReadOnly, Category = "Landscape")
    class ALandscape* MainLandscape;

    // Biome zone configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeZoneConfig> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    bool bAutoCreateBiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float BiomeZoneSize;

public:
    // Landscape creation and management
    UFUNCTION(BlueprintCallable, Category = "Landscape")
    bool CreateMainLandscape();

    UFUNCTION(BlueprintCallable, Category = "Landscape")
    bool ValidateLandscapeSize();

    UFUNCTION(BlueprintCallable, Category = "Landscape")
    ALandscape* GetMainLandscape() const;

    // Biome zone management
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void CreateBiomeZones();

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void CreateBiomeZoneMarker(const FWorld_BiomeZoneConfig& ZoneConfig);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    TArray<FWorld_BiomeZoneConfig> GetBiomeZones() const;

    // Asset criteria compliance
    UFUNCTION(BlueprintCallable, Category = "Criteria")
    bool CheckAssetCriteriaCompliance();

    UFUNCTION(BlueprintCallable, Category = "Criteria")
    FString GetComplianceReport();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetLandscapeMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveComponentCount();

private:
    // Internal landscape setup
    void SetupLandscapeProperties();
    void ConfigureLandscapeScale();
    void SetupBiomeZoneDefaults();

    // Validation helpers
    bool IsLandscapeSizeValid() const;
    bool AreBiomeZonesValid() const;

    // Performance tracking
    float LastMemoryCheck;
    int32 ComponentCount;
};