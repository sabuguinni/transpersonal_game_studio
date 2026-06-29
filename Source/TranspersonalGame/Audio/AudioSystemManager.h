#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeZone : uint8
{
    Jungle      UMETA(DisplayName = "Jungle"),
    Plains      UMETA(DisplayName = "Plains"),
    River       UMETA(DisplayName = "River"),
    Cave        UMETA(DisplayName = "Cave"),
    Volcano     UMETA(DisplayName = "Volcano"),
    Beach       UMETA(DisplayName = "Beach")
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
    TSoftObjectPtr<USoundBase> Sound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_BiomeZone BiomeZone = EAudio_BiomeZone::Jungle;

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
    TSoftObjectPtr<USoundBase> RoarSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> IdleBreathSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float RoarRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FootstepGroundShakeRadius = 1500.0f;
};

/**
 * UAudio_AudioSystemManager
 * Central audio manager for the prehistoric survival game.
 * Handles adaptive ambient layers, dinosaur audio profiles,
 * danger-state music transitions, and campfire audio.
 */
UCLASS(ClassGroup = "TranspersonalGame", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_AudioSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_AudioSystemManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Ambient Audio ---

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetBiomeZone(EAudio_BiomeZone NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetTimeOfDay(float NormalizedTime); // 0=midnight, 0.5=noon, 1=midnight

    // --- Dinosaur Audio ---

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayDinosaurRoar(FName Species, FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayDinosaurFootstep(FName Species, FVector WorldLocation, float MassKg);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerGroundShake(FVector EpicenterLocation, float Radius, float Intensity);

    // --- Campfire Audio ---

    UFUNCTION(BlueprintCallable, Category = "Audio|Campfire")
    void StartCampfireAudio(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio|Campfire")
    void StopCampfireAudio();

    // --- Survival Feedback ---

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayCraftingSound(FName ItemCrafted);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayDamageImpact(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayHungerWarning();

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayThirstWarning();

    // --- Dialogue / Narration ---

    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void PlayNarrationLine(const FString& AudioURL, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void StopNarration();

    // --- State ---

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    EAudio_BiomeZone CurrentBiome = EAudio_BiomeZone::Jungle;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    EAudio_DangerLevel CurrentDangerLevel = EAudio_DangerLevel::Safe;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    float CurrentTimeOfDay = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_DinosaurSoundProfile> DinosaurProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float DialogueVolume = 1.0f;

private:
    UPROPERTY()
    UAudioComponent* CampfireAudioComponent = nullptr;

    UPROPERTY()
    UAudioComponent* NarrationAudioComponent = nullptr;

    float GroundShakeTimer = 0.0f;
    float GroundShakeIntensity = 0.0f;

    void UpdateAmbientLayers(float DeltaTime);
    void ApplyGroundShakeToCamera(float DeltaTime);
    float GetDangerMusicIntensity() const;
};
