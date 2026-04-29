#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "ArchitectureManager.generated.h"

// Prehistoric structure types
UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    None            UMETA(DisplayName = "None"),
    CircularHut     UMETA(DisplayName = "Circular Hut"),
    CaveDwelling    UMETA(DisplayName = "Cave Dwelling"),
    RockShelter     UMETA(DisplayName = "Rock Shelter"),
    ElevatedPlatform UMETA(DisplayName = "Elevated Platform"),
    StoragePit      UMETA(DisplayName = "Storage Pit"),
    FirePit         UMETA(DisplayName = "Fire Pit"),
    WaterCatchment  UMETA(DisplayName = "Water Catchment")
};

// Construction materials for prehistoric structures
UENUM(BlueprintType)
enum class EArch_MaterialType : uint8
{
    Stone           UMETA(DisplayName = "Stone"),
    Wood            UMETA(DisplayName = "Wood"),
    Thatch          UMETA(DisplayName = "Thatch"),
    AnimalHide      UMETA(DisplayName = "Animal Hide"),
    Clay            UMETA(DisplayName = "Clay"),
    Bone            UMETA(DisplayName = "Bone"),
    Vine            UMETA(DisplayName = "Vine")
};

// Structure condition states
UENUM(BlueprintType)
enum class EArch_StructureCondition : uint8
{
    New             UMETA(DisplayName = "New"),
    Good            UMETA(DisplayName = "Good"),
    Weathered       UMETA(DisplayName = "Weathered"),
    Damaged         UMETA(DisplayName = "Damaged"),
    Ruined          UMETA(DisplayName = "Ruined"),
    Abandoned       UMETA(DisplayName = "Abandoned")
};

// Prehistoric structure data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_MaterialType PrimaryMaterial = EArch_MaterialType::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureCondition Condition = EArch_StructureCondition::Good;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructureRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructureHeight = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 OccupantCapacity = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasStorage = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsWeatherproof = false;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::CircularHut;
        PrimaryMaterial = EArch_MaterialType::Stone;
        Condition = EArch_StructureCondition::Good;
        StructureRadius = 300.0f;
        StructureHeight = 250.0f;
        OccupantCapacity = 2;
        bHasFirePit = true;
        bHasStorage = true;
        bIsWeatherproof = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitectureManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitectureManager();

protected:
    virtual void BeginPlay() override;

    // Root component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Main structure mesh
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StructureMesh;

    // Interior components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FirePitMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StorageMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* BeddingMesh;

    // Structure configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_StructureData StructureData;

    // Material references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* StoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WoodMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* ThatchMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* HideMaterial;

public:
    virtual void Tick(float DeltaTime) override;

    // Structure management functions
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetStructureMaterial(EArch_MaterialType NewMaterial);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetStructureCondition(EArch_StructureCondition NewCondition);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateStructureAppearance();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void AddInteriorElement(const FString& ElementType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RemoveInteriorElement(const FString& ElementType);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CanAccommodateOccupants(int32 NumOccupants) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float GetStructureIntegrity() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsWeatherproof() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FVector GetInteriorCenter() const;

    // Construction functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void ConstructCircularHut();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void ConstructRockShelter();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void ConstructElevatedPlatform();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void AddFirePit();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void AddStorageArea();

private:
    // Internal helper functions
    void InitializeComponents();
    void SetupDefaultMaterials();
    void ApplyMaterialToStructure();
    void UpdateInteriorLayout();
    void CalculateStructureDimensions();
    
    // Interior element management
    TArray<UStaticMeshComponent*> InteriorElements;
    
    // Structure integrity tracking
    float StructureIntegrity = 1.0f;
    float WeatheringRate = 0.001f;
};