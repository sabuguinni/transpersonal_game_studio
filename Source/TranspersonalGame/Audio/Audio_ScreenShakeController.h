#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Audio_ScreenShakeController.generated.h"

UENUM(BlueprintType)
enum class EAudio_ShakeIntensity : uint8
{
    Light UMETA(DisplayName = "Light Shake"),
    Medium UMETA(DisplayName = "Medium Shake"),
    Heavy UMETA(DisplayName = "Heavy Shake"),
    Extreme UMETA(DisplayName = "Extreme Shake")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ShakeProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    float Amplitude;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    float Frequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    float BlendInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    float BlendOutTime;

    FAudio_ShakeProfile()
    {
        Duration = 1.0f;
        Amplitude = 1.0f;
        Frequency = 10.0f;
        BlendInTime = 0.1f;
        BlendOutTime = 0.2f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
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
    void TriggerTRexFootstepShake(float Distance, float TRexMass = 7000.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerDamageShake(float DamageAmount, EAudio_ShakeIntensity IntensityOverride = EAudio_ShakeIntensity::Medium);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerCustomShake(const FAudio_ShakeProfile& ShakeProfile);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void StopAllShakes();

    // Proximity detection for T-Rex
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void CheckTRexProximity();

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void SetTRexProximityEnabled(bool bEnabled);

protected:
    // Shake profiles for different scenarios
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profiles")
    FAudio_ShakeProfile LightShakeProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profiles")
    FAudio_ShakeProfile MediumShakeProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profiles")
    FAudio_ShakeProfile HeavyShakeProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profiles")
    FAudio_ShakeProfile ExtremeShakeProfile;

    // T-Rex proximity settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Proximity")
    float TRexDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Proximity")
    float MaxShakeDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Proximity")
    bool bTRexProximityEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Proximity")
    float ProximityCheckInterval;

    // Internal state
    UPROPERTY()
    APlayerController* PlayerController;

    UPROPERTY()
    APlayerCameraManager* CameraManager;

    float LastProximityCheckTime;

private:
    // Helper functions
    FAudio_ShakeProfile GetShakeProfileByIntensity(EAudio_ShakeIntensity Intensity);
    float CalculateShakeIntensityByDistance(float Distance, float MaxDistance);
    void ApplyShakeProfile(const FAudio_ShakeProfile& Profile, float IntensityMultiplier = 1.0f);
    TArray<AActor*> FindNearbyTRexActors(float SearchRadius);
};