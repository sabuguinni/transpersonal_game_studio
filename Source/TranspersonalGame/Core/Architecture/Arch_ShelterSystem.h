#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Arch_ShelterSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    StoneShelter    UMETA(DisplayName = "Stone Shelter"),
    RockOverhang    UMETA(DisplayName = "Rock Overhang"),
    BoneHut         UMETA(DisplayName = "Bone Hut"),
    HideTent        UMETA(DisplayName = "Hide Tent")
};

UENUM(BlueprintType)
enum class EArch_ShelterCondition : uint8
{
    Pristine        UMETA(DisplayName = "Pristine"),
    WellMaintained  UMETA(DisplayName = "Well Maintained"),
    Weathered       UMETA(DisplayName = "Weathered"),
    Damaged         UMETA(DisplayName = "Damaged"),
    Ruined          UMETA(DisplayName = "Ruined")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ShelterProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::CaveEntrance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterCondition Condition = EArch_ShelterCondition::Weathered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float ProtectionLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float Capacity = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasToolStorage = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float StructuralIntegrity = 0.8f;

    FArch_ShelterProperties()
    {
        ShelterType = EArch_ShelterType::CaveEntrance;
        Condition = EArch_ShelterCondition::Weathered;
        ProtectionLevel = 0.7f;
        Capacity = 4.0f;
        bHasFirePit = true;
        bHasToolStorage = false;
        StructuralIntegrity = 0.8f;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_ShelterSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_ShelterSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    FArch_ShelterProperties ShelterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    TArray<AActor*> InteriorObjects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    float WeatheringRate = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    float LastMaintenanceTime = 0.0f;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void InitializeShelter(EArch_ShelterType InType, EArch_ShelterCondition InCondition);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool CanProvideProtection() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetProtectionLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void ApplyWeathering(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void RepairShelter(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void AddInteriorObject(AActor* Object);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void RemoveInteriorObject(AActor* Object);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    TArray<AActor*> GetInteriorObjects() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool HasFirePit() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool HasToolStorage() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetStructuralIntegrity() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void SetShelterCondition(EArch_ShelterCondition NewCondition);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    EArch_ShelterCondition GetShelterCondition() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    FString GetShelterDescription() const;

private:
    void UpdateShelterAppearance();
    void CheckStructuralIntegrity();
    float CalculateProtectionFromCondition() const;
};