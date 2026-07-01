#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "AudioZoneManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Camp        UMETA(DisplayName = "Camp Zone"),
    Jungle      UMETA(DisplayName = "Jungle Zone"),
    River       UMETA(DisplayName = "River Zone"),
    DinoArea    UMETA(DisplayName = "Dinosaur Area"),
    Cave        UMETA(DisplayName = "Cave Zone"),
    OpenPlain   UMETA(DisplayName = "Open Plain")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Jungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float TriggerRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bLoopAmbient = true;
};

USTRUCT(BlueprintType)
struct FAudio_NarratorLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    FString LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    EAudio_ThreatLevel ThreatContext = EAudio_ThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    bool bHasPlayed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    float CooldownSeconds = 120.0f;
};

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AAudioZoneManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioZoneManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // --- Zone Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ActiveZoneType = EAudio_ZoneType::Jungle;

    // --- Threat System ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Threat")
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Threat")
    float ThreatDetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Threat")
    float MusicTensionBlend = 0.0f;

    // --- Narrator Lines ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    TArray<FAudio_NarratorLine> NarratorLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    FString TRexWarningURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    FString FireSurvivalURL;

    // --- Freesound References ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    int32 CampfireSoundID = 681366;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    int32 RiverSoundID = 819768;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    FString CampfirePreviewURL = "https://cdn.freesound.org/previews/681/681366_5752443-hq.mp3";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    FString RiverPreviewURL = "https://cdn.freesound.org/previews/819/819768_4036966-hq.mp3";

    // --- Components ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components", meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components", meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientAudioComponent;

    // --- Public API ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetThreatLevel(EAudio_ThreatLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void TriggerNarratorLine(const FString& LineID);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetMusicTension(float TensionValue);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    float GetMusicTension() const { return MusicTensionBlend; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void RegisterDialogueZone(EAudio_ZoneType ZoneType, FVector Location);

    UFUNCTION(CallInEditor, Category = "Audio|Debug")
    void DebugPrintAudioState();

private:
    void UpdateThreatBasedAudio(float DeltaTime);
    void ScanForNearbyDinosaurs();
    void BuildDefaultNarratorLines();

    float LastThreatScanTime = 0.0f;
    float ThreatScanInterval = 2.0f;
    float LastNarratorPlayTime = 0.0f;
};
