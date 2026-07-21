#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Audio_FootstepParticleSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_FootstepSurface : uint8
{
    Dirt        UMETA(DisplayName = "Dirt"),
    Grass       UMETA(DisplayName = "Grass"),
    Rock        UMETA(DisplayName = "Rock"),
    Sand        UMETA(DisplayName = "Sand"),
    Mud         UMETA(DisplayName = "Mud"),
    Water       UMETA(DisplayName = "Water"),
    Snow        UMETA(DisplayName = "Snow"),
    Wood        UMETA(DisplayName = "Wood")
};

USTRUCT(BlueprintType)
struct FAudio_FootstepEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    UNiagaraSystem* ParticleEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundCue* FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float ParticleScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float SoundVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FLinearColor ParticleColor;

    FAudio_FootstepEffectData()
    {
        ParticleEffect = nullptr;
        FootstepSound = nullptr;
        ParticleScale = 1.0f;
        SoundVolume = 1.0f;
        ParticleColor = FLinearColor::White;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_FootstepParticleSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_FootstepParticleSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Main footstep trigger function
    UFUNCTION(BlueprintCallable, Category = "Audio Footstep")
    void TriggerFootstep(FVector Location, EAudio_FootstepSurface SurfaceType, float Intensity = 1.0f);

    // Detect surface type from location
    UFUNCTION(BlueprintCallable, Category = "Audio Footstep")
    EAudio_FootstepSurface DetectSurfaceType(FVector Location);

    // Auto-trigger footsteps based on movement
    UFUNCTION(BlueprintCallable, Category = "Audio Footstep")
    void EnableAutoFootsteps(bool bEnable);

    // Set footstep parameters
    UFUNCTION(BlueprintCallable, Category = "Audio Footstep")
    void SetFootstepParameters(float StepDistance, float MinSpeed, float MaxSpeed);

protected:
    // Surface effect mappings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TMap<EAudio_FootstepSurface, FAudio_FootstepEffectData> SurfaceEffects;

    // Auto-footstep settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Footsteps")
    bool bAutoFootstepsEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Footsteps")
    float FootstepDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Footsteps")
    float MinMovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Footsteps")
    float MaxMovementSpeed;

    // Footstep timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float FootstepCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float LastFootstepTime;

    // Movement tracking
    UPROPERTY()
    FVector LastPosition;

    UPROPERTY()
    float AccumulatedDistance;

    // Components
    UPROPERTY()
    class APawn* OwnerPawn;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

private:
    // Internal functions
    void InitializeSurfaceEffects();
    void UpdateAutoFootsteps(float DeltaTime);
    void SpawnFootstepEffect(FVector Location, const FAudio_FootstepEffectData& EffectData, float Intensity);
    void PlayFootstepSound(FVector Location, USoundCue* SoundCue, float Volume, float Intensity);
    FVector GetFootLocation();
    bool CanTriggerFootstep();
};

// Dinosaur-specific footstep component
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_DinosaurFootstepSystem : public UAudio_FootstepParticleSystem
{
    GENERATED_BODY()

public:
    UAudio_DinosaurFootstepSystem();

protected:
    virtual void BeginPlay() override;

public:
    // Dinosaur-specific footstep with weight and size
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Footstep")
    void TriggerDinosaurFootstep(FVector Location, float Weight, float Size, EAudio_FootstepSurface SurfaceType);

    // Set dinosaur parameters
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Footstep")
    void SetDinosaurParameters(float DinosaurWeight, float DinosaurSize, float StepForce);

protected:
    // Dinosaur-specific settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float DinosaurWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float DinosaurSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float StepForce;

    // Enhanced effects for large dinosaurs
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    bool bCreateGroundCracks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float GroundShakeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float GroundShakeIntensity;

private:
    void CreateGroundCrackEffect(FVector Location, float Intensity);
    void TriggerGroundShake(FVector Location, float Intensity);
};