#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Camera/CameraShakeBase.h"
#include "Audio_ScreenShakeSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ShakeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Range = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    bool bPlayAudio = true;

    FAudio_ShakeData()
    {
        Intensity = 1.0f;
        Duration = 2.0f;
        Range = 5000.0f;
        bPlayAudio = true;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* TRexFootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* RumbleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    FAudio_ShakeData TRexShakeData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    FAudio_ShakeData DefaultShakeData;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerTRexShake(FVector SourceLocation);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerCustomShake(FVector SourceLocation, const FAudio_ShakeData& ShakeData);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void PlayRumbleAudio(FVector Location, float VolumeMultiplier = 1.0f);

private:
    void ApplyScreenShake(FVector SourceLocation, const FAudio_ShakeData& ShakeData);
    float CalculateDistanceAttenuation(FVector SourceLocation, float MaxRange);
};