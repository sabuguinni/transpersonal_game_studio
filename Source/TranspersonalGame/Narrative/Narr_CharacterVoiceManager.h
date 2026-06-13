#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "Narr_CharacterVoiceManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_VoiceType : uint8
{
    None UMETA(DisplayName = "None"),
    ElderHunter UMETA(DisplayName = "Elder Hunter"),
    YoungScout UMETA(DisplayName = "Young Scout"),
    TribalLeader UMETA(DisplayName = "Tribal Leader"),
    WiseWoman UMETA(DisplayName = "Wise Woman"),
    WarriorChief UMETA(DisplayName = "Warrior Chief"),
    Storyteller UMETA(DisplayName = "Storyteller"),
    Shaman UMETA(DisplayName = "Shaman"),
    Survivor UMETA(DisplayName = "Survivor")
};

UENUM(BlueprintType)
enum class ENarr_EmotionalTone : uint8
{
    Neutral UMETA(DisplayName = "Neutral"),
    Urgent UMETA(DisplayName = "Urgent"),
    Fearful UMETA(DisplayName = "Fearful"),
    Confident UMETA(DisplayName = "Confident"),
    Wise UMETA(DisplayName = "Wise"),
    Aggressive UMETA(DisplayName = "Aggressive"),
    Cautious UMETA(DisplayName = "Cautious"),
    Encouraging UMETA(DisplayName = "Encouraging")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    ENarr_VoiceType VoiceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    ENarr_EmotionalTone EmotionalTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString AudioFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    bool bIsLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    float Volume;

    FNarr_VoiceLine()
    {
        LineID = TEXT("");
        DialogueText = TEXT("");
        VoiceType = ENarr_VoiceType::None;
        EmotionalTone = ENarr_EmotionalTone::Neutral;
        AudioFilePath = TEXT("");
        Duration = 3.0f;
        bIsLooping = false;
        Volume = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterVoiceProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Profile")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Profile")
    ENarr_VoiceType DefaultVoiceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Profile")
    TArray<FNarr_VoiceLine> VoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Profile")
    float BasePitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Profile")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Profile")
    bool bIsActive;

    FNarr_CharacterVoiceProfile()
    {
        CharacterName = TEXT("Unknown");
        DefaultVoiceType = ENarr_VoiceType::Survivor;
        BasePitch = 1.0f;
        BaseVolume = 0.8f;
        bIsActive = true;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_CharacterVoiceManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Management")
    TMap<FString, FNarr_CharacterVoiceProfile> CharacterVoiceProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Management")
    bool bVoiceSystemEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Management")
    float GlobalVoiceVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Management")
    float VoiceLineSpacing;

public:
    UFUNCTION(BlueprintCallable, Category = "Voice Management")
    void RegisterCharacterVoice(const FString& CharacterName, ENarr_VoiceType VoiceType, float Pitch = 1.0f, float Volume = 0.8f);

    UFUNCTION(BlueprintCallable, Category = "Voice Management")
    void AddVoiceLineToCharacter(const FString& CharacterName, const FString& LineID, const FString& DialogueText, ENarr_EmotionalTone Tone, const FString& AudioPath = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Voice Management")
    bool PlayCharacterVoiceLine(const FString& CharacterName, const FString& LineID);

    UFUNCTION(BlueprintCallable, Category = "Voice Management")
    bool PlayCharacterVoiceByTone(const FString& CharacterName, ENarr_EmotionalTone Tone);

    UFUNCTION(BlueprintCallable, Category = "Voice Management")
    void SetVoiceSystemEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Voice Management")
    void SetGlobalVoiceVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Voice Management")
    FNarr_VoiceLine GetVoiceLineByID(const FString& CharacterName, const FString& LineID) const;

    UFUNCTION(BlueprintCallable, Category = "Voice Management")
    TArray<FNarr_VoiceLine> GetVoiceLinesByTone(const FString& CharacterName, ENarr_EmotionalTone Tone) const;

    UFUNCTION(BlueprintCallable, Category = "Voice Management")
    bool HasCharacterVoiceProfile(const FString& CharacterName) const;

    UFUNCTION(BlueprintCallable, Category = "Voice Management")
    void LoadVoiceProfilesFromDataTable(class UDataTable* VoiceDataTable);

    UFUNCTION(BlueprintCallable, Category = "Voice Management")
    void InitializeDefaultVoiceProfiles();

protected:
    UFUNCTION()
    void PlayVoiceLineInternal(const FNarr_VoiceLine& VoiceLine, const FNarr_CharacterVoiceProfile& Profile);

    UFUNCTION()
    FString GetVoiceTypeDisplayName(ENarr_VoiceType VoiceType) const;

    UFUNCTION()
    FString GetEmotionalToneDisplayName(ENarr_EmotionalTone Tone) const;
};