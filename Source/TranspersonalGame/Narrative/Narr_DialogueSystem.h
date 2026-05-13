#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

// Forward declarations
class UNarr_DialogueComponent;
class ANarr_DialogueActor;

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Narrative,
    Character,
    Tutorial,
    Warning,
    Discovery,
    Quest
};

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    Proximity,
    Interaction,
    QuestProgress,
    TimeOfDay,
    PlayerState,
    Environmental
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundBase> VoiceClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bSubtitlesEnabled;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , DialogueText(FText::FromString(TEXT("...")))
        , DialogueType(ENarr_DialogueType::Narrative)
        , Duration(3.0f)
        , bSubtitlesEnabled(true)
    {
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 Priority;

    FNarr_DialogueSequence()
        : SequenceID(TEXT("Default"))
        , TriggerType(ENarr_DialogueTrigger::Proximity)
        , bRepeatable(false)
        , Priority(0)
    {
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Dialogue management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogueSequence(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueSequence(const FNarr_DialogueSequence& Sequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FString GetCurrentSpeaker() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FText GetCurrentDialogueText() const;

    // Trigger system
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void CheckProximityTriggers(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDialogueByType(ENarr_DialogueTrigger TriggerType);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float ProximityTriggerDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bAutoPlayOnTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DialogueVolume;

private:
    FNarr_DialogueSequence* CurrentSequence;
    int32 CurrentLineIndex;
    float CurrentLineTimer;
    bool bIsPlaying;
    TArray<FString> PlayedSequences;

    void PlayNextLine();
    void OnDialogueLineComplete();
    bool CanPlaySequence(const FNarr_DialogueSequence& Sequence) const;
};

UCLASS()
class TRANSPERSONALGAME_API ANarr_DialogueActor : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDialogue(AActor* PlayerActor);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNarr_DialogueComponent* DialogueComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* TriggerSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DefaultSequenceID;

    UFUNCTION()
    void OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueActor(ANarr_DialogueActor* DialogueActor);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void UnregisterDialogueActor(ANarr_DialogueActor* DialogueActor);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayGlobalDialogue(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopAllDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<ANarr_DialogueActor*> GetNearbyDialogueActors(const FVector& Location, float Radius) const;

protected:
    UPROPERTY()
    TArray<ANarr_DialogueActor*> RegisteredDialogueActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueSequence> GlobalDialogueSequences;
};