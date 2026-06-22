#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "AudioZoneSystem.generated.h"

// ============================================================
// Audio Zone System — Agent #16 Audio Agent
// Prehistoric survival ambient audio zones for MinPlayableMap
// ============================================================

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Campfire        UMETA(DisplayName = "Campfire"),
    Wind            UMETA(DisplayName = "Wind"),
    DinosaurRumble  UMETA(DisplayName = "Dinosaur Rumble"),
    FootstepSurface UMETA(DisplayName = "Footstep Surface"),
    RaptorPack      UMETA(DisplayName = "Raptor Pack"),
    River           UMETA(DisplayName = "River"),
    CaveEcho        UMETA(DisplayName = "Cave Echo")
};

UENUM(BlueprintType)
enum class EAudio_FootstepSurface : uint8
{
    Dirt    UMETA(DisplayName = "Dirt"),
    Rock    UMETA(DisplayName = "Rock"),
    Grass   UMETA(DisplayName = "Grass"),
    Mud     UMETA(DisplayName = "Mud"),
    Water   UMETA(DisplayName = "Water")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Wind;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float BlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float MaxVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bSpatialBlend = true;
};

USTRUCT(BlueprintType)
struct FAudio_FootstepEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    EAudio_FootstepSurface Surface = EAudio_FootstepSurface::Dirt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    float StepVelocity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    bool bIsRunning = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    FVector StepLocation = FVector::ZeroVector;
};

// ============================================================
// UAudio_ZoneComponent — attach to any actor for audio zone
// ============================================================
UCLASS(ClassGroup = "Audio", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ZoneComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ZoneComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float CurrentBlendWeight = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void SetBlendWeight(float Weight);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    float GetBlendWeight() const;

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void FadeIn();

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void FadeOut();

    UFUNCTION(BlueprintPure, Category = "Audio Zone")
    EAudio_ZoneType GetZoneType() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    bool bFadingIn = false;
    bool bFadingOut = false;
};

// ============================================================
// AAudio_AmbientZoneActor — placeable ambient audio zone
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_AmbientZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AmbientZoneActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* BlendSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    UAudio_ZoneComponent* ZoneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FAudio_ZoneConfig ZoneConfig;

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void OnPlayerEnterZone(AActor* OverlappingActor);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void OnPlayerExitZone(AActor* OverlappingActor);

    UFUNCTION(BlueprintPure, Category = "Audio Zone")
    bool IsPlayerInZone() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    bool bPlayerInZone = false;
    float BlendAlpha = 0.0f;
};

// ============================================================
// UAudio_FootstepManager — tracks surface type under player
// ============================================================
UCLASS(ClassGroup = "Audio", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_FootstepManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_FootstepManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    float StepIntervalWalk = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    float StepIntervalRun = 0.30f;

    UPROPERTY(BlueprintReadOnly, Category = "Footstep")
    EAudio_FootstepSurface CurrentSurface = EAudio_FootstepSurface::Dirt;

    UFUNCTION(BlueprintCallable, Category = "Footstep")
    void TriggerFootstep(bool bRunning);

    UFUNCTION(BlueprintCallable, Category = "Footstep")
    EAudio_FootstepSurface DetectSurfaceUnderFoot();

    UFUNCTION(BlueprintCallable, Category = "Footstep")
    void SetCurrentSurface(EAudio_FootstepSurface NewSurface);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float StepTimer = 0.0f;
    bool bIsMoving = false;
};
