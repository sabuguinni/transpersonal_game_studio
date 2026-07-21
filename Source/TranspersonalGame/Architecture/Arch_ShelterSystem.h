#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Arch_ShelterSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    None = 0,
    StoneRuin,
    WoodLeanTo,
    CaveEntrance,
    RockOverhang,
    BuriedShelter
};

UENUM(BlueprintType)
enum class EArch_ShelterCondition : uint8
{
    Pristine = 0,
    Weathered,
    Damaged,
    Ruined,
    Collapsed
};

USTRUCT(BlueprintType)
struct FArch_ShelterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterCondition Condition = EArch_ShelterCondition::Weathered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float MossGrowth = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float StructuralIntegrity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bCanProvideProtection = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float ProtectionRadius = 300.0f;

    FArch_ShelterData()
    {
        ShelterType = EArch_ShelterType::StoneRuin;
        Condition = EArch_ShelterCondition::Weathered;
        WeatheringLevel = 0.5f;
        MossGrowth = 0.3f;
        StructuralIntegrity = 0.7f;
        bCanProvideProtection = true;
        ProtectionRadius = 300.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArch_ShelterSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_ShelterSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter System")
    FArch_ShelterData ShelterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter System")
    TArray<UStaticMeshComponent*> ShelterMeshComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter System")
    UMaterialInterface* BaseStoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter System")
    UMaterialInterface* WeatheredStoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter System")
    UMaterialInterface* MossyMaterial;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shelter System")
    TArray<UMaterialInstanceDynamic*> DynamicMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter System")
    float WeatheringRate = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter System")
    float MossGrowthRate = 0.0005f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter System")
    bool bEnableWeathering = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter System")
    bool bEnableMossGrowth = true;

    // Core shelter management functions
    UFUNCTION(BlueprintCallable, Category = "Shelter System")
    void InitializeShelter(EArch_ShelterType InShelterType, EArch_ShelterCondition InCondition);

    UFUNCTION(BlueprintCallable, Category = "Shelter System")
    void UpdateShelterCondition(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Shelter System")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Shelter System")
    void ApplyMossGrowth(float MossAmount);

    UFUNCTION(BlueprintCallable, Category = "Shelter System")
    void UpdateMaterialParameters();

    UFUNCTION(BlueprintCallable, Category = "Shelter System")
    bool CanProvideProtection() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter System")
    float GetProtectionValue() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter System")
    void SetShelterType(EArch_ShelterType NewType);

    UFUNCTION(BlueprintCallable, Category = "Shelter System")
    void SetCondition(EArch_ShelterCondition NewCondition);

    UFUNCTION(BlueprintCallable, Category = "Shelter System")
    void RepairShelter(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Shelter System")
    void DamageShelter(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Shelter System")
    TArray<FVector> GetShelterBounds() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter System")
    bool IsPlayerInShelter(const FVector& PlayerLocation) const;

private:
    void CreateDynamicMaterials();
    void UpdateWeatheringEffects();
    void UpdateMossEffects();
    void ValidateShelterIntegrity();
    FLinearColor GetWeatheringColor() const;
    float GetMossOpacity() const;
};