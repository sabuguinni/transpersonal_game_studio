#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Narr_DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Warning,
    Information,
    Quest,
    Lore,
    Tutorial
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bOneTimeOnly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioFilePath;

    FNarr_DialogueEntry()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
        DialogueType = ENarr_DialogueType::Information;
        TriggerRadius = 500.0f;
        bOneTimeOnly = true;
        AudioFilePath = TEXT("");
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
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualMarker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    UDataTable* DialogueTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FName DialogueRowName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bHasBeenTriggered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* DialogueSound;

    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                              bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDialogue(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueEntry GetDialogueEntry();

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void OnDialogueTriggered(const FNarr_DialogueEntry& DialogueEntry);

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetDialogueEntry(FName RowName);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueAvailable() const;
};

#include "Narr_DialogueSystem.generated.h"