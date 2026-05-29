#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "Arch_DwellingTypes.generated.h"

UENUM(BlueprintType)
enum class EArch_DwellingType : uint8
{
    None            UMETA(DisplayName = "None"),
    CaveDwelling    UMETA(DisplayName = "Cave Dwelling"),
    TreePlatform    UMETA(DisplayName = "Tree Platform"),
    StoneCircle     UMETA(DisplayName = "Stone Circle"),
    CliffDwelling   UMETA(DisplayName = "Cliff Dwelling"),
    RockShelter     UMETA(DisplayName = "Rock Shelter"),
    BranchHut       UMETA(DisplayName = "Branch Hut")
};

UENUM(BlueprintType)
enum class EArch_ConstructionMaterial : uint8
{
    Stone           UMETA(DisplayName = "Stone"),
    Wood            UMETA(DisplayName = "Wood"),
    Bone            UMETA(DisplayName = "Bone"),
    Hide            UMETA(DisplayName = "Hide"),
    Mud             UMETA(DisplayName = "Mud"),
    Thatch          UMETA(DisplayName = "Thatch"),
    Vine            UMETA(DisplayName = "Vine")
};

UENUM(BlueprintType)
enum class EArch_DwellingCondition : uint8
{
    Pristine        UMETA(DisplayName = "Pristine"),
    WellMaintained  UMETA(DisplayName = "Well Maintained"),
    Weathered       UMETA(DisplayName = "Weathered"),
    Damaged         UMETA(DisplayName = "Damaged"),
    Ruined          UMETA(DisplayName = "Ruined"),
    Abandoned       UMETA(DisplayName = "Abandoned")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_DwellingProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    EArch_DwellingType DwellingType = EArch_DwellingType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    EArch_ConstructionMaterial PrimaryMaterial = EArch_ConstructionMaterial::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    EArch_DwellingCondition Condition = EArch_DwellingCondition::WellMaintained;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    int32 Capacity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    bool bHasStorage = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    bool bIsDefensive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    float WeatherProtection = 50.0f;

    FArch_DwellingProperties()
    {
        DwellingType = EArch_DwellingType::None;
        PrimaryMaterial = EArch_ConstructionMaterial::Stone;
        Condition = EArch_DwellingCondition::WellMaintained;
        Capacity = 1;
        StructuralIntegrity = 100.0f;
        bHasFirePit = false;
        bHasStorage = false;
        bIsDefensive = false;
        WeatherProtection = 50.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorLayout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> SleepingAreas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> StorageLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> WorkAreas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector FirePitLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> ToolRacks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> FoodStorage;

    FArch_InteriorLayout()
    {
        FirePitLocation = FVector::ZeroVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_PrehistoricDwelling : public AStaticMeshActor
{
    GENERATED_BODY()

public:
    AArch_PrehistoricDwelling();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* DwellingRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MainStructure;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* InteriorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling Properties")
    FArch_DwellingProperties Properties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FArch_InteriorLayout Layout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    TArray<class AActor*> InteriorObjects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    bool bIsOccupied = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    TArray<class APawn*> CurrentOccupants;

public:
    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void SetDwellingType(EArch_DwellingType NewType);

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void SetCondition(EArch_DwellingCondition NewCondition);

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    bool CanAccommodate(int32 NumberOfOccupants) const;

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void AddOccupant(class APawn* NewOccupant);

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void RemoveOccupant(class APawn* OccupantToRemove);

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    float GetWeatherProtection() const;

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void DamageStructure(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void RepairStructure(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void SetupInteriorLayout();

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void PlaceInteriorObject(class AActor* ObjectToPlace, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    TArray<FVector> GetAvailableStorageLocations() const;

    UFUNCTION(BlueprintCallable, Category = "Interior")
    bool HasFirePit() const { return Properties.bHasFirePit; }

    UFUNCTION(BlueprintCallable, Category = "Interior")
    FVector GetFirePitLocation() const { return Layout.FirePitLocation; }

private:
    void InitializeDwellingMesh();
    void SetupInteriorComponents();
    void ApplyWeatherEffects();
    void UpdateStructuralIntegrity();
};