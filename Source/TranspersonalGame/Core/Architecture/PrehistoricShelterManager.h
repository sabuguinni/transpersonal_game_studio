#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "PrehistoricShelterManager.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    StoneCircle     UMETA(DisplayName = "Stone Circle"),
    TreePlatform    UMETA(DisplayName = "Tree Platform"),
    CaveDwelling    UMETA(DisplayName = "Cave Dwelling"),
    Longhouse       UMETA(DisplayName = "Longhouse"),
    RockShelter     UMETA(DisplayName = "Rock Shelter")
};

UENUM(BlueprintType)
enum class EArch_ShelterCondition : uint8
{
    Pristine        UMETA(DisplayName = "Pristine"),
    WellMaintained  UMETA(DisplayName = "Well Maintained"),
    Weathered       UMETA(DisplayName = "Weathered"),
    Damaged         UMETA(DisplayName = "Damaged"),
    Ruins           UMETA(DisplayName = "Ruins")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ShelterProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::StoneCircle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterCondition Condition = EArch_ShelterCondition::WellMaintained;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    int32 Capacity = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float WarmthRating = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float ProtectionRating = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasStorageArea = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bIsOccupied = false;

    FArch_ShelterProperties()
    {
        ShelterType = EArch_ShelterType::StoneCircle;
        Condition = EArch_ShelterCondition::WellMaintained;
        Capacity = 4;
        WarmthRating = 0.7f;
        ProtectionRating = 0.8f;
        bHasFirePit = true;
        bHasStorageArea = true;
        bIsOccupied = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APrehistoricShelterManager : public AActor
{
    GENERATED_BODY()

public:
    APrehistoricShelterManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ShelterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FirePitMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StorageMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    FArch_ShelterProperties ShelterProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    UStaticMesh* StoneCircleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    UStaticMesh* TreePlatformMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    UStaticMesh* CaveDwellingMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    UStaticMesh* LonghouseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    UStaticMesh* FirePitBaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* StoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WoodMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* ThatchMaterial;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Shelter Management")
    void SetShelterType(EArch_ShelterType NewType);

    UFUNCTION(BlueprintCallable, Category = "Shelter Management")
    void SetShelterCondition(EArch_ShelterCondition NewCondition);

    UFUNCTION(BlueprintCallable, Category = "Shelter Management")
    void UpdateShelterMesh();

    UFUNCTION(BlueprintCallable, Category = "Shelter Management")
    void SetOccupancyStatus(bool bOccupied);

    UFUNCTION(BlueprintPure, Category = "Shelter Management")
    float GetWarmthRating() const;

    UFUNCTION(BlueprintPure, Category = "Shelter Management")
    float GetProtectionRating() const;

    UFUNCTION(BlueprintPure, Category = "Shelter Management")
    bool CanAccommodateOccupants(int32 NumOccupants) const;

    UFUNCTION(BlueprintCallable, Category = "Shelter Management")
    void ApplyWeatherDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Shelter Management")
    void RepairShelter(float RepairAmount);

private:
    void InitializeShelterMeshes();
    void ApplyMaterialsBasedOnType();
    void UpdateConditionVisuals();
    
    UPROPERTY()
    float CurrentDurability = 100.0f;
};