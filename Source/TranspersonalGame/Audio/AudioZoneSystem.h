#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "AudioZoneSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    RiverBank       UMETA(DisplayName = "River Bank"),
    ForestCore      UMETA(DisplayName = "Forest Core"),
    OpenPlain       UMETA(DisplayName = "Open Plain"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    NightCamp       UMETA(DisplayName = "Night Camp"),
    DangerZone      UMETA(DisplayName = "Danger Zone"),
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Day         UMETA(DisplayName = "Day"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::ForestCore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float BlendRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float MaxVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float DayVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float NightVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bActive = true;
};

USTRUCT(BlueprintType)
struct FAudio_ScreenShakeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float TriggerRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float ShakeIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float ShakeDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float ShakeFrequency = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    FString OwnerDinoLabel;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_TimeOfDay CurrentTimeOfDay;

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void SetTimeOfDay(EAudio_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void SetPlayerDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    float GetBlendedVolume() const;

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void ActivateZone();

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void DeactivateZone();

private:
    float PlayerDistance;
    float CurrentVolume;
    bool bPlayerInZone;

    void UpdateAudioBlend(float DeltaTime);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ScreenShakeTrigger : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ScreenShakeTrigger();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    FAudio_ScreenShakeConfig ShakeConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* DetectionSphere;

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerShake(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    bool IsPlayerInRange() const;

private:
    bool bPlayerCurrentlyInRange;
    float LastShakeTime;
    float ShakeCooldown;

    void CheckPlayerProximity();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_DayNightManager : public UObject
{
    GENERATED_BODY()

public:
    UAudio_DayNightManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night")
    float DayDurationSeconds = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night")
    float NightDurationSeconds = 300.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Day Night")
    float CurrentTimeNormalized = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Day Night")
    EAudio_TimeOfDay CurrentPhase;

    UFUNCTION(BlueprintCallable, Category = "Day Night")
    void AdvanceTime(float DeltaSeconds);

    UFUNCTION(BlueprintCallable, Category = "Day Night")
    EAudio_TimeOfDay GetCurrentPhase() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night")
    float GetSunPitch() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night")
    FLinearColor GetAmbientColor() const;

private:
    float ElapsedSeconds;
    float TotalCycleDuration;
};
