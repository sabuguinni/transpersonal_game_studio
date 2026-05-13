#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Audio_ScreenShakeSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_ShakeIntensity : uint8
{
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"), 
    Heavy       UMETA(DisplayName = "Heavy"),
    Extreme     UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ShakeProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Amplitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float FadeInTime = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float FadeOutTime = 0.5f;

    FAudio_ShakeProfile()
    {
        Duration = 1.0f;
        Amplitude = 1.0f;
        Frequency = 10.0f;
        FadeInTime = 0.1f;
        FadeOutTime = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ScreenShakeSystem : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ScreenShakeSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profiles")
    TMap<EAudio_ShakeIntensity, FAudio_ShakeProfile> ShakeProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Settings")
    float MaxShakeDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Settings")
    float MinShakeDistance = 100.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerFootstepShake(FVector FootstepLocation, float CreatureWeight);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerCustomShake(EAudio_ShakeIntensity Intensity, FVector SourceLocation);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerDamageFlash(float DamageAmount);

private:
    void ApplyScreenShake(const FAudio_ShakeProfile& Profile, float DistanceMultiplier);
    float CalculateDistanceMultiplier(FVector SourceLocation);
    APlayerController* GetLocalPlayerController();
};