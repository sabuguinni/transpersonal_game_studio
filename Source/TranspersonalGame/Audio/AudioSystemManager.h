#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystemManager.generated.h"

// ============================================================
// Audio Agent #16 — AudioSystemManager
// Prehistoric survival ambient audio + dialogue audio wiring
// ============================================================

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Jungle         UMETA(DisplayName = "Jungle Ambience"),
    River          UMETA(DisplayName = "River / Water"),
    Campfire       UMETA(DisplayName = "Campfire"),
    DangerZone     UMETA(DisplayName = "Danger / Predator Zone"),
    OpenPlain      UMETA(DisplayName = "Open Plain"),
    Cave           UMETA(DisplayName = "Cave / Enclosed")
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe           UMETA(DisplayName = "Safe"),
    Cautious       UMETA(DisplayName = "Cautious"),
    Danger         UMETA(DisplayName = "Danger"),
    Critical       UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FAudio_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    float DisplayDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    bool bHasPlayed = false;
};

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Jungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float DangerVolumeMultiplier = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* SoundAsset = nullptr;
};

// ============================================================
// AAudio_ZoneActor — Spatial ambient audio zone
// ============================================================
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Audio Zone Actor"))
class TRANSPERSONALGAME_API AAudio_ZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Zone configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Jungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float ZoneRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_AmbientLayer AmbientLayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_DangerLevel CurrentDangerLevel = EAudio_DangerLevel::Safe;

    // Freesound asset references (set in editor)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    USoundBase* PrimaryAmbientSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    USoundBase* DangerStingerSound = nullptr;

    // Voice lines wired from Agent #15 TTS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    TArray<FAudio_VoiceLine> VoiceLines;

    // Runtime state
    UPROPERTY(BlueprintReadOnly, Category = "Audio|State", meta = (AllowPrivateAccess = "true"))
    bool bPlayerInZone = false;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State", meta = (AllowPrivateAccess = "true"))
    float CurrentVolume = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State", meta = (AllowPrivateAccess = "true"))
    int32 CurrentVoiceLineIndex = 0;

    // Blueprint callable
    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void TriggerNextVoiceLine();

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void FadeInAmbient(float TargetVolume, float FadeTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void FadeOutAmbient(float FadeTime);

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    float GetCurrentVolume() const { return CurrentVolume; }

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    bool IsPlayerInZone() const { return bPlayerInZone; }

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    EAudio_DangerLevel GetDangerLevel() const { return CurrentDangerLevel; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* TriggerSphere = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudioComponent = nullptr;

    UFUNCTION()
    void OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    float VoiceLineTimer = 0.0f;
    float FadeTargetVolume = 0.0f;
    float FadeSpeed = 0.0f;
    bool bFading = false;
};

// ============================================================
// UAudio_SystemManager — GameInstance subsystem for global audio
// ============================================================
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Audio System Manager"))
class TRANSPERSONALGAME_API UAudio_SystemManager : public UObject
{
    GENERATED_BODY()

public:
    UAudio_SystemManager();

    // TTS voice line URLs from Agent #15 and #16
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|TTS")
    TMap<FString, FString> VoiceLineURLs;

    // Freesound asset IDs for runtime reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Freesound")
    TMap<FString, int32> FreesoundAssetIDs;

    // Global danger level (affects all zones)
    UPROPERTY(BlueprintReadOnly, Category = "Audio|Global")
    EAudio_DangerLevel GlobalDangerLevel = EAudio_DangerLevel::Safe;

    // Active zone tracking
    UPROPERTY(BlueprintReadOnly, Category = "Audio|Global")
    TArray<AAudio_ZoneActor*> ActiveZones;

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    void RegisterVoiceLineURL(const FString& LineID, const FString& URL);

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    void SetGlobalDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    void RegisterZone(AAudio_ZoneActor* Zone);

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    void UnregisterZone(AAudio_ZoneActor* Zone);

    UFUNCTION(BlueprintPure, Category = "Audio|System")
    FString GetVoiceLineURL(const FString& LineID) const;

    UFUNCTION(BlueprintPure, Category = "Audio|System")
    EAudio_DangerLevel GetGlobalDangerLevel() const { return GlobalDangerLevel; }

    // Initialize with known TTS URLs from this cycle
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Audio|System")
    void InitializeDefaultVoiceLines();
};
