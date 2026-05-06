#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "TranspersonalGame/SharedTypes.h"
#include "Narr_VoicelineDatabase.generated.h"

UENUM(BlueprintType)
enum class ENarr_VoicelineCategory : uint8
{
    FieldNotes = 0,
    EmergencyAlert,
    Discovery,
    WeatherWarning,
    BehavioralObservation,
    SafetyProtocol,
    ResearchUpdate,
    SurvivalTip,
    DangerWarning,
    EnvironmentalNote
};

UENUM(BlueprintType)
enum class ENarr_VoicelinePriority : uint8
{
    Low = 0,
    Normal,
    High,
    Critical,
    Emergency
};

UENUM(BlueprintType)
enum class ENarr_VoicelineContext : uint8
{
    General = 0,
    SwampBiome,
    ForestBiome,
    SavannaBiome,
    DesertBiome,
    MountainBiome,
    DinosaurEncounter,
    WeatherEvent,
    DayTime,
    NightTime,
    LowHealth,
    HighHunger,
    HighFear
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_VoicelineData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    FString VoicelineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    FText VoicelineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    ENarr_VoicelineCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    ENarr_VoicelinePriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    ENarr_VoicelineContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    TSoftObjectPtr<USoundBase> AudioAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    float Cooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    bool bCanRepeat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    TArray<FString> RequiredTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    TArray<FString> BlockingTags;

    FNarr_VoicelineData()
    {
        VoicelineID = TEXT("");
        VoicelineText = FText::GetEmpty();
        Category = ENarr_VoicelineCategory::FieldNotes;
        Priority = ENarr_VoicelinePriority::Normal;
        Context = ENarr_VoicelineContext::General;
        CharacterName = TEXT("Researcher");
        Duration = 10.0f;
        Cooldown = 30.0f;
        bCanRepeat = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_VoicelinePlaybackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playback")
    FString VoicelineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playback")
    float LastPlayedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playback")
    int32 PlayCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playback")
    bool bIsCurrentlyPlaying;

    FNarr_VoicelinePlaybackData()
    {
        VoicelineID = TEXT("");
        LastPlayedTime = 0.0f;
        PlayCount = 0;
        bIsCurrentlyPlaying = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_VoicelineDatabase : public UObject
{
    GENERATED_BODY()

public:
    UNarr_VoicelineDatabase();

    // Database management
    UFUNCTION(BlueprintCallable, Category = "Voiceline Database")
    void InitializeDatabase();

    UFUNCTION(BlueprintCallable, Category = "Voiceline Database")
    void LoadVoicelinesFromDataTable(UDataTable* DataTable);

    // Voiceline retrieval
    UFUNCTION(BlueprintCallable, Category = "Voiceline Database")
    FNarr_VoicelineData GetVoicelineByID(const FString& VoicelineID);

    UFUNCTION(BlueprintCallable, Category = "Voiceline Database")
    TArray<FNarr_VoicelineData> GetVoicelinesByCategory(ENarr_VoicelineCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Voiceline Database")
    TArray<FNarr_VoicelineData> GetVoicelinesByContext(ENarr_VoicelineContext Context);

    UFUNCTION(BlueprintCallable, Category = "Voiceline Database")
    TArray<FNarr_VoicelineData> GetVoicelinesByPriority(ENarr_VoicelinePriority MinPriority);

    UFUNCTION(BlueprintCallable, Category = "Voiceline Database")
    FNarr_VoicelineData GetRandomVoiceline(ENarr_VoicelineCategory Category, ENarr_VoicelineContext Context);

    // Voiceline filtering
    UFUNCTION(BlueprintCallable, Category = "Voiceline Database")
    TArray<FNarr_VoicelineData> FilterVoicelinesByTags(const TArray<FString>& RequiredTags, const TArray<FString>& BlockingTags);

    UFUNCTION(BlueprintCallable, Category = "Voiceline Database")
    bool CanPlayVoiceline(const FString& VoicelineID);

    // Playback tracking
    UFUNCTION(BlueprintCallable, Category = "Voiceline Database")
    void RegisterVoicelinePlayback(const FString& VoicelineID);

    UFUNCTION(BlueprintCallable, Category = "Voiceline Database")
    void UpdateVoicelinePlaybackStatus(const FString& VoicelineID, bool bIsPlaying);

    UFUNCTION(BlueprintCallable, Category = "Voiceline Database")
    float GetTimeSinceLastPlayed(const FString& VoicelineID);

    // Database queries
    UFUNCTION(BlueprintCallable, Category = "Voiceline Database")
    int32 GetTotalVoicelineCount() const;

    UFUNCTION(BlueprintCallable, Category = "Voiceline Database")
    int32 GetVoicelineCountByCategory(ENarr_VoicelineCategory Category) const;

    UFUNCTION(BlueprintCallable, Category = "Voiceline Database")
    TArray<FString> GetAllVoicelineIDs() const;

    // Preset voiceline creation
    UFUNCTION(BlueprintCallable, Category = "Voiceline Database")
    void CreatePresetVoicelines();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    TMap<FString, FNarr_VoicelineData> VoicelineDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    TMap<FString, FNarr_VoicelinePlaybackData> PlaybackHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    UDataTable* VoicelineDataTable;

private:
    void CreateFieldNotesVoicelines();
    void CreateEmergencyAlertVoicelines();
    void CreateDiscoveryVoicelines();
    void CreateWeatherWarningVoicelines();
    void CreateBehavioralObservationVoicelines();
    void CreateSafetyProtocolVoicelines();
    void CreateResearchUpdateVoicelines();
    void CreateSurvivalTipVoicelines();
    void CreateDangerWarningVoicelines();
    void CreateEnvironmentalNoteVoicelines();

    FNarr_VoicelineData CreateVoiceline(
        const FString& ID,
        const FText& Text,
        ENarr_VoicelineCategory Category,
        ENarr_VoicelinePriority Priority,
        ENarr_VoicelineContext Context,
        const FString& Character,
        float Duration,
        float Cooldown = 30.0f,
        bool bCanRepeat = true
    );
};