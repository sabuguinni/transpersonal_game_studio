#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ESurvivalContext Context;

    FNarr_DialogueLine()
    {
        CharacterName = TEXT("");
        DialogueText = TEXT("");
        AudioURL = TEXT("");
        Duration = 0.0f;
        Context = ESurvivalContext::Exploration;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ESurvivalSkill Expertise;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool bIsAvailable;

    FNarr_CharacterProfile()
    {
        CharacterName = TEXT("");
        BackgroundStory = TEXT("");
        Expertise = ESurvivalSkill::Gathering;
        bIsAvailable = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueSystem : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* InteractionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* VisualMarker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_CharacterProfile> Characters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FNarr_CharacterProfile CurrentCharacter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 CurrentDialogueIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsPlayingDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float InteractionRange;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void InitializeCharacters();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetCurrentCharacter(const FString& CharacterName);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueLine(int32 LineIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayNextDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueLine GetDialogueByContext(ESurvivalContext Context);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetAvailableCharacters();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Dialogue")
    void TestDialogueSystem();

protected:
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    void OnAudioFinished();

private:
    void CreateCharacterProfiles();
    void LoadAudioFromURL(const FString& AudioURL);
    
    UPROPERTY()
    class ATranspersonalCharacter* PlayerCharacter;

    UPROPERTY()
    bool bPlayerInRange;
};