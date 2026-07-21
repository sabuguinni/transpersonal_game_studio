#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ResponseOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 NextDialogueID;

    FNarr_DialogueEntry()
    {
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        AudioPath = TEXT("");
        Duration = 3.0f;
        NextDialogueID = -1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString VoiceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> KnownTopics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float TrustLevel;

    FNarr_CharacterProfile()
    {
        CharacterName = TEXT("");
        VoiceType = TEXT("Default");
        Personality = TEXT("Neutral");
        TrustLevel = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Narrative), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FNarr_DialogueEntry> DialogueDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FNarr_CharacterProfile> CharacterProfiles;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue System")
    int32 CurrentDialogueID;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue System")
    bool bIsDialogueActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    float DialogueRange;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    bool StartDialogue(const FString& CharacterName, int32 StartingDialogueID = 0);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    bool AdvanceDialogue(int32 ResponseChoice = 0);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    FNarr_DialogueEntry GetCurrentDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    TArray<FString> GetCurrentResponses() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void AddDialogueEntry(const FNarr_DialogueEntry& NewEntry);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void AddCharacterProfile(const FNarr_CharacterProfile& NewProfile);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    FNarr_CharacterProfile GetCharacterProfile(const FString& CharacterName) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void UpdateTrustLevel(const FString& CharacterName, float DeltaTrust);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void InitializeTribalDialogues();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void InitializeSurvivalDialogues();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void InitializeHuntingDialogues();

private:
    void LoadDefaultDialogues();
    void LoadDefaultCharacters();
    int32 FindDialogueIndex(int32 DialogueID) const;
    int32 FindCharacterIndex(const FString& CharacterName) const;
};