#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "VFX_ImpactManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    DinosaurFootstep    UMETA(DisplayName = "Dinosaur Footstep"),
    PlayerFootstep      UMETA(DisplayName = "Player Footstep"),
    RockImpact          UMETA(DisplayName = "Rock Impact"),
    WoodImpact          UMETA(DisplayName = "Wood Impact"),
    WaterSplash         UMETA(DisplayName = "Water Splash"),
    BloodSplatter       UMETA(DisplayName = "Blood Splatter"),
    DustCloud           UMETA(DisplayName = "Dust Cloud"),
    Sparks              UMETA(DisplayName = "Sparks")
};

UENUM(BlueprintType)
enum class EVFX_SurfaceType : uint8
{
    Dirt                UMETA(DisplayName = "Dirt"),
    Grass               UMETA(DisplayName = "Grass"),
    Rock                UMETA(DisplayName = "Rock"),
    Mud                 UMETA(DisplayName = "Mud"),
    Sand                UMETA(DisplayName = "Sand"),
    Water               UMETA(DisplayName = "Water"),
    Wood                UMETA(DisplayName = "Wood"),
    Bone                UMETA(DisplayName = "Bone")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    EVFX_ImpactType ImpactType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    EVFX_SurfaceType SurfaceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    FVector ImpactLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    FVector ImpactNormal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    float ImpactForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    float EffectScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    bool bPlaySound;

    FVFX_ImpactData()
    {
        ImpactType = EVFX_ImpactType::DinosaurFootstep;
        SurfaceType = EVFX_SurfaceType::Dirt;
        ImpactLocation = FVector::ZeroVector;
        ImpactNormal = FVector::UpVector;
        ImpactForce = 1.0f;
        EffectScale = 1.0f;
        bPlaySound = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_ImpactManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ImpactManager();

protected:
    virtual void BeginPlay() override;

    // Niagara Components for different effect types
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* DustImpactComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* BloodSplatterComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* WaterSplashComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* SparksComponent;

    // Audio Component for impact sounds
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* ImpactAudioComponent;

    // Niagara Systems for different effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* DustImpactSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* BloodSplatterSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* WaterSplashSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* SparksSystem;

    // Sound Cues for different impact types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundCue* DinosaurFootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundCue* PlayerFootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundCue* RockImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundCue* WoodImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundCue* WaterSplashSound;

public:
    // Main impact effect function
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void PlayImpactEffect(const FVFX_ImpactData& ImpactData);

    // Specific impact effect functions
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void PlayDinosaurFootstep(FVector Location, EVFX_SurfaceType SurfaceType, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void PlayPlayerFootstep(FVector Location, EVFX_SurfaceType SurfaceType, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void PlayBloodSplatter(FVector Location, FVector Normal, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void PlayWaterSplash(FVector Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void PlaySparks(FVector Location, FVector Direction, float Intensity = 1.0f);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SetEffectScale(float NewScale);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void EnableAudio(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void StopAllEffects();

protected:
    // Internal helper functions
    void InitializeNiagaraComponents();
    void InitializeAudioComponent();
    UNiagaraComponent* GetComponentForImpactType(EVFX_ImpactType ImpactType);
    USoundCue* GetSoundForImpactType(EVFX_ImpactType ImpactType, EVFX_SurfaceType SurfaceType);
    void PlayImpactSound(EVFX_ImpactType ImpactType, EVFX_SurfaceType SurfaceType, FVector Location);

private:
    // Effect scaling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings", meta = (AllowPrivateAccess = "true"))
    float GlobalEffectScale;

    // Audio settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings", meta = (AllowPrivateAccess = "true"))
    bool bAudioEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings", meta = (AllowPrivateAccess = "true"))
    float AudioVolumeMultiplier;
};