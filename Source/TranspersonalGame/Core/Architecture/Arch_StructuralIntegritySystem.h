#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Arch_StructuralIntegritySystem.generated.h"

UENUM(BlueprintType)
enum class EArch_StructuralMaterial : uint8
{
    Stone       UMETA(DisplayName = "Stone"),
    Wood        UMETA(DisplayName = "Wood"),
    Bone        UMETA(DisplayName = "Bone"),
    Clay        UMETA(DisplayName = "Clay"),
    Hide        UMETA(DisplayName = "Hide")
};

UENUM(BlueprintType)
enum class EArch_WeatheringLevel : uint8
{
    Pristine    UMETA(DisplayName = "Pristine"),
    Weathered   UMETA(DisplayName = "Weathered"),
    Damaged     UMETA(DisplayName = "Damaged"),
    Ruined      UMETA(DisplayName = "Ruined"),
    Collapsed   UMETA(DisplayName = "Collapsed")
};

USTRUCT(BlueprintType)
struct FArch_StructuralProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    float MaxIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    float CurrentIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    EArch_StructuralMaterial Material = EArch_StructuralMaterial::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    EArch_WeatheringLevel WeatheringLevel = EArch_WeatheringLevel::Pristine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    float WeatheringRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    bool bCanCollapse = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    float CollapseThreshold = 25.0f;

    FArch_StructuralProperties()
    {
        MaxIntegrity = 100.0f;
        CurrentIntegrity = 100.0f;
        Material = EArch_StructuralMaterial::Stone;
        WeatheringLevel = EArch_WeatheringLevel::Pristine;
        WeatheringRate = 0.1f;
        bCanCollapse = true;
        CollapseThreshold = 25.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_StructuralIntegritySystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_StructuralIntegritySystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Properties")
    FArch_StructuralProperties StructuralProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Effects")
    bool bAffectedByWeather = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Effects")
    bool bAffectedBySeismic = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Effects")
    float MossGrowthRate = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Effects")
    float CurrentMossLevel = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void ApplyDamage(float DamageAmount, bool bIsSeismic = false);

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void RepairStructure(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    bool IsStructureStable() const;

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    float GetIntegrityPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Effects")
    void UpdateWeathering(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Environmental Effects")
    void UpdateMossGrowth(float DeltaTime);

    UFUNCTION(BlueprintImplementableEvent, Category = "Structural Events")
    void OnStructureCollapsed();

    UFUNCTION(BlueprintImplementableEvent, Category = "Structural Events")
    void OnWeatheringLevelChanged(EArch_WeatheringLevel NewLevel);

    UFUNCTION(BlueprintImplementableEvent, Category = "Visual Effects")
    void UpdateVisualState();

private:
    void CheckForCollapse();
    void UpdateWeatheringLevel();
    float GetMaterialWeatheringMultiplier() const;
};