#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "Audio_MetaSoundsManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    River       UMETA(DisplayName = "River"), 
    Mountain    UMETA(DisplayName = "Mountain"),
    Plains      UMETA(DisplayName = "Plains"),
    Cave        UMETA(DisplayName = "Cave")
};

UENUM(BlueprintType)
enum class EAudio_IntensityLevel : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Tense       UMETA(DisplayName = "Tense"),
    Danger      UMETA(DisplayName = "Danger"),
    Combat      UMETA(DisplayName = "Combat")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* MusicLayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeTime;

    FAudio_BiomeAudioData()
    {
        BiomeType = EAudio_BiomeType::Forest;
        AmbientSound = nullptr;
        MusicLayer = nullptr;
        BaseVolume = 0.7f;
        FadeTime = 2.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ProximitySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MinDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* ProximitySound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier;

    FAudio_ProximitySettings()
    {
        MinDistance = 500.0f;
        MaxDistance = 2000.0f;
        ProximitySound = nullptr;
        VolumeMultiplier = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_MetaSoundsManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_MetaSoundsManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* MasterAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AmbienceComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* MusicComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* EffectsComponent;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TArray<FAudio_BiomeAudioData> BiomeAudioData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    FAudio_ProximitySettings DinosaurProximitySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AmbienceVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MusicVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float EffectsVolume;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAudio_IntensityLevel CurrentIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsNightTime;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float DinosaurProximityFactor;

public:
    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetCurrentBiome(EAudio_BiomeType NewBiome, bool bFadeTransition = true);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetIntensityLevel(EAudio_IntensityLevel NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetTimeOfDay(bool bIsNight);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void UpdateDinosaurProximity(float ProximityFactor);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void PlayFootstepEffect(FVector Location, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void PlayDamageEffect();

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void PlayDinosaurRoar(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Audio Control")
    void TestAudioSystem();

protected:
    UFUNCTION()
    void UpdateAudioLayers();

    UFUNCTION()
    void FadeBetweenBiomes(EAudio_BiomeType FromBiome, EAudio_BiomeType ToBiome);

    UFUNCTION()
    void ApplyIntensityModifiers();

private:
    float CurrentFadeTime;
    bool bIsFading;
    EAudio_BiomeType PreviousBiome;
    
    TMap<EAudio_BiomeType, class UAudioComponent*> BiomeAudioComponents;
    TArray<class UAudioComponent*> EffectAudioComponents;
};