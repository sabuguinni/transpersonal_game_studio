#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Combat_AdvancedDamageSystem.generated.h"

UENUM(BlueprintType)
enum class ECombat_DamageType : uint8
{
    Physical     UMETA(DisplayName = "Physical"),
    Environmental UMETA(DisplayName = "Environmental"),
    Fall         UMETA(DisplayName = "Fall"),
    Bite         UMETA(DisplayName = "Bite"),
    Claw         UMETA(DisplayName = "Claw"),
    Crush        UMETA(DisplayName = "Crush")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DamageData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float BaseDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float DamageVariance = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    ECombat_DamageType DamageType = ECombat_DamageType::Physical;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    bool bCanCauseBleeding = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float CriticalHitChance = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float CriticalMultiplier = 2.0f;

    FCombat_DamageData()
    {
        BaseDamage = 25.0f;
        DamageVariance = 10.0f;
        DamageType = ECombat_DamageType::Physical;
        bCanCauseBleeding = false;
        CriticalHitChance = 0.1f;
        CriticalMultiplier = 2.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DamageResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    float FinalDamage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    bool bWasCritical = false;

    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    bool bCausedBleeding = false;

    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    ECombat_DamageType DamageType = ECombat_DamageType::Physical;

    FCombat_DamageResult()
    {
        FinalDamage = 0.0f;
        bWasCritical = false;
        bCausedBleeding = false;
        DamageType = ECombat_DamageType::Physical;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_AdvancedDamageSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_AdvancedDamageSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage System")
    TMap<ECombat_DamageType, FCombat_DamageData> DamageTypeData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage System")
    float DamageReductionPercentage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage System")
    bool bIsInvulnerable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage System")
    float BleedingDamagePerSecond = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage System")
    float BleedingDuration = 10.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Damage System")
    bool bIsBleeding = false;

    UPROPERTY(BlueprintReadOnly, Category = "Damage System")
    float BleedingTimeRemaining = 0.0f;

    FTimerHandle BleedingTimerHandle;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Damage System")
    FCombat_DamageResult ApplyDamage(const FCombat_DamageData& DamageData, AActor* DamageSource = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Damage System")
    void SetDamageReduction(float ReductionPercentage);

    UFUNCTION(BlueprintCallable, Category = "Damage System")
    void SetInvulnerability(bool bInvulnerable);

    UFUNCTION(BlueprintCallable, Category = "Damage System")
    void StartBleeding(float Duration = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Damage System")
    void StopBleeding();

    UFUNCTION(BlueprintCallable, Category = "Damage System")
    float CalculateFallDamage(float FallHeight);

    UFUNCTION(BlueprintCallable, Category = "Damage System")
    FCombat_DamageData GetDamageDataForType(ECombat_DamageType DamageType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Damage System")
    void OnDamageApplied(const FCombat_DamageResult& DamageResult, AActor* DamageSource);

    UFUNCTION(BlueprintImplementableEvent, Category = "Damage System")
    void OnCriticalHit(const FCombat_DamageResult& DamageResult);

    UFUNCTION(BlueprintImplementableEvent, Category = "Damage System")
    void OnBleedingStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Damage System")
    void OnBleedingStopped();

private:
    void InitializeDamageTypes();
    void ProcessBleeding();
    float CalculateRandomDamage(const FCombat_DamageData& DamageData);
    bool RollForCritical(float CriticalChance);
};