#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Camera/CameraShakeBase.h"
#include "Audio_ScreenShakeSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_ShakeIntensity : uint8
{
    Light       UMETA(DisplayName = "Light Shake"),
    Medium      UMETA(DisplayName = "Medium Shake"),
    Heavy       UMETA(DisplayName = "Heavy Shake"),
    Extreme     UMETA(DisplayName = "Extreme Shake")
};

USTRUCT(BlueprintType)
struct FAudio_ShakeParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Amplitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    EAudio_ShakeIntensity Intensity = EAudio_ShakeIntensity::Medium;

    FAudio_ShakeParameters()
    {
        Duration = 1.0f;
        Amplitude = 1.0f;
        Frequency = 10.0f;
        Intensity = EAudio_ShakeIntensity::Medium;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ScreenShakeSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ScreenShakeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Screen shake functions
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerTRexProximityShake(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerFootstepShake(const FVector& FootstepLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerCustomShake(const FAudio_ShakeParameters& ShakeParams);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void StopAllShakes();

protected:
    // Shake parameters for different scenarios
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Shake")
    FAudio_ShakeParameters TRexCloseShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Shake")
    FAudio_ShakeParameters TRexMediumShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Shake")
    FAudio_ShakeParameters TRexFarShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Shake")
    FAudio_ShakeParameters FootstepShake;

    // Distance thresholds for T-Rex proximity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Thresholds")
    float TRexCloseDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Thresholds")
    float TRexMediumDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Thresholds")
    float TRexFarDistance = 3000.0f;

    // Internal shake management
    UPROPERTY()
    TSubclassOf<UCameraShakeBase> CameraShakeClass;

    UPROPERTY()
    class APlayerController* CachedPlayerController;

private:
    void InitializeShakeParameters();
    void ApplyScreenShake(const FAudio_ShakeParameters& ShakeParams);
    float CalculateShakeIntensity(float Distance, float MaxDistance) const;
};