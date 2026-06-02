#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Camera/CameraShakeBase.h"
#include "Audio_ScreenShakeController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ShakeParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Duration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Distance = 1000.0f;

    FAudio_ShakeParams()
    {
        Intensity = 1.0f;
        Duration = 0.5f;
        Frequency = 10.0f;
        Distance = 1000.0f;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Settings")
    FAudio_ShakeParams TRexFootstepShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Settings")
    FAudio_ShakeParams PlayerFootstepShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Settings")
    FAudio_ShakeParams DamageShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Settings")
    float TRexDetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Settings")
    float ShakeUpdateInterval = 0.1f;

private:
    FTimerHandle ShakeTimerHandle;
    class APlayerController* PlayerController;

public:
    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerTRexFootstepShake(const FVector& TRexLocation);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerPlayerFootstepShake();

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerDamageShake(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void StartTRexProximityShaking();

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void StopTRexProximityShaking();

private:
    UFUNCTION()
    void CheckTRexProximity();

    void ApplyScreenShake(const FAudio_ShakeParams& ShakeParams, const FVector& SourceLocation = FVector::ZeroVector);
    float CalculateShakeIntensity(const FVector& SourceLocation, float MaxDistance) const;
};

#include "Audio_ScreenShakeController.generated.h"