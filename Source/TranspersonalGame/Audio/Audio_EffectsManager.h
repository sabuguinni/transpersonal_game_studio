#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Camera/CameraShakeBase.h"
#include "SharedTypes.h"
#include "Audio_EffectsManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_FootstepData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    USoundCue* FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    float VolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    float PitchVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    bool bCreateDustEffect;

    FAudio_FootstepData()
    {
        FootstepSound = nullptr;
        VolumeMultiplier = 1.0f;
        PitchVariation = 0.1f;
        bCreateDustEffect = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    USoundCue* ImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    TSubclassOf<UCameraShakeBase> CameraShakeClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    float ShakeIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    float ShakeRadius;

    FAudio_ImpactData()
    {
        ImpactSound = nullptr;
        CameraShakeClass = nullptr;
        ShakeIntensity = 1.0f;
        ShakeRadius = 1000.0f;
    }
};

/**
 * Gestor de efeitos audiovisuais para feedback de gameplay
 * Gere footsteps, screen shake, damage effects e feedback táctil
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_EffectsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_EffectsManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === FOOTSTEP SYSTEM ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps")
    TMap<EGroundType, FAudio_FootstepData> FootstepDataMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps")
    float FootstepVolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps")
    float MinFootstepInterval;

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void PlayFootstep(EGroundType GroundType, FVector Location, float VelocityMultiplier = 1.0f);

    // === SCREEN SHAKE SYSTEM ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    TMap<EDinosaurSpecies, FAudio_ImpactData> DinosaurImpactMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float MaxShakeDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    bool bEnableScreenShake;

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void TriggerDinosaurFootstep(EDinosaurSpecies Species, FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void TriggerScreenShake(TSubclassOf<UCameraShakeBase> ShakeClass, FVector Location, float Intensity = 1.0f);

    // === DAMAGE EFFECTS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effects")
    float DamageFlashDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effects")
    FLinearColor DamageFlashColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effects")
    USoundCue* DamageSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effects")
    TSubclassOf<UCameraShakeBase> DamageCameraShake;

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void TriggerDamageEffect(float DamageAmount, FVector HitLocation);

    // === ENVIRONMENTAL EFFECTS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TMap<EBiomeType, USoundCue*> BiomeAmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float BiomeTransitionTime;

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void TransitionToBiome(EBiomeType NewBiome, float TransitionDuration = 2.0f);

    // === PROXIMITY EFFECTS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    float ProximityCheckRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    float HeartbeatIntensityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    USoundCue* HeartbeatSound;

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void UpdateProximityEffects(float DeltaTime);

private:
    // === INTERNAL STATE ===
    
    UPROPERTY()
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY()
    UAudioComponent* EffectsAudioComponent;

    UPROPERTY()
    UAudioComponent* HeartbeatAudioComponent;

    float LastFootstepTime;
    EBiomeType CurrentBiome;
    float DamageFlashTimer;
    bool bIsDamageFlashActive;

    // === HELPER METHODS ===
    
    void InitializeAudioComponents();
    void UpdateDamageFlash(float DeltaTime);
    float CalculateDistanceAttenuation(FVector SourceLocation, float MaxDistance);
    EGroundType DetectGroundType(FVector Location);
    void CreateDustParticle(FVector Location, EGroundType GroundType);
};