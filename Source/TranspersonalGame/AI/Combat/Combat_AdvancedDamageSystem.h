#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "../../SharedTypes.h"
#include "Combat_AdvancedDamageSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DamageInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float BaseDamage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    EDamageType DamageType = EDamageType::Physical;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector HitLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector HitDirection = FVector::ForwardVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    AActor* DamageSource = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float ArmorPenetration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    bool bIsCriticalHit = false;

    FCombat_DamageInfo()
    {
        BaseDamage = 10.0f;
        DamageType = EDamageType::Physical;
        HitLocation = FVector::ZeroVector;
        HitDirection = FVector::ForwardVector;
        DamageSource = nullptr;
        ArmorPenetration = 0.0f;
        bIsCriticalHit = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_BodyPartDamage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Part")
    FString BodyPartName = "Torso";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Part")
    float DamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Part")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Part")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Part")
    bool bIsVitalPart = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Part")
    bool bIsDisabled = false;

    FCombat_BodyPartDamage()
    {
        BodyPartName = "Torso";
        DamageMultiplier = 1.0f;
        CurrentHealth = 100.0f;
        MaxHealth = 100.0f;
        bIsVitalPart = false;
        bIsDisabled = false;
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

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === DAMAGE APPLICATION ===
    UFUNCTION(BlueprintCallable, Category = "Combat Damage")
    float ApplyDamage(const FCombat_DamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Combat Damage")
    float ApplyDamageToBodyPart(const FString& BodyPartName, const FCombat_DamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Combat Damage")
    void ApplyStatusEffect(EStatusEffect Effect, float Duration, float Intensity);

    // === DAMAGE CALCULATION ===
    UFUNCTION(BlueprintCallable, Category = "Combat Damage")
    float CalculateFinalDamage(float BaseDamage, EDamageType DamageType, const FString& BodyPartName);

    UFUNCTION(BlueprintCallable, Category = "Combat Damage")
    bool CheckCriticalHit(const FVector& HitLocation, const FVector& HitDirection);

    UFUNCTION(BlueprintCallable, Category = "Combat Damage")
    float GetDamageResistance(EDamageType DamageType);

    // === BODY PART SYSTEM ===
    UFUNCTION(BlueprintCallable, Category = "Combat Damage")
    void InitializeBodyParts();

    UFUNCTION(BlueprintCallable, Category = "Combat Damage")
    FCombat_BodyPartDamage GetBodyPartInfo(const FString& BodyPartName);

    UFUNCTION(BlueprintCallable, Category = "Combat Damage")
    void HealBodyPart(const FString& BodyPartName, float HealAmount);

    // === STATUS EFFECTS ===
    UFUNCTION(BlueprintCallable, Category = "Combat Damage")
    void ProcessStatusEffects(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat Damage")
    bool HasStatusEffect(EStatusEffect Effect);

    UFUNCTION(BlueprintCallable, Category = "Combat Damage")
    void RemoveStatusEffect(EStatusEffect Effect);

    // === DEATH AND REVIVAL ===
    UFUNCTION(BlueprintCallable, Category = "Combat Damage")
    void HandleDeath();

    UFUNCTION(BlueprintCallable, Category = "Combat Damage")
    bool CanRevive();

    UFUNCTION(BlueprintCallable, Category = "Combat Damage")
    void ReviveActor(float HealthPercentage = 0.25f);

protected:
    // === DAMAGE PROPERTIES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Damage", meta = (AllowPrivateAccess = "true"))
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Damage", meta = (AllowPrivateAccess = "true"))
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Damage", meta = (AllowPrivateAccess = "true"))
    TMap<EDamageType, float> DamageResistances;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Damage", meta = (AllowPrivateAccess = "true"))
    TArray<FCombat_BodyPartDamage> BodyParts;

    // === STATUS EFFECTS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Damage", meta = (AllowPrivateAccess = "true"))
    TMap<EStatusEffect, float> ActiveStatusEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Damage", meta = (AllowPrivateAccess = "true"))
    TMap<EStatusEffect, float> StatusEffectDurations;

    // === COMBAT SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Damage", meta = (AllowPrivateAccess = "true"))
    float CriticalHitChance = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Damage", meta = (AllowPrivateAccess = "true"))
    float CriticalHitMultiplier = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Damage", meta = (AllowPrivateAccess = "true"))
    bool bIsDead = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Damage", meta = (AllowPrivateAccess = "true"))
    bool bCanRevive = true;

    // === INTERNAL METHODS ===
    void UpdateHealthStatus();
    void BroadcastDamageEvent(const FCombat_DamageInfo& DamageInfo, float FinalDamage);
    void BroadcastDeathEvent();
    FString DetermineHitBodyPart(const FVector& HitLocation);
};