#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/Engine.h"
#include "Audio_ScreenShakeSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_ShakeIntensity : uint8
{
    Light    UMETA(DisplayName = "Light Shake"),
    Medium   UMETA(DisplayName = "Medium Shake"),
    Heavy    UMETA(DisplayName = "Heavy Shake"),
    Extreme  UMETA(DisplayName = "Extreme Shake")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ShakeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float Amplitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    bool bFadeIn = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    bool bFadeOut = true;
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
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* ShakeAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    FAudio_ShakeSettings LightShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    FAudio_ShakeSettings MediumShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    FAudio_ShakeSettings HeavyShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    FAudio_ShakeSettings ExtremeShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* ImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float TriggerRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoTrigger = true;

public:
    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerScreenShake(EAudio_ShakeIntensity Intensity);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerTRexFootstep();

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerDamageShake();

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void SetShakeIntensityByDistance(float Distance);

protected:
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                              bool bFromSweep, const FHitResult& SweepResult);

private:
    void InitializeShakeSettings();
    FAudio_ShakeSettings GetShakeSettingsByIntensity(EAudio_ShakeIntensity Intensity);
    void PlayShakeAudio(EAudio_ShakeIntensity Intensity);
};