#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "NarrativeDialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueTriggerType : uint8
{
    Proximity       UMETA(DisplayName = "Proximity"),
    QuestStart      UMETA(DisplayName = "Quest Start"),
    DinoEncounter   UMETA(DisplayName = "Dino Encounter"),
    Discovery       UMETA(DisplayName = "Discovery"),
    Survival        UMETA(DisplayName = "Survival Warning")
};

UENUM(BlueprintType)
enum class ENarr_SpeakerRole : uint8
{
    Elder       UMETA(DisplayName = "Elder"),
    Scout       UMETA(DisplayName = "Scout"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Narrator    UMETA(DisplayName = "Narrator")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_SpeakerRole SpeakerRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , SpeakerRole(ENarr_SpeakerRole::Narrator)
        , LineText(TEXT(""))
        , AudioAssetPath(TEXT(""))
        , DisplayDuration(5.0f)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasPlayed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bCanRepeat;

    FNarr_DialogueSequence()
        : SequenceID(TEXT(""))
        , TriggerType(ENarr_DialogueTriggerType::Proximity)
        , bHasPlayed(false)
        , bCanRepeat(false)
    {}
};

/**
 * ANarr_DialogueTrigger — placed in the world near key locations.
 * When the player enters the sphere radius, plays a contextual dialogue line
 * from a nearby NPC (scout, elder, hunter) about the environment or danger.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayNextLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsSequenceComplete() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetSequence();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

protected:
    UFUNCTION()
    void OnPlayerEnterRadius(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    FNarr_DialogueSequence DialogueSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    bool bDebugDraw;

    int32 CurrentLineIndex;
    bool bIsPlaying;
    float LineTimer;
};
