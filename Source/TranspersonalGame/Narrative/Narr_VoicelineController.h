#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "Narr_VoicelineController.generated.h"

UENUM(BlueprintType)
enum class ENarr_VoicelineType : uint8
{
    FieldObservation    UMETA(DisplayName = "Field Observation"),
    SecurityAlert       UMETA(DisplayName = "Security Alert"),
    EnvironmentalLog    UMETA(DisplayName = "Environmental Log"),
    SurvivalTip         UMETA(DisplayName = "Survival Tip"),
    QuestComplete       UMETA(DisplayName = "Quest Complete"),
    DangerWarning       UMETA(DisplayName = "Danger Warning"),
    Discovery           UMETA(DisplayName = "Discovery"),
    Tutorial            UMETA(DisplayName = "Tutorial")
};

UENUM(BlueprintType)
enum class ENarr_VoicelinePriority : uint8
{
    Low         UMETA(DisplayName = "Low Priority"),
    Normal      UMETA(DisplayName = "Normal Priority"),
    High        UMETA(DisplayName = "High Priority"),
    Critical    UMETA(DisplayName = "Critical Priority"),
    Emergency   UMETA(DisplayName = "Emergency Priority")
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
    ENarr_VoicelineType VoicelineType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    ENarr_VoicelinePriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    TSoftObjectPtr<USoundBase> AudioAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    TArray<FString> TriggerConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    bool bCanInterrupt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    float CooldownTime;

    FNarr_VoicelineData()
    {
        VoicelineID = TEXT("");
        VoicelineText = FText::GetEmpty();
        VoicelineType = ENarr_VoicelineType::FieldObservation;
        Priority = ENarr_VoicelinePriority::Normal;
        CharacterName = TEXT("Unknown");
        Duration = 0.0f;
        bCanInterrupt = true;
        CooldownTime = 30.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_ActiveVoiceline
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Active Voiceline")
    FNarr_VoicelineData VoicelineData;

    UPROPERTY(BlueprintReadOnly, Category = "Active Voiceline")
    float StartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Active Voiceline")
    bool bIsPlaying;

    UPROPERTY(BlueprintReadOnly, Category = "Active Voiceline")
    class UAudioComponent* AudioComponent;

    FNarr_ActiveVoiceline()
    {
        StartTime = 0.0f;
        bIsPlaying = false;
        AudioComponent = nullptr;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoicelineStarted, const FNarr_VoicelineData&, VoicelineData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoicelineCompleted, const FNarr_VoicelineData&, VoicelineData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoicelineInterrupted, const FNarr_VoicelineData&, VoicelineData);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_VoicelineController : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_VoicelineController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core voiceline playback functions
    UFUNCTION(BlueprintCallable, Category = "Narrative Voiceline")
    bool PlayVoiceline(const FString& VoicelineID);

    UFUNCTION(BlueprintCallable, Category = "Narrative Voiceline")
    bool PlayVoicelineByType(ENarr_VoicelineType VoicelineType, ENarr_VoicelinePriority MinPriority = ENarr_VoicelinePriority::Normal);

    UFUNCTION(BlueprintCallable, Category = "Narrative Voiceline")
    void StopCurrentVoiceline();

    UFUNCTION(BlueprintCallable, Category = "Narrative Voiceline")
    void StopAllVoicelines();

    // Voiceline management
    UFUNCTION(BlueprintCallable, Category = "Narrative Voiceline")
    void RegisterVoiceline(const FNarr_VoicelineData& VoicelineData);

    UFUNCTION(BlueprintCallable, Category = "Narrative Voiceline")
    bool IsVoicelineAvailable(const FString& VoicelineID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative Voiceline")
    bool IsAnyVoicelinePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative Voiceline")
    FNarr_VoicelineData GetCurrentVoiceline() const;

    // Priority and interruption system
    UFUNCTION(BlueprintCallable, Category = "Narrative Voiceline")
    bool CanInterruptCurrentVoiceline(ENarr_VoicelinePriority NewPriority) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative Voiceline")
    void SetVoicelineVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Narrative Voiceline")
    void SetVoicelinePaused(bool bPaused);

    // Data table integration
    UFUNCTION(BlueprintCallable, Category = "Narrative Voiceline")
    void LoadVoicelinesFromDataTable(UDataTable* VoicelineDataTable);

    // Event delegates
    UPROPERTY(BlueprintAssignable, Category = "Narrative Events")
    FOnVoicelineStarted OnVoicelineStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative Events")
    FOnVoicelineCompleted OnVoicelineCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative Events")
    FOnVoicelineInterrupted OnVoicelineInterrupted;

protected:
    // Internal voiceline management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voiceline State")
    FNarr_ActiveVoiceline CurrentVoiceline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline Database")
    TMap<FString, FNarr_VoicelineData> VoicelineDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline Settings")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline Settings")
    bool bAllowInterruptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline Settings")
    float DefaultCooldownTime;

    // Cooldown tracking
    UPROPERTY()
    TMap<FString, float> VoicelineCooldowns;

    // Internal helper functions
    void UpdateVoicelinePlayback(float DeltaTime);
    void UpdateCooldowns(float DeltaTime);
    bool IsVoicelineOnCooldown(const FString& VoicelineID) const;
    void StartVoicelineCooldown(const FString& VoicelineID, float CooldownDuration);
    void OnAudioComponentFinished();
    FNarr_VoicelineData* FindVoicelineByType(ENarr_VoicelineType VoicelineType, ENarr_VoicelinePriority MinPriority);
};

#include "Narr_VoicelineController.generated.h"