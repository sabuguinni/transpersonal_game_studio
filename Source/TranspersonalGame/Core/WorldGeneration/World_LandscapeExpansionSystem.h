#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "../SharedTypes.h"
#include "World_LandscapeExpansionSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWorld_LandscapeExpansion, Log, All);

UENUM(BlueprintType)
enum class EWorld_ExpansionState : uint8
{
    Planning        UMETA(DisplayName = "Planning"),
    ReadyToExpand   UMETA(DisplayName = "Ready To Expand"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Complete        UMETA(DisplayName = "Complete"),
    Failed          UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EWorld_TerrainType : uint8
{
    Grassland       UMETA(DisplayName = "Grassland"),
    Forest          UMETA(DisplayName = "Forest"),
    Desert          UMETA(DisplayName = "Desert"),
    Mountain        UMETA(DisplayName = "Mountain"),
    Wetland         UMETA(DisplayName = "Wetland"),
    Rocky           UMETA(DisplayName = "Rocky"),
    Coastal         UMETA(DisplayName = "Coastal")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BoundsMin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BoundsMax;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_TerrainType TerrainType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D ElevationRange;

    FWorld_BiomeDefinition()
    {
        BiomeName = TEXT("Unknown");
        CenterLocation = FVector::ZeroVector;
        BoundsMin = FVector::ZeroVector;
        BoundsMax = FVector::ZeroVector;
        TerrainType = EWorld_TerrainType::Grassland;
        ElevationRange = FVector2D(0.0f, 100.0f);
    }
};

/**
 * Manages landscape expansion to 200km2 with proper biome distribution
 * Handles terrain generation, biome boundaries, and coordinate management
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_LandscapeExpansionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_LandscapeExpansionSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Landscape expansion management
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void StartLandscapeExpansion();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void AssessCurrentLandscape();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    float GetExpansionProgress() const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FVector2D GetCurrentLandscapeSize() const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FVector2D GetTargetLandscapeSize() const;

    // Biome management
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FVector GetRandomLocationInBiome(const FString& BiomeName) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    bool IsLocationInBiome(const FVector& Location, const FString& BiomeName) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EWorld_TerrainType GetTerrainTypeAtLocation(const FVector& Location) const;

protected:
    // Core expansion properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape Expansion")
    FVector2D TargetLandscapeSize;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landscape Expansion")
    FVector2D CurrentLandscapeSize;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landscape Expansion")
    EWorld_ExpansionState CurrentExpansionState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landscape Expansion")
    float ExpansionProgress;

    // Biome definitions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biomes")
    TArray<FWorld_BiomeDefinition> BiomeDefinitions;

    // Internal state
    float ExpansionStartTime;

    // Core methods
    void InitializeBiomeDefinitions();
    void ExpandLandscapeToTarget();
    void UpdateExpansionProgress(float DeltaTime);
    void CompleteExpansion();
    void OnExpansionComplete();
    void CreateBiomeMarkers();
};