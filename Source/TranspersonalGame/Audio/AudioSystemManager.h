#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystemManager.generated.h"

// ============================================================
// Audio System Manager — Agent #16
// Adaptive audio: ambient layers, danger proximity, day/night
// ============================================================

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Aware       UMETA(DisplayName = "Aware"),
    Threatened  UMETA(DisplayName = "Threatened"),
    Critical    UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Day         UMETA(DisplayName = "Day"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* Sound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float CurrentVolume = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_TimeOfDay ActiveTimeOfDay = EAudio_TimeOfDay::Day;
};

USTRUCT(BlueprintType)
struct FAudio_DangerZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Radius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_DangerLevel DangerLevel = EAudio_DangerLevel::Aware;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString SourceActorName = TEXT("");
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ProximityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ProximityComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    float ProximityRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    EAudio_DangerLevel DangerLevel = EAudio_DangerLevel::Aware;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    USoundBase* ProximitySound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    float HeartbeatThresholdDistance = 800.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio|Proximity")
    float GetProximityVolume(float DistanceToPlayer) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Proximity")
    bool IsPlayerInDangerZone(float DistanceToPlayer) const;
};

UCLASS(Blueprintable, ClassGroup = (TranspersonalGame))
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ---- Danger State ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|State")
    EAudio_DangerLevel CurrentDangerLevel = EAudio_DangerLevel::Safe;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|State")
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Day;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|State")
    float DangerTransitionSpeed = 2.0f;

    // ---- Ambient Layers ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float AmbientCrossfadeSpeed = 1.5f;

    // ---- Danger Zones ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Danger")
    TArray<FAudio_DangerZone> ActiveDangerZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    float DangerScanInterval = 0.5f;

    // ---- Heartbeat ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Heartbeat")
    USoundBase* HeartbeatSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Heartbeat")
    float HeartbeatMinInterval = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Heartbeat")
    float HeartbeatMaxInterval = 1.2f;

    // ---- Screen Shake ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float TRexShakeRadius = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float TRexShakeIntensity = 1.0f;

    // ---- Audio Components ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components")
    UAudioComponent* AmbientAudioComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components")
    UAudioComponent* DangerAudioComponent = nullptr;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetTimeOfDay(EAudio_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterDangerZone(FVector Location, float Radius, EAudio_DangerLevel Level, FString SourceName);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UnregisterDangerZone(FString SourceName);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetNearestDangerDistance() const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerImpactSound(FVector ImpactLocation, float Magnitude);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    EAudio_DangerLevel EvaluateDangerFromZones() const;

private:
    float DangerScanTimer = 0.0f;
    float HeartbeatTimer = 0.0f;
    float CurrentHeartbeatInterval = 1.2f;
    APawn* CachedPlayerPawn = nullptr;

    void ScanForDanger(float DeltaTime);
    void UpdateAmbientLayers(float DeltaTime);
    void UpdateHeartbeat(float DeltaTime);
    void ApplyTRexScreenShake();
};
