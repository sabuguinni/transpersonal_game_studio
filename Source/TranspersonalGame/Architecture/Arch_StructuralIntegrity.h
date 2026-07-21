#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Arch_StructuralIntegrity.generated.h"

UENUM(BlueprintType)
enum class EArch_StructuralState : uint8
{
    Pristine        UMETA(DisplayName = "Pristine"),
    Weathered       UMETA(DisplayName = "Weathered"),
    Damaged         UMETA(DisplayName = "Damaged"),
    Crumbling       UMETA(DisplayName = "Crumbling"),
    Collapsed       UMETA(DisplayName = "Collapsed")
};

UENUM(BlueprintType)
enum class EArch_MaterialType : uint8
{
    Stone           UMETA(DisplayName = "Stone"),
    Wood            UMETA(DisplayName = "Wood"),
    Bone            UMETA(DisplayName = "Bone"),
    Clay            UMETA(DisplayName = "Clay"),
    Organic         UMETA(DisplayName = "Organic")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructuralData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
    float IntegrityPercentage = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
    EArch_StructuralState CurrentState = EArch_StructuralState::Pristine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
    EArch_MaterialType MaterialType = EArch_MaterialType::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
    float WeatheringRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
    float LoadCapacity = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
    float CurrentLoad = 0.0f;

    FArch_StructuralData()
    {
        IntegrityPercentage = 100.0f;
        CurrentState = EArch_StructuralState::Pristine;
        MaterialType = EArch_MaterialType::Stone;
        WeatheringRate = 0.1f;
        LoadCapacity = 1000.0f;
        CurrentLoad = 0.0f;
    }
};

UCLASS(ClassGroup=(Architecture), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_StructuralIntegrity : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_StructuralIntegrity();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Structural data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    FArch_StructuralData StructuralData;

    // Time-based weathering
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    bool bEnableWeathering = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float WeatheringMultiplier = 1.0f;

    // Environmental factors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float RainDamageMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float WindDamageMultiplier = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float TemperatureDamageMultiplier = 1.1f;

    // Structural functions
    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void ApplyLoad(float LoadAmount);

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void RemoveLoad(float LoadAmount);

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    bool CanSupportLoad(float TestLoad) const;

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void UpdateStructuralState();

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    float GetIntegrityPercentage() const { return StructuralData.IntegrityPercentage; }

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    EArch_StructuralState GetStructuralState() const { return StructuralData.CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    bool IsStructurallySound() const;

    // Visual feedback
    UFUNCTION(BlueprintImplementableEvent, Category = "Visual")
    void OnStructuralStateChanged(EArch_StructuralState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Visual")
    void OnStructuralCollapse();

private:
    void ProcessWeathering(float DeltaTime);
    void CheckStructuralFailure();
    float CalculateEnvironmentalDamage() const;

    // Internal timers
    float WeatheringTimer = 0.0f;
    float StateUpdateTimer = 0.0f;
    const float StateUpdateInterval = 1.0f;
};