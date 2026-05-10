#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Camera/CameraShakeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Audio_ScreenShakeController.generated.h"

UENUM(BlueprintType)
enum class EAudio_ShakeIntensity : uint8
{
    Light       UMETA(DisplayName = "Light Shake"),
    Medium      UMETA(DisplayName = "Medium Shake"),
    Heavy       UMETA(DisplayName = "Heavy Shake"),
    Extreme     UMETA(DisplayName = "Extreme Shake")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ShakeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float Amplitude = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    bool bFadeOut = true;

    FAudio_ShakeSettings()
    {
        Duration = 1.0f;
        Amplitude = 0.5f;
        Frequency = 10.0f;
        bFadeOut = true;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ScreenShakeController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ScreenShakeController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Screen shake functions
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerTRexProximityShake(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerFootstepShake(EAudio_ShakeIntensity Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerCustomShake(const FAudio_ShakeSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void StopAllShakes();

protected:
    // Shake settings for different scenarios
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Proximity")
    FAudio_ShakeSettings TRexCloseShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Proximity")
    FAudio_ShakeSettings TRexMediumShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Proximity")
    FAudio_ShakeSettings TRexFarShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Shakes")
    FAudio_ShakeSettings FootstepLightShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Shakes")
    FAudio_ShakeSettings FootstepHeavyShake;

    // Distance thresholds for T-Rex proximity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Proximity")
    float CloseDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Proximity")
    float MediumDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Proximity")
    float FarDistance = 3000.0f;

    // Internal state
    UPROPERTY()
    class APlayerController* CachedPlayerController;

    UPROPERTY()
    bool bIsShaking;

    UPROPERTY()
    float CurrentShakeTime;

private:
    void InitializeShakeSettings();
    void ExecuteShake(const FAudio_ShakeSettings& Settings);
    APlayerController* GetPlayerController();
};