#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Arch_StructuralIntegrity.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_DamageState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Damage")
    float IntegrityPercentage = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Damage")
    float WeatheringRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Damage")
    float LastDamageTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Damage")
    bool bIsCollapsed = false;

    FArch_DamageState()
    {
        IntegrityPercentage = 100.0f;
        WeatheringRate = 0.1f;
        LastDamageTime = 0.0f;
        bIsCollapsed = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_StructuralIntegrity : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_StructuralIntegrity();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural System")
    FArch_DamageState DamageState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural System")
    float MaxIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural System")
    float WeatherResistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural System")
    float ImpactThreshold = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural System")
    TArray<class UStaticMeshComponent*> StructuralElements;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Structural System")
    void ApplyDamage(float DamageAmount, const FVector& ImpactPoint);

    UFUNCTION(BlueprintCallable, Category = "Structural System")
    void ProcessWeathering(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Structural System")
    bool CheckStructuralCollapse();

    UFUNCTION(BlueprintCallable, Category = "Structural System")
    void UpdateVisualDamage();

    UFUNCTION(BlueprintCallable, Category = "Structural System")
    float GetIntegrityPercentage() const { return DamageState.IntegrityPercentage; }

    UFUNCTION(BlueprintCallable, Category = "Structural System")
    bool IsStructureStable() const { return DamageState.IntegrityPercentage > 25.0f && !DamageState.bIsCollapsed; }

    UFUNCTION(BlueprintCallable, Category = "Structural System")
    void RepairStructure(float RepairAmount);
};