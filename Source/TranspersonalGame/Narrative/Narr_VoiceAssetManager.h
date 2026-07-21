#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Sound/SoundWave.h"
#include "Engine/DataTable.h"
#include "Narr_VoiceAssetManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_VoiceCategory : uint8
{
    TacticalWarning,
    FieldResearch,
    EmergencyAlert,
    WisdomNarration,
    SurvivalInstruction,
    ThreatAssessment
};

UENUM(BlueprintType)
enum class ENarr_VoicePriority : uint8
{
    Low,
    Medium,
    High,
    Critical
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_VoiceAssetData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Asset")
    FString AssetID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Asset")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Asset")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Asset")
    float DurationSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Asset")
    ENarr_VoiceCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Asset")
    ENarr_VoicePriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Asset")
    FString TranscriptText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Asset")
    TArray<FString> TriggerKeywords;

    FNarr_VoiceAssetData()
    {
        AssetID = TEXT("");
        CharacterName = TEXT("");
        AudioURL = TEXT("");
        DurationSeconds = 0.0f;
        Category = ENarr_VoiceCategory::SurvivalInstruction;
        Priority = ENarr_VoicePriority::Medium;
        TranscriptText = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_VoicePlaybackRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playback")
    FString AssetID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playback")
    ENarr_VoicePriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playback")
    bool bInterruptCurrent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playback")
    float VolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playback")
    AActor* SourceActor;

    FNarr_VoicePlaybackRequest()
    {
        AssetID = TEXT("");
        Priority = ENarr_VoicePriority::Medium;
        bInterruptCurrent = false;
        VolumeMultiplier = 1.0f;
        SourceActor = nullptr;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_VoiceAssetManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_VoiceAssetManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Voice Asset Management
    UFUNCTION(BlueprintCallable, Category = "Voice Assets")
    void RegisterVoiceAsset(const FNarr_VoiceAssetData& AssetData);

    UFUNCTION(BlueprintCallable, Category = "Voice Assets")
    bool GetVoiceAsset(const FString& AssetID, FNarr_VoiceAssetData& OutAssetData);

    UFUNCTION(BlueprintCallable, Category = "Voice Assets")
    TArray<FNarr_VoiceAssetData> GetVoiceAssetsByCategory(ENarr_VoiceCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Voice Assets")
    TArray<FNarr_VoiceAssetData> GetVoiceAssetsByCharacter(const FString& CharacterName);

    // Voice Playback
    UFUNCTION(BlueprintCallable, Category = "Voice Playback")
    bool PlayVoiceAsset(const FNarr_VoicePlaybackRequest& Request);

    UFUNCTION(BlueprintCallable, Category = "Voice Playback")
    void StopCurrentVoice();

    UFUNCTION(BlueprintCallable, Category = "Voice Playback")
    void StopAllVoices();

    UFUNCTION(BlueprintCallable, Category = "Voice Playback")
    bool IsVoicePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Voice Playback")
    FString GetCurrentlyPlayingAssetID() const;

    // Voice Queue Management
    UFUNCTION(BlueprintCallable, Category = "Voice Queue")
    void QueueVoiceAsset(const FNarr_VoicePlaybackRequest& Request);

    UFUNCTION(BlueprintCallable, Category = "Voice Queue")
    void ClearVoiceQueue();

    UFUNCTION(BlueprintCallable, Category = "Voice Queue")
    int32 GetQueueLength() const;

    // Contextual Voice Selection
    UFUNCTION(BlueprintCallable, Category = "Contextual Voice")
    FString SelectContextualVoice(const TArray<FString>& Keywords, ENarr_VoiceCategory PreferredCategory);

    UFUNCTION(BlueprintCallable, Category = "Contextual Voice")
    void PlayContextualVoice(const TArray<FString>& Keywords, ENarr_VoiceCategory Category, ENarr_VoicePriority Priority);

protected:
    // Voice asset storage
    UPROPERTY()
    TMap<FString, FNarr_VoiceAssetData> VoiceAssets;

    // Playback queue
    UPROPERTY()
    TArray<FNarr_VoicePlaybackRequest> VoiceQueue;

    // Current playback state
    UPROPERTY()
    FString CurrentlyPlayingAssetID;

    UPROPERTY()
    bool bIsVoicePlaying;

    UPROPERTY()
    float CurrentVoiceStartTime;

    // Audio component for playback
    UPROPERTY()
    class UAudioComponent* VoiceAudioComponent;

    // Internal methods
    void ProcessVoiceQueue();
    void OnVoicePlaybackFinished();
    bool LoadAndPlayVoiceAsset(const FNarr_VoiceAssetData& AssetData, float VolumeMultiplier);
    void InitializeDefaultVoiceAssets();
};