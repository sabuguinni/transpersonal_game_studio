#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "ProceduralSFXManager.generated.h"

UENUM(BlueprintType)
enum class ESurfaceType : uint8
{
    Grass,
    Dirt,
    Rock,
    Water,
    Wood,
    Sand,
    Mud,
    Leaves
};

UENUM(BlueprintType)
enum class EMovementType : uint8
{
    Walk,
    Run,
    Sneak,
    Jump,
    Land,
    Slide
};

USTRUCT(BlueprintType)
struct FFootstepData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ESurfaceType SurfaceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMovementType MovementType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Pitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShouldAlert; // Should this footstep alert nearby dinosaurs?

    FFootstepData()
    {
        SurfaceType = ESurfaceType::Grass;
        MovementType = EMovementType::Walk;
        Volume = 1.0f;
        Pitch = 1.0f;
        bShouldAlert = true;
    }
};

USTRUCT(BlueprintType)
struct FSurfaceSoundSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class USoundCue*> FootstepSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class USoundCue*> LandingSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class USoundCue*> SlidingSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PitchVariation;

    FSurfaceSoundSet()
    {
        BaseVolume = 1.0f;
        PitchVariation = 0.1f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UProceduralSFXManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UProceduralSFXManager();

protected:
    virtual void BeginPlay() override;

    // Surface Sound Mapping
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Sounds")
    TMap<ESurfaceType, FSurfaceSoundSet> SurfaceSounds;

    // Movement Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkVolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunVolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SneakVolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SneakPitchMultiplier;

    // Audio Components
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class UAudioComponent* FootstepAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class UAudioComponent* MovementAudioComponent;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    ESurfaceType CurrentSurface;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EMovementType CurrentMovementType;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentMovementSpeed;

public:
    // Footstep System
    UFUNCTION(BlueprintCallable, Category = "Footsteps")
    void PlayFootstep(const FFootstepData& FootstepData);

    UFUNCTION(BlueprintCallable, Category = "Footsteps")
    void PlayFootstepAtLocation(const FFootstepData& FootstepData, const FVector& Location);

    // Surface Detection
    UFUNCTION(BlueprintCallable, Category = "Surface")
    ESurfaceType DetectSurfaceType(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Surface")
    void SetCurrentSurface(ESurfaceType NewSurface);

    // Movement Type
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetMovementType(EMovementType NewMovementType);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateMovementSpeed(float Speed);

    // Environmental Sounds
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void PlayEnvironmentalSound(class USoundCue* Sound, const FVector& Location, float Volume = 1.0f);

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "Audio")
    float CalculateFootstepVolume(EMovementType MovementType, ESurfaceType SurfaceType) const;

    UFUNCTION(BlueprintPure, Category = "Audio")
    float CalculateFootstepPitch(EMovementType MovementType, float SpeedMultiplier = 1.0f) const;

private:
    void InitializeSurfaceSounds();
    USoundCue* SelectRandomSound(const TArray<USoundCue*>& SoundArray) const;
    float GetMovementVolumeMultiplier(EMovementType MovementType) const;
};