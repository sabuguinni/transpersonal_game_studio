#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Audio_ScreenEffectManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ScreenEffectType : uint8
{
    DamageFlash,
    HealthLow,
    StaminaDepleted,
    FearEffect,
    TRexProximity,
    UnderwaterEffect
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ScreenEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudio_ScreenEffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor EffectColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPulsing = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PulseFrequency = 2.0f;

    FAudio_ScreenEffectData()
    {
        EffectType = EAudio_ScreenEffectType::DamageFlash;
        Intensity = 1.0f;
        Duration = 1.0f;
        EffectColor = FLinearColor::Red;
        bPulsing = false;
        PulseFrequency = 2.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_ScreenEffectManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudio_ScreenEffectManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Screen Effects")
    void TriggerScreenEffect(EAudio_ScreenEffectType EffectType, float Intensity = 1.0f, float Duration = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Screen Effects")
    void TriggerDamageFlash(float Damage, float MaxHealth);

    UFUNCTION(BlueprintCallable, Category = "Screen Effects")
    void TriggerTRexProximityEffect(float Distance, float MaxDistance = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "Screen Effects")
    void TriggerHealthWarning(float CurrentHealth, float MaxHealth);

    UFUNCTION(BlueprintCallable, Category = "Screen Effects")
    void StopScreenEffect(EAudio_ScreenEffectType EffectType);

    UFUNCTION(BlueprintCallable, Category = "Screen Effects")
    void StopAllScreenEffects();

    UFUNCTION(BlueprintCallable, Category = "Screen Effects")
    void SetScreenEffectIntensity(EAudio_ScreenEffectType EffectType, float NewIntensity);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
    TMap<EAudio_ScreenEffectType, FAudio_ScreenEffectData> ActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
    TMap<EAudio_ScreenEffectType, float> EffectTimers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
    UMaterialParameterCollection* ScreenEffectMPC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
    bool bEffectsEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
    float GlobalIntensityMultiplier = 1.0f;

private:
    void UpdateScreenEffect(EAudio_ScreenEffectType EffectType, float DeltaTime);
    void ApplyEffectToMaterial(EAudio_ScreenEffectType EffectType, float CurrentIntensity);
    float CalculateEffectIntensity(const FAudio_ScreenEffectData& EffectData, float Timer) const;
    void InitializeDefaultEffects();
};