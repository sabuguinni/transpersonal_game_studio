#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundAttenuation.h"
#include "AudioSystemManager.generated.h"

// ============================================================
// ENUMS — Audio_* prefix (unique across project)
// ============================================================

UENUM(BlueprintType)
enum class EAudio_AmbientZone : uint8
{
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    RiverBank       UMETA(DisplayName = "River Bank"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    NightTime       UMETA(DisplayName = "Night Time"),
    StormApproach   UMETA(DisplayName = "Storm Approaching")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe            UMETA(DisplayName = "Safe — No Threat"),
    Aware           UMETA(DisplayName = "Aware — Distant Threat"),
    Stalked         UMETA(DisplayName = "Stalked — Predator Nearby"),
    Combat          UMETA(DisplayName = "Combat — Active Attack"),
    Fleeing         UMETA(DisplayName = "Fleeing — Running for Life")
};

UENUM(BlueprintType)
enum class EAudio_DinoSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "T-Rex"),
    VelociraptorPack    UMETA(DisplayName = "Raptor Pack"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl         UMETA(DisplayName = "Pterodactyl"),
    Unknown             UMETA(DisplayName = "Unknown Creature")
};

// ============================================================
// STRUCTS — FAudio_* prefix
// ============================================================

USTRUCT(BlueprintType)
struct FAudio_SurvivalFeedback
{
    GENERATED_BODY()

    // Freesound IDs for campfire ambience
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Survival")
    int32 CampfireFreesoundId = 681366;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Survival")
    FString CampfirePreviewURL = TEXT("https://cdn.freesound.org/previews/681/681366_5752443-hq.mp3");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Survival")
    float CampfireRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Survival")
    float CampfireVolume = 0.7f;
};

USTRUCT(BlueprintType)
struct FAudio_NarratorLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    FString TranscriptText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    float EstimatedDurationSeconds = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    EAudio_ThreatLevel TriggerThreatLevel = EAudio_ThreatLevel::Safe;
};

USTRUCT(BlueprintType)
struct FAudio_DinoSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    EAudio_DinoSpecies Species = EAudio_DinoSpecies::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float RoarRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float FootstepRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float FootstepInterval = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float GroundShakeIntensity = 1.0f;

    // Freesound search query for this species
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    FString FreesoundQuery;
};

// ============================================================
// MAIN CLASS
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_SystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_SystemManager();

    // ---- Ambient Zone ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_AmbientZone CurrentAmbientZone = EAudio_AmbientZone::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float AmbientTransitionSpeed = 2.0f;

    // ---- Threat Level ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Threat")
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Threat")
    float ThreatMusicIntensity = 0.0f;

    // ---- Narrator Lines (from ElevenLabs TTS) ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    TArray<FAudio_NarratorLine> NarratorLines;

    // ---- Survival Audio ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Survival")
    FAudio_SurvivalFeedback SurvivalFeedback;

    // ---- Dino Sound Profiles ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    TArray<FAudio_DinoSoundProfile> DinoSoundProfiles;

    // ---- Audio Component ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientAudioComponent;

    // ---- Runtime State ----
    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    float TimeSinceLastNarratorLine = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|State")
    float NarratorCooldownSeconds = 45.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    bool bNarratorPlaying = false;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetAmbientZone(EAudio_AmbientZone NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetThreatLevel(EAudio_ThreatLevel NewThreat);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerNarratorLine(EAudio_ThreatLevel ForThreat);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerDinoFootstepShake(EAudio_DinoSpecies Species, float DistanceMeters);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    FAudio_DinoSoundProfile GetDinoProfile(EAudio_DinoSpecies Species) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Audio|Debug")
    void PopulateDefaultNarratorLines();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Audio|Debug")
    void PopulateDefaultDinoProfiles();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

private:
    float CurrentMusicBlend = 0.0f;
    EAudio_AmbientZone PreviousZone = EAudio_AmbientZone::OpenPlains;

    void UpdateMusicBlend(float DeltaTime);
    void UpdateNarratorCooldown(float DeltaTime);
};
