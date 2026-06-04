#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "GameFramework/Actor.h"
#include "Audio_MetaSoundsManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_AmbientType : uint8
{
    Forest          UMETA(DisplayName = "Forest"),
    Campfire        UMETA(DisplayName = "Campfire"),
    Danger          UMETA(DisplayName = "Danger"),
    Tribal          UMETA(DisplayName = "Tribal"),
    Hunt            UMETA(DisplayName = "Hunt")
};

UENUM(BlueprintType)
enum class EAudio_MusicState : uint8
{
    Calm            UMETA(DisplayName = "Calm"),
    Tense           UMETA(DisplayName = "Tense"),
    Combat          UMETA(DisplayName = "Combat"),
    Storytelling    UMETA(DisplayName = "Storytelling"),
    Exploration     UMETA(DisplayName = "Exploration")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SpatialAudioZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    EAudio_AmbientType AmbientType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    bool bIsActive;

    FAudio_SpatialAudioZone()
    {
        Location = FVector::ZeroVector;
        Radius = 1000.0f;
        AmbientType = EAudio_AmbientType::Forest;
        Volume = 1.0f;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_AdaptiveMusicLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    EAudio_MusicState MusicState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    class UMetaSoundSource* MetaSoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    float FadeInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    float FadeOutTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    bool bIsLooping;

    FAudio_AdaptiveMusicLayer()
    {
        MusicState = EAudio_MusicState::Calm;
        MetaSoundAsset = nullptr;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
        Volume = 1.0f;
        bIsLooping = true;
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

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MasterAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* SFXAudioComponent;

    // Spatial Audio System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    TArray<FAudio_SpatialAudioZone> SpatialAudioZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float SpatialAudioUpdateRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float MaxAudioDistance;

    // Adaptive Music System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    TArray<FAudio_AdaptiveMusicLayer> MusicLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    EAudio_MusicState CurrentMusicState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    EAudio_MusicState TargetMusicState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    float MusicTransitionSpeed;

    // Tribal Audio System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Audio")
    class UMetaSoundSource* TribalDrumsMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Audio")
    class UMetaSoundSource* CampfireMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Audio")
    class UMetaSoundSource* StorytellingMetaSound;

    // Audio State
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bIsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentMasterVolume;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FVector PlayerLocation;

public:
    // Core Audio Functions
    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void UpdateSpatialAudio(const FVector& NewPlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetMasterVolume(float NewVolume);

    // Adaptive Music Functions
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void TransitionToMusicState(EAudio_MusicState NewState);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void AddMusicLayer(const FAudio_AdaptiveMusicLayer& NewLayer);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void RemoveMusicLayer(EAudio_MusicState StateToRemove);

    // Spatial Audio Functions
    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void AddSpatialAudioZone(const FAudio_SpatialAudioZone& NewZone);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void RemoveSpatialAudioZone(int32 ZoneIndex);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void UpdateSpatialAudioZone(int32 ZoneIndex, const FAudio_SpatialAudioZone& UpdatedZone);

    // Tribal Audio Functions
    UFUNCTION(BlueprintCallable, Category = "Tribal Audio")
    void PlayTribalDrums(float Volume = 1.0f, bool bLoop = true);

    UFUNCTION(BlueprintCallable, Category = "Tribal Audio")
    void PlayCampfireAmbient(float Volume = 0.7f, bool bLoop = true);

    UFUNCTION(BlueprintCallable, Category = "Tribal Audio")
    void PlayStorytellingAmbient(float Volume = 0.8f, bool bLoop = true);

    UFUNCTION(BlueprintCallable, Category = "Tribal Audio")
    void StopTribalAudio();

    // Danger/Combat Audio Functions
    UFUNCTION(BlueprintCallable, Category = "Combat Audio")
    void TriggerDangerMusic();

    UFUNCTION(BlueprintCallable, Category = "Combat Audio")
    void TriggerCombatMusic();

    UFUNCTION(BlueprintCallable, Category = "Combat Audio")
    void ReturnToCalmMusic();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Audio Utility")
    float CalculateDistanceAttenuation(const FVector& SourceLocation, const FVector& ListenerLocation, float MaxDistance);

    UFUNCTION(BlueprintCallable, Category = "Audio Utility")
    EAudio_AmbientType GetCurrentAmbientType(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio Utility")
    bool IsPlayerInSpatialZone(int32 ZoneIndex, const FVector& PlayerLoc);

private:
    // Internal Functions
    void UpdateMusicTransition(float DeltaTime);
    void ProcessSpatialAudioZones();
    void InitializeMusicLayers();
    void InitializeSpatialZones();
    
    // Timer handles
    FTimerHandle SpatialAudioUpdateTimer;
    FTimerHandle MusicTransitionTimer;
};