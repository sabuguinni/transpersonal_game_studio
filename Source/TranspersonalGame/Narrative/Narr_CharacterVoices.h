#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "Narr_CharacterVoices.generated.h"

UENUM(BlueprintType)
enum class ENarr_VoiceType : uint8
{
    TribalElder,
    Scout,
    Warrior,
    Craftsman,
    Narrator,
    Beast
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_VoiceEntry : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    ENarr_VoiceType VoiceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString VoiceDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    TSoftObjectPtr<USoundBase> VoiceClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FText SampleDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    float BasePitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    float BaseVolume;

    FNarr_VoiceEntry()
    {
        VoiceType = ENarr_VoiceType::Narrator;
        CharacterName = TEXT("Unknown");
        VoiceDescription = TEXT("Generic voice");
        SampleDialogue = FText::FromString(TEXT("Sample dialogue"));
        BasePitch = 1.0f;
        BaseVolume = 1.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_CharacterVoices : public UObject
{
    GENERATED_BODY()

public:
    UNarr_CharacterVoices();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Database")
    TArray<FNarr_VoiceEntry> VoiceEntries;

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    FNarr_VoiceEntry GetVoiceByType(ENarr_VoiceType VoiceType) const;

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    FNarr_VoiceEntry GetVoiceByName(const FString& CharacterName) const;

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    void AddVoiceEntry(const FNarr_VoiceEntry& NewVoice);

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    TArray<FNarr_VoiceEntry> GetAllVoicesOfType(ENarr_VoiceType VoiceType) const;

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    void InitializeDefaultVoices();
};