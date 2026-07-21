#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Arch_StructuralIntegritySystem.generated.h"

UENUM(BlueprintType)
enum class EArch_StructuralCondition : uint8
{
    Pristine        UMETA(DisplayName = "Pristine"),
    Good           UMETA(DisplayName = "Good"),
    Weathered      UMETA(DisplayName = "Weathered"),
    Damaged        UMETA(DisplayName = "Damaged"),
    Crumbling      UMETA(DisplayName = "Crumbling"),
    Ruined         UMETA(DisplayName = "Ruined")
};

UENUM(BlueprintType)
enum class EArch_MaterialType : uint8
{
    Stone          UMETA(DisplayName = "Stone"),
    Wood           UMETA(DisplayName = "Wood"),
    Bone           UMETA(DisplayName = "Bone"),
    Clay           UMETA(DisplayName = "Clay"),
    Hide           UMETA(DisplayName = "Hide"),
    Thatch         UMETA(DisplayName = "Thatch")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructuralData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    float IntegrityValue = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    EArch_StructuralCondition Condition = EArch_StructuralCondition::Pristine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    EArch_MaterialType MaterialType = EArch_MaterialType::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    float WeatherResistance = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    float AgeInYears = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    bool bIsLoadBearing = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    float MaxLoad = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    float CurrentLoad = 0.0f;

    FArch_StructuralData()
    {
        IntegrityValue = 100.0f;
        Condition = EArch_StructuralCondition::Pristine;
        MaterialType = EArch_MaterialType::Stone;
        WeatherResistance = 0.8f;
        AgeInYears = 0.0f;
        bIsLoadBearing = false;
        MaxLoad = 1000.0f;
        CurrentLoad = 0.0f;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_StructuralIntegritySystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_StructuralIntegritySystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Data")
    FArch_StructuralData StructuralData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Degradation Settings")
    float DegradationRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Degradation Settings")
    float WeatherDamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Degradation Settings")
    bool bEnableTimeDegradation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Degradation Settings")
    bool bEnableWeatherDegradation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Updates")
    bool bAutoUpdateMaterials = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Updates")
    TArray<class UMaterialInterface*> ConditionMaterials;

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void RepairStructure(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    bool CanSupportLoad(float AdditionalLoad) const;

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void AddLoad(float LoadAmount);

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void RemoveLoad(float LoadAmount);

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    EArch_StructuralCondition GetConditionFromIntegrity(float Integrity) const;

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void UpdateVisualCondition();

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    float GetStructuralIntegrity() const { return StructuralData.IntegrityValue; }

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    EArch_StructuralCondition GetCurrentCondition() const { return StructuralData.Condition; }

    UFUNCTION(BlueprintPure, Category = "Structural Integrity")
    bool IsStructureStable() const;

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void SetMaterialType(EArch_MaterialType NewMaterialType);

protected:
    UFUNCTION()
    void ProcessDegradation();

    UFUNCTION()
    void CheckStructuralFailure();

    UFUNCTION()
    void UpdateMaterialBasedOnCondition();

private:
    FTimerHandle DegradationTimerHandle;
    float LastDegradationTime = 0.0f;
    
    float GetMaterialDegradationRate() const;
    float GetWeatherImpact() const;
    void InitializeMaterialArrays();
};