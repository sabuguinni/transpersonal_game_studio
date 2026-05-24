#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "Narr_VoicelineManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_VoicelineType : uint8
{
    Narrator        UMETA(DisplayName = "Narrator"),
    PlayerThought   UMETA(DisplayName = "Player Thought"),
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    WildernessGuide UMETA(DisplayName = "Wilderness Guide"),
    SurvivalTip     UMETA(DisplayName = "Survival Tip"),
    DangerWarning   UMETA(DisplayName = "Danger Warning")
};

UENUM(BlueprintType)
enum class ENarr_VoicelineTrigger : uint8
{
    OnPlayerSpawn       UMETA(DisplayName = "On Player Spawn"),
    OnDinosaurSighting  UMETA(DisplayName = "On Dinosaur Sighting"),
    OnLowHealth         UMETA(DisplayName = "On Low Health"),
    OnCrafting          UMETA(DisplayName = "On Crafting"),
    OnNightfall         UMETA(DisplayName = "On Nightfall"),
    OnDanger            UMETA(DisplayName = "On Danger"),
    OnDiscovery         UMETA(DisplayName = "On Discovery"),
    OnSurvival          UMETA(DisplayName = "On Survival")
};

USTRUCT(BlueprintType)
struct FNarr_VoicelineData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    FString VoicelineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    ENarr_VoicelineType VoicelineType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    ENarr_VoicelineTrigger TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    bool bIsPlayed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    float CooldownTime;

    FNarr_VoicelineData()
    {
        VoicelineText = TEXT("");
        VoicelineType = ENarr_VoicelineType::Narrator;
        TriggerCondition = ENarr_VoicelineTrigger::OnPlayerSpawn;
        Duration = 10.0f;
        AudioURL = TEXT("");
        bIsPlayed = false;
        CooldownTime = 30.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_VoicelineManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_VoicelineManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Voiceline Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_VoicelineData> VoicelineDatabase;

    // Current Playing State
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bIsPlayingVoiceline;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FNarr_VoicelineData CurrentVoiceline;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    float CurrentPlaybackTime;

    // Trigger Functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerVoiceline(ENarr_VoicelineTrigger TriggerType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayVoiceline(const FNarr_VoicelineData& VoicelineData);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopCurrentVoiceline();

    // Voiceline Management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AddVoiceline(const FNarr_VoicelineData& NewVoiceline);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_VoicelineData> GetVoicelinesByType(ENarr_VoicelineType VoicelineType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_VoicelineData> GetVoicelinesByTrigger(ENarr_VoicelineTrigger TriggerType);

    // Survival Context Integration
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnPlayerHealthLow();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnDinosaurDetected(const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnCraftingStarted(const FString& ItemType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnNightfallApproaching();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnDiscoveryMade(const FString& DiscoveryType);

private:
    // Internal State
    float LastVoicelineTime;
    TMap<ENarr_VoicelineTrigger, float> TriggerCooldowns;

    // Helper Functions
    bool CanPlayVoiceline(const FNarr_VoicelineData& VoicelineData);
    void UpdateCooldowns(float DeltaTime);
    FNarr_VoicelineData SelectRandomVoiceline(const TArray<FNarr_VoicelineData>& Voicelines);
};