#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PostProcessComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialParameterCollection.h"
#include "Camera/CameraShakeBase.h"
#include "Audio_ScreenEffectsManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_EffectIntensity : uint8
{
    None        UMETA(DisplayName = "None"),
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Heavy       UMETA(DisplayName = "Heavy"),
    Extreme     UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DamageFlashSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float FlashIntensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float FlashDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    FLinearColor FlashColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float FadeOutSpeed = 2.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ScreenShakeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float ShakeIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float ShakeDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float ShakeRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float FalloffExponent = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ScreenEffectsManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ScreenEffectsManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Post Process Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPostProcessComponent* PostProcessComponent;

    // Damage Flash Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Damage Flash")
    FAudio_DamageFlashSettings DamageFlashSettings;

    // Screen Shake Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Screen Shake")
    FAudio_ScreenShakeSettings TRexShakeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Screen Shake")
    FAudio_ScreenShakeSettings RaptorShakeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Screen Shake")
    FAudio_ScreenShakeSettings BrachiosaurusShakeSettings;

    // Current Effect State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bDamageFlashActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentFlashIntensity = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float FlashTimer = 0.0f;

    // Camera Shake Class
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Camera Shake")
    TSubclassOf<UCameraShakeBase> CameraShakeClass;

public:
    // Damage Flash Functions
    UFUNCTION(BlueprintCallable, Category = "Effects")
    void TriggerDamageFlash(float Intensity = 1.0f, FLinearColor Color = FLinearColor::Red);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void StopDamageFlash();

    // Screen Shake Functions
    UFUNCTION(BlueprintCallable, Category = "Effects")
    void TriggerTRexFootstepShake(FVector ShakeLocation);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void TriggerRaptorMovementShake(FVector ShakeLocation);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void TriggerBrachiosaurusShake(FVector ShakeLocation);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void TriggerCustomShake(FVector ShakeLocation, EAudio_EffectIntensity Intensity);

    // Distance-based Effect Triggers
    UFUNCTION(BlueprintCallable, Category = "Effects")
    void CheckDinosaurProximity();

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void OnPlayerDamaged(float DamageAmount, AActor* DamageSource);

protected:
    // Internal Functions
    void UpdateDamageFlash(float DeltaTime);
    void ApplyPostProcessEffects();
    float CalculateShakeIntensityByDistance(float Distance, float MaxDistance);
    
    // Player Reference
    UPROPERTY()
    class APawn* PlayerPawn;

    UPROPERTY()
    class APlayerController* PlayerController;
};