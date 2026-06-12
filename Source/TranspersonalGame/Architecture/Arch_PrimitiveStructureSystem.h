#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "Arch_PrimitiveStructureSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    None            UMETA(DisplayName = "None"),
    StoneShelter    UMETA(DisplayName = "Stone Shelter"),
    WoodLean        UMETA(DisplayName = "Wood Lean-to"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    RockFormation   UMETA(DisplayName = "Rock Formation"),
    BoneStructure   UMETA(DisplayName = "Bone Structure")
};

UENUM(BlueprintType)
enum class EArch_MaterialType : uint8
{
    Stone           UMETA(DisplayName = "Stone"),
    Wood            UMETA(DisplayName = "Wood"),
    Bone            UMETA(DisplayName = "Bone"),
    Hide            UMETA(DisplayName = "Hide"),
    Mud             UMETA(DisplayName = "Mud")
};

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_MaterialType PrimaryMaterial = EArch_MaterialType::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatherResistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsHabitable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 MaxOccupants = 1;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::None;
        PrimaryMaterial = EArch_MaterialType::Stone;
        StructuralIntegrity = 100.0f;
        WeatherResistance = 50.0f;
        bIsHabitable = false;
        MaxOccupants = 1;
    }
};

UCLASS(ClassGroup=(Architecture), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_PrimitiveStructureSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_PrimitiveStructureSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Data")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    UStaticMesh* StructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInterface*> StructureMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherDamageRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bAffectedByWeather = true;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void InitializeStructure(EArch_StructureType Type, EArch_MaterialType Material);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void ApplyWeatherDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void RepairStructure(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    bool CanSupportOccupants(int32 NumOccupants) const;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    FVector GetShelterPoint() const;

    UFUNCTION(BlueprintPure, Category = "Structure")
    bool IsStructureStable() const;

private:
    void UpdateStructureMaterial();
    void CalculateWeatherResistance();
};