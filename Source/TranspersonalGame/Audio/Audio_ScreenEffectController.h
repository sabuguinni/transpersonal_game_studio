#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Camera/CameraShakeBase.h"
#include "Materials/MaterialParameterCollection.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Audio_ScreenEffectController.generated.h"

UENUM(BlueprintType)
enum class EAudio_ScreenEffectType : uint8
{
    None            UMETA(DisplayName = "None"),
    DamageFlash     UMETA(DisplayName = "Damage Flash"),
    ProximityAlert  UMETA(DisplayName = "Proximity Alert"),
    EnvironmentalHit UMETA(DisplayName = "Environmental Hit"),
    TRexProximity   UMETA(DisplayName = "T-Rex Proximity"),
    LowHealth       UMETA(DisplayName = "Low Health"),
    Exhaustion      UMETA(DisplayName = "Exhaustion")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ScreenEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effect")
    EAudio_ScreenEffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effect")
    FLinearColor EffectColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effect")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effect")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effect")
    float FadeInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effect")
    float FadeOutTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effect")
    bool bPulseEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effect")
    float PulseFrequency;

    FAudio_ScreenEffect()
    {
        EffectType = EAudio_ScreenEffectType::None;
        EffectColor = FLinearColor::Red;
        Intensity = 0.5f;
        Duration = 1.0f;
        FadeInTime = 0.1f;
        FadeOutTime = 0.5f;
        bPulseEffect = false;
        PulseFrequency = 2.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_CameraShakeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
    TSubclassOf<UCameraShakeBase> ShakeClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
    float ShakeScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
    float MaxDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
    bool bOrientShakeTowardsEpicenter;

    FAudio_CameraShakeConfig()
    {
        ShakeClass = nullptr;
        ShakeScale = 1.0f;
        MaxDistance = 5000.0f;
        bOrientShakeTowardsEpicenter = true;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ScreenEffectController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ScreenEffectController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Screen Effect Functions
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Effects")
    void TriggerScreenEffect(EAudio_ScreenEffectType EffectType, float IntensityOverride = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Effects")
    void TriggerCustomScreenEffect(const FAudio_ScreenEffect& CustomEffect);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Effects")
    void StopAllScreenEffects();

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Effects")
    void StopScreenEffect(EAudio_ScreenEffectType EffectType);

    // Camera Shake Functions
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Effects")
    void TriggerCameraShake(const FVector& EpicenterLocation, float ShakeIntensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Effects")
    void TriggerTRexFootstepShake(const FVector& FootstepLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Effects")
    void TriggerDamageShake(float DamageAmount);

    // Proximity Effects
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Effects")
    void UpdateProximityEffects(float DistanceToThreat, float ThreatSize);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Effects")
    void TriggerProximityAlert(const FVector& ThreatLocation, float ThreatRadius);

    // Health-based Effects
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Effects")
    void UpdateHealthEffects(float CurrentHealth, float MaxHealth);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Effects")
    void TriggerLowHealthWarning();

protected:
    // Screen Effect Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects", meta = (AllowPrivateAccess = "true"))
    TMap<EAudio_ScreenEffectType, FAudio_ScreenEffect> ScreenEffectConfigs;

    // Camera Shake Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake", meta = (AllowPrivateAccess = "true"))
    FAudio_CameraShakeConfig DefaultShakeConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake", meta = (AllowPrivateAccess = "true"))
    FAudio_CameraShakeConfig TRexShakeConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake", meta = (AllowPrivateAccess = "true"))
    FAudio_CameraShakeConfig DamageShakeConfig;

    // Material Parameter Collection for screen effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects", meta = (AllowPrivateAccess = "true"))
    UMaterialParameterCollection* ScreenEffectMPC;

    // Current Effect State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State", meta = (AllowPrivateAccess = "true"))
    TArray<EAudio_ScreenEffectType> ActiveEffects;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State", meta = (AllowPrivateAccess = "true"))
    float CurrentEffectIntensity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State", meta = (AllowPrivateAccess = "true"))
    FLinearColor CurrentEffectColor;

    // Timers
    FTimerHandle EffectTimerHandle;
    FTimerHandle PulseTimerHandle;

    // Internal Functions
    void InitializeScreenEffects();
    void UpdateScreenEffectMaterial();
    void OnEffectComplete();
    void UpdatePulseEffect();
    void FadeEffect(float DeltaTime);

    // Effect State
    bool bEffectActive;
    float EffectStartTime;
    float EffectCurrentTime;
    FAudio_ScreenEffect CurrentEffect;
    float TargetIntensity;
    float CurrentFadeProgress;
};