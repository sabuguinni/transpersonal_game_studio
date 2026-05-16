#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "ArchitecturalStructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Shelter         UMETA(DisplayName = "Shelter"),
    StorageHut      UMETA(DisplayName = "Storage Hut"),
    Watchtower      UMETA(DisplayName = "Watchtower"),
    CraftingArea    UMETA(DisplayName = "Crafting Area"),
    FirePit         UMETA(DisplayName = "Fire Pit"),
    DefensiveWall   UMETA(DisplayName = "Defensive Wall"),
    Bridge          UMETA(DisplayName = "Bridge"),
    Ruins           UMETA(DisplayName = "Ancient Ruins")
};

UENUM(BlueprintType)
enum class EArch_MaterialType : uint8
{
    Stone           UMETA(DisplayName = "Stone"),
    Wood            UMETA(DisplayName = "Wood"),
    Bone            UMETA(DisplayName = "Bone"),
    Hide            UMETA(DisplayName = "Hide"),
    Clay            UMETA(DisplayName = "Clay"),
    Thatch          UMETA(DisplayName = "Thatch")
};

USTRUCT(BlueprintType)
struct FArch_StructureConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::Shelter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_MaterialType PrimaryMaterial = EArch_MaterialType::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Dimensions = FVector(300.0f, 300.0f, 200.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 Durability = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasInterior = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bProvideShelter = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatherResistance = 0.8f;

    FArch_StructureConfig()
    {
        StructureType = EArch_StructureType::Shelter;
        PrimaryMaterial = EArch_MaterialType::Stone;
        Dimensions = FVector(300.0f, 300.0f, 200.0f);
        Durability = 100;
        bHasInterior = true;
        bProvideShelter = true;
        WeatherResistance = 0.8f;
    }
};

USTRUCT(BlueprintType)
struct FArch_InteriorLayout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> SleepingAreas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> StorageAreas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector FirePitLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> CraftingStations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float InteriorTemperature = 20.0f;

    FArch_InteriorLayout()
    {
        SleepingAreas.Empty();
        StorageAreas.Empty();
        FirePitLocation = FVector::ZeroVector;
        CraftingStations.Empty();
        bHasFirePit = true;
        InteriorTemperature = 20.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitecturalStructureManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitecturalStructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Configuration")
    FArch_StructureConfig StructureConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Configuration")
    FArch_InteriorLayout InteriorLayout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Integration")
    EBiomeType AssociatedBiome = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    UStaticMesh* StoneShelterMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    UStaticMesh* WoodShelterMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    UMaterialInterface* StoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    UMaterialInterface* WoodMaterial;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void InitializeStructure();

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void SetPrimaryMaterial(EArch_MaterialType NewMaterial);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void ApplyWeatherDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void RepairStructure(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void SetupInteriorLayout();

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void AddSleepingArea(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void AddStorageArea(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void AddCraftingStation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void AdaptToBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Utility", CallInEditor)
    void SpawnInBiome();

    UFUNCTION(BlueprintPure, Category = "Structure Status")
    float GetStructuralIntegrity() const;

    UFUNCTION(BlueprintPure, Category = "Structure Status")
    bool IsStructureIntact() const;

    UFUNCTION(BlueprintPure, Category = "Interior Status")
    float GetInteriorTemperature() const;

    UFUNCTION(BlueprintPure, Category = "Interior Status")
    bool ProvidesAdequateShelter() const;

private:
    void UpdateStructureMesh();
    void UpdateMaterials();
    void CalculateInteriorTemperature();
    void ApplyBiomeAdaptations();
    FVector GetBiomeSpawnLocation(EBiomeType BiomeType) const;

    float CurrentDurability;
    float LastWeatherCheckTime;
    bool bIsInitialized;
};