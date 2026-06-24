#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "AudioZoneSystem.generated.h"

// === AUDIO ZONE SYSTEM — Agent #16 ===
// Hooks into ANarr_DialogueTriggerActor tone system.
// Each zone plays adaptive audio based on ENarr_DialogueTone proximity.

UENUM(BlueprintType)
enum class EAudio_ZoneTone : uint8
{
    Neutral     UMETA(DisplayName = "Neutral"),
    Warning     UMETA(DisplayName = "Warning"),
    Discovery   UMETA(DisplayName = "Discovery"),
    Danger      UMETA(DisplayName = "Danger"),
    Calm        UMETA(DisplayName = "Calm")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneTone Tone = EAudio_ZoneTone::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float BlendRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float MaxVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FString ZoneDescription;
};

USTRUCT(BlueprintType)
struct FAudio_ScreenShakeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float TriggerRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float ShakeIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float ShakeDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float ShakeFrequency = 8.0f;
};

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ZoneComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ZoneComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Zone", meta = (AllowPrivateAccess = "true"))
    bool bPlayerInZone = false;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void OnPlayerEnterZone();

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void OnPlayerExitZone();

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_ZoneTone GetCurrentTone() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    float GetVolumeForDistance(float Distance) const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float CurrentVolume = 0.0f;
    float TargetVolume = 0.0f;
    static constexpr float VolumeBlendSpeed = 2.0f;
};

UCLASS()
class TRANSPERSONALGAME_API AAudio_ZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zone")
    USphereComponent* ZoneSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zone")
    UAudioComponent* AudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zone")
    UAudio_ZoneComponent* AudioZoneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void ActivateZone();

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void DeactivateZone();

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    bool IsPlayerInRange() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    bool bIsActive = false;
};

UCLASS()
class TRANSPERSONALGAME_API AAudio_TRexProximityActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_TRexProximityActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|TRex")
    USphereComponent* ProximitySphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|TRex")
    FAudio_ScreenShakeConfig ShakeConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|TRex")
    float FootstepInterval = 1.8f;

    UFUNCTION(BlueprintCallable, Category = "Audio|TRex")
    void TriggerFootstepShake();

    UFUNCTION(BlueprintCallable, Category = "Audio|TRex")
    void TriggerRoarShake();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    float FootstepTimer = 0.0f;
    bool bPlayerInRange = false;

    UFUNCTION()
    void OnProximityBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnProximityEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
