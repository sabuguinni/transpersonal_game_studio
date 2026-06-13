#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "SharedTypes.h"
#include "Arch_ShelterSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    CircularStone UMETA(DisplayName = "Circular Stone"),
    RectangularStone UMETA(DisplayName = "Rectangular Stone"),
    NaturalCave UMETA(DisplayName = "Natural Cave"),
    RockOverhang UMETA(DisplayName = "Rock Overhang"),
    TreeHollow UMETA(DisplayName = "Tree Hollow"),
    CliffAlcove UMETA(DisplayName = "Cliff Alcove")
};

UENUM(BlueprintType)
enum class EArch_ShelterCondition : uint8
{
    Pristine UMETA(DisplayName = "Pristine"),
    Good UMETA(DisplayName = "Good"),
    Weathered UMETA(DisplayName = "Weathered"),
    Damaged UMETA(DisplayName = "Damaged"),
    Ruined UMETA(DisplayName = "Ruined"),
    Collapsed UMETA(DisplayName = "Collapsed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ShelterProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::CircularStone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterCondition Condition = EArch_ShelterCondition::Weathered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float WeatherProtection = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float TemperatureInsulation = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    int32 MaxOccupants = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float StructuralIntegrity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasStorageArea = true;

    FArch_ShelterProperties()
    {
        WeatherProtection = 0.75f;
        TemperatureInsulation = 0.6f;
        MaxOccupants = 4;
        StructuralIntegrity = 0.8f;
        bHasFirePit = false;
        bHasStorageArea = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_ShelterSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_ShelterSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    FArch_ShelterProperties ShelterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Components")
    UStaticMeshComponent* MainStructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Components")
    UStaticMeshComponent* RoofMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Components")
    UStaticMeshComponent* FloorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    UBoxComponent* InteriorVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatheringRate = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainDamageMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindDamageMultiplier = 1.2f;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void InitializeShelter(EArch_ShelterType Type, EArch_ShelterCondition InitialCondition);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool CanProvideWeatherProtection() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetTemperatureModifier() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    int32 GetAvailableSpace() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void ApplyWeatherDamage(float DeltaTime, bool bIsRaining, float WindStrength);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void RepairShelter(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    FVector GetOptimalFirePitLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    TArray<FVector> GetStorageLocations() const;

private:
    UPROPERTY()
    int32 CurrentOccupants = 0;

    UPROPERTY()
    float AccumulatedDamage = 0.0f;

    void UpdateShelterCondition();
    void UpdateVisualState();
    FVector CalculateInteriorCenter() const;
};