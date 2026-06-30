#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "DialogueSystem.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueTone : uint8
{
    Urgent      UMETA(DisplayName = "Urgent"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Informative UMETA(DisplayName = "Informative"),
    Warning     UMETA(DisplayName = "Warning"),
    Tactical    UMETA(DisplayName = "Tactical")
};

UENUM(BlueprintType)
enum class ENarr_CharacterRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Scout       UMETA(DisplayName = "Scout"),
    Elder       UMETA(DisplayName = "Elder"),
    Tracker     UMETA(DisplayName = "Tracker"),
    Survivor    UMETA(DisplayName = "Survivor")
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueTone Tone = ENarr_DialogueTone::Informative;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float TriggerRadius = 400.0f;

    FNarr_DialogueLine()
        : CharacterName(TEXT("Unknown"))
        , LineText(TEXT(""))
        , AudioURL(TEXT(""))
        , TriggerRadius(400.0f)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_NPCProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|NPC")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|NPC")
    ENarr_CharacterRole Role = ENarr_CharacterRole::Survivor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|NPC")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|NPC")
    FString BackstoryBrief;

    FNarr_NPCProfile()
        : Name(TEXT("Unknown"))
        , BackstoryBrief(TEXT(""))
    {}
};

// ─── Dialogue Trigger Actor ───────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, ClassGroup = "Narrative")
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void TriggerDialogue(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void RegisterDialogueLine(const FNarr_DialogueLine& Line);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool HasUnplayedLines() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FNarr_DialogueLine GetNextLine();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FNarr_NPCProfile NPCProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bOneShot = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float TriggerRadius = 400.0f;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative|Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    int32 CurrentLineIndex = 0;
    bool bHasTriggered = false;

    UFUNCTION()
    void OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};

// ─── Dialogue Manager (World Subsystem) ──────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Manager")
    void RegisterTrigger(ANarr_DialogueTrigger* Trigger);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Manager")
    void UnregisterTrigger(ANarr_DialogueTrigger* Trigger);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Manager")
    int32 GetActiveTriggerCount() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Manager")
    void LoadVoiceLineRegistry();

private:
    UPROPERTY()
    TArray<ANarr_DialogueTrigger*> RegisteredTriggers;

    // Voice line registry — character name → audio URL
    TMap<FString, FString> VoiceLineRegistry;

    void PopulateDefaultRegistry();
};
