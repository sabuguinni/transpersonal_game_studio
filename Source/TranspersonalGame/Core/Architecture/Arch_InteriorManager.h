#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerVolume.h"
#include "SharedTypes.h"
#include "Arch_InteriorManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FString InteriorName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float WindProtection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasFirePit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasSleepingArea;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasStorageArea;

    FArch_InteriorData()
    {
        InteriorName = TEXT("Unknown Interior");
        BiomeType = EBiomeType::Forest;
        Temperature = 20.0f;
        Humidity = 50.0f;
        WindProtection = 0.5f;
        bHasFirePit = false;
        bHasSleepingArea = false;
        bHasStorageArea = false;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_InteriorManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_InteriorManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior System")
    TArray<FArch_InteriorData> RegisteredInteriors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior System")
    FArch_InteriorData CurrentInterior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior System")
    bool bPlayerInsideInterior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior System")
    float InteriorCheckRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior System")
    float TemperatureModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior System")
    float HumidityModifier;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void RegisterInterior(const FArch_InteriorData& InteriorData, ATriggerVolume* TriggerVolume);

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void UnregisterInterior(const FString& InteriorName);

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    bool IsPlayerInsideInterior() const;

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    FArch_InteriorData GetCurrentInterior() const;

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void OnPlayerEnterInterior(const FArch_InteriorData& InteriorData);

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void OnPlayerExitInterior();

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    float GetInteriorTemperature() const;

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    float GetInteriorHumidity() const;

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    float GetWindProtectionLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    bool HasFirePit() const;

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    bool HasSleepingArea() const;

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    bool HasStorageArea() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Interior System")
    void CreateTestInterior();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Interior System")
    void ClearAllInteriors();

private:
    void CheckPlayerInteriorStatus();
    FArch_InteriorData* FindInteriorByName(const FString& InteriorName);
};