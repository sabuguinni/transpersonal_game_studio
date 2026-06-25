#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "AudioSystemManager.generated.h"

// ============================================================
// Audio Agent #16 — AudioSystemManager
// Adaptive audio zones, screen shake triggers, campfire/ambient
// ============================================================

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    JungleAmbience   UMETA(DisplayName = "Jungle Ambience"),
    WaterHole        UMETA(DisplayName = "Water Hole"),
    CampFire         UMETA(DisplayName = "Camp Fire"),
    DangerZone       UMETA(DisplayName = "Danger Zone"),
    OpenSavanna      UMETA(DisplayName = "Open Savanna"),
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::JungleAmbience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString FreesoundPreviewURL;
};

USTRUCT(BlueprintType)
struct FAudio_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    float Duration = 5.0f;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ZoneComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ZoneComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float CurrentBlendWeight = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetBlendWeight(float Weight);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    float GetBlendWeight() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_ZoneType GetZoneType() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};

UCLASS(ClassGroup = (TranspersonalGame), Blueprintable)
class TRANSPERSONALGAME_API AAudio_AmbientZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AmbientZoneActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    UAudio_ZoneComponent* AudioZoneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float TriggerRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    TArray<FAudio_DialogueLine> DialogueLines;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void OnPlayerEnterZone(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void OnPlayerExitZone(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    bool IsPlayerInZone() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    bool bPlayerInZone = false;
};

UCLASS(ClassGroup = (TranspersonalGame), Blueprintable)
class TRANSPERSONALGAME_API AAudio_ScreenShakeTrigger : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ScreenShakeTrigger();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float TriggerRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float ShakeIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float ShakeDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float RumbleFrequency = 4.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio|ScreenShake")
    void TriggerShake(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Audio|ScreenShake")
    float GetDistanceToPlayer() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    float ShakeCooldown = 0.0f;
    static constexpr float ShakeCooldownDuration = 2.0f;
};

UCLASS(ClassGroup = (TranspersonalGame), Blueprintable)
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|System")
    TArray<AAudio_AmbientZoneActor*> RegisteredZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|System")
    TArray<AAudio_ScreenShakeTrigger*> ScreenShakeTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|System")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|System")
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|System")
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|System")
    float AmbienceVolume = 0.8f;

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    void RegisterZone(AAudio_AmbientZoneActor* Zone);

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    void UnregisterZone(AAudio_AmbientZoneActor* Zone);

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    AAudio_AmbientZoneActor* GetNearestActiveZone(FVector PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    void UpdateAllZoneBlends(FVector PlayerLocation);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
};
