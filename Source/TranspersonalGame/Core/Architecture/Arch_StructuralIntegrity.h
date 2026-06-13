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
    Ruined          UMETA(DisplayName = "Ruined"),
    Collapsed       UMETA(DisplayName = "Collapsed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_DamagePattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float CrackSeverity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float MossGrowth = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float WeatheringIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    TArray<FVector> CrackLocations;

    FArch_DamagePattern()
    {
        CrackSeverity = 0.0f;
        MossGrowth = 0.0f;
        WeatheringIntensity = 0.0f;
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

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    EArch_StructuralState CurrentState = EArch_StructuralState::Pristine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    float StructuralHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    float MaxStructuralHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    float WeatheringRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    float RainDamageMultiplier = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    float WindDamageMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    FArch_DamagePattern DamagePattern;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    bool bCanCollapse = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    float CollapseThreshold = 10.0f;

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void ApplyWeatherDamage(float DamageAmount, EWeatherType WeatherType);

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void ApplyImpactDamage(float DamageAmount, FVector ImpactLocation);

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void UpdateStructuralState();

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void TriggerCollapse();

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    bool IsStructureStable() const;

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    float GetStructuralHealthPercentage() const;

private:
    void ProcessWeathering(float DeltaTime);
    void UpdateDamagePattern();
    void ApplyVisualDamage();
};