#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "NPCInteractionSystem.generated.h"

UENUM(BlueprintType)
enum class ENPC_InteractionType : uint8
{
    None = 0,
    Dialogue,
    Trade,
    Quest,
    Information,
    Warning,
    Hostile
};

USTRUCT(BlueprintType)
struct FNPC_InteractionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    ENPC_InteractionType InteractionType = ENPC_InteractionType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString InteractionText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bRequiresLineOfSight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float CooldownTime = 5.0f;

    FNPC_InteractionData()
    {
        InteractionType = ENPC_InteractionType::None;
        InteractionText = TEXT("Hello there!");
        InteractionRange = 300.0f;
        bRequiresLineOfSight = true;
        CooldownTime = 5.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCInteractionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCInteractionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Interaction Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Interaction")
    FNPC_InteractionData InteractionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Interaction")
    TArray<FNPC_InteractionData> AvailableInteractions;

    UPROPERTY(BlueprintReadOnly, Category = "NPC Interaction")
    APawn* CurrentInteractingPlayer;

    UPROPERTY(BlueprintReadOnly, Category = "NPC Interaction")
    float LastInteractionTime;

    // Interaction Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    bool CanInteractWithPlayer(APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    bool StartInteraction(APawn* Player, ENPC_InteractionType InteractionType);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void EndInteraction();

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    FString GetInteractionPrompt(APawn* Player);

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC Interaction")
    void OnInteractionStarted(APawn* Player, ENPC_InteractionType InteractionType);

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC Interaction")
    void OnInteractionEnded();

    // Line of sight check
    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    bool HasLineOfSightToPlayer(APawn* Player);

private:
    // Internal state
    bool bIsInteracting;
    float InteractionCooldownTimer;

    // Helper functions
    float GetDistanceToPlayer(APawn* Player);
    bool IsPlayerInRange(APawn* Player, float Range);
};