#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/DataTable.h"
#include "Components/AudioComponent.h"
#include "Audio_FootstepSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_SurfaceType : uint8
{
    Grass,
    Dirt,
    Stone,
    Sand,
    Water,
    Wood,
    Metal,
    Snow
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_FootstepData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudio_SurfaceType SurfaceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSoftObjectPtr<USoundBase>> FootstepSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PitchVariation = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinTimeBetweenSteps = 0.3f;

    FAudio_FootstepData()
    {
        SurfaceType = EAudio_SurfaceType::Grass;
        VolumeMultiplier = 1.0f;
        PitchVariation = 0.1f;
        MinTimeBetweenSteps = 0.3f;
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

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Footstep System")
    void PlayFootstepSound(EAudio_SurfaceType SurfaceType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Footstep System")
    void SetFootstepVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Footstep System")
    EAudio_SurfaceType DetectSurfaceType(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Footstep System")
    void EnableAutoFootsteps(bool bEnable);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep System")
    UDataTable* FootstepDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep System")
    float FootstepVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep System")
    float FootstepRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep System")
    bool bAutoDetectFootsteps = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep System")
    float MovementThreshold = 50.0f;

private:
    UPROPERTY()
    UAudioComponent* FootstepAudioComponent;

    FVector LastLocation;
    float LastFootstepTime;
    float AccumulatedDistance;

    void InitializeFootstepAudio();
    FAudio_FootstepData* GetFootstepData(EAudio_SurfaceType SurfaceType);
    bool ShouldPlayFootstep(float DeltaTime, float DistanceMoved);
};