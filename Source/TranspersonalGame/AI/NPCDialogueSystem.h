#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "NPCDialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENPC_EmotionalState RequiredMood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 Priority;

    FNPC_DialogueLine()
    {
        DialogueText = TEXT("");
        AudioURL = TEXT("");
        Duration = 0.0f;
        RequiredMood = ENPC_EmotionalState::Neutral;
        Priority = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_DialogueSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNPC_DialogueLine> GreetingLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNPC_DialogueLine> CasualLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNPC_DialogueLine> WarningLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNPC_DialogueLine> FarewellLines;

    FNPC_DialogueSet()
    {
        CharacterName = TEXT("Unknown");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCDialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCDialogueSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Dialogue Management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void InitializeDialogue(const FString& CharacterName);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNPC_DialogueLine GetRandomDialogue(ENPC_DialogueType DialogueType);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogue(const FNPC_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialoguePlaying() const { return bIsDialoguePlaying; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopCurrentDialogue();

    // Audio Management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void LoadAudioFromURL(const FString& AudioURL);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayLoadedAudio();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FNPC_DialogueSet DialogueSet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DialogueCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float InteractionRange;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsDialoguePlaying;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastDialogueTime;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FNPC_DialogueLine CurrentDialogue;

    // Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* VoiceAudioComponent;

private:
    void SetupDefaultDialogues();
    bool CanPlayDialogue() const;
    TArray<FNPC_DialogueLine>* GetDialogueArrayByType(ENPC_DialogueType DialogueType);
};