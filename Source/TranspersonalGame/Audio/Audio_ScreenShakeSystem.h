#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/CameraShake.h"
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
struct TRANSPERSONALGAME_API FAudio_ShakeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float Amplitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float FalloffRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* ShakeSound = nullptr;

    FAudio_ShakeData()
    {
        Duration = 1.0f;
        Amplitude = 1.0f;
        Frequency = 10.0f;
        FalloffRadius = 2000.0f;
        ShakeSound = nullptr;
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
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AudioComponent;

    // Shake presets for different intensities
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Presets")
    TMap<EAudio_ShakeIntensity, FAudio_ShakeData> ShakePresets;

    // T-Rex proximity shake settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Shakes")
    FAudio_ShakeData TRexFootstepShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Shakes")
    FAudio_ShakeData TRexRoarShake;

    // Combat shake settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Shakes")
    FAudio_ShakeData DamageShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Shakes")
    FAudio_ShakeData DeathShake;

public:
    virtual void Tick(float DeltaTime) override;

    // Main shake trigger function
    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerScreenShake(EAudio_ShakeIntensity Intensity, FVector ShakeLocation = FVector::ZeroVector);

    // Specific shake functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Shakes")
    void TriggerTRexFootstep(FVector FootstepLocation);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Shakes")
    void TriggerTRexRoar(FVector RoarLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat Shakes")
    void TriggerDamageShake();

    UFUNCTION(BlueprintCallable, Category = "Combat Shakes")
    void TriggerDeathShake();

    // Distance-based shake calculation
    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    float CalculateShakeIntensityByDistance(FVector ShakeLocation, float MaxDistance = 2000.0f);

    // Audio-visual synchronization
    UFUNCTION(BlueprintCallable, Category = "Audio Sync")
    void PlayShakeAudio(USoundBase* Sound, FVector Location, float VolumeMultiplier = 1.0f);

private:
    void InitializeShakePresets();
    void ApplyScreenShake(const FAudio_ShakeData& ShakeData, FVector ShakeLocation);
    class APlayerController* GetPlayerController();
};