#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundBase.h"
#include "DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueTriggerType : uint8
{
    Proximity       UMETA(DisplayName = "Proximity"),
    QuestStart      UMETA(DisplayName = "QuestStart"),
    QuestComplete   UMETA(DisplayName = "QuestComplete"),
    CombatAlert     UMETA(DisplayName = "CombatAlert"),
    ResourceFound   UMETA(DisplayName = "ResourceFound"),
    PlayerDeath     UMETA(DisplayName = "PlayerDeath")
};

UENUM(BlueprintType)
enum class ENarr_SpeakerRole : uint8
{
    ChiefHunter     UMETA(DisplayName = "ChiefHunter"),
    TribalElder     UMETA(DisplayName = "TribalElder"),
    ScoutRunner     UMETA(DisplayName = "ScoutRunner"),
    CampBuilder     UMETA(DisplayName = "CampBuilder"),
    Narrator        UMETA(DisplayName = "Narrator")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_SpeakerRole Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    USoundBase* VoiceAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float CooldownSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bPlayedThisSession;

    FNarr_DialogueLine()
        : LineText(TEXT(""))
        , Speaker(ENarr_SpeakerRole::Narrator)
        , TriggerType(ENarr_DialogueTriggerType::Proximity)
        , VoiceAsset(nullptr)
        , CooldownSeconds(30.0f)
        , bPlayedThisSession(false)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_NPCDialogueBank
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_SpeakerRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 CurrentLineIndex;

    FNarr_NPCDialogueBank()
        : Role(ENarr_SpeakerRole::Narrator)
        , CurrentLineIndex(0)
    {}
};

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueTriggerComponent : public USphereComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueTriggerComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_SpeakerRole AssignedSpeaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bOneShot;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    bool bHasTriggered;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ActivateTrigger(AActor* InstigatorActor);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetTrigger();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};

UCLASS()
class TRANSPERSONALGAME_API ANarr_DialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_NPCDialogueBank> DialogueBanks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float GlobalDialogueCooldown;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    float LastDialogueTime;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(ENarr_SpeakerRole Speaker, ENarr_DialogueTriggerType TriggerType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetNextLine(ENarr_SpeakerRole Speaker);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueBank(FNarr_NPCDialogueBank Bank);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetAllDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool CanPlayDialogue() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void InitialiseDefaultBanks();
};
