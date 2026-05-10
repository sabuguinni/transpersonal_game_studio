#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Audio_ScreenShakeManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ShakeIntensity : uint8
{
    None        UMETA(DisplayName = "None"),
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Heavy       UMETA(DisplayName = "Heavy"),
    Extreme     UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct FAudio_ShakeProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    float Amplitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    bool bFadeOut = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    float FadeOutTime = 0.5f;

    FAudio_ShakeProfile()
    {
        Duration = 1.0f;
        Amplitude = 1.0f;
        Frequency = 10.0f;
        bFadeOut = true;
        FadeOutTime = 0.5f;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ScreenShakeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ScreenShakeManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profiles")
    TMap<EAudio_ShakeIntensity, FAudio_ShakeProfile> ShakeProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Proximity")
    float TRexProximityThreshold = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Proximity")
    float TRexMaxShakeDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Proximity")
    float ShakeUpdateInterval = 0.1f;

    UPROPERTY()
    class APlayerController* CachedPlayerController;

    UPROPERTY()
    class APlayerCameraManager* CachedCameraManager;

    FTimerHandle ProximityCheckTimer;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerScreenShake(EAudio_ShakeIntensity Intensity);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerCustomShake(float Duration, float Amplitude, float Frequency);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerDamageShake();

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerTRexProximityShake(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void StartProximityMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void StopProximityMonitoring();

protected:
    UFUNCTION()
    void CheckTRexProximity();

    void InitializeShakeProfiles();
    void ExecuteShake(const FAudio_ShakeProfile& Profile);
    float CalculateDistanceToNearestTRex();
};