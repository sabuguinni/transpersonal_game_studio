#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Camera/CameraShakeBase.h"
#include "Audio_ScreenEffectsManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DamageEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effect")
    float FlashDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effect")
    FLinearColor FlashColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effect")
    float FlashIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effect")
    bool bUseScreenShake = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effect")
    float ShakeIntensity = 1.0f;

    FAudio_DamageEffect()
    {
        FlashDuration = 0.3f;
        FlashColor = FLinearColor::Red;
        FlashIntensity = 0.5f;
        bUseScreenShake = true;
        ShakeIntensity = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ProximityEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Effect")
    float MaxDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Effect")
    float MinShakeIntensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Effect")
    float MaxShakeIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Effect")
    float ShakeDuration = 0.5f;

    FAudio_ProximityEffect()
    {
        MaxDistance = 2000.0f;
        MinShakeIntensity = 0.1f;
        MaxShakeIntensity = 2.0f;
        ShakeDuration = 0.5f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ScreenEffectsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ScreenEffectsManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Screen flash effects
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Effects")
    void TriggerDamageFlash(const FAudio_DamageEffect& DamageEffect);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Effects")
    void TriggerCustomFlash(FLinearColor FlashColor, float Duration, float Intensity);

    // Camera shake effects
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Effects")
    void TriggerCameraShake(float Intensity, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Effects")
    void TriggerProximityShake(AActor* SourceActor, const FAudio_ProximityEffect& ProximityEffect);

    // T-Rex proximity detection
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Effects")
    void CheckTRexProximity();

    // Screen overlay management
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Effects")
    void SetScreenOverlay(FLinearColor OverlayColor, float Opacity);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Effects")
    void ClearScreenOverlay();

protected:
    // Damage flash properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects", meta = (AllowPrivateAccess = "true"))
    FAudio_DamageEffect DefaultDamageEffect;

    // Proximity effect properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects", meta = (AllowPrivateAccess = "true"))
    FAudio_ProximityEffect TRexProximityEffect;

    // Current flash state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Screen Effects", meta = (AllowPrivateAccess = "true"))
    bool bIsFlashing;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Screen Effects", meta = (AllowPrivateAccess = "true"))
    float CurrentFlashTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Screen Effects", meta = (AllowPrivateAccess = "true"))
    float FlashDuration;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Screen Effects", meta = (AllowPrivateAccess = "true"))
    FLinearColor FlashColor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Screen Effects", meta = (AllowPrivateAccess = "true"))
    float FlashIntensity;

    // Camera shake reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UCameraShakeBase> DefaultCameraShakeClass;

    // Player controller reference
    UPROPERTY()
    class APlayerController* PlayerController;

private:
    void UpdateFlashEffect(float DeltaTime);
    void ApplyScreenFlash(float Alpha);
    float CalculateProximityIntensity(float Distance, const FAudio_ProximityEffect& Effect);
};