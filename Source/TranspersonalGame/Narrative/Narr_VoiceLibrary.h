#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataAsset.h"
#include "Sound/SoundBase.h"
#include "Narr_VoiceLibrary.generated.h"

UENUM(BlueprintType)
enum class ENarr_CharacterVoiceType : uint8
{
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    Scout           UMETA(DisplayName = "Scout"),
    Mentor          UMETA(DisplayName = "Mentor"),
    HuntLeader      UMETA(DisplayName = "Hunt Leader"),
    Storyteller     UMETA(DisplayName = "Storyteller"),
    Warrior         UMETA(DisplayName = "Warrior"),
    Healer          UMETA(DisplayName = "Healer"),
    Child           UMETA(DisplayName = "Child")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_VoiceClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString ClipID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString TranscriptText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    ENarr_CharacterVoiceType VoiceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    bool bIsLooped;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    TSoftObjectPtr<USoundBase> LoadedAudio;

    FNarr_VoiceClip()
    {
        ClipID = TEXT("");
        AudioURL = TEXT("");
        TranscriptText = TEXT("");
        VoiceType = ENarr_CharacterVoiceType::TribalElder;
        Duration = 0.0f;
        bIsLooped = false;
        LoadedAudio = nullptr;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_VoiceLibrary : public UDataAsset
{
    GENERATED_BODY()

public:
    UNarr_VoiceLibrary();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Library")
    TArray<FNarr_VoiceClip> VoiceClips;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Library")
    TMap<ENarr_CharacterVoiceType, TArray<FString>> VoiceClipsByType;

public:
    UFUNCTION(BlueprintCallable, Category = "Voice Library")
    FNarr_VoiceClip GetVoiceClip(const FString& ClipID) const;

    UFUNCTION(BlueprintCallable, Category = "Voice Library")
    TArray<FNarr_VoiceClip> GetVoiceClipsByType(ENarr_CharacterVoiceType VoiceType) const;

    UFUNCTION(BlueprintCallable, Category = "Voice Library")
    FNarr_VoiceClip GetRandomVoiceClip(ENarr_CharacterVoiceType VoiceType) const;

    UFUNCTION(BlueprintCallable, Category = "Voice Library")
    void AddVoiceClip(const FNarr_VoiceClip& NewClip);

    UFUNCTION(BlueprintCallable, Category = "Voice Library")
    bool RemoveVoiceClip(const FString& ClipID);

    UFUNCTION(BlueprintCallable, Category = "Voice Library")
    void InitializeDefaultVoiceClips();

    UFUNCTION(BlueprintCallable, Category = "Voice Library")
    int32 GetVoiceClipCount() const { return VoiceClips.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Voice Library")
    TArray<FString> GetAllClipIDs() const;

protected:
    void RebuildVoiceClipsByType();
};