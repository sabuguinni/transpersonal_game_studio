#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_TribalCharacterSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_TribalCharacterData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ENarr_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 ExperienceLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> KnownDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString VoiceAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool bIsHostile;

    FNarr_TribalCharacterData()
    {
        CharacterName = TEXT("Unknown");
        TribalRole = ENarr_TribalRole::Hunter;
        Age = 25;
        ExperienceLevel = 1;
        VoiceAssetPath = TEXT("");
        TrustLevel = 0.5f;
        bIsHostile = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueEmotion Emotion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> RequiredConditions;

    FNarr_DialogueLine()
    {
        DialogueText = TEXT("");
        AudioAssetPath = TEXT("");
        Emotion = ENarr_DialogueEmotion::Neutral;
        Duration = 5.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_TribalCharacterSystem : public AActor
{
    GENERATED_BODY()

public:
    ANarr_TribalCharacterSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* CharacterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* VoiceAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
    FNarr_TribalCharacterData CharacterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> AvailableDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    class UDataTable* DialogueDataTable;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsInConversation;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    class APawn* CurrentInteractingPlayer;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void StartConversation(APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void EndConversation();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueLine(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetAvailableDialogueOptions();

    UFUNCTION(BlueprintCallable, Category = "Character")
    void UpdateTrustLevel(float DeltaAmount);

    UFUNCTION(BlueprintCallable, Category = "Character")
    bool CanInteract(APawn* Player) const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnConversationStarted(APawn* Player);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnConversationEnded();

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnDialogueLineFinished(const FString& DialogueID);

protected:
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    void OnVoiceAudioFinished();

    void LoadDialogueData();
    void InitializeCharacterAppearance();
    FNarr_DialogueLine* FindDialogueLine(const FString& DialogueID);
    bool CheckDialogueConditions(const TArray<FString>& Conditions) const;
};