#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "../SharedTypes.h"
#include "ArchitectureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_BuildingType : uint8
{
    None            UMETA(DisplayName = "None"),
    Hut             UMETA(DisplayName = "Stone Age Hut"),
    CaveDwelling    UMETA(DisplayName = "Cave Dwelling"),
    StoneCircle     UMETA(DisplayName = "Stone Circle"),
    UndergroundShelter UMETA(DisplayName = "Underground Shelter"),
    TribalHall      UMETA(DisplayName = "Tribal Hall"),
    StorageHut      UMETA(DisplayName = "Storage Hut"),
    CraftingArea    UMETA(DisplayName = "Crafting Area"),
    DefensiveWall   UMETA(DisplayName = "Defensive Wall")
};

UENUM(BlueprintType)
enum class EArch_StructuralMaterial : uint8
{
    Stone           UMETA(DisplayName = "Stone"),
    Wood            UMETA(DisplayName = "Wood"),
    AnimalHide      UMETA(DisplayName = "Animal Hide"),
    Thatch          UMETA(DisplayName = "Thatch"),
    Earth           UMETA(DisplayName = "Earth/Mud"),
    Bone            UMETA(DisplayName = "Bone"),
    Mixed           UMETA(DisplayName = "Mixed Materials")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_BuildingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    EArch_BuildingType BuildingType = EArch_BuildingType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    EArch_StructuralMaterial PrimaryMaterial = EArch_StructuralMaterial::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FVector Dimensions = FVector(500.0f, 500.0f, 300.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    int32 Capacity = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    bool bHasFireplace = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    bool bHasStorage = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    TArray<FString> InteriorFeatures;

    FArch_BuildingData()
    {
        InteriorFeatures.Add("Fire Pit");
        InteriorFeatures.Add("Sleeping Area");
        InteriorFeatures.Add("Tool Storage");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_SettlementLayout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    int32 MaxBuildings = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    bool bHasCentralFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    bool bHasDefensiveWalls = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    TArray<FArch_BuildingData> Buildings;
};

/**
 * Architecture Manager - Handles all building placement, settlement generation, and structural systems
 * Responsible for creating authentic Stone Age architecture with proper materials and layouts
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArchitectureManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UArchitectureManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Building Generation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    class AArch_BuildingActor* SpawnBuilding(EArch_BuildingType BuildingType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateSettlement(FVector CenterLocation, int32 BuildingCount = 8);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateTribalVillage(FVector Location, float Radius = 1500.0f);

    // Interior Systems
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void PopulateInterior(class AArch_BuildingActor* Building, EArch_BuildingType BuildingType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void AddInteriorFeature(class AArch_BuildingActor* Building, const FString& FeatureName, FVector RelativeLocation);

    // Structural Analysis
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float CalculateStructuralIntegrity(const FArch_BuildingData& BuildingData);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateBuildingPlacement(FVector Location, EArch_BuildingType BuildingType);

    // Material Systems
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    class UMaterialInterface* GetMaterialForType(EArch_StructuralMaterial MaterialType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeatheringEffects(class AArch_BuildingActor* Building, float WeatheringAmount = 0.3f);

    // Settlement Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<class AArch_BuildingActor*> GetBuildingsInRadius(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FArch_SettlementLayout GenerateSettlementLayout(FVector Center, int32 BuildingCount);

    // Debug and Testing
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture|Debug")
    void DebugSpawnTestBuilding();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture|Debug")
    void DebugGenerateTestSettlement();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture|Debug")
    void ClearAllBuildings();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<class AArch_BuildingActor*> ManagedBuildings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_SettlementLayout CurrentSettlement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TMap<EArch_StructuralMaterial, TSoftObjectPtr<UMaterialInterface>> MaterialLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MinBuildingDistance = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bAutoPopulateInteriors = true;

private:
    void InitializeMaterialLibrary();
    FVector FindValidBuildingLocation(FVector PreferredLocation, float SearchRadius = 1000.0f);
    void SetupBuildingDefaults(class AArch_BuildingActor* Building, EArch_BuildingType BuildingType);
};