#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "../SharedTypes.h"
#include "Arch_CretaceousStructureSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    ShelterBasic     UMETA(DisplayName = "Basic Shelter"),
    ShelterAdvanced  UMETA(DisplayName = "Advanced Shelter"),
    StoragePit       UMETA(DisplayName = "Storage Pit"),
    FirePit          UMETA(DisplayName = "Fire Pit"),
    WaterCatchment   UMETA(DisplayName = "Water Catchment"),
    DefensiveWall    UMETA(DisplayName = "Defensive Wall"),
    WatchTower       UMETA(DisplayName = "Watch Tower"),
    BridgeSimple     UMETA(DisplayName = "Simple Bridge"),
    CaveEntrance     UMETA(DisplayName = "Cave Entrance")
};

UENUM(BlueprintType)
enum class EArch_BuildMaterial : uint8
{
    Wood            UMETA(DisplayName = "Wood"),
    Stone           UMETA(DisplayName = "Stone"),
    Mud             UMETA(DisplayName = "Mud/Clay"),
    Bone            UMETA(DisplayName = "Bone"),
    Hide            UMETA(DisplayName = "Animal Hide"),
    Thatch          UMETA(DisplayName = "Thatch/Grass"),
    Mixed           UMETA(DisplayName = "Mixed Materials")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::ShelterBasic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_BuildMaterial PrimaryMaterial = EArch_BuildMaterial::Wood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatherResistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 MaxOccupants = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bProvidesShelter = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bProvidesStorage = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bProvidesWarmth = false;

    FArch_StructureConfig()
    {
        StructureType = EArch_StructureType::ShelterBasic;
        PrimaryMaterial = EArch_BuildMaterial::Wood;
        StructuralIntegrity = 100.0f;
        WeatherResistance = 50.0f;
        MaxOccupants = 2;
        bProvidesShelter = true;
        bProvidesStorage = false;
        bProvidesWarmth = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_CretaceousStructure : public AActor
{
    GENERATED_BODY()

public:
    AArch_CretaceousStructure();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MainStructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* SupportBeamsMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* RoofMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Config")
    FArch_StructureConfig StructureConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure State")
    float CurrentIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure State")
    float DamageAccumulated = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure State")
    bool bIsOccupied = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure State")
    int32 CurrentOccupants = 0;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void ApplyWeatherDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void RepairStructure(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    bool CanAccommodateOccupants(int32 NumOccupants) const;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void SetOccupancy(int32 NumOccupants);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    float GetShelterEffectiveness() const;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void UpdateStructureMeshes();

    UFUNCTION(BlueprintImplementableEvent, Category = "Structure")
    void OnStructureDestroyed();

    UFUNCTION(BlueprintImplementableEvent, Category = "Structure")
    void OnOccupancyChanged(int32 NewOccupants);

protected:
    void InitializeStructureComponents();
    void ApplyMaterialBasedProperties();
    void CalculateWeatherResistance();
    UStaticMesh* GetMeshForStructureType() const;
    UStaticMesh* GetMeshForMaterial(EArch_BuildMaterial Material) const;
};