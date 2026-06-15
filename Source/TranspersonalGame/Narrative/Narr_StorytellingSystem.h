#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/TriggerBox.h"
#include "TranspersonalCharacter.h"
#include "Narr_StorytellingSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryType : uint8
{
    Warning         UMETA(DisplayName = "Warning"),
    Wisdom          UMETA(DisplayName = "Wisdom"),
    Lore            UMETA(DisplayName = "Lore"),
    Aftermath       UMETA(DisplayName = "Aftermath"),
    Migration       UMETA(DisplayName = "Migration"),
    Hunting         UMETA(DisplayName = "Hunting")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString StoryID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryType StoryType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    class USoundBase* AudioClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bPlayOnce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bHasBeenTriggered;

    FNarr_StoryData()
    {
        StoryID = TEXT("");
        StoryType = ENarr_StoryType::Lore;
        DialogueText = TEXT("");
        AudioClip = nullptr;
        TriggerRadius = 500.0f;
        bPlayOnce = true;
        bHasBeenTriggered = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_StorytellingSystem : public AActor
{
    GENERATED_BODY()

public:
    ANarr_StorytellingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<FNarr_StoryData> StoryDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    float PlayerDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    float StoryTriggerCooldown;

    UPROPERTY(BlueprintReadOnly, Category = "Storytelling")
    float LastStoryTime;

    UPROPERTY(BlueprintReadOnly, Category = "Storytelling")
    class ATranspersonalCharacter* PlayerCharacter;

public:
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void TriggerStoryByLocation(FVector Location, ENarr_StoryType StoryType);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void TriggerStoryByID(const FString& StoryID);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void AddStoryToDatabase(const FNarr_StoryData& NewStory);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    bool CanTriggerStory() const;

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    FNarr_StoryData GetRandomStoryByType(ENarr_StoryType StoryType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Storytelling")
    void OnStoryTriggered(const FNarr_StoryData& StoryData);

    UFUNCTION(BlueprintImplementableEvent, Category = "Storytelling")
    void OnStoryCompleted(const FNarr_StoryData& StoryData);

private:
    void InitializeStoryDatabase();
    void CheckPlayerProximity();
    void PlayStoryAudio(const FNarr_StoryData& StoryData);
    FNarr_StoryData* FindStoryByID(const FString& StoryID);
};