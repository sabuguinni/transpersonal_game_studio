#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/Engine.h"
#include "Audio_ScreenShakeManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ShakeIntensity : uint8
{
    Light UMETA(DisplayName = "Light Shake"),
    Medium UMETA(DisplayName = "Medium Shake"), 
    Heavy UMETA(DisplayName = "Heavy Shake"),
    Extreme UMETA(DisplayName = "Extreme Shake")
};

USTRUCT(BlueprintType)
struct FAudio_ShakeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float Amplitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float FalloffRadius = 1000.0f;

    FAudio_ShakeSettings()
    {
        Duration = 1.0f;
        Amplitude = 1.0f;
        Frequency = 10.0f;
        FalloffRadius = 1000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ScreenShakeManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ScreenShakeManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* ShakeAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    TMap<EAudio_ShakeIntensity, FAudio_ShakeSettings> ShakePresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    class USoundBase* FootstepRumbleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    class USoundBase* ImpactShakeSound;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerScreenShake(EAudio_ShakeIntensity Intensity, FVector ShakeLocation);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerTRexFootstepShake(FVector TRexLocation);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerDamageImpactShake(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void StopAllShakes();

private:
    void InitializeShakePresets();
    float CalculateShakeIntensityByDistance(FVector ShakeLocation, float MaxDistance);
    
    UPROPERTY()
    class APlayerController* CachedPlayerController;
};