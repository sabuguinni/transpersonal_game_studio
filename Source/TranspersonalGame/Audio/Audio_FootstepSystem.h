#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "Audio_FootstepSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_SurfaceType : uint8
{
    Dirt        UMETA(DisplayName = "Dirt"),
    Grass       UMETA(DisplayName = "Grass"),
    Rock        UMETA(DisplayName = "Rock"),
    Sand        UMETA(DisplayName = "Sand"),
    Water       UMETA(DisplayName = "Water"),
    Wood        UMETA(DisplayName = "Wood"),
    Mud         UMETA(DisplayName = "Mud")
};

UENUM(BlueprintType)
enum class EAudio_MovementType : uint8
{
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Sneak       UMETA(DisplayName = "Sneak"),
    Jump        UMETA(DisplayName = "Jump"),
    Land        UMETA(DisplayName = "Land")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_FootstepData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Audio")
    TObjectPtr<UMetaSoundSource> FootstepMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Audio")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Audio")
    float PitchVariation = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Audio")
    float VolumeVariation = 0.1f;

    FAudio_FootstepData()
    {
        FootstepMetaSound = nullptr;
        BaseVolume = 0.7f;
        PitchVariation = 0.2f;
        VolumeVariation = 0.1f;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_FootstepSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_FootstepSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UAudioComponent> FootstepAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Data")
    TMap<EAudio_SurfaceType, TMap<EAudio_MovementType, FAudio_FootstepData>> FootstepSoundMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float FootstepInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float RunFootstepInterval = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float SneakFootstepInterval = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float SurfaceDetectionDistance = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAudio_SurfaceType CurrentSurface;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAudio_MovementType CurrentMovement;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float TimeSinceLastFootstep;

public:
    UFUNCTION(BlueprintCallable, Category = "Footstep Control")
    void TriggerFootstep(EAudio_MovementType MovementType);

    UFUNCTION(BlueprintCallable, Category = "Footstep Control")
    void SetMovementState(bool bMoving, bool bRunning, bool bSneaking);

    UFUNCTION(BlueprintCallable, Category = "Footstep Control")
    void TriggerJumpSound();

    UFUNCTION(BlueprintCallable, Category = "Footstep Control")
    void TriggerLandSound();

    UFUNCTION(BlueprintPure, Category = "Surface Detection")
    EAudio_SurfaceType GetCurrentSurface() const { return CurrentSurface; }

    UFUNCTION(BlueprintPure, Category = "Movement State")
    bool IsMoving() const { return bIsMoving; }

private:
    void DetectSurfaceType();
    void UpdateFootstepTiming(float DeltaTime);
    void PlayFootstepSound(EAudio_MovementType MovementType);
    float GetFootstepInterval() const;
    void InitializeFootstepData();
};