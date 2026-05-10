#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Arch_DwellingSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_DwellingType : uint8
{
    Cave           UMETA(DisplayName = "Cave Dwelling"),
    WoodenShelter  UMETA(DisplayName = "Wooden Shelter"),
    StoneCircle    UMETA(DisplayName = "Stone Circle"),
    ElevatedHut    UMETA(DisplayName = "Elevated Hut"),
    RockShelter    UMETA(DisplayName = "Rock Shelter")
};

UENUM(BlueprintType)
enum class EArch_DwellingCondition : uint8
{
    Pristine       UMETA(DisplayName = "Pristine"),
    WellMaintained UMETA(DisplayName = "Well Maintained"),
    Weathered      UMETA(DisplayName = "Weathered"),
    Damaged        UMETA(DisplayName = "Damaged"),
    Ruined         UMETA(DisplayName = "Ruined"),
    Abandoned      UMETA(DisplayName = "Abandoned")
};

USTRUCT(BlueprintType)
struct FArch_DwellingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    EArch_DwellingType DwellingType = EArch_DwellingType::Cave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    EArch_DwellingCondition Condition = EArch_DwellingCondition::WellMaintained;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    float WeatherResistance = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    int32 MaxOccupants = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    bool bHasFireplace = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    bool bHasStorageArea = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    bool bHasSleepingArea = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    FVector InteriorDimensions = FVector(400.0f, 300.0f, 250.0f);

    FArch_DwellingData()
    {
        DwellingType = EArch_DwellingType::Cave;
        Condition = EArch_DwellingCondition::WellMaintained;
        StructuralIntegrity = 100.0f;
        WeatherResistance = 75.0f;
        MaxOccupants = 4;
        bHasFireplace = false;
        bHasStorageArea = false;
        bHasSleepingArea = true;
        InteriorDimensions = FVector(400.0f, 300.0f, 250.0f);
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_DwellingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_DwellingSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling System")
    FArch_DwellingData DwellingData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling System")
    TArray<AActor*> InteriorObjects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling System")
    TArray<FVector> SpawnPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling System")
    float DeteriorationRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling System")
    bool bAutoDeteriorate = true;

    UFUNCTION(BlueprintCallable, Category = "Dwelling System")
    void InitializeDwelling(EArch_DwellingType Type, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Dwelling System")
    void SpawnInteriorObjects();

    UFUNCTION(BlueprintCallable, Category = "Dwelling System")
    void UpdateDwellingCondition(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dwelling System")
    bool CanAccommodateOccupants(int32 NumOccupants) const;

    UFUNCTION(BlueprintCallable, Category = "Dwelling System")
    void RepairDwelling(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Dwelling System")
    FVector GetRandomInteriorPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Dwelling System")
    void AddInteriorObject(AActor* Object);

    UFUNCTION(BlueprintCallable, Category = "Dwelling System")
    void RemoveInteriorObject(AActor* Object);

    UFUNCTION(BlueprintPure, Category = "Dwelling System")
    float GetStructuralIntegrityPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Dwelling System")
    bool IsHabitable() const;

private:
    void CreateFireplace(const FVector& Location);
    void CreateStorageArea(const FVector& Location);
    void CreateSleepingArea(const FVector& Location);
    void UpdateVisualCondition();
    FVector CalculateInteriorCenter() const;
};