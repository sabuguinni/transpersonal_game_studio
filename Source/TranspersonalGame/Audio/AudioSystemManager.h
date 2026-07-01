#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    River       UMETA(DisplayName = "River")
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Aware       UMETA(DisplayName = "Aware"),
    Threatened  UMETA(DisplayName = "Threatened"),
    Combat      UMETA(DisplayName = "Combat")
};

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* SoundCue = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_BiomeType Biome = EAudio_BiomeType::Forest;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudioSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioSystemManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Biome ambient audio
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetActiveBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void CrossfadeToLayer(EAudio_BiomeType TargetBiome, float FadeDuration = 2.0f);

    // Danger-reactive music
    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintPure, Category = "Audio|Music")
    EAudio_DangerLevel GetCurrentDangerLevel() const { return CurrentDangerLevel; }

    // T-Rex proximity — screen shake + audio swell
    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerTRexProximityResponse(float DistanceMeters);

    // Campfire audio
    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void PlayCampfireAudio(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void StopCampfireAudio();

    // Day/night audio transition
    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    void OnDayBegin();

    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    void OnNightBegin();

    // Footstep system
    UFUNCTION(BlueprintCallable, Category = "Audio|Footsteps")
    void PlayFootstep(FVector Location, bool bIsPlayerHeavy = false);

    UFUNCTION(BlueprintCallable, Category = "Audio|Footsteps")
    void PlayDinosaurFootstep(FVector Location, float DinosaurMassKg);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    UAudioComponent* CampfireAudioComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Config")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Music")
    USoundCue* MusicSafe;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Music")
    USoundCue* MusicThreat;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Music")
    USoundCue* MusicCombat;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|SFX")
    USoundCue* FootstepLight;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|SFX")
    USoundCue* FootstepHeavy;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|SFX")
    USoundCue* DinosaurFootstepSFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|SFX")
    USoundCue* CampfireSFX;

private:
    EAudio_BiomeType CurrentBiome;
    EAudio_DangerLevel CurrentDangerLevel;

    float DangerTransitionTimer;
    float TRexProximityLastTriggered;

    void UpdateMusicLayer();
    void ApplyScreenShakeForTRex(float Intensity);
};
