#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialParameterCollection.h"
#include "Arch_StructuralWeathering.generated.h"

UENUM(BlueprintType)
enum class EArch_WeatheringLevel : uint8
{
    Pristine        UMETA(DisplayName = "Pristine"),
    LightWeathering UMETA(DisplayName = "Light Weathering"),
    Moderate        UMETA(DisplayName = "Moderate Decay"),
    Heavy           UMETA(DisplayName = "Heavy Deterioration"),
    Collapsed       UMETA(DisplayName = "Collapsed")
};

UENUM(BlueprintType)
enum class EArch_MaterialType : uint8
{
    Stone           UMETA(DisplayName = "Stone/Basalt"),
    Wood            UMETA(DisplayName = "Prehistoric Wood"),
    Bone            UMETA(DisplayName = "Bone/Ivory"),
    Clay            UMETA(DisplayName = "Hardened Clay"),
    Hide            UMETA(DisplayName = "Animal Hide")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_WeatheringData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    EArch_WeatheringLevel CurrentLevel = EArch_WeatheringLevel::Pristine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    EArch_MaterialType MaterialType = EArch_MaterialType::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float WeatheringRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float MossGrowthFactor = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    bool bIsExposedToElements = true;

    FArch_WeatheringData()
    {
        CurrentLevel = EArch_WeatheringLevel::Pristine;
        MaterialType = EArch_MaterialType::Stone;
        WeatheringRate = 0.1f;
        MossGrowthFactor = 0.0f;
        StructuralIntegrity = 100.0f;
        bIsExposedToElements = true;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_StructuralWeathering : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_StructuralWeathering();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Weathering")
    FArch_WeatheringData WeatheringData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Weathering")
    float TimeAccelerationFactor = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Weathering")
    bool bEnableRealTimeWeathering = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Weathering")
    UMaterialParameterCollection* WeatheringMPC;

    UFUNCTION(BlueprintCallable, Category = "Structural Weathering")
    void ApplyWeatheringLevel(EArch_WeatheringLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Structural Weathering")
    void AccelerateWeathering(float AccelerationMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Structural Weathering")
    float GetStructuralIntegrity() const { return WeatheringData.StructuralIntegrity; }

    UFUNCTION(BlueprintCallable, Category = "Structural Weathering")
    bool IsStructureStable() const { return WeatheringData.StructuralIntegrity > 25.0f; }

    UFUNCTION(BlueprintCallable, Category = "Structural Weathering")
    void ApplySeismicDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Structural Weathering")
    void GrowMoss(float GrowthRate);

private:
    void UpdateWeatheringEffects();
    void UpdateMaterialParameters();
    float CalculateWeatheringRate() const;
    void CheckStructuralCollapse();

    UPROPERTY()
    float AccumulatedWeatheringTime = 0.0f;

    UPROPERTY()
    TArray<UMaterialInstanceDynamic*> DynamicMaterials;
};