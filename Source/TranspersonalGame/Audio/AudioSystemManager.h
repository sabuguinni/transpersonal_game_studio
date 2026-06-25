#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    JungleDense     UMETA(DisplayName = "Dense Jungle"),
    OpenSavanna     UMETA(DisplayName = "Open Savanna"),
    RiverBank       UMETA(DisplayName = "River Bank"),
    CaveInterior    UMETA(DisplayName = "Cave Interior"),
    VolcanicField   UMETA(DisplayName = "Volcanic Field"),
    NightTime       UMETA(DisplayName = "Night Time")
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_BiomeType BiomeType = EAudio_BiomeType::JungleDense;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = true;
};

USTRUCT(BlueprintType)
struct FAudio_DinosaurSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FName DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> IdleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> AlertSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> AttackSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FootstepGroundShakeRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FootstepGroundShakeIntensity = 0.8f;
};

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ProximityTrigger : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ProximityTrigger();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float TriggerRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> TriggerSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bPlayOnce = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void CheckPlayerProximity(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void ResetTrigger();

protected:
    virtual void BeginPlay() override;

private:
    bool bHasTriggered = false;
    UAudioComponent* ActiveAudioComponent = nullptr;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

    // Ambient layers per biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    // Dinosaur sound profiles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaurs")
    TArray<FAudio_DinosaurSoundProfile> DinosaurProfiles;

    // Current danger level drives music intensity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    EAudio_DangerLevel CurrentDangerLevel = EAudio_DangerLevel::Safe;

    // Master volume multipliers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Volumes", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Volumes", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Volumes", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MusicVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Volumes", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SFXVolume = 1.0f;

    // Nearest T-Rex distance for dynamic audio mixing
    UPROPERTY(BlueprintReadOnly, Category = "Audio|State", meta = (AllowPrivateAccess = "true"))
    float NearestDinosaurDistance = 99999.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State", meta = (AllowPrivateAccess = "true"))
    EAudio_BiomeType CurrentBiome = EAudio_BiomeType::JungleDense;

    // Blueprint-callable functions
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCurrentBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateNearestDinosaurDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurFootstep(FName Species, const FVector& FootLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerScreenShakeFromFootstep(float Intensity, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetDangerMusicIntensity() const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void FadeAmbientToNewBiome(EAudio_BiomeType TargetBiome, float FadeTime = 3.0f);

    UFUNCTION(BlueprintPure, Category = "Audio")
    bool IsPlayerInDanger() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void UpdateDynamicMix(float DeltaTime);
    void UpdateAmbientLayers();

    TArray<UAudioComponent*> ActiveAmbientComponents;
    float DangerMusicBlend = 0.0f;
    float BiomeFadeTimer = 0.0f;
    EAudio_BiomeType PreviousBiome = EAudio_BiomeType::JungleDense;
};
