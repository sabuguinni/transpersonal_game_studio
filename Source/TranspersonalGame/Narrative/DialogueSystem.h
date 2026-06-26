// DialogueSystem.h
// Agent #15 — Narrative & Dialogue Agent
// Dialogue trigger system for prehistoric survival game
// Activates context-sensitive dialogue when player enters proximity zones

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "DialogueSystem.generated.h"

// Narr_ prefix to avoid name collisions across agents

UENUM(BlueprintType)
enum class ENarr_DialogueTriggerType : uint8
{
    DinosaurEncounter   UMETA(DisplayName = "Dinosaur Encounter"),
    ResourceDiscovery   UMETA(DisplayName = "Resource Discovery"),
    DangerWarning       UMETA(DisplayName = "Danger Warning"),
    CampFound           UMETA(DisplayName = "Camp Found"),
    TrailDiscovery      UMETA(DisplayName = "Trail Discovery")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTriggerType TriggerType;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , LineText(TEXT(""))
        , DisplayDuration(4.0f)
        , TriggerType(ENarr_DialogueTriggerType::DinosaurEncounter)
    {}
};

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ANarr_DialogueTriggerActor : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTriggerActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative",
        meta = (AllowPrivateAccess = "true"))
    UBoxComponent* TriggerVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bOneShot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ActivateDialogue(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetTrigger();

    UFUNCTION(BlueprintPure, Category = "Narrative")
    bool HasBeenTriggered() const { return bHasBeenTriggered; }

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

private:
    bool bHasBeenTriggered;
    int32 CurrentLineIndex;

    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};
