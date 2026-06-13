#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "Arch_ShelterSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    None = 0,
    BasicLeanTo = 1,
    StoneFoundation = 2,
    CaveEntrance = 3,
    TreeHollow = 4,
    RockOverhang = 5,
    BuriedShelter = 6
};

UENUM(BlueprintType)
enum class EArch_ShelterCondition : uint8
{
    Pristine = 0,
    Good = 1,
    Weathered = 2,
    Damaged = 3,
    Ruined = 4,
    Collapsed = 5
};

USTRUCT(BlueprintType)
struct FArch_ShelterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterCondition Condition = EArch_ShelterCondition::Good;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float WeatheringLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float ProtectionValue = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    int32 MaxOccupants = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasStorage = false;

    FArch_ShelterData()
    {
        ShelterType = EArch_ShelterType::BasicLeanTo;
        Condition = EArch_ShelterCondition::Good;
        WeatheringLevel = 0.0f;
        StructuralIntegrity = 100.0f;
        ProtectionValue = 50.0f;
        MaxOccupants = 1;
        bHasFirePit = false;
        bHasStorage = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_ShelterSystem : public AActor
{
    GENERATED_BODY()

public:
    AArch_ShelterSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ShelterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Data")
    FArch_ShelterData ShelterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInterface*> WeatheringMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    TArray<UStaticMesh*> ShelterMeshVariants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatheringRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainDamageMultiplier = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindDamageMultiplier = 1.5f;

public:
    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void InitializeShelter(EArch_ShelterType Type, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void ApplyWeathering(float WeatherIntensity, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void RepairShelter(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool CanProvideProtection() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetProtectionValue() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void UpdateVisualCondition();

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void AddOccupant(AActor* Occupant);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void RemoveOccupant(AActor* Occupant);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool HasSpace() const;

    UFUNCTION(BlueprintPure, Category = "Shelter")
    EArch_ShelterType GetShelterType() const { return ShelterData.ShelterType; }

    UFUNCTION(BlueprintPure, Category = "Shelter")
    EArch_ShelterCondition GetCondition() const { return ShelterData.Condition; }

    UFUNCTION(BlueprintPure, Category = "Shelter")
    float GetStructuralIntegrity() const { return ShelterData.StructuralIntegrity; }

private:
    void UpdateShelterMesh();
    void UpdateMaterials();
    void CheckStructuralFailure();

    UPROPERTY()
    TArray<AActor*> CurrentOccupants;

    float LastWeatheringUpdate = 0.0f;
    float WeatheringUpdateInterval = 5.0f;
};