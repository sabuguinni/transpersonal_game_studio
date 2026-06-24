#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "AudioZoneManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Baseline        UMETA(DisplayName = "Baseline Forest"),
    Tension         UMETA(DisplayName = "Tension / Threat Near"),
    Dread           UMETA(DisplayName = "Dread / Discovery"),
    River           UMETA(DisplayName = "River Ambience"),
    Combat          UMETA(DisplayName = "Active Combat"),
    Night           UMETA(DisplayName = "Night Ambience"),
};

UENUM(BlueprintType)
enum class EAudio_MusicLayer : uint8
{
    None            UMETA(DisplayName = "None"),
    Percussion      UMETA(DisplayName = "Primitive Percussion"),
    Strings         UMETA(DisplayName = "Tension Strings"),
    Drone           UMETA(DisplayName = "Dread Drone"),
    Silence         UMETA(DisplayName = "Oppressive Silence"),
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Baseline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_MusicLayer MusicLayer = EAudio_MusicLayer::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float AmbienceVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float MusicVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bTriggerOnce = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FString LinkedNarrativeTrigger = "";
};

USTRUCT(BlueprintType)
struct FAudio_DinosaurSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    FName DinosaurSpecies = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepShakeRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepShakeMagnitude = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float RoarAudibleRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float BreathingRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    bool bTriggersMusicTension = true;
};

/**
 * AAudio_ZoneManager
 * Placed in the level to define an audio zone with ambience + music layer.
 * Overlapping the BoxComponent triggers zone transitions via the AudioSubsystem.
 * Integrates with ANarrDialogueTrigger zones from Agent #15.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Audio Zone Manager"))
class TRANSPERSONALGAME_API AAudio_ZoneManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zone",
        meta = (AllowPrivateAccess = "true"))
    UBoxComponent* TriggerVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zone",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbienceAudioComponent;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void ActivateZone(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void DeactivateZone(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_ZoneType GetZoneType() const { return ZoneConfig.ZoneType; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    float GetCurrentAmbienceVolume() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetMusicLayer(EAudio_MusicLayer NewLayer);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void TriggerTensionSting();

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void TriggerDreadSting();

private:
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    bool bZoneActive = false;
    bool bHasTriggeredOnce = false;
    float CurrentFadeAlpha = 0.0f;
    EAudio_MusicLayer ActiveMusicLayer = EAudio_MusicLayer::None;
};
